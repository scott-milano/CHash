# CHash

Dynamic Lists/Hashes in C

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
- Binary Lookup: List is sorted and insert/retrival is 26 time faster (run make timetest)
- Linear Lookup: If list order is important and needs to be maintained, build with -DLSEARCH option.
- Network Shared: (In development) List inserts and deletes can be broadcast via multicast
- Key/Value: Types can be simple ordinal types or structures.  String keys are supported by the DEFINE\_HASH() macro.

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



