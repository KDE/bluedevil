# - Try to find Googlegadgets
# Once done this will define
#
#  GOOGLEGADGETS_FOUND - system has Googlegadgets
#  GOOGLEGADGETS_INCLUDE_DIRS - the Googlegadgets include directory
#  GOOGLEGADGETS_LIBRARIES - Link these to use Googlegadgets
#  GOOGLEGADGETS_CFLAGS_OTHER - Compiler switches required for using Googlegadgets
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if ( GOOGLEGADGETS_INCLUDE_DIRS )
   # in cache already
   SET(Googlegadgets_FIND_QUIETLY TRUE)
endif ( GOOGLEGADGETS_INCLUDE_DIRS )

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
if( NOT WIN32 )
  INCLUDE(FindPkgConfig)
  PKG_CHECK_MODULES(GOOGLEGADGETS QUIET libggadget-1.0>=0.11.0 libggadget-qt-1.0>=0.11.0)
endif( NOT WIN32 )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Googlegadgets DEFAULT_MSG GOOGLEGADGETS_LIBRARIES GOOGLEGADGETS_INCLUDE_DIRS)

# show the GOOGLEGADGETS_INCLUDE_DIR and GOOGLEGADGETS_LIBRARIES variables only in the advanced view
MARK_AS_ADVANCED(GOOGLEGADGETS_INCLUDE_DIRS GOOGLEGADGETS_LIBRARIES)

