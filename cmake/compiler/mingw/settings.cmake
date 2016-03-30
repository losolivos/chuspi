# we do not need whole windows stuff at all
add_definitions(-DWIN32_LEAN_AND_MEAN=1)
add_definitions(-DNOMINMAX=1)

# target Windows XP at least
add_definitions(-D_WIN32_WINNT=0x0501)

# required till we fully switch to C++11 headers
add_definitions(-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS)

# Set build-directive (used in core to tell which buildtype we used)
add_definitions(-D_BUILD_DIRECTIVE=\\"${CMAKE_BUILD_TYPE}\\")

# set up output paths for executable binaries (.exe-files, and .dll-files on DLL-capable platforms)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(PLATFORM EQUAL 64)
  add_definitions(-D_WIN64)
  message(STATUS "MinGW: 64-bit platform, enforced -D_WIN64 parameter")
endif()

if( WITH_COREDEBUG )
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")
  message(STATUS "MinGW: Debug-flags set (-g3)")
endif()
