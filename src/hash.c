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
 * @copydetails LIST_FUNCTION_FREE
 * <hr>
 * @copydetails LIST_FUNCTION_LOCK
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

#include "mcast.h"
#include "hash.h"

/**@}*/
#ifdef HDEBUG
/** @if internal */
bool g_debug=false;  /* Global debug flag */
#define dbg(FMT,...) do { \
    if (g_debug)\
        fprintf(stderr,"%s:%d:%s " FMT "\n",__func__,__LINE__,\
        store->name,##__VA_ARGS__);fflush(stderr); \
} while (false)
#define dbgentry(E) do { \
    if (g_debug) {\
        fprintf(stderr,"%s:%d: Store(%s)=%lu   Entry(%lu) %s == ",__func__,__LINE__, \
                store->name,store->index-1,DIdx(E),store->key.print(&store->key,E->key));\
        fprintf(stderr,"%s\n",store->value.print(&store->value,E->val));\
        fflush(stderr); \
    } \
} while (false)
#define dbgindex(I) do { \
    if (g_debug) { \
        if ((I<0)||(I>=store->index)) { \
            dbg("WARNING: Index out of range:%d",I); \
        } else { \
            _entry_t *dieptr=((_entry_t *)store->list)+I; \
            dbgentry(dieptr); \
        }  \
    } \
} while (false)
#define DIdx(E) (((_entry_t*)E)-((_entry_t*)store->list))
#else
#define dbg(FMT,...)
#define dbgentry(E)
#define dbgindex(I)
#define DIdx(E)
#endif

/* Entry Pointer to Index */
#define EIdx(E) (((_entry_t*)E)-((_entry_t*)store->list))
/* Entry Index as void Ptr */
#define VEPtr(I) ((void*) ((I*store->size)+(store->list)))
/* Entry Index as entry Ptr */
#define EPtr(I) (((_entry_t *)store->list)+I)
#define STORE(I) (((_entry_t *)store->list[I])
#define VALUE(I) (STORE[I]).val
#define KEY(I) (STORE[I]).key
/** @endif */

/** @addtogroup HASH @{ */
/** List Storage struct */
typedef struct {
    void *key;      /**< Pointer to Key */
    void *val;      /**< Pointer to Value */
#ifdef LIST_ENTRY_LOCK
    pthread_mutex_t *lock;  /**< Mutex for individual entry */
    bool lock_en;           /**< Flag to indicate flag is not being deleted */
#endif
} _entry_t;

/* Utility Functions for managing list */
/* Init: */
static inline void *list_init(list_store_t *store);
/* Resize: */
static inline void *list_resize(list_store_t *store);
/* Central Search and insert function */
static void *hash_search(list_store_t *store,void *keyref,void *valref);
/* Bsearch variation that returns existing value or new insert slot */
static inline void * bfind(const void *key, const void *base, size_t *nmemb, 
        size_t size, __compar_fn_t compar,size_t *slot);

/* Find entry by index. Assumes list lock is held */
static int find_index(list_store_t *store,void *keyref);
static char *hash_print(const list_type_info_t *type,const void *val);

/* Delete Entry by index */
static void delete_entry(list_store_t *store,int index);
#ifdef LIST_ENTRY_LOCK
/* Delete lock by index */
static bool delete_lock(list_store_t *store,int index);
#endif

/* Mcast shared operations */
#define OP_NOP 0
#define OP_SET 1
#define OP_DEL 2
#define OP_SYNC 3

void *store_replication(void *vstore);
bool store_update(list_store_t *store,_entry_t *eptr);
bool store_remove(list_store_t *store,void *keyref);

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
static void *hash_search(list_store_t *store,void *keyref,void *valref)
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
        eptr=lfind(&entry, store->list, &store->index,store->size,store->key.cmp);
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
    eptr=hash_search(store,keyref,valref);
    if (eptr) {
        dbgentry(eptr);
        ret=true;
        if (store->port) store_update(store,eptr);
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
    eptr=hash_search(store,keyref,false);
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
    eptr=hash_search(store,keyref,false);
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
    index=find_index(store,keyref);
    dbgindex(index);
    pthread_mutex_unlock(&store->lock);
    return index;
}

bool _list_free(list_store_t *store)
{
    bool ret=true;
    dbg("list: %p, Size: %lu",store->list,store->index);

    /* Check parameter */
    if (!store) return false;

    if ((store->port)&&(store->index)) {
        uint16_t port=store->port;
        char c=OP_NOP;
        store->port=0;
        /* Wake up blocked receiver */
        mcast_send(store->sock,port,(uint8_t*)&c,1);
    }

    pthread_mutex_lock(&store->lock);
    while(store->index) {
        /* Delete from end */
#ifdef LIST_ENTRY_LOCK
        /* Need to switch locks */
        pthread_mutex_unlock(&store->lock);
        if (!delete_lock(store,store->index-1)) ret=false;
        pthread_mutex_lock(&store->lock);
#endif
        delete_entry(store,store->index-1);
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

bool _list_remove(list_store_t *store,void *keyref)
{
    bool ret=false;
    int index;

#ifdef LIST_ENTRY_LOCK
    /* Check if we have index. with the two locks, entry and list, we have more
     * to do for cleanup to avoid holding both at the same time. */
    index=_list_index(store,keyref);
    if (index<0) return false;
    delete_lock(store,index);
#endif

    /* Grab the lock and enure the entry key is still valid */
    pthread_mutex_lock(&store->lock);
    if (store->port) store_remove(store,keyref);
    index=find_index(store,keyref);
    dbgindex(index);
    if (index>=0) {
        delete_entry(store,index);
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
    eptr=hash_search(store,keyref,false);
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

#define OP_NOP 0
#define OP_SET 1
#define OP_DEL 2
#define OP_SYNC 3
typedef struct {
    uint8_t op;
    uint8_t buf[];
} entry_packet_t;

void *store_replication(void *vstore)
{
    list_store_t *store=(list_store_t *)vstore;
    _entry_t *eptr=NULL;
    int bytes;
    int size=1+store->key.size+store->value.size;
    uint8_t *buf,*op;
    void *key,*value;
    dbg("Replication Started");

    store->sock=mcast_init(store->port);
    dbg("Init Returned: %d",store->sock);
    if (store->sock<=0) return NULL;

    store->rcv=malloc(size);
    store->snd=malloc(size);
    buf=store->rcv;
    op=buf;
    key=buf+1;
    value=key+store->key.size;
    while (store->port) {
        bytes=mcast_recv(store->sock,buf,size);
        if (bytes>store->key.size) {
            switch (*op) {
                case OP_SET:
                    if (memcmp(store->snd+1,store->rcv+1,size-1)!=0)  {
                        eptr=hash_search(store,key,value);
                        if (eptr) dbgentry(eptr);
                        else dbg("Insert Failure");
                    }
                    break;
                case OP_DEL:
                    _list_remove(store,key);
                    break;
                case OP_NOP:
                    dbg("nop: %d %d, %d",bytes,size,store->sock);
                    break;
                default:
                    dbg("unknown: %u",*op);
                    break;
            }
        } else if (bytes==0) {
            dbg("Read not blocking");
            sleep(1);
        } else if (bytes>0) {
            printf("Undersized Packet: %d",bytes);
        } else {
            dbg("Error");
            sleep(1);
        }
    }
    if (store->rcv) free(store->rcv);
    store->rcv=NULL;
    if (store->snd) free(store->snd);
    store->snd=NULL;
    close(store->sock);
    store->sock=0;
    return NULL;
}

bool store_update(list_store_t *store,_entry_t *eptr)
{
    int size=1+store->key.size+store->value.size;
    int bytes;

    /* Ensure buffer is allocated */
    if (store->snd) {
        void *key,*value;
        key=store->snd+1;
        value=key+store->key.size;
        store->snd[0]=OP_SET;
        dbgentry(eptr);
        if (eptr->key) store->key.cp(key,eptr->key);
        if (eptr->val) store->value.cp(value,eptr->val);
        bytes=mcast_send(store->sock,store->port,store->snd,size);
        if (bytes==size) return true;
    }
    return false;
}

bool store_remove(list_store_t *store,void *keyref)
{
    int size=1+store->key.size;
    int bytes;

    /* Ensure buffer is allocated */
    if (store->snd) {
        void *key;
        key=store->snd+1;
        store->snd[0]=OP_DEL;
        store->key.cp(key,keyref);
        if (keyref) store->key.cp(key,keyref);
        bytes=mcast_send(store->sock,store->port,store->snd,size);
        if (bytes==size) return true;
    }
    return false;
}
/** Local find index, without locks */
static int find_index(list_store_t *store,void *keyref)
{
    int ret=-1;
    _entry_t *eptr=NULL; /**< Pointer to entry for lookup/search */
    assert(store);

    eptr=hash_search(store,keyref,false);
    //eptr=store->find(store,key,false);
    if (eptr) {
        dbgentry(eptr);
        ret=eptr-((_entry_t *)store->list);
    }
    return ret;
}

/** Delete entry by index */
#ifdef LIST_ENTRY_LOCK
static bool delete_lock(list_store_t *store,int index)
{
    bool ret=false;
    _entry_t *eptr=NULL; /**< Pointer to entry for lookup/search */
    pthread_mutex_lock *lptr=NULL;

    pthread_mutex_lock(&store->lock);
    /* Grab the lock from the entry */
    if ((store->list)&&(store->index)) {
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
static void delete_entry(list_store_t *store,int index)
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

/**@}*/
