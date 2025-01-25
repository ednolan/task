#-dk: note to self: PATH=/opt/llvm-19.1.6/bin:$PATH LDFLAGS=-fuse-ld=lld

.PHONY: config test default compile clean

BUILDDIR = build
PRESET  = gcc-debug
UNAME = $(shell uname -s)
ifeq ($(UNAME),Darwin)
    PRESET  = appleclang-debug
endif

default: compile

compile:
	cmake --workflow --preset=$(PRESET)

list:
	cmake --workflow --list-presets

format:
	git clang-format main

test: compile
	cd $(BUILDDIR); ctest

clean:
	$(RM) -r $(BUILDDIR) mkerr olderr
