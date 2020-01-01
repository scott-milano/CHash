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
 * Header file for mcast replication of the Hash
 *
 * @addtogroup HASH
 * @{
 */
 
#ifndef __REPLICATOR_H__
#define __REPLICATOR_H__

#include<stdint.h>
#include "entry.h"

#ifdef __cplusplus
extern "C" {
#endif

bool repl_start(list_store_t *store);
bool repl_update(list_store_t *store,_entry_t *eptr);
bool repl_remove(list_store_t *store,void *keyref);
void repl_close(list_store_t *store);

#ifdef __cplusplus
}
#endif
#endif /* __REPLICATOR_H__ */
/**@}*/
