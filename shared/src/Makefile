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
BUILD=$(ROOTDIR)/$(BUILDDIR)/shared
$(shell $(MKDIR_P) $(BUILD))

SRC=$(wildcard *.cpp)
OBJ :=$(patsubst %.cpp,%.o,$(SRC))
DEP :=$(patsubst %.cpp,%.d,$(SRC))

OBJS :=$(foreach obj,$(OBJ), $(BUILD)/$(obj))
LIBOBJS :=$(patsubst $(BUILD)/main.o,,$(OBJS))
DEPS :=$(foreach dep,$(DEP), $(BUILD)/$(dep))

MAPLEROOT=~/mapleall
MAPLEALL=$(MAPLEROOT)/mapleall
MAPLELIBPATH:=$(MAPLEROOT)/out/arm64-clang-debug/lib/64
#MAPLELIBS := $(MAPLELIBPATH)/libmplir.a \
#             $(MAPLELIBPATH)/liboption_parser.a \
#             $(MAPLELIBPATH)/liblibmpl2mpl.a \
#             $(MAPLELIBPATH)/libmplutil.a \
#             $(MAPLELIBPATH)/libmempool.a \
#             $(MAPLELIBPATH)/libHWSecureC.a

HUAWEI_SECUEW_C_ROOT=$(MAPLEROOT)/huawei_secure_c
INCLUDES := -I $(ROOTDIR)/shared/include \
            -I $(ROOTDIR)/$(LANG)/include \
            -I . \
            -I $(MAPLEALL)/maple_ir/include \
            -I $(MAPLEALL)/mpl2mpl/include \
            -I $(MAPLEALL)/mempool/include \
            -I $(MAPLEALL)/maple_util/include \
            -I $(HUAWEI_SECUEW_C_ROOT)/include

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

$(SHAREDLIB) : $(OBJS) $(MAPLELIBS)
	/usr/bin/ar rcs $(BUILD)/$(SHAREDLIB) $(LIBOBJS) $(MAPLELIBS)

clean:
	rm -rf $(BUILD)
