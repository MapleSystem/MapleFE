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
    TreeNode *parent =  node->GetParent();
    str += EmitTreeNode(node);
  }

  str += "\n\n void "s + name + "::__init_func__() { // bind \"this\" to current module\n"s;
  isInit = true;
  for (unsigned i = 0; i < node->GetTreesNum(); ++i) {
    if (auto n = node->GetTree(i)) {
      if (n->GetKind() != NK_Class)
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

std::string CppDef::EmitExportNode(ExportNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  std::string target;
  if (auto n = node->GetTarget()) {
    target = EmitTreeNode(n);
    str += "// target: "s + target + "\n"s;
  }
  auto num = node->GetPairsNum();
  for (unsigned i = 0; i < node->GetPairsNum(); ++i) {
    if (auto n = node->GetPair(i))
      str += EmitXXportAsPairNode(n);
  }
  return HandleTreeNode(str, node);
}

std::string CppDef::EmitXXportAsPairNode(XXportAsPairNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (node->IsDefault()) {
    if (auto n = node->GetBefore())
      str += "{ "s + EmitTreeNode(n) + " as default }"s;
  } else if (node->IsEverything()) {
    str += " * "s;
    if (auto n = node->GetBefore())
      str += "as "s + EmitTreeNode(n);
  } else {
    if (auto n = node->GetBefore()) {
      if (n->GetKind() == NK_Identifier)
        str += "{ "s;
      str += EmitTreeNode(n);
      if (auto n = node->GetAfter())
        str += " as "s + EmitTreeNode(n);
      if (n->GetKind() == NK_Identifier)
        str += " }"s;
    }
  }
  str = "/* CppDef::EmitXXportAsPairNode \n "s + str + ";\n*/"s;;
  return HandleTreeNode(str, node);
}

inline std::string GetClassName(FunctionNode* f) {
  TreeNode* n = f->GetParent();
  if (n && n->GetKind()==NK_Class)
    return n->GetName();
  return ""s;
}

inline bool IsClassMethod(FunctionNode* f) {
  return (f && f->GetParent() && f->GetParent()->GetKind()==NK_Class);
}

std::string EmitCtorInstance(FunctionNode *node) {
  // assert(node->IsConstructor());
  ClassNode* c = static_cast<ClassNode*>(node->GetParent());
  std::string str, thisClass, baseClass, ctor, proto, prototypeProto;
  ctor = "&Function_ctor";
  thisClass = c->GetName();
  if (c->GetSuperClassesNum() == 0) {
    proto = "Function_ctor.prototype";
    prototypeProto = "Object_ctor.prototype";
  } else {
    baseClass = "Ctor_"s+c->GetSuperClass(0)->GetName(); 
    proto = "&"s+baseClass;
    prototypeProto = baseClass+".prototype"s;
  }
  str = "\n// Instantiate constructor"s;
  str += "\nCtor_"s+thisClass +" "+ thisClass+"_ctor("s +ctor+","s+proto+","+prototypeProto+");\n\n"s;
  return str;  
}

std::string CppDef::EmitFunctionNode(FunctionNode *node) {
  if (isInit || node == nullptr)
    return std::string();

  std::string str, className;
  if (node->IsConstructor()) {
    className = GetClassName(node);
    str = "\n"s;
    str += className + "*"s + " Ctor_"s + className + "::operator()"s;
  } else {
    str = mCppDecl.GetTypeString(node->GetType(), node->GetType());
    if(node->GetStrIdx())
      str += " "s + (IsClassMethod(node)?GetClassName(node):GetModuleName()) + "::"s + node->GetName();
  }
  str += "("s;

  for (unsigned i = 0; i < node->GetParamsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetParam(i)) {
      str += mCppDecl.EmitTreeNode(n);
    }
  }
  str += ")"s;
  int bodyPos = str.size();
  if (auto n = node->GetBody()) {
    auto s = EmitBlockNode(n);
    if(s.empty() || s.front() != '{')
      str += "{"s + s + "}\n"s;
    else
      str += s;
  } else
    str += "{}\n"s;

  if (node->IsConstructor()) {
    Emitter::Replace(str, "this.", "obj->", 0);
    std::string newObj = "\n  "s+className+"* obj = new "s+className+"(this, this->prototype);"s;
    str.insert(bodyPos+1, newObj, 0, std::string::npos);
    std::string ctorBody;
    ctorBody += "  return obj;\n"s;
    str.insert(str.size()-2, ctorBody, 0, std::string::npos);
    str += EmitCtorInstance(node);
  }

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
  if(!quoted && (s.front() != '"' || s.front() != '\''|| s.back() != '"' || s.back() != '\'' ))
    return false;
  if(!quoted)
    s = s.substr(1, s.length() - 2);
  Emitter::Replace(s, "\"", "\\\"", 0);
  s = "\"" + s + "\"s";
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
    str += " << std::endl;";
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

std::string CppDef::EmitCondBranchNode(CondBranchNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("if("s);
  if (auto n = node->GetCond()) {
    auto cond = EmitTreeNode(n);
    str += Clean(cond);
  }
  str += ")"s;
  if (auto n = node->GetTrueBranch()) {
    str += EmitTreeNode(n);
  }
  if (auto n = node->GetFalseBranch()) {
    str += "else"s + EmitTreeNode(n);
  }
  if(auto n = node->GetLabel()) {
    str += "__label_break_"s + EmitTreeNode(n) + ":;\n"s;
  }
  return str;
}

std::string CppDef::EmitBlockNode(BlockNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("{\n");
  for (unsigned i = 0; i < node->GetChildrenNum(); ++i) {
    if (auto n = node->GetChildAtIndex(i)) {
      str += EmitTreeNode(n);
    }
  }
  str += "}\n"s;
  if(auto n = node->GetLabel()) {
    str += "__label_break_"s + EmitTreeNode(n) + ":;\n"s;
  }
  mPrecedence = '\030';
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
          std::string s = EmitTreeNode(n);
          str += "auto "s + Clean(s);
        }
        str += " : "s;
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
    str += "__label_cont_"s + lstr + ":;\n}\n"s + "__label_break_"s + lstr + ":;\n"s;
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
  OprId k = node->GetOprId();
  std::string str;
  if(k == OPR_Exp) {
    str = "std::pow("s + lhs + ", "s + rhs + ")";
  } else {
    switch(k) {
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
    str = lhs + " "s + std::string(op + 1) + " "s + rhs;
  }
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

std::string CppDef::EmitLiteralNode(LiteralNode *node) {
  if (node == nullptr)
    return std::string();
  return Emitter::EmitLiteralNode(node);
}

std::string CppDef::EmitSwitchNode(SwitchNode *node) {
  if (node == nullptr)
    return std::string();
  bool doable = true;
  for (unsigned i = 0; i < node->GetCasesNum(); ++i)
    if (SwitchCaseNode* c = node->GetCaseAtIndex(i))
      for (unsigned j = 0; j < c->GetLabelsNum(); ++j) {
        auto l = c->GetLabelAtIndex(j);
        if (l && l->GetKind() == NK_SwitchLabel) {
          auto ln = static_cast<SwitchLabelNode*>(l);
          if (auto v = ln->GetValue())
            if(v->GetKind() != NK_Literal || v->GetTypeId() != TY_Int) {
              doable = false;
              goto out_of_loops;
            }
        }
      }
out_of_loops:
  std::string label;
  TreeNode* lab = node->GetLabel();
  if(lab)
    label = "__label_break_"s + EmitTreeNode(lab);
  else
    label = "__label_switch_" + std::to_string(node->GetNodeId());
  std::string str;
  if(doable) {
    str = "switch("s;
    if (TreeNode* n = node->GetExpr()) {
      std::string expr = EmitTreeNode(n);
      str += Clean(expr);
    }
    str += "){\n"s;
    for (unsigned i = 0; i < node->GetCasesNum(); ++i) {
      if(SwitchCaseNode* n = node->GetCaseAtIndex(i))
        str += EmitTreeNode(n);
    }
    str += "}\n"s;
  } else {
    std::string tmp = "__tmp_"s + std::to_string(node->GetNodeId());
    str = "do { // switch\nauto "s + tmp + " = "s;
    if (TreeNode* n = node->GetExpr()) {
      std::string expr = EmitTreeNode(n);
      str += Clean(expr);
    }
    str += ";\n"s;
    std::string body;
    std::string other = "goto "s + label + ";\n"s;;
    for (unsigned i = 0; i < node->GetCasesNum(); ++i)
      if (SwitchCaseNode* cn = node->GetCaseAtIndex(i)) {
        for (unsigned j = 0; j < cn->GetLabelsNum(); ++j)
          if (SwitchLabelNode* ln = cn->GetLabelAtIndex(j)) {
            if(ln->IsDefault())
              other = "goto __case_"s + std::to_string(cn->GetNodeId()) + ";\n"s;
            else {
              std::string le = EmitTreeNode(ln->GetValue());
              str += "if("s + tmp + " == ("s + Clean(le)
                + "))\ngoto __case_"s + std::to_string(cn->GetNodeId()) + ";\n"s;
            }
          }
        body += "__case_"s + std::to_string(cn->GetNodeId()) + ":\n"s;
        for (unsigned s = 0; s < cn->GetStmtsNum(); ++s)
          if (TreeNode* t = cn->GetStmtAtIndex(s))
            body += EmitTreeNode(t);
      }
    str += other + body;
    str += "} while(0);\n"s;
  }
  if(!doable || lab)
    str += label + ":;\n"s;
  return str;
}

std::string CppDef::EmitTypeOfNode(TypeOfNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("t2crt::__js_typeof("s), rhs;
  if (auto n = node->GetExpr())
    rhs = EmitTreeNode(n);
  str += rhs + ")"s;
  if (node->IsStmt())
    str += ";\n"s;
  return HandleTreeNode(str, node);
}

std::string CppDef::EmitNewNode(NewNode *node) {
  if (node == nullptr)
    return std::string();

  std::string str;
  if (auto n = node->GetId()) {
    if (n->GetTypeId() == TY_Class)
      // Generate call to object constructor if doing new on class obj
      str = n->GetName() + "_ctor"s;
    else 
      str = "new "s + EmitTreeNode(n);
  }

  str += "("s;
  auto num = node->GetArgsNum();
  for (unsigned i = 0; i < num; ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetArg(i)) {
      str += EmitTreeNode(n);
    }
  }
  str += ")"s;
  if (auto n = node->GetBody()) {
    str += " "s + EmitBlockNode(n);
  }
  mPrecedence = '\024';
  if (node->IsStmt())
    str += ";\n"s;
  return HandleTreeNode(str, node);
}

} // namespace maplefe
