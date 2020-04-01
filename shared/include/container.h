/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v1.
* You can use this software according to the terms and conditions of the Mulan PSL v1.
* You may obtain a copy of Mulan PSL v1 at:
*
*  http://license.coscl.org.cn/MulanPSL
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v1 for more details.
*/

///////////////////////////////////////////////////////////////////////////////
// This file contains the popular container for most data structures.
//
// [NOTE] For containers, there is one basic rule that is THE CONTAINTER ITSELF
//        SHOULD BE SELF CONTAINED. In another word, all the memory should be
//        contained in the container itself. Once the destructor or Release()
//        is called, all memory should be freed.
//
// The user of containers need explictly call Release() to free
// the memory, if he won't trigger the destructor of the containers. This
// is actually common case, like tree nodes which is maintained by a memory
// pool and won't call destructor, so any memory allocated at runtime won't
// be released.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include "mempool.h"
#include "massert.h"

// We define a ContainerMemPool, which is slightly different than the other MemPool.
// There are two major differences.
// (1) Size of each allocation will be the same.
// (2) Support indexing.

class ContainerMemPool : public MemPool {
public:
  unsigned mElemSize;
public:
  char* AddrOfIndex(unsigned index);
  void  SetElemSize(unsigned i) {mElemSize = i;}
  char* AllocElem() {return Alloc(mElemSize);}
};

// SmallVector is widely used in the tree nodes to save children nodes, and
// they should call Release() explicitly, if the destructor of SmallVector
// won't be called.

// NOTE: When we locate an element in the memory pool, we don't check if it's
//       out of boundary. It's the user of SmallVector to ensure element index
//       is valid.

template <class T> class SmallVector {
private:
  ContainerMemPool mMemPool;
  unsigned         mNum;     // element number

public:
  SmallVector() {
    mNum = 0;
    SetBlockSize(128);
    mMemPool.SetElemSize(sizeof(T));
  }
  ~SmallVector() {Release();}

  void SetBlockSize(unsigned i) {mMemPool.SetBlockSize(i);}
  void Release() {mMemPool.Release();}

  void PushBack(T t) {
    char *addr = mMemPool.AllocElem();
    *(T*)addr = t;
    mNum++;
  }

  void PopBack(T);

  unsigned GetNum() {return mNum;}

  T Back();

  T ValueAtIndex(unsigned i) {
    char *addr = mMemPool.AddrOfIndex(i);
    return *(T*)addr;
  }

  T* RefAtIndex(unsigned i) {
    char *addr = mMemPool.AddrOfIndex(i);
    return (T*)addr;
  }
};

/////////////////////////////////////////////////////////////////////////
//                      Guamian
// Guamian, aka Hanging Noodle, represents a 2-D data structure shown below.
//
//  --K--->K--->K--->K-->
//    |    |    |    |
//    E    E    E    E
//    |    |         |
//    E    E         E
//         |
//         E
// The horizontal bar is a one directional linked list. It's like the stick
// of Guamian. Each vertical list is like one noodle string, which is also
// one direction linked list. The node on the stick is called (K)nob, the node
// on the noodle string is called (E)lement.
//
// None of the two lists are sorted since our target scenarios are usually
// at small scope.
//
// Duplication of knobs or elements is not supported in Guamian.
/////////////////////////////////////////////////////////////////////////

template <class K, class E> class Guamian {
private:
  struct Elem{
    E     mData;
    Elem *mNext;
  };
  struct Knob{
    K     mData;
    Knob *mNext;
    Elem *mChildren; // pointing to the first child
  };

private:
  MemPool mMemPool;
  Knob   *mHeader;

  // allocate a new knob
  Knob* NewKnob() {
    Knob *knob = (Knob*)mMemPool.Alloc(sizeof(Knob));
    knob->mNext = NULL;
    return knob;
  }

  // allocate a new element
  Elem* NewElem() {
    Elem *elem = (Elem*)mMemPool.Alloc(sizeof(Elem));
    elem->mNext = NULL;
    return elem;
  }

public:
  Guamian() {mHeader = NULL;}
  ~Guamian(){Release();}

  void AddElem(K key, E data) {
    Knob *knob = FindOrCreateKnob(key);
    Elem *elem = knob->mChildren;
    Elem *found = NULL;
    while (elem) {
      if (elem->mData == data) {
        found = elem;
        std::cout << "duplicate element" << std::endl;
        break;
      }
      elem = elem->mNext;
    }

    if (!found) {
      std::cout << "new element" << std::endl;
      Elem *e = NewElem();
      e->mData = data;
      e->mNext = knob->mChildren;
      knob->mChildren = e;
    }
  }

  // Try to find the first child of Knob k. Return the data.
  // found is set to false if fails, or true.
  // [NOTE] It's the user's responsibilty to make sure the Knob
  //        of 'key' exists.
  E FindFirstElem(K key, bool &found) {
    Knob *knob = FindKnob(key);
    if (!knob) {
      found = false;
      return 0;   // return value doesn't matter when fails.
    }

    Elem *e = knob->mChildren;
    if (!e) {
      found = false;
      return 0;
    }

    found = true;
    return e->mData;
  }

  // Just try to find the Knob.
  // return NULL if fails.
  Knob* FindKnob(K key) {
    Knob *result = NULL;
    Knob *knob = mHeader;
    while (knob) {
      if (knob->mData == key) {
        result = knob;
        break;
      }
      knob = knob->mNext;
    }
    return result;
  }

  // Try to find the Knob. Create one if failed
  // and add it to the list.
  Knob* FindOrCreateKnob(K key) {
    // Find the knob. If cannot find, create a new one
    // We always put the new one as the new header.
    Knob *knob = FindKnob(key);
    if (!knob) {
      knob = NewKnob();
      knob->mNext = mHeader;
      knob->mData = key;
      mHeader = knob;
    }
    return knob;
  }

  void Release() {mMemPool.Release();}
};

#endif
