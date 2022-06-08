#include <stdbool.h>
#include <config.h>
#include <string.h>

typedef struct Playlist {
	char** files;
	int fileCount;
	int cursor;
} Playlist;

int LastIndexOf(const char* path, char c) {
	int last = -1;
	for(int i = 0; path[i] != '\0'; i++) {
		if (path[i] == c) last = i;
	}
	return last;
}

bool ExtensionIsSupported(const char* path) {
	int lastDotIndex = LastIndexOf(path, '.');

	// it doesn't have an extension
	if (lastDotIndex == -1) return false;

	const char* pathExt = path+lastDotIndex;

	const char* legalExtensions[] = {
#ifdef SUPPORT_PNG
		".png",
#endif
#ifdef SUPPORT_JPEG
		".jpg", ".jpeg", ".jpe", ".jif", ".jfif",".jfi",
#endif
#ifdef SUPPORT_BMP
		".bmp", ".dib",
#endif
#ifdef SUPPORT_PSD
		".psd",
#endif
#ifdef SUPPORT_TGA
		".tga", ".icb", ".vda", ".vst",
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
		".pbm", ".pgm", ".ppm", ".pnm",
#endif
	};

	for(int i = 0; i<sizeof(legalExtensions)/sizeof(char*); i++) {
		const char* legalExt = legalExtensions[i];

		if (!strcmp(pathExt,legalExt)) return true;
	}
	return false;
}

/*
Filters file list based on supported file extensions,
returns a playlist of all the files to play
*/
Playlist PlaylistFromFileList(char** files, int count) {
	// let's just be lazy and allocate the max possible size
	char** filteredList = malloc(sizeof(char*)*count);
	int filteredCount = 0;
	for(int i = 0; i < count; i++) {
		char* filePath = files[i];
		if (ExtensionIsSupported(filePath)) {
			puts(filePath);
			filteredList[filteredCount++] = filePath;
		}
	}
	Playlist p = {
		.files = filteredList,
		.fileCount = filteredCount,
		.cursor = 0,
	};
	return p;
}

void ReplacePath(char* path, char find, char replace) {
	for (int i = 0; path[i] != '\0'; i++) {
		if (path[i] == find) path[i] = replace;
	}
}

Playlist PlaylistFromFilesInSameDirectoryAs(const char* path, char** buffer, int bufferLen) {

	char directoryPath[MAX_PATH_LENGTH];
	char filePath[MAX_PATH_LENGTH];
	strcpy(directoryPath, path);
	ReplacePath(directoryPath, '\\', '/');

	int lastSlash = LastIndexOf(directoryPath, '/');

	if (lastSlash == -1) {
		strcpy(directoryPath, "/");
		strcpy(filePath,path);
	} else {
		directoryPath[lastSlash] = '\0';
		strcpy(filePath,path+lastSlash+1);
	}
	printf("playlist for file: %s\n", filePath);

	ReplacePath(directoryPath, '/', '\\');
	printf("Directory: %s\n", directoryPath);

    WIN32_FIND_DATAA fdFile;
    HANDLE hFind = NULL;

    char sPath[2048];

    //Specify a file mask. *.* = We want everything!
    sprintf(sPath, "%s\\*.*", directoryPath);

    if((hFind = FindFirstFileA(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        printf("Path not found: [%s]\n", directoryPath);
        return (Playlist){0};
    }

	int i = 0;
	int startIndex = 0;
    do
    {
		if (i>=bufferLen) break;

        //Find first file will always return "."
        //    and ".." as the first two directories.
        if(strcmp(fdFile.cFileName, ".") != 0
                && strcmp(fdFile.cFileName, "..") != 0)
        {
            //Build up our file path using the passed in
            //  [sDir] and the file/foldername we just found:
            sprintf(sPath, "%s\\%s", directoryPath, fdFile.cFileName);

            //Is the entity a File or Folder?
            if(fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
            {
                // printf("Directory: %s\n", sPath);
                // ListDirectoryContents(sPath); //Recursion, I love it!
            }
            else {
				if (ExtensionIsSupported(sPath)) {
					puts(filePath);
					puts(fdFile.cFileName);
					if (strcmp(filePath, fdFile.cFileName) == 0) {
						startIndex=i;
						printf("matched! %s\n", filePath);
					}
					strncpy(buffer[i],sPath, MAX_PATH_LENGTH);
					i++;
				}
            }
        }
    }
    while(FindNextFileA(hFind, &fdFile)); //Find the next file.

    FindClose(hFind); //Always, Always, clean things up!

	return (Playlist) {
		.files=buffer,
		.fileCount = i,
		.cursor=startIndex,
	};
}

char* PlaylistNext(Playlist* list) {
	list->cursor = (list->cursor+1) % list->fileCount;
	return list->files[list->cursor];
}

char* PlaylistPrevious(Playlist* list) {
	list->cursor = (list->cursor-1 + list->fileCount) % list->fileCount;
	return list->files[list->cursor];
}

/*
Frees resources allocated by `PlaylistFromFileList` and resets the playlist
*/
void FreePlaylist(Playlist* list) {
	free(list->files);
	list->files=0;
	list->fileCount=0;
	list->cursor=0;
}
