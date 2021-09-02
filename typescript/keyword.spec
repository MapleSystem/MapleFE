# Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
#
# OpenArkFE is licensed under the Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#
#  http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.
#
##################################################################################
# This file contains the keyword of a language. It doesn't clarify the semantics
# of a keyword, which will be done by RULEs, i.e., the parser will check the keyword
# while traversing the rule tables.
#
# The generated table of keywords are only used for
#  (1) Parser to check while traversing rule tables
#  (2) check the correctness of names so as not to conflict with keywords
##################################################################################

STRUCT KeyWord : ((boolean),
                  (number),
                  (string),
                  (symbol),
                  (unique),
                  (any),
                  (unknown),
                  (never),
                  (undefined),
                  (type),

                  (get),
                  (set),
                  (as),
                  (from),
                  (constructor),
                  (namespace),
                  (module),
                  (declare),

                  (break),
                  (do),
                  (in),
                  (is),
                  (of),
                  (typeof),
                  (keyof),
                  (infer),
                  (case),
                  (else),
                  (instanceof),
                  (var),
                  (catch),
                  (export),
                  (new),
                  (void),
                  (class),
                  (extends),
                  (return),
                  (while),
                  (const),
                  (finally),
                  (super),
                  (with),
                  (continue),
                  (for),
                  (switch),
                  (yield),
                  (debugger),
                  (function),
                  (this),
                  (default),
                  (if),
                  (throw),
                  (delete),
                  (import),
                  (require),
                  (try),
                  (asserts),

#this part is for strict mode
                  (let),
                  (static),
                  (implements),
                  (package),
                  (protected),
                  (interface),
                  (private),
                  (public),
                  (abstract),

                  (readonly),

#this part is for future reserved
                  (enum),
                  (await),
                  (async),

#this is for 'fake rule only'
                  (this_is_for_fake_rule))
