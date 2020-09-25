SET(PROJECT_NAME Capture)
PROJECT(${PROJECT_NAME})
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# set(BUILD_TYPE Release)
set(BUILD_TYPE Debug)

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

# cmake-qt
# Findn includes in corresponding directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets REQUIRED)

# OpenCV
find_package(OpenCV REQUIRED)
if (OpenCV_FOUND)
	message("--- found OpenCV")
	include_directories( ${OpenCV_INCLUDE_DIRS} )
	# message( ${OpenCV_LIB} )
else ()
	message("--- could not find OpenCV")
endif ()


include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
) 
set(FILES 
	${PROJECT_SOURCE_DIR}/main.cpp 
	${PROJECT_SOURCE_DIR}/DVSCapture.cpp 
	${PROJECT_SOURCE_DIR}/D435Capture.cpp 
)
link_directories(${PROJECT_SOURCE_DIR}/lib)
set(RS_LIBS realsense2)
set(SY_LIBS sydevmanager)

add_executable(${PROJECT_NAME} ${FILES})
target_link_libraries(
	${PROJECT_NAME} 
	${OpenCV_LIBS} 
	${RS_LIBS} 
	${SY_LIBS} 
	Qt5::Widgets)
