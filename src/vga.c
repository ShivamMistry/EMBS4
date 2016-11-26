#include "vga.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BITS_PER_PIXEL 8
#define FRAME_BUFFER XPAR_DDR_SDRAM_MPMC_BASEADDR

#define BLACK   0b00000000
#define WHITE   0b00000111
#define RED     0b00000100
#define GREEN   0b00000010
#define BLUE    0b00000001
#define CYAN    0b00000011
#define YELLOW  0b00000110
#define MAGENTA 0b00000101

#define MIN(x, y) (x <= y ? x : y)

void draw_rect(int xLoc, int yLoc, int width, int height, u8 colour) {
	int x, y;

	for (y = yLoc; y < yLoc + height; y++) {
		for (x = xLoc; x < xLoc + width; x++) {
			*((volatile u8 *) FRAME_BUFFER + x + (SCREEN_WIDTH * y)) = colour;
		}
	}
}

void fill_screen(void) {
	int x, y;

	for (y = 0; y < SCREEN_HEIGHT; y++) {
		for (x = 0; x < SCREEN_WIDTH; x++) {
			*((volatile u8 *) FRAME_BUFFER + x + (SCREEN_WIDTH * y)) = BLACK;
		}
	}
}

void draw_line(int x1, int y1, int x2, int y2, u8 colour) {
	int dx = x2 - x1;
	int dy = y2 - y1;
	u8 x, y;
	for (x = x1; x < x2; x++) {
		y = y1 + dy * (x - x1) / dx;
		*((volatile u8 *) FRAME_BUFFER + x + (SCREEN_WIDTH * y)) = colour;
	}
}

void draw_solution(int start_x, int start_y, int end_x, int end_y,
		int direction) {
	u8 width_per_square = SCREEN_WIDTH / reply_world.width;
	u8 height_per_square = SCREEN_HEIGHT / reply_world.height;
	if (start_x > end_x) {
		int t = start_x;
		start_x = end_x;
		end_x = t;
	}
	if (start_y > end_y) {
		int t = start_y;
		start_y = end_y;
		end_y = t;
	}
	// Take the min of width/height to make the squares actually squares
	u8 square_size = MIN(width_per_square, height_per_square);
	// Get the size of the whole grid
	u16 grid_width = square_size * reply_world.width;
	u16 grid_height = square_size * reply_world.height;
	u16 screen_x = (SCREEN_WIDTH - grid_width) / 2;
	u16 screen_y = (SCREEN_HEIGHT - grid_height) / 2;
	screen_y = screen_y + (start_y * square_size);
	u16 screen_y_end = screen_y + ((end_y - start_y) * square_size);
	screen_x = screen_x + (start_x * square_size);
	u16 screen_x_end = screen_x + ((end_x - start_x) * square_size);
	if (direction) {
		int draw_y;
		for (draw_y = screen_y + (square_size / 2); draw_y < screen_y_end + (square_size / 2); draw_y++) {
			*((volatile u8 *) FRAME_BUFFER + screen_x + (square_size / 2) + (SCREEN_WIDTH * draw_y)) = MAGENTA;
		}
	} else {
		int draw_x;
		for (draw_x = screen_x + (square_size / 2); draw_x < screen_x_end + (square_size / 2); draw_x++) {
			*((volatile u8 *) FRAME_BUFFER + draw_x
					+ (SCREEN_WIDTH * (screen_y + (square_size / 2)))) = MAGENTA;
		}
	}
}

void draw_grid(int sx, int sy, u8 s) {
	u16 original_sx = sx;
	u16 original_sy = sy;
	u8 x, y;
	for (x = 0; x < reply_world.width; x++) {
		sy = original_sy;
		for (y = 0; y < reply_world.height; y++) {
			u8 dy;
			for (dy = 0; dy < s; dy++) {
				*((volatile u8 *) FRAME_BUFFER + sx + (SCREEN_WIDTH * (sy + dy))) = BLACK;
			}
			sy += s;
		}
		sx += s;
	}
	sx = original_sx;
	for (x = 0; x < reply_world.width; x++) {
		sy = original_sy;
		for (y = 0; y < reply_world.height; y++) {
			u8 dx;
			for (dx = 0; dx < s; dx++) {
				*((volatile u8 *) FRAME_BUFFER + sx + dx + (SCREEN_WIDTH * sy)) = BLACK;
			}
			sy += s;
		}
		sx += s;
	}
}

void draw_world() {
	fill_screen();
	u16 wps = SCREEN_WIDTH / reply_world.width;
	u16 hps = SCREEN_HEIGHT / reply_world.height;

	// Take the min of width/height to make the squares actually squares
	u8 s = MIN(wps, hps);
	// Get the size of the whole grid
	u16 gw = s * reply_world.width;
	u16 gh = s * reply_world.height;
	u16 sx = (SCREEN_WIDTH - gw) / 2;
	u16 sy = (SCREEN_HEIGHT - gh) / 2;
	u8 x,y;
	for (x = 0; x < reply_world.width; x++) {
		sy = (SCREEN_HEIGHT - gh) / 2;
		for (y = 0; y < reply_world.height; y++) {
			// Select the colour
			// Walls red, waypoints blue
			u8 colour = WHITE;
			u8 wpti;
			for (wpti = 0; wpti < reply_world.num_waypoints; wpti++) {
				waypoint_t *wpt = &reply_world.waypoints[wpti];
				if (wpt->x == x && wpt->y == y) {
					colour = wpti == 0 ? GREEN : BLUE;
				}
			}
			for (wpti = 0; wpti < reply_world.num_walls; wpti++) {
				wall_t *w = &reply_world.walls[wpti];
				u8 wx, wy;
				if (w->direction == 1) {
					for (wy = w->y; wy < w->y + w->length; wy++) {
						if (y == wy && x == w->x) {
							colour = RED;
						}
					}
				} else if (w->direction == 0) {
					for (wx = w->x; wx < w->x + w->length; wx++) {
						if (x == wx && y == w->y) {
							colour = RED;
						}
					}
				}
			}
			draw_rect(sx, sy, s, s, colour);
			sy += s;
		}
		sx += s;
	}
	sx = (SCREEN_WIDTH - gw) / 2;
	sy = (SCREEN_HEIGHT - gh) / 2;
	draw_grid(sx, sy, s);

}
