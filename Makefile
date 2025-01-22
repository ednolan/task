#-dk: note to self: PATH=/opt/llvm-19.1.6/bin:$PATH LDFLAGS=-fuse-ld=lld

.PHONY: config test default compile clean

BUILDDIR = build

default: compile

compile:
	cmake --workflow --preset=appleclang-debug

format:
	git clang-format main

test: compile
	cd $(BUILDDIR); ctest

clean:
	$(RM) -r $(BUILDDIR) mkerr olderr
