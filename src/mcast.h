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
 * @{
 */
 
#ifndef __MCAST_H__
#define __MCAST_H__

#include<stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int mcast_init(uint16_t port);
int mcast_recv(int sock, uint8_t *buf, int size);
int mcast_send(int sock,uint16_t port, uint8_t *buf, int size);

/**@}*/
#ifdef __cplusplus
}
#endif
#endif /* __MCAST_H__ */
