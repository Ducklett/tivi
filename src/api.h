#pragma once

void renderFrame();
void Blit();
void Fail(const char* msg);
void ExitApp();
void ShowImage(const char *path);
void ToggleFullscreen();
void ToggleOnTop();
void NextImage();
void PrevImage();
void ResetZoom();

typedef struct KeyBind {
	char key;
	void(*action)();
} KeyBind;
