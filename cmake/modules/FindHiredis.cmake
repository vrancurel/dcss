# - Try to find Hiredis
#
# Once done this will define
#  Hiredis_FOUND        - System has Hiredis.
#  Hiredis_INCLUDE_DIRS - The Hiredis include directories.
#  Hiredis_LIBRARIES    - The libraries needed to use Hiredis.

include(LibFindMacros)

# Use pkg-config to get hints about paths.
libfind_pkg_check_modules(Hiredis_PKGCONF hiredis)

# Include directory.
find_path(Hiredis_INCLUDE_DIR
  NAMES hiredis/hiredis.h
  PATHS ${Hiredis_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself.
find_library(Hiredis_LIBRARY
  NAMES hiredis
  PATHS ${Hiredis_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir/libraries variables and let libfind_process do the rest.
libfind_process(Hiredis)
