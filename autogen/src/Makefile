include ../../Makefile.in

# create build first
BUILD=$(ROOTDIR)/$(BUILDDIR)/autogen
$(shell $(MKDIR_P) $(BUILD))

SHAREDSRC := mempool.cpp stringmap.cpp stringpool.cpp
SRC := $(wildcard *.cpp) $(SHAREDSRC)
OBJ := $(patsubst %.cpp, %.o, $(SRC))
DEP := $(patsubst %.cpp, %.d, $(SRC))

OBJS := $(foreach obj, $(OBJ), $(BUILD)/$(obj))
LIBOBJS :=$(patsubst $(BUILD)/main.o,,$(OBJS))
DEPS := $(foreach dep, $(DEP), $(BUILD)/$(dep))

INCLUDES := -I $(ROOTDIR)/autogen/include -I $(ROOTDIR)/shared/include -I $(ROOTDIR)/java/include

TARGET = autogen

AUTOGENLIB = autogen.a

.PHONY: all

all: $(TARGET)

$(TARGET) : $(AUTOGENLIB)
	$(LD) -o $(BUILD)/$(TARGET) $(BUILD)/main.o $(BUILD)/$(AUTOGENLIB)
	@ln -sf $(ROOTDIR)/java/identifier.spec $(BUILD)/identifier.spec
	@ln -sf $(ROOTDIR)/java/literal.spec $(BUILD)/literal.spec
	@ln -sf $(ROOTDIR)/java/operator.spec $(BUILD)/operator.spec
	@ln -sf $(ROOTDIR)/java/separator.spec $(BUILD)/separator.spec
	@ln -sf $(ROOTDIR)/java/keyword.spec $(BUILD)/keyword.spec
	@ln -sf $(ROOTDIR)/java/type.spec $(BUILD)/type.spec
	@ln -sf $(ROOTDIR)/java/local_var.spec $(BUILD)/local_var.spec
	@ln -sf $(ROOTDIR)/java/block.spec $(BUILD)/block.spec
	@ln -sf $(ROOTDIR)/java/expr.spec $(BUILD)/expr.spec
	@ln -sf $(ROOTDIR)/autogen/reserved.spec $(BUILD)/reserved.spec

$(AUTOGENLIB) : $(OBJS)
	/usr/bin/ar rcs $(BUILD)/$(AUTOGENLIB) $(LIBOBJS)

-include $(DEPS)
.PHONY: clean

vpath %.cpp $(ROOTDIR)/shared/src
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
