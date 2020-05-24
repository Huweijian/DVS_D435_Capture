SET(PROJECT_NAME Capture)

PROJECT(${PROJECT_NAME})
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
#set(CMAKE_VERBOSE_MAKEFILE ON)
#set(BUILD_TYPE RelWithDebInfo)

set(BUILD_TYPE Release)
# set(BUILD_TYPE Debug)

if(${BUILD_TYPE} MATCHES "Debug")
	message("!!!Debug mode!!!")
	set(CMAKE_BUILD_TYPE "Debug")
	set(CMAKE_CXX_FLAGS "${SSE_FLAGS} -O0 -g -std=c++0x -march=native -pthread" )
	
else()
	message("!!!Release mode!!!")
	set(CMAKE_BUILD_TYPE "Release")
	add_definitions("-DENABLE_SSE")
	set(CMAKE_CXX_FLAGS	"${SSE_FLAGS} -O3 -g -std=c++0x -march=native -pthread")

endif()
	
set(EXECUTABLE_OUTPUT_PATH bin)

# required librarie

# optional libraries
find_package(OpenCV REQUIRED)

include_directories(
  ${PROJECT_SOURCE_DIR}
) 

set(SEE_INCLUDE_DIRS /home/opt304/Dev/sees_sdk-v1.5.1/libiness/include)
set(SEE_LIB_DIRS /home/opt304/Dev/sees_sdk-v1.5.1/libiness/lib/linux64)
set(SEE_LIBS iness)
set(RS_LIBS realsense2)

# decide if we have openCV
if (OpenCV_FOUND)
	message("--- found OpenCV")
	include_directories( ${OpenCV_INCLUDE_DIRS} ${SEE_INCLUDE_DIRS} )
	# message( ${OpenCV_LIB} )
	
else ()
	message("--- could not find OpenCV")
endif ()


set(FILES 
	${PROJECT_SOURCE_DIR}/main.cpp 
	${PROJECT_SOURCE_DIR}/DVSCapture.cpp 
	${PROJECT_SOURCE_DIR}/D435Capture.cpp 
)


link_directories(${SEE_LIB_DIRS})
add_executable(${PROJECT_NAME} ${FILES})
target_link_libraries(${PROJECT_NAME}  ${OpenCV_LIBS} ${SEE_LIBS} ${RS_LIBS})
