cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(MY_PROJECT_NAME OIJSON)
project(${MY_PROJECT_NAME} C)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/debug)
else()
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/release)
endif()

add_executable(${MY_PROJECT_NAME}_test ./src/main.c ./src/oijson.c)

# Make compiler scream out every possible warning
if(MSVC)
	target_compile_options(${MY_PROJECT_NAME}_test PRIVATE /W4 /WX /D_CRT_SECURE_NO_WARNINGS)
else()
	target_compile_options(${MY_PROJECT_NAME}_test PRIVATE -Wstrict-prototypes -Wconversion -Wall -Wextra -Wpedantic -pedantic -Werror)
endif()

#copy all resources
file(GLOB_RECURSE resources ${CMAKE_SOURCE_DIR}/res/*.*)
file(COPY ${resources} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res)
