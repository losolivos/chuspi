#
# Find the ACE client includes and library
#
# Once done this will define
# ACE_FOUND - System has ACE
# ACE_INCLUDE_DIRS - ACE include directories
# ACE_LIBRARIES - Libraries needed to use ACE

find_path(ACE_INCLUDE_DIR ace/ACE.h
  PATHS
    /usr/include
    /usr/local/include
    $ENV{ACE_ROOT}
  DOC
    "Specify include-directories that might contain ace.h here."
)

find_library(ACE_LIBRARY
  NAMES
    ace ACE
  PATHS
    /usr/lib
    /usr/local/lib
    $ENV{ACE_ROOT}/lib
  DOC
    "Specify library-locations that might contain the ACE library here."
)

mark_as_advanced(
  ACE_LIBRARY
  ACE_INCLUDE_DIR
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ACE DEFAULT_MSG
  ACE_LIBRARY ACE_INCLUDE_DIR
)

if(ACE_FOUND)
  set(ACE_INCLUDE_DIRS ${ACE_INCLUDE_DIR})
  set(ACE_LIBRARIES ${ACE_LIBRARY})
endif()
