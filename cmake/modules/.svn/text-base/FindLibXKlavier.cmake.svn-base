# - Try to find LibXKlavier
# Once done this will define
#
#  LIBXKLAVIER_FOUND - system has LibXKlavier
#  LIBXKLAVIER_LDFLAGS - the libraries needed to use LibXKlavier
#  LIBXKLAVIER_CFLAGS - Compiler switches required for using LibXKlavier
#  LIBXKLAVIER_VERSION - Version of LibXKlavier

if (LIBXKLAVIER_CFLAGS AND LIBXKLAVIER_LDFLAGS)

    # in cache already
    SET(LIBXKLAVIER_FOUND TRUE)

else (LIBXKLAVIER_CFLAGS AND LIBXKLAVIER_LDFLAGS)

    IF (NOT WIN32)
        # use pkg-config to get the directories and then use these values
        # in the FIND_PATH() and FIND_LIBRARY() calls
        INCLUDE(UsePkgConfig)

	pkg_check_modules(LIBXKLAVIER libxklavier>=3.0)
    ENDIF (NOT WIN32)

    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibXKlavier DEFAULT_MSG LIBXKLAVIER_CFLAGS LIBXKLAVIER_LDFLAGS)

    MARK_AS_ADVANCED(LIBXKLAVIER_CFLAGS LIBXKLAVIER_LDFLAGS LIBXKLAVIER_VERSION)

endif (LIBXKLAVIER_CFLAGS AND LIBXKLAVIER_LDFLAGS)
