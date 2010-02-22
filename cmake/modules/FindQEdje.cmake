# - Try to find QEdje and QZion
# Once done this will define
#
#  QEDJE_FOUND - system has QEdje
#  QZION_INCLUDE_DIRS - the QZion include directory
#  QEDJE_INCLUDE_DIRS - the QEdje include directory
#  QZION_LIBRARIES - Link these to use QZion
#  QEDJE_LIBRARIES - Link these to use QEdje
#  QZION_CFLAGS_OTHER - Compiler switches required for using QZion
#  QEDJE_CFLAGS_OTHER - Compiler switches required for using QEdje
#

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
if( NOT WIN32 )
  find_package(PkgConfig)
  pkg_check_modules(PC_QEdje qzion>=0.4.0 qedje>=0.4.0)
endif( NOT WIN32 )

# use this just to create a nice message at FindPackageHandleStandardArgs
if (PC_QEdje_FOUND)
  FIND_PATH(QEDJE_QEdje_INCLUDE_DIR qedje.h
    HINTS
    ${PC_QEdje_INCLUDE_DIRS}
  )
  FIND_PATH(QEDJE_QZion_INCLUDE_DIR qzion.h
    HINTS
    ${PC_QEdje_INCLUDE_DIRS}
  )

  FIND_LIBRARY(QEDJE_QEdje_LIBRARY NAMES qedje
    PATHS
    ${PC_QEdje_LIBRARY_DIRS}
  )
  FIND_LIBRARY(QEDJE_QZion_LIBRARY NAMES qzion
    PATHS
    ${PC_QEdje_LIBRARY_DIRS}
  )

  SET(QEDJE_LIBRARIES ${QEDJE_QEdje_LIBRARY} ${QEDJE_QZion_LIBRARY} CACHE INTERNAL "All libraries needed for QEdje")
  SET(QEDJE_INCLUDE_DIRS  ${QEDJE_QEdje_INCLUDE_DIR} ${QEDJE_QZion_INCLUDE_DIR} CACHE INTERNAL "All include directories needed for QEdje")

else (PC_QEdje_FOUND)
  MESSAGE(STATUS "Could not find QZion and/or QEdje. Please download them here (http://code.openbossa.org/projects/qedje/pages/Home).")
endif (PC_QEdje_FOUND)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(QEdje DEFAULT_MSG QEDJE_LIBRARIES QEDJE_INCLUDE_DIRS)

# show QEdje_LIBRARY and QZion_LIBRARY variables only in the advanced view
MARK_AS_ADVANCED(QEDJE_QEdje_LIBRARY QEDJE_QZion_LIBRARY QEDJE_QEdje_INCLUDE_DIR QEDJE_QZion_INCLUDE_DIR)
