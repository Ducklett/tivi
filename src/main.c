#ifndef UNICODE
#define UNICODE
#endif

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <windows.h>
#include<glad.h>
#include<gl/GL.h>
#include <synchapi.h>
#include <stdlib.h>
#include "../assets.h"

#include "imageDecoder.c"
#include "playlist.c"

#ifndef clamp
#define clamp(v,a,b) min(b,max(v,a))
#endif
// general app state
WINDOWPLACEMENT placement = {sizeof(placement)};
HWND hwnd;
HDC hdc;
HGLRC ctx;
int windowWidth,windowHeight;
int aspectHandle;
int scaleHandle;
int offsetHandle;

// image viewing state
Playlist playlist = {0};
float imageScale=1;
float imageOffX=0;
float imageOffY=0;
float imageAspect;
float windowAspect;

// animation
bool realtime=false;
float scaleA=0;
float offXA=0;
float offYA=0;

char pathBuffer[MAX_FILES_IN_PLAYLIST][MAX_PATH_LENGTH];
char* pathPointers[MAX_FILES_IN_PLAYLIST];

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* ======================== API ==================================*/

float lerp(float a, float b, float t) {
	return (1-t)*a + b*t;
}

void renderFrame() {
	const float animSpeed=.25;
	scaleA = lerp(scaleA, imageScale,animSpeed);
	offXA = lerp(offXA, imageOffX,animSpeed);
	offYA = lerp(offYA, imageOffY,animSpeed);

	glUniform1f(scaleHandle, scaleA);
	glUniform2f(offsetHandle, offXA/windowWidth*2,offYA/windowHeight*2);

	glClearColor(0,0,0,0.);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	SwapBuffers(hdc);
}

void ResetZoom() {
	imageScale=1;
	imageOffX=0;
	imageOffY=0;
	Blit();
}

void Blit() {
	// if (windowSizeChanged) {
		RECT rect;
		GetClientRect(hwnd,&rect);
		windowWidth = rect.right-rect.left;
		windowHeight = rect.bottom-rect.top;
		windowAspect = (float)windowWidth / windowHeight;
		glUniform2f(aspectHandle, windowAspect, imageAspect);

		glViewport(0,0,windowWidth,windowHeight);
	// }

	if (!realtime) {
		// double buffered
		renderFrame();
		renderFrame();
	}
}

void Fail(const char* msg) {
	MessageBoxA(hwnd, msg, "Error", MB_OK|MB_ICONERROR);
	PostQuitMessage(1);
}
void ExitApp()
{
	// TODO: figure out how to properly stop the app
	ExitProcess(0);
	// PostQuitMessage(0);
}

void ShowImage(const char *path)
{
	const int wantedChannels = 4;
	int width, height, nChannels;
	char *img = decodeImage(path, &width, &height, &nChannels, wantedChannels);
	// assert(nChannels == wantedChannels);

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

	imageAspect = (float)width / height;

	// TODO: set client rect instead of window size (this leaves small black borders)
	// TODO: don't do it in fullscreen
	SetWindowPos(hwnd, NULL, 0, 0, windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
	SetWindowTextA(hwnd, path);

	static unsigned int texture =-1;

	if (texture != -1) {
		glDeleteTextures(1,&texture);
	}

	glDeleteTextures(1, &texture);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag > 1 ? GL_NEAREST : GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	freeDecodedImage(img);

	imageAspect = (float)width / height;

	Blit();
}

void ToggleFullscreen()
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
			SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
			windowWidth = mi.rcMonitor.right - mi.rcMonitor.left;
			windowHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;
			SetWindowPos(hwnd,
						 HWND_TOP,
						 mi.rcMonitor.left,
						 mi.rcMonitor.top,
						 windowWidth,
						 windowHeight,
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

		}
	} else {
#ifndef FEATURE_BORDERLESS_WINDOW
		SetWindowLongW(hwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
#endif
		SetWindowPlacement(hwnd, &placement);
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
					 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}

	Blit();
}

void ToggleOnTop() {
	static bool onTop = false;
	onTop = !onTop;
	if (onTop) {
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	} else {
		SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
}

void AnimateImageTransition(float direction) {
	imageScale=1;
	imageOffX=0;
	imageOffY=0;
	offXA = 300*direction;
	scaleA=.8;
}

void NextImage() {
	AnimateImageTransition(1);
	char* imagePath = PlaylistNext(&playlist);
	ShowImage(imagePath);
}

void PrevImage() {
	AnimateImageTransition(-1);
	char* imagePath = PlaylistPrevious(&playlist);
	ShowImage(imagePath);
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
			playlist = PlaylistFromFilesInSameDirectoryAs(argv[1],pathPointers, MAX_FILES_IN_PLAYLIST);
		} else {
			playlist = PlaylistFromFileList(argv+1,argc-1);
		}
	}

	wchar_t className[] = L"imageviewer";
	WNDCLASSEXW window = {0};
	window.cbSize = sizeof(WNDCLASSEXA);
	window.style = CS_HREDRAW |CS_VREDRAW|CS_OWNDC;
	window.lpfnWndProc = WindowProc;
	window.hInstance = hInstance;
	window.lpszClassName = className;

	window.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	window.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);

	if (!RegisterClassExW(&window)) Fail("RegisterClass failed");

	hwnd = CreateWindowExW( //
#ifdef FEATURE_DRAG_AND_DROP
		WS_EX_ACCEPTFILES,
#else
		0,
#endif
		className,
		L"ImageViewer",
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

	if (hwnd == NULL) Fail("hwnd was null");

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
	static bool leftClicking = false;

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
		if (!pixelFormat) Fail("Cound not find buffer matching pixel format");
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

		aspectHandle = glGetUniformLocation(program, "aspect");
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
		ExitApp();
	} break;
	case WM_ACTIVATE: {
		// HACK: load first image here because doing it in create doesn't work...
		if (!windowWidth && playlist.fileCount) {
			ShowImage(playlist.files[playlist.cursor]);
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
			FreePlaylist(&playlist);
		}

		char** buf = pathPointers;

		if (fileCount>MAX_FILES_IN_PLAYLIST) fileCount = MAX_FILES_IN_PLAYLIST;
		if (fileCount == 1) {
			DragQueryFileA(drop, 0, pathPointers[0], MAX_PATH_LENGTH);
			playlist = PlaylistFromFilesInSameDirectoryAs(pathPointers[0],pathPointers, MAX_FILES_IN_PLAYLIST);
		} else {
			for(int i = 0;i<fileCount;i++) {
				// DragQueryFileA(drop, i, pathPointers[fileCount-i], MAX_PATH_LENGTH);
				DragQueryFileA(drop, i, pathPointers[i], MAX_PATH_LENGTH);
			}

			playlist = PlaylistFromFileList(buf,fileCount);
		}

		DragFinish(drop);

		ShowImage(playlist.files[playlist.cursor]);
		// ShowImage(path);
		return 0;
	} break;
#endif
	// case WM_NCHITTEST: {
	// 	LRESULT hit = DefWindowProc(hwnd, message, wParam, lParam);
	// 	if (hit == HTCLIENT) {
	// 		const int resize = HTBOTTOMRIGHT;
	// 		const int drag = HTCAPTION;
	// 		hit = shiftHeld ? resize : drag;
	// 	}
	// 	return hit;
	// } break;
	case WM_MOUSEMOVE: {
		static int prevX = 0;
		static int prevY = 0;

		// middle mouse button down
		if (wParam & MK_MBUTTON) {
			int mouseX = lParam&0xFFFF;
			int mouseY = lParam>>16;
			if (prevX||prevY) {
				int deltaX = mouseX-prevX;
				int deltaY = mouseY-prevY;
				imageOffX += (float)deltaX/imageScale;
				imageOffY -= (float)deltaY/imageScale;
				imageOffX = clamp(imageOffX,-windowWidth*.9, windowWidth*.9);
				imageOffY = clamp(imageOffY,-windowHeight*.9, windowHeight*.9);
				Blit();
			}
			prevX = mouseX;
			prevY = mouseY;
			// printf("pan %d,%d -- %d\n", lParam&0xFFFF, lParam>>16, wParam);
		} else {
			prevY=0;
			prevX=0;
		}

		return DefWindowProc(hwnd, message, wParam, lParam);
	} break;
	case WM_MOUSEWHEEL: {
		int delta = (int)wParam>>16;
		imageScale += (float)delta * .001 * imageScale;

		Blit();
		return DefWindowProc(hwnd, message, wParam, lParam);
	} break;
	// case WM_LBUTTONDOWN: {
	// 	printf("click down!\n");
	// 	leftClicking=true;
	// } break;
	// case WM_LBUTTONUP: {
	// 	printf("click up!\n");
	// 	leftClicking=false;
	// } break;
	case WM_CHAR:    /* FALLTHROUGH */
	case WM_KEYDOWN: {
		if (wParam == VK_SHIFT) shiftHeld = true;

		for(int i = 0; i < keyBindingsCount;i++) {
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
		Blit();
		// TODO: figure out how to do proper game loop
		if (realtime) {
			renderFrame();
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
