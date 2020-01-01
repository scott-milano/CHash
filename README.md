# CHash

Dynamic Lists/Hashes in C

**Note:** This library is very lightly tested (with the included unit test) and should not be depended upon in production/critical code.

This library provides dynamic Lists and Hashes in C using macro generated static functions.  This provides basic type checking in the compiler and protects against minor type mismatches.

These types of structures are available in more advanced languages, but missed in C.
This module provides a simple binary sorted list.  This library provides a generic hash that allows for key type and value type to be defined at compile time.

The list can be generated with the macros defined in hash.h

-  DEFINE\_LIST()
-  DEFINE\_HASH()

## Features

This library has the following features currently or in development.

- Thread safe:  List access is mutex protected
- Item Level Mutex: (In development) Lock/Unlock calls for individual entries
- Binary Lookup: List is sorted and insert/retrival is 26 time faster than a linear insert (run make timetest)
- Linear Lookup: If list order is important and needs to be maintained, build with -DLSEARCH option.
- Network Shared: List inserts and deletes can be broadcast via multicast. New joins get updated with latest data.
- Key/Value: Types can be simple ordinal types or structures.  String keys are supported by the DEFINE\_HASH() macro.
- Tested with Valgrind for memory leaks (run make memtest)

## Build

Build with simple: make

    # make
    
Make install copies the output files from .../CHash/src to .../CHash/lib and .../CHash/include for linking.

    # make install
    
Then when building your own executable add .../CHash/lib/libhash.a to the link line, and add -I.../CHash/include/ to CFLAGS

    # gcc mymain.c ~/CHash/lib/libhash.a -I ~/CHash/include -lpthread -o myBinary

Doxgen can be built with:

    # make docs

## Usage

These macros generate the list along with the accesor methods.  Method name prefix is the first parameter passed to the macro. For example:

    DEFINE_LIST(IntFloatList,int,float)

    DEFINE_HASH(AddrHash,in_addr_t)

Entries are then created with:

    IntFloatListSet(1,1.0);
    AddrHashSet("ANY",inet_addr("0.0.0.0");

Retrieval:

    float f;
    in_addr_t addr;

    if (IntFloatListGet(1,&f)) {
        printf("Value Found: %f\n",f);
    }

    AddrHashGet("ANY",&addr);


### ListGet static inline bool LNameGet(List\_k key,List\_v *value);

ListGet Get the value of existing list entry

Parameters:

- key List entry key value
-  value reference to value for return

Returns true on success, false on failure

Get the value referenced by the key:

Example:

    bool ListGet(int key, in_addr_t *value);

### ListSet static inline bool LNameSet(LKeyType key,LValType value)

Add or Set the value of existing list entry.  Sets the value currently referenced, or add new value

Parameters:

- key List entry key value
-  value to create of update

Returns true on success, false on failure

Example:

    void ListSet(int key, in_addr_t value);

### ListVal static inline LValType LNameVal(LKeyType key)

Returns the value as return value.  If value unavailable then a "0" version is returned from List\_zero.  This "zero" is a variable that can be changed.

Parameters:

- key Key value for list return

Returns the value of List entry
Returns the value as return value.  If value unavailable then a "0" version is returned from List\_zero.  This "zero" is a variable that can be changed.

Example:

    in_addr_t ListVal(int key);

### ListPtr static inline LValType *LNamePtr(LKeyType key)

Get a pointer to the value.  Pointer may be broken at next list sort.  This
occurs when a value is added or removed.  Pointer is null if item not 
available:

Parameters:

- key Key value for list return

Return the pointer to a List entry


Example:
    in_addr_t *ListPtr(int key);

### ListCount static inline int LNameCount(void)

Get Number of Items in List. Return size of hash list

Returns Number of items in list

Example:

    int ListCount();

### ListKeys static inline LKeyType LNameKeys(int i)

Returns the key as return value from the sorted list position.  If value
unavailable then the index is wraped on the list. ListKeys(-1) returns 
the last key.

Parameters:

- i List item index (sorted order)

Example:

    int ListKeys(int i);

### ListItem static inline bool LNameItem(int i,LValType *val)
@brief Retrieve item at index i
Allows for grabbing wrapped indexes.  For example ListItem(-1,&value);
returns the last item in the list.  If list is empty, value is not updated.
For wrapped values, or empty list, false is returned.  An in range index
returns true.

Parameters:

- i Index of item (sorted order)

- val Pointer for list item return (output parameter)

Returns true for in range list item placed in val
Returns false if list is empty or the returned item is wrapped

Example:

    int val;
    ListSet(List,0);
    ListSet(List,1);
    ListSet(List,2);
    ListSet(List,3);
    
    ListItem(2,&val);  // Returns true and sets val to 2
    ListItem(4,&val);  // Returns false and sets val to 0
    ListItem(-1,&val); // Returns false and sets val to 3
    ListFree();
    ListItem(2,&val);  // Returns false and does not set val


### ListIndex static inline int LNameIndex(LKeyType key)

Get index of List Key

Parameters:

- key List item key

Returns index of list position
Returns -1 if item is not in list

Example:

    i=ListIndex(int i);

### ListHasKey static inline bool LNameHasKey(LKeyType key)

Bool indidicating if the key is in list.

Parameters:

- key List item key

Returns true if item is in list
Returns false if item is not in list

Example:

    bool ListHasKey(int key);

Returns true if the item is in the list false otherwise

### ListDel static inline bool LNameDel(LKeyType key)

Delete list entry at key

Parameters:

- key List item key

Returns true if item was in list and has been deleted
Returns false if item is not in list

Example:

    bool ListDel(int key);

### ListFree static inline bool LNameFree(void)

Free entire list, and reset to empty working list.  All allocated memory
is freed.  On first use following ListFree, the initial count is allocated.

Returns true if list has been freed without issue
Returns false list was freed but some error occurred

Example:

    void ListFree();

### ListLock static inline bool LNameLock(LKeyType key)\n

static inline bool LNameUnLock(LKeyType key)

Set the mutex lock of a single list entry. 

## Tests

The following tests are included.  The memory test uses valgrind, (sudo apt install valgrind)

### Unit Test

    mypc(user)~/CHash$ make test
    make -C src test
    make[1]: Entering directory '/home/user/CHash/src'
    testLargeHash:979: Load count: 4000000 time: 0.002139 seconds

    ALL TESTS PASSED
    Tests run: 15 Time: 0.0830
    make[1]: Leaving directory '/home/user/CHash/src'
 
 Look for "ALL TESTS PASSED"
 
### Memory Leak Test

    mypc(user)~/CHash$ make memtest
    make -C src memtest
    make[1]: Entering directory '/home/user/CHash/src'
    testLargeHash:979: Load count: 4000000 time: 0.002022 seconds

    ALL TESTS PASSED
    Tests run: 15 Time: 0.0855
    testLargeHash:979: Load count: 4000000 time: 0.002023 seconds

    ALL TESTS PASSED
    Tests run: 15 Time: 0.0845
    ==4601== Memcheck, a memory error detector
    ==4601== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
    ==4601== Using Valgrind-3.13.0 and LibVEX; rerun with -h for copyright info
    ==4601== Command: ./unittest
    ==4601==
    ==4601== error calling PR_SET_PTRACER, vgdb might block
    testLargeHash:979: Load count: 4000000 time: 0.075594 seconds

    ALL TESTS PASSED
    Tests run: 15 Time: 0.6243
    ==4601==
    ==4601== HEAP SUMMARY:
    ==4601==     in use at exit: 0 bytes in 0 blocks
    ==4601==   total heap usage: 32,612 allocs, 32,612 frees, 1,276,805 bytes allocated
    ==4601==
    ==4601== All heap blocks were freed -- no leaks are possible
    ==4601==
    ==4601== For counts of detected and suppressed errors, rerun with: -v
    ==4601== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
    make[1]: Leaving directory '/home/user/CHash/src'

Look for "ALL TESTS PASSED" and "All heap blocks were freed -- no leaks are possible"

### Timing Test

    mypc(user)~/CHash$ make timetest
    make -C src timetest
    make[1]: Entering directory '/home/user/CHash/src'
    gcc -DTIMETEST -DUNIT_TEST -DMAXSIZE=5000 -DTK1=int64_t -DTV1=int64_t -O2 -lpthread mcast.c test.c hash.c -Wall -O2 -lz -lpthread -o unittest && ./unittest
    all_tests(1144) Start: Total: 2, Section: 0
    testLargeHash(895) start: Total: 140682, Section: 140680
    testLargeHash(915) First Set done: Total: 150093, Section: 9410
    testLargeHash(926) Compare done: Total: 153100, Section: 3008
    testLargeHash(942) Delete done: Total: 161112, Section: 8012
    testLargeHash(944) Set allocated List again: Total: 161448, Section: 336
    testLargeHash(955) Set List again: Total: 166884, Section: 5436
    testLargeHash(958) Two Frees full list done: Total: 167683, Section: 799
    testLargeHash(965) Complete: Total: 168973, Section: 1290
    testLargeHash:979: Load count: 25000000 time: 0.009176 seconds

    testThreadMain(1073) start: Total: 179812, Section: 826
    testThreadMain(1081) Condition sent: Total: 182370, Section: 2558
    test2Thread(1022) GO: Total: 0, Section: 0
    test1Thread(1000) GO: Total: 0, Section: 0
    test3Thread(1047) GO: Total: 0, Section: 0
    test2Thread(1039) Complete: Total: 12351, Section: 12351
    test3Thread(1063) Complete: Total: 12301, Section: 12301
    test1Thread(1014) Complete: Total: 12319, Section: 12320
    testThreadMain(1115) Done: Total: 200086, Section: 17716
    all_tests(1162) End: Total: 200356, Section: 270
    ALL TESTS PASSED
    Tests run: 15 Time: 0.4689
    gcc -DUNIT_TEST -DMAXSIZE=5000 -DTK1=int64_t -DTV1=int64_t -O2 -lpthread mcast.c test.c hash.c -Wall -O2 -lz -lpthread -o unittest && ./unittest
    testLargeHash:979: Load count: 54989 time: 0.276119 seconds

    ALL TESTS PASSED
    Tests run: 15 Time: 0.4595
    gcc -DLSEARCH -DUNIT_TEST -DMAXSIZE=5000 -DTK1=int64_t -DTV1=int64_t -O2 -lpthread mcast.c test.c hash.c -Wall -O2 -lz -lpthread -o unittest && ./unittest
    testLargeHash:979: Load count: 54989 time: 5.352992 seconds

    ALL TESTS PASSED
    Tests run: 15 Time: 0.9584
    make[1]: Leaving directory '/home/user/CHash/src'

This test is more for timing information, but it shows for 55,000 loops  the binary insert/search complete in 270ms.  Where the Linear insert/search takes 5353ms.  This is 20ish times faster.
