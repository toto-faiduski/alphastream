cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0026 OLD)

add_definitions(-std=c++11)

# Source files specified path must be relative path from project root and
# without the closing backslash character
set ( SRC_DIR "sources" )

set ( ASTREAM_SRCS
      ${SRC_DIR}/astream.cpp
    )

add_executable(astream ${ASTREAM_SRCS})
include_directories("../libastream/sources/")
target_link_libraries (astream libastream)

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

#if(WIN32)
#	set(BOOST_ROOT "D:/Develops/Main/packages/boost.1.60.0.0/lib" )
#	set(BOOST_ROOT "D:/Develops/ThirdParty/Boost/boost_1.55.0")
#endif()

find_package(Boost COMPONENTS filesystem program_options system thread date_time regex REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries (astream ${Boost_LIBRARIES})

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries (astream ${CURL_LIBRARY})

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

find_package(json_spirit REQUIRED)
include_directories(${JSON_SPIRIT_INCLUDE_DIR})
target_link_libraries (astream ${JSON_SPIRIT_LIBRARY})
