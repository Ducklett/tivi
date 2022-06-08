#pragma once

#include <windows.h>
#include <api.h>

// +---------------------------------
// included features
// +--------------------------
#define FEATURE_DRAG_AND_DROP
// #define FEATURE_BORDERLESS_WINDOW

// +---------------------------------
// included image format decoders
// +--------------------------
#define SUPPORT_JPEG
#define SUPPORT_PNG
#define SUPPORT_BMP
// #define SUPPORT_PSD
#define SUPPORT_TGA
// #define SUPPORT_GIF
// #define SUPPORT_HDR
// #define SUPPORT_PIC
#define SUPPORT_PNM

// +---------------------------------
// bounds
// +--------------------------
#define MAX_FILES_IN_PLAYLIST 1024
#define MAX_PATH_LENGTH 260

// +--------------------------------
// keyboard shortcuts
// +--------------------------
const KeyBind keyBindings[] = {
	{ .key = 'o',        .action = ToggleOnTop },
	{ .key = 'f',        .action = ToggleFullscreen },
	{ .key = 'q',        .action = ExitApp },
	{ .key = VK_ESCAPE,  .action = ExitApp },
	{ .key = 'r',        .action = Blit },
	{ .key = '0',        .action = ResetZoom },
	{ .key = 'j',        .action = NextImage },
	{ .key = 'k',        .action = PrevImage },
	{ .key = VK_RIGHT,   .action = NextImage },
	{ .key = VK_LEFT,    .action = PrevImage },
};
const int keyBindingsCount = sizeof(keyBindings) / sizeof(KeyBind);