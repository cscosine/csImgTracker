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
		csImgTracker::cscmake_base_include
		Qt6::Widgets
    opencv_core
	opencv_videoio
	opencv_imgcodecs
)

cs_install_target(csImgTracker COMPONENT csImgTracker DEFAULT)

# deployment: a folder that runs on another machine, made by the install (Release), with the Qt plugins it needs
cs_deployment(app TARGETS csImgTracker)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  cs_deployment(
    app
    PLUGINS Qt6::QXcbIntegrationPlugin Qt6::QWaylandIntegrationPlugin Qt6::QOffscreenIntegrationPlugin
    DESTINATION plugins/platforms
  )
  cs_deployment(
    app
    PLUGINS Qt6::QXcbGlxIntegrationPlugin Qt6::QXcbEglIntegrationPlugin
    DESTINATION plugins/xcbglintegrations
  )
  cs_deployment(app PLUGINS Qt6::QWaylandXdgShellIntegrationPlugin DESTINATION plugins/wayland-shell-integration)
  cs_deployment(
    app
    PLUGINS Qt6::QWaylandEglClientBufferPlugin
    DESTINATION plugins/wayland-graphics-integration-client
  )
  cs_deployment(app PLUGINS Qt6::QWaylandBradientDecorationPlugin DESTINATION plugins/wayland-decoration-client)
elseif(WIN32)
  # Windows: next to the executable (bin/platforms), where Qt looks for them (the layout of windeployqt)
  cs_deployment(
    app
    PLUGINS Qt6::QWindowsIntegrationPlugin Qt6::QOffscreenIntegrationPlugin
    DESTINATION bin/platforms
  )
  # the software OpenGL of Qt (loaded at run time), for the machines without GPU (QT_OPENGL=software)
  if(EXISTS "${QT6_INSTALL_PREFIX}/bin/opengl32sw.dll")
    cs_deployment(app FILES "${QT6_INSTALL_PREFIX}/bin/opengl32sw.dll" DESTINATION bin)
  else()
    message(STATUS "opengl32sw.dll not in ${QT6_INSTALL_PREFIX}/bin: not deployed")
  endif()
endif()
