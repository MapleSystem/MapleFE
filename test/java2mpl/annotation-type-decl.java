//
//Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
//
//OpenArkFE is licensed under the Mulan PSL v2.
//You can use this software according to the terms and conditions of the Mulan PSL v2.
//You may obtain a copy of Mulan PSL v2 at:
//
// http://license.coscl.org.cn/MulanPSL2
//
//THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
//FIT FOR A PARTICULAR PURPOSE.
//See the Mulan PSL v2 for more details.
//
/**
* Describes the "request-for-enhancement" (RFE)
* that led to the presence of the annotated API element.
*/
@interface RequestForEnhancement {
  int id(); // Unique ID number associated with RFE
  String synopsis(); // Synopsis of RFE
  String engineer(); // Name of engineer who implemented RFE
  String date(); // Date RFE was implemented
}
