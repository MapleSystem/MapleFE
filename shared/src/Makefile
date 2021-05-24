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
BUILD=$(BUILDDIR)/shared
$(shell $(MKDIR_P) $(BUILD))

SRC=$(wildcard *.cpp)
OBJ :=$(patsubst %.cpp,%.o,$(SRC))
DEP :=$(patsubst %.cpp,%.d,$(SRC))

OBJS :=$(foreach obj,$(OBJ), $(BUILD)/$(obj))
LIBOBJS :=$(patsubst $(BUILD)/main.o,,$(OBJS))
DEPS :=$(foreach dep,$(DEP), $(BUILD)/$(dep))

GENDIR:=${BUILDDIR}/gen

SRCG:=$(wildcard ${GENDIR}/gen_*.cpp)
OBJG:=$(patsubst %.cpp,%.o,$(SRCG))
DEPG:=$(patsubst %.cpp,%.d,$(SRCG))

GENASTDIR:=${BUILDDIR}/ast_gen/shared

ASTSRCG:=$(wildcard ${GENASTDIR}/*.cpp)
ASTOBJG:=$(patsubst %.cpp,%.o,$(ASTSRCG))
ASTDEPG:=$(patsubst %.cpp,%.d,$(ASTSRCG))

INCLUDES := -I $(MAPLEFE_ROOT)/shared/include -I ${GENASTDIR} -I ${GENDIR}

SHAREDLIB := shared.a
GENASTLIB := genast.a
GENLIB    := gen.a

.PHONY: all gen_doc clean

all: $(BUILD)/$(SHAREDLIB) $(GENASTDIR)/$(GENASTLIB) $(GENDIR)/$(GENLIB)

-include $(DEPS)

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

$(GENASTDIR)/%.cpp : gendoc

gen_doc:
	(cd $(MAPLEFE_ROOT); ./scripts/maplefe-autogen.py)

$(GENASTDIR)/%.o : $(GENASTDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -w -c $< -o $@

$(GENASTDIR)/%.d : $(GENASTDIR)/%.cpp
	@$(CXX) $(CXXFLAGS) -std=c++11 -MM $(INCLUDES) $< > $@

$(GENDIR)/%.o : $(GENDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -w -c $< -o $@

$(GENDIR)/%.d : $(GENDIR)/%.cpp
	@$(CXX) $(CXXFLAGS) -std=c++11 -MM $(INCLUDES) $< > $@

$(BUILD)/$(SHAREDLIB) : $(LIBOBJS)
	/usr/bin/ar rcs $(BUILD)/$(SHAREDLIB) $(LIBOBJS)

$(GENASTDIR)/$(GENASTLIB) : $(ASTOBJG)
	/usr/bin/ar rcs $(GENASTDIR)/$(GENASTLIB) $(ASTOBJG)

$(GENDIR)/$(GENLIB) : $(OBJG)
	/usr/bin/ar rcs $(GENDIR)/$(GENLIB) $(OBJG)

clean:
	rm -rf $(BUILD)
