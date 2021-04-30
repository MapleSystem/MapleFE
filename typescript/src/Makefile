# Copyright (C) [2021] Futurewei Technologies, Inc. All rights reverved.
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

include ../../Makefile.in
BUILD=$(BUILDDIR)/typescript
BUILDGEN=$(BUILDDIR)/gen
$(shell $(MKDIR_P) $(BUILD) $(BUILDGEN))

SRC=$(wildcard *.cpp)
OBJ :=$(patsubst %.cpp,%.o,$(SRC))
DEP :=$(patsubst %.cpp,%.d,$(SRC))

SRCG := $(wildcard $(BUILDGEN)/gen*.cpp)
OBJG := $(patsubst %.cpp, %.o, $(SRCG))
DEPG := $(patsubst %.cpp, %.d, $(SRCG))

OBJS :=$(foreach obj,$(OBJ), $(BUILD)/$(obj)) $(OBJG)
DEPS :=$(foreach dep,$(DEP), $(BUILD)/$(dep)) $(DEPG)

INCLUDES := -I $(MAPLEFE_ROOT)/shared/include \
            -I $(MAPLEFE_ROOT)/ast2cpp/include \
            -I $(MAPLEFE_ROOT)/typescript/include \
            -I $(MAPLEFE_ROOT)/autogen/include \
            -I $(BUILDGEN)

INCLUDEGEN := -I $(BUILDGEN) -I $(MAPLEFE_ROOT)/shared/include

TARGET=ts2cpp

SHAREDLIB = $(BUILDDIR)/ast2cpp/ast2cpp.a $(BUILDDIR)/shared/shared.a

.PHONY: all
all: $(BUILD)/$(TARGET)

-include $(DEPS)
.PHONY: clean

vpath %.o $(BUILD)
vpath %.d $(BUILD)

# Pattern Rules
$(BUILD)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -w -c $< -o $@

$(BUILD)/%.d : %.cpp
	@$(CXX) $(CXXFLAGS) -std=c++11 -MM $(INCLUDES) $< > $@
	@mv -f $(BUILD)/$*.d $(BUILD)/$*.d.tmp
	@sed -e 's|.*:|$(BUILD)/$*.o:|' < $(BUILD)/$*.d.tmp > $(BUILD)/$*.d
	@rm -f $(BUILD)/$*.d.tmp

$(BUILDGEN)/%.o : $(BUILDGEN)/%.cpp $(BUILDGEN)/%.d
	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDEGEN) -w -c $< -o $@

$(BUILDGEN)/%.d : $(BUILDGEN)/%.cpp
	@$(CXX) $(CXXFLAGS) -std=c++11 -MM $(INCLUDEGEN) $< > $@
	@mv -f $(BUILDGEN)/$*.d $(BUILDGEN)/$*.d.tmp
	@sed -e 's|.*:|$(BUILDGEN)/$*.o:|' < $(BUILDGEN)/$*.d.tmp > $(BUILDGEN)/$*.d
	@rm -f $(BUILDGEN)/$*.d.tmp

# TARGET depends on OBJS and shared OBJS from shared directory
# as well as mapleall libraries
$(BUILD)/$(TARGET): $(OBJS) $(SHAREDLIB)
	$(LD) -o $(BUILD)/$(TARGET) $(OBJS) $(SHAREDLIB)

clean:
	rm -rf $(BUILD)
