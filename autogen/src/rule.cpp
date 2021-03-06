/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*
*  http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/
#include <iostream>
#include "rule.h"
#include "massert.h"
#include "base_struct.h"
#include "ruletable.h"  // need MAX_ACT_ELEM_NUM

namespace maplefe {

////////////////////////////////////////////////////////////////////////////
//                            RuleAction                                  //
////////////////////////////////////////////////////////////////////////////

void RuleAction::AddArg(uint8_t idx) {
  if (mArgs.size() >= MAX_ACT_ELEM_NUM) {
    std::cout << "Too many arguments of RuleAction." << std::endl;
    exit(1);
  }
  mArgs.push_back(idx);
}

void RuleAction::Dump() {
  std::cout << mName << "(";
  for (unsigned i = 0; i < mArgs.size(); i++) {
    if (i != 0)
      std::cout << ",";
    std::cout << "%" << (int)mArgs[i];
  }
  std::cout << ")";
}

////////////////////////////////////////////////////////////////////////////
//                            RuleAttr                                    //
////////////////////////////////////////////////////////////////////////////

RuleAttr::~RuleAttr() {
  std::vector<RuleAction*>::iterator it = mValidity.begin();
  for (; it != mValidity.end(); it++) {
    RuleAction *vld = *it;
    if (vld)
      delete vld;
  }
  mValidity.clear();

  it = mAction.begin();
  for (; it != mAction.end(); it++) {
    RuleAction *act = *it;
    if (act)
      delete act;
  }
  mAction.clear();
}

void RuleAttr::DumpValidity(int i) {
  if (mValidity.size()) {
    if (i == 0)
      std::cout << "    attr.validity : ";
    else
      std::cout << "    attr.validity.%" << i << " : ";
    int j = 0;
    for (auto it: mValidity) {
      it->Dump();
      if (++j != mValidity.size())
        std::cout << "; ";
    }
    std::cout << "\n";
  }
}

void RuleAttr::DumpAction(int i) {
  if (mAction.size()) {
    if (i == 0)
      std::cout << "    attr.action : ";
    else
      std::cout << "    attr.action.%" << i << " : ";
    int j = 0;
    for (auto it: mAction) {
      it->Dump();
      if (++j != mAction.size())
        std::cout << "; ";
    }
    std::cout << "\n";
  }
}

////////////////////////////////////////////////////////////////////////////
//                            RuleElem                                    //
////////////////////////////////////////////////////////////////////////////

// [NOTE] Only call Destructor, Not to free memory
// We don't really free the memory since RuleElem is created through plancement new.
// So only call the constructor function
//
RuleElem::~RuleElem() {
  std::vector<RuleElem*>::iterator it = mSubElems.begin();
  for (; it != mSubElems.end(); it++) {
    RuleElem *elem = *it;
    MASSERT(elem && "mSubElems[i] cannot be NULL");
    elem->~RuleElem();
  }
}

const char *RuleElem::GetElemTypeName() {
  switch (mType) {
    case ET_Char:    return "ET_Char";
    case ET_String:  return "ET_String";
    case ET_Rule:    return "ET_Rule";
    case ET_Op:      return "ET_Op";
    case ET_Type:    return "ET_Type";
    case ET_Token:   return "ET_Token";
    case ET_Pending: return "ET_Pending";
    case ET_NULL:    return "ET_NULL";
    default:         MASSERT(0 && "Wrong ElemType");
  }
}

const char *RuleElem::GetRuleOpName() {
  switch (mData.mOp) {
    case RO_Oneof:       return "ONEOF";
    case RO_Zeroormore:  return "ZEROORMORE";
    case RO_Zeroorone:   return "ZEROORONE";
    case RO_Concatenate:
    case RO_Null:
    default:             return "";
  }
}

bool RuleElem::IsLeaf() {
  if (mType == ET_Char ||
      mType == ET_String ||
      mType == ET_Op ||
      mType == ET_Type)
    return true;

  return IsIdentifier();
}

bool RuleElem::IsIdentifier() {
  if (mType != ET_Rule)
    return false;
  if (mData.mRule->mName == "Identifier")
    return true;
  return false;
}

void RuleElem::Dump(bool newline) {
  switch (mType) {
    case ET_Char:
      std::cout << "'" << mData.mChar << "'";
      break;
    case ET_String:
      std::cout << "\"" << mData.mString << "\"";
      break;
    case ET_Rule:
      std::cout << mData.mRule->mName;
      break;
    case ET_Op:
    {
      std::cout << GetRuleOpName();
      if (mData.mOp == RO_Oneof || mData.mOp == RO_Zeroormore || mData.mOp == RO_Zeroorone)
        std::cout << "(";

      std::vector<RuleElem *>::iterator it = mSubElems.begin();
      switch (mData.mOp) {
        case RO_Oneof:
        {
          for (; it != mSubElems.end(); it++) {
            (*it)->Dump();
            if (it+1 != mSubElems.end()) {
              std::cout << ", ";
            }
          }
          break;
        }
        case RO_Concatenate:
        {
          for (; it != mSubElems.end(); it++) {
            (*it)->Dump();
            if (it+1 != mSubElems.end()) {
              std::cout << " + ";
            }
          }
          break;
        }
        case RO_Zeroormore:
        case RO_Zeroorone:
        {
          (*it)->Dump();
          break;
        }
        default:
          break;
      }

      if (mData.mOp == RO_Oneof || mData.mOp == RO_Zeroormore || mData.mOp == RO_Zeroorone)
        std::cout << ")";
      break;
    }
    case ET_Type:
      std::cout << GetTypeString(mData.mTypeId);
      break;
    case ET_Pending:
      std::cout << mData.mString;
      break;
    case ET_Token:
      std::cout << "\"" << mData.mString << "\"";
      break;
    case ET_NULL:
      break;
  }

  if (0 && !mAttr.Empty()) {
    std::cout << " -- ";
    // Dump Attributes
    DumpAttr();
  }

  if (newline) {
    std::cout << std::endl;
  }

  return;
}

void RuleElem::DumpAttr() {
  mAttr.DumpValidity(0);
  for (int i = 0; i < mSubElems.size(); i++) {
    mSubElems[i]->mAttr.DumpValidity(i+1);
  }
  mAttr.DumpAction(0);
  for (int i = 0; i < mSubElems.size(); i++) {
    mSubElems[i]->mAttr.DumpAction(i+1);
  }
}

////////////////////////////////////////////////////////////////////////////
//                              Rule                                      //
////////////////////////////////////////////////////////////////////////////

// Release the RuleElem
Rule::~Rule() {
  mElement->~RuleElem();
}

void Rule::Dump() {
  std::cout << "rule " << mName << " : ";
  if (!mElement) {
    return;
  }

  const std::string opname(mElement->GetRuleOpName());
  if (mElement->mType != ET_Op || opname.length() == 0) {
    mElement->Dump();
    std::cout << std::endl;
    return;
  }

  std::cout << opname;
  if (mElement->mSubElems.size())
    std::cout << "(";
  for (auto it: mElement->mSubElems) {
    it->Dump();
    if (it != mElement->mSubElems.back()) {
      std::cout << ",\n" << std::string(9 + mName.length() + opname.length(), ' ');
    }
  }
  if (mElement->mSubElems.size())
    std::cout << ")" << std::endl;

  if (!mAttr.Empty()) {
    std::cout << " -- ";
    // Dump Attributes
    DumpAttr();
  }

  return;
}

void Rule::DumpAttr() {
  mAttr.DumpValidity(0);
  for (int i = 0; i < mElement->mSubElems.size(); i++) {
    mElement->mSubElems[i]->mAttr.DumpValidity(i+1);
  }
  mAttr.DumpAction(0);
  for (int i = 0; i < mElement->mSubElems.size(); i++) {
    mElement->mSubElems[i]->mAttr.DumpAction(i+1);
  }
}
}


