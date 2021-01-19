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

# for testing batch1/Arithmetic/Add.java:
# make Add

build = ../output/test
$(shell mkdir -p $(build))

DIR = . $(wildcard * */* */*/*)

vpath %.java $(DIR)
vpath %.mpl $(DIR)
vpath %.c $(DIR)
vpath %.cpp $(DIR)

all: test

autogen:
	make -C ../autogen

shared: autogen test
	make -C ../shared

full:
	make -C ..

clean:
	make -C .. clean

clobber:
	make -C .. clobber

rebuild:
	make -C .. rebuild

dbg:
	(cd $(BUILDDIR)/autogen/; gdb -args ./sharedfe -verbose=3 ../../test/output/t.java)

# only run autogen for now
test: autogen
	@echo "\ngdb command:\n(cd $(BUILDDIR)/autogen/; gdb ./autogen)\n"
	(cd $(BUILDDIR)/autogen; ./autogen)
	@echo "\ngdb command:\n(cd $(BUILDDIR)/autogen/; gdb ./autogen)"

p0:
	@echo "\ngdb command:\n(cd $(BUILDDIR)/autogen/; gdb --args ./autogen -p ../../test/test.spec)\n"
	(cd $(BUILDDIR)/autogen; ./autogen -p ../../test/test.spec)
	@echo "\ngdb command:\n(cd $(BUILDDIR)/autogen/; gdb --args ./autogen -p ../../test/test.spec)"

p:
	@echo "\ngdb command:\n(cd $(BUILDDIR)/autogen/; gdb --args ./autogen -verbose=3 -p ../../test/test.spec)\n"
	(cd $(BUILDDIR)/autogen; ./autogen -verbose=3 -p ../../test/test.spec)
	@echo "\ngdb command:\n(cd $(BUILDDIR)/autogen/; gdb --args ./autogen -verbose=3 -p ../../test/test.spec)"

% : %.java
	@cp $< $(build)
	@echo gdb --args ../output/java/java2mpl $(build)/$@.java --trace-a2m
	../output/java/java2mpl $(build)/$@.java --trace-a2m
	cat $(build)/$@.mpl

% : %.c
	cp $< $(build)
	@echo "\ngdb command:\n(cd $(BUILDDIR)/autogen/; gdb -args ./sharedfe -verbose=3 ../../test/$(build)/$<)"
	(cd $(BUILDDIR)/autogen; ./sharedfe -verbose=3 ../../test/$(build)/$<)
	@echo "\ncommand:\n(cd $(BUILDDIR)/autogen/; ./sharedfe ../../test/$(build)/$<)"
	@echo "\ngdb command:\n(cd $(BUILDDIR)/autogen/; gdb -args ./sharedfe -verbose=3 ../../test/$(build)/$<)"

% : %.cpp
	cp $< $(build)
	@echo "\ngdb command:\n(cd $(BUILDDIR)/autogen/; gdb -args ./sharedfe -verbose=3 ../../test/$(build)/$<)"
	(cd $(BUILDDIR)/autogen; ./sharedfe -verbose=3 ../../test/$(build)/$<)
	@echo "\ncommand:\n(cd $(BUILDDIR)/autogen/; ./sharedfe ../../test/$(build)/$<)"
	@echo "\ngdb command:\n(cd $(BUILDDIR)/autogen/; gdb -args ./sharedfe -verbose=3 ../../test/$(build)/$<)"
