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

#include "emitter.h"
#include <cstring>
#include <limits>
#include <cctype>

namespace maplefe {

std::string Emitter::Emit(const char *title) {
  std::string code;
  code = "// [Beginning of Emitter: "s + title + "\n"s;
  code += EmitTreeNode(GetASTModule());
  code += "// End of Emitter]\n"s;
  return code;
}

std::string Emitter::GetEnding(TreeNode *n) {
  if (n->IsExport()) {
    ExportNode *ex = static_cast<ExportNode *>(n);
    if (ex->GetPairsNum() == 1) {
      if (auto p = ex->GetPair(0)->GetBefore())
        n = p;
    }
  }
  if (n->IsDeclare()) {
    DeclareNode *d = static_cast<DeclareNode *>(n);
    if (d->GetDeclsNum() == 1)
      if (auto p = d->GetDeclAtIndex(0))
        n = p;
  }
  std::string str;
  switch(n->GetKind()) {
    default:
      str += ';';
    //case NK_Function:
    case NK_Block:
    case NK_Switch:
    case NK_ForLoop:
    case NK_WhileLoop:
    case NK_DoLoop:
    case NK_CondBranch:
    case NK_Class:
    case NK_Struct:
    case NK_Namespace:
    case NK_Declare:
      str += "\n"s;
  }
  return str;
}

std::string Emitter::Clean(std::string &s) {
  auto len = s.length();
  if(len >= 1 && s.back() == '\n')
    s = s.erase(len - 1);
  if(len >= 2 && s.back() == ';')
    return s.erase(len - 2);
  return s;
}

std::string Emitter::GetBaseFilename() {
  std::string str(GetASTModule()->GetFilename());
  auto len = str.length();
  if(len >= 3 && str.substr(len - 3) == ".ts")
    return str.erase(len - 3);
  return str;
}

std::string Emitter::GetModuleName(const char *p) {
  std::string str = p ? p : GetBaseFilename();
  size_t pos = str.rfind("/", std::string::npos);
  str = pos == std::string::npos ? str : str.substr(pos);
  for (auto &c : str)
    if(std::ispunct(c))
        c = '_';
  return "M_"s + str;
}

std::string Emitter::GetEnumAttrId(AttrId k) {
  std::string str(AstDump::GetEnumAttrId(k) + 5);
  Emitter::Replace(str, "etter", "et");
  str += ' ';
  return str;
}

void Emitter::Replace(std::string &str, const char *o, const char *n, int cnt) {
  size_t len = std::strlen(o);
  if(cnt > 0) {
    size_t index = 0;
    int num = cnt;
    do {
      index = str.find(o, index);
      if (index == std::string::npos) break;
      str.replace(index, len, n);
      index += len;
    } while(--num && index < str.size());
  } else {
    size_t index = std::string::npos;
    int num = cnt ? -cnt : std::numeric_limits<int>::max();
    do {
      index = str.rfind(o, index);
      if (index == std::string::npos) break;
      str.replace(index, len, n);
    } while(--num);
  }
}

std::string Emitter::EmitAnnotationNode(AnnotationNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetId()) {
    str += EmitTreeNode(n);
  }
  if (auto num = node->GetArgsNum()) {
    str += '(';
    for (unsigned i = 0; i < num; ++i) {
      if (i)
        str += ", "s;
      if (auto n = node->GetArgAtIndex(i))
        str += EmitTreeNode(n);
    }
    str += ')';
  }
  if (auto n = node->GetType()) {
    str += ": "s + EmitAnnotationTypeNode(n);
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitAsTypeNode(AsTypeNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetType()) {
    str += " as "s + EmitTreeNode(n);
  }
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitIdentifierNode(IdentifierNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;

  for (unsigned i = 0; i < node->GetAnnotationsNum(); ++i)
    if (auto n = node->GetAnnotationAtIndex(i))
      str += '@' + EmitTreeNode(n) + "\n"s;

  std::string accessor1, accessor2;
  for (unsigned i = 0; i < node->GetAttrsNum(); ++i) {
    std::string s = GetEnumAttrId(node->GetAttrAtIndex(i));
    if (s == "get "s || s == "set "s)
      accessor2 += s;
    else
      accessor1 += s;
  }
  std::string name(node->GetName());
  if (accessor1 == "private "s && name == "private"s)
    str += "#private"s;
  else
    str += accessor1 + accessor2 + name;
  str = HandleTreeNode(str, node);
  //if (auto n = node->GetDims()) {
  //  str += ' ' + EmitDimensionNode(n);
  //}

  if (auto n = node->GetType()) {
    std::string s = EmitTreeNode(n);
    if(s.length() > 9 && s.substr(0, 9) == "function ") {
      std::size_t loc = s.find("(");
      if(loc != std::string::npos)
        s = s.substr(loc);
    }
    str += ": "s + s;
  }
  if (auto n = node->GetInit()) {
    str += " = "s + EmitTreeNode(n);
  }
  mPrecedence = '\030';
  return str;
}

std::string Emitter::EmitFunctionNode(FunctionNode *node) {
  if (node == nullptr)
    return std::string();
  std::string pre;
  for (unsigned i = 0; i < node->GetAnnotationsNum(); ++i)
    if (auto n = node->GetAnnotationAtIndex(i))
      pre += '@' + EmitTreeNode(n) + "\n"s;

  auto p = node->GetParent();
  NodeKind k = p ? p->GetKind() : NK_Null;
  bool inside = k == NK_Class || k == NK_Struct || k == NK_Interface;
  bool func = !inside;
  std::string accessor;
  for (unsigned i = 0; i < node->GetAttrsNum(); ++i) {
    std::string s = GetEnumAttrId(node->GetAttrAtIndex(i));
    if (s == "get "s || s == "set "s) {
      func = false;
      accessor += s;
    } else
      pre += s;
  }
  pre += accessor;

  std::string str;
  if (node->IsConstructor())
    str = "constructor "s;
  bool has_name;
  if(TreeNode* name = node->GetFuncName()) {
    std::string s = EmitTreeNode(name);
    has_name = s.substr(0, 9) != "__lambda_";
    if (has_name)
      str += s;
  } else
    has_name = k == NK_XXportAsPair;

  if (node->IsConstructSignature())
    str += "new"s;

  auto num = node->GetTypeParamsNum();
  if(num) {
    str += '<';
    for (unsigned i = 0; i < num; ++i) {
      if (i)
        str += ", "s;
      if (auto n = node->GetTypeParamAtIndex(i)) {
        str += EmitTreeNode(n);
      }
    }
    str += '>';
  }

  /*
  for (unsigned i = 0; i < node->GetThrowsNum(); ++i) {
    if (auto n = node->GetThrowAtIndex(i)) {
      str += ' ' + EmitExceptionNode(n);
    }
  }
  */

  str += '(';
  for (unsigned i = 0; i < node->GetParamsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetParam(i)) {
      str += EmitTreeNode(n);
    }
  }
  str += ')';

  if (auto n = node->GetAssert())
    str += " : asserts "s + EmitTreeNode(n);

  auto body = node->GetBody();
  if (auto n = node->GetType()) {
    std::string s = EmitTreeNode(n);
    if(!s.empty()) {
      str += (body || has_name || inside ? " : "s : " => "s) + s;
      if (!body && !has_name)
        func = false;
    }
  }
  str = pre + (func ? "function "s : ""s) + str;

  if (body) {
    auto s = EmitBlockNode(body);
    if(s.empty() || s.front() != '{')
      str += '{' + s + "}\n"s;
    else
      str += s;
  }
  else
    if (k == NK_Block || k == NK_Struct || k == NK_Class)
      str += ";\n"s;
  /*
  if (auto n = node->GetDims()) {
    str += ' ' + EmitDimensionNode(n);
  }
  str += ' ' + std::to_string(node->IsConstructor());
  */
  mPrecedence = '\004';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitUserTypeNode(UserTypeNode *node) {
  if (node == nullptr)
    return std::string();
  auto k = node->GetType();
  Precedence precd;
  switch (k) {
    case UT_Union:
      precd = '\010';
      break;
    case UT_Inter:
      precd = '\012';
      break;
    default:
      precd = '\030';
  }
  std::string attrs, accessor, str;
  for (unsigned i = 0; i < node->GetAttrsNum(); ++i) {
    std::string s = GetEnumAttrId(node->GetAttrAtIndex(i));
    if (s == "get "s || s == "set "s)
      accessor += s;
    else
      attrs += s;
  }
  attrs += accessor;
  if (auto n = node->GetId()) {
    str += EmitTreeNode(n);
    auto num = node->GetTypeGenericsNum();
    if(num) {
      str += '<';
      for (unsigned i = 0; i < num; ++i) {
        if (i)
          str += ", "s;
        if (auto n = node->GetTypeGeneric(i)) {
          str += EmitTreeNode(n);
        }
      }
      str += '>';
    }
    precd = mPrecedence;
  }

  if(k != UT_Regular) {
    if(!str.empty())
      str = "type "s + str + " = "s;
    std::string op = k == UT_Union ? " | "s : " & "s;
    for (unsigned i = 0; i < node->GetUnionInterTypesNum(); ++i) {
      if(i)
        str += op;
      std::string s = EmitTreeNode(node->GetUnionInterType(i));
      if (precd >= mPrecedence)
        s = '(' + s + ')';
      str += s;
    }
    mPrecedence = precd;
  }

  if (auto n = node->GetDims()) {
    std::string s = EmitDimensionNode(n);
    if (precd < mPrecedence)
      str = '(' + str + ')';
     str += s;
  }
  str = attrs + str;
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitComputedNameNode(ComputedNameNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("[");
  if (auto n = node->GetExpr()) {
    str += EmitTreeNode(n);
  }
  str += "] "s;

  if (auto prop = node->GetProp()) {
    if (prop & CNP_Rem_ReadOnly)
      str = "-readonly "s + str;
    if (prop & CNP_Add_ReadOnly)
      str = "readonly "s + str;
    if (prop & CNP_Rem_Optional)
      str += "-?"s;
    if (prop & CNP_Add_Optional)
      str += '?';
  }

  str = HandleTreeNode(str, node);
  if (auto n = node->GetExtendType()) {
    str += ": "s + EmitTreeNode(n);
  }
  if (auto n = node->GetInit()) {
    str += " = "s + EmitTreeNode(n);
  }
  return str;
}

std::string Emitter::EmitPackageNode(PackageNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "package";
  if (auto n = node->GetPackage()) {
    str += ' ' + EmitTreeNode(n);
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitXXportAsPairNode(XXportAsPairNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (node->IsDefault()) {
    if (auto n = node->GetBefore())
      str += "default "s + EmitTreeNode(n);
  } else if (node->IsEverything()) {
    str += " * "s;
    if (auto n = node->GetBefore())
      str += "as "s + EmitTreeNode(n);
  } else if (node->IsSingle()) {
    if (auto a = node->GetAfter())
      str += EmitTreeNode(a);
    if (auto n = node->GetBefore()) {
      str += " = "s;
      std::string s = EmitTreeNode(n);
      str += n->IsLiteral() ? "require("s + s + ')' : s;
    }
  } else {
    if (auto n = node->GetBefore()) {
      std::string s = EmitTreeNode(n);
      if (auto a = node->GetAfter())
        s += " as "s + EmitTreeNode(a);
      if (n->IsIdentifier())
        s = "{ "s + s + " }"s;
      str += s;
    }
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitDeclareNode(DeclareNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str, accessor;
  for (unsigned i = 0; i < node->GetAttrsNum(); ++i) {
    std::string s = GetEnumAttrId(node->GetAttrAtIndex(i));
    if (s == "get "s || s == "set "s)
      accessor += s;
    else
      str += s;
  }
  str += accessor;

  unsigned num = node->GetDeclsNum();
  if (node->IsGlobal() || num != 1) {
    str += "declare "s;
    if (node->IsGlobal())
      str += "global "s;
    str += "{\n"s;
    for (unsigned i = 0; i < num; ++i) {
      if (auto n = node->GetDeclAtIndex(i))
        str += EmitTreeNode(n) + GetEnding(n);
    }
    str += "}\n"s;
  } else {
    if (auto n = node->GetDeclAtIndex(0)) {
      std::string s = EmitTreeNode(n);
      if (n->IsModule()) {
        s = "module \""s + static_cast<ModuleNode *>(n)->GetFilename()
          + "\" {\n"s + s + "}\n"s;
      }
      str += "declare "s + s;
    }
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitExportNode(ExportNode *node) {
  if (node == nullptr)
    return std::string();
  std::string deco;
  for (unsigned i = 0; i < node->GetAnnotationsNum(); ++i)
    if (auto n = node->GetAnnotationAtIndex(i))
      deco += '@' + EmitTreeNode(n) + "\n"s;

  std::string str;
  auto num = node->GetPairsNum();
  for (unsigned i = 0; i < num; ++i) {
    if (auto n = node->GetPair(i)) {
      std::string s = EmitXXportAsPairNode(n);
      if (!s.empty() && s.front() == '{' && !str.empty() && str.back() == '}') {
        str.pop_back();
        s.erase(0, 1);
      }
      str += i ? ", "s + s : s;
    }
  }
  if (auto n = node->GetTarget()) {
    str += " from "s + EmitTreeNode(n);
  }
  str = Clean(str);
  if (str.empty())
    str = "{}"s;
  str = deco + "export "s + str;
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitImportNode(ImportNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("import ");
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
  if (num == 1 && node->GetTarget() == nullptr)
    if (XXportAsPairNode *pair = node->GetPair(0))
      if (auto a = pair->GetAfter())
        if (auto b = pair->GetBefore())
          if (b->IsIdentifier() && a->IsIdentifier()) {
            str += EmitTreeNode(a) + " = "s + EmitTreeNode(b);
            return HandleTreeNode(str, node);
          }

  for (unsigned i = 0; i < node->GetPairsNum(); ++i) {
    if (auto n = node->GetPair(i)) {
      std::string s = EmitXXportAsPairNode(n);
      auto len = s.length();
      if(len > 13 && s.substr(len - 13) == " as default }"s)
        s = s.substr(1, len - 13); // default export from a module
      if(len > 8 && s.substr(0, 8) == "default "s)
        s = s.substr(8, len - 1);
      if (!s.empty() && s.front() == '{' && !str.empty() && str.back() == '}') {
        str.pop_back();
        s.erase(0, 1);
      }
      str += i ? ", "s + s : s;
    }
  }
  if (auto n = node->GetTarget()) {
    std::string s = EmitTreeNode(n);
    if (num)
      str += " from "s + s;
    else {
      auto p = node->GetParent();
      if (p && p->IsField())
        str += '(' + s + ')';
      else
        str += s;
    }
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitUnaOperatorNode(UnaOperatorNode *node) {
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
      opr = '(' + opr + ')';
  }
  else
      opr = "(NIL)"s;
  std::string str;
  if(node->IsPost())
    str = opr + std::string(op + 1) + ' ';
  else
    str = ' ' + std::string(op + 1) + opr;
  mPrecedence = precd;
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitBinOperatorNode(BinOperatorNode *node) {
  if (node == nullptr)
    return std::string();
  const char *op = Emitter::GetEnumOprId(node->GetOprId());
  const Precedence precd = *op & 0x3f;
  const bool rl_assoc = *op >> 6; // false: left-to-right, true: right-to-left
  std::string lhs, rhs;
  if (auto n = node->GetOpndA()) {
    lhs = EmitTreeNode(n);
    if(precd > mPrecedence || (precd == mPrecedence && rl_assoc))
      lhs = '(' + lhs + ')';
  }
  else
    lhs = "(NIL) "s;
  if (auto n = node->GetOpndB()) {
    rhs = EmitTreeNode(n);
    if(precd > mPrecedence || (precd == mPrecedence && !rl_assoc))
      rhs = '(' + rhs + ')';
  }
  else
    rhs = " (NIL)"s;
  std::string str(lhs + ' ' + std::string(op + 1) + ' ' + rhs);
  mPrecedence = precd;
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitTerOperatorNode(TerOperatorNode *node) {
  if (node == nullptr)
    return std::string();
  const Precedence precd = '\004';
  const bool rl_assoc = true; // true: right-to-left
  std::string str;
  if (auto n = node->GetOpndA()) {
    str = EmitTreeNode(n);
    if(precd > mPrecedence || (precd == mPrecedence && rl_assoc))
      str = '(' + str + ')';
  }
  str += " ? "s;
  if (auto n = node->GetOpndB()) {
    str += EmitTreeNode(n);
  }
  str += " : "s;
  if (auto n = node->GetOpndC()) {
    auto s = EmitTreeNode(n);
    if(precd > mPrecedence)
      s = '(' + s + ')';
    str += s;
  }
  mPrecedence = '\004';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitTypeAliasNode(TypeAliasNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("type ");
  if (auto n = node->GetId()) {
    str += EmitUserTypeNode(n);
  }
  if (auto n = node->GetAlias()) {
    str += " = "s + EmitTreeNode(n);
  }
  mPrecedence = '\030';
  return str;
}

std::string Emitter::EmitConditionalTypeNode(ConditionalTypeNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  Precedence precd = '\030';
  if (auto n = node->GetTypeA()) {
    str = EmitTreeNode(n);
    precd = mPrecedence;
  }
  if (auto n = node->GetTypeB()) {
    if (precd < '\024')
      str = '(' + str + ')';
    str += " extends "s + EmitTreeNode(n);
  }
  if (auto n = node->GetTypeC()) {
    str += " ? "s + EmitTreeNode(n);
  }
  if (auto n = node->GetTypeD()) {
    str += " : "s + EmitTreeNode(n);
  }
  mPrecedence = '\004';
  return str;
}

std::string Emitter::EmitTypeParameterNode(TypeParameterNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetId()) {
    str = EmitTreeNode(n);
  }
  if (auto n = node->GetExtends()) {
    str += " extends "s + EmitTreeNode(n);
  }
  if (auto n = node->GetDefault()) {
    str += " = "s + EmitTreeNode(n);
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitBlockNode(BlockNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if(auto n = node->GetLabel()) {
    str = EmitTreeNode(n) + ":\n"s;
  }
  str += "{\n"s;
  for (unsigned i = 0; i < node->GetChildrenNum(); ++i) {
    if (auto n = node->GetChildAtIndex(i)) {
      str += EmitTreeNode(n) + GetEnding(n);
    }
  }
  str += "}\n"s;
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitNewNode(NewNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("new ");
  if (auto id = node->GetId()) {
    std::string idstr = EmitTreeNode(id);
    auto k = id->GetKind();
    if (k == NK_Call || k == NK_BinOperator)
      idstr = '(' + idstr + ')';
    str += ' ' + idstr;
    if(k != NK_Function && k != NK_Lambda) {
      auto num = node->GetArgsNum();
      str += '(';
      for (unsigned i = 0; i < num; ++i) {
        if (i)
          str += ", "s;
        if (auto n = node->GetArg(i)) {
          str += EmitTreeNode(n);
        }
      }
      str += ')';
    }
  }
  if (auto n = node->GetBody()) {
    str += ' ' + EmitBlockNode(n);
  }
  mPrecedence = '\024';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitDeleteNode(DeleteNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("delete "s);
  if (auto n = node->GetExpr()) {
    str += EmitTreeNode(n);
  }
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitAnnotationTypeNode(AnnotationTypeNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetId()) {
    str += ' ' + EmitIdentifierNode(n);
  }
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitDimensionNode(DimensionNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  for (unsigned i = 0; i < node->GetDimensionsNum(); ++i) {
    auto n = node->GetDimension(i);
    std::string d(n ? std::to_string(n) : ""s);
    str += '[' + d + ']';
  }
  mPrecedence = '\024';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitDeclNode(DeclNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str(Emitter::GetEnumDeclProp(node->GetProp()));
  if (auto n = node->GetVar()) {
    str += ' ' + EmitTreeNode(n);
  }
  if (auto n = node->GetInit()) {
    str += " = "s + EmitTreeNode(n);
  }
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitCastNode(CastNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetDestType()) {
    str += '<' + EmitTreeNode(n) + ">";
  }
  if (auto n = node->GetExpr()) {
    str += EmitTreeNode(n);
  }
  mPrecedence = '\021';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitParenthesisNode(ParenthesisNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetExpr()) {
    str += '(' + EmitTreeNode(n) + ')';
  }
  mPrecedence = '\025';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitFieldNode(FieldNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  const Precedence precd = '\024';
  if (auto n = node->GetUpper()) {
    str = EmitTreeNode(n);
    if (precd > mPrecedence)
      str = '(' + str + ')';
  }
  if (auto n = node->GetField()) {
    str += '.' + EmitIdentifierNode(n);
  }
  mPrecedence = precd;
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitArrayElementNode(ArrayElementNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetArray()) {
    str = EmitTreeNode(n);
    if(mPrecedence < '\024')
      str = '(' + str + ')';
  }
  str = Clean(str);
  for (unsigned i = 0; i < node->GetExprsNum(); ++i) {
    if (auto n = node->GetExprAtIndex(i)) {
      if (str.back() == '?')
        str += '.';
      str += '[' + EmitTreeNode(n) + ']';
    }
  }
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitArrayLiteralNode(ArrayLiteralNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("[");
  for (unsigned i = 0; i < node->GetLiteralsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetLiteral(i)) {
      str += EmitTreeNode(n);
    }
  }
  str += ']';
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitBindingElementNode(BindingElementNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetVariable()) {
    str += EmitTreeNode(n);
  }
  if (auto n = node->GetElement()) {
    if (!str.empty())
      str += ": "s;
    str += EmitTreeNode(n);
  }
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitBindingPatternNode(BindingPatternNode *node) {
  if (node == nullptr)
    return std::string();
  // Needs a flag to distinguish between array destructuring and object destructuring
  // Object destructuring: optional-prop.ts
  // Array destructuring:  trailing-commas.ts
  std::string str;

  for (unsigned i = 0; i < node->GetElementsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetElement(i)) {
      str += EmitTreeNode(n);
    }
  }

  if (node->GetProp() == BPP_ArrayBinding)
    str = '[' + str + ']';
  else
    str = '{' + str + '}';

  if (auto n = node->GetType()) {
    str += ": "s + EmitTreeNode(n);
  }
  if (auto n = node->GetInit()) {
    str += " = "s + EmitTreeNode(n);
  }
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitNumIndexSigNode(NumIndexSigNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetKey())
    str += "[ "s + EmitTreeNode(n) + " : number ]";
  if (auto n = node->GetDataType()) {
    str += " : "s + EmitTreeNode(n);
  }
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitStrIndexSigNode(StrIndexSigNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetKey())
    str += "[ "s + EmitTreeNode(n) + " : string ]";
  if (auto n = node->GetDataType()) {
    str += " : "s + EmitTreeNode(n);
  }
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

static std::string MethodString(std::string &func) {
  size_t s = func.substr(0, 9) == "function " ? 9 : 0;
  return func.back() == '}' ? func.substr(s) + "\n"s : func.substr(s) + ";\n"s;
}

std::string Emitter::EmitStructNode(StructNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  const char *suffix = ";\n";
  switch(node->GetProp()) {
    case SProp_CStruct:
      str = "struct "s;
      break;
    case SProp_TSInterface:
      str = "interface "s;
      break;
    case SProp_TSEnum:
      str = "enum "s;
      suffix = ",\n";
      break;
    case SProp_NA:
      str = ""s;
      break;
    default:
      MASSERT(0 && "Unexpected enumerator");
  }

  if (auto n = node->GetStructId()) {
    str += EmitIdentifierNode(n);
    if (str.substr(0,16) == "AnonymousStruct_")
      str = "class "s + str;
  }

  auto num = node->GetTypeParametersNum();
  if(num) {
    str += '<';
    for (unsigned i = 0; i < num; ++i) {
      if (i)
        str += ", "s;
      if (auto n = node->GetTypeParameterAtIndex(i))
        str += EmitTreeNode(n);
    }
    str += '>';
  }

  for (unsigned i = 0; i < node->GetSupersNum(); ++i) {
    str += i ? ", "s : " extends "s;
    if (auto n = node->GetSuper(i))
      str += EmitTreeNode(n);
  }
  str += " {\n"s;
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    if (auto n = node->GetField(i)) {
      str += EmitTreeNode(n) + suffix;
    }
  }

  if (auto n = node->GetNumIndexSig()) {
    str += EmitNumIndexSigNode(n) + "\n"s;;
  }
  if (auto n = node->GetStrIndexSig()) {
    str += EmitStrIndexSigNode(n) + "\n";
  }

  for (unsigned i = 0; i < node->GetMethodsNum(); ++i) {
    if (auto n = node->GetMethod(i)) {
      std::string func = EmitFunctionNode(n);
      func = Clean(func);
      str += MethodString(func);
    }
  }

  str += "}\n"s;
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitFieldLiteralNode(FieldLiteralNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  auto lit = node->GetLiteral();
  if (auto n = node->GetFieldName()) {
    if(lit && lit->IsFunction() &&
        static_cast<FunctionNode *>(lit)->GetFuncName() == n) {
      str = EmitTreeNode(lit);
      if (str.substr(0, 9) == "function ")
        str = str.substr(9);
      lit = nullptr;
    } else {
      str = EmitTreeNode(n);
      if(lit)
        str += ": "s;
    }
  }
  if(lit) {
    auto s = EmitTreeNode(lit);
    if(s.size() > 4 && (s[0] == 's' || s[0] == 'g') && !s.compare(1, 3, "et "))
      str = s;
    else
      str += s;
  }
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitStructLiteralNode(StructLiteralNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("{");
  auto num = node->GetFieldsNum();
  for (unsigned i = 0; i < num; ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetField(i)) {
      str += EmitFieldLiteralNode(n);
    }
  }

  // Workaround for an identifier issue
  if (num > 1 && str.length() > 6 &&
      (str.substr(str.length() - 6) == ": true" ||
       str.substr(str.length() - 7) == ": false"))
    str += ',';

  str += '}';
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitNamespaceNode(NamespaceNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("namespace ");
  if (auto n = node->GetId()) {
    std::string s = EmitTreeNode(n);
    str += Clean(s);
  }
  str += " {\n"s;
  for (unsigned i = 0; i < node->GetElementsNum(); ++i) {
    if (auto n = node->GetElementAtIndex(i)) {
      str += EmitTreeNode(n) + GetEnding(n);
    }
  }
  str += "}\n"s;
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitVarListNode(VarListNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;

  for (unsigned i = 0; i < node->GetVarsNum(); ++i) {
    if (auto n = node->GetVarAtIndex(i)) {
      str += ' ' + EmitIdentifierNode(n);
    }
  }

  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitExprListNode(ExprListNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;

  for (unsigned i = 0; i < node->GetExprsNum(); ++i) {
    if (auto n = node->GetExprAtIndex(i)) {
      str += ' ' + EmitTreeNode(n);
    }
  }

  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitTemplateLiteralNode(TemplateLiteralNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("`");
  auto num = node->GetTreesNum();
  for (unsigned i = 0; i < num; ++i) {
    if (auto n = node->GetTreeAtIndex(i)) {
      std::string s(EmitTreeNode(n));
      if (i & 0x1)
        str += "${"s + s+ '}';
      else
        str += s.front() == '"' && s.back() == '"' && s.size() >= 2 ? s.substr(1, s.size() - 2) : s;
    }
  }
  str += '`';
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitLiteralNode(LiteralNode *node) {
  if (node == nullptr)
    return std::string();
  LitData lit = node->GetData();
  std::string str(AstDump::GetEnumLitData(lit));
  if(lit.mType == LT_StringLiteral || lit.mType == LT_CharacterLiteral)
    str = "\"" + str + "\"";
  str = HandleTreeNode(str, node);
  if (auto n = node->GetType()) {
    str += ": "s + EmitTreeNode(n);
  }
  if (auto n = node->GetInit()) {
    str += " = "s + EmitTreeNode(n);
  }
  mPrecedence = '\030';
  return str;
}

std::string Emitter::EmitRegExprNode(RegExprNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (const char* e = node->GetData().mExpr)
    str = "/"s + e + '/';
  if (const char* f = node->GetData().mFlags)
    str += f;
  return str;
}

std::string Emitter::EmitThrowNode(ThrowNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("throw ");
  for (unsigned i = 0; i < node->GetExceptionsNum(); ++i) {
    if (auto n = node->GetExceptionAtIndex(i)) {
      str += EmitTreeNode(n);
    }
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitCatchNode(CatchNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("catch(");
  for (unsigned i = 0; i < node->GetParamsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetParamAtIndex(i)) {
      str += EmitTreeNode(n);
    }
  }
  str += ')';
  if (auto n = node->GetBlock()) {
    str += EmitBlockNode(n);
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitFinallyNode(FinallyNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("finally ");
  if (auto n = node->GetBlock()) {
    str += EmitBlockNode(n);
  }
  else
    str += "{}\n"s;
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitTryNode(TryNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("try ");
  if (auto n = node->GetBlock()) {
    str += EmitBlockNode(n);
  }
  for (unsigned i = 0; i < node->GetCatchesNum(); ++i) {
    if (auto n = node->GetCatchAtIndex(i)) {
      str += EmitCatchNode(n);
    }
  }
  if (auto n = node->GetFinally()) {
    str += EmitFinallyNode(n);
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitExceptionNode(ExceptionNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetException()) {
    str += ' ' + EmitIdentifierNode(n);
  }
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitReturnNode(ReturnNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("return");
  if (auto n = node->GetResult()) {
    str += ' ' + EmitTreeNode(n);
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitCondBranchNode(CondBranchNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if(auto n = node->GetLabel()) {
    str = EmitTreeNode(n) + ":\n"s;
  }
  str += "if("s;
  if (auto n = node->GetCond()) {
    auto cond = EmitTreeNode(n);
    str += Clean(cond);
  }
  str += ")\n"s;
  if (auto n = node->GetTrueBranch()) {
    str += EmitTreeNode(n) + GetEnding(n);
  }
  if (auto n = node->GetFalseBranch()) {
    str += "else\n"s + EmitTreeNode(n) + GetEnding(n);
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitBreakNode(BreakNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("break");
  if (auto n = node->GetTarget()) {
    str += ' ' + EmitTreeNode(n);
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitContinueNode(ContinueNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("continue");
  if (auto n = node->GetTarget()) {
    str += ' ' + EmitTreeNode(n);
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitForLoopNode(ForLoopNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if(auto n = node->GetLabel()) {
    str = EmitTreeNode(n) + ":\n"s;
  }
  str += "for("s;
  switch(node->GetProp()) {
    case FLP_Regular:
      {
        for (unsigned i = 0; i < node->GetInitsNum(); ++i)
          if (auto n = node->GetInitAtIndex(i)) {
            std::string init = EmitTreeNode(n);
            if (i) {
              str += ", "s;
              if(init.substr(0, 4) == "let " || init.substr(0, 4) == "var ")
                init = init.substr(4);
              else if(init.substr(0, 6) == "const ")
                init = init.substr(6);
            }
            str += init;
          }
        str += "; "s;
        if (auto n = node->GetCond()) {
          str += EmitTreeNode(n);
        }
        str += "; "s;
        for (unsigned i = 0; i < node->GetUpdatesNum(); ++i)
          if (auto n = node->GetUpdateAtIndex(i)) {
            if (i)
              str += ", "s;
            str += EmitTreeNode(n);
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
  str += ')';

  if (auto n = node->GetBody()) {
    str += EmitTreeNode(n) + GetEnding(n);
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitWhileLoopNode(WhileLoopNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if(auto n = node->GetLabel()) {
    str = EmitTreeNode(n) + ":\n"s;
  }
  str += "while("s;
  if (auto n = node->GetCond()) {
    str += EmitTreeNode(n);
  }
  str += ')';
  if (auto n = node->GetBody()) {
    str += EmitTreeNode(n) + GetEnding(n);
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitDoLoopNode(DoLoopNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if(auto n = node->GetLabel()) {
    str = EmitTreeNode(n) + ":\n"s;
  }
  str += "do "s;
  if (auto n = node->GetBody()) {
    str += EmitTreeNode(n);
  }
  str += "while("s;
  if (auto n = node->GetCond()) {
    str += EmitTreeNode(n);
  }
  str += ");\n"s;
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitSwitchLabelNode(SwitchLabelNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if(node->IsDefault())
    str += "default:\n"s;
  if(auto n = node->GetValue()) {
    auto ce = EmitTreeNode(n);
    str += "case "s + Clean(ce) + ":\n"s;
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitSwitchCaseNode(SwitchCaseNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  for (unsigned i = 0; i < node->GetLabelsNum(); ++i) {
    if (auto n = node->GetLabelAtIndex(i)) {
      str += EmitTreeNode(n);
    }
  }
  for (unsigned i = 0; i < node->GetStmtsNum(); ++i) {
    if (auto n = node->GetStmtAtIndex(i))
      str += EmitTreeNode(n) + GetEnding(n);
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitSwitchNode(SwitchNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if(auto n = node->GetLabel()) {
    str = EmitTreeNode(n) + ":\n"s;
  }
  str += "switch("s;
  if (auto n = node->GetExpr()) {
    auto expr = EmitTreeNode(n);
    str += Clean(expr);
  }
  str += "){\n"s;
  for (unsigned i = 0; i < node->GetCasesNum(); ++i) {
    if(auto n = node->GetCaseAtIndex(i))
      str += EmitTreeNode(n);
  }
  str += "}\n"s;
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitAssertNode(AssertNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetExpr()) {
    str += ' ' + EmitTreeNode(n);
  }
  if (auto n = node->GetMsg()) {
    str += ' ' + EmitTreeNode(n);
  }
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitCallNode(CallNode *node) {
  if (node == nullptr)
    return std::string();
  // Function call: left-to-right, precedence = 20
  std::string str;
  if (auto n = node->GetMethod()) {
    std::string s = EmitTreeNode(n);
    bool optional = n->IsOptional();
    if (optional && !s.empty() && s.back() == '?')
      s.pop_back();
    auto k = n->GetKind();
    if(k == NK_Function || k == NK_Lambda)
      str += '(' + s + ')';
    else
      str += s;
    if (optional)
      str += "?."s; // for optional chaining
  }
  if(auto num = node->GetTypeArgumentsNum()) {
    str += '<';
    for (unsigned i = 0; i < num; ++i) {
      if (i)
        str += ", "s;
      if (auto n = node->GetTypeArgumentAtIndex(i)) {
        str += EmitTreeNode(n);
      }
    }
    str += '>';
  }
  str += '(';
  for (unsigned i = 0; i < node->GetArgsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetArg(i))
      str += EmitTreeNode(n);
  }
  str += ')';
  mPrecedence = '\024';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitInterfaceNode(InterfaceNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = "interface "s + node->GetName();

  /*
  auto num = node->GetTypeParametersNum();
  if(num) {
    str += '<';
    for (unsigned i = 0; i < num; ++i) {
      if (i)
        str += ", "s;
      if (auto n = node->GetTypeParameterAtIndex(i))
        str += EmitTreeNode(n);
    }
    str += '>';
  }
  */

  for (unsigned i = 0; i < node->GetSuperInterfacesNum(); ++i) {
    str += i ? ", "s : " implements "s;
    if (auto n = node->GetSuperInterfaceAtIndex(i))
      str += EmitTreeNode(n);
  }
  str += " {\n"s;

  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    if (auto n = node->GetField(i)) {
      str += EmitIdentifierNode(n) + ";\n"s;
    }
  }

  for (unsigned i = 0; i < node->GetMethodsNum(); ++i) {
    if (auto n = node->GetMethod(i)) {
      std::string func = EmitFunctionNode(n);
      func = Clean(func);
      str += MethodString(func);
    }
  }

  str += "}\n"s;
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitClassNode(ClassNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  for (unsigned i = 0; i < node->GetAnnotationsNum(); ++i)
    if (auto n = node->GetAnnotationAtIndex(i))
      str += '@' + EmitTreeNode(n) + "\n"s;

  for (unsigned i = 0; i < node->GetAttributesNum(); ++i)
    str += GetEnumAttrId(node->GetAttribute(i));

  str += "class "s + node->GetName();

  auto num = node->GetTypeParametersNum();
  if(num) {
    str += '<';
    for (unsigned i = 0; i < num; ++i) {
      if (i)
        str += ", "s;
      if (auto n = node->GetTypeParameterAtIndex(i))
        str += EmitTreeNode(n);
    }
    str += '>';
  }

  auto classNum = node->GetSuperClassesNum();
  for (unsigned i = 0; i < classNum; ++i) {
    str += i ? ", "s : " extends "s;
    if (auto n = node->GetSuperClass(i))
      str += EmitTreeNode(n);
  }
  for (unsigned i = 0; i < node->GetSuperInterfacesNum(); ++i) {
    str += i ? ", "s : " implements "s;
    if (auto n = node->GetSuperInterface(i))
      str += EmitTreeNode(n);
  }
  str += " {\n"s;

  for (unsigned i = 0; i < node->GetDeclaresNum(); ++i) {
    if (auto n = node->GetDeclare(i)) {
      std::string s = EmitTreeNode(n);
      Replace(s, " var ", " "); // TODO: JS_Var for field
      str += s + ";\n"s;
    }
  }

  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    if (auto n = node->GetField(i)) {
      str += EmitTreeNode(n) + ";\n"s;
    }
  }

  for (unsigned i = 0; i < node->GetConstructorsNum(); ++i) {
    if (auto n = node->GetConstructor(i)) {
      std::string func = EmitFunctionNode(n);
      if (func.substr(0, 9) == "function ")
        func = func.substr(9);
      func = Clean(func);
      str += func.back() == '}' ? func + "\n"s : func + ";\n"s;
    }
  }

  for (unsigned i = 0; i < node->GetMethodsNum(); ++i) {
    if (FunctionNode *n = node->GetMethod(i)) {
      std::string func = EmitFunctionNode(n);
      func = Clean(func);
      str += MethodString(func);
    }
  }

  for (unsigned i = 0; i < node->GetInstInitsNum(); ++i) {
    if (auto n = node->GetInstInit(i)) {
      str += EmitBlockNode(n) + "\n"s;
    }
  }

  for (unsigned i = 0; i < node->GetLocalClassesNum(); ++i) {
    if (auto n = node->GetLocalClass(i)) {
      str += EmitClassNode(n) + "\n"s;
    }
  }

  for (unsigned i = 0; i < node->GetLocalInterfacesNum(); ++i) {
    if (auto n = node->GetLocalInterface(i)) {
      str += EmitInterfaceNode(n) + "\n"s;
    }
  }

  str += "}\n";
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitPassNode(PassNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("PassNode {");

  for (unsigned i = 0; i < node->GetChildrenNum(); ++i) {
    if (auto n = node->GetChild(i)) {
      str += ' ' + EmitTreeNode(n);
    }
  }

  str += '}';
  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitLambdaNode(LambdaNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  switch (node->GetProperty()) {
    case LP_JSArrowFunction:
      break;
    case LP_JavaLambda:
    case LP_NA:
    default:
      MASSERT(0 && "Unexpected enumerator");
  }

  auto num = node->GetTypeParametersNum();
  if(num) {
    str += '<';
    for (unsigned i = 0; i < num; ++i) {
      if (i)
        str += ", "s;
      if (auto n = node->GetTypeParameterAtIndex(i)) {
        str += EmitTreeNode(n);
      }
    }
    str += '>';
  }

  str += '(';
  for (unsigned i = 0; i < node->GetParamsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetParam(i)) {
      str += EmitTreeNode(n);
    }
  }
  str += ')';

  if (auto n = node->GetBody()) {
    if (auto t = node->GetType()) {
      str += ": "s + EmitTreeNode(t);
    }
    std::string s = EmitTreeNode(n);
    s = Clean(s);
    if (n->IsStructLiteral())
      s = '(' + s + ')';
    str += " => "s + s;
  }
  else {
    if (auto t = node->GetType()) {
      str += " => "s + EmitTreeNode(t);
    }
  }

  mPrecedence = '\004';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitInstanceOfNode(InstanceOfNode *node) {
  if (node == nullptr)
    return std::string();
  const Precedence precd = '\014';
  const bool rl_assoc = false;         // false: left-to-right
  std::string lhs, rhs;
  if (auto n = node->GetLeft()) {
    lhs = EmitTreeNode(n);
    if(precd > mPrecedence)
      lhs = '(' + lhs + ')';
  }
  else
    lhs = "(NIL) "s;
  if (auto n = node->GetRight()) {
    rhs = EmitTreeNode(n);
    if(precd > mPrecedence || (precd == mPrecedence && !rl_assoc))
      rhs = '(' + rhs + ')';
  }
  else
    rhs = " (NIL)"s;
  std::string str(lhs + " instanceof "s + rhs);
  mPrecedence = precd;
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitTypeOfNode(TypeOfNode *node) {
  if (node == nullptr)
    return std::string();
  const Precedence precd = '\121' & 0x3f;
  std::string str("typeof "s), rhs;
  if (auto n = node->GetExpr()) {
    rhs = EmitTreeNode(n);
    if(precd > mPrecedence) // right-to-left
      rhs = '(' + rhs + ')';
  }
  else
    rhs = " (NIL)"s;
  str += rhs;
  mPrecedence = precd;
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitKeyOfNode(KeyOfNode *node) {
  if (node == nullptr)
    return std::string();
  const Precedence precd = '\121' & 0x3f;
  std::string str("keyof "s), rhs;
  if (auto n = node->GetExpr()) {
    rhs = EmitTreeNode(n);
    if(precd > mPrecedence)
      rhs = '(' + rhs + ')';
  }
  else
    rhs = " (NIL)"s;
  str += rhs;
  mPrecedence = precd;
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitInferNode(InferNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("infer ");
  if (auto n = node->GetExpr()) {
    str += EmitTreeNode(n);
  }
  mPrecedence = '\004';
  return str;
}

std::string Emitter::EmitInNode(InNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetLeft()) {
    str += EmitTreeNode(n);
  }
  str += " in "s;
  if (auto n = node->GetRight()) {
    str += EmitTreeNode(n);
  }
  mPrecedence = '\014';
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitIsNode(IsNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetLeft()) {
    str = EmitTreeNode(n);
  }
  str += " is "s;
  if (auto n = node->GetRight()) {
    str += EmitTreeNode(n);
  }
  return str;
}

std::string Emitter::EmitNameTypePairNode(NameTypePairNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetVar()) {
    str += EmitTreeNode(n) + ": "s;
  }
  if (auto n = node->GetType()) {
    str += EmitTreeNode(n);
  }
  return str;
}

std::string Emitter::EmitTupleTypeNode(TupleTypeNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("[ ");

  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetField(i)) {
      str += EmitNameTypePairNode(n);
    }
  }
  str += " ]"s;

  mPrecedence = '\030';
  return str;
}
std::string Emitter::EmitModuleNode(ModuleNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("// Filename: "s);
  str += node->GetFilename() + "\n"s;
  //str += AstDump::GetEnumSrcLang(node->GetSrcLang());
  /*
  if (auto n = node->GetPackage()) {
    str += ' ' + EmitPackageNode(n);
  }

  for (unsigned i = 0; i < node->GetImportsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetImport(i)) {
      str += ' ' + EmitImportNode(n);
    }
  }
  */
  for (unsigned i = 0; i < node->GetTreesNum(); ++i) {
    if (auto n = node->GetTree(i)) {
      str += EmitTreeNode(n) + GetEnding(n);
    }
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitAttrNode(AttrNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str(GetEnumAttrId(node->GetId()));
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitPrimTypeNode(PrimTypeNode *node) {
  if (node == nullptr)
    return std::string();
  auto k = node->GetPrimType();
  std::string str = k == TY_None ? std::string() : Emitter::GetEnumTypeId(k);
  if (node->IsUnique())
    str = "unique "s + str;
  mPrecedence = '\030';
  return str;
}

std::string Emitter::EmitPrimArrayTypeNode(PrimArrayTypeNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str, accessor;
  for (unsigned i = 0; i < node->GetAttrsNum(); ++i) {
    std::string s = GetEnumAttrId(node->GetAttrAtIndex(i));
    if (s == "get "s || s == "set "s)
      accessor += s;
    else
      str += s;
  }
  str += accessor;
  if (auto n = node->GetPrim()) {
    str += EmitPrimTypeNode(n);
  }
  if (auto n = node->GetDims()) {
    str += EmitDimensionNode(n);
    Replace(str, "never[", "[");
  }
  return HandleTreeNode(str, node);
}

std::string Emitter::EmitTreeNode(TreeNode *node) {
  if (node == nullptr)
    return std::string();
  switch (node->GetKind()) {
  case NK_Module:
    return EmitModuleNode(static_cast<ModuleNode *>(node));
    break;
  case NK_Package:
    return EmitPackageNode(static_cast<PackageNode *>(node));
    break;
  case NK_XXportAsPair:
    return EmitXXportAsPairNode(static_cast<XXportAsPairNode *>(node));
    break;
  case NK_Import:
    return EmitImportNode(static_cast<ImportNode *>(node));
    break;
  case NK_Export:
    return EmitExportNode(static_cast<ExportNode *>(node));
    break;
  case NK_Declare:
    return EmitDeclareNode(static_cast<DeclareNode *>(node));
    break;
  case NK_Decl:
    return EmitDeclNode(static_cast<DeclNode *>(node));
    break;
  case NK_Identifier:
    return EmitIdentifierNode(static_cast<IdentifierNode *>(node));
    break;
  case NK_Field:
    return EmitFieldNode(static_cast<FieldNode *>(node));
    break;
  case NK_Dimension:
    return EmitDimensionNode(static_cast<DimensionNode *>(node));
    break;
  case NK_Attr:
    return EmitAttrNode(static_cast<AttrNode *>(node));
    break;
  case NK_NameTypePair:
    return EmitNameTypePairNode(static_cast<NameTypePairNode *>(node));
    break;
  case NK_PrimType:
    return EmitPrimTypeNode(static_cast<PrimTypeNode *>(node));
    break;
  case NK_PrimArrayType:
    return EmitPrimArrayTypeNode(static_cast<PrimArrayTypeNode *>(node));
    break;
  case NK_UserType:
    return EmitUserTypeNode(static_cast<UserTypeNode *>(node));
    break;
  case NK_TypeParameter:
    return EmitTypeParameterNode(static_cast<TypeParameterNode *>(node));
    break;
  case NK_AsType:
    return EmitAsTypeNode(static_cast<AsTypeNode *>(node));
    break;
  case NK_TypeAlias:
    return EmitTypeAliasNode(static_cast<TypeAliasNode *>(node));
    break;
  case NK_ConditionalType:
    return EmitConditionalTypeNode(static_cast<ConditionalTypeNode *>(node));
    break;
  case NK_TupleType:
    return EmitTupleTypeNode(static_cast<TupleTypeNode *>(node));
    break;
  case NK_Cast:
    return EmitCastNode(static_cast<CastNode *>(node));
    break;
  case NK_Parenthesis:
    return EmitParenthesisNode(static_cast<ParenthesisNode *>(node));
    break;
  case NK_BindingElement:
    return EmitBindingElementNode(static_cast<BindingElementNode *>(node));
    break;
  case NK_BindingPattern:
    return EmitBindingPatternNode(static_cast<BindingPatternNode *>(node));
    break;
  case NK_Struct:
    return EmitStructNode(static_cast<StructNode *>(node));
    break;
  case NK_StructLiteral:
    return EmitStructLiteralNode(static_cast<StructLiteralNode *>(node));
    break;
  case NK_FieldLiteral:
    return EmitFieldLiteralNode(static_cast<FieldLiteralNode *>(node));
    break;
  case NK_NumIndexSig:
    return EmitNumIndexSigNode(static_cast<NumIndexSigNode *>(node));
    break;
  case NK_StrIndexSig:
    return EmitStrIndexSigNode(static_cast<StrIndexSigNode *>(node));
    break;
  case NK_ComputedName:
    return EmitComputedNameNode(static_cast<ComputedNameNode *>(node));
    break;
  case NK_ArrayElement:
    return EmitArrayElementNode(static_cast<ArrayElementNode *>(node));
    break;
  case NK_ArrayLiteral:
    return EmitArrayLiteralNode(static_cast<ArrayLiteralNode *>(node));
    break;
  case NK_VarList:
    return EmitVarListNode(static_cast<VarListNode *>(node));
    break;
  case NK_ExprList:
    return EmitExprListNode(static_cast<ExprListNode *>(node));
    break;
  case NK_TemplateLiteral:
    return EmitTemplateLiteralNode(static_cast<TemplateLiteralNode *>(node));
    break;
  case NK_RegExpr:
    return EmitRegExprNode(static_cast<RegExprNode *>(node));
    break;
  case NK_Literal:
    return EmitLiteralNode(static_cast<LiteralNode *>(node));
    break;
  case NK_UnaOperator:
    return EmitUnaOperatorNode(static_cast<UnaOperatorNode *>(node));
    break;
  case NK_BinOperator:
    return EmitBinOperatorNode(static_cast<BinOperatorNode *>(node));
    break;
  case NK_TerOperator:
    return EmitTerOperatorNode(static_cast<TerOperatorNode *>(node));
    break;
  case NK_Lambda:
    return EmitLambdaNode(static_cast<LambdaNode *>(node));
    break;
  case NK_InstanceOf:
    return EmitInstanceOfNode(static_cast<InstanceOfNode *>(node));
    break;
  case NK_TypeOf:
    return EmitTypeOfNode(static_cast<TypeOfNode *>(node));
    break;
  case NK_KeyOf:
    return EmitKeyOfNode(static_cast<KeyOfNode *>(node));
    break;
  case NK_In:
    return EmitInNode(static_cast<InNode *>(node));
    break;
  case NK_Is:
    return EmitIsNode(static_cast<IsNode *>(node));
    break;
  case NK_Infer:
    return EmitInferNode(static_cast<InferNode *>(node));
    break;
  case NK_Block:
    return EmitBlockNode(static_cast<BlockNode *>(node));
    break;
  case NK_Function:
    return EmitFunctionNode(static_cast<FunctionNode *>(node));
    break;
  case NK_Class:
    return EmitClassNode(static_cast<ClassNode *>(node));
    break;
  case NK_Interface:
    return EmitInterfaceNode(static_cast<InterfaceNode *>(node));
    break;
  case NK_Namespace:
    return EmitNamespaceNode(static_cast<NamespaceNode *>(node));
    break;
  case NK_AnnotationType:
    return EmitAnnotationTypeNode(static_cast<AnnotationTypeNode *>(node));
    break;
  case NK_Annotation:
    return EmitAnnotationNode(static_cast<AnnotationNode *>(node));
    break;
  case NK_Try:
    return EmitTryNode(static_cast<TryNode *>(node));
    break;
  case NK_Catch:
    return EmitCatchNode(static_cast<CatchNode *>(node));
    break;
  case NK_Finally:
    return EmitFinallyNode(static_cast<FinallyNode *>(node));
    break;
  case NK_Exception:
    return EmitExceptionNode(static_cast<ExceptionNode *>(node));
    break;
  case NK_Throw:
    return EmitThrowNode(static_cast<ThrowNode *>(node));
    break;
  case NK_Return:
    return EmitReturnNode(static_cast<ReturnNode *>(node));
    break;
  case NK_CondBranch:
    return EmitCondBranchNode(static_cast<CondBranchNode *>(node));
    break;
  case NK_Break:
    return EmitBreakNode(static_cast<BreakNode *>(node));
    break;
  case NK_Continue:
    return EmitContinueNode(static_cast<ContinueNode *>(node));
    break;
  case NK_ForLoop:
    return EmitForLoopNode(static_cast<ForLoopNode *>(node));
    break;
  case NK_WhileLoop:
    return EmitWhileLoopNode(static_cast<WhileLoopNode *>(node));
    break;
  case NK_DoLoop:
    return EmitDoLoopNode(static_cast<DoLoopNode *>(node));
    break;
  case NK_New:
    return EmitNewNode(static_cast<NewNode *>(node));
    break;
  case NK_Delete:
    return EmitDeleteNode(static_cast<DeleteNode *>(node));
    break;
  case NK_Call:
    return EmitCallNode(static_cast<CallNode *>(node));
    break;
  case NK_Assert:
    return EmitAssertNode(static_cast<AssertNode *>(node));
    break;
  case NK_SwitchLabel:
    return EmitSwitchLabelNode(static_cast<SwitchLabelNode *>(node));
    break;
  case NK_SwitchCase:
    return EmitSwitchCaseNode(static_cast<SwitchCaseNode *>(node));
    break;
  case NK_Switch:
    return EmitSwitchNode(static_cast<SwitchNode *>(node));
    break;
  case NK_Pass:
    return EmitPassNode(static_cast<PassNode *>(node));
    break;
  case NK_Null:
    // Ignore NullNode
    break;
  default:
    MASSERT(0 && "Unexpected node kind");
  }
  return std::string();
}

static std::string &AddParentheses(std::string &str, TreeNode *node) {
  if (!node->IsIdentifier() && !node->IsLiteral() && !node->IsArrayLiteral())
    str = '(' + str + ')';
  return str;
}

std::string &Emitter::HandleTreeNode(std::string &str, TreeNode *node) {
  auto num = node->GetAsTypesNum();
  if(num > 0) {
    str = "(("s + str + ')';
    for (unsigned i = 0; i < num; ++i)
      if (auto t = node->GetAsTypeAtIndex(i))
        str += EmitAsTypeNode(t);
    str += ')';
  }
  if(node->IsOptional())
    str = AddParentheses(str, node) + '?';
  if(node->IsNonNull())
    str = AddParentheses(str, node) + '!';
  if(node->IsRest())
    str = "..."s + AddParentheses(str, node);
  if(node->IsConst())
    if(node->IsField())
      str += " as const"s;
    else
      str = AddParentheses(str, node) + " as const"s;
  return str;
}

std::string Emitter::GetEnumTypeId(TypeId k) {
  std::string str(AstDump::GetEnumTypeId(k) + 3);
  if (k != TY_Function && k != TY_Object)
    str[0] = std::tolower(str[0]);
  return str;
}

std::string Emitter::GetEnumDeclProp(DeclProp k) {
  std::string str(AstDump::GetEnumDeclProp(k) + 3);
  if(str != "NA")
    str[0] = std::tolower(str[0]);
  return str;
}

const char *Emitter::GetEnumOprId(OprId k) {
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
  case OPR_Exp:
    return          "\120**";
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
  case OPR_NullAssign:
    return          "\103??=";
  case OPR_Arrow:
    return          "\030 OPR_Arrow";
  case OPR_Diamond:
    return          "\030 OPR_Diamond";
  case OPR_StEq:
    return          "\013===";
  case OPR_StNe:
    return          "\013!==";
  case OPR_ArrowFunction:
    return          "\030 OPR_ArrowFunction";
  case OPR_NullCoalesce:
    return          "\005??";
  case OPR_NA:
    return          "\030 OPR_NA";
  default:
    MASSERT(0 && "Unexpected enumerator");
  }
  return "UNEXPECTED OprId";
}

} // namespace maplefe
