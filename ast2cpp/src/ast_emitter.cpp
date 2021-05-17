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

#include "ast_emitter.h"

namespace maplefe {

using namespace std::string_literals;

std::string AstEmitter::AstEmitAnnotationNode(AnnotationNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetId()) {
    str += " "s + AstEmitIdentifierNode(n);
  }
  if (auto n = node->GetType()) {
    str += " "s + AstEmitAnnotationTypeNode(n);
  }
  if (auto n = node->GetExpr()) {
    str += " "s + AstEmitTreeNode(n);
  }
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitPackageNode(PackageNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "package";
  if (auto n = node->GetPackage()) {
    str += " "s + AstEmitTreeNode(n);
  }
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitXXportAsPairNode(XXportAsPairNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (node->IsDefault()) {
    if (auto n = node->GetBefore())
      str += " "s + AstEmitTreeNode(n);
  } else if (node->IsEverything()) {
    if (auto n = node->GetAfter())
      str += " * as "s + AstEmitTreeNode(n);
  } else {
    if (auto n = node->GetBefore()) {
      if (n->GetKind() == NK_Identifier)
        str += "{ "s;
      str += AstEmitTreeNode(n);
      if (auto n = node->GetAfter())
        str += " as "s + AstEmitTreeNode(n);
      if (n->GetKind() == NK_Identifier)
        str += " }"s;
    }
  }
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitExportNode(ExportNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "export "s;
  auto num = node->GetPairsNum();
  for (unsigned i = 0; i < node->GetPairsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetPair(i))
      str += AstEmitXXportAsPairNode(n);
  }
  if (auto n = node->GetTarget()) {
    str += " from "s + AstEmitTreeNode(n);
  }

  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitImportNode(ImportNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "import "s;
  /*
  switch (node->GetProperty()) {
    case ImpNone:
      break;
    case ImpType:
      break;
    case ImpStatic:
      break;
    case ImpSingle:
      break;
    case ImpAll:
      break;
    case ImpLocal:
      break;
    case ImpSystem:
      break;
    default:
      MASSERT(0 && "Unexpected enumerator");
  }
  */
  auto num = node->GetPairsNum();
  for (unsigned i = 0; i < node->GetPairsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetPair(i))
      str += AstEmitXXportAsPairNode(n);
  }
  if (auto n = node->GetTarget()) {
    str += " from "s + AstEmitTreeNode(n);
  }
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitUnaOperatorNode(UnaOperatorNode *node) {
  if (node == nullptr)
    return std::string();
  bool isPost = node->IsPost();
  const char *op = AstEmitter::GetEnumOprId(node->GetOprId());
  const Precedence precd = *op & 0x1f;
  const bool rl_assoc = *op >> 6; // false: left-to-right, true: right-to-left
  std::string opr;
  if (auto n = node->GetOpnd()) {
    opr = AstEmitTreeNode(n);
    if(precd > mPrecedence || (precd == mPrecedence && (rl_assoc && isPost || !rl_assoc && !isPost)))
      opr = "("s + opr + ")"s;
  }
  else
      opr = "(NIL)"s;
  std::string str;
  if(node->IsPost())
    str = opr + std::string(op + 1) + " "s;
  else
    str = " "s + std::string(op + 1) + opr;
  mPrecedence = precd;
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitBinOperatorNode(BinOperatorNode *node) {
  if (node == nullptr)
    return std::string();
  const char *op = AstEmitter::GetEnumOprId(node->GetOprId());
  const Precedence precd = *op & 0x1f;
  const bool rl_assoc = *op >> 6; // false: left-to-right, true: right-to-left
  std::string lhs, rhs;
  if (auto n = node->GetOpndA()) {
    lhs = AstEmitTreeNode(n);
    if(precd > mPrecedence || (precd == mPrecedence && rl_assoc))
      lhs = "("s + lhs + ")"s;
  }
  else
    lhs = "(NIL) "s;
  if (auto n = node->GetOpndB()) {
    rhs = AstEmitTreeNode(n);
    if(precd > mPrecedence || (precd == mPrecedence && !rl_assoc))
      rhs = "("s + rhs + ")"s;
  }
  else
    rhs = " (NIL)"s;
  std::string str(lhs + " "s + std::string(op + 1) + " "s + rhs);
  mPrecedence = precd;
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitTerOperatorNode(TerOperatorNode *node) {
  if (node == nullptr)
    return std::string();
  const Precedence precd = '\004';
  const bool rl_assoc = true; // true: right-to-left
  std::string str;
  if (auto n = node->GetOpndA()) {
    str = AstEmitTreeNode(n);
    if(precd > mPrecedence || (precd == mPrecedence && rl_assoc))
      str = "("s + str + ")"s;
  }
  str += " ? "s;
  if (auto n = node->GetOpndB()) {
    str += AstEmitTreeNode(n);
  }
  str += " : "s;
  if (auto n = node->GetOpndC()) {
    auto s = AstEmitTreeNode(n);
    if(precd > mPrecedence)
      s = "("s + s + ")"s;
    str += s;
  }
  mPrecedence = '\004';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitBlockNode(BlockNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "{\n"s;
  for (unsigned i = 0; i < node->GetChildrenNum(); ++i) {
    if (auto n = node->GetChildAtIndex(i)) {
      str += AstEmitTreeNode(n);
    }
  }
  str += "}\n"s;

  /*
  str += " "s + std::to_string(node->IsInstInit());

  for (unsigned i = 0; i < node->GetAttrsNum(); ++i) {
    str += " "s + AstDump::GetEnumAttrId(node->GetAttrAtIndex(i));
  }

  if (auto n = const_cast<TreeNode *>(node->GetSync())) {
    str += " "s + AstEmitTreeNode(n);
  }
  */
  mPrecedence = '\030';
  return str;
}

std::string AstEmitter::AstEmitNewNode(NewNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "new "s;
  if (auto n = node->GetId()) {
    str += " "s + AstEmitTreeNode(n);
  }
  str += "("s;
  auto num = node->GetArgsNum();
  for (unsigned i = 0; i < num; ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetArg(i)) {
      str += AstEmitTreeNode(n);
    }
  }
  str += ")"s;
  if (auto n = node->GetBody()) {
    str += " "s + AstEmitBlockNode(n);
  }
  mPrecedence = '\024';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitDeleteNode(DeleteNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("delete "s);
  if (auto n = node->GetExpr()) {
    str += AstEmitTreeNode(n);
  }
  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitDimensionNode(DimensionNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  for (unsigned i = 0; i < node->GetDimensionsNum(); ++i) {
    auto n = node->GetDimension(i);
    std::string d(n ? std::to_string(n) : ""s);
    str += "["s + d + "]"s;
  }
  return str;
}

std::string AstEmitter::AstEmitIdentifierNode(IdentifierNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str(node->GetName());

  //for (unsigned i = 0; i < node->GetAttrsNum(); ++i) {
  //  str += " "s + AstDump::GetEnumAttrId(node->GetAttrAtIndex(i));
  //}
  //if (auto n = node->GetDims()) {
  //  str += " "s + AstEmitDimensionNode(n);
  //}

  if (auto n = node->GetType()) {
    str += ": "s + AstEmitTreeNode(n);
  }
  if (auto n = node->GetInit()) {
    str += " = "s + AstEmitTreeNode(n);
  }
  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitDeclNode(DeclNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str(AstEmitter::GetEnumDeclProp(node->GetProp()));
  if (auto n = node->GetVar()) {
    str += " "s + AstEmitTreeNode(n);
  }
  if (auto n = node->GetInit()) {
    str += " = "s + AstEmitTreeNode(n);
  }
  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitAnnotationTypeNode(AnnotationTypeNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetId()) {
    str += " "s + AstEmitIdentifierNode(n);
  }
  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitCastNode(CastNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetDestType()) {
    str += "("s + AstEmitTreeNode(n) + ")";
  }
  if (auto n = node->GetExpr()) {
    str += AstEmitTreeNode(n);
  }
  mPrecedence = '\021';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitParenthesisNode(ParenthesisNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetExpr()) {
    str += "("s + AstEmitTreeNode(n) + ")"s;
  }
  mPrecedence = '\025';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitFieldNode(FieldNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetUpper()) {
    str += AstEmitTreeNode(n);
  }
  if (auto n = node->GetField()) {
    str += "."s + AstEmitIdentifierNode(n);
  }
  mPrecedence = '\024';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitArrayElementNode(ArrayElementNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetArray()) {
    str = AstEmitIdentifierNode(n);
  }

  for (unsigned i = 0; i < node->GetExprsNum(); ++i) {
    if (auto n = node->GetExprAtIndex(i)) {
      str += "["s + AstEmitTreeNode(n) + "]"s;
    }
  }

  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitArrayLiteralNode(ArrayLiteralNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("["s);
  for (unsigned i = 0; i < node->GetLiteralsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetLiteral(i)) {
      str += AstEmitTreeNode(n);
    }
  }
  str += "]"s;
  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitStructNode(StructNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  switch(node->GetProp()) {
    case SProp_CStruct:
      str = "struct "s;
      break;
    case SProp_TSInterface:
      str = "interface "s;
      break;
    case SProp_NA:
      str = "SProp_NA "s;
      break;
    default:
      MASSERT(0 && "Unexpected enumerator");
  }

  if (auto n = node->GetStructId()) {
    str += AstEmitIdentifierNode(n);
  }
  str += "{\n"s;
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    if (auto n = node->GetField(i)) {
      str += AstEmitIdentifierNode(n) + ";\n"s;
    }
  }
  str += "}\n"s;
  return str;
}

std::string AstEmitter::AstEmitFieldLiteralNode(FieldLiteralNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetFieldName()) {
    str = AstEmitIdentifierNode(n);
  }
  str += ": "s;
  if (auto n = node->GetLiteral()) {
    str += AstEmitTreeNode(n);
  }
  mPrecedence = '\030';
  return str;
}

std::string AstEmitter::AstEmitStructLiteralNode(StructLiteralNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "{"s;
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetField(i)) {
      str += AstEmitFieldLiteralNode(n);
    }
  }
  str += "}"s;
  mPrecedence = '\030';
  return str;
}

std::string AstEmitter::AstEmitVarListNode(VarListNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;

  for (unsigned i = 0; i < node->GetVarsNum(); ++i) {
    if (auto n = node->GetVarAtIndex(i)) {
      str += " "s + AstEmitIdentifierNode(n);
    }
  }

  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitExprListNode(ExprListNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;

  for (unsigned i = 0; i < node->GetExprsNum(); ++i) {
    if (auto n = node->GetExprAtIndex(i)) {
      str += " "s + AstEmitTreeNode(n);
    }
  }

  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitTemplateLiteralNode(TemplateLiteralNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "`"s;

  for (unsigned i = 0; i < node->GetStringsNum(); ++i) {
    if(auto s = node->GetStringAtIndex(i))
      str += s;
  }

  for (unsigned i = 0; i < node->GetPatternsNum(); ++i) {
    if(auto s = node->GetPatternAtIndex(i))
      str += s;
  }

  for (unsigned i = 0; i < node->GetTreesNum(); ++i) {
    if (auto n = node->GetTreeAtIndex(i)) {
      str += "${"s + AstEmitTreeNode(n) + "}"s;
    }
  }
  str += "`"s;
  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitLiteralNode(LiteralNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str(AstDump::GetEnumLitData(node->GetData()));
  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitThrowNode(ThrowNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "throw "s;
  for (unsigned i = 0; i < node->GetExceptionsNum(); ++i) {
    if (auto n = node->GetExceptionAtIndex(i)) {
      str += AstEmitTreeNode(n);
    }
  }
  str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitCatchNode(CatchNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "catch("s;
  for (unsigned i = 0; i < node->GetParamsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetParamAtIndex(i)) {
      str += AstEmitTreeNode(n);
    }
  }
  str += ")"s;
  if (auto n = node->GetBlock()) {
    str += AstEmitBlockNode(n);
  }
  return str;
}

std::string AstEmitter::AstEmitFinallyNode(FinallyNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "finally "s;
  if (auto n = node->GetBlock()) {
    str += AstEmitBlockNode(n);
  }
  else
    str += "{}\n"s;
  return str;
}

std::string AstEmitter::AstEmitTryNode(TryNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "try "s;
  if (auto n = node->GetBlock()) {
    str += AstEmitBlockNode(n);
  }
  for (unsigned i = 0; i < node->GetCatchesNum(); ++i) {
    if (auto n = node->GetCatchAtIndex(i)) {
      str += AstEmitCatchNode(n);
    }
  }
  if (auto n = node->GetFinally()) {
    str += AstEmitFinallyNode(n);
  }
  return str;
}

std::string AstEmitter::AstEmitExceptionNode(ExceptionNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetException()) {
    str += " "s + AstEmitIdentifierNode(n);
  }
  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitReturnNode(ReturnNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "return"s;
  if (auto n = node->GetResult()) {
    str += " "s + AstEmitTreeNode(n);
  }
  str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitCondBranchNode(CondBranchNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "if("s;
  if (auto n = node->GetCond()) {
    auto cond = AstEmitTreeNode(n);
    str += Clean(cond);
  }
  str += ")"s;
  if (auto n = node->GetTrueBranch()) {
    str += AstEmitTreeNode(n);
  }
  if (auto n = node->GetFalseBranch()) {
    str += "else"s + AstEmitTreeNode(n);
  }
  return str;
}

std::string AstEmitter::AstEmitBreakNode(BreakNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "break"s;
  if (auto n = node->GetTarget()) {
    str += " "s + AstEmitTreeNode(n);
  }
  str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitContinueNode(ContinueNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "continue"s;
  if (auto n = node->GetTarget()) {
    str += " "s + AstEmitTreeNode(n);
  }
  str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitForLoopNode(ForLoopNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if(auto n = node->GetLabel()) {
    str = AstEmitTreeNode(n) + ":\n"s;
  }
  str += "for("s;
  switch(node->GetProp()) {
    case FLP_Regular:
      {
        for (unsigned i = 0; i < node->GetInitsNum(); ++i)
          if (auto n = node->GetInitAtIndex(i)) {
            auto init = AstEmitTreeNode(n);
            if (i)
              str += ", "s;
            str += Clean(init);
          }
        str += "; "s;
        if (auto n = node->GetCond()) {
          auto cond = AstEmitTreeNode(n);
          str += Clean(cond);
        }
        str += "; "s;
        for (unsigned i = 0; i < node->GetUpdatesNum(); ++i)
          if (auto n = node->GetUpdateAtIndex(i)) {
            auto update = AstEmitTreeNode(n);
            if (i)
              str += ", "s;
            str += Clean(update);
          }
        break;
      }
    case FLP_JSIn:
      {
        if (auto n = node->GetVariable()) {
          str += AstEmitTreeNode(n);
        }
        str += " in "s;
        if (auto n = node->GetSet()) {
          str += AstEmitTreeNode(n);
        }
        break;
      }
    case FLP_JSOf:
      {
        if (auto n = node->GetVariable()) {
          str += AstEmitTreeNode(n);
        }
        str += " of "s;
        if (auto n = node->GetSet()) {
          str += AstEmitTreeNode(n);
        }
        break;
      }
    case FLP_NA:
      return "FLP_NA"s;
    default:
      MASSERT(0 && "Unexpected enumerator");
  }
  str += ")"s;

  if (auto n = node->GetBody()) {
    str += AstEmitTreeNode(n);
  }
  return str;
}

std::string AstEmitter::AstEmitWhileLoopNode(WhileLoopNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if(auto n = node->GetLabel()) {
    str = AstEmitTreeNode(n) + ":\n"s;
  }
  str += "while("s;
  if (auto n = node->GetCond()) {
    auto s = AstEmitTreeNode(n);
    str += Clean(s);
  }
  str += ")"s;
  if (auto n = node->GetBody()) {
    str += AstEmitTreeNode(n);
  }
  return str;
}

std::string AstEmitter::AstEmitDoLoopNode(DoLoopNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if(auto n = node->GetLabel()) {
    str = AstEmitTreeNode(n) + ":\n"s;
  }
  str += "do "s;
  if (auto n = node->GetBody()) {
    str += AstEmitTreeNode(n);
  }
  str += "while("s;
  if (auto n = node->GetCond()) {
    auto s = AstEmitTreeNode(n);
    str += Clean(s);
  }
  str += ");\n"s;
  return str;
}

std::string AstEmitter::AstEmitSwitchLabelNode(SwitchLabelNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if(node->IsDefault())
    str += "default:\n"s;
  if(auto n = node->GetValue()) {
    auto ce = AstEmitTreeNode(n);
    str += "case "s + Clean(ce) + ":\n";
  }
  return str;
}

std::string AstEmitter::AstEmitSwitchCaseNode(SwitchCaseNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  for (unsigned i = 0; i < node->GetLabelsNum(); ++i) {
    if (auto n = node->GetLabelAtIndex(i)) {
      str += AstEmitTreeNode(n);
    }
  }
  for (unsigned i = 0; i < node->GetStmtsNum(); ++i) {
    if (auto t = node->GetStmtAtIndex(i))
      str += AstEmitTreeNode(t);
  }
  return str;
}

std::string AstEmitter::AstEmitSwitchNode(SwitchNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "switch("s;
  if (auto n = node->GetExpr()) {
    str += AstEmitTreeNode(n);
  }
  str += "){\n"s;
  for (unsigned i = 0; i < node->GetCasesNum(); ++i) {
    if(auto n = node->GetCaseAtIndex(i))
      str += AstEmitTreeNode(n);
  }
  str += "}\n"s;
  return str;
}

std::string AstEmitter::AstEmitAssertNode(AssertNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetExpr()) {
    str += " "s + AstEmitTreeNode(n);
  }
  if (auto n = node->GetMsg()) {
    str += " "s + AstEmitTreeNode(n);
  }
  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitCallNode(CallNode *node) {
  if (node == nullptr)
    return std::string();
  // Function call: left-to-right, precedence = 20
  std::string str;
  if (auto n = node->GetMethod()) {
    auto s = AstEmitTreeNode(n);
    if(n->GetKind() == NK_Function)
      str += "("s + s + ")"s;
    else
      str += s;
  }
  str += "("s;
  for (unsigned i = 0; i < node->GetArgsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetArg(i))
      str += AstEmitTreeNode(n);
  }
  str += ")"s;
  mPrecedence = '\024';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitFunctionNode(FunctionNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "function "s;

  if(const char *name = node->GetName())
    str += name;

  /*
  for (unsigned i = 0; i < node->GetAttrsNum(); ++i) {
    str += " "s + AstDump::GetEnumAttrId(node->GetAttrAtIndex(i));
  }

  for (unsigned i = 0; i < node->GetAnnotationsNum(); ++i) {
    if (auto n = node->GetAnnotationAtIndex(i)) {
      str += " "s + AstEmitAnnotationNode(n);
    }
  }

  for (unsigned i = 0; i < node->GetThrowsNum(); ++i) {
    if (auto n = node->GetThrowAtIndex(i)) {
      str += " "s + AstEmitExceptionNode(n);
    }
  }
  */

  str += "("s;
  for (unsigned i = 0; i < node->GetParamsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetParam(i)) {
      str += AstEmitTreeNode(n);
    }
  }
  str += ")"s;

  if (auto n = node->GetType()) {
    str += " : "s + AstEmitTreeNode(n);
  }

  if (auto n = node->GetBody()) {
    auto s = AstEmitBlockNode(n);
    if(s.empty() || s.front() != '{')
      str += "{"s + s + "}\n"s;
    else
      str += s;
  } else
    str += "{}\n"s;
  /*
  if (auto n = node->GetDims()) {
    str += " "s + AstEmitDimensionNode(n);
  }
  str += " "s + std::to_string(node->IsConstructor());
  */
  mPrecedence = '\030';
  return str;
}

std::string AstEmitter::AstEmitInterfaceNode(InterfaceNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "interface "s + node->GetName() + " {"s;
  str += std::to_string(node->IsAnnotation());

  for (unsigned i = 0; i < node->GetSuperInterfacesNum(); ++i) {
    if (auto n = node->GetSuperInterfaceAtIndex(i)) {
      str += " "s + AstEmitInterfaceNode(n);
    }
  }

  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    if (auto n = node->GetFieldAtIndex(i)) {
      str += " "s + AstEmitIdentifierNode(n);
    }
  }

  for (unsigned i = 0; i < node->GetMethodsNum(); ++i) {
    if (auto n = node->GetMethodAtIndex(i)) {
      str += " "s + AstEmitFunctionNode(n);
    }
  }

  str += "};\n"s;
  return str;
}

std::string AstEmitter::AstEmitClassNode(ClassNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "class "s + node->GetName() + " {"s;
  //str += " "s + std::to_string(node->IsJavaEnum());

  for (unsigned i = 0; i < node->GetSuperClassesNum(); ++i) {
    if (auto n = node->GetSuperClass(i)) {
      str += " "s + AstEmitClassNode(n);
    }
  }

  for (unsigned i = 0; i < node->GetSuperInterfacesNum(); ++i) {
    if (auto n = node->GetSuperInterface(i)) {
      str += " "s + AstEmitInterfaceNode(n);
    }
  }

  for (unsigned i = 0; i < node->GetAttributesNum(); ++i) {
    str += " "s + AstDump::GetEnumAttrId(node->GetAttribute(i));
  }

  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    if (auto n = node->GetField(i)) {
      str += " "s + AstEmitIdentifierNode(n) + ";\n"s;
    }
  }

  for (unsigned i = 0; i < node->GetMethodsNum(); ++i) {
    if (auto n = node->GetMethod(i)) {
      str += " "s + AstEmitFunctionNode(n);
    }
  }

  for (unsigned i = 0; i < node->GetConstructorsNum(); ++i) {
    if (auto n = node->GetConstructor(i)) {
      str += " "s + AstEmitFunctionNode(n);
    }
  }

  for (unsigned i = 0; i < node->GetInstInitsNum(); ++i) {
    if (auto n = node->GetInstInit(i)) {
      str += " "s + AstEmitBlockNode(n);
    }
  }

  for (unsigned i = 0; i < node->GetLocalClassesNum(); ++i) {
    if (auto n = node->GetLocalClass(i)) {
      str += " "s + AstEmitClassNode(n);
    }
  }

  for (unsigned i = 0; i < node->GetLocalInterfacesNum(); ++i) {
    if (auto n = node->GetLocalInterface(i)) {
      str += " "s + AstEmitInterfaceNode(n);
    }
  }

  str += "};\n";
  return str;
}

std::string AstEmitter::AstEmitPassNode(PassNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "PassNode {"s;

  for (unsigned i = 0; i < node->GetChildrenNum(); ++i) {
    if (auto n = node->GetChild(i)) {
      str += " "s + AstEmitTreeNode(n);
    }
  }

  str += "}"s;
  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitLambdaNode(LambdaNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  str += " "s + AstDump::GetEnumLambdaProperty(node->GetProperty());
  if (auto n = node->GetType()) {
    str += " "s + AstEmitTreeNode(n);
  }

  for (unsigned i = 0; i < node->GetParamsNum(); ++i) {
    if (auto n = node->GetParam(i)) {
      str += " "s + AstEmitTreeNode(n);
    }
  }

  if (auto n = node->GetBody()) {
    str += " "s + AstEmitTreeNode(n);
  }
  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitInstanceOfNode(InstanceOfNode *node) {
  if (node == nullptr)
    return std::string();
  const Precedence precd = '\014';
  const bool rl_assoc = false;         // false: left-to-right
  std::string lhs, rhs;
  if (auto n = node->GetLeft()) {
    lhs = AstEmitTreeNode(n);
    if(precd > mPrecedence)
      lhs = "("s + lhs + ")"s;
  }
  else
    lhs = "(NIL) "s;
  if (auto n = node->GetRight()) {
    rhs = AstEmitTreeNode(n);
    if(precd > mPrecedence || (precd == mPrecedence && !rl_assoc))
      rhs = "("s + rhs + ")"s;
  }
  else
    rhs = " (NIL)"s;
  std::string str(lhs + " instanceof "s + rhs);
  mPrecedence = precd;
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitTypeOfNode(TypeOfNode *node) {
  if (node == nullptr)
    return std::string();
  const Precedence precd = '\014';
  std::string str("typeof "s), rhs;
  if (auto n = node->GetExpr()) {
    rhs = AstEmitTreeNode(n);
    if(precd > mPrecedence) // right-to-left
      rhs = "("s + rhs + ")"s;
  }
  else
    rhs = " (NIL)"s;
  str += rhs;
  mPrecedence = precd;
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitInNode(InNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetLeft()) {
    str += AstEmitTreeNode(n);
  }
  str += " in "s;
  if (auto n = node->GetRight()) {
    str += AstEmitTreeNode(n);
  }
  mPrecedence = '\014';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitAttrNode(AttrNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  str += " "s + AstDump::GetEnumAttrId(node->GetId());
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

static std::string GetTypeOp(UserTypeNode *node) {
  switch(node->GetType()) {
    case UT_Regular:
      return " UT_Regular "s;
    case UT_Union:
      return " | "s;
    case UT_Inter:
      return " & "s;
    case UT_Alias:
      if(auto t = node->GetChildA()) {
        if(t->GetKind() == NK_UserType)
          return GetTypeOp(static_cast<UserTypeNode *>(t));
      }
      return " | "s;
    default:
      MASSERT(0 && "Unexpected enumerator");
  }
  return "UNEXPECTED UT_Type"s;
}

std::string AstEmitter::AstEmitUserTypeNode(UserTypeNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetId()) {
    str += "type "s + AstEmitTreeNode(n) + " = "s;
  }
  str += AstEmitTreeNode(node->GetChildA());
  str += GetTypeOp(node);
  str += AstEmitTreeNode(node->GetChildB());
  /*
  for (unsigned i = 0; i < node->GetTypeArgumentsNum(); ++i) {
    if (i)
      str += " | "s;
    if (auto n = node->GetTypeArgument(i)) {
      str += AstEmitIdentifierNode(n);
    }
  }
  */
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitPrimTypeNode(PrimTypeNode *node) {
  if (node == nullptr)
    return std::string();
  return AstEmitter::GetEnumTypeId(node->GetPrimType());
}

std::string AstEmitter::AstEmitPrimArrayTypeNode(PrimArrayTypeNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetPrim()) {
    str += AstEmitPrimTypeNode(n);
  }
  if (auto n = node->GetDims()) {
    str += AstEmitDimensionNode(n);
  }
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string AstEmitter::AstEmitTreeNode(TreeNode *node) {
  if (node == nullptr)
    return std::string();
  switch (node->GetKind()) {
  case NK_Package:
    return AstEmitPackageNode(static_cast<PackageNode *>(node));
    break;
  case NK_XXportAsPair:
    return AstEmitXXportAsPairNode(static_cast<XXportAsPairNode *>(node));
    break;
  case NK_Import:
    return AstEmitImportNode(static_cast<ImportNode *>(node));
    break;
  case NK_Export:
    return AstEmitExportNode(static_cast<ExportNode *>(node));
    break;
  case NK_Decl:
    return AstEmitDeclNode(static_cast<DeclNode *>(node));
    break;
  case NK_Identifier:
    return AstEmitIdentifierNode(static_cast<IdentifierNode *>(node));
    break;
  case NK_Field:
    return AstEmitFieldNode(static_cast<FieldNode *>(node));
    break;
  case NK_Dimension:
    return AstEmitDimensionNode(static_cast<DimensionNode *>(node));
    break;
  case NK_Attr:
    return AstEmitAttrNode(static_cast<AttrNode *>(node));
    break;
  case NK_PrimType:
    return AstEmitPrimTypeNode(static_cast<PrimTypeNode *>(node));
    break;
  case NK_PrimArrayType:
    return AstEmitPrimArrayTypeNode(static_cast<PrimArrayTypeNode *>(node));
    break;
  case NK_UserType:
    return AstEmitUserTypeNode(static_cast<UserTypeNode *>(node));
    break;
  case NK_Cast:
    return AstEmitCastNode(static_cast<CastNode *>(node));
    break;
  case NK_Parenthesis:
    return AstEmitParenthesisNode(static_cast<ParenthesisNode *>(node));
    break;
  case NK_Struct:
    return AstEmitStructNode(static_cast<StructNode *>(node));
    break;
  case NK_StructLiteral:
    return AstEmitStructLiteralNode(static_cast<StructLiteralNode *>(node));
    break;
  case NK_FieldLiteral:
    return AstEmitFieldLiteralNode(static_cast<FieldLiteralNode *>(node));
    break;
  case NK_ArrayElement:
    return AstEmitArrayElementNode(static_cast<ArrayElementNode *>(node));
    break;
  case NK_ArrayLiteral:
    return AstEmitArrayLiteralNode(static_cast<ArrayLiteralNode *>(node));
    break;
  case NK_VarList:
    return AstEmitVarListNode(static_cast<VarListNode *>(node));
    break;
  case NK_ExprList:
    return AstEmitExprListNode(static_cast<ExprListNode *>(node));
    break;
  case NK_TemplateLiteral:
    return AstEmitTemplateLiteralNode(static_cast<TemplateLiteralNode *>(node));
    break;
  case NK_Literal:
    return AstEmitLiteralNode(static_cast<LiteralNode *>(node));
    break;
  case NK_UnaOperator:
    return AstEmitUnaOperatorNode(static_cast<UnaOperatorNode *>(node));
    break;
  case NK_BinOperator:
    return AstEmitBinOperatorNode(static_cast<BinOperatorNode *>(node));
    break;
  case NK_TerOperator:
    return AstEmitTerOperatorNode(static_cast<TerOperatorNode *>(node));
    break;
  case NK_Lambda:
    return AstEmitLambdaNode(static_cast<LambdaNode *>(node));
    break;
  case NK_InstanceOf:
    return AstEmitInstanceOfNode(static_cast<InstanceOfNode *>(node));
    break;
  case NK_TypeOf:
    return AstEmitTypeOfNode(static_cast<TypeOfNode *>(node));
    break;
  case NK_In:
    return AstEmitInNode(static_cast<InNode *>(node));
    break;
  case NK_Block:
    return AstEmitBlockNode(static_cast<BlockNode *>(node));
    break;
  case NK_Function:
    return AstEmitFunctionNode(static_cast<FunctionNode *>(node));
    break;
  case NK_Class:
    return AstEmitClassNode(static_cast<ClassNode *>(node));
    break;
  case NK_Interface:
    return AstEmitInterfaceNode(static_cast<InterfaceNode *>(node));
    break;
  case NK_AnnotationType:
    return AstEmitAnnotationTypeNode(static_cast<AnnotationTypeNode *>(node));
    break;
  case NK_Annotation:
    return AstEmitAnnotationNode(static_cast<AnnotationNode *>(node));
    break;
  case NK_Try:
    return AstEmitTryNode(static_cast<TryNode *>(node));
    break;
  case NK_Catch:
    return AstEmitCatchNode(static_cast<CatchNode *>(node));
    break;
  case NK_Finally:
    return AstEmitFinallyNode(static_cast<FinallyNode *>(node));
    break;
  case NK_Exception:
    return AstEmitExceptionNode(static_cast<ExceptionNode *>(node));
    break;
  case NK_Throw:
    return AstEmitThrowNode(static_cast<ThrowNode *>(node));
    break;
  case NK_Return:
    return AstEmitReturnNode(static_cast<ReturnNode *>(node));
    break;
  case NK_CondBranch:
    return AstEmitCondBranchNode(static_cast<CondBranchNode *>(node));
    break;
  case NK_Break:
    return AstEmitBreakNode(static_cast<BreakNode *>(node));
    break;
  case NK_Continue:
    return AstEmitContinueNode(static_cast<ContinueNode *>(node));
    break;
  case NK_ForLoop:
    return AstEmitForLoopNode(static_cast<ForLoopNode *>(node));
    break;
  case NK_WhileLoop:
    return AstEmitWhileLoopNode(static_cast<WhileLoopNode *>(node));
    break;
  case NK_DoLoop:
    return AstEmitDoLoopNode(static_cast<DoLoopNode *>(node));
    break;
  case NK_New:
    return AstEmitNewNode(static_cast<NewNode *>(node));
    break;
  case NK_Delete:
    return AstEmitDeleteNode(static_cast<DeleteNode *>(node));
    break;
  case NK_Call:
    return AstEmitCallNode(static_cast<CallNode *>(node));
    break;
  case NK_Assert:
    return AstEmitAssertNode(static_cast<AssertNode *>(node));
    break;
  case NK_SwitchLabel:
    return AstEmitSwitchLabelNode(static_cast<SwitchLabelNode *>(node));
    break;
  case NK_SwitchCase:
    return AstEmitSwitchCaseNode(static_cast<SwitchCaseNode *>(node));
    break;
  case NK_Switch:
    return AstEmitSwitchNode(static_cast<SwitchNode *>(node));
    break;
  case NK_Pass:
    return AstEmitPassNode(static_cast<PassNode *>(node));
    break;
  case NK_Null:
    // Ignore NullNode
    break;
  default:
    MASSERT(0 && "Unexpected node kind");
  }
  return std::string();
}


const char *AstEmitter::GetEnumTypeId(TypeId k) {
  switch (k) {
    case TY_Boolean:
      return "boolean";
    case TY_Byte:
      return "byte";
    case TY_Short:
      return "short";
    case TY_Int:
      return "int";
    case TY_Long:
      return "long";
    case TY_Char:
      return "char";
    case TY_Float:
      return "float";
    case TY_Double:
      return "double";
    case TY_Void:
      return "void";
    case TY_Null:
      return "null";
    case TY_Unknown:
      return "unknown";
    case TY_Undefined:
      return "undefined";
    case TY_String:
      return "string";
    case TY_Number:
      return "number";
    case TY_Any:
      return "any";
    case TY_NA:
      return "";
    default:
      MASSERT(0 && "Unexpected enumerator");
  }
  return "UNEXPECTED TypeId";
}

const char *AstEmitter::GetEnumDeclProp(DeclProp k) {
  switch (k) {
  case JS_Var:
    return "var";
  case JS_Let:
    return "let";
  case JS_Const:
    return "const";
  case DP_NA:
    return "";
  default:
    MASSERT(0 && "Unexpected enumerator");
  }
  return "UNEXPECTED DeclProp";
}

const char *AstEmitter::GetEnumOprId(OprId k) {
  // The first char in the returned string includes operator precedence and associativity info
  //
  // bits   7   6   5   4   3   2   1   0
  //        0   ^   ^---^---^-+-^---^---^
  //            |             |__ operator precedence
  //            |
  //            |__ associativity, 0: left-to-right, 1: right-to-left
  //
  switch (k) {
  case OPR_Plus:
    return          "\121+";
  case OPR_Add:
    return          "\016+";
  case OPR_Minus:
    return          "\121-";
  case OPR_Sub:
    return          "\016-";
  case OPR_Mul:
    return          "\017*";
  case OPR_Div:
    return          "\017/";
  case OPR_Mod:
    return          "\017%";
  case OPR_PreInc:
    return          "\121++";
  case OPR_Inc:
    return          "\022++";
  case OPR_PreDec:
    return          "\121--";
  case OPR_Dec:
    return          "\022--";
  case OPR_EQ:
    return          "\013==";
  case OPR_NE:
    return          "\013!=";
  case OPR_GT:
    return          "\014>";
  case OPR_LT:
    return          "\014<";
  case OPR_GE:
    return          "\014>=";
  case OPR_LE:
    return          "\014<=";
  case OPR_Band:
    return          "\012&";
  case OPR_Bor:
    return          "\010|";
  case OPR_Bxor:
    return          "\011^";
  case OPR_Bcomp:
    return          "\121~";
  case OPR_Shl:
    return          "\015<<";
  case OPR_Shr:
    return          "\015>>";
  case OPR_Zext:
    return          "\015>>>";
  case OPR_Land:
    return          "\007&&";
  case OPR_Lor:
    return          "\006||";
  case OPR_Not:
    return          "\121!";
  case OPR_Assign:
    return          "\103=";
  case OPR_AddAssign:
    return          "\103+=";
  case OPR_SubAssign:
    return          "\103-=";
  case OPR_MulAssign:
    return          "\103*=";
  case OPR_DivAssign:
    return          "\103/=";
  case OPR_ModAssign:
    return          "\103%=";
  case OPR_ShlAssign:
    return          "\103<<=";
  case OPR_ShrAssign:
    return          "\103>>=";
  case OPR_BandAssign:
    return          "\103&=";
  case OPR_BorAssign:
    return          "\103|=";
  case OPR_BxorAssign:
    return          "\103^=";
  case OPR_ZextAssign:
    return          "\103>>>=";
  case OPR_Arrow:
    return          "\030 OPR_Arrow";
  case OPR_Select:
    return          "\030 OPR_Select";
  case OPR_Cond:
    return          "\030 OPR_Cond";
  case OPR_Diamond:
    return          "\030 OPR_Diamond";
  case OPR_StEq:
    return          "\013===";
  case OPR_StNe:
    return          "\013!==";
  case OPR_ArrowFunction:
    return          "\030 OPR_ArrowFunction";
  case OPR_NA:
    return          "\030 OPR_NA";
  default:
    MASSERT(0 && "Unexpected enumerator");
  }
  return "UNEXPECTED OprId";
}

} // namespace maplefe
