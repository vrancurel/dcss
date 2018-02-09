# - Try to find GNU libmicrohttpd
#
# Once done this will define
#  MHD_FOUND        - System has GNU libmicrohttpd.
#  MHD_INCLUDE_DIRS - The GNU libmicrohttpd include directories.
#  MHD_LIBRARIES    - The libraries needed to use GNU libmicrohttpd.

include(LibFindMacros)

# Use pkg-config to get hints about paths.
libfind_pkg_check_modules(MHD_PKGCONF microhttpd)

# Include directory.
find_path(MHD_INCLUDE_DIR
  NAMES microhttpd.h
  PATHS ${MHD_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself.
find_library(MHD_LIBRARY
  NAMES microhttpd
  PATHS ${MHD_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir/libraries variables and let libfind_process do the rest.
libfind_process(MHD)
