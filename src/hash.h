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
 * This module provides a hashed array with binary search access of array items.
 * The Hash is created with a macro call.  This generates static inline access
 * functions.
 * @addtogroup HASH
 * @{
 */

#ifndef __HASH_H__
#define __HASH_H__

#include<stdint.h>
#include<stdbool.h>
#include<pthread.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define HDEBUG
#include "dbg.h"


/* Forward declaration */
struct list_store;
typedef struct list_store list_store_t;

struct list_type_info;
typedef struct list_type_info list_type_info_t;

struct repl_info;
typedef struct repl_info repl_info_t;

/* Function pointer typedefs */
/** Allocation function for key/value of entry */
typedef void* (*_list_alloc_fn_t)(const void*);
/** Access function for copy of value */
typedef void* (*_list_copy_fn_t)(void *,const void*);
/** Returns size of type, for varible sized types (strings) */
typedef size_t (*_list_size_fn_t)(const void*);
/** Debug print function */
typedef char* (*_list_print_fn_t)(const list_type_info_t*, const void*);

/** accessor methods used for generate inlines, not for external use */
void *_list_reference(list_store_t *store,void *keyref);
void *_list_keyref(list_store_t *store,int index);
void *_list_valref(list_store_t *store,int index);
bool _list_copy(list_store_t *store,void *keyref,void *value);
bool _list_items(list_store_t *store,int index,void *key,void *value);
int  _list_index(list_store_t *store,void *keyref);
bool _list_insert(list_store_t *store,void *keyref,void *value);
bool _list_netstart(list_store_t *store, uint16_t port);
bool _list_load(list_store_t *store,char *file);
bool _list_save(list_store_t *store,char *file);
bool _list_free(list_store_t *store);
bool _list_remove(list_store_t *store,void *keyref);
bool _list_remove_value(list_store_t *store,int index,void *value);
bool _list_lock(list_store_t *store,void *keyref,bool lock);
static inline int _index_wrap(int i,size_t m);
/* Replication Thread Function */

/**
 * @addtogroup HASH
 * @{
 */

/** List key/value type information with replacable callbacks for each type
 * This allows the module using this code to implement custom compare or
 * even a deep copy
 */
struct list_type_info {
    const char *name;           /**< Name of type */
    size_t size;                /**< Size of type */
    __compar_fn_t cmp;          /**< Comparison function for sorts/searches */
    _list_alloc_fn_t alloc;     /**< Allocation function */
    _list_copy_fn_t cp;         /**< Access method for custom copy */
    _list_size_fn_t sz;         /**< Access method for type size */
    _list_print_fn_t print;     /**< Access method for custom print */
};

/**
 * @brief Hash storage structure
 * This structure holds all the items needed to store
 * a generic list
 */
struct list_store {
    const char *name;           /**< Name of hash */
    uint32_t id;                /**< Hash Id unique to data types */
    void *list;                 /**< Hash storage pointer */
    int imax;                   /**< Initial max size */
    int max;                    /**< Current max size */
    size_t index;               /**< Index for next new item */
    size_t size;                /**< Size of a complete Key/Value item */
    uint16_t port;              /**< Port for network replication */
    pthread_t nethandle;        /**< Handle for network thread */
    repl_info_t *net;           /**< Information on the network service */
    list_type_info_t key;       /**< Key info and callbacks */
    list_type_info_t value;     /**< Value info and callbacks */
    pthread_mutex_t lock;       /**< Lock for list list access */
};

/** String key value support:
 * Use as "STR" in macro */
typedef char* STR;
#define HASH_MAX_STR 80 /**< Max key length for the @ref DEFINE_HASH_STRKEY key */

/**
 * @brief List generation macro.
 * This is the static version of the List.
 * @param HN Hash name prefix.  Accessor functions begin with this name
 * @param HK Type for list key.  Must be a defined type, and not a pointer to a type.
 * @param HV Type for list value.  Must be a defined type, and not a pointer to a type.
 * Hash starts with 30 items and increases as needed
 * @copydetails HASH
 */
#define DEFINE_LIST(HN,HK,HV) \
    LIST_KEYTYPE(HN,HK) \
    LIST_TYPEFN(HN##_k) \
    LIST_VALTYPE(HN,HV) \
    LIST_TYPEFN(HN##_v) \
    static DECLARE_LIST(HN) \
    static HN##_v HN##_zero; \
    LIST_FUNCTION_GET(HN,&key) \
    LIST_FUNCTION_SET(HN,&key) \
    LIST_FUNCTION_POP(HN) \
    LIST_FUNCTION_NEXT(HN) \
    LIST_FUNCTION_PTR(HN,&key) \
    LIST_FUNCTION_VAL(HN,&key) \
    LIST_FUNCTION_COUNT(HN) \
    LIST_FUNCTION_KEYS(HN,key) \
    LIST_FUNCTION_ITEM(HN) \
    LIST_FUNCTION_INDEX(HN,&key) \
    LIST_FUNCTION_HASKEY(HN,&key) \
    LIST_FUNCTION_DEL(HN,&key) \
    LIST_FUNCTION_LOAD(HN) \
    LIST_FUNCTION_SAVE(HN) \
    LIST_FUNCTION_FREE(HN) \
    LIST_FUNCTION_NETSTART(HN) \
    DECLARE_HANDLER_TYPE(HN) \
    DECLARE_INSTANCE(HN)

/**
 * @brief Hash generation macro
 * This is the static version of the Hash.
 * @param HN Hash name prefix.  Accessor functions begin with this name
 * @param HV Type for list value.  Must be a defined type, and not a pointer to a type.
 * Key type for this list type is a variable length string
 * Hash starts with 30 items and increases as needed
 * @copydetails HASH
 */
#define DEFINE_HASH(HN,HV) \
    LIST_KEYTYPE(HN,STR) \
    HASH_TYPEFN(HN##_k) \
    LIST_VALTYPE(HN,HV) \
    LIST_TYPEFN(HN##_v) \
    static DECLARE_LIST(HN) \
    static HN##_v HN##_zero; \
    LIST_FUNCTION_GET(HN,key) \
    LIST_FUNCTION_SET(HN,key) \
    LIST_FUNCTION_POP(HN) \
    LIST_FUNCTION_NEXT(HN) \
    LIST_FUNCTION_PTR(HN,key) \
    LIST_FUNCTION_VAL(HN,key) \
    LIST_FUNCTION_COUNT(HN) \
    LIST_FUNCTION_KEYS(HN,&key) \
    LIST_FUNCTION_ITEM(HN) \
    LIST_FUNCTION_INDEX(HN,key) \
    LIST_FUNCTION_HASKEY(HN,key) \
    LIST_FUNCTION_DEL(HN,key) \
    LIST_FUNCTION_LOAD(HN) \
    LIST_FUNCTION_SAVE(HN) \
    LIST_FUNCTION_FREE(HN) \
    LIST_FUNCTION_NETSTART(HN) \
    DECLARE_HANDLER_TYPE(HN) \
    DECLARE_INSTANCE(HN)


/** Type for FIFO Key */
typedef struct timespec timespec_t;

/**
 * @brief Fifo generation macro
 * @param HN Hash name prefix.  Accessor functions begin with this name
 * @param HV Type for list value.  Must be a defined type, and not a pointer to a type.
 * Key type for this list type is a integer, provided by the library
 * Fifo starts with 30 items and increases as needed
 * @copydetails HASH
 */
#define DEFINE_FIFO(HN,HV) \
    LIST_KEYTYPE(HN,timespec_t) \
    FIFO_TYPEFN(HN##_k) \
    LIST_VALTYPE(HN,HV) \
    LIST_TYPEFN(HN##_v) \
    static DECLARE_LIST(HN) \
    LIST_FUNCTION_POP(HN) \
    LIST_FUNCTION_NEXT(HN) \
    LIST_FUNCTION_PUSH(HN) \
    LIST_FUNCTION_COUNT(HN) \
    LIST_FUNCTION_ITEM(HN) \
    LIST_FUNCTION_LOAD(HN) \
    LIST_FUNCTION_SAVE(HN) \
    LIST_FUNCTION_FREE(HN) \
    LIST_FUNCTION_NETSTART(HN) \
    DECLARE_FIFO_HANDLER_TYPE(HN) \
    DECLARE_FIFO_INSTANCE(HN)


/**
 * @brief Reference extern List generated in another C module
 * This macro creates a global version of the list/hash that can be accessed throught
 * an application.
 * @param HN Hash name prefix.  Accessor functions begin with this name
 * @param HK Type for list key.  Must be a defined type, and not a pointer to a type.
 * @param HV Type for list value.  Must be a defined type, and not a pointer to a type.
 * Hash starts with 30 items and increases as needed
 * @copydetails HASH
 */
#define EXTERN_LIST(HN,HK,HV) \
    LIST_KEYTYPE(HN,HK) \
    LIST_TYPEFN(HN##_k) \
    LIST_VALTYPE(HN,HV) \
    LIST_TYPEFN(HN##_v) \
    extern list_store_t HN##_store; \
    static HN##_v HN##_zero; \
    LIST_FUNCTION_GET(HN,&key) \
    LIST_FUNCTION_SET(HN,&key) \
    LIST_FUNCTION_POP(HN) \
    LIST_FUNCTION_NEXT(HN) \
    LIST_FUNCTION_PTR(HN,&key) \
    LIST_FUNCTION_VAL(HN,&key) \
    LIST_FUNCTION_COUNT(HN) \
    LIST_FUNCTION_KEYS(HN,key) \
    LIST_FUNCTION_ITEM(HN) \
    LIST_FUNCTION_INDEX(HN,&key) \
    LIST_FUNCTION_HASKEY(HN,&key) \
    LIST_FUNCTION_DEL(HN,&key) \
    LIST_FUNCTION_LOAD(HN) \
    LIST_FUNCTION_SAVE(HN) \
    LIST_FUNCTION_FREE(HN) \
    LIST_FUNCTION_NETSTART(HN) \
    DECLARE_HANDLER_TYPE(HN) \
    DECLARE_INSTANCE(HN)

/**
 * @brief Referenc extern Hash generated in another C source file
 * @param HN Hash name prefix.  Accessor functions begin with this name
 * @param HV Type for list value.  Must be a defined type, and not a pointer to a type.
 * Key type for this list type is a variable length string
 * Hash starts with 30 items and increases as needed
 * @copydetails HASH
 */
#define EXTERN_HASH(HN,HV) \
    LIST_KEYTYPE(HN,STR) \
    HASH_TYPEFN(HN##_k) \
    LIST_VALTYPE(HN,HV) \
    LIST_TYPEFN(HN##_v) \
    extern list_store_t HN##_store; \
    static HN##_v HN##_zero; \
    LIST_FUNCTION_GET(HN,key) \
    LIST_FUNCTION_SET(HN,key) \
    LIST_FUNCTION_POP(HN) \
    LIST_FUNCTION_NEXT(HN) \
    LIST_FUNCTION_PTR(HN,key) \
    LIST_FUNCTION_VAL(HN,key) \
    LIST_FUNCTION_COUNT(HN) \
    LIST_FUNCTION_ITEM(HN) \
    LIST_FUNCTION_INDEX(HN,key) \
    LIST_FUNCTION_HASKEY(HN,key) \
    LIST_FUNCTION_DEL(HN,key) \
    LIST_FUNCTION_LOAD(HN) \
    LIST_FUNCTION_SAVE(HN) \
    LIST_FUNCTION_FREE(HN) \
    LIST_FUNCTION_KEYS(HN,&key) \
    LIST_FUNCTION_NETSTART(HN) \
    DECLARE_HANDLER_TYPE(HN) \
    DECLARE_INSTANCE(HN)

/**
 * @brief Fifo generation macro
 * @param HN Hash name prefix.  Accessor functions begin with this name
 * @param HV Type for list value.  Must be a defined type, and not a pointer to a type.
 * Key type for this list type is a integer, provided by the library
 * Fifo starts with 30 items and increases as needed
 * @copydetails HASH
 */
#define EXTERN_FIFO(HN,HV) \
    LIST_KEYTYPE(HN,timespec_t) \
    FIFO_TYPEFN(HN##_k) \
    LIST_VALTYPE(HN,HV) \
    LIST_TYPEFN(HN##_v) \
    extern list_store_t HN##_store; \
    LIST_FUNCTION_POP(HN) \
    LIST_FUNCTION_NEXT(HN) \
    LIST_FUNCTION_PUSH(HN) \
    LIST_FUNCTION_COUNT(HN) \
    LIST_FUNCTION_ITEM(HN) \
    LIST_FUNCTION_LOAD(HN) \
    LIST_FUNCTION_SAVE(HN) \
    LIST_FUNCTION_FREE(HN) \
    LIST_FUNCTION_NETSTART(HN) \
    DECLARE_FIFO_HANDLER_TYPE(HN) \
    DECLARE_FIFO_INSTANCE(HN)

/**
 * @brief List and Hash storage structure creation macro
 * Creates the list storage data structure for list/hash access. 
 * Can be used to create a global version of the hash structure
 * @param HN Hash name prefix.  Accessor functions begin with this name
 * Key type for this list type is a variable length string
 * Hash starts with 30 items and increases as needed
 * @copydetails HASH
 */
#define DECLARE_LIST(HN) \
    list_store_t HN##_store={.name=#HN,.lock=PTHREAD_MUTEX_INITIALIZER,\
        .key=LIST_TYPEINFO(HN##_k),.value=LIST_TYPEINFO(HN##_v), \
};
/** Internal macro used by DECLARE_LIST */
#define LIST_TYPEINFO(KT) {.name=KT##_name,.size=sizeof(KT), \
    .cmp=KT##_cmp, .alloc=KT##_alloc, .cp=KT##_cp, .sz=KT##_sz,\
}

/** Internal macro used by DEFINE_LIST/DEFINE_HASH */
#define DECLARE_HANDLER_TYPE(HN) \
    typedef struct { \
        bool (*get)(HN##_k,HN##_v*); \
        bool (*set)(HN##_k,HN##_v); \
        bool (*pop)(HN##_v*); \
        bool (*next)(HN##_v*); \
        HN##_v* (*addr)(HN##_k); \
        HN##_v (*val)(HN##_k); \
        int (*count)(void); \
        HN##_k (*key)(int); \
        bool (*item)(int,HN##_v*); \
        int (*index)(HN##_k); \
        bool (*hasKey)(HN##_k); \
        bool (*del)(HN##_k); \
        bool (*load)(char*); \
        bool (*save)(char*); \
        bool (*free)(void); \
    } HN##_handler_t;

/** An instance of the LIST/HASH that includes methods for accessing data */
#define DECLARE_INSTANCE(HN) \
    HN##_handler_t HN={ \
        .get=HN##Get, \
        .set=HN##Set, \
        .pop=HN##Pop, \
        .next=HN##Next, \
        .addr=HN##Ptr, \
        .val=HN##Val, \
        .count=HN##Count, \
        .key=HN##Keys, \
        .item=HN##Item, \
        .index=HN##Index, \
        .hasKey=HN##HasKey, \
        .del=HN##Del, \
        .load=HN##Load, \
        .save=HN##Save, \
        .free=HN##Free, \
    };

/** Internal macro used by DEFINE_LIST/DEFINE_HASH */
#define DECLARE_FIFO_HANDLER_TYPE(HN) \
    typedef struct { \
        bool (*pop)(HN##_v*); \
        bool (*next)(HN##_v*); \
        bool (*push)(HN##_v); \
        int (*count)(void); \
        bool (*item)(int,HN##_v*); \
        bool (*load)(char*); \
        bool (*save)(char*); \
        bool (*free)(void); \
    } HN##_handler_t;

/** An instance of the LIST/HASH that includes methods for accessing data */
#define DECLARE_FIFO_INSTANCE(HN) \
    HN##_handler_t HN={ \
        .pop=HN##Pop, \
        .next=HN##Next, \
        .push=HN##Push, \
        .count=HN##Count, \
        .item=HN##Item, \
        .load=HN##Load, \
        .save=HN##Save, \
        .free=HN##Free, \
    };


/**
 * Macro to generate handler functions for key.
 * @param HN List name
 * @param KT key type
 */
#define LIST_KEYTYPE(HN,KT) \
    typedef KT HN##_k; \
    const char HN##_k_name[]=#KT; \

/**
 * Macro to generate handler functions for value.
 * @param HN List name
 * @param KT key type
 */
#define LIST_VALTYPE(HN,KT) \
    typedef KT HN##_v; \
    const char HN##_v_name[]=#KT; \

/**
 * Default callbacks for entries
 * @param HN List name
 * @param KT key/val type
 * - cmp with memcmp
 * - cp with memcpy
 * - sz with constant
 * - allocate with calloc and copy to dst with memcpy
 */
#define LIST_TYPEFN(KT) \
    static int KT##_cmp(const void *m1, const void *m2) \
    { \
        return memcmp(*((KT **)m1),*((KT **)m2),sizeof(KT)); \
    } \
    static void *KT##_cp(void *dst, const void *src) \
    { \
        return memcpy(dst,src,sizeof(KT));\
    } \
    static size_t KT##_sz(const void *src) \
    { \
        return sizeof(KT);\
    } \
    static void *KT##_alloc(const void *src) \
    { \
        void *dst=calloc(1,sizeof(KT)); \
        if (dst) return memcpy(dst,src,sizeof(KT));\
        else return NULL; \
    }

/**
 * Macro to generate hash functions for key.
 * @param HN List name
 * @param KT key type, (Type is always STR or char*)
 * Default callbacks for entries
 * - cmp with strcmp
 * - cp with strcpy
 * - allocate variable size of strlen+1
 */
#define HASH_TYPEFN(KT) \
    static int KT##_cmp(const void *m1, const void *m2) \
    {\
        return strncmp(*((char **)m1), *((char **)m2), HASH_MAX_STR);\
    }\
    static void *KT##_cp(void *dst, const void *src) \
    {\
        size_t sz=strnlen((char*)src,HASH_MAX_STR);\
        assert(sz<HASH_MAX_STR);\
        return strncpy(((char *)dst), ((char *)src), sz+1);\
    }\
    static size_t KT##_sz(const void *src) \
    {   /* Call with NULL to get max possible size */ \
        if (src) return strnlen((char*)src,HASH_MAX_STR)+1;\
        else return HASH_MAX_STR;\
    } \
    static void *KT##_alloc(const void *src) \
    {\
        size_t sz=strnlen((char*)src,HASH_MAX_STR);\
        assert(sz<HASH_MAX_STR);\
        void *dst=malloc(sz+1); \
        if (dst) return strncpy(dst,src, sz+1);\
        else return NULL; \
    }

#define FIFO_TYPEFN(KT) \
    static int KT##_cmp(const void *m1, const void *m2) \
    { \
        timespec_t *a=*(timespec_t **)m1; \
        timespec_t *b=*(timespec_t **)m2; \
        if (a->tv_sec == b->tv_sec) return a->tv_nsec - b->tv_nsec; \
        else return a->tv_sec - b->tv_sec; \
    } \
    static void *KT##_cp(void *dst, const void *src) \
    { \
        return memcpy(dst,src,sizeof(KT));\
    } \
    static size_t KT##_sz(const void *src) \
    { \
        return sizeof(KT);\
    } \
    static void *KT##_alloc(const void *src) \
    { \
        void *dst=calloc(1,sizeof(KT)); \
        if (dst) return memcpy(dst,src,sizeof(KT));\
        else return NULL; \
    }


/* Access methods */
/**
 * @par ListGet static inline bool LNameGet(List_k key,List_v *value);
 * ListGet Get the value of existing list entry
 * @param key List entry key value
 * @param value reference to value for return
 * @return true on success, false on failure
 * Get the value referenced by the key:
 * \code{.c}
 * bool ListGet(int key, in_addr_t *value);
 * \endcode
 */
#define LIST_FUNCTION_GET(HN,KEY) \
    static inline bool HN##Get(HN##_k key,HN##_v *value) \
    { \
        return _list_copy(&HN##_store, KEY, value); \
    }

/**
 * @par ListSet static inline bool LNameSet(LKeyType key,LValType value)
 * @param key List entry key value
 * @param value to create of update
 * @return true on success, false on failure
 * Add or Set the value of existing list entry
 * Set the value currently referenced, or add new value
 * \code{.c}
 * void ListSet(int key, in_addr_t value);
 * \endcode
 * 
 */
#define LIST_FUNCTION_SET(HN,KEY) \
    static inline bool HN##Set(HN##_k key,HN##_v value) \
    { \
        return _list_insert(&HN##_store, KEY, &value); \
    }

/**
 * @par ListPop static inline bool LNamePop(List_v *value);
 * ListPop Pull the value from the end of the list and delete it
 * @param value reference to value for return
 * @return true on success, false on failure
 * Remove and return Last Entry from list
 * \code{.c}
 * bool ListPop(in_addr_t *value);
 * \endcode
 */
#define LIST_FUNCTION_POP(HN) \
    static inline bool HN##Pop(HN##_v *value) \
    { \
        return _list_remove_value(&HN##_store,-1,value); \
    }

/**
 * @par ListNext static inline bool LNameNext(List_v *value);
 * ListNext Pull the first value from the front of the list and deletes it
 * @param value reference to value for return
 * @return true on success, false on failure
 * Remove and return First Entry from list
 * \code{.c}
 * bool ListNext(in_addr_t *value);
 * \endcode
 */
#define LIST_FUNCTION_NEXT(HN) \
    static inline bool HN##Next(HN##_v *value) \
    { \
        return _list_remove_value(&HN##_store,0,value); \
    }

/**
 * @par ListPush static inline bool LNamePush(LKeyType key,LValType value)
 * @param value to create of update
 * @return true on success, false on failure
 * Push a new value onto end of list
 * Set the value currently referenced, or add new value
 * \code{.c}
 * void ListPush(in_addr_t value);
 * \endcode
 *
 */
#define LIST_FUNCTION_PUSH(HN) \
    static inline bool HN##Push(HN##_v value) \
    { \
        HN##_k key; \
        clock_gettime(CLOCK_REALTIME,&key); \
        return _list_insert(&HN##_store, &key, &value); \
    }


/**
 * @par ListPtr static inline LValType *LNamePtr(LKeyType key)
 * Return the pointer to a List entry
 * Get a pointer to the value.  Pointer may be broken at next list sort.  This
 * occurs when a value is added or removed.  Pointer is null if item not 
 * available:
 * \code{.c}
 * in_addr_t *ListPtr(int key);
 * \endcode
 * 
 */
#define LIST_FUNCTION_PTR(HN,KEY) \
    static inline HN##_v *HN##Ptr(HN##_k key) \
    { \
        return _list_reference(&HN##_store, KEY); \
    }

/**
 * @par ListVal static inline LValType LNameVal(LKeyType key)
 * @param key Key value for list return
 * @return the value of List entry
 * @returns the value as return value.  If value unavailable then a "0" version
 * is returned from List_zero.  This "zero" is a variable that can be changed:
 * \code{.c}
 * in_addr_t ListVal(int key);
 * \endcode
 */
#define LIST_FUNCTION_VAL(HN,KEY) \
    static inline HN##_v HN##Val(HN##_k key) \
    { \
        HN##_v ret=HN##_zero; \
        _list_copy(&HN##_store, KEY, &ret);\
        return ret; \
    } \

/**
 * @par ListCount static inline int LNameCount(void)
 * Get Number of Items in List
 * @parblock
 * @brief Return size of hash list
 * Size of list.
 * \code{.c}
 * int ListCount();
 * \endcode
 * @return Number of items in list
 * @endparblock
 *
 */
#define LIST_FUNCTION_COUNT(HN) \
    static inline int HN##Count(void) \
    { \
        return HN##_store.index; \
    }

/**
 * @par ListKeys static inline LKeyType LNameKeys(int i)
 * Returns the key as return value from the sorted list position.  If value
 * unavailable then the index is wraped on the list. ListKeys(-1) returns 
 * the last key.
 * is a variable that can be changed:
 * \code{.c}
 * int ListKeys(int i);
 * \endcode
 */
#define LIST_FUNCTION_KEYS(HN,KEY) \
    static inline HN##_k HN##Keys(int i) \
    {  \
        HN##_k kval; \
        HN##_k *key=_list_keyref(&HN##_store,i);\
        if (!key) { \
            /* Index is out of range, wrap it */ \
            i=_index_wrap(i,HN##Count()); \
            if (i<0) { \
                /* List is empty just memset a zero */ \
                memset(&kval,0x00,sizeof(kval)); \
                return kval; \
            } \
            /* Read again with corrected index */ \
            key=_list_keyref(&HN##_store,i);\
        } \
        memcpy(&kval,KEY,sizeof(kval)); \
        return kval; \
    }

/**
 * @par ListItem static inline bool LNameItem(int i,LValType *val)
 * @brief Retrieve item at index i
 * Allows for grabbing wrapped indexes.  For example ListItem(-1,&value);
 * returns the last item in the list.  If list is empty, value is not updated.
 * For wrapped values, or empty list, false is returned.  An in range index
 * returns true.
 * \code{.c}
 * int val;
 * ListSet(List,0);
 * ListSet(List,1);
 * ListSet(List,2);
 * ListSet(List,3);
 *
 * ListItem(2,&val);  // Returns true and sets val to 2
 * ListItem(4,&val);  // Returns false and sets val to 0
 * ListItem(-1,&val); // Returns false and sets val to 3
 * ListFree();
 * ListItem(2,&val);  // Returns false and does not set val
 * \endcode
 * @param(in) i Index of item
 * @param(out) val Pointer for list item return
 * @return true for in range list item placed in val
 * @return false if list is empty or the returned item is wrapped
 *
 */
#define LIST_FUNCTION_ITEM(HN) \
    static inline bool HN##Item(int i,HN##_v *value) \
    { \
        HN##_v *vptr=_list_valref(&HN##_store,i); \
        if (vptr) { \
            memcpy(value,vptr,sizeof(*value)); \
            return true; \
        } else { \
            i=_index_wrap(i,HN##Count()); \
            if (i>=0) { \
                vptr=_list_valref(&HN##_store,i); \
                memcpy(value,vptr,sizeof(*value)); \
            } \
            return false; \
        } \
    } \

/**
 * @par ListIndex static inline int LNameIndex(LKeyType key)
 * int ListIndex(ListKey key)
 * Get index of List Key
 * Return the index of the key parameter
 * @param key List item key
 * @return index of list position
 * @return -1 if item is not in list
 * \code{.c}
 * i=ListIndex(int i);
 * \endcode
 */
#define LIST_FUNCTION_INDEX(HN,KEY) \
    static inline int HN##Index(HN##_k key) \
    { \
        return _list_index(&HN##_store,KEY); \
    }

/**
 * @par ListHasKey static inline bool LNameHasKey(LKeyType key)
 * @brief Bool indidicating if the key is in list.
 * @param key List item key
 * @return true if item is in list
 * @return false if item is not in list
 * \code{.c}
 * bool ListHasKey(int key);
 * \endcode
 * @param key key to check
 * @return true if the item is in the list false otherwise
 *
 */
#define LIST_FUNCTION_HASKEY(HN,KEY) \
    static inline bool HN##HasKey(HN##_k key) \
    { \
        if (_list_index(&HN##_store,KEY)>=0) return true; \
        else return false; \
    }

/**
 * @par ListDel static inline bool LNameDel(LKeyType key)
 * Delete list entry at key
 * @param key List item key
 * @return true if item was in list and has been deleted
 * @return false if item is not in list
 * \code{.c}
 * bool ListDel(int key);
 * \endcode
 * 
 */
#define LIST_FUNCTION_DEL(HN,KEY) \
    static inline bool HN##Del(HN##_k key) \
    { \
        return _list_remove(&HN##_store,KEY); \
    }

/**
 * @par ListLock static inline bool LNameLock(LKeyType key)\n
 * static inline bool LNameUnLock(LKeyType key)
 * Set the mutex lock of a single list entry.  Enabled with @ref LIST_ENTRY_LOCK
 */
#define LIST_FUNCTION_LOCK(HN,KEY) \
    static inline bool HN##Lock(HN##_k key) \
    { \
        _list_lock(&HN##_store,KEY,false); \
    }\
    static inline bool HN##Unlock(HN##_k key) \
    { \
        _list_lock(&HN##_store,KEY,false); \
    }

/**
 * @par ListNetStart static inline bool LNameNetStart(uint16_t port)
 * Sets the port number for multicast packets and starts the sharing
 * thread.  Thread is closed when free is called.
 * @param port Port for network sharing.
 * @return true on successful start, false on failure or if already
 * running.
 * \code{.c}
 * ListNetStart(6500);
 * \endcode
 * 
 */
#define LIST_FUNCTION_NETSTART(HN) \
    static inline bool HN##NetStart(uint16_t port) \
    { \
        return _list_netstart(&HN##_store,port); \
    }\

/**
 * @par ListLoad static inline bool LNameLoad(char *file)
 * Load data from file into hash.
 * @return true if list has been loaded without issue
 * @return false list load has failed
 * \code{.c}
 * void ListLoad("/var/data/datafile.hash");
 * \endcode
 * 
 */
#define LIST_FUNCTION_LOAD(HN) \
    static inline bool HN##Load(char *file) \
    { \
        return _list_load(&HN##_store,file);\
    }

/**
 * @par ListSave static inline bool LNameSave(char *file)
 * Save data from file into hash.
 * @return true if list has been loaded without issue
 * @return false list load has failed
 * \code{.c}
 * void ListSave("/var/data/datafile.hash");
 * \endcode
 * 
 */
#define LIST_FUNCTION_SAVE(HN) \
    static inline bool HN##Save(char *file) \
    { \
        return _list_save(&HN##_store,file);\
    }

/**
 * @par ListFree static inline bool LNameFree(void)
 * Free entire list, and reset to empty working list.  All allocated memory
 * is freed.  On first use following ListFree, the initial count is allocated.
 * @return true if list has been freed without issue
 * @return false list was freed but some error occured
 * \code{.c}
 * void ListFree();
 * \endcode
 * 
 */
#define LIST_FUNCTION_FREE(HN) \
    static inline bool HN##Free(void) \
    { \
        return _list_free(&HN##_store);\
    }


/* Need extra macro layer for MKI(__LINE__) to work */
#define MKIPRE(i) I##i
#define MKI(i) MKIPRE(i)

/**
 * @par HASH_FOREACH(List1,item);
 * Loop through each entry in order.
 * The count is started at the end and subtracted from the end on each entry to
 * suppor deleted entries durring the foreach
 * @param HN name of List/Hash 
 * @param VAR variable for receiving entries
 * @return Each entry is returned in list order, sorted by key
 * \code{.c}
 *  entry_t entry;
 *  HASH_FOREACH(Test2,entry) {
 *      proccess_entry(entry);
 *  }
 * \endcode
 */
#define HASH_FOREACH(HN,VAR) \
int MKI(__LINE__)=HN##Count(); \
while (HN##Item((HN##Count()-MKI(__LINE__)--),&VAR))

/**
 * @par HASH_FOREACH_ADDR(List1,*item);
 * Loop through each entry in order.
 * The count is started at the end and subtracted from the end on each entry to
 * suppor deleted entries durring the foreach
 * @param HN name of List/Hash 
 * @param VAR variable for receiving entries
 * @return Each entry is returned in list order, sorted by key
 * \code{.c}
 *  entry_t *eptr;
 *  HASH_FOREACH_ADDR(Test2,eptr) {
 *      proccess_entry(*eptr);
 *  }
 * \endcode
 */
#define HASH_FOREACH_ADDR(HN,VAR) \
int MKI(__LINE__)=HN##Count(); \
while ((VAR=(HN##_v*)_list_valref(&HN##_store,(HN##Count()-MKI(__LINE__)--))))

#define HASH_FOREACH_KEY(HN,VAR) \
int MKI(__LINE__)=HN##Count(); \
while ((VAR=(HN##_k*)_list_keyref(&HN##_store,(HN##Count()-MKI(__LINE__)--))))

#define DEFINE_LIST_ITERATOR(HN,HT,HF) \
    static inline HN##_v *HN##_iter_r_##HF(int *index, HT search) \
    { \
        HN##_v *vptr=NULL; \
        while ((vptr=_list_valref(&HN##_store,*index))) { \
            *index+=1; \
            if (vptr->HF==search) return vptr; \
        } \
        *index=0; \
        return NULL; \
    } \
    static inline HN##_v *HN##_iter_##HF(HT search) \
    { \
        static int __thread index=0; \
        HN##_v *value=NULL;\
        value=HN##_iter_r_##HF(&index, search); \
        return value;\
    } \
    static inline bool HN##_next_##HF(HN##_v *value,HT search) \
    { \
        static int __thread index=0; \
        if (value) { \
            HN##_v *next=HN##_iter_r_##HF(&index, search); \
            if (next) { \
                *value=*next; \
                return true; \
            } \
        } \
        return false; \
    }

/* To support index wrap and allow ListVal(-1); to return last entry in list
 * wrap out of range indexes
 * @param i index to wrap if out of range of list size m
 * @param m size of List
 * @return index guarenteed to be in range if m!=0 
 * @return if m==0, return -1
 */
static inline int _index_wrap(int i,size_t m)
{
    if (i<0) {
        if (m==0) return -1;
        /* Mod of negative number in C are negative */
        i=(i%(int)m)+m;
        i=(i%(int)m);
    } else if (i>=m) {
        if (m==0) return -1;
        i=(i%(int)m);
    }
    return i;
}

/**@}*/
#ifdef __cplusplus
}
#endif
#endif /* __HASH_H__ */
