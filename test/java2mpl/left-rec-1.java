//
//Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
//
//OpenArkFE is licensed under the Mulan PSL v1.
//You can use this software according to the terms and conditions of the Mulan PSL v1.
//You may obtain a copy of Mulan PSL v1 at:
//
// http://license.coscl.org.cn/MulanPSL
//
//THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
//FIT FOR A PARTICULAR PURPOSE.
//See the Mulan PSL v1 for more details.
//

// Java has a rule 'Definite Assignment'.
//
// This case is from JLS 8.
//
// In this case, k is definitely assigned before use. Compiler should be able to evaluate
// the special treatment of the conditional boolean operators &&, ||, and
// ? : and of boolean-valued constant expressions
//
// All other expressions are not taken into account.
//
//if ((k = System.in.read()) >= 0)
System.out.println(k);
