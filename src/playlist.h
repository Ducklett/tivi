#pragma once

typedef struct Playlist {
	char** files;
	int fileCount;
	int cursor;
} Playlist;

/*
Filters file list based on supported file extensions,
returns a playlist of all the files to play.
*/
Playlist playlist_from_file_list(char** files, int count);

/*
Replaces any character matching `find` with the `replace` character.
*/
void replace_path(char* path, char find, char replace);

/*
Given a path `foo/bar/baz.png`,
this function finds all image files in directory `foo/bar` and creates a playlist.
*/
Playlist playlist_from_files_in_same_directory_as(const char* path, char** buffer, int bufferLen);

/*
Increments the playlist cursor and return the path of the new image.
*/
char* playlist_next(Playlist* list);

/*
Decrements the playlist cursor and return the path of the new image.
Wraps around to the last image when called at index `0`.
*/
char* playlist_previous(Playlist* list);

/*
Frees resources allocated by `playlist_from_file_list` and resets the playlist
Wraps around to the first image when called at index `pathCount-1`.
*/
void playlist_free(Playlist* list);
