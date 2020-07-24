
# - Try to find the Gcrypt library
# Once run this will define
#
#  BOTAN_FOUND - set if the system has the gcrypt library
#  BOTAN_CFLAGS - the required gcrypt compilation flags
#  BOTAN_LIBRARIES - the linker libraries needed to use the gcrypt library
#
# Copyright (c) 2006 Brad Hards <bradh@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# libgcrypt is moving to pkg-config, but earlier version don't have it

#reset variables
set(BOTAN_LIBRARIES)
set(BOTAN_CFLAGS)

find_program(BOTANCONFIG_EXECUTABLE NAMES botan)

if(WIN32)
  find_library(BOTANCONFIG_LIBRARY NAMES botan)
  if(BOTANCONFIG_LIBRARY)
    get_filename_component(BOTAN_ROOT_DIRECTORY ${BOTANCONFIG_LIBRARY} DIRECTORY)
    set(BOTAN_LIBRARIES ${BOTANCONFIG_LIBRARY} "user32.lib" "ws2_32.lib")
    set(BOTAN_LDFLAGS "")
    set(BOTAN_CFLAGS "-I${BOTAN_ROOT_DIRECTORY}/../include/botan-2")
    set(BOTAN_FOUND TRUE)
  endif(BOTANCONFIG_LIBRARY)
else(WIN32)
  if(BOTANCONFIG_EXECUTABLE)
    exec_program(${BOTANCONFIG_EXECUTABLE} ARGS config libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE BOTAN_LIBRARIES)
    exec_program(${BOTANCONFIG_EXECUTABLE} ARGS config ldflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE BOTAN_LDFLAGS)
    exec_program(${BOTANCONFIG_EXECUTABLE} ARGS config cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE BOTAN_CFLAGS)

    IF(BOTAN_LIBRARIES)
      set(BOTAN_FOUND TRUE)
    ENDIF(BOTAN_LIBRARIES)

    MARK_AS_ADVANCED(BOTAN_CFLAGS BOTAN_LIBRARIES)
  endif(BOTANCONFIG_EXECUTABLE)
endif(WIN32)

if (BOTAN_FOUND)
  if (NOT Botan_FIND_QUIETLY)
    message(STATUS "Found Botan: ${BOTAN_LIBRARIES}")
  endif (NOT Botan_FIND_QUIETLY)
else (BOTAN_FOUND)
   if (Botan_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find Botan libraries")
   endif (Botan_FIND_REQUIRED)
endif (BOTAN_FOUND)