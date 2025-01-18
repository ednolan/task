.PHONY: config test default compile clean

BUILDDIR = build

default: compile

compile: config
	#cmake --build $(BUILDDIR) -j
	cmake --workflow --preset=appleclang-debug --fresh

format:
	git clang-format main

config:
	#CXXFLAGS=-fsanitize=thread cmake -DCMAKE_BUILD_TYPE=Debug -B $(BUILDDIR)

test: compile
	cd $(BUILDDIR); ctest

clean:
	$(RM) -r $(BUILDDIR) mkerr olderr
