include ../../Makefile.in
BUILD=$(ROOTDIR)/$(BUILDDIR)/shared
$(shell $(MKDIR_P) $(BUILD))

SRC=$(wildcard *.cpp)
OBJ :=$(patsubst %.cpp,%.o,$(SRC))
DEP :=$(patsubst %.cpp,%.d,$(SRC))

OBJS :=$(foreach obj,$(OBJ), $(BUILD)/$(obj))
LIBOBJS :=$(patsubst $(BUILD)/main.o,,$(OBJS))
DEPS :=$(foreach dep,$(DEP), $(BUILD)/$(dep))

INCLUDES := -I $(ROOTDIR)/shared/include \
            -I $(ROOTDIR)/java/include \
            -I .

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
