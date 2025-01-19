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
