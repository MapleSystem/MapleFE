include ../../Makefile.in
BUILD=$(ROOTDIR)/$(BUILDDIR)/java
$(shell $(MKDIR_P) $(BUILD))

SRC=$(wildcard *.cpp)
OBJ :=$(patsubst %.cpp,%.o,$(SRC))
DEP :=$(patsubst %.cpp,%.d,$(SRC))

OBJS :=$(foreach obj,$(OBJ), $(BUILD)/$(obj))
DEPS :=$(foreach dep,$(DEP), $(BUILD)/$(dep))

INCLUDES := -I $(ROOTDIR)/shared/include \
            -I $(ROOTDIR)/java/include \
            -I $(ROOTDIR)/autogen/include \
            -I .

TARGET=java2mpl

SHAREDLIB = $(ROOTDIR)/$(BUILDDIR)/shared/shared.a

.PHONY: all
all: $(TARGET)

-include $(DEPS)
.PHONY: clean

vpath %.o $(BUILD)
vpath %.d $(BUILD)

# Pattern Rules
$(BUILD)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -w -c $< -o $@ 

$(BUILD)/%.d : %.cpp
	@$(CXX) $(CXXFLAGS) -std=c++11 -MM $(INCLUDES) $< > $@
	@mv -f $(BUILD)/$*.d $(BUILD)/$*.d.tmp
	@sed -e 's|.*:|$(BUILD)/$*.o:|' < $(BUILD)/$*.d.tmp > $(BUILD)/$*.d
	@rm -f $(BUILD)/$*.d.tmp

# TARGET depends on OBJS and shared OBJS from shared directory
#
$(TARGET): $(OBJS) $(SHAREDLIB)
	$(LD) -o $(BUILD)/$(TARGET) $(OBJS) $(SHAREDLIB)

#.cpp.o:
#	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -w -c $*.cpp -o $(BUILD)/$*.o
#	$(CXX) $(CXXFLAGS) -std=c++11 -MM $(INCLUDES) $*.cpp > $(BUILD)/$*.d

clean:
	rm -rf $(BUILD)
