# for testing batch1/Arithmetic/Add.java:
# make Add

build = output
$(shell mkdir -p $(build))

DIR0 = . $(wildcard * */* */*/*)
DIR = $(subst $(build), ,$(DIR0))

vpath %.java $(DIR)
vpath %.c $(DIR)
vpath %.cpp $(DIR)

all: test

autogen:
	make -C ../autogen

shared: autogen test
	make -C ../shared

full:
	make -C ..

clean:
	make -C .. clean

clobber:
	make -C .. clobber

rebuild:
	make -C .. rebuild

dbg:
	(cd ../build64/autogen/; gdb -args ./sharedfe -verbose=3 ../../test/output/t.java)

# only run autogen for now
test: autogen
	@echo "\ngdb command:\n(cd ../build64/autogen/; gdb ./autogen)\n"
	(cd ../build64/autogen; ./autogen)
	@echo "\ngdb command:\n(cd ../build64/autogen/; gdb ./autogen)"

p0:
	@echo "\ngdb command:\n(cd ../build64/autogen/; gdb --args ./autogen -p ../../test/test.spec)\n"
	(cd ../build64/autogen; ./autogen -p ../../test/test.spec)
	@echo "\ngdb command:\n(cd ../build64/autogen/; gdb --args ./autogen -p ../../test/test.spec)"

p:
	@echo "\ngdb command:\n(cd ../build64/autogen/; gdb --args ./autogen -verbose=3 -p ../../test/test.spec)\n"
	(cd ../build64/autogen; ./autogen -verbose=3 -p ../../test/test.spec)
	@echo "\ngdb command:\n(cd ../build64/autogen/; gdb --args ./autogen -verbose=3 -p ../../test/test.spec)"

% : %.java
	cp $< $(build)
	@echo "\ngdb command:\n(cd ../build64/autogen/; gdb -args ./sharedfe -verbose=3 ../../test/$(build)/$<)"
	(cd ../build64/autogen; ./sharedfe -verbose=3 ../../test/$(build)/$<)
	@echo "\ncommand:\n(cd ../build64/autogen/; ./sharedfe ../../test/$(build)/$<)"
	@echo "\ngdb command:\n(cd ../build64/autogen/; gdb -args ./sharedfe -verbose=3 ../../test/$(build)/$<)"

% : %.c
	cp $< $(build)
	@echo "\ngdb command:\n(cd ../build64/autogen/; gdb -args ./sharedfe -verbose=3 ../../test/$(build)/$<)"
	(cd ../build64/autogen; ./sharedfe -verbose=3 ../../test/$(build)/$<)
	@echo "\ncommand:\n(cd ../build64/autogen/; ./sharedfe ../../test/$(build)/$<)"
	@echo "\ngdb command:\n(cd ../build64/autogen/; gdb -args ./sharedfe -verbose=3 ../../test/$(build)/$<)"

% : %.cpp
	cp $< $(build)
	@echo "\ngdb command:\n(cd ../build64/autogen/; gdb -args ./sharedfe -verbose=3 ../../test/$(build)/$<)"
	(cd ../build64/autogen; ./sharedfe -verbose=3 ../../test/$(build)/$<)
	@echo "\ncommand:\n(cd ../build64/autogen/; ./sharedfe ../../test/$(build)/$<)"
	@echo "\ngdb command:\n(cd ../build64/autogen/; gdb -args ./sharedfe -verbose=3 ../../test/$(build)/$<)"
