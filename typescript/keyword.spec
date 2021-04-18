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
                  (byte),
                  (char),
                  (class),
                  (double),
                  (enum),
                  (float),
                  (int),
                  (interface),
                  (long),
                  (package),
                  (short),
                  (void),

                  (var),
                  
                  (break),
                  (case),
                  (catch),
                  (continue),
                  (default),
                  (do),
                  (else),
                  (finally),
                  (for),
                  (goto),
                  (if),
                  (return),
                  (switch),
                  (try),
                  (while),
                  
                  (abstract),
                  (const),
                  (volatile),
                  
                  (assert),
                  (new),
                  
                  (instanceof),
                  (extends),
                  (implements),
                  (import),
                  (super),
                  (synchronized),
                  (this),
                  (throw),
                  (throws),
                  (transient),
                  
                  (final),
                  (native),
                  (private),
                  (protected),
                  (public),
                  (static),
                  (strictfp))
