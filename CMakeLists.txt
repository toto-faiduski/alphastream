cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0026 OLD)
project (alphastream)

add_definitions(-std=c++11)


add_subdirectory(libastream)

add_subdirectory(astream)

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
