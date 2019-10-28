#include <iostream>
#include "rule.h"
#include "massert.h"
#include "base_struct.h"

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
//                            RuleAttr                                    //
////////////////////////////////////////////////////////////////////////////

std::string RuleAttr::GetTokenTypeString(TokenType t) {
  switch (t) {
    case TT_Identifier: return "Identifier";
    case TT_Literal: return "Literal";
    case TT_Type: return "Type";
  }
}

void RuleAttr::DumpDataType(int i) {
  if (mDataType) {
    std::cout << "    attr.datatype : " << mDataType->mName << std::endl;
  }
}

void RuleAttr::DumpTokenType(int i) {
  std::cout << "    attr.tokentype : " << GetTokenTypeString(mTokenType) << std::endl;
}

void RuleAttr::DumpValidity(int i) {
  if (mValidity.size()) {
    if (i == 0)
      std::cout << "    attr.validity : ";
    else
      std::cout << "    attr.validity.%" << i << " : ";
    int i = 0;
    for (auto it: mValidity) {
      it->Dump();
      if (++i != mValidity.size())
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
    int i = 0;
    for (auto it: mAction) {
      it->Dump();
      if (++i != mAction.size())
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

  if (newline) {
    std::cout << std::endl;
  }
  return;
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

  // Dump Attributes
  DumpAttr();

  return;
}

void Rule::DumpAttr() {
  mAttr->DumpDataType(0);
  mAttr->DumpTokenType(0);
  mAttr->DumpValidity(0);
  for (int i = 0; i < mElement->mSubElems.size(); i++) {
    mElement->mSubElems[i]->mAttr->DumpValidity(i);
  }
  mAttr->DumpAction(0);
  for (int i = 0; i < mElement->mSubElems.size(); i++) {
    mElement->mSubElems[i]->mAttr->DumpAction(i);
  }
}

