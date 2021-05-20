.PHONY: all release debug scan-build clean test_release test_debug test

all:
	make debug
	make release

release:
	mkdir -p build/release
	cd build/release && cmake ../.. -DCMAKE_INSTALL_LIBDIR=/usr/local/lib -DCMAKE_BUILD_TYPE=Release && make all -j8

debug:
	mkdir -p build/debug
	cd build/debug && cmake ../.. -DCMAKE_INSTALL_LIBDIR=/usr/local/lib -DCMAKE_BUILD_TYPE=Debug && make all -j8

scan-build:
	make release
	make debug
	cd build/release && scan-build make
	cd build/debug && scan-build make

clean:
	rm -r build

test_release:
	cd build/release && ctest -V all

test_debug:
	cd build/debug && ctest -V all

test:
	make test_debug
	make test_release

fmt:
	find examples/ -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i
	find library/ -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i
	find tests/ -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i