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
 
#ifdef UNIT_TEST
#ifndef __TEST_H__
#define __TEST_H__

#include "hash.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Custom test types for hashes, implemented for basic tests, can be set in CC line */
#ifndef TK1
#define TK1 int
#endif
#define TK1_0 ((TK1)0)
#ifndef TK2
#define TK2 STR
#endif
#define TK2_0 ((TK2)0)
#ifndef TV1
#define TV1 int
#endif
#define TV1_0 ((TV1)0)
#ifndef TV2
#define TV2 TV1
#endif
#define TV2_0 ((TV2)0)

/* Test data structure */
typedef struct {int ifield; bool bfield; float ffield;} test_fields_t;

EXTERN_LIST(Test1,TK1,TV1);
EXTERN_HASH(Test4,test_fields_t);
EXTERN_FIFO(Test6,TV2);

/**@}*/
#ifdef __cplusplus
}
#endif
#endif /* __TEST_H__ */
#endif /* UNIT_TEST */
