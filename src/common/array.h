// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// Define _DEBUG or __ARRAY_DEBUG to enable additional error-state checks. Errors are
// reported through the TRACE_E and TRACE_C macros.
// Define SAFE_ALLOC to remove the code that tests for memory-allocation failures
// (see allochan.*).

// This module must stay independent of the TRACE macros. If TRACE is unavailable we
// provide no-op replacements, so no error reporting is produced in that build.
#if !defined(TRACE_I) && !defined(TRACE_E) && !defined(TRACE_C)
inline void __TraceEmptyFunction() {}
#define TRACE_I(str) __TraceEmptyFunction()
#define TRACE_E(str) __TraceEmptyFunction()
#define TRACE_C(str) (*((int*)NULL) = 0x666)
#endif // TRACE

enum CArrayDirection
{
    drUp,
    drDown
};

enum CDeleteType
{
    dtNoDelete, // do not delete the pointers stored in the indirect array
    dtDelete    // delete the pointers stored in the indirect array
};

enum CErrorType
{
    etNone,         // OK
    etLowMemory,    // new - NULL
    etUnknownIndex, // index is outside the array range
    etBadInsert,    // index of the inserted item is outside the array range
    etBadDispose,   // index of the disposed item is outside the array range
    etDestructed,   // the array has already been destroyed via Destroy()
};

#ifdef TRACE_ENABLE
std::ostream& operator<<(std::ostream& out, const CErrorType& err);
#endif //TRACE_ENABLE

#ifndef __ARRAY_CPP

// ****************************************************************************
// TDirectArray:
//  - behaves like a plain array and can pre-allocate a larger or smaller buffer
//    (see the constructor's 'base' and 'delta' parameters)
//  - adding an item invokes the copy constructor
//  - deleting an item invokes the destructor; override CallDestructor to change
//    this behaviour
//  - suited to simple types and to objects that do not own internal pointers,
//    because:
//      objects are relocated inside the array (for example when reallocating or
//      inserting at the beginning) by a raw memmove, so their constructors and
//      destructors are not triggered; example:
//        char Path[MAX_PATH];  // full file name
//        char* Name;           // points inside 'Path' to the file name (without the path)
//      Workaround: store offsets instead of absolute pointers

template <class DATA_TYPE>
class TDirectArray
{
public:
    CErrorType State; // etNone when the array is healthy; otherwise stores the error
    int Count;        // current number of items in the array

    TDirectArray<DATA_TYPE>(int base, int delta);
    virtual ~TDirectArray() { Destroy(); }

    BOOL IsGood() const { return State == etNone; }
    void ResetState()
    {
        State = etNone;
        TRACE_I("Array state reset (TDirectArray).");
    }

    void Insert(int index, const DATA_TYPE& member);
    int Add(const DATA_TYPE& member); // appends an item and returns its index

    void Insert(int index, const DATA_TYPE* members, int count); // insert 'count' items from 'members'
    int Add(const DATA_TYPE* members, int count);                // append 'count' items from 'members'

    DATA_TYPE& At(int index) // returns a reference to the item at 'index'
    {
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        if (index >= 0 && index < Count)
#endif
            return Data[index];
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        else
        {
            TRACE_C("Index is out of range (index = " << index
                                                      << ", Count = " << Count << ").");
            Error(etUnknownIndex);
            return Data[0]; // the compiler still requires returning something, even if it is invalid
        }
#endif
    }

    DATA_TYPE& operator[](int index) // returns a reference to the item at 'index'
    {
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        if (index >= 0 && index < Count)
#endif
            return Data[index];
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        else
        {
            TRACE_C("Index is out of range (index = " << index
                                                      << ", Count = " << Count << ").");
            Error(etUnknownIndex);
            return Data[0]; // the compiler still requires returning something, even if it is invalid
        }
#endif
    }

    void DetachArray()
    {
        Data = NULL;
        State = etDestructed;
        Count = 0;
        Available = 0;
    }
    DATA_TYPE* GetData() { return Data; }

    void DestroyMembers();             // destroy the stored items but keep the array memory
    void DetachMembers();              // detach all items without running destructors; keep the array
    void Destroy();                    // destroy the items and free the array memory
    void Delete(int index);            // delete the item at 'index' (runs the destructor) and shift the rest
    void Delete(int index, int count); // delete 'count' items at 'index' (runs destructors) and shift the rest
    void Detach(int index);            // detach the item at 'index' without running the destructor; shift the rest
    void Detach(int index, int count); // detach 'count' items at 'index' without running destructors; shift the rest

    int SetDelta(int delta); // change 'Delta' and return the value actually used; NOTE: allowed only on an empty array

protected:
    DATA_TYPE* Data; // pointer to the storage buffer
    int Available;   // number of allocated slots in the array
    int Base;        // minimum number of allocated slots
    int Delta;       // number of slots added or removed when the array grows or shrinks

    virtual void Error(CErrorType err) // handle an error reported by the array
    {
        if (State == etNone)
            State = err;
        else
            TRACE_E("Incorrect call to Error method (State = " << State << ").");
    }
    void EnlargeArray(); // grow the array buffer
    void ReduceArray();  // shrink the array buffer

    void Move(CArrayDirection direction, int first, int count); // shift a block of items forward/backward

    void CallCopyConstructor(DATA_TYPE* placement, const DATA_TYPE& member)
    {
#ifdef new
//#pragma push_macro("new")  // push_macro/pop_macro do not work here: the memory-leak test below
                             // would be reported with an incorrect module and line because we do
                             // not redefine 'new' with a simple macro as MFC does ("#define new DEBUG_NEW")
#define __ARRAY_REDEF_NEW
#undef new
#endif

        ::new (placement) DATA_TYPE(member); // invoke the copy constructor

#ifdef __ARRAY_REDEF_NEW
//#pragma pop_macro("new")  // same issue as above with push_macro/pop_macro and the memory-leak test
#if defined(_DEBUG) && defined(_MSC_VER) // without passing file+line to 'new' operator, list of memory leaks shows only 'crtdbg.h(552)'
#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define __ARRAY_STR2(x) #x
#define __ARRAY_STR(x) __ARRAY_STR2(x)
#pragma message(__FILE__ "(" __ARRAY_STR(__LINE__) "): error: operator new with unknown reason for redefinition")
#endif
#undef __ARRAY_REDEF_NEW
#endif

        //      new int(5);  // memory leak test
    }

    virtual void CallDestructor(DATA_TYPE& member) { member.~DATA_TYPE(); }

private: // guard against calling these legacy helpers
    TDirectArray<DATA_TYPE>() {}
    TDirectArray<DATA_TYPE>(const TDirectArray<DATA_TYPE>&) {}
    TDirectArray<DATA_TYPE>& operator=(TDirectArray<DATA_TYPE>&) { return *this; }

    // The compiler would report an error here; the goal is to trap code written for an older
    // version of TDirectArray. Use CallDestructor instead of Destructor and keep in mind that
    // the new TDirectArray invokes copy constructors and destructors.
    virtual int Destructor(int) { return 0; }
};

// ****************************************************************************
// CArray:
//  - base class for all indirect arrays

class CArray : public TDirectArray<void*>
{
protected:
    CDeleteType DeleteType;

public:
    CArray(int base, int delta, CDeleteType dt) : TDirectArray<void*>(base, delta)
    {
        DeleteType = dt;
    }
};

// ****************************************************************************
// TIndirectArray:
//  - stores pointers to objects (allocated or not)
//  - see CArray and TDirectArray<void*> for additional behaviour shared with this base

template <class DATA_TYPE>
class TIndirectArray : public CArray
{
public:
    TIndirectArray(int base, int delta, CDeleteType dt = dtDelete)
        : CArray(base, delta, dt) {}
    DATA_TYPE*& At(int index)
    {
        return (DATA_TYPE*&)(CArray::operator[](index));
    }

    DATA_TYPE*& operator[](int index)
    {
        return (DATA_TYPE*&)(CArray::operator[](index));
    }

    void Insert(int index, DATA_TYPE* member)
    {
        CArray::Insert(index, (void*)member);
    }

    int Add(DATA_TYPE* member)
    {
        return CArray::Add((void*)member);
    }

    void Insert(int index, DATA_TYPE* const* members, int count)
    {
        CArray::Insert(index, (void**)members, count);
    }

    int Add(DATA_TYPE* const* members, int count)
    {
        return CArray::Add((void**)members, count);
    }

    DATA_TYPE** GetData()
    {
        return (DATA_TYPE**)Data;
    }

    virtual ~TIndirectArray() { Destroy(); }

protected:
    virtual void CallDestructor(void*& member)
    {
        if (DeleteType == dtDelete && (DATA_TYPE*)member != NULL)
            delete ((DATA_TYPE*)member);
    }
};

// ****************************************************************************
// TIndirectClassArray:
//  - stores pointers to objects but keeps their indices fixed
//  - never shifts items in the array—the object stays at its index
//  - reuses gaps left behind by previously released items when inserting a new one

template <class CLASS_TYPE>
class TIndirectClassArray : public TIndirectArray<CLASS_TYPE>
{
public:
    int FirstFreeIndex;

    TIndirectClassArray(int base, int delta, CDeleteType dt = dtDelete)
        : TIndirectArray<CLASS_TYPE>(base, delta, dt) { FirstFreeIndex = 0; }

    int Add(CLASS_TYPE* c);
    void Dispose(int index);

    void DestroyMembers()
    {
        TIndirectArray<CLASS_TYPE>::DestroyMembers();
        FirstFreeIndex = 0;
    }

    void DetachMembers()
    {
        TIndirectArray<CLASS_TYPE>::DetachMembers();
        FirstFreeIndex = 0;
    }

    void Destroy()
    {
        TIndirectArray<CLASS_TYPE>::Destroy();
        FirstFreeIndex = 0;
    }

protected: // block operations that would otherwise move items, etc.
    void Move(CArrayDirection, int, int) {}
    void Insert(int, void*) {}
    void Insert(int, void**, int) {}
    int Add(void*) { return ULONG_MAX; }
    int Add(void**, int) { return ULONG_MAX; }
    void Delete(int) {}
    void DeleteAt(int) {}
    void Detach(int) {}
};

// ****************************************************************************
// TSmallerDirectArray:
//  -see TDirectArray for the general behaviour, but this specialization is not
//   suitable for storing objects (constructors and destructors are not invoked)
//  - shrinks the bookkeeping overhead by limiting the array to 65535 items,
//    using WORD instead of int and omitting the virtual CallDestructor method
//  - well suited as a class member for types that have many instances
//  - memory footprint: 6 B (only +2 B compared to a raw array; TDirectArray uses 25 B)
//  - the smaller footprint comes at the cost of instantiating the template for every
//    combination of 'Base' and 'Delta' and by narrowing the addressable range

template <class DATA_TYPE, WORD Base, WORD Delta> // only 65535 items
class TSmallerDirectArray
{
public:
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    CErrorType State; // records the error state whenever it differs from etNone
#endif
    DATA_TYPE* Data; // pointer to the storage array; kept public instead of using etDestructed
    WORD Count;      // current number of items in the collection

    TSmallerDirectArray<DATA_TYPE, Base, Delta>();
    ~TSmallerDirectArray() { Destroy(); }

    BOOL IsGood() const
    {
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        return State == etNone;
#else
        return TRUE;
#endif
    }

    void ResetState()
    {
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        State = etNone;
        TRACE_I("Array state reset (TSmallerDirectArray).");
#endif
    }

    void Insert(int index, const DATA_TYPE& member);
    inline WORD Add(const DATA_TYPE& member);      // adds an item to the end of the array,
                                                   // returns the item index
    WORD Add(const DATA_TYPE* members, int count); // adds 'count' members items

    DATA_TYPE& At(int index) // returns a reference to the item at the position
    {                        // int used only to silence warnings—the range is up to 65535
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        if (index >= 0 && index < Count)
#endif
            return Data[index];
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        else
        {
            TRACE_C("Index is out of range (index = " << index
                                                      << ", Count = " << Count << ").");
            Error(etUnknownIndex);
            return Data[0]; // the compiler still requires returning something, even if it is invalid
        }
#endif
    }

    DATA_TYPE& operator[](int index) // returns a reference to the item at the position
    {                                // int used only to silence warnings—the range is up to 65535
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        if (index >= 0 && index < Count)
#endif
            return Data[index];
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        else
        {
            TRACE_C("Index is out of range (index = " << index
                                                      << ", Count = " << Count << ").");
            Error(etUnknownIndex);
            return Data[0]; // the compiler still requires returning something, even if it is invalid
        }
#endif
    }

    void DetachArray()
    {
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        State = etDestructed;
#endif
        Data = NULL;
        Count = 0;
    }

    DATA_TYPE* GetData() { return Data; }

    void DestroyMembers();           // destroy the items but keep the storage buffer
    void Destroy();                  // destroy the items and release the storage buffer
    void Delete(int index);          // remove the item at 'index' and shift the rest
    void Reduce(WORD newCount);      // remove items from 'newCount' to the end
    void Delete(WORD from, WORD to); // remove the range <from..to) and shift the rest

protected:
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    virtual void Error(CErrorType err) // handle an error reported by the collection
    {
        if (State == etNone)
            State = err;
        else
            TRACE_E("Incorrect call to Error method (State = " << State << ").");
    }
#endif
    void EnlargeArray(); // grow the array buffer
    void ReduceArray();  // shrink the array buffer

    void Move(CArrayDirection direction, WORD first, WORD count);
    // helper routines for shifting by one element
private: // disable copy construction/assignment
    TSmallerDirectArray<DATA_TYPE, Base, Delta>(const TSmallerDirectArray<DATA_TYPE, Base, Delta>&) {}
    TSmallerDirectArray<DATA_TYPE, Base, Delta>& operator=(TSmallerDirectArray<DATA_TYPE, Base, Delta>&)
    {
        return *this;
    }
};

// ****************************************************************************
// TClassArray:
//  - stores many small objects while keeping their indices stable
//  - allocates CLASS_TYPE objects directly inside the array, invoking their
//    constructors and destructors
//  - never shifts items—the object always stays at its index
//  - an item is valid when (itemIndex < Count && !At(itemIndex).IsEmpty())
//
// requirements for CLASS_TYPE:
//  1) an IsEmpty() method that reports whether the destructor already ran
//  2) a DEFINE_NEW(CLASS_TYPE) macro that provides the placement new operator
//
// example:
//  class CSimpleObject
//  {
//    public:
//      BOOL Destructed;
//
//      CSimpleObject() {Destructed = FALSE;}
//      ~CSimpleObject() {Destructed = TRUE;}
//
//      BOOL IsEmpty() {return Destructed;}
//
//      DEFINE_NEW(CSimpleObject)
//  };
//
//  TClassArray<CSimpleObject> Simples(10, 5);
//
// adding an item to the array:
//  int itemIndex = Simples.FirstFreeIndex;  // first free index in the array
//  new (&Simples)CSimpleObject();   // returns the object address (NULL on error);
//                                   // the new item resides at index itemIndex

template <class CLASS_TYPE>
class TClassArray : public TDirectArray<CLASS_TYPE>
{
public:
    int FirstFreeIndex;

    TClassArray<CLASS_TYPE>(int base, int delta) : TDirectArray<CLASS_TYPE>(base, delta) { FirstFreeIndex = 0; }

    void* GetFreeArraySpace();
    void Dispose(int index);
    virtual void Destructor(int i)
    {
        if (!this->Data[i].IsEmpty())
            this->Data[i].~CLASS_TYPE();
    }

    void DestroyMembers()
    {
        TDirectArray<CLASS_TYPE>::DestroyMembers();
        FirstFreeIndex = 0;
    }

    void Destroy()
    {
        TDirectArray<CLASS_TYPE>::Destroy();
        FirstFreeIndex = 0;
    }

    ~TClassArray() { Destroy(); }

private: // prevent operations that would shift items, etc.
    void Insert(int, const CLASS_TYPE&) {}
    void Insert(int, const CLASS_TYPE*, int) {}
    int Add(const CLASS_TYPE&) { return ULONG_MAX; }
    int Add(const CLASS_TYPE*, int) { return ULONG_MAX; }
    void Delete(int) {}
    void Move(CArrayDirection, int, int) {}
    void DetachMembers() {} // would skip destructor calls
};

#define DEFINE_NEW(CLASS_TYPE) \
    void* operator new(size_t siz) \
    { \
        TRACE_C("Incorrect 'new' operator was called."); \
        return malloc(siz); \
    } \
    void* operator new(size_t, void* p) \
    { \
        return ((TClassArray<CLASS_TYPE>*)p)->GetFreeArraySpace(); \
    }

#ifndef WITHOUT_INDIRECTARRAY

//
// ****************************************************************************
// TIndirectClassArray
//

template <class CLASS_TYPE>
int TIndirectClassArray<CLASS_TYPE>::Add(CLASS_TYPE* c)
{
    if (this->State == etNone)
    {
        if (FirstFreeIndex == this->Count)
        {
            TIndirectArray<CLASS_TYPE>::Add(c);
            if (this->IsGood())
            {
                FirstFreeIndex = this->Count;
                return this->Count - 1;
            }
        }
        else
        {
            int ret = FirstFreeIndex;
            this->At(FirstFreeIndex) = c;
            int i;
            for (i = FirstFreeIndex + 1; i < this->Count; i++)
                if (this->At(i) == NULL)
                    break;
            FirstFreeIndex = i;
            return ret;
        }
    }
    else
        TRACE_E("Incorrect call to array method (State = " << this->State << ").");
    return ULONG_MAX;
}

template <class CLASS_TYPE>
void TIndirectClassArray<CLASS_TYPE>::Dispose(int index)
{
    if (this->State == etNone)
    {
        if (index < 0 || index >= this->Count)
        {
            TRACE_C("Attempt to dispose item out of array range (index = " << index << ", Count = " << this->Count << ").");
            this->Error(etBadDispose);
            return;
        }
        Destructor(this->At(index));
        this->At(index) = NULL;
        if (this->Count == index + 1)
        {
            if (--(this->Count) > 0)
                for (int i = this->Count; 0 < i; i--)
                    if (this->At(i - 1) == NULL)
                        this->Count--;
                    else
                        break;

            while (this->IsGood() && this->Available > this->Base && this->Available - this->Delta >= this->Count)
                this->DestroyArray();

            if (FirstFreeIndex > this->Count)
                FirstFreeIndex = this->Count;
        }
        else if (FirstFreeIndex > index)
            FirstFreeIndex = index;
    }
    else
        TRACE_E("Incorrect call to array method (State = " << this->State << ").");
}

#endif

//
// ****************************************************************************
// TClassArray
//

template <class CLASS_TYPE>
void* TClassArray<CLASS_TYPE>::GetFreeArraySpace()
{
    if (this->State == etNone)
    {
        if (FirstFreeIndex == this->Count)
        {
            if (this->Count == this->Available)
                this->EnlargeArray();
            if (this->State == etNone)
            {
                FirstFreeIndex = this->Count + 1;
                return this->Data + this->Count++;
            }
        }
        else
        {
            void* ret = this->Data + FirstFreeIndex;
            int i;
            for (i = FirstFreeIndex + 1; i < this->Count; i++)
                if (this->Data[i].IsEmpty())
                    break;
            FirstFreeIndex = i;
            return ret;
        }
    }
    else
        TRACE_E("Incorrect call to array method (State = " << this->State << ").");
    return NULL;
}

template <class CLASS_TYPE>
void TClassArray<CLASS_TYPE>::Dispose(int index)
{
    if (this->State == etNone)
    {
        if (index < 0 || index >= this->Count)
        {
            TRACE_C("Attempt to dispose item out of array range (index = " << index << ", Count = " << this->Count << ").");
            this->Error(etBadDispose);
            return;
        }
        Destructor(index);
        if (this->Count == index + 1)
        {
            if (--(this->Count) > 0)
                for (int i = this->Count; 0 < i; i--)
                    if (this->Data[i - 1].IsEmpty())
                        this->Count--;
                    else
                        break;

            while (this->IsGood() && this->Available > this->Base && this->Available - this->Delta >= this->Count)
                this->ReduceArray();

            if (FirstFreeIndex > this->Count)
                FirstFreeIndex = this->Count;
        }
        else if (FirstFreeIndex > index)
            FirstFreeIndex = index;
    }
    else
        TRACE_E("Incorrect call to array method (State = " << this->State << ").");
}

//
// ****************************************************************************
// TDirectArray
//

template <class DATA_TYPE>
TDirectArray<DATA_TYPE>::TDirectArray(int base, int delta)
{
    if (base <= 0)
        TRACE_E("Base is less or equal to zero, correcting to 1.");
    Base = (base > 0) ? base : 1;
    if (delta <= 0)
        TRACE_E("Delta is less or equal to zero, correcting to 1.");
    Delta = (delta > 0) ? delta : 1;
    State = etNone;
    Available = Count = 0;
    Data = (DATA_TYPE*)malloc(Base * sizeof(DATA_TYPE));
#ifndef SAFE_ALLOC
    if (Data == NULL)
    {
        TRACE_E("Out of memory.");
        Error(etLowMemory);
        return;
    }
#endif // SAFE_ALLOC
    Available = Base;
}

template <class DATA_TYPE>
void TDirectArray<DATA_TYPE>::Destroy()
{
    if (State == etNone) // it can be also etDestructed
    {
        if (Data != NULL)
        {
            for (int i = 0; i < Count; i++)
                CallDestructor(Data[i]);
            free(Data);
            Data = NULL;
            State = etDestructed;
        }
    }
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    else if (State != etDestructed)
        TRACE_E("Incorrect call to array method (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
void TDirectArray<DATA_TYPE>::Insert(int index, const DATA_TYPE& member)
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
        if (Count == Available && &member >= Data && &member < Data + Count)
        {
            TRACE_C("Inserted item could become invalid during operation.");
            return;
        }
        if (Count < Available &&
            &member >= Data + index + 1 && &member < Data + Count + 1)
            TRACE_C("Inserted item will change value during operation.");
#endif
        if (index >= 0 && index <= Count)
        {
            Move(drDown, index, Count - index);
            if (State == etNone)
            {
                Count++;
                CallCopyConstructor(&At(index), member);
            }
        }
        else
        {
            TRACE_C("Attempt to insert item out of array range.");
            State = etBadInsert;
        }
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    }
    else
        TRACE_E("Incorrect call to array method (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
int TDirectArray<DATA_TYPE>::Add(const DATA_TYPE& member)
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
        if (Count == Available && &member >= Data && &member < Data + Count)
        {
            TRACE_C("Added item could become invalid during operation.");
            return ULONG_MAX;
        }
#endif
        if (Count == Available)
            EnlargeArray();
        if (State == etNone)
        {
            Count++;
            CallCopyConstructor(&At(Count - 1), member);
            return Count - 1;
        }
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    }
    else
        TRACE_E("Incorrect call to array method (State = " << State << ").");
#endif
    return ULONG_MAX;
}

template <class DATA_TYPE>
void TDirectArray<DATA_TYPE>::Insert(int index, const DATA_TYPE* members, int count)
{
    if (count <= 0)
        return; // nothing to do
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
        if (Count + count > Available && members + count - 1 >= Data && // array will be reallocated and inserted items are from this array (items will be released after array reallocation)
            members < Data + Count)
        {
            TRACE_C("Inserted items could become invalid during operation.");
            return;
        }
        if (Count + count <= Available &&                  // if array will not be reallocated
            members + count - 1 >= Data + index + count && // and if inserted part is from memory where array will be enlarged
            members < Data + Count + count)
        {
            TRACE_C("Inserted items will change value during operation.");
        }
#endif
        if (index >= 0 && index <= Count)
        {
            int needed = Count + count;
            if (needed > Available)
            {
                needed -= Base + 1;
                needed = needed - (needed % Delta) + Delta + Base;
                DATA_TYPE* newData = (DATA_TYPE*)realloc(Data, needed * sizeof(DATA_TYPE));
#ifndef SAFE_ALLOC
                if (newData == NULL)
                {
                    TRACE_E("Low memory for array enlargement.");
                    Error(etLowMemory);
                    return;
                }
#endif // SAFE_ALLOC
                Available = needed;
                Data = newData;
            }
            memmove(Data + index + count, Data + index, (Count - index) * sizeof(DATA_TYPE));

            DATA_TYPE* placement = Data + index;
            for (int i = 0; i < count; i++)
                CallCopyConstructor(placement + i, members[i]);

            Count += count;
        }
        else
        {
            TRACE_C("Attempt to insert item out of array range.");
            State = etBadInsert;
        }
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    }
    else
        TRACE_E("Incorrect call to array method (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
int TDirectArray<DATA_TYPE>::Add(const DATA_TYPE* members, int count)
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
        if (Count + count > Available && members + count - 1 >= Data &&
            members < Data + Count)
        {
            TRACE_C("Added items could become invalid during operation.");
            return ULONG_MAX;
        }
#endif
        int needed = Count + count;
        if (needed > Available)
        {
            needed -= Base + 1;
            needed = needed - (needed % Delta) + Delta + Base;
            DATA_TYPE* newData = (DATA_TYPE*)realloc(Data, needed * sizeof(DATA_TYPE));
#ifndef SAFE_ALLOC
            if (newData == NULL)
            {
                TRACE_E("Low memory for array enlargement.");
                Error(etLowMemory);
                return ULONG_MAX;
            }
#endif // SAFE_ALLOC
            Available = needed;
            Data = newData;
        }

        DATA_TYPE* placement = Data + Count;
        for (int i = 0; i < count; i++)
            CallCopyConstructor(placement + i, members[i]);

        Count += count;
        return Count - count;
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    }
    return ULONG_MAX;
#endif
}

template <class DATA_TYPE>
void TDirectArray<DATA_TYPE>::DestroyMembers()
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
#endif
        if (Count > 0)
            for (int i = 0; i < Count; i++)
                CallDestructor(Data[i]);
        else
            return;
        Count = 0;
        if (Available == Base)
            return;

        DATA_TYPE* newData = (DATA_TYPE*)realloc(Data, Base * sizeof(DATA_TYPE));
#ifndef SAFE_ALLOC
        if (newData == NULL)
        {
            TRACE_E("Low memory for operations related to item destruction.");
            Error(etLowMemory);
            return;
        }
#endif // SAFE_ALLOC
        Available = Base;
        Data = newData;
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    }
#endif
}

template <class DATA_TYPE>
void TDirectArray<DATA_TYPE>::DetachMembers()
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
#endif
        if (Count == 0)
            return;
        Count = 0;
        if (Available == Base)
            return;

        DATA_TYPE* newData = (DATA_TYPE*)realloc(Data, Base * sizeof(DATA_TYPE));
#ifndef SAFE_ALLOC
        if (newData == NULL)
        {
            TRACE_E("Low memory for operations related to item destruction.");
            Error(etLowMemory);
            return;
        }
#endif // SAFE_ALLOC
        Available = Base;
        Data = newData;
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    }
#endif
}

template <class DATA_TYPE>
void TDirectArray<DATA_TYPE>::Delete(int index)
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
        if (index >= 0 && index < Count)
        {
#endif
            CallDestructor(Data[index]);
            Move(drUp, index + 1, Count - index - 1);
            Count--;
            if (Available > Base && Available - Delta == Count)
                ReduceArray();
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        }
        else
            TRACE_C("Attempt to delete item out of array range. index = " << index << ", count = " << Count);
    }
    else
        TRACE_E("Incorrect call to array method (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
void TDirectArray<DATA_TYPE>::Delete(int index, int count)
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
        if (index >= 0 && index + count - 1 < Count)
        {
#endif
            for (int i = index; i < index + count; i++)
                CallDestructor(Data[i]);
            memmove(Data + index, Data + index + count, (Count - count - index) * sizeof(DATA_TYPE));
            Count -= count;
            if (Available > Base && Available - Delta >= Count)
            {
                int a = (Count <= Base) ? Base : Base + Delta * ((Count - Base - 1) / Delta + 1);
                DATA_TYPE* New = (DATA_TYPE*)realloc(Data, a * sizeof(DATA_TYPE));
#ifndef SAFE_ALLOC
                if (New == NULL)
                {
                    TRACE_E("Low memory for operations related to array reducing.");
                    Error(etLowMemory);
                }
                else
                {
#endif // SAFE_ALLOC
                    Data = New;
                    Available = a;
#ifndef SAFE_ALLOC
                }
#endif // SAFE_ALLOC
            }
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        }
        else
            TRACE_C("Attempt to delete item out of array range. index = " << index << ", count = " << count << ", Count = " << Count);
    }
    else
        TRACE_E("Incorrect call to array method (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
void TDirectArray<DATA_TYPE>::Detach(int index)
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
        if (index >= 0 && index < Count)
        {
#endif
            Move(drUp, index + 1, Count - index - 1);
            Count--;
            if (Available > Base && Available - Delta == Count)
                ReduceArray();
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        }
        else
            TRACE_C("Attempt to detach item out of array range. index = " << index << ", count = " << Count);
    }
    else
        TRACE_E("Incorrect call to array method (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
void TDirectArray<DATA_TYPE>::Detach(int index, int count)
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
        if (index >= 0 && index + count - 1 < Count)
        {
#endif
            memmove(Data + index, Data + index + count, (Count - count - index) * sizeof(DATA_TYPE));
            Count -= count;
            if (Available > Base && Available - Delta >= Count)
            {
                int a = (Count <= Base) ? Base : Base + Delta * ((Count - Base - 1) / Delta + 1);
                DATA_TYPE* New = (DATA_TYPE*)realloc(Data, a * sizeof(DATA_TYPE));
#ifndef SAFE_ALLOC
                if (New == NULL)
                {
                    TRACE_E("Low memory for operations related to array reducing.");
                    Error(etLowMemory);
                }
                else
                {
#endif // SAFE_ALLOC
                    Data = New;
                    Available = a;
#ifndef SAFE_ALLOC
                }
#endif // SAFE_ALLOC
            }
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        }
        else
            TRACE_C("Attempt to detach item out of array range. index = " << index << ", count = " << count << ", Count = " << Count);
    }
    else
        TRACE_E("Incorrect call to array method (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
int TDirectArray<DATA_TYPE>::SetDelta(int delta)
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
#endif // defined(_DEBUG) || defined(__ARRAY_DEBUG)
        if (Count == 0)
        {
            if (delta > 0 && delta > Delta)
                Delta = delta;
        }
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        else
            TRACE_E("SetDelta() may be used only for empty array. (Count = " << Count << ").");
    }
    else
        TRACE_E("Incorrect call to array method (State = " << State << ").");
#endif // defined(_DEBUG) || defined(__ARRAY_DEBUG)
    return Delta;
}

template <class DATA_TYPE>
void TDirectArray<DATA_TYPE>::EnlargeArray()
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
#endif
        DATA_TYPE* New = (DATA_TYPE*)realloc(Data, (Available + Delta) * sizeof(DATA_TYPE));
#ifndef SAFE_ALLOC
        if (New == NULL)
        {
            TRACE_E("Low memory for array enlargement.");
            Error(etLowMemory);
            return;
        }
#endif // SAFE_ALLOC
        Data = New;
        Available += Delta;
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    }
    else
        TRACE_E("Incorrect call to array method (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
void TDirectArray<DATA_TYPE>::ReduceArray()
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
#endif
        DATA_TYPE* New = (DATA_TYPE*)realloc(Data, (Available - Delta) * sizeof(DATA_TYPE));
#ifndef SAFE_ALLOC
        if (New == NULL)
        {
            TRACE_E("Low memory for operations related to array reducing.");
            Error(etLowMemory);
            return;
        }
#endif // SAFE_ALLOC
        Data = New;
        Available -= Delta;
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    }
    else
        TRACE_E("Incorrect call to array method (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
void TDirectArray<DATA_TYPE>::Move(CArrayDirection direction, int first, int count)
{
    if (count == 0)
    {
        if (direction == drDown && Available == Count)
            EnlargeArray();
        return;
    }
    if (direction == drDown)
    {
        if (Available == Count)
            EnlargeArray();
        if (State == etNone)
            memmove(Data + first + 1, Data + first, count * sizeof(DATA_TYPE));
    }
    else // Up
        memmove(Data + first - 1, Data + first, count * sizeof(DATA_TYPE));
}

//
// ****************************************************************************
// TSmallerDirectArray
//

template <class DATA_TYPE, WORD Base, WORD Delta>
TSmallerDirectArray<DATA_TYPE, Base, Delta>::TSmallerDirectArray()
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    State = etNone;
#endif
    Count = 0;
    Data = (DATA_TYPE*)malloc(Base * sizeof(DATA_TYPE));
#ifndef SAFE_ALLOC
    if (Data == NULL)
    {
        TRACE_E("Out of memory.");
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        Error(etLowMemory);
#endif
        return;
    }
#endif // SAFE_ALLOC
}

template <class DATA_TYPE, WORD Base, WORD Delta>
void TSmallerDirectArray<DATA_TYPE, Base, Delta>::Destroy()
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone) // etDestructed can also occur
    {
#endif
        if (Data != NULL)
        {
            //      for (WORD i = 0; i < Count; i++)
            //        Destructor(i);
            free(Data);
            Data = NULL;
            Count = 0;
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
            State = etDestructed;
#endif
        }
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    }
    else if (State != etDestructed)
        TRACE_E("Incorrect call to array method (State = " << State << ").");
#endif
}

template <class DATA_TYPE, WORD Base, WORD Delta>
void TSmallerDirectArray<DATA_TYPE, Base, Delta>::Insert(int index, const DATA_TYPE& member)
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
        if (Count >= Base && ((Count - Base) % Delta) == 0 &&
            &member >= Data && &member < Data + Count)
        {
            TRACE_C("Inserted item could become invalid during operation.");
            return;
        }
        if ((Count < Base || ((Count - Base) % Delta) != 0) &&
            &member >= Data + index + 1 && &member < Data + Count + 1)
            TRACE_C("Inserted item will change value during operation.");
#endif
        Move(drDown, (short int)index, (short int)(Count - index));
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        if (State == etNone)
        {
#endif
            Count++;
            At(index) = member;
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        }
    }
    else
        TRACE_E("Incorrect call to array method (State = " << State << ").");
#endif
}

template <class DATA_TYPE, WORD Base, WORD Delta>
WORD TSmallerDirectArray<DATA_TYPE, Base, Delta>::Add(const DATA_TYPE& member)
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
        if (Count >= Base && ((Count - Base) % Delta) == 0 &&
            &member >= Data && &member < Data + Count)
        {
            TRACE_C("Added item could become invalid during operation.");
            return USHRT_MAX;
        }
#endif
        if (Count >= Base && ((Count - Base) % Delta) == 0)
            EnlargeArray();
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        if (State == etNone)
        {
#endif
            Count++;
            At(Count - 1) = member;
            return (short int)(Count - 1);
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        }
    }
    else
        TRACE_E("Incorrect call to array method (State = " << State << ").");
    return USHRT_MAX;
#endif
}

template <class DATA_TYPE, WORD Base, WORD Delta>
WORD TSmallerDirectArray<DATA_TYPE, Base, Delta>::Add(const DATA_TYPE* members, int count)
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
#endif
        int needed = Count + count;
        int available;
        if (Count > Base)
        {
            available = Count - Base - 1;
            available = available - (available % Delta) + Delta + Base;
        }
        else
            available = Base;

        if (needed > available)
        {
            needed -= Base + 1;
            needed = needed - (needed % Delta) + Delta + Base;
            DATA_TYPE* newData = (DATA_TYPE*)realloc(Data, needed * sizeof(DATA_TYPE));
#ifndef SAFE_ALLOC
            if (newData == NULL)
            {
                TRACE_E("Low memory for array enlargement.");
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
                Error(etLowMemory);
#endif
                return USHRT_MAX;
            }
#endif // SAFE_ALLOC
            Data = newData;
        }
        memmove(Data + Count, members, count * sizeof(DATA_TYPE));
        Count += (WORD)count;
        return (WORD)(Count - count);
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    }
    return USHRT_MAX;
#endif
}

template <class DATA_TYPE, WORD Base, WORD Delta>
void TSmallerDirectArray<DATA_TYPE, Base, Delta>::DestroyMembers()
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
#endif
        if (Count > 0)
            ; // for (WORD i = 0; i < Count; i++) Destructor(i);
        else
            return;
        if (Count <= Base)
        {
            Count = 0;
            return;
        }
        Count = 0;

        DATA_TYPE* newData = (DATA_TYPE*)realloc(Data, Base * sizeof(DATA_TYPE));
#ifndef SAFE_ALLOC
        if (newData == NULL)
        {
            TRACE_E("Low memory for operations related to item destruction.");
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
            Error(etLowMemory);
#endif
            return;
        }
#endif // SAFE_ALLOC
        Data = newData;
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    }
#endif
}

template <class DATA_TYPE, WORD Base, WORD Delta>
void // int used only because of warnings - range up to 65535
TSmallerDirectArray<DATA_TYPE, Base, Delta>::Delete(int index)
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
        if (index >= 0 && index < Count)
        {
#endif
            //      Destructor((short int)index);
            Move(drUp, (short int)(index + 1), (short int)(Count - index - 1));
            Count--;
            if (Count >= Base && ((Count - Base) % Delta) == 0)
                ReduceArray();
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        }
        else
            TRACE_C("Attempt to delete item out of array range. index = " << index << ", count = " << Count);
    }
    else
        TRACE_E("Incorrect call to array method (State = " << State << ").");
#endif
}

template <class DATA_TYPE, WORD Base, WORD Delta>
void TSmallerDirectArray<DATA_TYPE, Base, Delta>::Reduce(WORD newCount)
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
#endif
        if (newCount < Count)
        {
            int needed;
            if (newCount > Base)
                needed = Base + (newCount - Base - 1) - ((newCount - Base - 1) % Delta) + Delta;
            else
                needed = Base;

            DATA_TYPE* New = (DATA_TYPE*)realloc(Data, needed * sizeof(DATA_TYPE));
#ifndef SAFE_ALLOC
            if (New == NULL)
            {
                TRACE_E("Low memory for array enlargement.");
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
                Error(etLowMemory);
#endif
                return;
            }
#endif // SAFE_ALLOC
            Data = New;
            Count = newCount;
        }
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    }
#endif
}

template <class DATA_TYPE, WORD Base, WORD Delta>
void TSmallerDirectArray<DATA_TYPE, Base, Delta>::Delete(WORD from, WORD to)
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
        if (from < 0 || from > to || to > Count)
        {
            TRACE_C("Incorrect call to Delete method (from=" << from << ", to=" << to << ", count = " << Count << ").");
        }
        else
        {
#endif
            memmove(Data + from, Data + to, (Count - to) * sizeof(DATA_TYPE));
            Reduce((WORD)(Count - (to - from)));
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        }
    }
#endif
}

template <class DATA_TYPE, WORD Base, WORD Delta>
void TSmallerDirectArray<DATA_TYPE, Base, Delta>::EnlargeArray()
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
        if (Count < Base || ((Count - Base) % Delta) != 0)
            TRACE_C("Incorrect call to EnlargeArray: some allocated items are not used.");
#endif
        DATA_TYPE* New = (DATA_TYPE*)realloc(Data, (Count + Delta) * sizeof(DATA_TYPE));
#ifndef SAFE_ALLOC
        if (New == NULL)
        {
            TRACE_E("Low memory for array enlargement.");
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
            Error(etLowMemory);
#endif
            return;
        }
#endif // SAFE_ALLOC
        Data = New;
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    }
    else
        TRACE_E("Incorrect call to array method (State = " << State << ").");
#endif
}

template <class DATA_TYPE, WORD Base, WORD Delta>
void TSmallerDirectArray<DATA_TYPE, Base, Delta>::ReduceArray()
{
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    if (State == etNone)
    {
        if (Count < Base || ((Count - Base) % Delta) != 0)
            TRACE_C("Incorrect call to ReduceArray: number of unused allocated items is not divisible by Delta.");
#endif
        DATA_TYPE* New = (DATA_TYPE*)realloc(Data, Count * sizeof(DATA_TYPE));
#ifndef SAFE_ALLOC
        if (New == NULL)
        {
            TRACE_E("Low memory for operations related to array reducing.");
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
            Error(etLowMemory);
#endif
            return;
        }
#endif // SAFE_ALLOC
        Data = New;
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
    }
    else
        TRACE_E("Incorrect call to array method (State = " << State << ").");
#endif
}

template <class DATA_TYPE, WORD Base, WORD Delta>
void TSmallerDirectArray<DATA_TYPE, Base, Delta>::Move(CArrayDirection direction, WORD first, WORD count)
{
    if (count == 0)
    {
        if (direction == drDown && Count >= Base && ((Count - Base) % Delta) == 0)
            EnlargeArray();
        return;
    }
    if (direction == drDown)
    {
        if (Count >= Base && ((Count - Base) % Delta) == 0)
            EnlargeArray();
#if defined(_DEBUG) || defined(__ARRAY_DEBUG)
        if (State == etNone)
#endif
            memmove(Data + first + 1, Data + first, count * sizeof(DATA_TYPE));
    }
    else // Up
        memmove(Data + first - 1, Data + first, count * sizeof(DATA_TYPE));
}

#endif // __ARRAY_CPP
