# Find LibBlueDevil includes and library
#
# This module defines
#  LibBlueDevil_INCLUDE_DIR
#  LibBlueDevil_LIBRARIES, the libraries to link against to use LibBlueDevil.
#  LibBlueDevil_FOUND, If false, do not try to use LibBlueDevil
#
# Copyright 2010, Aleix Pol Gonzalez
#
# Redistribution and use is allowed according to the terms of the BSD license.

IF (LibBlueDevil_LIBRARIES AND LibBlueDevil_INCLUDE_DIR)
    SET(LibBlueDevil_FIND_QUIETLY TRUE) # Already in cache, be silent
ENDIF (LibBlueDevil_LIBRARIES AND LibBlueDevil_INCLUDE_DIR)

MESSAGE(STATUS "Looking for LibBlueDevil")
# find_path(LibBlueDevil_INCLUDE_DIR bluedevilmanager.h PATH_SUFFIXES bluedevil)
find_path(LibBlueDevil_INCLUDE_DIR bluedevil/bluedevilmanager.h) #apol's NOTE: the one above looks better but doesn't fit the way you work
find_library(LibBlueDevil_LIBRARIES bluedevil)

MARK_AS_ADVANCED(LibBlueDevil_INCLUDE_DIR)
MARK_AS_ADVANCED(LibBlueDevil_LIBRARIES)

IF (LibBlueDevil_INCLUDE_DIR AND LibBlueDevil_LIBRARIES)
    SET(LibBlueDevil_FOUND TRUE)
ENDIF (LibBlueDevil_INCLUDE_DIR AND LibBlueDevil_LIBRARIES)

IF (LibBlueDevil_FOUND)
   IF (NOT LibBlueDevil_FIND_QUIETLY)
        MESSAGE(STATUS "Found LibBlueDevil")
        MESSAGE(STATUS "  libraries : ${LibBlueDevil_LIBRARIES}")
        MESSAGE(STATUS "  includes  : ${LibBlueDevil_INCLUDE_DIR}")
   ENDIF (NOT LibBlueDevil_FIND_QUIETLY)
ELSE (LibBlueDevil_FOUND)
    IF (LibBlueDevil_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find LibBlueDevil")
    ENDIF (LibBlueDevil_FIND_REQUIRED)
ENDIF (LibBlueDevil_FOUND)
