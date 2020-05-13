/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v1.
* You can use this software according to the terms and conditions of the Mulan PSL v1.
* You may obtain a copy of Mulan PSL v1 at:
*
*  http://license.coscl.org.cn/MulanPSL
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v1 for more details.
*/

#include "common_header_autogen.h"
#include "ruletable_util.h"
#include "rec_detect.h"

////////////////////////////////////////////////////////////////////////////////////
// The idea of Rursion Dectect is through a Depth First Traversal in the tree.
// There are a few things we need make it clear.
//  1) We are looking for back edge when traversing the tree. Those back edges form
//     the recursions. We differentiate a recursion using the first node, ie, the topmost
//     node in the tree in this recursion.
//  2) Each node (ie rule table) could have multiple recursions.
//  3) Recursions could include children recursions inside. 
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
//                                RecPath
////////////////////////////////////////////////////////////////////////////////////

void RecPath::Dump() {
  for (unsigned i = 0; i < mPositions.GetNum(); i++) {
    std::cout << mPositions.ValueAtIndex(i) << ",";
  }
  std::cout << std::endl;
}

std::string RecPath::DumpToString() {
  std::string s;
  s += "{";
  for (unsigned i = 0; i < mPositions.GetNum(); i++) {
    std::string num = std::to_string(mPositions.ValueAtIndex(i));
    s += num;
    if (i < mPositions.GetNum() - 1)
      s += ",";
  }
  s += "}";
  return s;
}

////////////////////////////////////////////////////////////////////////////////////
//                                Recursion
////////////////////////////////////////////////////////////////////////////////////

void Recursion::Release() {
  for (unsigned i = 0; i < mPaths.GetNum(); i++) {
    RecPath *path = mPaths.ValueAtIndex(i);
    path->Release();
  }

  mPaths.Release();
}

////////////////////////////////////////////////////////////////////////////////////
//                                RecDetector
////////////////////////////////////////////////////////////////////////////////////

void RecDetector::SetupTopTables() {
  mTopTables.PushBack(&TblStatement);
  mTopTables.PushBack(&TblClassDeclaration);
  mTopTables.PushBack(&TblInterfaceDeclaration);
}

// A talbe is already been processed.
bool RecDetector::IsInProcess(RuleTable *t) {
  for (unsigned i = 0; i < mInProcess.GetNum(); i++) {
    if (t == mInProcess.ValueAtIndex(i))
      return true;
  }
  return false;
}

// A talbe is already done.
bool RecDetector::IsDone(RuleTable *t) {
  for (unsigned i = 0; i < mDone.GetNum(); i++) {
    if (t == mDone.ValueAtIndex(i))
      return true;
  }
  return false;
}

// There is a back edge from 'p' to the first appearance of 'rt'. Actually in our
// traversal tree, in the current path (from 'p' upward to the root) there is one
// and only node representing 'rt', since the second appearance will be treated
// as a back edge. 
void RecDetector::AddRecursion(RuleTable *rt, ContTreeNode<RuleTable*> *p) {
  std::cout << "Recursion in " << GetRuleTableName(rt) << std::endl;

  RecPath *path = (RecPath*)gMemPool.Alloc(sizeof(RecPath));
  new (path) RecPath();

  // Step 1. Traverse upwards to find the target tree node, and add the node
  //         to the node_list. But the first to add is the back edge.

  ContTreeNode<RuleTable*> *target = NULL;
  ContTreeNode<RuleTable*> *node = p;
  SmallVector<RuleTable*> node_list;

  node_list.PushBack(rt);

  while (node) {
    if (node->GetData() == rt) {
      MASSERT(!target && "duplicated target node in a path.");
      target = node;
      break;
    } else {
      node_list.PushBack(node->GetData());
    }
    node = node->GetParent();
  }

  MASSERT(node_list.GetNum() > 0);
  MASSERT(target);
  MASSERT(target->GetData() == rt);

  // Step 2. Construct the path from target to 'p'. It's already in node_list,
  //         and we simply read it in reverse order. Also find the index of
  //         of each child rule in the parent rule.

  RuleTable *parent_rule = rt;
  for (int i = node_list.GetNum() - 1; i >= 0; i--) {
    RuleTable *child_rule = node_list.ValueAtIndex(i);
    unsigned index = 0;
    bool succ = RuleFindChild(parent_rule, child_rule, index);
    MASSERT(succ && "Cannot find child rule in parent rule.");
    path->AddPos(index);
    parent_rule = child_rule;

    std::cout << " child: " << GetRuleTableName(child_rule) << "@" << index << std::endl;
  }

  // Step 3. Get the right Recursion, Add the path to the Recursioin.
  Recursion *rec = FindOrCreateRecursion(rt);
  rec->AddPath(path);
}

// Find the Recursion of 'rule'.
// If Not found, create one.
Recursion* RecDetector::FindOrCreateRecursion(RuleTable *rule) {
  for (unsigned i = 0; i < mRecursions.GetNum(); i++) {
    Recursion *rec = mRecursions.ValueAtIndex(i);
    if (rec->GetRuleTable() == rule)
      return rec;
  }

  Recursion *rec = (Recursion*)gMemPool.Alloc(sizeof(Recursion));
  new (rec) Recursion();
  rec->SetRuleTable(rule);
  mRecursions.PushBack(rec);

  return rec;
}

// 1. There is one and only one chance to traverse a rule table. Once it's done
//    it will never be traversed again.
void RecDetector::DetectRuleTable(RuleTable *rt, ContTreeNode<RuleTable*> *p) {
  if (IsDone(rt))
    return;

  // If find a new Recursion, we are done. Don't need go deeper into
  // children nodes. However, 'rt' is not done yet since the current path
  // is just one path. We cannot set IsDone() at here.
  if (IsInProcess(rt)) {
    AddRecursion(rt, p);
    return;
  } else {
    mInProcess.PushBack(rt);
  }

  // Create new tree node.
  ContTreeNode<RuleTable*> *node = mTree.NewNode(rt, p);

  EntryType type = rt->mType;
  switch(type) {
  case ET_Oneof:
    DetectOneof(rt, node);
    break;
  case ET_Data:
  case ET_Zeroorone:
  case ET_Zeroormore:
    DetectZeroormore(rt, node);
    break;
  case ET_Concatenate:
    DetectConcatenate(rt, node);
    break;
  case ET_Null:
  default:
    break;
  }

  // It's done.
  MASSERT(!IsDone(rt));
  mDone.PushBack(rt);
}

// For Oneof rule, we try to detect for all its children if they are a rule table too.
void RecDetector::DetectOneof(RuleTable *rule_table, ContTreeNode<RuleTable*> *p) {
  for (unsigned i = 0; i < rule_table->mNum; i++) {
    TableData *data = rule_table->mData + i;
    if (data->mType == DT_Subtable) {
      RuleTable *child = data->mData.mEntry;
      DetectRuleTable(child, p);
    }
  }
}

// Data, Zeroormore and Zeroorone has the same way to handle.
void RecDetector::DetectZeroormore(RuleTable *rule_table, ContTreeNode<RuleTable*> *p) {
  MASSERT((rule_table->mNum == 1) && "zeroormore node has more than one elements?");
  TableData *data = rule_table->mData;
  if (data->mType == DT_Subtable) {
    RuleTable *child = data->mData.mEntry;
    DetectRuleTable(child, p);
  }
}

// Concatenate node is quite complicated, let's look at the first case.
//  E ---> '{' + E + '}',
//     |-> other rules
// It's obvious that E on RHS won't be considered as recursion child, and we can stop
// at this point. Now let's look at the second case.
//  A ---> '{' + E + '}',
//     |-> other rules
// E on RHS is the first time it's computed, and it's possible there are recursions
// inside E. We of course need keep working on it.
//
// So here is the question we need answer, when should we keep working on an item
// of a concatenate node? The answer is, if E is already InProcess(), we stop, otherwise,
// we keep working.
//
// There is a special child node, which is the first one. It always will go through
// further process in DectecRuleTable() who will handle it over there.

void RecDetector::DetectConcatenate(RuleTable *rule_table, ContTreeNode<RuleTable*> *p) {
  TableData *data = rule_table->mData;
  if (data->mType == DT_Subtable) {
    RuleTable *child = data->mData.mEntry;
    DetectRuleTable(child, p);
  }

  for (unsigned i = 1; i < rule_table->mNum; i++) {
    TableData *data = rule_table->mData + i;
    if (data->mType == DT_Subtable) {
      RuleTable *child = data->mData.mEntry;
      if (!IsInProcess(child))
        DetectRuleTable(child, p);
    }
  }
}

// We start from the top tables.
// Tables not accssible from top tables won't be handled.
void RecDetector::Detect() {
  mDone.Clear();

  SetupTopTables();
  for (unsigned i = 0; i < mTopTables.GetNum(); i++) {
    mInProcess.Clear();
    mTree.Clear();
    RuleTable *top = mTopTables.ValueAtIndex(i);
    DetectRuleTable(top, NULL);
  }
}

// The reason I have a Release() is to make sure the destructors of Recursion and Paths
// are invoked ahead of destructor of gMemPool.
void RecDetector::Release() {
  for (unsigned i = 0; i < mRecursions.GetNum(); i++) {
    Recursion *rec = mRecursions.ValueAtIndex(i);
    rec->Release();
  }

  mRecursions.Release();
  mTopTables.Release();
  mInProcess.Release();
  mDone.Release();
  mTree.Release();
}

// The header file would be java/include/gen_recursion.h.
void RecDetector::WriteHeaderFile() {
  mHeaderFile->WriteOneLine("#ifndef __GEN_RECUR_H__", 23);
  mHeaderFile->WriteOneLine("#define __GEN_RECUR_H__", 23);
  mHeaderFile->WriteOneLine("#include \"recursion.h\"", 22);
  mHeaderFile->WriteOneLine("#endif", 6);
}

void RecDetector::WriteCppFile() {
  mCppFile->WriteOneLine("#include \"gen_recursion.h\"", 26);
  mCppFile->WriteOneLine("#include \"common_header_autogen.h\"", 34);

  // Step 1. Dump num of Recursions
  std::string s = "unsigned gLeftRecursionsNum=";
  std::string num = std::to_string(mRecursions.GetNum());
  s += num;
  s += ';';
  mCppFile->WriteOneLine(s.c_str(), s.size());

  //Step 2. Dump paths of a rule table's recursions.
  //  unsigned tablename_path_1[N]={1, 2, ...};
  //  unsigned tablename_path_2[M]={1, 2, ...};
  //  unsigned *tablename_path_list[2] = {tablename_path_1, tablename_path_2};
  //  LeftRecursion tablename_rec = {&Tbltablename, 2, tablename_path_list};
  for (unsigned i = 0; i < mRecursions.GetNum(); i++) {
    Recursion *rec = mRecursions.ValueAtIndex(i);
    const char *tablename = GetRuleTableName(rec->GetRuleTable());

    // dump comment of tablename
    std::string comment("// ");
    comment += tablename;
    mCppFile->WriteOneLine(comment.c_str(), comment.size());

    // dump : unsigned tablename_path_1[N]={1, 2, ...};
    for (unsigned j = 0; j < rec->PathsNum(); j++) {
      RecPath *path = rec->GetPath(j);
      std::string path_str = "unsigned ";
      path_str += tablename;
      path_str += "_path_";
      std::string index_str = std::to_string(j);
      path_str += index_str;
      path_str += "[";
      std::string num_str = std::to_string(path->PositionsNum());
      path_str += num_str;
      path_str += "]=";
      std::string path_dump = path->DumpToString();
      path_str += path_dump;
      path_str += ";";
      mCppFile->WriteOneLine(path_str.c_str(), path_str.size());
    }

    //  to dump
    //  unsigned *tablename_path_list[2] = {tablename_path_1, tablename_path_2};
    std::string path_list = "unsigned *";
    path_list += tablename;
    path_list += "_path_list[";
    std::string num_str = std::to_string(rec->PathsNum());
    path_list += num_str;
    path_list += "]={";
    for (unsigned j = 0; j < rec->PathsNum(); j++) {
      std::string path_str;
      path_str += tablename;
      path_str += "_path_";
      std::string index_str = std::to_string(j);
      path_str += index_str;
      if (j < rec->PathsNum() - 1)
        path_str += ",";
      path_list += path_str;
    }
    path_list += "};";
    mCppFile->WriteOneLine(path_list.c_str(), path_list.size());
  }
}

// Write the recursion to java/gen_recursion.h and java/gen_recursion.cpp
void RecDetector::Write() {
  std::string lang_path_header("../../java/include/");
  std::string lang_path_cpp("../../java/src/");

  std::string file_name = lang_path_cpp + "gen_recursion.cpp";
  mCppFile = new Write2File(file_name);
  file_name = lang_path_header + "gen_recursion.h";
  mHeaderFile = new Write2File(file_name);

  WriteHeaderFile();
  WriteCppFile();

  delete mCppFile;
  delete mHeaderFile;
}

int main(int argc, char *argv[]) {
  gMemPool.SetBlockSize(4096);
  RecDetector dtc;
  dtc.Detect();
  dtc.Write();
  dtc.Release();
  return 0;
}
