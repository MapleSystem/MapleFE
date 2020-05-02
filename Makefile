# Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
#
# OpenArkFE is licensed under the Mulan PSL v1.
# You can use this software according to the terms and conditions of the Mulan PSL v1.
# You may obtain a copy of Mulan PSL v1 at:
#
#  http://license.coscl.org.cn/MulanPSL
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v1 for more details.
#

include Makefile.in

TARGS = autogen shared recdetect java2mpl

# create BUILDDIR first
$(shell $(MKDIR_P) $(BUILDDIR))

java2mpl: autogen recdetect shared
	$(MAKE) LANG=java -C java

recdetect: autogen shared
	(cd recdetect; ./build.sh java)

shared: autogen
	$(MAKE) LANG=java -C shared

autogen:
	$(MAKE) LANG=java -C autogen
	(cd $(BUILDDIR)/autogen; ./autogen)

test: autogen
	$(MAKE) LANG=java -C test

clean:
	rm -rf $(BUILDDIR)

clobber: clean
	rm -rf java/include/gen_*.h java/src/gen_*.cpp

rebuild:
	make clobber
	make -j8

.PHONY: $(TARGS)

