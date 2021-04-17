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

TARGS = autogen shared recdetect ladetect java2mpl ast2mpl ts2cpp ast2cpp

# create BUILDDIR first
$(shell $(MKDIR_P) $(BUILDDIR))

java2mpl: autogen recdetect ladetect shared ast2mpl
	$(MAKE) LANG=$(SRCLANG) -C $(SRCLANG)

ts2cpp: autogen recdetect ladetect shared ast2cpp
	$(MAKE) LANG=$(SRCLANG) -C $(SRCLANG)

recdetect: autogen shared
	$(MAKE) LANG=$(SRCLANG) -C recdetect
	(cd $(BUILDDIR)/recdetect; ./recdetect)

ladetect: autogen shared
	$(MAKE) LANG=$(SRCLANG) -C ladetect
	(cd $(BUILDDIR)/ladetect; ./ladetect)

ast2mpl: shared
	$(MAKE) -C ast2mpl

ast2cpp: shared
	$(MAKE) -C ast2cpp

shared: autogen
	$(MAKE) -C shared

autogen:
	$(MAKE) -C autogen
	(cd $(BUILDDIR)/autogen; ./autogen $(SRCLANG))

mapleall:
	./scripts/build_mapleall.sh

test: autogen
	$(MAKE) LANG=$(SRCLANG) -C test

testall:
	(cd test; ./runtests.pl all)

test1:
	@cp test/java2mpl/t1.java .
	@echo gdb --args ./output/java/java2mpl t1.java --trace-a2m
	./output/java/java2mpl t1.java --trace-a2m
	cat t1.mpl

clean:
	rm -rf $(BUILDDIR)

clobber:
	rm -rf output

rebuild:
	make clobber
	make -j8

.PHONY: $(TARGS)

