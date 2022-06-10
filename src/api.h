#pragma once

void renderFrame();
void Blit();
void Fail(const char* msg);
void ExitApp();
void ShowImage(const char *path);
void ToggleFullscreen();
void ToggleAliasing();
void ToggleOnTop();
void NextImage();
void PrevImage();
void ResetZoom();

void ImageFit();
void ImageFill();
void ImageFitHorizontal();
void ImageFitVertical();
void ImageUseRealSize();

typedef struct KeyBind {
	char key;
	void(*action)();
} KeyBind;
