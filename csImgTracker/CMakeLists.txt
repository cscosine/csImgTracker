project(csImgTracker LANGUAGES CXX)


# prepare custom project asserts, note: use standard find_package
find_package(csCore REQUIRED COMPONENTS assert CONFIG)

list(APPEND CMAKE_MODULE_PATH "${csCore_BASE_DIR}/cmake/csCore/assert/")
include(csAssert)

# set defaults for Paranoid additional configuration
csSetAssertDefaultOnForConfig(${PROJECT_NAME} ASSERT RELEASE Paranoid ON)
csSetAssertDefaultOnForConfig(${PROJECT_NAME} ASSERT RELWITHDEBINFO Paranoid ON)
csSetAssertDefaultOnForConfig(${PROJECT_NAME} ASSERT DEBUG Paranoid ON)
csSetAssertDefaultOnForConfig(${PROJECT_NAME} ASSERT PARANOID Paranoid ON)
csSetAssertDefaultOnForConfig(${PROJECT_NAME} ASSERT SAFE_MATH Paranoid ON)

csAssertGenerateProject(csImgTracker csImgTracker) #generate target csImgTracker::csImgTrackerAssert
list(POP_BACK CMAKE_MODULE_PATH)

# prepare standard library
cs_add_executable(csImgTracker)
cs_add_executable(csImgTracker::csImgTracker ALIAS csImgTracker)

cs_find_package(csImgTracker PUBLIC Eigen3 REQUIRED CONFIG)
cs_find_package(csImgTracker PUBLIC csCore COMPONENTS core eigenUtils REQUIRED CONFIG)
cs_find_package(csImgTracker PUBLIC csVisOpenGL REQUIRED COMPONENTS csVisOpenGL painters renderers CONFIG)

message(STATUS "Looking for opencv, if error, on linux, try `sudo apt-get install -y libopencv-dev`")
cs_find_package(csImgTracker PUBLIC OpenCV REQUIRED CONFIG)
get_filename_component(OpenCV_CONFIG_PATH "${OpenCV_DIR}" ABSOLUTE)
message(STATUS "OpenCVConfig.cmake is located at: ${OpenCV_CONFIG_PATH}")

cs_find_package(csImgTracker PUBLIC Qt6 COMPONENTS Widgets REQUIRED CONFIG)

cs_target_sources(csImgTracker
	PRIVATE
  Common.h
  Common.cpp
  CvMatAndQImage.h
  CvMatAndQImage.cpp
	Visualizer.h
	Visualizer.cpp
	TreeWidgetItem.h
  TreeWidgetItem.cpp
  Window.h
	Window.cpp
	main.cpp
	Window.ui
)
set_target_properties(csImgTracker PROPERTIES AUTOMOC ON)
set_target_properties(csImgTracker PROPERTIES AUTOUIC ON)
set_target_properties(csImgTracker PROPERTIES AUTORCC ON)

cs_target_link_libraries(csImgTracker
	PRIVATE
		Eigen3::Eigen
    	csCore::core
		csCore::eigenUtils
		csVisOpenGL::csVisOpenGL
		csVisOpenGL::renderers
		csVisOpenGL::painters
		csImgTracker::csImgTrackerAssert
		csImgTrackerSol::csImgTrackerSol_csCMakeBaseInclude
		Qt6::Widgets
    opencv_core
	opencv_videoio
)

cs_install_target(csImgTracker csImgTracker DEFAULT)

cs_finalize_target(csImgTracker)
