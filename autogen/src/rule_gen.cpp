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
#include "massert.h"
#include "rule_gen.h"
#include "buffer2write.h"
#include "base_gen.h"
#include "token_table.h"

namespace maplefe {

// Generate one initialization function for each rule. Below is the elaboration of most of
// of rule case.
//
// 1) rule NonZeroDigit   : ONEOF('1', '2', '3', '4', '5', '6', '7', '8', '9')
//
//                                   ==>
//
//    TableData TblNonZeroDigit_data[9] = {{DT_Char, {.mChar='1'}}, ... {DT_Char, {.mChar='9'}}};
//    RuleTable TblNonZeroDigit = {ET_Oneof, 9, TblNonZeroDigit_data};
//
// 2) rule Underscores    : '_' + ZEROORMORE('_')
//
//                                   ==>
//
//    TableData TblUnderscores_sub1_data[1] = {{DT_Char, {.mChar='_'}}};
//    RuleTable TblUnderscores_sub1 = {ET_Zeroormore, 1, TblUnderscores_sub1_data};
//
//    TableData TblUnderscores_data[2] = {{DT_Char, {.mChar='_'}}, {DT_Subtable, {.mEntry=&TblUnderscores_sub1}}};
//    RuleTable TblUnderscores = {ET_Concatenate, 2, TblUnderscores_data};
//
//    So look carefully at the RuleTable of a rule, it contains two types: an ET_xxx
//    and a set of DT_xxx in the data set.
//
// 3) If the rule has attr
//
//   rule AdditiveExpression : ONEOF(
//     MultiplicativeExpression,
//     AdditiveExpression + '+' + MultiplicativeExpression,
//     AdditiveExpression + '-' + MultiplicativeExpression)
//     attr.action.%2,%3 : BuildBinaryOperation(%1, %2, %3)
//
//                                   ==>
//
// TableData TblAdditiveExpression_sub1_data[3] ={{DT_Subtable, &TblAdditiveExpression},
//                                                {DT_Char, {.mChar='+'}},
//                                                {DT_Subtable, &TblMultiplicativeExpression}};
// Action TblAdditiveExpression_sub1_action[1] = {{ACT_BuildBinaryOperation, 1, 2, 3}};
// RuleTable TblAdditiveExpression_sub1 ={ET_Concatenate, 3, TblAdditiveExpression_sub1_data,
//                                                        1, &TblAdditiveExpression_sub1_action};
//
// TableData TblAdditiveExpression_sub2_data[3] ={{DT_Subtable, &TblAdditiveExpression},
//                                                {DT_Char, {.mChar='-'}},
//                                                {DT_Subtable, &TblMultiplicativeExpression}};
// Action TblAdditiveExpression_sub2_action[1] = {{ACT_BuildBinaryOperation, 1, 2, 3}};
// RuleTable TblAdditiveExpression_sub2 ={ET_Concatenate, 3, TblAdditiveExpression_sub2_data,
//                                                        1, &TblAdditiveExpression_sub2_action};
//
// TableData TblAdditiveExpression_data[3] ={{DT_Subtable, &TblMultiplicativeExpression},
//                                           {DT_Subtable, &TblAdditiveExpression_sub1},
//                                           {DT_Subtable, &TblAdditiveExpression_sub2}};
// RuleTable TblAdditiveExpression ={ET_Oneof, 3, TblAdditiveExpression_data, 0, NULL};



// Generate the table name for mRule
std::string RuleGen::GetTblName(const Rule *rule) {
  std::string tn = "Tbl" + rule->mName;
  return tn;
}

// Generate the table name for sub rules in mRule
// The table id will be introduced here
std::string RuleGen::GetSubTblName() {
  std::string tn = "Tbl" + mRule->mName + "_sub" + std::to_string(mSubTblNum);
  return tn;
}

std::string RuleGen::GetPropertyName(const RuleAttr *attr) {
  if (!attr)
    return "RP_NA";

  unsigned size = attr->mProperty.size();
  if (size == 0)
    return "RP_NA";

  std::string name;
  for (unsigned i = 0; i < size; i++) {
    std::string s = "RP_";
    s += attr->mProperty[i];
    if (i < size - 1)
      s += "|";
    name += s;
  }

  return name;
}

// Return the string name of an entry type
// In Rule, it has two info <type,op> to decide an element's type
// but in the generated rule table it has only one info, EntryType
//
std::string RuleGen::GetEntryTypeName(ElemType type, RuleOp op) {
  std::string name;
  switch(type) {
  case ET_Char:
  case ET_String:
  case ET_Rule:
  case ET_Type:
  case ET_Token:
    name = "ET_Data";
    break;
  case ET_Op: {
    switch(op) {
    case RO_Oneof:
      name = "ET_Oneof";
      break;
    case RO_Zeroormore:
      name = "ET_Zeroormore";
      break;
    case RO_Zeroorone:
      name = "ET_Zeroorone";
      break;
    case RO_Concatenate:
      name = "ET_Concatenate";
      break;
    default:
      MERROR("unknown RuleOp");
      break;
    }
    break;
  }
  default:
    MERROR("unknown ElemType");
    break;
  }
  return name;
}

// Return the string for one rule element
// Keep in mind, this is already in the data set part, so only DT_xxx
// is generated.
std::string RuleGen::Gen4RuleElem(const RuleElem *elem) {
  std::string data;
  data = '{';

  switch(elem->mType) {
  // character need special handling of escape character, please see comments
  // in StringToValue::StringToString() in shared/stringutil.cpp.
  case ET_Char:
    data += "DT_Char, {.mChar=\'";
    if (elem->mData.mChar == '\\')
      data += "\\\\";
    else if (elem->mData.mChar == '\'')
      data += "\\\'";
    else
      data += elem->mData.mChar;
    data += "\'}";
    break;
  case ET_String:
    data += "DT_String, {.mString=\"";
    data += elem->mData.mString;
    data += "\"}";
    break;
  case ET_Type:
    data += "DT_Type, {.mTypeId=TY_";
    data += GetTypeString(elem->mData.mTypeId);
    data += "}";
    break;
  case ET_Token: {
    data += "DT_Token, {.mTokenId=";
    std::string id_str = std::to_string(elem->mData.mTokenId);
    data += id_str;
    data += "}";
    break;
  }
  case ET_Rule:
    // Rule has its own table generated, so just need insert the table name.
    // Note: The table could be defined in other files. Need include them.
    data += "DT_Subtable, &";
    data += GetTblName(elem->mData.mRule);
    break;
  case ET_Op: {
    // Each Op will be generated as a new sub table
    mSubTblNum++;
    std::string tbl_name = GetSubTblName();
    data += "DT_Subtable, &";
    data += tbl_name;
    Gen4Table(NULL, elem);
    break;
  }
  default:
    MERROR("unknown ElemType");
    break;
  }

  data += "}";
  return data;
}

// generates TableData
std::string RuleGen::Gen4TableData(const RuleElem *elem) {
  std::string table_data;

  // see comments in Gen4Table(), there could be cases with ZERO mSubElems
  // then 'elem' itself is the data to be output.
  if (elem->mSubElems.size() == 0) {
    table_data = Gen4RuleElem(elem);
  } else {
    std::vector<RuleElem *>::const_iterator it = elem->mSubElems.begin();
    unsigned idx = 0;
    for(; it != elem->mSubElems.end(); it++, idx++) {
      RuleElem *it_elem = *it;
      std::string str = Gen4RuleElem(it_elem);
      table_data += str;
      if (idx < elem->mSubElems.size() - 1)
        table_data += ",";
    }
  }
  return table_data;
}

void RuleGen::Gen4TableHeader(const std::string &rule_table_name){
  std::string extern_decl;
  extern_decl = "extern RuleTable ";
  extern_decl += rule_table_name;
  extern_decl += ";";
  mHeaderBuffer->NewOneBuffer(extern_decl.size(), true);
  mHeaderBuffer->AddStringWholeLine(extern_decl);
}

void RuleGen::GenDebug(const std::string &rule_table_name) {
  std::string addr_name_mapping;
  addr_name_mapping = "{&";
  addr_name_mapping += rule_table_name;
  addr_name_mapping += ", \"";
  addr_name_mapping += rule_table_name;
  addr_name_mapping += "\", ";
  addr_name_mapping += std::to_string(gRuleTableNum);
  addr_name_mapping += "}\,";
  gSummaryCppFile->WriteOneLine(addr_name_mapping.c_str(), addr_name_mapping.size());
  gRuleTableNum++;
}

// The format of RuleAttr table is like below,
//   Action TblAdditiveExpression_sub1_action[2] = {
//           {ACT_BuildBinaryOperation, 3, {1, 2, 3}}, {ACT_XXX, 2, {2, 3}}};
//
// For the time being, I'll generate only actions. The validity will be done later

void RuleGen::Gen4RuleAttr(std::string rule_table_name, const RuleAttr *attr) {
  std::string attr_table;

  if (attr->mAction.size() == 0)
    return;

  attr_table += "Action ";
  attr_table += rule_table_name;
  attr_table += "_action[";
  attr_table += std::to_string(attr->mAction.size());
  attr_table += "] = {";

  // Add all actions
  for (unsigned i = 0; i < attr->mAction.size(); i++) {
    RuleAction *action = attr->mAction[i];
    // The validity of action->mName should be already checked by the SPECParser
    attr_table += "{ACT_";
    attr_table += action->mName;
    attr_table += ",";

    // the number of element index should be already checked by SPECParser,
    // it should be <= MAX_ACT_ELEM_NUM
    attr_table += std::to_string(action->mArgs.size());
    attr_table += ",";

    // action element
    attr_table += "{";
    for (unsigned j = 0; j < action->mArgs.size(); j++) {
      attr_table += std::to_string(action->mArgs[j]);
      if (j < action->mArgs.size() - 1)
        attr_table += ",";
    }
    attr_table += "}";

    // end of this action
    attr_table += "}";
    if (i < attr->mAction.size() - 1)
      attr_table += ",";
  }

  attr_table += "};";

  mCppBuffer->NewOneBuffer(attr_table.size(), true);
  mCppBuffer->AddStringWholeLine(attr_table);
}

// Either rule or elem is used.
// If it's a rule, we are generating for a rule defined in .spec
// If it's an elem, we are generating a sub table for an elemen,
//    and the table name has a suffix "_sub".
void RuleGen::Gen4Table(const Rule *rule, const RuleElem *elem){
  std::string rule_table_name;
  const RuleAttr *attr;

  if(rule) {
    rule_table_name = GetTblName(rule);
    elem = rule->mElement;
    attr = &(rule->mAttr);
  } else {
    rule_table_name = GetSubTblName();
    attr = &(elem->mAttr);
  }

  // Check if it's a top rule. We only check for 'rule', since
  // 'elem' can NOT be a top rule.
  if (rule) {
    std::vector<std::string> properties = attr->mProperty;
    std::vector<std::string>::iterator p_it = properties.begin();
    for (; p_it != properties.end(); p_it++) {
      std::string p = *p_it;
      std::size_t found = p.find("Top");
      if ((found!=std::string::npos) && (p.size() == 3)) {
        gTopRules.push_back(rule_table_name);
        break;
      }
    }
  }

  Gen4RuleAttr(rule_table_name, attr);
  Gen4TableHeader(rule_table_name);
  unsigned index = gRuleTableNum;
  GenDebug(rule_table_name);

  std::string rule_table_data_name = rule_table_name + "_data";
  std::string rule_table_data;
  std::string rule_table;

  // 1. Add the LHS of table decl
  rule_table = "RuleTable " + rule_table_name + " =";
  rule_table_data = "TableData " + rule_table_data_name + "[";
  std::string elemnum = std::to_string(elem->mSubElems.size());

  // There are some cases where a rule has just one 'data' in RHS, could be autogen keyword, e.g
  //    rule BoolType : Boolean
  // Boolean is autogen recognized keyword, a supported type. This type of rules has NO mSubElems.
  // And we want to have a data table with one entry for them.
  if (elem && (elem->mSubElems.size() == 0))
    elemnum = "1";

  rule_table_data += elemnum + "] =";

  // 2. Add the beginning '{'
  rule_table += '{';

  // 3. Add the Entry, it contains a type and a set of data
  //    EntryType, Property, NumOfElem, TableData, NumAction, ActionTable
  std::string entrytype = GetEntryTypeName(elem->mType, elem->mData.mOp);
  entrytype = entrytype + ", ";

  std::string properties = GetPropertyName(attr);
  properties += ", ";

  rule_table += entrytype + properties + elemnum + ", " + rule_table_data_name;

  if (attr->mAction.size() > 0) {
    rule_table += ", ";
    rule_table += std::to_string(attr->mAction.size());
    rule_table += ", ";
    rule_table += rule_table_name;
    rule_table += "_action";
  } else {
    rule_table += ", 0, NULL";
  }

  // 4. Index
  rule_table += ", ";
  rule_table += std::to_string(index);

  rule_table += "};";


  // 4. go through the rule elements, generate rule table data
  std::string data = Gen4TableData(elem);
  rule_table_data += '{';
  rule_table_data += data;
  rule_table_data += "};";

  // 5. dump
  mCppBuffer->NewOneBuffer(rule_table_data.size(), true);
  mCppBuffer->AddStringWholeLine(rule_table_data);
  mCppBuffer->NewOneBuffer(rule_table.size(), true);
  mCppBuffer->AddStringWholeLine(rule_table);
}

// The structure of rule and its sub-rule can be viewed as a tree.
// We generate tables for rule and its sub-rules in depth first order.
//
void RuleGen::Generate() {
  bool need_patch = true;
  if ((mRule->mName.compare("CHAR") == 0) ||
      (mRule->mName.compare("DIGIT") == 0) ||
      (mRule->mName.compare("ASCII") == 0) ||
      (mRule->mName.compare("ESCAPE") == 0))
    need_patch = false;

  if (need_patch)
    PatchToken();

  Gen4Table(mRule, NULL);
}

// Change the string or char elements to system token if they are.
void RuleGen::PatchToken() {
  if (mRule->mElement->mSubElems.size() == 0) {
    PatchTokenOnElem(mRule->mElement);
    return;
  }

  std::vector<RuleElem *>::iterator it = mRule->mElement->mSubElems.begin();
  unsigned idx = 0;
  for(; it != mRule->mElement->mSubElems.end(); it++, idx++) {
    RuleElem *elem = *it;
    PatchTokenOnElem(elem);
  }
}

void RuleGen::PatchTokenOnElem(RuleElem *elem) {
  switch(elem->mType) {
  case ET_Char: {
    unsigned id;
    bool found = gTokenTable.FindCharTokenId(elem->mData.mChar, id);
    if (found) {
      elem->SetTokenId(id);
    }
    break;
  }
  case ET_String: {
    unsigned id;
    bool found = gTokenTable.FindStringTokenId(elem->mData.mString, id);
    if (found) {
      elem->SetTokenId(id);
    }
    break;
  }
  case ET_Rule:
    // This is the reference of another rule. Don't need go into it
    // since it will be patched by itself.
    break;
  case ET_Op: {
    // An ET_Op has children elements. Need go into each of them.
    std::vector<RuleElem *>::iterator it = elem->mSubElems.begin();
    unsigned idx = 0;
    for(; it != elem->mSubElems.end(); it++, idx++) {
      RuleElem *elem = *it;
      PatchTokenOnElem(elem);
    }
    break;
  }
  case ET_Token:
    // It's possible we meet a element which is patched as token earlier.
    // Eg. a string "static" appears in multiple places and is created
    //     as a shared RuleElem.
    break;
  case ET_Type:
    // So far there is no element of ET_Type.
    break;
  default:
    MERROR("Unsupported ElemType");
    break;
  }
}

}
