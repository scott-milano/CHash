/**
 * @file
 * @author Scott Milano
 * @copyright Copyright 2019 Scott Milano
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @brief Generic Hashing and Storage
 *
 * @defgroup HASH Hash List module
 * @brief This module provides a simple binary sorted list.
 * @details This library provides a generic has that allows for
 * key type and value type to be defined at compile time.
 *
 * The list can be generated with the macros defined in hash.h
 * - @ref DEFINE_LIST()
 * - @ref DEFINE_HASH()
 *
 * These functions generate the list along with the following
 * accesor methods.  Method name prefix is the first parameter
 * passed to the macro. For example:
 * \code{.c}
 * DEFINE_LIST(List,int,in_addr_t)
 *
 * DEFINE_HASH(List,in_addr_t)
 * \endcode
 *
 * @copydetails LIST_FUNCTION_GET
 * <hr>
 *
 * @copydetails LIST_FUNCTION_SET
 * <hr>
 *
 * @copydetails LIST_FUNCTION_VAL
 * <hr>
 *
 * @copydetails LIST_FUNCTION_PTR
 * <hr>
 * @copydetails LIST_FUNCTION_PTR
 * <hr>
 * @copydetails LIST_FUNCTION_COUNT
 * <hr>
 * @copydetails LIST_FUNCTION_KEYS
 * <hr>
 * @copydetails LIST_FUNCTION_ITEM
 * <hr>
 * @copydetails LIST_FUNCTION_INDEX
 * <hr>
 * @copydetails LIST_FUNCTION_HASKEY
 * <hr>
 * @copydetails LIST_FUNCTION_DEL
 * <hr>
 * @copydetails LIST_FUNCTION_NETSTART
 * <hr>
 * @copydetails LIST_FUNCTION_LOAD
 * <hr>
 * @copydetails LIST_FUNCTION_SAVE
 * <hr>
 * @copydetails LIST_FUNCTION_FREE
 * <hr>
 * @copydetails LIST_FUNCTION_LOCK
 *
 * The following calls were introduced for the FIFO but are available for
 * other types.
 *
 * @copydetails LIST_FUNCTION_POP
 * <hr>
 * @copydetails LIST_FUNCTION_NEXT
 *
 * This final call only makes sense for fifo types which use timespec key
 * @copydetails LIST_FUNCTION_PUSH
 * <hr>
 *
 *
 * @{
 *
 */

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<search.h>
#include<unistd.h>
#include<assert.h>

#include "hash.h"
#include "repl.h"
#include "entry.h"

#ifdef HDEBUG
#include <errno.h>
bool g_debug=false;  /* Global debug flag */
#endif

/** @addtogroup HASH @{ */
/* Init: */
static inline void *list_init(list_store_t *store);
/* Resize: */
static inline void *list_resize(list_store_t *store);
/* Bsearch variation that returns existing value or new insert slot */
static inline void * bfind(const void *key, const void *base, size_t *nmemb, 
        size_t size, __compar_fn_t compar,size_t *slot);
static char *hash_print(const list_type_info_t *type,const void *val);

static uint32_t pyHash(const uint8_t *a,int s,uint32_t x);

#ifdef LIST_ENTRY_LOCK
/* Delete lock by index */
static bool _delete_lock(list_store_t *store,int index);
#endif

/**
 * @brief Internal list search function used by the hash library
 * This function initializes and reallocates memory as needed.
 * This function is based around qsort and bsearch
 *
 * Entries are added in sort order to allow binary search for quicker
 * lookup.
 * @param store pointer to storage structure.
 * @param key search key
 * @param valref Pointer to value, just return exiting value when NULL
 * a new entry is added to the end of the list.
 * @return pointer to the entry on successful lookup or NULL otherwise.
 * @note Do not call directly.
 */
void *_hash_search(list_store_t *store,void *keyref,void *valref)
{
    _entry_t *eptr=NULL;
    assert(store);

    if (list_init(store)) { /* If needed initialize new or free'd list */
        size_t slot=store->index;   /**< Slot for entry insertion */
        _entry_t entry;             /**< Temp entry for key lookup */

        /* Check if list needs to be increased */
        if ((valref)&&(!list_resize(store))) {
            /* Don't allow new entry on failure to increase size */
            valref=NULL;
        }

#ifndef LSEARCH
        /* Place key in temp entry for comparison, and convert pointers for
         * strings char ** to char* */
        entry.key=keyref;
        eptr=bfind(&entry, store->list, &store->index,store->size,
                    store->key.cmp,&slot);
#else
        /* Place key in temp entry for comparison, and convert pointers for
         * strings char ** to char* */
        entry.key=keyref;
        eptr=lfind(&entry, store->list, &store->index,store->size,
                store->key.cmp);
        slot=store->index;
#endif

        if (eptr) {
            /* Value already exists, Update it with new value */
            if (valref)
                memcpy(eptr->val,valref,store->value.size);
        } else if (valref) {
            /* Convert value reference if needed */
            entry.val=valref;

            /* Move list up one entry for insertion. For lfind or bfind at end
             * of list, move size is zero */
            eptr=(((_entry_t *)store->list)+slot);
            memmove(eptr+1, eptr, (store->index-slot)*sizeof(*eptr));
            memset(eptr,0x00,sizeof(*eptr));

            /* Allocate values */
            eptr->key=store->key.alloc(entry.key);
            eptr->val=store->value.alloc(entry.val);
            if ((eptr->key)&&(eptr->val)) {
                store->index++;
            } else {
                /* On failure return values as needed and clear the slot */
                dbg("Mem:%s allocation failure: size: %lu slot: %lu key: %p  val: %p",
                        store->name,store->index,slot,eptr->key,eptr->val);
                if (eptr->key) free(eptr->key);
                if (eptr->val) free(eptr->val);
                memset(eptr,0x00,sizeof(*eptr));
                eptr=NULL;
            }
        }
    }

    return eptr;
}

bool _list_insert(list_store_t *store,void *keyref,void *valref)
{
    bool ret=false;
    _entry_t *eptr=NULL; /**< Pointer to entry for lookup/search */

    pthread_mutex_lock(&store->lock);
    eptr=_hash_search(store,keyref,valref);
    if (eptr) {
        dbgentry(eptr);
        ret=true;
        if (store->port) repl_update(store,eptr);
#ifdef LIST_ENTRY_LOCK
        eptr.lock=calloc(1,sizeof(pthread_mutex_t));
        if (eptr->lock) {
            pthread_mutex_init(eptr->lock, NULL);
            eptr->lock_en=true;
        } else {
            eptr->lock_en=false;
            dbg("Memory allocation for entry mutex: %p",eptr->lock);
        }
#endif
    }
    pthread_mutex_unlock(&store->lock);
    return ret;
}

void *_list_reference(list_store_t *store,void *keyref)
{
    _entry_t *eptr=NULL; /**< Pointer to entry for lookup/search */
    void *value=NULL;

    pthread_mutex_lock(&store->lock);
    eptr=_hash_search(store,keyref,false);
    if (eptr) {
        assert(eptr->val);
        value=eptr->val;
    }
    pthread_mutex_unlock(&store->lock);
    return value;
}

void *_list_valref(list_store_t *store,int index)
{
    _entry_t *eptr=NULL; /**< Pointer to entry for lookup/search */
    void *value=NULL;
    assert(store);

    pthread_mutex_lock(&store->lock);
    /* Ensure the store is initialized and has an entry */
    if ((store->list)&&(store->index)) {
        if ((index>=0)&&(index<store->index)&&(store->index)) {
            eptr=((_entry_t *)store->list)+index;
            /* Populate return value with pointer conversion if needed */
            if (eptr) value=eptr->val;
        }
        //dbg("Index %d at: %p",index,value);
    }
    pthread_mutex_unlock(&store->lock);
    return value;
}


void *_list_keyref(list_store_t *store,int index)
{
    _entry_t *eptr=NULL; /**< Pointer to entry for lookup/search */
    void *key=NULL;
    /* Handle pointers to keys */
    assert(store);

    pthread_mutex_lock(&store->lock);
    dbgindex(index);
    /* Ensure the store is initialized and has an entry */
    if ((store->list)&&(store->index)) {
        if ((index>=0)&&(index<store->index)&&(store->index)) {
            eptr=((_entry_t *)store->list)+index;
            /* Populate return value with pointer conversion if needed */
            if (eptr) key=eptr->key;
            dbg("I: %d Ptr: %p",index,eptr->key);
            dbgentry(eptr);
        }
    }
    pthread_mutex_unlock(&store->lock);
    return key;
}

bool _list_copy(list_store_t *store,void *keyref,void *value)
{
    bool ret=false;
    _entry_t *eptr=NULL; /**< Pointer to entry for lookup/search */
    assert(store);

    pthread_mutex_lock(&store->lock);
    eptr=_hash_search(store,keyref,false);
    if (eptr) {
        assert(eptr->val);
        store->value.cp(value,eptr->val);
        ret=true;
        dbgentry(eptr);
    }
    pthread_mutex_unlock(&store->lock);

    return ret;
}


/**
 * @brief Internal list lookup function used by the hash library
 * This function returns the entry at a specific location.
 *
 * Entries are added to the end of the list, and insert order is
 * maintained.
 * @param store pointer to storage structure.
 * @param index of item
 * @param key pointer for returned key
 * @param value pointer for returned value
 * @return true on success false on range error, with a wrapped return
 * @note Do not call directly.
 */
bool _list_items(list_store_t *store,int index,void *keyref,void *value)
{
    bool ret=false;
    _entry_t *eptr=NULL; /**< Pointer to entry for lookup/search */
    assert(store);

    pthread_mutex_lock(&store->lock);
    /* Ensure the store is initialized and has an entry */
    if ((store->list)&&(store->index)) {
        if ((index>=0)&&(index<store->index)) {
            eptr=((_entry_t *)store->list)+index;
            ret=true;
        } else {
            index%=((int)store->index);
            if (index<0) index+=store->index;
            eptr=((_entry_t *)store->list)+index;
            ret=false;
        }
        if (keyref) store->key.cp(keyref,eptr->key);
        if (value)  store->value.cp(value,eptr->val);
    }
    pthread_mutex_unlock(&store->lock);
    return ret;
}

/**
 * Lookup index of item by key
 * @param store pointer to store structure.
 * @param keyref pointer to the key
 * @return index of item on success, -1 on failure
 */
int _list_index(list_store_t *store,void *keyref)
{
    int index=-1;
    assert(store);

    pthread_mutex_lock(&store->lock);
    index=_find_index(store,keyref);
    dbgindex(index);
    pthread_mutex_unlock(&store->lock);
    return index;
}

/**
 * Start sharing list/hash on network at port
 * @param store pointer to store structure.
 * @param port mcast/udp port number
 * @return true on success, false on fail
 */
bool _list_netstart(list_store_t *store,uint16_t port)
{
    bool ret=true;
    dbg("Starting thread on port: %u",port);

    /* Ensure list is initialized */
    if (!list_init(store)) return false;

    if (store->net) {
        dbg("Error, thread already running");
        ret=false;
    } else {
        if (port) {
            store->port=port;
            ret=repl_start(store);
        } else {
            dbg("Error, port not non-zero");
            ret=false;
        }
    }

    return ret;
}

bool _list_load(list_store_t *store,char *file)
{
    FILE *fp=NULL;
    uint32_t loadId=0;
    void *key=NULL;
    void *val=NULL;
    assert(store);

    pthread_mutex_lock(&store->lock);
    /* Check if store is initialized */
    if (!list_init(store)) return false;
    pthread_mutex_unlock(&store->lock);
    
    dbg("list: %p, Size: %lu",store->list,store->index);
    
    /* Open file for reading */
    if ((fp=fopen(file,"r"))==NULL) {
        dbg("Failed to open file %s for reading: %s",file,strerror(errno));
        return false;
    }

    /* Check header matches this list/hash type and data sizes
     * Header: name hash */
    if ((fread(&loadId,sizeof(loadId),1,fp)!=1)||
        (loadId!=store->id)) {
        dbg("Header error for %s: %s",file,strerror(errno));
        fclose(fp);
        return false;
    }

    /* Allocate buffer for key */
    if ((key=malloc(store->key.size))==NULL) {
        dbg("Memory error for %s: %s",file,strerror(errno));
        fclose(fp);
        return false;
    }
    /* Allocate buffer for value */
    if ((val=malloc(store->value.size))==NULL) {
        dbg("Memory error for %s: %s",file,strerror(errno));
        free(key);
        fclose(fp);
        return false;
    }
    
    /* Loop through entries and store */
    while ((fread(key,store->key.size,1,fp))==1) {
        if (fread(val,store->value.size,1,fp)==1) {
            if (!_list_insert(store,key,val)) {
                dbg("Insert failed for %s: %s",file,strerror(errno));
                free(key);
                free(val);
                fclose(fp);
                return false;
            }
        } else {
            dbg("Value read failed for %s: %s",file,strerror(errno));
            free(key);
            free(val);
            fclose(fp);
            return false;
        }
    }
    free(key);
    free(val);

    /* Load complete close file and return success */
    fclose(fp);
    return true;
}


bool _list_save(list_store_t *store,char *file)
{
    FILE *fp=NULL;
    int i;
    _entry_t *eptr=NULL; /**< Pointer to entry for lookup/search */
    assert(store);

    pthread_mutex_lock(&store->lock);
    /* Check if store is initialized */
    if (!list_init(store)) return false;
    
    dbg("list: %p, Size: %lu",store->list,store->index);
    
    /* Open file for writting */
    if ((fp=fopen(file,"w"))==NULL) {
        dbg("Failed to open file %s for writting: %s",file,strerror(errno));
        pthread_mutex_unlock(&store->lock);
        return false;
    }

    /* Create header to match on load so that incompatible lists cannot be loaded
     * Header: name hash */
    if (fwrite(&store->id,sizeof(store->id),1,fp)!=1) {
        dbg("Header error for %s: %s",file,strerror(errno));
        fclose(fp); unlink(file);
        pthread_mutex_unlock(&store->lock);
        return false;
    }

    /* Loop through entries and store */
    for (i=0;i<store->index;i++) {
        eptr=((_entry_t *)store->list)+i;
        if ((eptr->key)&&(eptr->val)) {
            if (fwrite(eptr->key,store->key.size,1,fp)!=1) {
                dbg("key write error for %s: %s",file,strerror(errno));
                fclose(fp); unlink(file);
                pthread_mutex_unlock(&store->lock);
                return false;
            }
            if (fwrite(eptr->val,store->value.size,1,fp)!=1) {
                dbg("value write error for %s: %s",file,strerror(errno));
                fclose(fp); unlink(file);
                pthread_mutex_unlock(&store->lock);
                return false;
            }
        } else {
            dbg("List corruption error on index: %d",i);
            fclose(fp); unlink(file);
            pthread_mutex_unlock(&store->lock);
            return false;
        }
    }

    /* Save complete close file and return success */
    fclose(fp);
    pthread_mutex_unlock(&store->lock);
    return true;
}

/**
 * Free entire list
 * @param store pointer to store structure.
 * @return true on success, false on fail
 */
bool _list_free(list_store_t *store)
{
    bool ret=true;
    dbg("list: %p, Size: %lu",store->list,store->index);

    /* Check parameter */
    if (!store) return false;

    if (store->port) {
        repl_close(store);
    }

    pthread_mutex_lock(&store->lock);
    while(store->index) {
        /* Delete from end */
#ifdef LIST_ENTRY_LOCK
        /* Need to switch locks */
        pthread_mutex_unlock(&store->lock);
        if (!_delete_lock(store,store->index-1)) ret=false;
        pthread_mutex_lock(&store->lock);
#endif
        _delete_entry(store,store->index-1);
    }
    dbg("free: %p, Size: %lu",store->list,store->index);
    if (store->list) {
        free(store->list);
        store->list=NULL;
        store->max=0;
        store->index=0;
    }
    pthread_mutex_unlock(&store->lock);
    assert(ret);
    return ret;
}

/** Remove item referenced by index and grab value as one mutexed operation
 * @param keyref Key of item to remove
 * @param index of item, -1 for last item
 * @param value pointer
 * @return True on success, False on failure
 */
bool _list_remove_value(list_store_t *store,int index,void *value)
{
    _entry_t *eptr=NULL; /**< Pointer to entry for lookup/search */
    bool ret=false;

#ifdef LIST_ENTRY_LOCK
    /* Check if we have index. with the two locks, entry and list, we have more
     * to do for cleanup to avoid holding both at the same time. */
    _delete_lock(store,index);
#endif

    /* Grab the lock and enure the entry key is still valid */
    pthread_mutex_lock(&store->lock);
    if ((index<((int)store->index))&&(store->index)) {
        if (index<0) index=_index_wrap(index,store->index);
        eptr=((_entry_t *)store->list)+index;
        /* Save a copy if requested */
        if (value) store->value.cp(value,eptr->val);
        if (store->port) {
            if (eptr->key) repl_remove(store,eptr->key);
        }
        dbgindex(index);
        if (index>=0) {
            _delete_entry(store,index);
            ret=true;
        }
    }
    pthread_mutex_unlock(&store->lock);
    return ret;
}

/** Remove item referenced by keyref
 * @param keyref Key of item to remove
 * @param store pointer to store structure.
 * @return True on success, False on failure
 */
bool _list_remove(list_store_t *store,void *keyref)
{
    bool ret=false;
    int index;

#ifdef LIST_ENTRY_LOCK
    /* Check if we have index. with the two locks, entry and list, we have more
     * to do for cleanup to avoid holding both at the same time. */
    index=_list_index(store,keyref);
    if (index<0) return false;
    _delete_lock(store,index);
#endif

    /* Grab the lock and enure the entry key is still valid */
    pthread_mutex_lock(&store->lock);
    if (store->port) repl_remove(store,keyref);
    index=_find_index(store,keyref);
    dbgindex(index);
    if (index>=0) {
        _delete_entry(store,index);
        ret=true;
    }
    pthread_mutex_unlock(&store->lock);
    return ret;
}

bool _list_lock(list_store_t *store,void *keyref,bool lock)
{
#ifdef LIST_ENTRY_LOCK
    bool ret=false;
    _entry_t *eptr=NULL; /**< Pointer to entry for lookup/search */
    pthread_mutex_lock *lptr=NULL;

    pthread_mutex_lock(&store->lock);
    eptr=_hash_search(store,keyref,false);
    if (eptr) {
        lptr=eptr->lock;
    }
    pthread_mutex_unlock(&store->lock);
    if (lock) {
        /* Ensure the lock is not being deleted, check lock_en */
        if ((lptr)&&(eptr->lock_en)&&(lock)) {
            pthread_mutex_lock(lptr);
            if (eptr->lock_en) {
                ret=true;
            } else {
                /* Interupted by cleanup */
                pthread_mutex_unlock(lptr);
            }
        }
    } else { /* unlock */
        if (lptr) {
            pthread_mutex_unlock(lptr);
            ret=true;
        }
    }
    return ret;
#else
    return true;
#endif
}

/** Local find index, without locks */
int _find_index(list_store_t *store,void *keyref)
{
    int ret=-1;
    _entry_t *eptr=NULL; /**< Pointer to entry for lookup/search */
    assert(store);

    eptr=_hash_search(store,keyref,false);
    //eptr=store->find(store,key,false);
    if (eptr) {
        dbgentry(eptr);
        ret=eptr-((_entry_t *)store->list);
    }
    return ret;
}

/** Delete entry by index */
#ifdef LIST_ENTRY_LOCK
bool _delete_lock(list_store_t *store,int index)
{
    bool ret=false;
    _entry_t *eptr=NULL; /**< Pointer to entry for lookup/search */
    pthread_mutex_lock *lptr=NULL;

    pthread_mutex_lock(&store->lock);
    /* Grab the lock from the entry */
    if ((store->list)&&(store->index)) {
        if (index<0) index=_index_wrap(index.store->index);
        if ((index>=0)&&(index<store->index)) {
            eptr=((_entry_t *)store->list)+index;
            if (eptr) {
                /* Check for two threads cleaning up */
                assert(eptr->lock_en);
                /* Grab the lock and set enable to false
                 * so other threads won't grab it. */
                lptr=eptr->lock;
                eptr->lock_en=false;
            }
        }
    }
    pthread_mutex_unlock(&store->lock);

    if (lptr) {
        /* Get and free entry lock */
        pthread_mutex_lock(lptr);
        assert(eptr->lock==lptr);
        eptr->lock=NULL;
        pthread_mutex_unlock(lptr);
        if (lptr) pthread_mutex_destroy(lptr);
        if (lptr) free(lptr);
        return true;
    }
    return false;
}
#endif

/** Delete entry by index
 * Internal function assumes parameters have been verified
 * by calling function
 * Also assumes lock is held by calling function.
 */
void _delete_entry(list_store_t *store,int index)
{
    _entry_t *eptr=NULL; /**< Pointer to entry for lookup/search */
    /* Ensure the store is initialized and has an entry */
    eptr=((_entry_t *)store->list)+index;
    dbgindex(index);
    if (eptr->key) free(eptr->key);
    if (eptr->val) free(eptr->val);
    memset(eptr,0x00,sizeof(*eptr));
    memmove(eptr,(eptr+1), (store->list+(store->size*store->index))-((void*)eptr));
    store->index-=1;
    return;
}

static inline void *list_init(list_store_t *store)
{
    /* Initialize new or freed list */
    if (!store->list) {
        dbg("Init");
        /* Define initial max, and allocate memory */
        store->max=store->imax;
        if (store->max==0) store->max=30;
        store->index=0;
        store->size=sizeof(_entry_t);
        store->list=calloc(store->size,store->max);
        if (!store->key.print) store->key.print=hash_print;
        if (!store->value.print) store->value.print=hash_print;
        if (store->key.sz) store->key.size=store->key.sz(NULL);
        if (store->value.sz) store->value.size=store->value.sz(NULL);
        /* Generate uniq id from hash configuration */
        store->id=store->key.size + store->key.size;
        store->id=pyHash((uint8_t *)store->key.name,strlen(store->key.name),
            store->id);
        store->id=pyHash((uint8_t *)store->value.name,strlen(store->value.name),
            store->id);
    }
    return store->list;
}

/* Internal function to enlarge storage as needed */
static inline void *list_resize(list_store_t *store)
{
    /* See if we need to allocate more memory */
    if (store->index==store->max) {
        /* Attempt to increase by 25% */
        int increase=store->max/4+1;
        dbg("Resize from %d to %d",store->max,store->max+increase);
        void *newmem=realloc(store->list,store->size * (store->max+increase));
        if (!newmem) {/* Something really wrong, but try adding one */
            dbg("Resize ERROR %d to %d",store->max,store->max+increase);
            increase=1;
            newmem=realloc(store->list,store->size * (store->max+increase));
            if (!newmem) return NULL;
        }
        /* Move pointer to new allocation */
        store->list=newmem;
        store->max+=increase;
    }
    return store->list + (store->index * store->size);
}

/* Standard lib bsearch modified to return insertion location on failed lookup */
static inline void * bfind(const void *key, const void *base, size_t *nmemb, 
        size_t size, __compar_fn_t compar,size_t *slot)
{
    size_t l, u, idx;
    const void *p;
    int comparison;
    assert(slot);

    l = 0;
    u = *nmemb;
    while (l < u) {
      idx = (l + u) / 2;
      p = (void *) (((const char *) base) + (idx * size));
      comparison = (*compar) (key, p);
      if (comparison < 0)
          u = idx;
      else if (comparison > 0)
          l = idx + 1;
      else {
          return (void *) p;
      }
    }

    /* Save insert position for caller's optional use */
    *slot=u;
    return NULL;
}

static char *hash_print(const list_type_info_t *type,const void *val)
{
    static __thread char buf[80];
    char *cptr=buf;
    const uint8_t *vptr=val;

    cptr+=sprintf(cptr,"%s(%lu):",type->name,type->size);
    if (val) {
    }
    
    if ((!val)) sprintf(cptr,"%s:NULL",type->name);
    else if (!strcmp(type->name,"STR")) sprintf(cptr,"%s",((char *)val));
    else if (!strcmp(type->name,"timespec_t")) sprintf(cptr,"%ld:%ld",
            (((timespec_t *)val)->tv_sec),(((timespec_t *)val)->tv_nsec));
    else if (type->size==8) sprintf(cptr,"0x%lx",*((uint64_t *)val));
    else if (type->size==4) sprintf(cptr,"0x%x",*((uint32_t *)val));
    else if (type->size==2) sprintf(cptr,"0x%x",*((uint16_t *)val));
    else if (type->size==1) sprintf(cptr,"0x%x",*((uint8_t *)val));
    else {
        int i,s=type->size;
        if (s>16) s=10;
        sprintf(cptr,"ptr:%p",((void *)val));
        for (i=0; i<s;i++) {
            cptr+=sprintf(cptr,"%02x",vptr[i]);
        }
    }
    return buf;
}

/**
 * @brief Function return a list that matches the version calculated by python
 * @param a pointer to hash source item
 * @param s size of item to hash
 * @param x current sum for accumulating multiple sums
 * Example:
 * \code{.c}
 * propMsg=tlv;
 * h=pyHash(&propMsg,sizeof(propMsg),0);
 * \endcode
 * @return hash value
 */
static uint32_t pyHash(const uint8_t *a,int s,uint32_t x)
{
    register int len=s;
    const register uint8_t *p=a;

    x |= *p << 7;
    while (--len >= 0)
        x = (1000003*x) ^ *p++;
    x ^= s;
    if (x == -1)
        x = -2;
    return ((uint32_t) (x&0xffffffff));
}

/**@}*/

