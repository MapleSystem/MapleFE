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
BUILDBIN=$(BUILDDIR)/bin
BUILD=$(BUILDDIR)/ast2cpp
BUILDGEN=$(BUILDDIR)/gen
BUILDASTGEN=$(BUILDDIR)/ast_gen/shared
$(shell $(MKDIR_P) $(BUILD))

SRC=$(wildcard *.cpp)
OBJ :=$(patsubst %.cpp,%.o,$(SRC))
DEP :=$(patsubst %.cpp,%.d,$(SRC))

SRCG := $(wildcard $(BUILDGEN)/gen*.cpp)
OBJG := $(patsubst %.cpp, %.o, $(SRCG))
DEPG := $(patsubst %.cpp, %.d, $(SRCG))

OBJS :=$(foreach obj,$(OBJ), $(BUILD)/$(obj)) $(OBJG)
DEPS :=$(foreach dep,$(DEP), $(BUILD)/$(dep)) $(DEPG)

LIBOBJS :=$(patsubst $(BUILD)/main.o,,$(OBJS))

GENDIR:=${BUILDDIR}/ast_gen/shared

INCLUDES := -I $(MAPLEFE_ROOT)/shared/include \
            -I $(MAPLEFE_ROOT)/astopt/include \
            -I $(MAPLEFE_ROOT)/ast2cpp/include \
            -I $(MAPLEFE_ROOT)/autogen/include \
            -I $(MAPLEFE_ROOT)/shared/include \
            -I $(MAPLEFE_ROOT)/typescript/include \
            $(MAPLEALL_INC) -I ${GENDIR}

INCLUDEGEN := -I $(MAPLEFE_ROOT)/shared/include -I $(BUILDDIR)/gen -I $(BUILDASTGEN)

TARGET=ast2cpp
SHAREDLIB = $(BUILDDIR)/astopt/astopt.a $(BUILDDIR)/shared/shared.a $(BUILDASTGEN)/genast.a
LANGSPEC=$(BUILDDIR)/typescript/lang_spec.o

.PHONY: all
all: $(BUILDBIN)/$(TARGET)

-include $(DEPS)
.PHONY: clean

vpath %.o $(BUILD)
vpath %.d $(BUILD)

#Pattern Rules
$(BUILD)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -w -c $< -o $@

$(BUILD)/%.d : %.cpp
	@$(CXX) $(CXXFLAGS) -MM $(INCLUDES) $< > $@
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
$(BUILDBIN)/$(TARGET): $(OBJS) $(SHAREDLIB)
	@mkdir -p $(BUILDBIN)
	$(LD) -o $(BUILDBIN)/$(TARGET) $(OBJS) $(LANGSPEC) $(SHAREDLIB)

clean:
	rm -rf $(BUILD)
