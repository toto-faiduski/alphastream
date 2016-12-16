cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0026 OLD)
project (alphastream)

add_definitions(-std=c++11)

# Source files specified path must be relative path from project root and
# without the closing backslash character
set ( SRC_DIR "sources" )

set ( LIBASTREAM_SRCS
      ${SRC_DIR}/astream.cpp
      ${SRC_DIR}/HttpClient.cpp
    )

add_subdirectory(libastream)




set ( ASTREAM_SRCS
      astream/${SRC_DIR}/astream.cpp
    )

add_executable(astream ${ASTREAM_SRCS})
include_directories("libastream/sources/")
target_link_libraries (astream libastream)

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

#if(WIN32)
#	set(BOOST_ROOT "D:/Develops/Main/packages/boost.1.60.0.0/lib" )
#	set(BOOST_ROOT "D:/Develops/ThirdParty/Boost/boost_1.55.0")
#endif()

find_package(Boost COMPONENTS program_options system thread date_time REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries (astream ${Boost_LIBRARIES})

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries (astream ${CURL_LIBRARY})

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

# Print all variables
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

find_package(json_spirit REQUIRED)
include_directories(${JSON_SPIRIT_INCLUDE_DIR})
target_link_libraries (astream ${JSON_SPIRIT_LIBRARY})

# copy Configuration file to binary dir
# cf: http://stackoverflow.com/questions/14474659/cmake-how-to-have-a-target-for-copying-files
#file(COPY astream.conf DESTINATION ${CMAKE_BINARY_DIR})
file(GLOB ConfigFiles #${CMAKE_SOURCE_DIR}/*.xml
                      ${CMAKE_SOURCE_DIR}/*.conf)

add_custom_target(copyconf)
get_target_property(MyTargetLocation astream LOCATION)
get_filename_component(MyTargetDir ${MyTargetLocation} PATH)
foreach(ConfigFile ${ConfigFiles})
  add_custom_command( TARGET copyconf PRE_BUILD
                      COMMAND ${CMAKE_COMMAND} -E
                        copy ${ConfigFile} ${MyTargetDir})
endforeach()
add_dependencies(astream copyconf)
