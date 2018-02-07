# - Try to find JsonCpp
#
# Once done this will define
#  JsonCpp_FOUND        - System has JsonCpp.
#  JsonCpp_INCLUDE_DIRS - The JsonCpp include directories.
#  JsonCpp_LIBRARIES    - The libraries needed to use JsonCpp.

include(LibFindMacros)

# Use pkg-config to get hints about paths.
libfind_pkg_check_modules(JsonCpp_PKGCONF jsoncpp)

# Include directory.
find_path(JsonCpp_INCLUDE_DIR
  NAMES json/json.h
  PATH_SUFFIXES jsoncpp
  PATHS ${JsonCpp_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself.
find_library(JsonCpp_LIBRARY
  NAMES jsoncpp
  PATHS ${JsonCpp_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir/libraries variables and let libfind_process do the rest.
libfind_process(JsonCpp)
