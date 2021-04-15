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
            -I $(MAPLEFE_ROOT)/autogen \
            -I $(MAPLEFE_ROOT)/shared/include \
            -I $(MAPLEFE_ROOT)/$(LANG)/include \
            -I $(MAPLEFE_ROOT)/$(LANG)

TARGET = autogen

AUTOGENLIB = autogen.a

.PHONY: all

all: $(BUILD)/$(TARGET)

$(BUILD)/$(TARGET) : $(BUILD)/$(AUTOGENLIB)
	$(LD) -o $(BUILD)/$(TARGET) $(BUILD)/main.o $(BUILD)/$(AUTOGENLIB)

$(BUILD)/$(AUTOGENLIB) : $(OBJS)
	/usr/bin/ar rcs $(BUILD)/$(AUTOGENLIB) $(LIBOBJS)

-include $(DEPS)
.PHONY: clean

vpath %.cpp $(MAPLEFE_ROOT)/shared/src
vpath %.o $(BUILD)
vpath %.d $(BUILD)

CXXFLAGS := $(CXXFLAGS) -I $(MAPLEFE_ROOT)/$(LANG)

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
