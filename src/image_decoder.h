#pragma once

#include <stdbool.h>

int last_index_of(const char *path, char c);
bool extension_is_supported(const char *path);
char *decode_image(const char *path, int *width, int *height, int *channels, int desiredChannels);
/*
Frees data allocated by decode_image.
*/
void decoded_image_free(char *imgData);
