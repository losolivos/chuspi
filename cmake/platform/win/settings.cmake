# Package overloads
set(ACE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dep/acelite)
set(ACE_LIBRARY "ace")
set(ZEROMQ_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dep/zmq)
set(ZEROMQ_LIBRARY ${CMAKE_SOURCE_DIR}/dep/zmq/zmq.lib)
set(BZIP2_LIBRARIES "bzip2")
set(ZLIB_LIBRARIES "zlib")
set(ZLIB_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/dep/zlib)

# check the CMake preload parameters (commented out by default)

# overload CMAKE_INSTALL_PREFIX if not being set properly
#if( WIN32 )
#  if( NOT CYGWIN )
#    if( NOT CMAKE_INSTALL_PREFIX )
#      set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/bin")
#    endif()
#  endif()
#endif()

include(${CMAKE_SOURCE_DIR}/cmake/compiler/msvc/settings.cmake)
