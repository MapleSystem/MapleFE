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

tt:
	../output/java/java2mpl tt.java --trace-a2m
	@echo "\n======================== tt.mpl ========================"
	@cat -n tt.mpl
	@echo "========================================================\n"
	$(MAPLEALL_ROOT)/output/aarch64-clang-debug/bin/irbuild tt.mpl
	$(MAPLEALL_ROOT)/output/aarch64-clang-debug/bin/irbuild tt.irb.mpl
	diff -uwb tt.irb.mpl tt.irb.irb.mpl

dbg:
	gdb --args ../output/java/java2mpl tt.java --trace-a2m

# only run autogen for now
test:
	./runtests.pl all

% : %.java
	@cp $< $(build)
	@echo gdb --args ../output/java/java2mpl $(build)/$@.java --trace-a2m
	../output/java/java2mpl $(build)/$@.java --trace-a2m
	@cat -n $(build)/$@.java
	@echo "\n======================= $@.mpl ========================="
	@cat -n $(build)/$@.mpl
	@echo "========================================================\n"
	$(MAPLEALL_ROOT)/output/aarch64-clang-debug/bin/irbuild $(build)/$@.mpl
	$(MAPLEALL_ROOT)/output/aarch64-clang-debug/bin/irbuild $(build)/$@.irb.mpl
	diff -uwb $(build)/$@.irb.mpl $(build)/$@.irb.irb.mpl
	@echo gdb --args ../output/java/java2mpl $(build)/$@.java --trace-a2m

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
