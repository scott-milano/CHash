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
 * @brief This header file contains the entry structure.  It has been placed
 * in its own header file so it can be used by other has source modules.
 *
 * @addtogroup HASH
 * @{
 */
 
#ifndef __ENTRY_H__
#define __ENTRY_H__

#include<stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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
/* Central Search and insert function */
void *_hash_search(list_store_t *store,void *keyref,void *valref);
int _find_index(list_store_t *store,void *keyref);
void _delete_entry(list_store_t *store,int index);

#ifdef __cplusplus
}
#endif
#endif /* __ENTRY_H__ */
/**@}*/
