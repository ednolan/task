.PHONY: config test default compile clean

BUILDDIR = build

default: test

compile: config
	cmake --build $(BUILDDIR) -j

format:
	git clang-format main

config:
	cmake -DCMAKE_BUILD_TYPE=Debug -B $(BUILDDIR)

test: compile
	cd $(BUILDDIR); ctest

clean:
	$(RM) -r $(BUILDDIR) mkerr olderr
