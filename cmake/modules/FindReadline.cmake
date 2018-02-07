# - Try to find GNU Readline
#
# Once done this will define
#  Readline_FOUND        - System has GNU Readline.
#  Readline_INCLUDE_DIRS - The GNU Readline include directories.
#  Readline_LIBRARIES    - The libraries needed to use GNU Readline.

include(LibFindMacros)

# Use pkg-config to get hints about paths.
libfind_pkg_check_modules(Readline_PKGCONF readline)

# Include directory.
find_path(Readline_INCLUDE_DIR
  NAMES readline/readline.h
  PATHS ${Readline_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself.
find_library(Readline_LIBRARY
  NAMES readline
  PATHS ${Readline_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir/libraries variables and let libfind_process do the rest.
libfind_process(Readline)
