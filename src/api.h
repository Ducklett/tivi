#pragma once

void render_frame();
void blit();
void fail(const char* msg);
void exit_app();
void show_image(const char *path);
void toggle_fullscreen();
void toggle_aliasing();
void toggle_on_top();
void next_image();
void prev_image();
void reset_zoom();

void image_fit();
void image_fill();
void image_fit_horizontal();
void image_fit_vertical();
void image_use_real_size();

typedef struct KeyBind {
	char key;
	void(*action)();
} KeyBind;
