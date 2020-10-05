project(obs-notifications)

if(DISABLE_NOTIFICATIONS)
	message(STATUS "Notification support disabled")
	return()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)

include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/UI/obs-frontend-api")

set(obs-notifications_SOURCES
		src/obs_notifications.cpp)
add_library(obs-notifications MODULE
		${obs-notifications_SOURCES})
target_link_libraries(obs-notifications
		libobs)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(LIBNOTIFY libnotify glib-2.0 gdk-pixbuf-2.0)
	if(LIBNOTIFY_FOUND)
		include_directories(${LIBNOTIFY_INCLUDE_DIRS})
		target_link_libraries(obs-notifications notify)
	endif()
endif()

target_include_directories(obs-notifications
		PUBLIC
		"${PROJECT_BINARY_DIR}/src/")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/src/config.hpp)

message("${CMAKE_CURRENT_SOURCE_DIR}/src/config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/src/config.hpp")

set_target_properties(obs-notifications PROPERTIES FOLDER "plugins")

install_obs_plugin_with_data(obs-notifications data)
