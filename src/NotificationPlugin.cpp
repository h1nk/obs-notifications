#include <libobs/obs-module.h>
#include <obs-frontend-api.h>

#include "Config.hpp"

#ifdef LIBNOTIFY_FOUND
#include <libnotify/notify.h>
#endif

#define PLUGIN_NAME         "Desktop Notifications"
#define PLUGIN_DESCRIPTION  "Show desktop notifications for certain actions and events"
#define PLUGIN_AUTHOR       "@h1nk"
// See: https://github.com/obsproject/obs-studio/blob/master/UI/xdg-data/com.obsproject.Studio.desktop#L10
#define PLUGIN_ICON         "/usr/share/icons/Papirus/48x48/apps/com.obsproject.Studio.svg"

OBS_DECLARE_MODULE() // NOLINT(hicpp-signed-bitwise)

void ShowNotification(const char* text)
{
	#ifdef LIBNOTIFY_FOUND
	NotifyNotification *notification = notify_notification_new(text, nullptr, PLUGIN_ICON);
	notify_notification_set_timeout(notification, 5'000);

	GError **error = nullptr;
	if (!notify_notification_show(notification, error))
	{
		// TODO
	}
	#endif
}

void OBSFrontendEventCallback(enum obs_frontend_event event, void* private_data)
{
	if (event == OBS_FRONTEND_EVENT_RECORDING_STARTED)
	{
		ShowNotification("Recording Started");
	}
	else if (event == OBS_FRONTEND_EVENT_RECORDING_PAUSED)
	{
		ShowNotification("Recording Paused");
	}
	else if (event == OBS_FRONTEND_EVENT_RECORDING_UNPAUSED)
	{
		ShowNotification("Recording Unpaused");
	}
	else if (event == OBS_FRONTEND_EVENT_RECORDING_STOPPED)
	{
		ShowNotification("Recording Stopped");
	}

	UNUSED_PARAMETER(private_data);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

MODULE_EXPORT bool obs_module_load()
{
	#ifdef LIBNOTIFY_FOUND
	// Initialize libnotify
	if (!notify_init(PLUGIN_NAME))
	{
		// TODO: failed to initialize notification library
	}
	#endif

	obs_frontend_add_event_callback(OBSFrontendEventCallback, nullptr);

	return true;
}

void obs_module_unload()
{
	#ifdef LIBNOTIFY_FOUND
	// Uninitialize libnotify before exiting
	notify_uninit();
	#endif
}

MODULE_EXPORT const char* obs_module_name()
{
	return PLUGIN_NAME;
}

MODULE_EXPORT const char* obs_module_description()
{
	return PLUGIN_DESCRIPTION;
}

OBS_MODULE_AUTHOR(PLUGIN_AUTHOR)

#pragma clang diagnostic pop
