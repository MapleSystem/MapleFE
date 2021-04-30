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

include ../../Makefile.in

# create build first
BUILD=$(BUILDDIR)/autogen
$(shell $(MKDIR_P) $(BUILD) $(BUILDDIR)/$(SRCLANG) $(BUILDDIR)/gen)

SHAREDSRC := token.cpp mempool.cpp stringmap.cpp stringpool.cpp write2file.cpp
SRC := $(wildcard *.cpp) $(SHAREDSRC)
OBJ := $(patsubst %.cpp, %.o, $(SRC))
DEP := $(patsubst %.cpp, %.d, $(SRC))

OBJS := $(foreach obj, $(OBJ), $(BUILD)/$(obj))
LIBOBJS :=$(patsubst $(BUILD)/main.o,,$(OBJS))
DEPS := $(foreach dep, $(DEP), $(BUILD)/$(dep))

INCLUDES := -I $(MAPLEFE_ROOT)/autogen/include \
            -I $(MAPLEFE_ROOT)/autogen \
            -I $(MAPLEFE_ROOT)/shared/include

SPECS := $(wildcard $(MAPLEFE_ROOT)/$(SRCLANG)/*.spec)

TARGET = autogen

AUTOGENLIB = autogen.a

.PHONY: all
all: $(BUILD)/$(TARGET)

$(BUILD)/$(TARGET) : $(BUILD)/$(AUTOGENLIB)
	$(LD) -o $(BUILD)/$(TARGET) $(BUILD)/main.o $(BUILD)/$(AUTOGENLIB)
	(cd $(BUILD); ./$(TARGET) $(SRCLANG))
	(cd $(MAPLEFE_ROOT); ./scripts/maplefe-autogen.py)

$(BUILD)/$(AUTOGENLIB) : $(OBJS)
	/usr/bin/ar rcs $(BUILD)/$(AUTOGENLIB) $(LIBOBJS)

-include $(DEPS)
.PHONY: clean

vpath %.cpp $(MAPLEFE_ROOT)/shared/src
vpath %.o $(BUILD)
vpath %.d $(BUILD)

#Pattern Rules
$(BUILD)/%.o : %.cpp $(BUILD)/%.d
	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -w -c $< -o $@

$(BUILD)/%.d : %.cpp $(SPECS)
	@$(CXX) $(CXXFLAGS) -std=c++11 -MM $(INCLUDES) $< > $@
	@mv -f $(BUILD)/$*.d $(BUILD)/$*.d.tmp
	@sed -e 's|.*:|$(BUILD)/$*.o:|' < $(BUILD)/$*.d.tmp > $(BUILD)/$*.d
	@rm -f $(BUILD)/$*.d.tmp

clean:
	rm -rf $(BUILD)
