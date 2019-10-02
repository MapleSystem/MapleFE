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
            -I $(ROOTDIR)/autogen/include \
            -I $(ROOTDIR)/java/include \
            -I .

TARGET = sharedfe

AUTOGENLIB = $(ROOTDIR)/$(BUILDDIR)/autogen/autogen.a
SHAREDLIB = shared.a

.PHONY: all
all: $(TARGET)

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

$(TARGET) : $(OBJS) $(AUTOGENLIB) $(SHAREDLIB)
	$(LD) -o $(BUILD)/$(TARGET) $(BUILD)/main.o $(AUTOGENLIB) $(BUILD)/$(SHAREDLIB)
	cp -f $(BUILD)/$(TARGET) $(ROOTDIR)/$(BUILDDIR)/autogen
	@ln -sf $(ROOTDIR)/java/expr.spec $(ROOTDIR)/$(BUILDDIR)/autogen/expr.spec
	@ln -sf $(ROOTDIR)/java/stmt.spec $(ROOTDIR)/$(BUILDDIR)/autogen/stmt.spec

$(SHAREDLIB) : $(OBJS)
	/usr/bin/ar rcs $(BUILD)/$(SHAREDLIB) $(LIBOBJS)

clean:
	rm -rf $(BUILD)
