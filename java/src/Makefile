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

include ../../Makefile.in
BUILD=$(BUILDDIR)/java
$(shell $(MKDIR_P) $(BUILD))

SRC=$(wildcard *.cpp)
OBJ :=$(patsubst %.cpp,%.o,$(SRC))
DEP :=$(patsubst %.cpp,%.d,$(SRC))

SRCG := $(wildcard $(BUILDDIR)/gen/gen*.cpp)
OBJG := $(patsubst %.cpp, %.o, $(SRCG))
DEPG := $(patsubst %.cpp, %.d, $(SRCG))

OBJS :=$(foreach obj,$(OBJ), $(BUILD)/$(obj)) $(OBJG)
DEPS :=$(foreach dep,$(DEP), $(BUILD)/$(dep)) $(DEPG)

INCLUDES := -I $(MAPLEFE_ROOT)/shared/include \
            -I $(MAPLEFE_ROOT)/ast2mpl/include \
            -I $(MAPLEFE_ROOT)/java/include \
            -I $(MAPLEFE_ROOT)/autogen/include \
            -I $(BUILDDIR)/gen \
            -I . $(MAPLEALL_INC)

TARGET=java2mpl

SHAREDLIB = $(BUILDDIR)/shared/shared.a $(BUILDDIR)/ast2mpl/ast2mpl.a

.PHONY: all $(TARGET)
all: $(TARGET)

-include $(DEPS)
.PHONY: clean

vpath %.o $(BUILD)
vpath %.d $(BUILD)

# Pattern Rules
$(BUILD)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -w -c $< -o $@

$(BUILDDIR)/gen/%.o : $(BUILDDIR)/gen/%.cpp
	$(CXX) $(CXXFLAGS) -fpermissive -I $(BUILDDIR)/gen -I $(MAPLEFE_ROOT)/shared/include -w -c $< -o $@

$(BUILD)/%.d : %.cpp
	@$(CXX) $(CXXFLAGS) -std=c++11 -MM $(INCLUDES) $< > $@
	@mv -f $(BUILD)/$*.d $(BUILD)/$*.d.tmp
	@sed -e 's|.*:|$(BUILD)/$*.o:|' < $(BUILD)/$*.d.tmp > $(BUILD)/$*.d
	@rm -f $(BUILD)/$*.d.tmp

# TARGET depends on OBJS and shared OBJS from shared directory
# as well as mapleall libraries
$(TARGET): $(OBJS) $(SHAREDLIB)
	$(LD) -o $(BUILD)/$(TARGET) $(OBJS) $(SHAREDLIB) $(MAPLELIBS)

#.cpp.o:
#	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -w -c $*.cpp -o $(BUILD)/$*.o
#	$(CXX) $(CXXFLAGS) -std=c++11 -MM $(INCLUDES) $*.cpp > $(BUILD)/$*.d

clean:
	rm -rf $(BUILD) $(OBJS) $(DEPS)
