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

#include "cpp_definition.h"

namespace maplefe {

std::string CppDef::EmitModuleNode(ModuleNode *node) {
  if (node == nullptr)
    return std::string();
  std::string name = GetModuleName();
  std::string str("// TypeScript filename: "s + node->GetFileName() + "\n"s);
  str += "#include <iostream>\n#include \""s + GetBaseFileName() + ".h\"\n\n"s;

  // definitions of all top-level functions
  isInit = false;
  CfgFunc *module = mHandler->GetCfgFunc();
  auto num = module->GetNestedFuncsNum();
  for(unsigned i = 0; i < num; ++i) {
    CfgFunc *func = module->GetNestedFuncAtIndex(i);
    TreeNode *node = func->GetFuncNode();
    str += EmitTreeNode(node);
  }

  str += "\n\n void "s + name + "::__init_func__() { // bind \"this\" to current module\n"s;
  isInit = true;
  for (unsigned i = 0; i < node->GetTreesNum(); ++i) {
    if (auto n = node->GetTree(i)) {
      str += EmitTreeNode(n);
    }
  }
  str += "}\n\n"s + name + " _"s + name + R"""(;

// If the program starts from this module, generate the main function
int main(int argc, char **argv) {
)""" + "  _"s + name + R"""(.__init_func__(); // only call to its __init_func__()
  return 0;
}
)""";
  return str;
}

std::string CppDef::EmitFunctionNode(FunctionNode *node) {
  if (isInit || node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetType()) {
    str += mCppDecl.EmitTreeNode(n) + " "s;
  }
  else
    str += "auto "s;
  if(node->GetStrIdx())
    str += GetModuleName() + "::"s + node->GetName();
  str += "("s;

  for (unsigned i = 0; i < node->GetParamsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetParam(i)) {
      str += mCppDecl.EmitTreeNode(n);
    }
  }
  str += ")"s;
  if (auto n = node->GetBody()) {
    auto s = EmitBlockNode(n);
    if(s.empty() || s.front() != '{')
      str += "{"s + s + "}\n"s;
    else
      str += s;
  } else
    str += "{}\n"s;
  return str;
}

std::string CppDef::EmitIdentifierNode(IdentifierNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  str += node->GetName();
  if (auto n = node->GetInit()) {
    str += " = "s + EmitTreeNode(n);
  }
  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string CppDef::EmitDeclNode(DeclNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  //std::string str(Emitter::GetEnumDeclProp(node->GetProp()));
  if (auto n = node->GetVar()) {
    str += isInit ? EmitTreeNode(n) : mCppDecl.EmitTreeNode(n);
  }
  if (auto n = node->GetInit()) {
    if(n->GetKind() == NK_ArrayLiteral)
      str += ".clear();\n"s + str + ".insert("s + str + ".end(), "s
        + EmitTreeNode(n) + ")"s;
    else
      str += " = "s + EmitTreeNode(n);
  }
  str += ";\n"s;
  return str;
}

static bool QuoteStringLiteral(std::string &s, bool quoted = false) {
  if(!quoted && (s.front() != '"' || s.back() != '"'))
    return false;
  if(!quoted)
    s = s.substr(1, s.length() - 2);
  Emitter::Replace(s, "\"", "\\\"", 0);
  s = "\"" + s + "\"";
  return true;
}

std::string CppDef::EmitCallNode(CallNode *node) {
  if (node == nullptr)
    return std::string();
  bool log = false;
  std::string str;
  if (auto n = node->GetMethod()) {
    auto s = EmitTreeNode(n);
    if(n->GetKind() == NK_Function)
      str += "("s + s + ")"s;
    else {
      if(s.compare(0, 11, "console.log") == 0) {
        str += "std::cout"s;
        log = true;
      } else
        str += s;
    }
  }
  if(!log)
    str += "("s;
  unsigned num = node->GetArgsNum();
  for (unsigned i = 0; i < num; ++i) {
    if(log) {
      std::string s = EmitTreeNode(node->GetArg(i));
      if(QuoteStringLiteral(s)) {
        if(num > 1)
          s += "\"'"s + s + "'\""s;
      } else if(mPrecedence <= 13) // '\015'
        s = "("s + s + ")"s;
      if (i)
        str += " << ' ' "s;
      str += " << "s + s;
    } else {
      if (i)
        str += ", "s;
      if (auto n = node->GetArg(i))
        str += EmitTreeNode(n);
    }
  }
  if(!log)
    str += ")"s;
  else
    str += " << std::endl";
  mPrecedence = '\024';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string CppDef::EmitPrimTypeNode(PrimTypeNode *node) {
  return std::string();
}

std::string CppDef::EmitPrimArrayTypeNode(PrimArrayTypeNode *node) {
  return std::string();
}

std::string CppDef::EmitArrayLiteralNode(ArrayLiteralNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("{"s);
  for (unsigned i = 0; i < node->GetLiteralsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetLiteral(i)) {
      str += EmitTreeNode(n);
    }
  }
  str += "}"s;
  return str;
}

std::string CppDef::EmitFieldNode(FieldNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetUpper()) {
    str += EmitTreeNode(n);
  }
  if (auto n = node->GetField()) {
    std::string field = EmitIdentifierNode(n);
    Emitter::Replace(field, "length", "size()");
    str += "."s + field;
  }
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string CppDef::EmitForLoopNode(ForLoopNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  str += "for("s;
  switch(node->GetProp()) {
    case FLP_Regular:
      {
        for (unsigned i = 0; i < node->GetInitsNum(); ++i)
          if (auto n = node->GetInitAtIndex(i)) {
            auto init = EmitTreeNode(n);
            if (i)
              str += ", "s;
            str += Clean(init);
          }
        str += "; "s;
        if (auto n = node->GetCond()) {
          auto cond = EmitTreeNode(n);
          str += Clean(cond);
        }
        str += "; "s;
        for (unsigned i = 0; i < node->GetUpdatesNum(); ++i)
          if (auto n = node->GetUpdateAtIndex(i)) {
            auto update = EmitTreeNode(n);
            if (i)
              str += ", "s;
            str += Clean(update);
          }
        break;
      }
    case FLP_JSIn:
      {
        if (auto n = node->GetVariable()) {
          str += EmitTreeNode(n);
        }
        str += " in "s;
        if (auto n = node->GetSet()) {
          str += EmitTreeNode(n);
        }
        break;
      }
    case FLP_JSOf:
      {
        if (auto n = node->GetVariable()) {
          str += EmitTreeNode(n);
        }
        str += " of "s;
        if (auto n = node->GetSet()) {
          str += EmitTreeNode(n);
        }
        break;
      }
    case FLP_NA:
      return "FLP_NA"s;
    default:
      MASSERT(0 && "Unexpected enumerator");
  }
  str += ")"s;

  auto label = node->GetLabel();
  std::string lstr;
  if(label) {
    lstr = EmitTreeNode(label);
    str += "{\n"s;
  }
  if (auto n = node->GetBody()) {
    str += EmitTreeNode(n);
  }
  if(label)
    str += "__label_cont_" + lstr + ":;\n}\n"s + "__label_break_" + lstr + ":;\n"s;
  return str;
}

std::string CppDef::EmitBreakNode(BreakNode *node) {
  if (node == nullptr)
    return std::string();
  auto target = node->GetTarget();
  std::string str = target ? "goto __label_break_"s + EmitTreeNode(target) : "break"s;
  str += ";\n"s;
  return str;
}

std::string CppDef::EmitContinueNode(ContinueNode *node) {
  if (node == nullptr)
    return std::string();
  auto target = node->GetTarget();
  std::string str = target ? "goto __label_cont_"s + EmitTreeNode(target) : "continue"s;
  str += ";\n"s;
  return str;
}

std::string CppDef::EmitBinOperatorNode(BinOperatorNode *node) {
  if (node == nullptr)
    return std::string();
  const char *op = Emitter::GetEnumOprId(node->GetOprId());
  const Precedence precd = *op & 0x3f;
  const bool rl_assoc = *op >> 6; // false: left-to-right, true: right-to-left
  std::string lhs, rhs;
  if (auto n = node->GetOpndA()) {
    lhs = EmitTreeNode(n);
    if(precd > mPrecedence || (precd == mPrecedence && rl_assoc))
      lhs = "("s + lhs + ")"s;
  }
  else
    lhs = "(NIL) "s;
  if (auto n = node->GetOpndB()) {
    rhs = EmitTreeNode(n);
    if(precd > mPrecedence || (precd == mPrecedence && !rl_assoc))
      rhs = "("s + rhs + ")"s;
  }
  else
    rhs = " (NIL)"s;
  switch(node->GetOprId()) {
    case OPR_Band:
    case OPR_Bor:
    case OPR_Bxor:
    case OPR_Shl:
    case OPR_Shr:
      lhs = "static_cast<int32_t>("s + lhs + ")"s;
      break;
    case OPR_Zext:
      lhs = "static_cast<uint32_t>("s + lhs + ")"s;
      op = "\015>>";
      break;
  }
  std::string str(lhs + " "s + std::string(op + 1) + " "s + rhs);
  mPrecedence = precd;
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string CppDef::EmitUnaOperatorNode(UnaOperatorNode *node) {
  if (node == nullptr)
    return std::string();
  bool isPost = node->IsPost();
  const char *op = Emitter::GetEnumOprId(node->GetOprId());
  const Precedence precd = *op & 0x3f;
  const bool rl_assoc = *op >> 6; // false: left-to-right, true: right-to-left
  std::string opr;
  if (auto n = node->GetOpnd()) {
    opr = EmitTreeNode(n);
    if(precd > mPrecedence || (precd == mPrecedence && (rl_assoc && isPost || !rl_assoc && !isPost)))
      opr = "("s + opr + ")"s;
  }
  else
      opr = "(NIL)"s;
  if(node->GetOprId() == OPR_Bcomp)
    opr = "static_cast<int32_t>("s + opr + ")"s;
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

std::string CppDef::EmitTemplateLiteralNode(TemplateLiteralNode *node) {
  if (node == nullptr)
    return std::string();
  auto num = node->GetTreesNum();
  std::string str;
  for (unsigned i = 0; i < num; ++i) {
    if (auto n = node->GetTreeAtIndex(i)) {
      if (!std::empty(str))
        str += " + "s;
      std::string s(EmitTreeNode(n));
      if(i & 0x1)
        str += "t2crt::to_string("s + s+ ")"s;
      else {
        QuoteStringLiteral(s, true);
        str += s;
      }
    }
  }
  mPrecedence = '\016';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

} // namespace maplefe
