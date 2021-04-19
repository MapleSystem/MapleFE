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

include ../Makefile.in

# create build first
BUILD=$(BUILDDIR)/ladetect
BUILDGEN=$(BUILDDIR)/gen
$(shell $(MKDIR_P) $(BUILD) $(BUILDGEN)) 

SRCG := $(wildcard $(BUILDGEN)/gen_*.cpp)
OBJG := $(patsubst %.cpp, %.o, $(SRCG))
DEPG := $(patsubst %.cpp, %.d, $(SRCG))

SRC := $(wildcard *.cpp)
OBJ := $(patsubst %.cpp, %.o, $(SRC))
OBJL := $(foreach obj, $(OBJ), $(BUILD)/$(obj))
DEP := $(patsubst %.cpp, %.d, $(SRC))
DEPL := $(foreach dep, $(DEP), $(BUILD)/$(dep))

OBJS := $(OBJG) $(OBJL)
DEPS := $(DEPG) $(DEPL)

INCLUDES := -I $(MAPLEFE_ROOT)/ladetect \
            -I $(MAPLEFE_ROOT)/shared/include \
            -I $(MAPLEFE_ROOT)/$(SRCLANG)/include \
            -I $(BUILDGEN)

INCLUDEGEN := -I $(BUILDGEN) -I $(MAPLEFE_ROOT)/shared/include

SHAREDLIB = $(BUILDDIR)/shared/shared.a

TARGET = ladetect

.PHONY: all

all: $(BUILD)/$(TARGET)

$(BUILD)/$(TARGET) : $(OBJS) $(SHAREDLIB)
	$(LD) -o $(BUILD)/$(TARGET) $(OBJS) $(SHAREDLIB)
	(cd $(BUILD); ./$(TARGET))

-include $(DEPS)
.PHONY: clean

vpath %.o $(BUILD)
vpath %.d $(BUILD)

$(BUILDGEN)/%.o : $(BUILDGEN)/%.cpp $(BUILDGEN)/%.d
	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDEGEN) -w -c $< -o $@

$(BUILDGEN)/%.d : $(BUILDGEN)/%.cpp
	@$(CXX) $(CXXFLAGS) -std=c++11 -MM $(INCLUDEGEN) $< > $@
	@mv -f $(BUILDGEN)/$*.d $(BUILDGEN)/$*.d.tmp
	@sed -e 's|.*:|$(BUILDGEN)/$*.o:|' < $(BUILDGEN)/$*.d.tmp > $(BUILDGEN)/$*.d
	@rm -f $(BUILDGEN)/$*.d.tmp

#Pattern Rules
$(BUILD)/%.o : %.cpp $(BUILD)/%.d
	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -w -c $< -o $@

$(BUILD)/%.d : %.cpp
	@$(CXX) $(CXXFLAGS) -std=c++11 -MM $(INCLUDES) $< > $@
	@mv -f $(BUILD)/$*.d $(BUILD)/$*.d.tmp
	@sed -e 's|.*:|$(BUILD)/$*.o:|' < $(BUILD)/$*.d.tmp > $(BUILD)/$*.d
	@rm -f $(BUILD)/$*.d.tmp

clean:
	rm -rf $(BUILD) $(OBJG) $(DEPG)
