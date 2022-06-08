#include <config.h>
#ifdef SUPPORT_JPEG
#define STBI_ONLY_JPEG
#endif
#ifdef SUPPORT_PNG
#define STBI_ONLY_PNG
#endif
#ifdef SUPPORT_BMP
#define STBI_ONLY_BMP
#endif
#ifdef SUPPORT_PSD
#define STBI_ONLY_PSD
#endif
#ifdef SUPPORT_TGA
#define STBI_ONLY_TGA
#endif
#ifdef SUPPORT_GIF
#define STBI_ONLY_GIF
#endif
#ifdef SUPPORT_HDR
#define STBI_ONLY_HDR
#endif
#ifdef SUPPORT_PIC
#define STBI_ONLY_PIC
#endif
#ifdef SUPPORT_PNM
#define STBI_ONLY_PNM
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

char* decodeImage(const char* path, int* width, int* height, int* channels, int desiredChannels) {
	stbi_set_flip_vertically_on_load(1);
	char *imgData = stbi_load(path, width, height, channels, desiredChannels);
	return imgData;
}

void freeDecodedImage(char* imgData) {
	stbi_image_free(imgData);
}
