#include "massert.h"
#include "rule_gen.h"
#include "buffer2write.h"

// Generate one initialization function for each rule. Below is the elaboration of most of
// of rule case.
//
// 1) rule NonZeroDigit   : ONEOF('1', '2', '3', '4', '5', '6', '7', '8', '9')
//
//                                   ==>
//
//
//    EntryData TblNonZeroDigit_data[9] = {{DT_Char, {.mChar='1'}}, ... {DT_Char, {.mChar='9'}}};
//    RuleTable TblNonZeroDigit = {ET_Oneof, 9, TblNonZeroDigit_data};
//
// 2) rule Underscores    : '_' + ZEROORMORE('_')
//
//                                   ==>
//
//    EntryData TblUnderscores_sub1_data[1] = {{DT_Char, {.mChar='_'}}};
//    RuleTable TblUnderscores_sub1 = {ET_Zeroormore, 1, TblUnderscores_sub1_data};
//
//    EntryData TblUnderscores_data[2] = {{DT_Char, {.mChar='_'}}, {DT_Subtable, {.mEntry=&TblUnderscores_sub1}}};
//    RuleTable TblUnderscores = {ET_Concatenate, 2, TblUnderscores_data};
//
//
// So look carefully at the RuleTable of a rule, it contains two types: an ET_xxx
// and a set of DT_xxx in the data set.

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
  case ET_Char:
    data += "DT_Char, {.mChar=\'";
    if (elem->mData.mChar == '\'')
      data += "\\'";
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
    Gen4TableHeader(NULL, elem);
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

// Either rule or elem is used.
// If it's a rule, we are generating for a rule in .spec
// If it's an elem, we are generating a sub table for an elemen in a rule in .spec.
void RuleGen::Gen4TableHeader(const Rule *rule, const RuleElem *elem){
  std::string rule_table_name;
  if(rule) {
    rule_table_name = GetTblName(rule);
  } else {
    rule_table_name = GetSubTblName();
  }
  std::string extern_decl;
  extern_decl = "extern RuleTable ";
  extern_decl += rule_table_name;
  extern_decl += ";";
  mHeaderBuffer->NewOneBuffer(extern_decl.size(), true);
  mHeaderBuffer->AddStringWholeLine(extern_decl);
}

// Either rule or elem is used.
// If it's a rule, we are generating for a rule defined in .spec
// If it's an elem, we are generating a sub table for an elemen,
//    and the table name has a suffix "_sub".
void RuleGen::Gen4Table(const Rule *rule, const RuleElem *elem){
  std::string rule_table_name;
  if(rule) {
    rule_table_name = GetTblName(rule);
    elem = rule->mElement;
  } else {
    rule_table_name = GetSubTblName();
  }

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
  //    EntryType, NumOfElem, TableData
  std::string entrytype = GetEntryTypeName(elem->mType, elem->mData.mOp);
  entrytype = entrytype + ", ";
  rule_table += entrytype + elemnum + ", " + rule_table_data_name;
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
  Gen4TableHeader(mRule, NULL);
  Gen4Table(mRule, NULL);
}

