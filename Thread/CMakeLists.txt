PROJECT(CommonUtility)

FILE(GLOB_RECURSE all_files
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

FILE(GLOB_RECURSE include_files
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

FILE(GLOB_RECURSE src_files 
        ${CMAKE_CURRENT_SOURCE_DIR}/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

SOURCE_GROUP("Header Files" FILES "${include_files}")
SOURCE_GROUP("Source Files" FILES "${src_files}")

ADD_EXECUTABLE(Thread ${all_files})