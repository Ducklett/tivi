#ifndef UNICODE
#define UNICODE
#endif

#include <stdbool.h>
#include <stdio.h>
#include <windows.h>
#include<glad.h>
#include<gl/GL.h>
#include <stdlib.h>
#include <config.h>
#include <image_decoder.h>
#include <playlist.h>
#include "../assets.h"

#ifndef CLAMP
#define CLAMP(v,a,b) min(b,max(v,a))
#endif

// general app state
WINDOWPLACEMENT placement = {sizeof(placement)};
HWND hwnd;
HDC hdc;
HGLRC ctx;
int windowWidth,windowHeight;
int imageWidth,imageHeight;
int stretchHandle;
int scaleHandle;
int offsetHandle;

typedef enum FitMode {
	FM_REAL_SIZE,
	FM_FIT,
	FM_FILL,
	FM_FIT_HORIZONTAL,
	FM_FIT_VERTICAL
} FitMode;

// image viewing state
Playlist playlist = {0};
bool aliasing=false;
float imageScale=1;
float imageOffX=0;
float imageOffY=0;
float stretchX=1;
float stretchY=1;
FitMode fitMode = FM_FIT;

// animation
bool realtime=false;
float scaleA=0;
float offXA=0;
float offYA=0;
float stretchXA=1;
float stretchYA=1;

char pathBuffer[MAX_FILES_IN_PLAYLIST][MAX_PATH_LENGTH];
char* pathPointers[MAX_FILES_IN_PLAYLIST];

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* ======================== API ==================================*/

void image_fit() { 
	fitMode = FM_FIT;
	imageScale=1;
	imageOffX=0;
	imageOffY=0;
	blit();
}
void image_fill() { 
	fitMode = FM_FILL;
	imageScale=1;
	imageOffX=0;
	imageOffY=0;
	blit();
}
void image_fit_horizontal() { 
	fitMode = FM_FIT_HORIZONTAL;
	imageScale=1;
	imageOffX=0;
	imageOffY=0;
	blit();
}
void image_fit_vertical() { 
	fitMode = FM_FIT_VERTICAL;
	imageScale=1;
	imageOffX=0;
	imageOffY=0;
	blit();
}
void image_use_real_size() { 
	fitMode = FM_REAL_SIZE;
	imageScale=1;
	imageOffX=0;
	imageOffY=0;
	blit();
}

float lerp(float a, float b, float t) {
	return (1-t)*a + b*t;
}

void render_frame() {
	scaleA = lerp(scaleA, imageScale,SCALE_SPEED);
	offXA = lerp(offXA, imageOffX,PAN_SPEED);
	offYA = lerp(offYA, imageOffY,PAN_SPEED);
	stretchXA = lerp(stretchXA,stretchX,STRETCH_SPEED);
	stretchYA = lerp(stretchYA,stretchY,STRETCH_SPEED);

	glUniform1f(scaleHandle, scaleA);
	glUniform2f(offsetHandle, offXA/windowWidth*2,offYA/windowHeight*2);
	
	// TODO: figure out why this happens...
	if (isnan(stretchYA)) {
		stretchYA = stretchY;
	}

	glUniform2f(stretchHandle, stretchXA, stretchYA);

	glClearColor(0,0,0,0.);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	SwapBuffers(hdc);
}

void reset_zoom() {
	imageScale=1;
	imageOffX=0;
	imageOffY=0;
	blit();
}

void blit() {
	RECT rect;
	GetClientRect(hwnd,&rect);
	windowWidth = rect.right-rect.left;
	windowHeight = rect.bottom-rect.top;

	switch (fitMode) {
	case FM_FIT: {
		float aspect = ((float)imageWidth/imageHeight)/((float)windowWidth/windowHeight);
		if (aspect<1) {
			stretchX=aspect;
			stretchY=1;
		} else {
			stretchX=1;
			stretchY=1/aspect;
		}
	} break;
	case FM_FILL: {
		float aspect = ((float)imageWidth/imageHeight)/((float)windowWidth/windowHeight);
		if (aspect<1) {
			stretchX=1;
			stretchY=1/aspect;
		} else {
			stretchX=aspect;
			stretchY=1;
		}
	} break;
	case FM_REAL_SIZE: {
		stretchX=(float)imageWidth/windowWidth;
		stretchY=(float)imageHeight/windowHeight;
	} break;
	case FM_FIT_HORIZONTAL: {
		float aspect = ((float)imageWidth/imageHeight)/((float)windowWidth/windowHeight);
		stretchX=1;
		stretchY=1/aspect;
	} break;
	case FM_FIT_VERTICAL: {
		float aspect = ((float)imageWidth/imageHeight)/((float)windowWidth/windowHeight);
		stretchX=aspect;
		stretchY=1;
	} break;
	default: {
		printf("unsupported fit mode %d\n", fitMode);
		exit(1);
	} break;
	}

	glViewport(0,0,windowWidth,windowHeight);

	if (!realtime) {
		// double buffered
		render_frame();
		render_frame();
	}
}

void fail(const char* msg) {
	MessageBoxA(hwnd, msg, "Error", MB_OK|MB_ICONERROR);
	PostQuitMessage(1);
}
void exit_app()
{
	// TODO: figure out how to properly stop the app
	ExitProcess(0);
	// PostQuitMessage(0);
}


void show_image(const char *path)
{
	static unsigned int texture =-1;
	const int wantedChannels = 4;
	int width, height, nChannels;
	char *img = decode_image(path, &width, &height, &nChannels, wantedChannels);
	// assert(nChannels == wantedChannels);
	imageWidth=width;
	imageHeight=height;

	if (!img) {
		printf("failed to load image: %s\n", path);
		exit(1);
	}

	const int minSize = 300;
	const int maxWidth = 1920;
	const int maxHeight = 1080;
	float mag = 1;
	if (width < height) {
		assert(width > 0);
		while (width * mag < minSize && height * mag < maxHeight) {
			mag++;
		}
	} else {
		assert(width > 0);
		while (height * mag < minSize && width * mag < maxWidth) {
			mag++;
		}
	}

	if (width*mag>maxWidth) {
		float factor = (float)maxWidth/(width*mag);
		mag*=factor;
	}

	if (height*mag>maxHeight) {
		float factor = (float)maxHeight/(height*mag);
		mag*=factor;
	}

	windowWidth = max(windowWidth, width*mag);
	windowHeight = max(windowHeight, height*mag);

	// TODO: set client rect instead of window size (this leaves small black borders)
	// TODO: don't do it in fullscreen
	SetWindowPos(hwnd, NULL, 0, 0, windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
	SetWindowTextA(hwnd, path);

	if (texture != -1) {
		glDeleteTextures(1,&texture);
	}

	glDeleteTextures(1, &texture);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, aliasing ? GL_NEAREST : GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	decoded_image_free(img);

	blit();
}

void toggle_aliasing() {
	aliasing=!aliasing;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, aliasing ? GL_NEAREST : GL_LINEAR);
	blit();
}

void toggle_fullscreen()
{
	long dwStyle = GetWindowLong(hwnd, GWL_STYLE);
	static bool fullscreen = false;
	fullscreen = !fullscreen;

	if (fullscreen) {
		MONITORINFO mi = {sizeof(mi)};

		bool gotPlacement = GetWindowPlacement(hwnd, &placement);
		bool gotMonitor = GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi);
		if (!gotPlacement) puts("Could not get placement\n");
		if (!gotMonitor) puts("Could not get monitor\n");
		if (gotPlacement&& gotMonitor) {
			scaleA=.5;
			windowWidth = mi.rcMonitor.right - mi.rcMonitor.left;
			windowHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;
			SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(hwnd,
						 HWND_TOP,
						 mi.rcMonitor.left,
						 mi.rcMonitor.top,
						 windowWidth+1,   // if we don't do this we get a flicker when entering fullscreen...
						 windowHeight+1,
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

		}
	} else {
		scaleA=2;
#ifndef FEATURE_BORDERLESS_WINDOW
		SetWindowLongW(hwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
#endif
		SetWindowPlacement(hwnd, &placement);
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
					 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}

	blit();
}

void toggle_on_top() {
	static bool onTop = false;
	onTop = !onTop;
	if (onTop) {
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	} else {
		SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
}

void animate_image_transition(float direction) {
	imageScale=1;
	imageOffX=0;
	imageOffY=0;
	offXA = 300*direction;
	scaleA=.8;
}

void next_image() {
	animate_image_transition(1);
	char* imagePath = playlist_next(&playlist);
	show_image(imagePath);
}

void prev_image() {
	animate_image_transition(-1);
	char* imagePath = playlist_previous(&playlist);
	show_image(imagePath);
}

/* ====================== END API ================================*/
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	int argc = __argc;
	char** argv = __argv;

	// initialize path pointers
	for(int i = 0; i < MAX_FILES_IN_PLAYLIST; i++) {
		char* pathPointer = pathBuffer[i];
		pathPointers[i] = pathPointer;
	}

	if (argc>1) {
		if (argc==2) {
			playlist = playlist_from_files_in_same_directory_as(argv[1],pathPointers, MAX_FILES_IN_PLAYLIST);
		} else {
			playlist = playlist_from_file_list(argv+1,argc-1);
		}
	}

	wchar_t className[] = L"tivi";
	WNDCLASSEXW window = {0};
	window.cbSize = sizeof(WNDCLASSEXA);
	window.style = CS_HREDRAW |CS_VREDRAW|CS_OWNDC;
	window.hCursor = LoadCursor(NULL, IDC_ARROW);
	window.lpfnWndProc = WindowProc;
	window.hInstance = hInstance;
	window.lpszClassName = className;

	window.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	window.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);

	if (!RegisterClassExW(&window)) fail("RegisterClass failed");

	hwnd = CreateWindowExW( //
#ifdef FEATURE_DRAG_AND_DROP
		WS_EX_ACCEPTFILES,
#else
		0,
#endif
		className,
		L"Tivi - Drop image files to view",
		WS_OVERLAPPEDWINDOW,
		// x,y
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		// width,height
		500,
		500,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (hwnd == NULL) fail("hwnd was null");

	ShowWindow(hwnd, nCmdShow);

#ifdef FEATURE_BORDERLESS_WINDOW
	long dwStyle = GetWindowLong(hwnd, GWL_STYLE);
	SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
#endif

	realtime=true;

	MSG msg = {0};
	while (GetMessage(&msg, hwnd, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool shiftHeld = false;

	switch (message) {
	case WM_CREATE: {
		const PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,                    // version
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
			PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
			32,                   // Colordepth of the framebuffer.
			0, 0, 0, 0, 0, 0,     // redBits,redShift,greenBits,greenShift,blueBits,blueShift
			0,0,                  // alphaBits, alphaShift
			0,                    // accumBits (??)
			0, 0, 0, 0,           // accum r,g,b,a
			24,                   // Number of bits for the depthbuffer
			8,                    // Number of bits for the stencilbuffer
			0,                    // Number of Aux buffers in the framebuffer.
			PFD_MAIN_PLANE,       // layer type
			0,                    // (reseved)
			0, 0, 0               // layerMask,visibleMask,damageMask (??)
		};
		hdc = GetDC(hwnd);
		int pixelFormat = ChoosePixelFormat(hdc,&pfd);
		if (!pixelFormat) fail("Cound not find buffer matching pixel format");
		SetPixelFormat(hdc,pixelFormat,&pfd);

		ctx = wglCreateContext(hdc);
		wglMakeCurrent(hdc,ctx);

		gladLoadGL();

		const char* v = layoutVertexShaderSource;
		const char* f = layoutFragmentShaderSource;
		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &v, NULL);
		glCompileShader(vertexShader);
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &f, NULL);
		glCompileShader(fragmentShader);
		unsigned int program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		float vertices[] = {
			-1, -1, 0, // bottom left
			+1, -1, 0, // bottom right
			-1, +1, 0, // top left
			+1, +1, 0, // top right
		};
		unsigned int indices[] = {
			0, 1, 2, 1, 3, 2,
		};
		unsigned int VBO, VAO, EBO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);

		glUseProgram(program);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(VAO);

		stretchHandle = glGetUniformLocation(program, "stretch");
		scaleHandle = glGetUniformLocation(program, "scale");
		offsetHandle = glGetUniformLocation(program, "offset");

		glUniform1f(scaleHandle, imageScale);
		glUniform2f(offsetHandle, 0,0);

		wglSwapIntervalEXT(1);

		return 0;
	} break;
	case WM_DESTROY: {
		printf("%s\n", "WM_DESTROY called");
		wglMakeCurrent(hdc,NULL);
		wglDeleteContext(ctx);
		ReleaseDC(hwnd, hdc);
		exit_app();
	} break;
	case WM_ACTIVATE: {
		// HACK: load first image here because doing it in create doesn't work...
		if (!windowWidth && playlist.fileCount) {
			show_image(playlist.files[playlist.cursor]);
		}
		return DefWindowProc(hwnd, message, wParam, lParam);
	} break;
#ifdef FEATURE_DRAG_AND_DROP
	case WM_DROPFILES: {
		HDROP drop = (HDROP)wParam;

		int fileCount = DragQueryFileA(drop, 0xFFFFFFFF, NULL, 0);

		if (!fileCount) return 0;

		// HACK: don't free static path buffer
		if (playlist.files != pathPointers) {
			playlist_free(&playlist);
		}

		char** buf = pathPointers;

		if (fileCount>MAX_FILES_IN_PLAYLIST) fileCount = MAX_FILES_IN_PLAYLIST;
		if (fileCount == 1) {
			DragQueryFileA(drop, 0, pathPointers[0], MAX_PATH_LENGTH);
			playlist = playlist_from_files_in_same_directory_as(pathPointers[0],pathPointers, MAX_FILES_IN_PLAYLIST);
		} else {
			for(int i = 0;i<fileCount;i++) {
				DragQueryFileA(drop, i, pathPointers[i], MAX_PATH_LENGTH);
			}

			playlist = playlist_from_file_list(buf,fileCount);
		}

		DragFinish(drop);

		show_image(playlist.files[playlist.cursor]);
		return 0;
	} break;
#endif
	case WM_NCHITTEST: {
		LRESULT hit = DefWindowProc(hwnd, message, wParam, lParam);
		if (hit == HTCLIENT) {
			const int resize = HTBOTTOMRIGHT;
			const int drag = HTCAPTION;
			hit = shiftHeld ? resize : drag;
		}
		return hit;
	} break;
	// passthrough for middle mouse button since we use it for panning
	case WM_NCMBUTTONDOWN: {
		SetCapture(hwnd);
		return 0;
	} break;
	case WM_MOUSEMOVE: {
		static int prevX = 0;
		static int prevY = 0;

		// middle mouse button down
		if (wParam & MK_MBUTTON || shiftHeld) {
			SetCapture(hwnd);
			int mouseX = (int16_t)(lParam&0xFFFF);
			int mouseY = (int16_t)(lParam>>16);
			if (prevX||prevY) {
				int deltaX = mouseX-prevX;
				int deltaY = mouseY-prevY;
				imageOffX += (float)deltaX/imageScale;
				imageOffY -= (float)deltaY/imageScale;

				float clampMultiplier = max(imageScale,1)*.9; 
				imageOffX = CLAMP(imageOffX,-imageWidth*clampMultiplier, imageWidth*clampMultiplier);
				imageOffY = CLAMP(imageOffY,-imageHeight*clampMultiplier, imageHeight*clampMultiplier);
				blit();
			}
			prevX = mouseX;
			prevY = mouseY;
			// printf("pan %d,%d -- %d\n", lParam&0xFFFF, lParam>>16, wParam);
		} else {
			ReleaseCapture();
			prevY=0;
			prevX=0;
		}

		return DefWindowProc(hwnd, message, wParam, lParam);
	} break;
	case WM_MOUSEWHEEL: {
		int delta = (int)wParam>>16;

		if (aliasing && fitMode==FM_REAL_SIZE) {
			// let's scale in fixed increments to preserve the pixel perfect aesthetics
			if (delta>0) {
				if (imageScale<1) {
					imageScale*=2;
				} else {
					imageScale+=1;
				}
			} else {
				if (imageScale<=1) {
					imageScale*=.5;
				} else {
					imageScale-=1;
				}
			}
		} else {
			imageScale += (float)delta * .001 * imageScale;
		}

		blit();
		return DefWindowProc(hwnd, message, wParam, lParam);
	} break;
	case WM_CHAR:    /* FALLTHROUGH */
	case WM_KEYDOWN: {
		if (wParam == VK_SHIFT) shiftHeld = true;

		for(int i = 0; i < KEYBINDINGS_COUNT;i++) {
			KeyBind k = keyBindings[i];
			if (k.key == wParam) {
				k.action();
				return 0;
			}
		}

		return DefWindowProc(hwnd, message, wParam, lParam);
	} break;
	case WM_KEYUP: {
		if (wParam == VK_SHIFT) shiftHeld = false;
		return DefWindowProc(hwnd, message, wParam, lParam);
	} break;

	case WM_PAINT: {
		blit();
		// TODO: figure out how to do proper game loop
		if (realtime) {
			render_frame();
		} else {
			return DefWindowProc(hwnd, message, wParam, lParam);
		}
	} break;
	default: {
		return DefWindowProc(hwnd, message, wParam, lParam);
	} break;
	}
	return 0;
}
