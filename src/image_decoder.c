#include <config.h>
#include <image_decoder.h>

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
#ifdef SUPPORT_QOI
#define QOI_IMPLEMENTATION
#include <qoi.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

int last_index_of(const char *path, char c) {
	int last = -1;
	for (int i = 0; path[i] != '\0'; i++) {
		if (path[i] == c) last = i;
	}
	return last;
}

bool extension_is_supported(const char *path) {
	int lastDotIndex = last_index_of(path, '.');

	// it doesn't have an extension
	if (lastDotIndex == -1) return false;

	const char *pathExt = path + lastDotIndex;

	const char *legalExtensions[] = {
#ifdef SUPPORT_PNG
		".png",
#endif
#ifdef SUPPORT_JPEG
		".jpg", ".jpeg", ".jpe", ".jif", ".jfif", ".jfi",
#endif
#ifdef SUPPORT_BMP
		".bmp", ".dib",
#endif
#ifdef SUPPORT_PSD
		".psd",
#endif
#ifdef SUPPORT_TGA
		".tga", ".icb",	 ".vda", ".vst",
#endif
#ifdef SUPPORT_GIF
		".gif",
#endif
#ifdef SUPPORT_HDR
		".hdr",
#endif
#ifdef SUPPORT_PIC
		".PIC",
#endif
#ifdef SUPPORT_PNM
		".pbm", ".pgm",	 ".ppm", ".pnm",
#endif
#ifdef SUPPORT_QOI
		".qoi",
#endif
	};

	for (int i = 0; i < sizeof(legalExtensions) / sizeof(char *); i++) {
		const char *legalExt = legalExtensions[i];

		if (!strcmp(pathExt, legalExt)) return true;
	}
	return false;
}

char *decode_image(const char *path, int *width, int *height, int *channels, int desiredChannels) {
#ifdef SUPPORT_QOI
	int dotIndex = last_index_of(path, '.');
	if (dotIndex > -1 && !strcmp(path + dotIndex, ".qoi")) {
		printf("is qoi!\n");

		qoi_desc desc;
		char *imgData = qoi_read(path, &desc, desiredChannels);

		*width	  = desc.width;
		*height	  = desc.height;
		*channels = desc.channels;

		*width = desc.width;

		return imgData;
	} else {
#endif
		char *imgData = stbi_load(path, width, height, channels, desiredChannels);
		return imgData;
#ifdef SUPPORT_QOI
	}
#endif
}

void decoded_image_free(char *imgData) {
	stbi_image_free(imgData);
}
