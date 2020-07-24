![Build and test](https://github.com/ko1n/networkio/workflows/Build%20and%20test/badge.svg)

# networkio
Modern C++ Networking Library

## Prereq
- make
- cmake
- gcc/clang
- botan for optional tls support

## Building
- Run `make all` to build debug + release
- Run `make all test` to build and execute tests
- Run `make scan-build` ro tun scan-build

## Integrate it into other projects
- Add this repo as a Subproject or just clone it.
- Include the build script and dependencies in your CMakeLists.txt:
```
include("${CMAKE_CURRENT_SOURCE_DIR}/external/networkio/cmake/deps.cmake")

if (NOT TARGET networkio)
  add_subdirectory(external/networkio/library)
endif()
```

## Example

Detailed examples can be found in the examples/ folder.

## License

Licensed under MIT License, see the [LICENSE](LICENSE) file.

## Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any additional terms or conditions.
