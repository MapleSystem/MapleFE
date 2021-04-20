# Copyright (C) [2020-2021] Futurewei Technologies, Inc. All rights reverved.
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

build = ../output/$(SRCLANG)/test
$(shell mkdir -p $(build))

ifeq ($(SRCLANG),java)
  TARGET := java
	DIR := . $(wildcard java java/* java/*/* java/*/*/*)
	FLAGS := --trace-a2m
else ifeq ($(SRCLANG),typescript)
  TARGET := ts
	DIR := . $(wildcard typescript typescript/* typescript/*/* typescript/*/*/*)
	FLAGS := --trace-lexer
endif

vpath %.java $(DIR)
vpath %.mpl $(DIR)
vpath %.c $(DIR)
vpath %.cpp $(DIR)
vpath %.ts $(DIR)

.PHONY: $(TARGET)

all: $(TARGET)

java:
	./new_runtests.pl java

typescript:
	./new_runtests.pl typescript

ts: typescript

autogen:
	make -C ../autogen

shared: autogen
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
	gdb --args ../output/java/java/java2mpl test.java $(FLAGS)

% : %.java
	@cp $< $(build)
	@echo gdb --args ../output/java/java/java2mpl $(build)/$@.java $(FLAGS)
	../output/java/java/java2mpl $(build)/$@.java $(FLAGS)
	@cat -n $(build)/$@.java
	@echo "\n======================= $@.mpl ========================="
	@cat -n $(build)/$@.mpl
	@echo "========================================================\n"
	@echo gdb --args ../output/java/java/java2mpl $(build)/$@.java $(FLAGS)
	$(MAPLEALL_ROOT)/output/aarch64-clang-debug/bin/irbuild $(build)/$@.mpl
	$(MAPLEALL_ROOT)/output/aarch64-clang-debug/bin/irbuild $(build)/$@.irb.mpl
	diff -uwb $(build)/$@.irb.mpl $(build)/$@.irb.irb.mpl

% : %.ts
	@cp $< $(build)
	@echo gdb --args ../output/typescript/typescript/ts2cpp $(build)/$@.ts $(FLAGS)
	../output/typescript/typescript/ts2cpp $(build)/$@.ts $(FLAGS)
	@cat -n $(build)/$@.ts
	@echo "\n======================= $@.cpp ========================="
	@cat -n $(build)/$@.cpp
	@echo "========================================================\n"
	@echo gdb --args ../output/typescript/typescript/ts2cpp $(build)/$@.ts $(FLAGS)

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
