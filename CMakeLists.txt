cmake_minimum_required(VERSION 3.12)
project("OBS Notifications Plugin" VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)

include(CheckIncludeFile)
include(FetchContent)
set(FETCHCONTENT_QUIET NO)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(LIBNOTIFY libnotify glib-2.0 gdk-pixbuf-2.0)
	if(LIBNOTIFY_FOUND)
		include_directories(${LIBNOTIFY_INCLUDE_DIRS})
	endif()
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows" OR "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	FetchContent_Declare(
			WinToast
			GIT_REPOSITORY		https://github.com/mohabouje/WinToast.git
			GIT_REMOTE_NAME		github
			GIT_TAG				v1.2.0
			GIT_SHALLOW			YES
			GIT_PROGRESS		YES
	)
	message(STATUS "Fetching WinToast...")
	FetchContent_Populate(WinToast)
	include_directories("${WinToast_SOURCE_DIR}/src/")

	set(HAVE_WINTOAST YES)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/Config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/src/Config.hpp)

include_directories(
		SYSTEM "${CMAKE_SOURCE_DIR}/libobs"
		SYSTEM "${CMAKE_SOURCE_DIR}/UI/obs-frontend-api"
)
add_library(obs-notifications MODULE src/NotificationPlugin.cpp)
if(LIBNOTIFY_FOUND)
	target_link_libraries(obs-notifications libobs notify)
endif()
target_include_directories(obs-notifications
		PRIVATE "${PROJECT_BINARY_DIR}/src/")

# See: https://github.com/obsproject/obs-studio/blob/master/cmake/Modules/ObsHelpers.cmake#L550-L553
install_obs_plugin(obs-notifications)
