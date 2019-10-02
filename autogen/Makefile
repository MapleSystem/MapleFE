include ../Makefile.in

TARGS = autogen

$(TARGS):
	$(MAKE) -C src

test:
	$(MAKE) -C ../test

p:
	$(MAKE) -C ../test p

clean:
	rm -rf $(ROOTDIR)/$(BUILDDIR)/autogen

.PHONY: $(TARGS)

