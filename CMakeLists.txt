cmake_minimum_required(VERSION 3.9)
project(libnetworkio VERSION 1.0 DESCRIPTION "fast network input/output library")

include(GNUInstallDirs)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(MSVC)
  set(CMAKE_CXX_FLAGS "/EHsc")
elseif(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -g")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -g")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -fpermissive")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -fpermissive")
endif()

# source grouping
macro(group_sources curdir)
   file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${curdir} ${CMAKE_CURRENT_SOURCE_DIR}/${curdir}/*)
   foreach(child ${children})
      if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${curdir}/${child})
          group_sources(${curdir}/${child})
      else()
          string(REPLACE "/" "\\" groupname ${curdir})
          source_group(${groupname} FILES ${CMAKE_CURRENT_SOURCE_DIR}/${curdir}/${child})
      endif()
   endforeach()
endmacro()

# output directories
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/out_dirs.cmake)

# linting
# set(CMAKE_CXX_CLANG_TIDY
#     clang-tidy;
#     -header-filter=.;
#     #-checks=,,,cppcoreguidelines-*,modernize-*,readability-*,hicpp,google,cert,bugprone,abseil,misc,llvm,mpi;)
#     -checks=clang-analyzer-*,portability-*,performance-*,portability-*,bugprone-*;)

# dependencies
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/deps.cmake)

set(NETWORKIO_LIBRARIES Threads::Threads ${BOTAN_LIBRARIES} ${BOTAN_LDFLAGS})

# projects
add_subdirectory(library)
add_subdirectory(examples)

# tests
enable_testing()
add_subdirectory(tests)