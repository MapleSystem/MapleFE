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

#ifndef __AST_COMMON_HEADER__
#define __AST_COMMON_HEADER__

namespace maplefe {

#define NOTYETIMPL(M) { if (mFlags & FLG_trace) { MNYI(M);        }}
#define MSGNOLOC0(M)  { if (mFlags & FLG_trace_3) { MMSGNOLOC0(M);  }}
#define MSGNOLOC(M,v) { if (mFlags & FLG_trace_3) { MMSGNOLOC(M,v); }}

enum AST_Flags {
  FLG_trace_1      = 0x00000001,
  FLG_trace_2      = 0x00000002,
  FLG_trace_3      = 0x00000004,
  FLG_trace_4      = 0x00000008,
  FLG_trace        = 0x0000000f,

  FLG_emit_ts      = 0x00000010,
  FLG_emit_ts_only = 0x00000020,
  FLG_format_cpp   = 0x00000040,
  FLG_no_imported  = 0x00000080,
};

}
#endif
