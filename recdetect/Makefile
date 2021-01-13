include ../Makefile.in

# create build first
BUILD=$(BUILDDIR)/recdetect
$(shell $(MKDIR_P) $(BUILD))

# TEMPVAR=$(shell rm -rf $(LANG))
# TEMPVAR=$(shell mkdir -p $(LANG))
# TEMPVAR=$(shell cp ../$(LANG)/include/gen_*.h $(LANG)/)
# TEMPVAR=$(shell cp ../$(LANG)/src/gen_*.cpp $(LANG)/)
# TEMPVAR=$(shell mkdir -p $(BUILD))
# TEMPVAR=$(shell mkdir -p $(BUILD)/$(LANG))

LANGSRC=$(shell ls $(LANG)/*.cpp)
SRC := $(LANGSRC) $(wildcard *.cpp)
OBJ := $(patsubst %.cpp, %.o, $(SRC))
DEP := $(patsubst %.cpp, %.d, $(SRC))

OBJS := $(foreach obj, $(OBJ), $(BUILD)/$(obj))
DEPS := $(foreach dep, $(DEP), $(BUILD)/$(dep))

INCLUDES := -I $(MAPLEFE_ROOT)/recdetect/include \
            -I $(MAPLEFE_ROOT)/recdetect/$(LANG) \
            -I $(MAPLEFE_ROOT)/shared/include
SHAREDLIB = $(BUILDDIR)/shared/shared.a

TARGET = recdetect

.PHONY: all

all: $(TARGET)

$(TARGET) : $(OBJS) $(SHAREDLIB)
	$(LD) -o $(BUILD)/$(TARGET) $(OBJS) $(SHAREDLIB)

-include $(DEPS)
.PHONY: clean

vpath %.cpp $(MAPLEFE_ROOT)/shared/src
vpath %.o $(BUILD)
vpath %.d $(BUILD)

CXXFLAGS := $(CXXFLAGS) -I ../$(LANG)/

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
