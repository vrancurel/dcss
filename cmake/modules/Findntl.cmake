# Find ntl
#
# Find the ntl includes and library
#
# if you nee to add a custom library search path, do it via via CMAKE_PREFIX_PATH
#
# This module defines
#  NTL_INCLUDE_DIR, where to find header, etc.
#  NTL_LIBRARY, the libraries needed to use ntl.
#  NTL_FOUND, If false, do not try to use ntl.
#  NTL_INCLUDE_PREFIX, include prefix for ntl.
#  ntl_lib_static imported library.

set(NTL_INCLUDE_DIR "/home/vr/Ironman/ntl/src")
set(NTL_LIBRARY "/home/vr/Ironman/ntl/src/libntl.a")
set(NTL_FOUND true)
set(NTL_INCLUDE_PREFIX "ntlxxx")
set(ntl_lib_static "ntl")

add_library(ntl_lib_static UNKNOWN IMPORTED)
set_target_properties(
	ntl_lib_static
	PROPERTIES
	IMPORTED_LOCATION "${NTL_LIBRARY}"
	INTERFACE_INCLUDE_DIRECTORIES "${NTL_INCLUDE_DIR}"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ntl DEFAULT_MSG NTL_INCLUDE_DIR NTL_LIBRARY)

mark_as_advanced(
  NTL_INCLUDE_DIR
  NTL_LIBRARY
)
