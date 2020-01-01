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
 * This header file provides debug macros for printing hash entries as they
 * are processed.
 * 
 * @{
 */
 
#ifndef __DBG_H__
#define __DBG_H__

#include<stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#ifdef HDEBUG
#else
#endif

#ifdef HDEBUG
extern bool g_debug;
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
#define DBUG_SW(F) g_debug=F
#else
#define dbg(FMT,...)
#define dbgentry(E)
#define dbgindex(I)
#define DIdx(E)
#define DBUG_SW(F)
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

#ifdef __cplusplus
}
#endif
#endif /* __DBG_H__ */
/**@}*/
