#include <config.h>
#include <image_decoder.h>
#include <playlist.h>
#include <stdbool.h>
#include <string.h>

Playlist playlist_from_file_list(char **files, int count) {
	// let's just be lazy and allocate the max possible size
	char **filteredList = malloc(sizeof(char *) * count);
	int filteredCount	= 0;
	for (int i = 0; i < count; i++) {
		char *filePath = files[i];
		if (extension_is_supported(filePath)) {
			puts(filePath);
			filteredList[filteredCount++] = filePath;
		}
	}
	Playlist p = {
		.files	   = filteredList,
		.fileCount = filteredCount,
		.cursor	   = 0,
	};
	return p;
}

void replace_path(char *path, char find, char replace) {
	for (int i = 0; path[i] != '\0'; i++) {
		if (path[i] == find) path[i] = replace;
	}
}

Playlist playlist_from_files_in_same_directory_as(const char *path, char **buffer, int bufferLen) {

	char directoryPath[MAX_PATH_LENGTH];
	char filePath[MAX_PATH_LENGTH];
	strcpy(directoryPath, path);
	replace_path(directoryPath, '\\', '/');

	int lastSlash = last_index_of(directoryPath, '/');

	if (lastSlash == -1) {
		strcpy(directoryPath, "/");
		strcpy(filePath, path);
	} else {
		directoryPath[lastSlash] = '\0';
		strcpy(filePath, path + lastSlash + 1);
	}

	replace_path(directoryPath, '/', '\\');

	WIN32_FIND_DATAA fdFile;
	HANDLE hFind = NULL;

	char sPath[2048];

	// Specify a file mask. *.* = We want everything!
	sprintf(sPath, "%s\\*.*", directoryPath);

	if ((hFind = FindFirstFileA(sPath, &fdFile)) == INVALID_HANDLE_VALUE) {
		printf("Path not found: [%s]\n", directoryPath);
		return (Playlist){0};
	}

	int i		   = 0;
	int startIndex = 0;
	do {
		if (i >= bufferLen) break;
		if (strcmp(fdFile.cFileName, ".") == 0) continue;
		if (strcmp(fdFile.cFileName, "..") == 0) continue;

		sprintf(sPath, "%s\\%s", directoryPath, fdFile.cFileName);

		if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

		if (extension_is_supported(sPath)) {
			if (strcmp(filePath, fdFile.cFileName) == 0) {
				startIndex = i;
			}
			strncpy(buffer[i], sPath, MAX_PATH_LENGTH);
			i++;
		}
	} while (FindNextFileA(hFind, &fdFile));

	FindClose(hFind);

	return (Playlist){
		.files	   = buffer,
		.fileCount = i,
		.cursor	   = startIndex,
	};
}

char *playlist_next(Playlist *list) {
	list->cursor = (list->cursor + 1) % list->fileCount;
	return list->files[list->cursor];
}

char *playlist_previous(Playlist *list) {
	list->cursor = (list->cursor - 1 + list->fileCount) % list->fileCount;
	return list->files[list->cursor];
}

void playlist_free(Playlist *list) {
	free(list->files);
	list->files		= 0;
	list->fileCount = 0;
	list->cursor	= 0;
}
