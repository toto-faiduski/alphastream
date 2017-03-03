#cmake_minimum_required (VERSION 2.6)
#cmake_policy(SET CMP0026 OLD)
#project (astream)

#add_definitions(-std=c++11)

# Source files specified path must be relative path from project root and
# without the closing backslash character
set ( SRC_DIR "sources" )

set ( LIBASTREAM_SRCS
      ${SRC_DIR}/astream.cpp
      ${SRC_DIR}/HttpClient.cpp
      ${SRC_DIR}/json_helpers.cpp
    )


add_library(libastream ${LIBASTREAM_SRCS})

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

if(MSVC)
#	set(Boost_COMPILER -vc100)
#	set(Boost_DETAILED_FAILURE_MSG ON )
#	set(Boost_DEBUG ON )

#	set(BOOST_ROOT			"C:/local/boost_1_60_0" )
#	set(BOOST_LIBRARYDIR	"C:/local/boost_1_60_0/lib32-msvc-10.0" )

#	set(BOOST_ROOT			"D:/Develops/ThirdParty/Boost/boost_1_60_0" )
#	set(BOOST_LIBRARYDIR	"D:/Develops/ThirdParty/Boost/boost_1_60_0/lib32-msvc-14.0" )
endif()

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})
#target_link_libraries (libastream ${Boost_LIBRARIES})

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})
#target_link_libraries (libastream ${CURL_LIBRARY})

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

find_package(json_spirit REQUIRED)
include_directories(${JSON_SPIRIT_INCLUDE_DIR})
#target_link_libraries (libastream ${JSON_SPIRIT_LIBRARY})
