#include <iostream>
#include "rule.h"
#include "massert.h"

////////////////////////////////////////////////////////////////////////////
//                            RuleAction                                  //
////////////////////////////////////////////////////////////////////////////

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
  return;
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
  if (mAction) {
    std::cout << " ==> ";
    mAction->Dump();
  }
  if (newline) {
    std::cout << std::endl;
  }
  return;
}

////////////////////////////////////////////////////////////////////////////
//                            RuleBase                                    //
////////////////////////////////////////////////////////////////////////////

// Release the RuleElem
RuleBase::~RuleBase() {
  mElement->~RuleElem();
}

void RuleBase::Dump() {
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
  return;
}

