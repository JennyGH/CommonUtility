PROJECT(System)

########################## Basic ##########################
FILE(GLOB_RECURSE all_files
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

FILE(GLOB_RECURSE include_files
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)

FILE(GLOB_RECURSE src_files 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
    
SOURCE_GROUP("Header Files" FILES "${include_files}")
SOURCE_GROUP("Source Files" FILES "${src_files}")

ADD_LIBRARY(
    ${PROJECT_NAME}
    STATIC
    ${all_files}
)
###########################################################

# Link external libraries.
TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME}
)