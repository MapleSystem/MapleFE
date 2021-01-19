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

include Makefile.in

TARGS = autogen shared recdetect ladetect java2mpl

# create BUILDDIR first
$(shell $(MKDIR_P) $(BUILDDIR))

java2mpl: autogen recdetect ladetect shared
	$(MAKE) LANG=java -C java

recdetect: autogen shared
	(cd recdetect; ./build.sh java)
	(cd $(BUILDDIR)/recdetect; ./recdetect)

ladetect: autogen shared
	(cd ladetect; ./build.sh java)
	(cd $(BUILDDIR)/ladetect; ./ladetect)

shared: autogen
	$(MAKE) LANG=java -C shared

autogen:
	$(MAKE) LANG=java -C autogen
	(cd $(BUILDDIR)/autogen; ./autogen)

mapleall:
	./scripts/build_mapleall.sh

test: autogen
	$(MAKE) LANG=java -C test

testall:
	(cd test; ./runtests.pl all)

test1:
	@cp test/java2mpl/t1.java .
	@echo gdb --args ./output/java/java2mpl t1.java --trace-a2m
	./output/java/java2mpl t1.java --trace-a2m
	cat t1.mpl

clean:
	rm -rf $(BUILDDIR)

clobber: clean
	rm -rf java/include/gen_*.h java/src/gen_*.cpp ladetect/java recdetect/java

rebuild:
	make clobber
	make -j8

.PHONY: $(TARGS)

