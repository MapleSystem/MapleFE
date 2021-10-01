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
#ifndef __EXPR_BUFFER_H__
#define __EXPR_BUFFER_H__

#include "mempool.h"

//////////////////////////////////////////////////////////////////////////
//                        ExprBuffer                                    //
// Expression buffer is special since it cannot be broken into multiple //
// lines in generated files. So an ExprBuffer is assigned a memory block//
// of LINES_PER_BLOCK * MAX_LINE_LIMIT bytes.                           //
//                                                                      //
// The hard part of this buffer is how to dump the expression           //
//////////////////////////////////////////////////////////////////////////

namespace maplefe {

#define EXPR_SIZE 256
#define NAME_SIZE 128

// Expression is organized as a tree.

enum OprCode {
  OPC_Add = 0,
  OPC_Sub,
  OPC_Not,
  OPC_LT,  // less than
  OPC_LE,  // less than or equal
  OPC_GT,  // greater than
  OPC_GE,  // greater than or equal
  OPC_Call,// function call
  OPC_Str, // a string
  OPC_Lit, // literal, "literal" enclosed by double quoation
  OPC_Null
};

enum OprType {
  OPT_Uni = 0,
  OPT_Bin,
  OPT_Mult,
  OPT_Null  // string, literal
};

struct OprInfo {
  OprCode mCode;
  OprType mType;
  char   *mText;
};

// Use OprCode as index.
extern OprInfo gOprInfo[OPC_Null];

// 1. The string data will be allocated by the caller of ExprBuffer
//    and will be freed by destructor
// 2. The creation and destroy of ExprNode is handled by ExprBuffer
// 3. Expr is a tree, so we don't allow a node be duplicated in multiple
//    places.

class ExprBuffer;

class ExprNode {
public:
  OprCode   mOpc;
  char      mName[NAME_SIZE]; // for name of function call, string, literal.
  unsigned  mChildNum; // used only for dynamically allocated children nodes.
  ExprNode *mLeft;     // used for two purposes.
                       //  (1) the only node of uni-opr;
                       //  (2) left node of bin-opr;
                       //  (3) the address of node array of multi-opr;
  ExprNode *mRight;

public:
  ExprNode();
  ~ExprNode();

  void SetLeftChild(ExprNode *l) {mLeft = l;}
  void SetRightChild(ExprNode *r) {mRight = r;}
  void SetOpc(OprCode op) {mOpc = op;}
  void SetCall(const char *s);
  void SetString(const char *s);
  void SetLiteral(const char *s);

  bool IsString() {return mOpc == OPC_Str;}
  bool IsLiteral() {return mOpc == OPC_Lit;}

  // the following functions are used for un-determined number of children.
  void AllocChildren(unsigned i, ExprBuffer *buf);
  ExprNode* GetChild(unsigned index);
};

class ExprBuffer {
private:
  char     *mData;
  unsigned  mSize;  // total size of mData
  unsigned  mPos;   // current pos
  ExprNode *mRoot;
  MemPool   mMemPool;

public:

  ExprNode* GetRoot() { return mRoot; }

  // New an array of ExprNode-s
  ExprNode* NewExprNodes(unsigned i);

  void  W2BRecursive(ExprNode*);
  void  WriteStringNode(const ExprNode *n);
  void  WriteLiteralNode(const ExprNode *n);
  void  WriteOperatorNode(const ExprNode *n);
  void  WriteChar(const char c);

  void  Extend(unsigned reqSize);

public:
  ExprBuffer();
  ~ExprBuffer();

  // write the expr to the buffer, mData, in text format
  void Write2Buffer();
  unsigned BufferLength() {return mPos;}
  char *GetData() {return mData;}
};

}

#endif
