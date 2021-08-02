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
  FLAGS := --trace-a2m
else ifeq ($(SRCLANG),typescript)
  FLAGS1 := --trace-lexer
  FLAGS2 := --trace=3 --format-cpp
endif

DIRS := . $(shell find $(SRCLANG) -type d)

vpath %.java $(DIRS)
vpath %.ts $(DIRS)
vpath %.mpl $(DIRS)
vpath %.c $(DIRS)
vpath %.cpp $(DIRS)

.PHONY: $(SRCLANG)
.PHONY: mssetup

$(SRCLANG): mssetup
	env LANG=en_US.UTF-8 ./new_runtests.pl $(SRCLANG)

full:
	make -C .. -j8

rebuild:
	make -C .. rebuild -j8

clean:
	rm -rf $(build)

clobber: clean

dbg:
	gdb --args ../output/java/java/java2mpl test.java $(FLAGS)

ifeq ($(SRCLANG),java)

mssetup:

% : %.java
	@cp $< $(build)
	@echo gdb --args ../output/java/java/java2mpl $(build)/$@.java $(FLAGS)
	../output/java/java/java2mpl $(build)/$@.java $(FLAGS)
	@echo "\n======================= $@.java ========================="
	@cat -n $(build)/$@.java
	@echo "\n======================= $@.mpl ========================="
	@cat -n $(build)/$@.mpl
	@echo "========================================================\n"
	@echo gdb --args ../output/java/java/java2mpl $(build)/$@.java $(FLAGS)
	$(MAPLEALL_ROOT)/output/aarch64-clang-debug/bin/irbuild $(build)/$@.mpl
	$(MAPLEALL_ROOT)/output/aarch64-clang-debug/bin/irbuild $(build)/$@.irb.mpl
	diff -uwb $(build)/$@.irb.mpl $(build)/$@.irb.irb.mpl

else ifeq ($(SRCLANG),typescript)

mssetup:
	./msts_test.sh setup

testms:
	./msts_test.sh

testmslocal: mssetup
	env LANG=en_US.UTF-8 ./new_runtests.pl typescript TypeScript/tests/cases/compiler

unit:
	env LANG=en_US.UTF-8 ./new_runtests.pl typescript typescript/unit_tests

ms: mssetup
	env LANG=en_US.UTF-8 ./new_runtests.pl typescript typescript/ms_tests

% : %.ts
	@cp $< $(build)
	@echo gdb --args ../output/typescript/bin/ts2ast $(build)/$@.ts $(FLAGS1)
	@echo gdb --args ../output/typescript/bin/ast2cpp $(build)/$@.ts.ast $(FLAGS2)
	../output/typescript/bin/ts2ast $(build)/$@.ts $(FLAGS1)
	../output/typescript/bin/ast2cpp $(build)/$@.ts.ast $(FLAGS2)
	@echo "\n======================= $@.ts ========================="
	@cat -n $(build)/$@.ts
	@echo "========================================================\n"
	@echo gdb --args ../output/typescript/bin/ts2ast $(build)/$@.ts $(FLAGS1)
	@echo gdb --args ../output/typescript/bin/ast2cpp $(build)/$@.ts.ast $(FLAGS2)
	@echo "\n======================= $@.h ========================="
	@cat -n $(build)/$@.h
	@echo "\n======================= $@.cpp ========================="
	@cat -n $(build)/$@.cpp
	@echo "========================================================\n"
	@echo gdb --args ../output/typescript/bin/ts2ast $(build)/$@.ts $(FLAGS1)
	@echo gdb --args ../output/typescript/bin/ast2cpp $(build)/$@.ts.ast $(FLAGS2)
endif
