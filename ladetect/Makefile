include ../Makefile.in

# create build first
BUILD=$(BUILDDIR)/ladetect
$(shell $(MKDIR_P) $(BUILD))

SRCG := $(wildcard $(BUILDDIR)/gen/gen_*.cpp)
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
            -I $(BUILDDIR)/gen
SHAREDLIB = $(BUILDDIR)/shared/shared.a

TARGET = ladetect

.PHONY: all

all: $(TARGET)

$(TARGET) : $(OBJS) $(SHAREDLIB)
	$(LD) -o $(BUILD)/$(TARGET) $(OBJS) $(SHAREDLIB)

-include $(DEPS)
.PHONY: clean

vpath %.cpp $(BUILD) $(BUILDDIR)/gen
vpath %.o $(BUILD) $(BUILDDIR)/gen
vpath %.d $(BUILD) $(BUILDDIR)/gen

#CXXFLAGS := $(CXXFLAGS) -I $(BUILDDIR)/gen -I ../shared/include

z:
	echo $(SRCG)

%.o : %.cpp %.d
	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -w -c $< -o $@

%.d : %.cpp
	@$(CXX) $(CXXFLAGS) -std=c++11 -MM $(INCLUDES) $< > $@

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
