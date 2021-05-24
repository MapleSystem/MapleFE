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
BUILD=$(BUILDDIR)/astopt
$(shell $(MKDIR_P) $(BUILD))

SRC=$(wildcard *.cpp)
OBJ :=$(patsubst %.cpp,%.o,$(SRC))
DEP :=$(patsubst %.cpp,%.d,$(SRC))

OBJS :=$(foreach obj,$(OBJ), $(BUILD)/$(obj))
DEPS :=$(foreach dep,$(DEP), $(BUILD)/$(dep))

LIBOBJS :=$(patsubst $(BUILD)/main.o,,$(OBJS))


INCLUDES := -I $(MAPLEFE_ROOT)/shared/include \
            -I $(MAPLEFE_ROOT)/astopt/include \
            -I ${BUILDDIR}/ast_gen/shared

TARGET=astopt.a

.PHONY: all
all: $(BUILD)/$(TARGET)

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

# TARGET depends on OBJS and shared OBJS from shared directory
# as well as mapleall libraries
$(BUILD)/$(TARGET): $(OBJS)
	/usr/bin/ar rcs $(BUILD)/$(TARGET) $(OBJS)

clean:
	rm -rf $(BUILD)
