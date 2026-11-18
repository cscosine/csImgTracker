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
# Windows: a GUI application in Release (no console window next to it; the deployments and the release archives are
# Release), a console application in the other configurations (the console for development). Qt adds the entry point
# (Qt6::EntryPoint, WinMain calling main).
set_target_properties(csImgTracker PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)

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
  Style.h
  Style.cpp
	Visualizer.h
	Visualizer.cpp
	TreeWidgetItem.h
  TreeWidgetItem.cpp
  Window.h
	Window.cpp
	main.cpp
	Window.ui
)
# the icon (icon/generate.sh makes the files from icon/csImgTracker.svg): the window icon, a Qt resource; on Windows
# also the icon of the executable, a resource compiled by rc
cs_target_sources(csImgTracker PRIVATE icon/icon.qrc)
if(WIN32)
  cs_target_sources(csImgTracker PRIVATE icon/csImgTracker.rc)
endif()

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

# deployment: a folder that runs on another machine, made by the install (Release), with the Qt plugins of a GUI
# application (csCMake cs_deployment_qt_gui, offscreen too) and the sample video (data/, with its credits)
cs_deployment(app TARGETS csImgTracker)
cs_deployment(
  app
  FILES
    "${CMAKE_CURRENT_LIST_DIR}/../datasets/big_buck_bunny_2m12s-2m22s.avi"
    "${CMAKE_CURRENT_LIST_DIR}/../datasets/CREDITS.md"
  DESTINATION data
)
cs_deployment_qt_gui(app OFFSCREEN)

# licenses/ of the deployment: the license of csImgTracker, the notices and license texts of what it contains
# (licenses/THIRD-PARTY-NOTICES.md), and the licenses of the third-party code of OpenCV, as the OpenCV package of the
# platform installs them (Linux, macOS: share/licenses/opencv5; Windows: etc/licenses), found up from OpenCV_DIR
set(licenses_dir "${CMAKE_CURRENT_LIST_DIR}/../licenses")
cs_deployment(
  app
  FILES
    "${CMAKE_CURRENT_LIST_DIR}/../LICENSE"
    "${licenses_dir}/THIRD-PARTY-NOTICES.md"
    "${licenses_dir}/Qt-third-party-notices.md"
    "${licenses_dir}/csfoundation-LICENSE.txt"
    "${licenses_dir}/libassert-LICENSE.txt"
    "${licenses_dir}/cpptrace-LICENSE.txt"
    "${licenses_dir}/libdwarf-COPYING.txt"
    "${licenses_dir}/libdwarf-LIBDWARFCOPYRIGHT.txt"
    "${licenses_dir}/zstd-LICENSE.txt"
  DIRECTORY "${licenses_dir}/texts"
  DESTINATION licenses
)
set(opencv_licenses "")
set(dir "${OpenCV_DIR}")
foreach(level RANGE 4)
  foreach(candidate "${dir}/share/licenses/opencv5" "${dir}/etc/licenses")
    if(opencv_licenses STREQUAL "" AND IS_DIRECTORY "${candidate}")
      set(opencv_licenses "${candidate}")
    endif()
  endforeach()
  cmake_path(GET dir PARENT_PATH dir)
endforeach()
if(opencv_licenses STREQUAL "")
  message(FATAL_ERROR "the licenses of OpenCV (share/licenses/opencv5 or etc/licenses) not found up from ${OpenCV_DIR}")
endif()
cs_deployment(app DIRECTORY "${opencv_licenses}/" DESTINATION licenses/OpenCV)
