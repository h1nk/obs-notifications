#include <string>       // std::string
#include <cstddef>      // std::size_t
#include <cstring>      // std::memcpy

#include <sys/mman.h>   // mprotect(2), PROT_READ, PROT_WRITE, PROT_EXEC
#include <dlfcn.h>
#include <link.h>
#include <elf.h>        // Elf64_Addr
#include <unistd.h>     // getpagesize(2)

#include <libobs/obs-module.h>
#include <obs-frontend-api.h>

#include "Config.hpp"

#ifdef LIBNOTIFY_FOUND
  #include <libnotify/notify.h>
  #include <functional>
#elif HAVE_WINTOAST
  #include "wintoastlib.h"
#endif

#define PLUGIN_NAME         "Desktop Notifications"
#define PLUGIN_DESCRIPTION  "Show desktop notifications for certain actions and events"
#define PLUGIN_AUTHOR       "@h1nk"

OBS_DECLARE_MODULE() // NOLINT(hicpp-signed-bitwise)

static Elf64_Addr moduleBase = 0;

int mempatch(void *dst, void *src, size_t size) {
    std::size_t pageMask = ~(getpagesize() - 1);
    void* aligned = (void*) (((size_t) dst) & pageMask);
    std::size_t aligned_size = reinterpret_cast<std::size_t>(dst) - reinterpret_cast<std::size_t>(aligned) + size;
    if (mprotect(aligned, aligned_size, PROT_READ | PROT_WRITE) != 0) {
        perror("mprotect");
        return 1;
    }
    memcpy(dst, src, size);
    if (mprotect(aligned, aligned_size, PROT_READ | PROT_EXEC) != 0) {
        perror("mprotect");
        return 1;
    }
    return 0;
}

void show_notification_str(const std::string& text)
{
	#ifdef LIBNOTIFY_FOUND
	NotifyNotification *notification = notify_notification_new(text.c_str(), nullptr, nullptr);
	notify_notification_set_timeout(notification, 5'000);

	GError** error = nullptr;
	if (!notify_notification_show(notification, error))
	{
		// TODO
	}
	#endif
}

extern "C"
{
void show_replay_saved_notif() {
    show_notification_str("Saving Replay Buffer...");
}
}

extern "C"
{
  void _show_replay_saved_notif();
}
asm(R"(
_show_replay_saved_notif:
push %rbx;
push %rdx;
call show_replay_saved_notif;
pop %rdx;
pop %rbx;
mov %rdx,0x88(%rbx);
pop %rbx;
ret;
)");

[[maybe_unused]] void OBSFrontendEventCallback(enum obs_frontend_event event, void* private_data) {
	if (event == OBS_FRONTEND_EVENT_RECORDING_STARTED)
        show_notification_str("Recording Started");
	else if (event == OBS_FRONTEND_EVENT_RECORDING_PAUSED)
        show_notification_str("Recording Paused");
	else if (event == OBS_FRONTEND_EVENT_RECORDING_UNPAUSED)
        show_notification_str("Recording Unpaused");
	else if (event == OBS_FRONTEND_EVENT_RECORDING_STOPPED)
        show_notification_str("Recording Stopped");
	else if (event == OBS_FRONTEND_EVENT_STREAMING_STARTED)
        show_notification_str("Streaming Started");
	else if (event == OBS_FRONTEND_EVENT_STREAMING_STOPPED)
        show_notification_str("Streaming Stopped");
	else if (event == OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED)
        show_notification_str("Replay Buffer Started");
	else if (event == OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED)
        show_notification_str("Replay Buffer Stopped");

	UNUSED_PARAMETER(private_data);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

static int
callback([[maybe_unused]] struct dl_phdr_info *info, [[maybe_unused]] size_t size, [[maybe_unused]] void* data) {
    if (std::string(info->dlpi_name).find("obs-ffmpeg.so") != std::string::npos)
        moduleBase = info->dlpi_addr;

    return 0;
}

MODULE_EXPORT bool obs_module_load() {
	#ifdef LIBNOTIFY_FOUND
	// Initialize libnotify
	if (!notify_init(PLUGIN_NAME)) {
		// TODO: failed to initialize notification library
	}
	#endif

    dl_iterate_phdr(callback, nullptr);

    const auto ptr1 = &_show_replay_saved_notif;

    // replay_buffer_hotkey: 53 48 89 FB 0F B6 47 39 84 C0 75 04 5B C3 ("obs-ffmpeg.so" + 0xD770)
    mempatch((void*)(moduleBase + 0xD7AE), (void*)"\x48\xB8" /* movabs %rax, */, 2);
    mempatch((void*)(moduleBase + 0xD7AE + 2), (void*)&ptr1 /* 0x7fffbcc40cae etc. */, 8);
    mempatch((void*)(moduleBase + 0xD7AE + 2 + 8), (void*)"\xFF\xE0" /* jmp %rax */, 2);
    mempatch((void*)(moduleBase + 0xD7AE + 2 + 8 + 2), (void*)"\xC3" /* ret */, 1);

    // TODO: replay_buffer_mux_thread: F3 0F 1E FA 41 54 55 53 48 89 FB 48 83 EC 20 ("obs-ffmpeg.so" + 0xE120)

	return true;
}

void obs_module_unload() {
	#ifdef LIBNOTIFY_FOUND
	// Uninitialize libnotify before exiting
	notify_uninit();
	#endif
}

MODULE_EXPORT const char* obs_module_name() {
	return PLUGIN_NAME;
}

MODULE_EXPORT const char* obs_module_description() {
	return PLUGIN_DESCRIPTION;
}

OBS_MODULE_AUTHOR(PLUGIN_AUTHOR)

#pragma clang diagnostic pop
