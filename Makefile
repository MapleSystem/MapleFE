include Makefile.in

TARGS = autogen shared java2mpl

# create BUILDDIR first
$(shell $(MKDIR_P) $(BUILDDIR))

java2mpl: autogen shared
	$(MAKE) LANG=java -C java

shared: autogen
	$(MAKE) LANG=java -C shared

autogen:
	$(MAKE) LANG=java -C autogen
	(cd $(BUILDDIR)/autogen; ./autogen)

test: autogen
	$(MAKE) LANG=java -C test

clean:
	rm -rf $(BUILDDIR)

clobber: clean
	rm -rf java/include/gen_*.h java/src/gen_*.cpp

rebuild:
	make clobber
	make -j8

.PHONY: $(TARGS)

