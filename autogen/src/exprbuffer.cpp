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
#include <cstring>

#include "exprbuffer.h"
#include "massert.h"

namespace maplefe {

// Note: This must be in the order as in 'enum OprCode', and this is the
//       reason I duplicate OprCode here in order to make the code easy
//       to maintain.
OprInfo gOprInfo[OPC_Null] = {
  {OPC_Add, OPT_Bin, "+"},
  {OPC_Sub, OPT_Bin, "-"},
  {OPC_Not, OPT_Uni, "!"},
  {OPC_LT,  OPT_Bin, "<"},
  {OPC_LE,  OPT_Bin, "<="},
  {OPC_GT,  OPT_Bin, ">"},
  {OPC_GE,  OPT_Bin, ">="},
  {OPC_Call,OPT_Mult, ""},  // function call
  {OPC_Str, OPT_Null, ""},  // String
  {OPC_Lit, OPT_Null, ""}   // Literal
};

//////////////////////////////////////////////////////////////////////////
//                        ExprNode                                      //
//////////////////////////////////////////////////////////////////////////
ExprNode::ExprNode(){
  mChildNum = 0;
  mLeft = NULL;
  mRight = NULL;
  memset((void*)(mName), 0, NAME_SIZE);
}

ExprNode::~ExprNode(){
  if (mChildNum > 0) {
    delete [] mLeft;
  } else if (mLeft) {
    delete mLeft;
  } else if (mRight) {
    delete mRight;
  }
}

// This must be called before any AddOneChild()
//
void ExprNode::AllocChildren(unsigned n, ExprBuffer *buf) {
  mChildNum = n;
  mLeft = buf->NewExprNodes(n);
}

// return the index-th child.
ExprNode* ExprNode::GetChild(unsigned index) {
  return mLeft + index;
}

void ExprNode::SetCall(const char *s) {
  unsigned len = strlen(s);
  MASSERT(len < NAME_SIZE);
  strncpy(mName, s, len);
  mOpc = OPC_Call;
}

void ExprNode::SetString(const char *s) {
  unsigned len = strlen(s);
  MASSERT(len < NAME_SIZE);
  strncpy(mName, s, len);
  mOpc = OPC_Str;
}

void ExprNode::SetLiteral(const char *s) {
  unsigned len = strlen(s);
  MASSERT(len < NAME_SIZE);
  strncpy(mName, s, len);
  mOpc = OPC_Lit;
}

//////////////////////////////////////////////////////////////////////////
//                        ExprBuffer                                    //
//////////////////////////////////////////////////////////////////////////

ExprBuffer::ExprBuffer() {
  mSize = EXPR_SIZE;
  mData = (char*)malloc(mSize);
  MASSERT(mData && "Cannot malloc buffer for ExprBuffer");
  mPos = 0;
  mRoot = (ExprNode*)mMemPool.Alloc(sizeof(ExprNode));
  new (mRoot) ExprNode;
}

// Depth-first search to free string memory which is allocated by
// the caller of ExprBuffer.
ExprBuffer::~ExprBuffer() {
  if (mData);
    free(mData);
}

// Extend the size of mData
// newSize is the minimum request of new size.
void ExprBuffer::Extend(unsigned reqSize) {
  unsigned oldSize = mSize;
  unsigned newSize = (reqSize/mSize + 1) * mSize;
  mSize = newSize;
  mData = (char*)realloc(mData, mSize);
  memset((void*)(mData + oldSize), 0, newSize - oldSize);
}

// New an array of ExprNode's
// return the address of first node.
ExprNode* ExprBuffer::NewExprNodes(unsigned num) {
  ExprNode *node = (ExprNode*)mMemPool.Alloc(sizeof(ExprNode) * num);
  ExprNode *p = node;
  for (unsigned i=0; i<num; i++) {
    new (p) ExprNode;
    p++;
  }
  return node;
}

// Depth-first traverse to write the expression to mData
// In-Order
void ExprBuffer::Write2Buffer(){
  MASSERT(mRoot && "Root of expression is NULL!");
  W2BRecursive(mRoot);
}

void ExprBuffer::WriteStringNode(const ExprNode *n){
  int len = strlen(n->mName);
  if (mPos + len > mSize)
    Extend(mPos + len);

  strncpy(mData+mPos, n->mName, len);
  mPos += len;
}

void ExprBuffer::WriteLiteralNode(const ExprNode *n){
  int len = strlen(n->mName);
  if (mPos + len + 2> mSize)
    Extend(mPos + len + 2);

  WriteChar('\"');
  strncpy(mData+mPos, n->mName, len);
  mPos += len;
  WriteChar('\"');
}

void ExprBuffer::WriteOperatorNode(const ExprNode *n){
  int len = 0;
  const char *text = NULL;
  if (n->mOpc == OPC_Call) {
    text = n->mName;
  } else {
    text = gOprInfo[n->mOpc].mText;
  }

  len = strlen(text);
  if (mPos + len > mSize)
    Extend(mPos + len);
  strncpy(mData + mPos, text, len);
  mPos += len;
}

void ExprBuffer::WriteChar(const char c){
  if (mPos + 1 > mSize)
    Extend(mPos + 1);

  *(mData + mPos) = c;
  mPos++;
}

//Recursively W2B of node.
void ExprBuffer::W2BRecursive(ExprNode *n){
  if (n->IsString()) {
    WriteStringNode(n);
  } else if (n->IsLiteral()) {
    WriteLiteralNode(n);
  } else {
    switch (gOprInfo[n->mOpc].mType) {
    case OPT_Uni:
      WriteOperatorNode(n);
      WriteChar('(');
      W2BRecursive(n->mLeft);
      WriteChar(')');
      break;
    case OPT_Bin:
      WriteChar('(');
      if (n->mLeft)
        W2BRecursive(n->mLeft);
      WriteOperatorNode(n);
      if (n->mRight)
        W2BRecursive(n->mRight);
      WriteChar(')');
      break;
    case OPT_Mult:
    {
      WriteOperatorNode(n);
      WriteChar('(');
      ExprNode *child = n->mLeft;
      for (int i=0; i < n->mChildNum - 1; i++) {
        W2BRecursive(child);
        WriteChar(',');
        child++;
      }
      W2BRecursive(child);
      WriteChar(')');
      break;
    }
    default:
      break;
    }
  }
}

}
