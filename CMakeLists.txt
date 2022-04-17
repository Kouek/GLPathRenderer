cmake_minimum_required (VERSION 3.1.0)

project ("GLPathRenderer" CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_DEMO "Build a demo program built on GLFW" FALSE)

set(THIRD_PARTY_DIR "${PROJECT_SOURCE_DIR}/3rdparty")

# GLAD dependencies
include_directories("${THIRD_PARTY_DIR}/glad/include")

# GLM dependencies
include("${THIRD_PARTY_DIR}/glm.cmake")

if(BUILD_DEMO)
	# GLFW dependencies
	include("${THIRD_PARTY_DIR}/glfw.cmake")

	add_executable(
		"demo"
		"./src/main.cpp"
		"${THIRD_PARTY_DIR}/glad/src/glad.c"
	)
	target_include_directories(
		"demo"
		PRIVATE
		${CMAKE_CURRENT_LIST_DIR}
	)
	target_link_libraries(
		"demo"
		PRIVATE
		"glfw" "glm"
	)
endif()
