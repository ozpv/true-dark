#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int radius = 0;
static int screen_width = 0, screen_height = 0;
static int screen_x = 0, screen_y = 0;
static COLORREF darkness = 0xFFFFFF;

void draw_circle_around_cursor(HWND window_handle) {
	int cursor_x = 0, cursor_y = 0;

	HRGN window_rgn = CreateRectRgn(screen_x, screen_y, screen_width, screen_height);

	POINT cursor_pos;
	GetCursorPos(&cursor_pos);

	cursor_x = cursor_pos.x - screen_x;
	cursor_y = cursor_pos.y - screen_y;

	HRGN circle_rgn = CreateEllipticRgn(cursor_x - radius, cursor_y - radius, cursor_x + radius, cursor_y + radius);

	CombineRgn(window_rgn, window_rgn, circle_rgn, RGN_XOR);

	SetWindowRgn(window_handle, window_rgn, FALSE);
}

int draw_rect_selection(void) {
	
}

LRESULT CALLBACK true_dark_callback(HWND window_handle, UINT message, WPARAM message_info, LPARAM param) {
	switch (message) {
		case WM_CREATE: {
			HDC window_dc = GetDC(window_handle);
			RECT window_rect = { screen_x, screen_y, screen_width, screen_height };
			FillRect(window_dc, &window_rect, CreateSolidBrush(darkness));

			DeleteDC(window_dc);

			draw_circle_around_cursor(window_handle);
			break;
		}
		case WM_MOUSEMOVE:
			draw_circle_around_cursor(window_handle);
			break;
		default:
			break;
	}

	return DefWindowProcW(window_handle, message, message_info, param);
}

void setup(void) {
	int def = 0;
	HWND progman_handle = FindWindowW(L"Progman", NULL);

	if (progman_handle == NULL) {
		printf("Error: true dark could not find the Progman window. Defaulting to the primary monitor.");
		def = 1;
	}

	RECT screen_rect;
	
#pragma warning(disable : 6387)
	if (!GetWindowRect(progman_handle, &screen_rect)) {
		printf("Error: true dark could not get the dimensions of Progman. Defaulting to the primary monitor.");
		def = 1;
	}

	if (def) {
		screen_rect.left = 0;
		screen_rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
		screen_rect.top = 0;
		screen_rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
	}

	screen_x = screen_rect.left;
	screen_y = screen_rect.top;
	screen_width = screen_rect.right - screen_x;
	screen_height = screen_rect.bottom - screen_y;

	WNDCLASSW window_class = { 0, true_dark_callback, 0, 0, GetModuleHandle(NULL), NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL, L"true dark" };

	RegisterClassW(&window_class);

	CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, window_class.lpszClassName, L"true dark", WS_POPUP, screen_x, screen_y, screen_width, screen_height, HWND_DESKTOP, NULL, window_class.hInstance, NULL);

	MSG message;

	while (GetMessageW(&message, NULL, 0, 0)) {
		DispatchMessageW(&message);
	}
}

void usage() {
	printf("true dark usage:\n\t[-r] [number] set the radius of the circle around your cursor.\n\t[-r] [hex e.g. 0xffffff] set the color of the background.");

	exit(0);
}

int main(int argc, char *argv[]) {
	int result = 0;
	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-r"))
			radius = atoi(argv[++i]);
		else if (!strcmp(argv[i], "-c"))
			result = sscanf(argv[++i], "%x", &darkness);
		else {
			usage();
		}
	}

	setup();

	return 0;
}