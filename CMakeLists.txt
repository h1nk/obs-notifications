cmake_minimum_required(VERSION 3.12)
project("OBS Notifications Plugin" VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)

include(CheckIncludeFile)
include(FetchContent)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(LIBNOTIFY libnotify glib-2.0 gdk-pixbuf-2.0)
	if(LIBNOTIFY_FOUND)
		include_directories(${LIBNOTIFY_INCLUDE_DIRS})
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/Config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/src/Config.hpp)
	endif()
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows" OR "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	message(STATUS "Fetching WinToast...")

	FetchContent_Declare(
			WinToast
			GIT_REPOSITORY		https://github.com/mohabouje/WinToast.git
			GIT_REMOTE_NAME		github
			GIT_TAG				v1.2.0
			GIT_SHALLOW			YES
			GIT_PROGRESS		YES
	)
	FetchContent_Populate(WinToast)
	include_directories("${WinToast_SOURCE_DIR}/src/")
endif()

add_library(obs-notifications MODULE src/NotificationPlugin.cpp)
target_link_libraries(obs-notifications notify)
# Out-of-source plugin builds are unsupported by OBS
# This is a dirty hack...
FetchContent_Declare(
		obs-studio
		GIT_REPOSITORY  https://github.com/obsproject/obs-studio.git
		GIT_REMOTE_NAME github
		GIT_TAG         24.0.3
		GIT_SHALLOW		YES
		GIT_PROGRESS	YES
)
FetchContent_Populate(obs-studio)
include_directories(
		"${obs-studio_SOURCE_DIR}"
		"${obs-studio_SOURCE_DIR}/UI/obs-frontend-api/"
		"${obs-studio_SOURCE_DIR}/libobs/")

target_include_directories(obs-notifications PUBLIC "${PROJECT_BINARY_DIR}/src/")

# See: https://github.com/obsproject/obs-studio/blob/master/cmake/Modules/ObsHelpers.cmake#L550-L553
#[[
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${obs-studio_SOURCE_DIR}/cmake/Modules/")
include(ObsHelpers)
install_obs_plugin_with_data(obs-notifications data)
]]
