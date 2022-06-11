#pragma once

#include <api.h>
#include <windows.h>

// +---------------------------------
// included features
// +--------------------------
#define FEATURE_DRAG_AND_DROP
#define FEATURE_BORDERLESS_WINDOW

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
#define SUPPORT_QOI

// +---------------------------------
// bounds
// +--------------------------
#define MAX_FILES_IN_PLAYLIST 1024
#define MAX_PATH_LENGTH		  260

// +--------------------------------
// keyboard shortcuts
// +--------------------------
const KeyBind keyBindings[] = {
	{.key = 'o', .action = toggle_on_top},
	{.key = 'f', .action = toggle_fullscreen},
	{.key = 'q', .action = exit_app},
	{.key = VK_ESCAPE, .action = exit_app},
	{.key = 'r', .action = blit},
	{.key = 'a', .action = toggle_aliasing},
	{.key = 'j', .action = next_image},
	{.key = 'k', .action = prev_image},
	{.key = VK_RIGHT, .action = next_image},
	{.key = VK_LEFT, .action = prev_image},

	{.key = '0', .action = reset_zoom},
	{.key = '1', .action = image_use_real_size},
	{.key = '2', .action = image_fit},
	{.key = '3', .action = image_fit_horizontal},
	{.key = 'h', .action = image_fit_horizontal},
	{.key = '4', .action = image_fit_vertical},
	{.key = 'v', .action = image_fit_vertical},
	{.key = '5', .action = image_fill},
};

const int KEYBINDINGS_COUNT = sizeof(keyBindings) / sizeof(KeyBind);

// +--------------------------------
// animation
// +--------------------------
const float SCALE_SPEED	  = .15; // 1 = instant
const float PAN_SPEED	  = .25; // 1 = instant
const float STRETCH_SPEED = .6;	 // 1 = instant
