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

#ifndef __SUCC_MATCH_H__
#define __SUCC_MATCH_H__

#include <iostream>
#include <fstream>
#include <stack>
#include <list>

#include "container.h"

namespace maplefe {

// To save all the successful results of a rule, we use a container called Guamian, aka
// Hanging Noodle. We save two types of information, one is the succ AppealNodes. The
// other one is the last matching token index. The second info can be also found through
// AppealNode, but we save it for convenience of query.
//
// This is a per-rule data structure.
// 1. The first 'unsigned' is the start token index. It's the key to the knob.
// 2. The second 'unsigned' is used for 'IsDone' right now. It tells if the current
//    recursion group has finished its parsing on this StartIndex. We conduct the parsing
//    on every recursion group in a wavefront manner. Although after each iteration of
//    the wavefront we got succ/fail info, but it's not complete yet. This field tells
//    if we have reached the fixed point or not.
//
//    The second 'unsigned' in mMatches is not used, since putting IsDone in mNodes
//    is enough.
//
// 3. The third data is the content which users are looking for, either AppealNodes or
//    matching tokens.

class AppealNode;
class SuccMatch {
private:
  Guamian<unsigned, unsigned, AppealNode*> mNodes;
  Guamian<unsigned, unsigned, unsigned> mMatches;

public:
  SuccMatch(){}
  ~SuccMatch() {mNodes.Release(); mMatches.Release();}

  void Clear() {mNodes.Clear(); mMatches.Clear();}

public:
  // The following functions need be used together, as the first one set the start
  // token (aka the key), the second one add a matching AppealNode and also updates
  // matchings tokens in mMatches.
  void AddStartToken(unsigned token);
  void AddSuccNode(AppealNode *node);
  void AddMatch(unsigned);

  ////////////////////////////////////////////////////////////////////////////
  //                     Query functions.
  // All functions in this section should be used together with GetStartToken()
  // or AddStartToken() above. Internal data is defined in GetStartToken(i);
  ////////////////////////////////////////////////////////////////////////////

  bool        GetStartToken(unsigned t); // trying to get succ info for 't'

  unsigned    GetSuccNodesNum();         // number of matching nodes at a token;
  AppealNode* GetSuccNode(unsigned i);   // get succ node at index i;
  bool        FindNode(AppealNode*);     // can we find the node?
  void        RemoveNode(AppealNode*);

  unsigned    GetMatchNum();             // Num of matchings.
  unsigned    GetOneMatch(unsigned i);   // The ith matching.
  bool        FindMatch(unsigned);       // can we find the matching token?

  // Note, the init value of Knob's data is set to 0, meaning IsDone is false.
  void        SetIsDone();
  bool        IsDone();

  ////////////////////////////////////////////////////////////////////////////
  // Below are independent functions. The start token is in argument.
  ////////////////////////////////////////////////////////////////////////////
  bool FindMatch(unsigned starttoken, unsigned target_match);
};

}
#endif
