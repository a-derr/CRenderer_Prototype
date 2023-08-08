#include "Display.h"
#include "Vector.h"

int window_width = 800;
int window_height = 600;

 SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* color_buffer_texture = NULL;

uint32_t* color_buffer = NULL;
float* z_buffer = NULL;

bool initialize_window(void) {
	// SDL Library initialization
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error Initializing SDL.\n");
		return false;
	}
	//Use sdl to query what is the full screen max width and height
	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);
	window_width = display_mode.w;
	window_height = display_mode.h;

	// To do: create window
	window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_BORDERLESS);
	if (!window) {
		fprintf(stderr, "Error creating SDL window\n");
		return false;
	}
	// To do: create renderer
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		fprintf(stderr, "Error creating SDL renderer\n");
	}

	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

	return true;
}

void render_color_buffer(void) {
	// Populates the color_buffer_texture with the pixel information from color_buffer
	SDL_UpdateTexture(
		color_buffer_texture,
		NULL,
		color_buffer,
		(int)(window_width * sizeof(uint32_t))
	);
	// Copies a portion of the texture (in this case everything) to the render target
	SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void clear_color_buffer(uint32_t color) {
	for (int y = 0; y < window_height; y++) {
		for (int x = 0; x < window_width; x++) {
			color_buffer[(window_width * y) + x] = color;
		}
	}
}

void clear_z_buffer(void) {
	for (int y = 0; y < window_height; y++) {
		for (int x = 0; x < window_width; x++) {
			z_buffer[(window_width * y) + x] = 1.0;
		}
	}
}


void draw_pixel(int x, int y, uint32_t color) {
	if (x < window_width && x >= 0 && y < window_height && y >= 0) {
		color_buffer[(window_width * y) + x] = color;
	}
}

void draw_line_Bren(int x0, int y0, int x1, int y1, uint32_t color) {
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2;
	for (;;) {
		draw_pixel(x0,y0, color);
		if (x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if (e2 >= dy) { 
			err += dy; 
			x0 += sx; 
		}
		if (e2 <= dx) {
			err += dx; 
			y0 += sy;
		}
	}
}

void draw_line_DDA(int x0, int y0, int x1, int y1, uint32_t color) {
	int delta_x = (x1 - x0);
	int delta_y = (y1 - y0);

	int longest_side_length = (abs(delta_x) >= abs(delta_y)) ? abs(delta_x) : abs(delta_y);

	float x_inc = delta_x / (float)longest_side_length;
	float y_inc = delta_y / (float)longest_side_length;

	float currX = x0;
	float currY = y0;

	for (int i = 0; i <= longest_side_length; i++) {
		draw_pixel(round(currX), round(currY), color);
		currX += x_inc;
		currY += y_inc;
	}
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
	/*draw_line_DDA(x0, y0, x1, y1, color);
	draw_line_DDA(x1, y1, x2, y2, color);
	draw_line_DDA(x2, y2, x0, y0, color);*/
	draw_line_Bren(x0, y0, x1, y1, color);
	draw_line_Bren(x1, y1, x2, y2, color);
	draw_line_Bren(x2, y2, x0, y0, color);
	// attempt at Bresenhams algorithm
	/*if (y0 == y1 && y0 == y2) {
		return;
	}
	if (y0 > y1) {
		float tempx = x0, tempy = y0;
		x0 = x1;
		y0 = y1;
		x1 = tempx;
		y1 = tempy;
	}
	if (y0 > y2) {
		float tempx = x0, tempy = y0;
		x0 = x2;
		y0 = y2;
		x2 = tempx;
		y2 = tempy;
	}
	if (y1 > y2) {
		float tempx = x1, tempy = y1;
		x1 = x2;
		y1 = y2;
		x2 = tempx;
		y2 = tempy;
	}
	int totalHeight = y2 - y0;
	for (int i = 0; i < totalHeight; i++) {
		bool second_half = i > y1 - y0 || y1 == y0;
		int segment_height = second_half ? y2 - y1 : y1 - y0;
		float alpha = (float)i / totalHeight;
		float beta = (float)(i - (second_half ? y1 - y0 : 0)) / segment_height;
		vec2_t a = {
			.x = x0 + (x2-x0)*alpha ,
			.y = y0 + (y2-y0)*alpha
		};
		vec2_t b = {
			.x = second_half ? x1 + (x2 - x1)*beta : x0 + (x1-x0)*beta,
			.y = second_half ? y1 + (y2 - y1) * beta : y0 + (y1 - y0) * beta
		};
		if (a.x > b.x) {
			vec2_t temp = a;
			a = b;
			b = a;
		}
		for (int j = a.x; j < b.x; j++) {
			draw_pixel(j, y0 + i, color);
		}
	}*/
}

void draw_grid(void) {
	// Lines should be rendered at every row/col that is a multiple of 10
	for (int y = 0; y < window_height; y++) {
		for (int x = 0; x < window_width; x++) {
			if (x % 10 == 0 || y % 10 == 0) {
				color_buffer[(window_width * y) + x] = 0xFF333333;
			}
		}
	}
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
	for (int currY = y; currY < y + height; currY++) {
		for (int currX = x; currX < x + width; currX++) {
			draw_pixel(currX, currY, color);
		}
	}
}

void destroy_window(void) {
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}