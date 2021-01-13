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
BUILD=$(BUILDDIR)/shared
$(shell $(MKDIR_P) $(BUILD))

SRC=$(wildcard *.cpp)
OBJ :=$(patsubst %.cpp,%.o,$(SRC))
DEP :=$(patsubst %.cpp,%.d,$(SRC))

OBJS :=$(foreach obj,$(OBJ), $(BUILD)/$(obj))
LIBOBJS :=$(patsubst $(BUILD)/main.o,,$(OBJS))
DEPS :=$(foreach dep,$(DEP), $(BUILD)/$(dep))

MAPLELIBPATH:=$(MAPLEALL_ROOT)/out/aarch64-clang-debug/lib/64
MAPLELIBS := -L $(MAPLELIBPATH) -lmplir -loption_parser -lmpl2mpl -lmplutil -lmempool -lHWSecureC

INCLUDES := -I $(MAPLEFE_ROOT)/shared/include \
            -I $(MAPLEFE_ROOT)/$(LANG)/include \
            -I . \
            -I $(MAPLEALL_SRC)/maple_ir/include \
            -I $(MAPLEALL_SRC)/mpl2mpl/include \
            -I $(MAPLEALL_SRC)/mempool/include \
            -I $(MAPLEALL_SRC)/maple_util/include \
            -I $(MAPLEALL_ROOT)/huawei_secure_c/include

SHAREDLIB = shared.a

.PHONY: all
all: $(SHAREDLIB)

-include $(DEPS)
.PHONY: clean

vpath %.o $(BUILD)
vpath %.d $(BUILD)

#Pattern Rules
$(BUILD)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -w -c $< -o $@ 

$(BUILD)/%.d : %.cpp
	@$(CXX) $(CXXFLAGS) -std=c++11 -MM $(INCLUDES) $< > $@
	@mv -f $(BUILD)/$*.d $(BUILD)/$*.d.tmp
	@sed -e 's|.*:|$(BUILD)/$*.o:|' < $(BUILD)/$*.d.tmp > $(BUILD)/$*.d
	@rm -f $(BUILD)/$*.d.tmp

$(SHAREDLIB) : $(OBJS)
	/usr/bin/ar rcs $(BUILD)/$(SHAREDLIB) $(LIBOBJS)

clean:
	rm -rf $(BUILD)
