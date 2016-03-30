# - Try to find jemalloc
# Once done this will define
#  JEMALLOC_FOUND - System has jemalloc
#  JEMALLOC_INCLUDE_DIRS - jemalloc include directories
#  JEMALLOC_LIBRARIES - Libraries needed to use jemalloc

find_path(JEMALLOC_INCLUDE_DIR jemalloc/jemalloc.h
  PATHS
    /usr/local/include
    /opt/local/include
)

find_library(JEMALLOC_LIBRARY
  NAMES
    jemalloc
  PATHS
    /usr/local/lib
    /opt/local/lib
)

list(APPEND JEMALLOC_LIBRARIES ${JEMALLOC_LIBRARY})
list(APPEND JEMALLOC_INCLUDE_DIRS ${JEMALLOC_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(jemalloc DEFAULT_MSG
  JEMALLOC_LIBRARY JEMALLOC_INCLUDE_DIR
)

mark_as_advanced(
  JEMALLOC_LIBRARY
  JEMALLOC_INCLUDE_DIR
)
