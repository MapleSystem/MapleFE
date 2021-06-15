/*
* Copyright (C) [2021] Futurewei Technologies, Inc. All rights reverved.
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

#include <stack>
#include <set>
#include "ast_handler.h"
#include "ast_ti.h"

#define NOTYETIMPL(K)      { if (mTrace) { MNYI(K);      }}

namespace maplefe {

void TypeInfer::TypeInference(AstFunction *func) {
  if (mTrace) std::cout << "============== TypeInfer ==============" << std::endl;
  TypeInferVisitor visitor(mHandler, mTrace, true);

  std::unordered_set<unsigned> done_list;
  std::deque<AstBasicBlock *> working_list;

  AstBasicBlock *bb = func->GetEntryBB();
  MASSERT(bb && "null BB");
  unsigned bbid = bb->GetId();

  working_list.push_back(bb);

  unsigned bitnum = 0;

  while(working_list.size()) {
    bb = working_list.front();
    MASSERT(bb && "null BB");
    bbid = bb->GetId();

    // process bb not visited
    if (done_list.find(bbid) == done_list.end()) {
      std::cout << "working_list work " << bbid << std::endl;
      for (int i = 0; i < bb->GetStatementsNum(); i++) {
        TreeNode *stmt = bb->GetStatementAtIndex(i);
        visitor.Visit(stmt);
      }

      for (int i = 0; i < bb->GetSuccessorsNum(); i++) {
        working_list.push_back(bb->GetSuccessorAtIndex(i));
      }

      done_list.insert(bbid);
    }

    working_list.pop_front();
  }
}

DeclNode *TypeInferVisitor::VisitDeclNode(DeclNode *node) {
  (void) AstVisitor::VisitDeclNode(node);
  TreeNode *init = node->GetInit();
  if (init) {
    VisitTreeNode(init);
    node->SetTypeId(init->GetTypeId());
    node->GetVar()->SetTypeId(init->GetTypeId());
  }

  return node;
}

LiteralNode *TypeInferVisitor::VisitLiteralNode(LiteralNode *node) {
  (void) AstVisitor::VisitLiteralNode(node);
  LitId id = node->GetData().mType;
  switch (id) {
    case LT_IntegerLiteral:
      node->SetTypeId(TY_Int);
      break;
    case LT_FPLiteral:
      node->SetTypeId(TY_Float);
      break;
    case LT_DoubleLiteral:
      node->SetTypeId(TY_Double);
      break;
    case LT_StringLiteral:
      node->SetTypeId(TY_String);
      break;
    default:
      break;
  }
  return node;
}

IdentifierNode *TypeInferVisitor::VisitIdentifierNode(IdentifierNode *node) {
  (void) AstVisitor::VisitIdentifierNode(node);
  TreeNode *decl = mHandler->FindDecl(node);
  if (decl) {
    node->SetTypeId(decl->GetTypeId());
  }
  return node;
}

TypeId TypeInferVisitor::MergeTypeId(TypeId tia,  TypeId tib) {
  if (tia == tib) {
    return tia;
  } else if (tib == TY_Invalid) {
    return tia;
  } else if (tia == TY_Invalid) {
    return tib;
  } else if ((tia == TY_Int && tib == TY_Long) ||
             (tib == TY_Int && tia == TY_Long)) {
    return TY_Long;
  } else if ((tia == TY_Int && tib == TY_Float) ||
             (tib == TY_Int && tia == TY_Float)) {
    return TY_Float;
  } else if ((tia == TY_Long && tib == TY_Float) ||
             (tib == TY_Long && tia == TY_Float)) {
    return TY_Double;
  }
  NOTYETIMPL("MergeTypeId()");
  return TY_Invalid;
}

BinOperatorNode *TypeInferVisitor::VisitBinOperatorNode(BinOperatorNode *node) {
  (void) AstVisitor::VisitBinOperatorNode(node);
  OprId op = node->GetOprId();
  TreeNode *ta = node->GetOpndA();
  TreeNode *tb = node->GetOpndB();
  TreeNode *mod = NULL;
  TypeId tia = ta->GetTypeId();
  TypeId tib = tb->GetTypeId();
  switch (op) {
    case OPR_EQ:
    case OPR_NE:
    case OPR_GT:
    case OPR_LT:
    case OPR_GE:
    case OPR_LE: {
      if (tia != TY_Invalid && tib == TY_Invalid) {
        tb->SetTypeId(tia);
        mod = tb;
      } else if (tia == TY_Invalid && tib != TY_Invalid) {
        ta->SetTypeId(tib);
        mod = ta;
      }
      node->SetTypeId(TY_Boolean);
      break;
    }
    case OPR_Assign:
    case OPR_AddAssign:
    case OPR_SubAssign:
    case OPR_MulAssign:
    case OPR_DivAssign:
    case OPR_ModAssign:
    case OPR_ShlAssign:
    case OPR_ShrAssign:
    case OPR_BandAssign:
    case OPR_BorAssign:
    case OPR_BxorAssign:
    case OPR_ZextAssign:
      ta->SetTypeId(tb->GetTypeId());
      node->SetTypeId(tb->GetTypeId());
      mod = ta;
      break;
    case OPR_Add:
    case OPR_Sub:
    case OPR_Mul:
    case OPR_Div:
    case OPR_Mod:
    case OPR_Band:
    case OPR_Bor:
    case OPR_Bxor:
    case OPR_Shl:
    case OPR_Shr:
    case OPR_Zext:
    case OPR_Land:
    case OPR_Lor: {
      if (tia != TY_Invalid && tib == TY_Invalid) {
        tb->SetTypeId(tia);
        mod = tb;
      } else if (tia == TY_Invalid && tib != TY_Invalid) {
        ta->SetTypeId(tib);
        mod = ta;
      }
      TypeId ti = MergeTypeId(tia, tib);
      node->SetTypeId(ti);
      break;
    }
    default: {
      NOTYETIMPL("VisitBinOperatorNode()");
      break;
    }
  }
  // if mod is an identifier, update its decl
  if (mod && mod->IsIdentifier()) {
    TreeNode *decl = mHandler->FindDecl(static_cast<IdentifierNode *>(mod));
    if (decl) {
      decl->SetTypeId(mod->GetTypeId());
    }
  }
  return node;
}

UnaOperatorNode *TypeInferVisitor::VisitUnaOperatorNode(UnaOperatorNode *node) {
  (void) AstVisitor::VisitUnaOperatorNode(node);
  OprId op = node->GetOprId();
  TreeNode *ta = node->GetOpnd();
  switch (op) {
    case OPR_Add:
    case OPR_Sub:
      node->SetTypeId(ta->GetTypeId());
      break;
    case OPR_Inc:
    case OPR_Dec:
      ta->SetTypeId(TY_Int);
      node->SetTypeId(TY_Int);
      break;
    case OPR_Bcomp:
      ta->SetTypeId(TY_Int);
      node->SetTypeId(TY_Int);
      break;
    case OPR_Not:
      ta->SetTypeId(TY_Boolean);
      node->SetTypeId(TY_Boolean);
      break;
    default: {
      NOTYETIMPL("VisitUnaOperatorNode()");
      break;
    }
  }
  return node;
}

}
