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
 * @brief Unit Test for HASH Module
 */

#ifdef UNIT_TEST
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "hash.h"
#include "test.h"

#define mu_assert(message, test) do { if (!(test)) {printf("Fail %s:%d ",__func__,__LINE__); return message;} } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
                                    if (message) return message; } while (0)

/* Value conversions between test items */
#define key2value(k) (k)
#define key1Tokey2(k) buf;sprintf(buf,"%d",(int)k)
#define value2key(v) (v)
#define key2index(k) ((int)(k-1))

/* Hash function */
uint32_t pyHash(uint8_t *a,int s);
/* Time functions */
static inline uint64_t usectime();
static inline uint64_t usecelapsed();
#ifdef TIMETEST
#define TIMEINFO(M) timeInfo(M,__func__,__LINE__);
static inline void timeInfo(char *msg,const char *func,int line)
{
    printf("%s(%d) %s: Total: %lu, Section: %lu\n",func,line,msg,usectime(),usecelapsed());
}
#else
#define TIMEINFO(M)
static inline void timeInfo(char *msg,const char *func,int line) {}
#endif

#ifdef HDEBUG
#define PRINT(FMT,...) fprintf(stderr,"%s:%d: " FMT "\n",__func__,__LINE__,##__VA_ARGS__);
#else
#define PRINT(FMT,...)
#endif
/* Size for larger tests */
#ifndef MAXSIZE
#define MAXSIZE 2000
#endif

char buf[80];
int g_count=0;

int tests_run = 0;

/* Test of basic functions c */
//DEFINE_LIST(Test1,TK1,TV1);
DECLARE_LIST(Test1);
DEFINE_HASH(Test2,TV2);
int rc,c=0;

/** Test for Set */
static char *testHashSet()
{ 
    TK1 key1=1;
    TK2 key2=buf;
    TV1 expect1;
    TV1 result1;
    TV2 expect2;
    TV2 result2;
    bool ret;

    /* Set initial precondition */
    ret=Test1Free(); mu_assert("Initial Free",ret);
    ret=Test2Free(); mu_assert("Initial Free",ret);

    /* Test setting 1=1 */
    key1=1; expect1=key2value(key1);
    ret=Test1Set(key1,expect1); mu_assert("Set Value",ret);
    mu_assert("Count increase",Test1Count()==(int) key1);
    result1=Test1Val(key1); mu_assert("Set Value result",expect1==result1);
    key2=key1Tokey2(key1); expect2=(TV2) expect1;
    //key2=key1Tokey2;
    ret=Test2Set(key2,expect2); mu_assert("Set Value",ret);
    mu_assert("Count increase",Test2Count()==(int) key1);
    result2=Test2Val(key2); mu_assert("Set Value result",expect2==result2);

    /* Test setting 2=10, and count doesn't increase */
    key1=2; expect1=10;
    ret=Test1.set(key1,expect1); mu_assert("Set Value",ret);
    mu_assert("Count increase",Test1.count()==(int) key1);
    result1=Test1.val(key1); mu_assert("Set Value result",expect1==result1);
    key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=Test2.set(key2,expect2); mu_assert("Set Value",ret);
    mu_assert("Count increase",Test2.count()==(int) key1);
    result2=Test2.val(key2); mu_assert("Set Value result",expect2==result2);

    /* Test setting 2=2 */
    key1=2; expect1=key2value(key1);
    ret=Test1Set(key1,expect1); mu_assert("Set Value",ret);
    mu_assert("Count increase",Test1Count()==(int) key1);
    result1=Test1Val(key1); mu_assert("Set Value result",expect1==result1);
    key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=Test2Set(key2,expect2); mu_assert("Set Value",ret);
    mu_assert("Count increase",Test2Count()==(int) key1);
    result2=Test2Val(key2); mu_assert("Set Value result",expect2==result2);

    /* Test setting 3=3 */
    key1=3; expect1=key2value(key1);
    ret=Test1.set(key1,expect1); mu_assert("Set Value",ret);
    mu_assert("Count increase",Test1.count()==(int) key1);
    result1=Test1.val(key1); mu_assert("Set Value result",expect1==result1);
    key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=Test2.set(key2,expect2); mu_assert("Set Value",ret);
    mu_assert("Count increase",Test2.count()==(int) key1);
    result2=Test2.val(key2); mu_assert("Set Value result",expect2==result2);

    /* Test setting 4=4 */
    key1=4; expect1=key2value(key1);
    ret=Test1Set(key1,expect1); mu_assert("Set Value",ret);
    result1=Test1Val(key1); mu_assert("Set Value result",expect1==result1);
    key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=Test2Set(key2,expect2); mu_assert("Set Value",ret);
    result2=Test2Val(key2); mu_assert("Set Value result",expect2==result2);
    mu_assert("Count increase",Test1Count()==(int) key1);
    mu_assert("Count increase",Test2Count()==(int) key1);

    /* Test setting 5=5 */
    key1=5; expect1=key2value(key1);
    ret=Test1.set(key1,expect1); mu_assert("Set Value",ret);
    mu_assert("Count increase",Test1.count()==(int) key1);
    result1=Test1.val(key1); mu_assert("Set Value result",expect1==result1);
    key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=Test2.set(key2,expect2); mu_assert("Set Value",ret);
    mu_assert("Count increase",Test2.count()==(int) key1);
    result2=Test2.val(key2); mu_assert("Set Value result",expect2==result2);

    /* Test setting 6=6 */
    key1=6; expect1=key2value(key1);
    ret=Test1Set(key1,expect1); mu_assert("Set Value",ret);
    mu_assert("Count increase",Test1Count()==(int) key1);
    result1=Test1Val(key1); mu_assert("Set Value result",expect1==result1);
    key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=Test2Set(key2,expect2); mu_assert("Set Value",ret);
    mu_assert("Count increase",Test2Count()==(int) key1);
    result2=Test2Val(key2); mu_assert("Set Value result",expect2==result2);

    /* Global 1count value shared by other tests */
    g_count=Test1Count();

    return 0;
}

/** Test for Get */
static char *testHashGet()
{
    TK1 key1=1;
    TK2 key2=buf;
    TV1 expect1;
    TV1 result1;
    TV2 expect2;
    TV2 result2;
    bool ret;
    int count=g_count;

    /* Note uses values from previous case */
    mu_assert("Existing key1",!testHashSet());

    /* Get key1 with value from previous test */
    key1=1; expect1=key1;key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=Test1Get(key1,&result1); mu_assert("Successful Get",ret);
    mu_assert("Existing key1",expect1==result1);
    key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=Test2Get(key2,&result2); mu_assert("Successful Get",ret);
    mu_assert("Existing key1",expect2==result1);
    count=Test2Count(); mu_assert("Count increase",count==g_count);

    /* Get with missing key1 */
    key1=10; key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=Test1Get(key1,&result1); mu_assert("Missing key1 Get",!ret);
    ret=Test2Get(key2,&result2); mu_assert("Missing key1 Get",!ret);

    /* Get with missing key1 */
    key1=10; key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=Test1.get(key1,&result1); mu_assert("Missing key1 Get",!ret);
    ret=Test2.get(key2,&result2); mu_assert("Missing key1 Get",!ret);


    /* Get pointer with deleted key1 */
    key1=6; key2=key1Tokey2(key1); g_count--;
    ret=Test1Del(key1); mu_assert("Delete of key1 successful",ret);
    ret=Test2.del(key2); mu_assert("Delete of key1 successful",ret);
    mu_assert("Count decrease",Test1Count()==g_count);
    mu_assert("Count decrease",Test2Count()==g_count);

    ret=Test1Get(key1,&result1); mu_assert("Missing key1 Get",!ret);
    ret=Test2Get(key2,&result2); mu_assert("Missing key1 Get",!ret);
    ret=Test1.get(key1,&result1); mu_assert("Missing key1 Get",!ret);
    ret=Test2.get(key2,&result2); mu_assert("Missing key1 Get",!ret);

    return 0;
}


/** Test for Get Pointer */
static char *testHashPtr()
{
    TK1 key1=1;
    TK2 key2=buf;
    TV1 expect1;
    TV1 *result1;
    TV2 expect2;
    TV2 *result2;
    bool ret;
    int count=g_count;

    /* Note uses values from previous case */
    mu_assert("Existing key1",!testHashSet());

    /* Get key1 with value from previous test */
    key1=1; expect1=key1;key2=key1Tokey2(key1); expect2=(TV2) expect1;
    result1=Test1Ptr(key1); mu_assert("Successful Get",expect1==*result1);
    key2=key1Tokey2(key1); expect2=(TV2) expect1;
    result2=Test2Ptr(key2); mu_assert("Successful Get",expect2==*result2);
    count=Test2Count(); mu_assert("Count increase",count==g_count);
    key1=1; expect1=key1;key2=key1Tokey2(key1); expect2=(TV2) expect1;
    result1=Test1.addr(key1); mu_assert("Successful Get",expect1==*result1);
    key2=key1Tokey2(key1); expect2=(TV2) expect1;
    result2=Test2.addr(key2); mu_assert("Successful Get",expect2==*result2);
    count=Test2.count(); mu_assert("Count increase",count==g_count);

    /* Get pointer with missing key1 */
    key1=10; key2=key1Tokey2(key1); expect2=(TV2) expect1;
    result1=Test1Ptr(key1); mu_assert("Missing key1 Get",!result1);
    result2=Test2Ptr(key2); mu_assert("Missing key1 Get",!result2);
    result1=Test1.addr(key1); mu_assert("Missing key1 Get",!result1);
    result2=Test2.addr(key2); mu_assert("Missing key1 Get",!result2);


    /* Get pointer with deleted key1 */
    key1=6; key2=key1Tokey2(key1); g_count--;
    mu_assert("Delete of key1 successful",ret=Test1.del(key1));
    mu_assert("Delete of key2 successful",ret=Test2Del(key2));
    mu_assert("Count decrease",g_count==Test1.count());
    mu_assert("Count decrease",g_count==Test2Count());

    result1=Test1Ptr(key1); mu_assert("Missing key1 Get",!result1);
    result2=Test2Ptr(key2); mu_assert("Missing key1 Get",!result2);

    return 0;
}

/** Test for Get by Value */
static char *testHashVal()
{
    TK1 key1=1;
    TK2 key2=buf;
    TV1 expect1;
    TV1 result1;
    TV2 expect2;
    TV2 result2;
    bool ret;

    /* Note uses values from previous case */
    mu_assert("Existing key1",!testHashSet());

    /* Get key1 with value from previous test */
    key1=1; expect1=key1;key2=key1Tokey2(key1); expect2=(TV2) expect1;
    result1=Test1Val(key1); mu_assert("Successful Val",expect1==result1);
    result1=Test1.val(key1); mu_assert("Successful Val",expect1==result1);
    key2=key1Tokey2(key1); expect2=(TV2) expect1;
    result2=Test2Val(key2); mu_assert("Successful Val",expect2==result2);
    result2=Test2.val(key2); mu_assert("Successful Val",expect2==result2);

    /* Get pointer with missing key1 */
    key1=10; key2=key1Tokey2(key1); expect2=(TV2) expect1;
    result1=Test1Val(key1); mu_assert("Missing key1 Get",!result1);
    result2=Test2Val(key2); mu_assert("Missing key1 Get",!result2);
    result1=Test1.val(key1); mu_assert("Missing key1 Get",!result1);
    result2=Test2.val(key2); mu_assert("Missing key1 Get",!result2);


    /* Get pointer with deleted key1 */
    key1=6; key2=key1Tokey2(key1); g_count--;
    mu_assert("Delete of key1 successful",ret=Test1Del(key1));
    mu_assert("Delete of key2 successful",ret=Test2Del(key2));
    mu_assert("Count decrease",g_count==Test1Count());
    mu_assert("Count decrease",g_count==Test2Count());

    result1=Test1Val(key1); mu_assert("Missing key1 Get",!result1);
    result2=Test2Val(key2); mu_assert("Missing key1 Get",!result2);

    return 0;
}

/** Test for get Keys */
static char *testHashKeys()
{
    TK1 key1=1;
    TK2 key2=buf;
    int input1;
    TK1 expect1;
    TK1 result1;
    TK2 expect2;
    int input2;
    TK2 result2;
    bool ret;

    /* Note uses values from previous case */
    mu_assert("Existing Key",!testHashSet());

    /* Test with existing key */
    key1=1; expect1=key1; input1=key2index(key1); input2=input1;
    key2=key1Tokey2(key1); expect2=key2;
    result1=Test1Keys(input1); mu_assert("Keys result",expect1==result1);
    result2=Test2Keys(input2);
    mu_assert("Keys result",!strcmp(expect2,result2));

    /* Test with out of range negative key */
    key1=-7; input1=key2index(key1);
    input2=input1;
    key2=key1Tokey2(key1);
    result1=Test1Keys(input1);
    result2=Test2Keys(input2);
    /* Add wrap if needed */
    expect1=key2index(key1);
    if (input1<0) expect1=(((int)expect1)%g_count)+g_count;
    else expect1%=g_count;
    expect1+=1;
    expect2=key1Tokey2(expect1);
    mu_assert("Keys result",expect1==result1);
    mu_assert("Keys result",!strcmp(expect2,result2));

    /* Check list key after delete with positive key */
    key1=6; key2=key1Tokey2(key1); g_count--;
    mu_assert("Delete of key1 successful",ret=Test1Del(key1));
    mu_assert("Delete of key2 successful",ret=Test2Del(key2));
    mu_assert("Count decrease",g_count==Test1Count());
    mu_assert("Count decrease",g_count==Test2Count());
    input1=key2index(key1);
    input2=input1;
    result1=Test1Keys(input1);
    result2=Test2.key(input2);
    /* Add wrap if needed */
    expect1=key2index(key1);
    if (input1<0) expect1=(((int)expect1)%g_count)+g_count;
    else expect1%=g_count;
    expect1+=1;
    expect2=key1Tokey2(expect1);
    mu_assert("Keys result",expect1==result1);
    mu_assert("Keys result",!strcmp(key2,result2));

    /* Test wrap if list size is 0 */
    Test1Free();
    Test2.free();
    g_count=0;
    mu_assert("Count decrease",g_count==Test1Count());
    mu_assert("Count decrease",g_count==Test2Count());
    expect1=TK1_0; expect2=TK2_0;
    result1=Test1Keys(input1);mu_assert("Get Keys from empty list",expect1==result1);
    result2=Test2Keys(input2);mu_assert("Get Keys from empty list",expect2==result2);

    return 0;
}

/** Test for Index */
static char *testHashIndex()
{
    TK1 key1=1;
    TK2 key2=buf;
    int expect1;
    int result1;
    int expect2;
    int result2;
    bool ret;

    /* Note uses values from previous case */
    mu_assert("Existing key1",!testHashSet());

    /* Get key1 with value from previous test */
    key1=1; key2=key1Tokey2(key1); expect1=key2index(key1); expect2=(int) expect1;
    result1=Test1Index(key1); mu_assert("Successful Index",expect1==result1);
    result2=Test2Index(key2); mu_assert("Successful Index",expect2==result2);
    result1=Test1.index(key1); mu_assert("Successful Index",expect1==result1);
    result2=Test2.index(key2); mu_assert("Successful Index",expect2==result2);

    /* Get pointer with missing key1 */
    key1=-101; key2=key1Tokey2(key1); expect1=-1; expect2=(int) expect1;
    result1=Test1Index(key1); mu_assert("Missing Index",expect1==result1);
    result2=Test2Index(key2); mu_assert("Missing Index",expect2==result2);
    result1=Test1.index(key1); mu_assert("Missing Index",expect1==result1);
    result2=Test2.index(key2); mu_assert("Missing Index",expect2==result2);

    /* Test after delete of existing key */
    key1=6; key2=key1Tokey2(key1); g_count--;
    expect1=-1; expect2=(int) expect1;
    mu_assert("Delete of key1 successful",ret=Test1Del(key1));
    mu_assert("Delete of key2 successful",ret=Test2Del(key2));
    mu_assert("Count decrease",g_count==Test1.count());
    mu_assert("Count decrease",g_count==Test2.count());
    result1=Test1Index(key1); mu_assert("Missing Index",expect1==result1);
    result2=Test2Index(key2); mu_assert("Missing Index",expect2==result2);
    result1=Test1.index(key1); mu_assert("Missing Index",expect1==result1);
    result2=Test2.index(key2); mu_assert("Missing Index",expect2==result2);

    return 0;
}

/** Test for Get Item */
static char *testHashItem()
{
    TK1 key1=1;
    TK2 key2=buf;
    int input1;
    TV1 expect1;
    TV1 result1;
    TV2 expect2;
    int input2;
    TV2 result2;
    bool ret;

    /* Note uses values from previous case */
    mu_assert("Existing Key",!testHashSet());

    /* Test with existing key */
    key1=1; expect1=key1;
    key2=key1Tokey2(key1); expect2=(int) expect1;
    input1=key2index(key1); input2=input1;
    ret=Test1Item(input1,&result1);mu_assert("Index return",ret);
    mu_assert("Index result",expect1==result1);
    ret=Test2Item(input2,&result2);mu_assert("Index return",ret);
    mu_assert("Index result",expect2==result2);

    /* Test with out of range key */
    key1=-712; expect1=0;
    key2=key1Tokey2(key1); expect2=(int) expect1;
    input1=key2index(key1); input2=input1;
    result1=Test1Item(input1,&result1);mu_assert("Index return",ret);
    result2=Test2Item(input2,&result2);mu_assert("Index return",ret);
    mu_assert("Index result",expect1==result1);
    mu_assert("Index result",expect2==result2);

    /* Check list key after delete */
    key1=6; key2=key1Tokey2(key1); g_count--;
    input1=key2index(key1); input2=input1;
    expect1=0; expect2=0;
    mu_assert("Delete of key1 successful",ret=Test1Del(key1));
    mu_assert("Delete of key2 successful",ret=Test2Del(key2));
    mu_assert("Count decrease",g_count==Test1Count());
    mu_assert("Count decrease",g_count==Test2Count());
    result1=Test1Item(input1,&result1);mu_assert("Index return",ret);
    result2=Test2Item(input2,&result2);mu_assert("Index return",ret);
    mu_assert("Index result",expect1==result1);
    mu_assert("Index result",expect2==result2);


    return 0;
}

/** Test for Has Key */
static char *testHashHasKey()
{
    TK1 key1=1;
    TK2 key2=buf;
    int input1;
    TV1 expect1;
    TV1 result1;
    TV2 expect2;
    int input2;
    TV2 result2;
    bool ret;

    /* Note uses values from previous case */
    mu_assert("Existing Key",!testHashSet());

    /* Test with existing key */
    key1=1; expect1=key1;
    key2=key1Tokey2(key1); expect2=(int) expect1;
    input1=key2index(key1); input2=input1;
    ret=Test1Item(input1,&result1);mu_assert("Index return",ret);
    mu_assert("Index result",expect1==result1);
    ret=Test2Item(input2,&result2);mu_assert("Index return",ret);
    mu_assert("Index result",expect2==result2);
    /* Note uses values from previous case */
    mu_assert("Existing Key",!testHashSet());

    /* Note uses values from previous case */
    key1=3; expect1=key1;
    key2=key1Tokey2(key1); expect2=(int) expect1;
    mu_assert("HashHasKey Successful HasKey",Test1HasKey(key1));
    mu_assert("HashHasKey Successful HasKey",Test2HasKey(key2));
    mu_assert("HashHasKey Successful HasKey",Test1.hasKey(key1));
    mu_assert("HashHasKey Successful HasKey",Test2.hasKey(key2));

    key1=7; expect1=key1;
    key2=key1Tokey2(key1); expect2=(int) expect1;
    mu_assert("HashHasKey Missing Key",!Test1HasKey(key1));
    mu_assert("HashHasKey Missing Key",!Test2HasKey(key2));
    mu_assert("HashHasKey Missing Key",!Test1.hasKey(key1));
    mu_assert("HashHasKey Missing Key",!Test2.hasKey(key2));
    
    key1=3; expect1=key1;g_count--;
    key2=key1Tokey2(key1); expect2=(int) expect1;
    Test1Del(key1);
    mu_assert("HashDel deleting key decreased count",Test1Count()==g_count);
    mu_assert("HashHasKey Deleted Key",!Test1HasKey(key1));
    Test2Del(key2);
    mu_assert("HashDel deleting key decreased count",Test2Count()==g_count);
    mu_assert("HashHasKey Deleted Key",!Test2HasKey(key2));

    return 0;
}

/** Test for Delete */
static char *testHashDel()
{
    TK1 key1=1;
    TK2 key2=buf;


    /* Note uses values from previous case */
    mu_assert("Existing Key",!testHashSet());

    //int lin=0;HASH_FOREACH(Test1,v) { printf("%d: %d==%d\n",__LINE__,++lin,v); }
    /* Note uses values from previous case */
    key1=3; key2=key1Tokey2(key1); g_count--;
    mu_assert("HashHasKey Successful HasKey",Test1HasKey(key1));
    mu_assert("HashHasKey Successful HasKey",Test2HasKey(key2));
    mu_assert("HashDel Successful Del",Test1Del(key1));
    mu_assert("HashDel Successful Del",Test2Del(key2));
    mu_assert("HashDel deleting key decreased count",Test1Count()==g_count);
    mu_assert("HashDel deleting key decreased count",Test2Count()==g_count);
    mu_assert("HashHasKey Deleted Key",!Test1HasKey(key1));
    mu_assert("HashHasKey Deleted Key",!Test2HasKey(key2));
    key1=1; key2=key1Tokey2(key1);
    mu_assert("HashHasKey Remainging after delete",Test1HasKey(key1));
    mu_assert("HashHasKey Successful after delete",Test2HasKey(key2));
    key1=2; key2=key1Tokey2(key1);
    mu_assert("HashHasKey Remainging after delete",Test1.hasKey(key1));
    mu_assert("HashHasKey Successful after delete",Test2.hasKey(key2));
    key1=5; key2=key1Tokey2(key1);
    mu_assert("HashHasKey Remainging after delete",Test1.hasKey(key1));
    mu_assert("HashHasKey Successful after delete",Test2.hasKey(key2));
    key1=6; key2=key1Tokey2(key1);
    mu_assert("HashHasKey Remainging after delete",Test1HasKey(key1));
    mu_assert("HashHasKey Successful after delete",Test2HasKey(key2));

    key1=4; key2=key1Tokey2(key1); g_count--;
    mu_assert("HashHasKey Successful HasKey",Test1HasKey(key1));
    mu_assert("HashHasKey Successful HasKey",Test2HasKey(key2));
    mu_assert("HashDel Successful Del",Test1Del(key1));
    mu_assert("HashDel Successful Del",Test2Del(key2));
    mu_assert("HashDel deleting key decreased count",Test1Count()==g_count);
    mu_assert("HashDel deleting key decreased count",Test2Count()==g_count);
    mu_assert("HashHasKey Deleted HasKey",!Test1HasKey(key1));
    mu_assert("HashHasKey Deleted HasKey",!Test2HasKey(key2));
    key1=1; key2=key1Tokey2(key1);
    mu_assert("HashHasKey Remainging after delete",Test1HasKey(key1));
    mu_assert("HashHasKey Successful after delete",Test2HasKey(key2));
    key1=2; key2=key1Tokey2(key1);
    mu_assert("HashHasKey Remainging after delete",Test1HasKey(key1));
    mu_assert("HashHasKey Successful after delete",Test2HasKey(key2));
    key1=5; key2=key1Tokey2(key1);
    mu_assert("HashHasKey Remainging after delete",Test1HasKey(key1));
    mu_assert("HashHasKey Successful after delete",Test2HasKey(key2));
    key1=6; key2=key1Tokey2(key1);
    mu_assert("HashHasKey Remainging after delete",Test1HasKey(key1));
    mu_assert("HashHasKey Successful after delete",Test2HasKey(key2));


    key1=0; key2=key1Tokey2(key1);
    mu_assert("HashHasKey Deleted HasKey",!Test1HasKey(key1));
    mu_assert("HashHasKey Deleted HasKey",!Test2HasKey(key2));
    mu_assert("HashDel Missing Key",!Test1.del(key1));
    mu_assert("HashDel Missing Key",!Test2.del(key2));


    return 0;
}

DEFINE_LIST(Test3,int,test_fields_t);
DECLARE_LIST(Test4);
DEFINE_LIST_ITERATOR(Test3,int,ifield)
DEFINE_LIST_ITERATOR(Test3,bool,bfield)
DEFINE_LIST_ITERATOR(Test4,int,ifield)
DEFINE_LIST_ITERATOR(Test4,bool,bfield)

/** Test for For Each */
static char *testForEach()
{
    TK1 key1=1;
    TK2 key2=buf;
    TV1 result1;
    TV2 result2;
    TV1 *rptr1;
    TV1 expect1;
    TV2 expect2;
    test_fields_t expect;
    test_fields_t result;
    test_fields_t *tptr;
    int in,rin;

    /* Note uses values from previous case */
    mu_assert("Existing Key",!testHashSet());

    for (in=0;in<20;in++) {
        /* Set fields */
        expect.ifield=in;
        if (in%2) expect.bfield=true;
        else expect.bfield=false;

        expect.ffield=1.5 + (float)(in%4);
        key1=30+in; key2=key1Tokey2(key1);
        Test3Set(key1,expect);
        Test4Set(key2,expect);

        mu_assert("Struct get",Test3Get(key1,&result));
        tptr=Test3Ptr(key1);
        mu_assert("Struct ptr",tptr);
        mu_assert("IField",expect.ifield==tptr->ifield);
        mu_assert("BField",expect.bfield==tptr->bfield);
        mu_assert("FField",expect.ffield==tptr->ffield);

        mu_assert("Struct get",Test4Get(key2,&result));
        tptr=Test4Ptr(key2);
        mu_assert("Struct ptr",tptr);
        mu_assert("IField",expect.ifield==tptr->ifield);
        mu_assert("BField",expect.bfield==tptr->bfield);
        mu_assert("FField",expect.ffield==tptr->ffield);
    }

    in=0;
    HASH_FOREACH(Test3,result) {
        int iflf;
        bool bflf;
        float fflf;
        
        /* Set fields */
        iflf=result.ifield;
        //iflf=in;
        if (iflf%2) bflf=true;
        else bflf=false;

        fflf=1.5 + (float)(iflf%4);

        //PRINT("in: %d in:%d==%d, B:%d==%d F:%f=%f",in,result.ifield,iflf,
                //result.bfield,bflf,result.ffield, fflf);

        mu_assert("ForEach Intfield",iflf==result.ifield);
        mu_assert("ForEach boolfield",bflf==result.bfield);
        mu_assert("ForEach floatfield",fflf==result.ffield);
        in++;
     }

    in=0;result1=7;
    while ((tptr=Test3_iter_ifield(result1))) {
        mu_assert("Ifield",tptr->ifield==result1);
        mu_assert("bfield",tptr->bfield==(result1%2));
        mu_assert("ffield",tptr->ffield==(1.5+(float)(result1%4)));
        in++;
    }
    in=0;rin=0;
    while ((tptr=Test4_iter_r_bfield(&rin,true))) {
        mu_assert("Ifield",tptr->ifield==(rin-1));
        mu_assert("bfield",tptr->bfield==((rin-1)%2));
        mu_assert("ffield",tptr->ffield==(1.5+(float)((rin-1)%4)));
        in++;
    }
    in=0;result1=0;
    while (Test3_next_bfield(&result,false)) {
        result1=in*2;
        mu_assert("Ifield",result.ifield==(result1));
        mu_assert("bfield",result.bfield==((result1)%2));
        mu_assert("ffield",result.ffield==(1.5+(float)((result1)%4)));
        in++;
    }
    mu_assert("Iter Count",in==10);

    Test3Free();
    Test4Free();

    /* Note uses values from previous case */
    mu_assert("Existing Key",!testHashSet());

    expect1=1;
    HASH_FOREACH_ADDR(Test1,rptr1) {
        key1=(TK1) *rptr1;
        mu_assert("Delete Entries in ForEach",*rptr1==(TV1)expect1);
        mu_assert("Delete Entry count in ForEach",g_count==Test1Count());
        Test1Del(key1);
        g_count--;
        expect1++;
    }
    mu_assert("Delete Entry final count",0==Test1Count());

    /* Note uses values from previous case */
    mu_assert("Existing Key",!testHashSet());

    expect2=1;
    HASH_FOREACH(Test2,result2) {
        key1=(TK1) result2;
        key2=key1Tokey2(key1);
        mu_assert("Delete Entries in ForEach",result2==(TV2)expect2);
        mu_assert("Delete Entry count in ForEach",g_count==Test2Count());
        Test2Del(key2);
        g_count--;
        expect2++;
    }
    mu_assert("Delete Entry final count",0==Test2Count());
    
    return 0;
}

/** Test for Load/Save */
static char *testHashLoad()
{
    TK1 key1=1;
    TK2 key2=buf;
    TV1 expect1;
    TV1 result1;
    TV2 expect2;
    TV2 result2;
    bool ret;
    int count=g_count;

    /* Note uses values from previous case */
    mu_assert("Existing key1",!testHashSet());

    mu_assert("Save Test 1",Test1.save("/tmp/test1.hash"));
    mu_assert("Save Test 2",Test2Save("/tmp/test2.hash"));

    mu_assert("Free returns successful",Test1Free());
    mu_assert("Free returns successful",Test2Free());

    mu_assert("Test1 is Empty",Test1Count()==0);
    mu_assert("Test2 is Empty",Test2Count()==0);

    mu_assert("Load Test 1",Test1Load("/tmp/test1.hash"));
    mu_assert("Load Test 2",Test2.load("/tmp/test2.hash"));
    mu_assert("Load Test 2",Test2.load("/tmp/test2.hash"));

    unlink("/tmp/test1.hash");
    unlink("/tmp/test2.hash");

    count=Test1Count(); mu_assert("Count increase",count==g_count);
    count=Test2Count(); mu_assert("Count increase",count==g_count);

    /* Get key1 with value from previous test */
    key1=1; expect1=key1;key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=Test1Get(key1,&result1); mu_assert("Successful Get",ret);
    mu_assert("Existing key1",expect1==result1);
    key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=Test2Get(key2,&result2); mu_assert("Successful Get",ret);
    mu_assert("Existing key1",expect2==result1);
    count=Test2Count(); mu_assert("Count increase",count==g_count);

    /* Get with missing key1 */
    key1=10; key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=Test1Get(key1,&result1); mu_assert("Missing key1 Get",!ret);
    ret=Test2Get(key2,&result2); mu_assert("Missing key1 Get",!ret);

    /* Get with missing key1 */
    key1=10; key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=Test1.get(key1,&result1); mu_assert("Missing key1 Get",!ret);
    ret=Test2.get(key2,&result2); mu_assert("Missing key1 Get",!ret);


    /* Get pointer with deleted key1 */
    key1=6; key2=key1Tokey2(key1); g_count--;
    ret=Test1Del(key1); mu_assert("Delete of key1 successful",ret);
    ret=Test2.del(key2); mu_assert("Delete of key1 successful",ret);
    mu_assert("Count decrease",Test1Count()==g_count);
    mu_assert("Count decrease",Test2Count()==g_count);

    ret=Test1Get(key1,&result1); mu_assert("Missing key1 Get",!ret);
    ret=Test2Get(key2,&result2); mu_assert("Missing key1 Get",!ret);
    ret=Test1.get(key1,&result1); mu_assert("Missing key1 Get",!ret);
    ret=Test2.get(key2,&result2); mu_assert("Missing key1 Get",!ret);


    return 0;
}

DEFINE_FIFO(Test5,TV1);
DECLARE_LIST(Test6);
/** Test for Fifo */
static char *testFifo()
{
    TV1 result5;
    TV2 result6;
    TV1 expect5;
    TV2 expect6;
    int count=0;
    bool ret;
    //DBUG_SW(true);

    expect5=(TV1) 1;
    expect6=(TV2) 1;
    ret=Test5Push(expect5); mu_assert("Push Value",ret); count++;
    ret=Test6Push(expect6); mu_assert("Push Value",ret);
    mu_assert("FIFOPush count increase",Test5Count()==count);
    mu_assert("FIFOPush count increase",Test6Count()==count);
    ret=Test5Pop(&result5); mu_assert("Set Value result",expect5==result5); count--;
    ret=Test6Pop(&result6); mu_assert("Set Value result",expect6==result6);
    mu_assert("FifoPop count decrease",Test5Count()==count);
    mu_assert("FifoPop count decrease",Test6Count()==count);

    expect5=(TV1) 1;
    expect6=(TV2) 1;
    ret=Test5Push(expect5); mu_assert("Push Value",ret); count++;
    ret=Test6Push(expect6); mu_assert("Push Value",ret);
    mu_assert("FIFOPush count increase",Test5Count()==count);
    mu_assert("FIFOPush count increase",Test6Count()==count);

    expect5=(TV1) 2;
    expect6=(TV2) 2;
    ret=Test5Push(expect5); mu_assert("Push Value",ret); count++;
    ret=Test6Push(expect6); mu_assert("Push Value",ret);
    mu_assert("FIFOPush count increase",Test5Count()==count);
    mu_assert("FIFOPush count increase",Test6Count()==count);

    expect5=(TV1) 3;
    expect6=(TV2) 3;
    ret=Test5.push(expect5); mu_assert("Push Value",ret); count++;
    ret=Test6.push(expect6); mu_assert("Push Value",ret);
    mu_assert("FIFOPush count increase",Test5Count()==count);
    mu_assert("FIFOPush count increase",Test6Count()==count);

    expect5=0; HASH_FOREACH(Test5,result5) {
        mu_assert("HashForEach5 next val",++expect5==result5);
    }
    expect6=0; HASH_FOREACH(Test6,result6) {
        mu_assert("HashForEach6 next val",++expect6==result6);
    }

    expect5=3;
    expect6=1;
    ret=Test5Pop(&result5); mu_assert("Set Value result",expect5==result5); count--;
    ret=Test6Next(&result6); mu_assert("Set Value result",expect6==result6);
    mu_assert("FifoPop count decrease",Test5Count()==count);
    mu_assert("FifoPop count decrease",Test6Count()==count);

    expect5=2;
    expect6=2;
    ret=Test5.pop(&result5); mu_assert("Set Value result",expect5==result5); count--;
    ret=Test6.next(&result6); mu_assert("Set Value result",expect6==result6);
    mu_assert("FifoPop count decrease",Test5Count()==count);
    mu_assert("FifoPop count decrease",Test6Count()==count);

    expect5=1;
    expect6=3;
    ret=Test5Pop(&result5); mu_assert("Set Value result",expect5==result5); count--;
    ret=Test6Next(&result6); mu_assert("Set Value result",expect6==result6);
    mu_assert("FifoPop count decrease",Test5Count()==count);
    mu_assert("FifoPop count decrease",Test6Count()==count);
    DBUG_SW(false);

    return 0;
}

/** Test for Free list */
static char *testHashFree()
{
    /* Note uses values from previous case */
    mu_assert("Existing Key",!testHashSet());

    /* Note uses values from previous case */
    mu_assert("HashDel deleting key decreased count",Test1Count()==g_count);
    mu_assert("HashDel deleting key decreased count",Test2Count()==g_count);
    mu_assert("Free returns successful",Test1Free());
    mu_assert("Free returns successful",Test2Free());
    g_count=0;
    mu_assert("HashDel deleting key decreased count",Test1Count()==g_count);
    mu_assert("HashDel deleting key decreased count",Test2Count()==g_count);
    mu_assert("Free returns successful",Test1Free());
    mu_assert("Free returns successful",Test2Free());
    mu_assert("HashDel deleting key decreased count",Test1Count()==g_count);
    mu_assert("HashDel deleting key decreased count",Test2Count()==g_count);

    return 0;
}

/* Test shared list */
DEFINE_LIST(TestB,TK1,TV1);
DEFINE_LIST(TestC,TK1,TV1);
DEFINE_LIST(TestD,TK1,TV1);
DEFINE_HASH(TestS1,TV1);
DEFINE_HASH(TestS2,TV1);

static char * testNetShare(void)
{
    TK1 key1;
    TK2 key2=buf;
    TV2 expect2;
    TV2 result2;
    TV1 expect1;
    TV1 result1;
    TK1 keyB;
    TV1 expectB;
    TV1 resultB;
    TV1 resultC;
    TV1 resultD;
    bool ret;
    int netPort=6501;
#ifdef TIMETEST
    int i;
#endif

    Test1Free();

    Test1NetStart(netPort);
    TestBNetStart(netPort);
    TestCNetStart(netPort);

    key1=1;    expect1=key2value(key1);
    keyB=key1; expectB=key2value(keyB);
    ret=Test1Set(key1,expect1); mu_assert("Set Value",ret); usleep(10000);
    mu_assert("Count increase",Test1Count()==(int) key1);
    result1=Test1Val(key1); mu_assert("Set Value result",expect1==result1);
    mu_assert("Count increase",TestBCount()==(int) key1);
    resultB=TestBVal(keyB); mu_assert("Set Value result",expectB==resultB);
    mu_assert("Count increase",TestCCount()==(int) key1);
    resultC=TestCVal(key1); mu_assert("Set Value result",expect1==resultC);
    mu_assert("Count increase",TestDCount()==(int) 0);

    key1=2;    expect1=key2value(key1);
    keyB=key1; expectB=key2value(keyB);
    ret=TestBSet(keyB,expectB); mu_assert("Set Value",ret); usleep(10000);
    mu_assert("Count increase",Test1Count()==(int) key1);
    result1=Test1Val(key1); mu_assert("Set Value result",expect1==result1);
    mu_assert("Count increase",TestBCount()==(int) key1);
    resultB=TestBVal(keyB); mu_assert("Set Value result",expectB==resultB);
    mu_assert("Count increase",TestCCount()==(int) key1);
    resultC=TestCVal(key1); mu_assert("Set Value result",expect1==resultC);
    mu_assert("Count increase",TestDCount()==(int) 0);

    TestDNetStart(netPort);

    sleep(1);
    key1=3;    expect1=key2value(key1);
    keyB=key1; expectB=key2value(keyB);
    ret=Test1Set(key1,expect1); mu_assert("Set Value",ret); usleep(10000);
    mu_assert("Count increase",Test1Count()==(int) key1);
    result1=Test1Val(key1); mu_assert("Set Value result",expect1==result1);
    mu_assert("Count increase",TestBCount()==(int) key1);
    resultB=TestBVal(keyB); mu_assert("Set Value result",expectB==resultB);
    mu_assert("Count increase",TestCCount()==(int) key1);
    resultC=TestCVal(key1); mu_assert("Set Value result",expect1==resultC);
    mu_assert("Count increase",TestDCount()==(int) key1);
    resultD=TestDVal(key1); mu_assert("Set Value result",expect1==resultD);

    key1=4;    expect1=key2value(key1);
    keyB=key1; expectB=key2value(keyB);
    ret=TestBSet(keyB,expectB); mu_assert("Set Value",ret); usleep(10000);
    mu_assert("Count increase",Test1Count()==(int) key1);
    result1=Test1Val(key1); mu_assert("Set Value result",expect1==result1);
    mu_assert("Count increase",TestBCount()==(int) key1);
    resultB=TestBVal(keyB); mu_assert("Set Value result",expectB==resultB);
    mu_assert("Count increase",TestCCount()==(int) key1);
    resultC=TestCVal(key1); mu_assert("Set Value result",expect1==resultC);
    mu_assert("Count increase",TestDCount()==(int) key1);
    resultD=TestDVal(key1); mu_assert("Set Value result",expect1==resultD);

    key1=5;    expect1=key2value(key1);
    keyB=key1; expectB=key2value(keyB);
    ret=Test1Set(key1,expect1); mu_assert("Set Value",ret); usleep(10000);
    mu_assert("Count increase",Test1Count()==(int) key1);
    result1=Test1Val(key1); mu_assert("Set Value result",expect1==result1);
    mu_assert("Count increase",TestBCount()==(int) key1);
    resultB=TestBVal(keyB); mu_assert("Set Value result",expectB==resultB);
    mu_assert("Count increase",TestCCount()==(int) key1);
    resultC=TestCVal(key1); mu_assert("Set Value result",expect1==resultC);
    mu_assert("Count increase",TestDCount()==(int) key1);
    resultD=TestDVal(key1); mu_assert("Set Value result",expect1==resultD);

    key1=6;    expect1=key2value(key1);
    keyB=key1; expectB=key2value(keyB);
    ret=TestBSet(keyB,expectB); mu_assert("Set Value",ret); usleep(10000);
    mu_assert("Count increase",Test1Count()==(int) key1);
    result1=Test1Val(key1); mu_assert("Set Value result",expect1==result1);
    mu_assert("Count increase",TestBCount()==(int) key1);
    resultB=TestBVal(keyB); mu_assert("Set Value result",expectB==resultB);
    mu_assert("Count increase",TestCCount()==(int) key1);
    resultC=TestCVal(key1); mu_assert("Set Value result",expect1==resultC);
    mu_assert("Count increase",TestDCount()==(int) key1);
    resultD=TestDVal(key1); mu_assert("Set Value result",expect1==resultD);

    g_count=6;
    key1=2;    expect1=key2value(key1+10);
    keyB=key1; expectB=key2value(keyB+10);
    ret=Test1Set(key1,expect1); mu_assert("Set Value",ret);
    mu_assert("Count increase",Test1Count()==(int) g_count); usleep(10000);
    result1=Test1Val(key1); mu_assert("Set Value result",expect1==result1);
    mu_assert("Count increase",TestBCount()==(int) g_count);
    resultB=TestBVal(keyB); mu_assert("Set Value result",expectB==resultB);
    mu_assert("Count increase",TestCCount()==(int) g_count);
    resultC=TestCVal(key1); mu_assert("Set Value result",expect1==resultC);
    mu_assert("Count increase",TestDCount()==(int) g_count);
    resultD=TestDVal(key1); mu_assert("Set Value result",expect1==resultD);

    key1=3;    expect1=key2value(key1+10);
    keyB=key1; expectB=key2value(keyB+10);
    ret=TestBSet(keyB,expectB); mu_assert("Set Value",ret); usleep(10000);
    mu_assert("Count increase",Test1Count()==(int) g_count);
    result1=Test1Val(key1); mu_assert("Set Value result",expect1==result1);
    mu_assert("Count increase",TestBCount()==(int) g_count);
    resultB=TestBVal(keyB); mu_assert("Set Value result",expectB==resultB);
    mu_assert("Count increase",TestCCount()==(int) g_count);
    resultC=TestCVal(key1); mu_assert("Set Value result",expect1==resultC);
    mu_assert("Count increase",TestDCount()==(int) g_count);
    resultD=TestDVal(key1); mu_assert("Set Value result",expect1==resultD);

    key1=3;g_count--;
    ret=Test1Del(key1); mu_assert("Delete of key1 successful",ret); usleep(10000);
    mu_assert("Count decrease",Test1Count()==g_count);
    mu_assert("HashHasKey Deleted HasKey",!Test1HasKey(key1));
    mu_assert("Count decrease",TestBCount()==g_count);
    mu_assert("HashHasKey Deleted HasKey",!TestBHasKey(key1));
    mu_assert("Count decrease",TestCCount()==g_count);
    mu_assert("HashHasKey Deleted HasKey",!TestCHasKey(key1));
    mu_assert("Count decrease",TestDCount()==g_count);
    mu_assert("HashHasKey Deleted HasKey",!TestDHasKey(key1));

    key1=2;g_count--;
    ret=TestDDel(key1); mu_assert("Delete of key1 successful",ret); usleep(10000);
    mu_assert("Count decrease",Test1Count()==g_count);
    mu_assert("HashHasKey Deleted HasKey",!Test1HasKey(key1));
    mu_assert("Count decrease",TestBCount()==g_count);
    mu_assert("HashHasKey Deleted HasKey",!TestBHasKey(key1));
    mu_assert("Count decrease",TestCCount()==g_count);
    mu_assert("HashHasKey Deleted HasKey",!TestCHasKey(key1));
    mu_assert("Count decrease",TestDCount()==g_count);
    mu_assert("HashHasKey Deleted HasKey",!TestDHasKey(key1));

#ifdef TIMETEST
    usecelapsed();
    for (i=10;i<500;i+=4) {
        key1=i;
        ret=Test1Set(key1+0,key1+0); mu_assert("Set Value",ret);g_count++;
        ret=TestBSet(key1+1,key1+1); mu_assert("Set Value",ret);g_count++;
        ret=TestCSet(key1+2,key1+2); mu_assert("Set Value",ret);g_count++;
        ret=TestDSet(key1+3,key1+3); mu_assert("Set Value",ret);g_count++;
        //PRINT("1: %d 9: %d A: %d B: %d", Test1Count(), TestBCount(),TestCCount(), TestDCount());
    }
    PRINT("Load count: %d time: %0.6lf seconds\n",g_count,((double) usecelapsed())/1000000.0);
    mu_assert("Count match",Test1Count()==g_count);
    mu_assert("Count match",TestBCount()==g_count);
    mu_assert("Count match",TestCCount()==g_count);
    mu_assert("Count match",TestDCount()==g_count);
#endif


    Test1Free();
    TestBFree();
    TestCFree();
    TestDFree();

    TestS1NetStart(netPort);
    TestS2NetStart(netPort);

    key1=1;    expect1=key2value(key1);
    key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=TestS1Set(key2,expect2); mu_assert("Set Value",ret); usleep(10000);
    mu_assert("Count increase",TestS1Count()==(int) key1);
    result2=TestS1Val(key2); mu_assert("Set Value result",expect2==result2);
    mu_assert("Count increase",TestS2Count()==(int) key1);
    result2=TestS2Val(key2); mu_assert("Set Value result",expect2==result2);

    key1=2;    expect1=key2value(key1);
    key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=TestS2Set(key2,expect2); mu_assert("Set Value",ret); usleep(10000);
    mu_assert("Count increase",TestS2Count()==(int) key1);
    result2=TestS2Val(key2); mu_assert("Set Value result",expect2==result2);
    mu_assert("Count increase",TestS1Count()==(int) key1);
    result2=TestS1Val(key2); mu_assert("Set Value result",expect2==result2);

    key1=3;    expect1=key2value(key1);
    key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=TestS1Set(key2,expect2); mu_assert("Set Value",ret); usleep(10000);
    mu_assert("Count increase",TestS1Count()==(int) key1);
    result2=TestS1Val(key2); mu_assert("Set Value result",expect2==result2);
    mu_assert("Count increase",TestS2Count()==(int) key1);
    result2=TestS2Val(key2); mu_assert("Set Value result",expect2==result2);

    key1=4; g_count=key1; expect1=key2value(key1);
    key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=TestS2Set(key2,expect2); mu_assert("Set Value",ret); usleep(10000);
    mu_assert("Count increase",TestS2Count()==(int) g_count);
    result2=TestS2Val(key2); mu_assert("Set Value result",expect2==result2);
    mu_assert("Count increase",TestS1Count()==(int) g_count);
    result2=TestS1Val(key2); mu_assert("Set Value result",expect2==result2);

    key1=2;g_count--;
    key2=key1Tokey2(key1); expect2=(TV2) expect1;
    ret=TestS1Del(key2); mu_assert("Delete of key1 successful",ret); usleep(10000);
    mu_assert("Count decrease",TestS1Count()==g_count);
    mu_assert("HashHasKey Deleted HasKey",!TestS1HasKey(key2));
    mu_assert("Count decrease",TestS2Count()==g_count);
    mu_assert("HashHasKey Deleted HasKey",!TestS2HasKey(key2));

    TestS1Free();
    TestS2Free();

    return 0;
}


/* Test larger dataset */
DEFINE_LIST(Test7,int,uint32_t);
DEFINE_LIST(Test8,uint64_t,uint64_t);
DEFINE_LIST(Test9,int,uint32_t);
DEFINE_HASH(TestA,uint64_t);
static char * testLargeHash(void)
{
    TIMEINFO("start");/* Print time info if enabled */
    int i;
    int max=MAXSIZE;
    char buf[80];
    uint32_t h;
    int v;
    int count;

    /* Set items and ensure count is correct */
    for (i=0;i<max;i++) {
        h=pyHash((uint8_t *)&i,sizeof(i));
        v=h>>2;
        Test7Set(i,h);
        sprintf(buf,"%05x",h);
        TestASet(buf,v);
        Test9Set(h,v);
        if ((i%(max/10))==0) {
            mu_assert("Load Hash Count",Test9Count()==(i+1));
            mu_assert("Load Hash Count",TestACount()==(i+1));
        }
    }
    TIMEINFO("First Set done");/* Print time info if enabled */
    /* Get Items and compare values */
    for (i=0;i<max;i++) {
        int t1;
        int t2;
        h=Test7Val(i);
        sprintf(buf,"%05x",h);
        t2=TestAVal(buf);
        t1=Test9Val(h);
        mu_assert("Compare Values",t1==t2);
    }
    TIMEINFO("Compare done");/* Print time info if enabled */
    /* Delete items and check count */
    for (i=0;i<max;i++) {
        h=Test7Val(i);
        sprintf(buf,"%05x",h);
        TestAHasKey(buf);
        Test9HasKey(h);
        mu_assert("HasKey Entry",TestAHasKey(buf));
        mu_assert("HasKey Entry",Test9HasKey(h));
        mu_assert("Delete Entry",TestADel(buf));
        mu_assert("Delete Entry",Test9Del(h));
        if ((i%(max/10))==0) {
            mu_assert("Delete Hash Count",Test9Count()==(max-i-1));
            mu_assert("Delete Hash Count",TestACount()==(max-i-1));
        }
    }
    TIMEINFO("Delete done");/* Print time info if enabled */
    /* Set items and ensure count is correct */
    TIMEINFO("Set allocated List again");/* Print time info if enabled */
    for (i=0;i<max;i++) {
        h=Test7Val(i)>>1;
        sprintf(buf,"%d",i);
        TestASet(buf,h);
        Test9Set(h,i);
        if ((i%(max/10))==0) {
            mu_assert("Load Hash Count",Test9Count()==(i+1));
            mu_assert("Load Hash Count",TestACount()==(i+1));
        }
    }
    TIMEINFO("Set List again");/* Print time info if enabled */
    count=Test7.count();
    TestA.save("/tmp/testA.hash");
    Test9.save("/tmp/test9.hash");
    TIMEINFO("Save Done");/* Print time info if enabled */
    TestAFree();
    Test9Free();
    TIMEINFO("Two Frees full list done");/* Print time info if enabled */
    mu_assert("Free Hash Count",TestACount()==0);
    mu_assert("Free Hash Count",Test9Count()==0);
    TestAFree();
    Test9Free();
    mu_assert("Free Hash Count",TestACount()==0);
    mu_assert("Free Hash Count",Test9Count()==0);
    /* Set items and ensure count is correct */
    TIMEINFO("Set allocated List again");/* Print time info if enabled */
    TestA.load("/tmp/testA.hash");
    Test9.load("/tmp/test9.hash");
    unlink("/tmp/testA.hash");
    unlink("/tmp/test9.hash");
    mu_assert("Free Hash Count",TestACount()==count);
    mu_assert("Free Hash Count",Test9Count()==count);
    for (i=0;i<max;i++) {
        uint32_t u9;
        uint64_t uA;
        h=Test7Val(i)>>1;
        sprintf(buf,"%d",i);
        mu_assert("Get Loaded Value",Test9Get(h,&u9));
        mu_assert("Check Loaded Value",u9==i);
        mu_assert("Get Loaded Value",TestAGet(buf,&uA));
        mu_assert("Check Loaded Value",uA==h);
    }

    Test9Free();
    TestAFree();
    TIMEINFO("Two Frees full list done");/* Print time info if enabled */
    mu_assert("Free Hash Count",Test9Count()==0);
    mu_assert("Free Hash Count",TestACount()==0);
    Test9Free();
    TestAFree();
    mu_assert("Free Hash Count",Test9Count()==0);
    mu_assert("Free Hash Count",TestACount()==0);

    TIMEINFO("Complete");/* Print time info if enabled */

    /* Set items and ensure count is correct */
    {
        int cnt=0,j=0,c;
        max=Test7Count();
        c=11;
        h=0;
        usecelapsed();
        for (i=0;i<c;i++) for (j=1;j<max;j++) {
            Test7Get(max-j,&h);
            Test8Set(h+i,h+i+j);
            cnt++;
        }
        PRINT("Load count: %d time: %0.6lf seconds\n",cnt,((double) usecelapsed())/1000000.0);
    }
    Test8Free();
    return 0;
}

DEFINE_LIST(Thread1,int,uint32_t);
DEFINE_LIST(Thread2,int,uint32_t);
DEFINE_LIST(Thread3,int,uint32_t);
static pthread_t test1Handle; /** Handle to thread */
#ifndef MEM_TEST
static pthread_t test2Handle; /** Handle to thread */
static pthread_t test3Handle; /** Handle to thread */
#endif

pthread_mutex_t mutexCond = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t testStartCond = PTHREAD_COND_INITIALIZER;

static char *test1Thread(void *parm)
{
    pthread_mutex_lock(&mutexCond);
    pthread_cond_wait(&testStartCond, &mutexCond);
    pthread_mutex_unlock(&mutexCond);
    TIMEINFO("GO");/* Print time info if enabled */
    while (Test7Count()) {
        int k;
        uint32_t v;
        k=Test7Keys(0);
        if (Test7Get(k,&v)) {
            if (Test7Del(k)) {
                Thread1Set(k,v);
            }
        }
    }
    /* Ensure the other threads are not still sleeping */
    pthread_cond_broadcast(&testStartCond);

    TIMEINFO("Complete");/* Print time info if enabled */
    return 0;
}
#ifndef MEM_TEST
static char *test2Thread(void *parm)
{
    pthread_mutex_lock(&mutexCond);
    pthread_cond_wait(&testStartCond, &mutexCond);
    pthread_mutex_unlock(&mutexCond);
    TIMEINFO("GO");/* Print time info if enabled */
    while (Test7Count()) {
        int k;
        uint32_t v;
        k=Test7Keys(0);
        Test7Get(k,&v);
        if (Test7Get(k,&v)) {
            if (Test7Del(k)) {
                Thread2Set(k,v);
            }
        }
    }
    /* Ensure the other threads are not still sleeping */
    pthread_cond_broadcast(&testStartCond);

    TIMEINFO("Complete");/* Print time info if enabled */
    return 0;
}
static char *test3Thread(void *parm)
{
    pthread_mutex_lock(&mutexCond);
    pthread_cond_wait(&testStartCond, &mutexCond);
    pthread_mutex_unlock(&mutexCond);
    TIMEINFO("GO");/* Print time info if enabled */
    while (Test7Count()) {
        int k;
        uint32_t v;
        k=Test7Keys(0);
        if (Test7Get(k,&v)) {
            if (Test7Del(k)) {
                Thread3Set(k,v);
            }
        }
    }
    /* Ensure the other threads are not still sleeping */
    pthread_cond_broadcast(&testStartCond);

    TIMEINFO("Complete");/* Print time info if enabled */
    return 0;
}
#endif

static char *testThreadMain(void)
{
    char *ret1=NULL;
    char *ret2=NULL;
    char *ret3=NULL;
    int i;
    TIMEINFO("start");/* Print time info if enabled */

    pthread_create(&test1Handle, NULL, (void*)test1Thread, NULL);
#ifndef MEM_TEST
    pthread_create(&test2Handle, NULL, (void*)test2Thread, NULL);
    pthread_create(&test3Handle, NULL, (void*)test3Thread, NULL);
#endif

    usleep(1000);
    pthread_cond_broadcast(&testStartCond);
    TIMEINFO("Condition sent");/* Print time info if enabled */
    usleep(100);
    pthread_cond_broadcast(&testStartCond);
    usleep(100);
    pthread_cond_broadcast(&testStartCond);

    pthread_join(test1Handle, (void*)&ret1);
#ifndef MEM_TEST
    pthread_join(test2Handle, (void*)&ret2);
    pthread_join(test3Handle, (void*)&ret3);
#endif

    if (ret1) return ret1;
    if (ret2) return ret2;
    if (ret3) return ret3;

    //PRINT("T1: %d T2: %d T3: %d", Thread1Count(), Thread2Count(), Thread3Count());
    for (i=0;i<MAXSIZE;i++) {
        uint32_t v;
        if (Thread1HasKey(i)) {
            mu_assert("Duplicate Key",!Thread2HasKey(i));
            mu_assert("Duplicate Key",!Thread3HasKey(i));
            v=Thread1Val(i);
        } else if (Thread2HasKey(i)) {
            mu_assert("Duplicate Key",!Thread3HasKey(i));
            v=Thread2Val(i);
        } else {
            mu_assert("Missing Key",Thread3HasKey(i));
            v=Thread3Val(i);
        }
        mu_assert("Value mismatch",pyHash((uint8_t*) &i,sizeof(i))==v);
    }
    Test7Free();
    Thread1Free();
    Thread2Free();
    Thread3Free();
    TIMEINFO("Done");/* Print time info if enabled */
        
    return 0; 
}

static char *testCleanup(void)
{
    mu_assert("Test1Hash",!Test1Count());
    mu_assert("Test2Hash",!Test2Count());
    mu_assert("Test3Hash",!Test3Count());
    mu_assert("Test4Hash",!Test4Count());
    mu_assert("Test7Hash",!Test7Count());
    mu_assert("Test8Hash",!Test8Count());
    mu_assert("Thread1Hash",!Thread1Count());
    mu_assert("Thread2Hash",!Thread2Count());
    mu_assert("Thread3Hash",!Thread3Count());
    Test1Free();
    Test2Free();
    Test3Free();
    Thread1Free();
    Thread2Free();
    Thread3Free();
    Test4Free();
    Test5Free();
    Test6Free();
    Test7Free();
    Test8Free();
    return 0;
}

static char * all_tests() {
    TIMEINFO("Start");/* Print time info if enabled */
    DBUG_SW(false);
    mu_run_test(testHashSet);
    mu_run_test(testHashGet);
    mu_run_test(testHashPtr);
    mu_run_test(testHashVal);
    mu_run_test(testHashKeys);
    mu_run_test(testHashIndex);
    mu_run_test(testHashItem);
    mu_run_test(testHashHasKey);
    mu_run_test(testHashDel);
    mu_run_test(testForEach);
    mu_run_test(testHashLoad);
    mu_run_test(testFifo);
    mu_run_test(testHashFree);
    mu_run_test(testNetShare);
    DBUG_SW(false);
    mu_run_test(testLargeHash);
    mu_run_test(testThreadMain);
    mu_run_test(testCleanup);
    TIMEINFO("End");/* Print time info if enabled */
    return 0;
}

int main(int argc, char **argv) {
   char *result;
   double runtime;
   usectime();
   result = all_tests();
   runtime=(double)usectime();
   runtime/=1000000;
   if (result != 0) {
       printf("%s\n", result);
   }
   else {
       printf("ALL TESTS PASSED\n");
   }
   printf("Tests run: %d Time: %0.4lf\n", tests_run,runtime);

   return result != 0;
}

/* Print timing information */
static inline uint64_t usectime()
{
    struct timeval now;
    static __thread uint64_t start=0;
    uint64_t tm=0;

    gettimeofday(&now,NULL);
    tm=(now.tv_sec*1000000 +now.tv_usec);
    if (!start) {
        start=tm;
    }
    return  tm-start;
}
static inline uint64_t usecelapsed()
{
    struct timeval now;
    static __thread struct timeval last={0};
    uint64_t tm;
    gettimeofday(&now,NULL);
    if (now.tv_sec==last.tv_sec) {
        tm=now.tv_usec-last.tv_usec;
        last.tv_usec=now.tv_usec;
    } else {
        if (last.tv_sec) {
            tm=(now.tv_sec*1000000 +now.tv_usec)-(last.tv_sec*1000000 +last.tv_usec);
        } else {
            tm=0;
        }
        last.tv_sec=now.tv_sec;
        last.tv_usec=now.tv_usec;
    }
    return  tm;
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
uint32_t pyHash(uint8_t *a,int s)
{
    int len=s;
    uint8_t *p=a;
    uint32_t x=0;

    x = *p << 7;
    while (--len >= 0)
        x = (1000003*x) ^ *p++;
    x ^= s;
    if (x == -1)
        x = -2;
    return ((uint32_t) (x&0xffffffff));
}

#endif /* UNIT_TEST */
