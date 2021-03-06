include ../../Makefile.in

# create build first
BUILD=$(BUILDDIR)/autogen
$(shell $(MKDIR_P) $(BUILD))

SHAREDSRC := token.cpp mempool.cpp stringmap.cpp stringpool.cpp write2file.cpp
SRC := $(wildcard *.cpp) $(SHAREDSRC)
OBJ := $(patsubst %.cpp, %.o, $(SRC))
DEP := $(patsubst %.cpp, %.d, $(SRC))

OBJS := $(foreach obj, $(OBJ), $(BUILD)/$(obj))
LIBOBJS :=$(patsubst $(BUILD)/main.o,,$(OBJS))
DEPS := $(foreach dep, $(DEP), $(BUILD)/$(dep))

INCLUDES := -I $(MAPLEFE_ROOT)/autogen/include \
            -I $(MAPLEFE_ROOT)/shared/include \
            -I $(MAPLEFE_ROOT)/java/include \
            -I $(MAPLEFE_ROOT)/java

TARGET = autogen

AUTOGENLIB = autogen.a

.PHONY: all

all: $(BUILD)/$(TARGET)

$(BUILD)/$(TARGET) : $(BUILD)/$(AUTOGENLIB)
	$(LD) -o $(BUILD)/$(TARGET) $(BUILD)/main.o $(BUILD)/$(AUTOGENLIB)
	@ln -sf $(MAPLEFE_ROOT)/java/identifier.spec $(BUILD)/identifier.spec
	@ln -sf $(MAPLEFE_ROOT)/java/literal.spec $(BUILD)/literal.spec
	@ln -sf $(MAPLEFE_ROOT)/java/operator.spec $(BUILD)/operator.spec
	@ln -sf $(MAPLEFE_ROOT)/java/separator.spec $(BUILD)/separator.spec
	@ln -sf $(MAPLEFE_ROOT)/java/keyword.spec $(BUILD)/keyword.spec
	@ln -sf $(MAPLEFE_ROOT)/java/type.spec $(BUILD)/type.spec
	@ln -sf $(MAPLEFE_ROOT)/java/attr.spec $(BUILD)/attr.spec
	@ln -sf $(MAPLEFE_ROOT)/java/block.spec $(BUILD)/block.spec
	@ln -sf $(MAPLEFE_ROOT)/java/expr.spec $(BUILD)/expr.spec
	@ln -sf $(MAPLEFE_ROOT)/java/stmt.spec $(BUILD)/stmt.spec
	@ln -sf $(MAPLEFE_ROOT)/autogen/reserved.spec $(BUILD)/reserved.spec

$(BUILD)/$(AUTOGENLIB) : $(OBJS)
	/usr/bin/ar rcs $(BUILD)/$(AUTOGENLIB) $(LIBOBJS)

-include $(DEPS)
.PHONY: clean

vpath %.cpp $(MAPLEFE_ROOT)/shared/src
vpath %.o $(BUILD)
vpath %.d $(BUILD)

ifeq ($(LANG), java)
CXXFLAGS := $(CXXFLAGS) -I ../java/
endif

#Pattern Rules
$(BUILD)/%.o : %.cpp $(BUILD)/%.d
	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -w -c $< -o $@

$(BUILD)/%.d : %.cpp
	@$(CXX) $(CXXFLAGS) -std=c++11 -MM $(INCLUDES) $< > $@
	@mv -f $(BUILD)/$*.d $(BUILD)/$*.d.tmp
	@sed -e 's|.*:|$(BUILD)/$*.o:|' < $(BUILD)/$*.d.tmp > $(BUILD)/$*.d
	@rm -f $(BUILD)/$*.d.tmp


#.cpp.o:
#	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -w -c $*.cpp -o $(BUILD)/$*.o
#	$(CXX) $(CXXFLAGS) -std=c++11 -MM $(INCLUDES) $*.cpp > $(BUILD)/$*.d

clean:
	rm -rf $(BUILD)
