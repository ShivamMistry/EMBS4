#include "vga.h"
#include "network.h"
#include "fsl.h"

int read_int() {
	int val = 0;
	char c;
	do {
		c = XUartLite_RecvByte(XPAR_RS232_DTE_BASEADDR);
		if (c >= '0' && c <= '9') {
			val *= 10;
			val += (int) (c - '0');
		}
		XUartLite_SendByte(XPAR_RS232_DTE_BASEADDR, c);
	} while (c != '\r' && c != '\n');
	return val;
}

void debug_world(reply_world_t *world) {
	xil_printf("\tto_hw.write(%d);\r\n", world->width);
	xil_printf("\tto_hw.write(%d);\r\n", world->height);
	xil_printf("\tto_hw.write(%d);\r\n", world->num_waypoints);
	int i;
	for (i = 0; i < world->num_waypoints; i++) {
		xil_printf("\tto_hw.write(%d);\r\n", world->waypoints[i].x);
		xil_printf("\tto_hw.write(%d);\r\n", world->waypoints[i].y);
	}
	int x, y;
	for (x = 0; x < world->width; x++) {
		for (y = 0; y < world->height; y++) {
			int blocked = 0;
			for (i = 0; i < world->num_walls; i++) {
				wall_t w = world->walls[i];
				int wx, wy;
				if (w.direction == 1) {
					for (wy = w.y; wy < w.y + w.length; wy++) {
						if (y == wy && x == w.x) {
							blocked = 1;
						}
					}
				} else if (w.direction == 0) {
					for (wx = w.x; wx < w.x + w.length; wx++) {
						if (x == wx && y == w.y) {
							blocked = 1;
						}
					}
				}
			}
			xil_printf("\tto_hw.write(%d);\r\n", blocked);
		}
	}
}

void send_world() {
	putfslx(reply_world.width, 0, FSL_DEFAULT);
	putfslx(reply_world.height, 0, FSL_DEFAULT);
	putfslx(reply_world.num_waypoints, 0, FSL_DEFAULT);
	int i;
	for (i = 0; i < reply_world.num_waypoints; i++) {
		putfslx(reply_world.waypoints[i].x, 0, FSL_DEFAULT);
		putfslx(reply_world.waypoints[i].y, 0, FSL_DEFAULT);
	}
	int x, y;
	for (x = 0; x < reply_world.width; x++) {
		for (y = 0; y < reply_world.height; y++) {
			int blocked = 0;
			for (i = 0; i < reply_world.num_walls; i++) {
				wall_t *w = &reply_world.walls[i];
				int wx, wy;
				if (w->direction == 1) {
					for (wy = w->y; wy < w->y + w->length; wy++) {
						if (y == wy && x == w->x) {
							blocked = 1;
						}
					}
				} else if (w->direction == 0) {
					for (wx = w->x; wx < w->x + w->length; wx++) {
						if (x == wx && y == w->y) {
							blocked = 1;
						}
					}
				}
			}
			putfslx(blocked, 0, FSL_DEFAULT);

		}
	}
}

u16 receive_solution() {
	u16 cost = 0;
	u32 x = 255, y = 255, last_x = reply_world.waypoints[0].x, last_y =
			reply_world.waypoints[0].y;
	while (1) {
		u32 size;
		getfslx(size, 0, FSL_DEFAULT);
		if (size == 0)
			break;
		cost += size;
		u16 i;
		for (i = 0; i < size; i++) {

			getfslx(x, 0, FSL_DEFAULT);
			getfslx(y, 0, FSL_DEFAULT);
			//xil_printf("(%d, %d),", x, y);
			u8 direction = x == last_x;
			//xil_printf("Drawing %d %d %d %d %d\r\n", last_x, last_y, x, y, direction);

			draw_solution(last_x, last_y, x, y, direction);
			last_x = x;
			last_y = y;

		}
		xil_printf("\r\n");

	}

	xil_printf("Total cost: %d\r\n", cost);
	return cost;
}

int main(void) {
	*((volatile unsigned int *) XPAR_EMBS_VGA_0_BASEADDR + 1) = 1;
	*((volatile u32 *) XPAR_EMBS_VGA_0_BASEADDR) = XPAR_DDR_SDRAM_MPMC_BASEADDR;
	while (1) {
		int size;
		do {
			xil_printf("Select a world size(0=small,1=medium,2=large): ");
			size = read_int();
			XUartLite_SendByte(XPAR_RS232_DTE_BASEADDR, '\n');
		} while (size < 0 || size > 2);
		int world;
		do {
			xil_printf("Select a world: ");
			world = read_int();
			XUartLite_SendByte(XPAR_RS232_DTE_BASEADDR, '\n');
		} while (world < 0 || world > 100000);
		initialise_network();
		request_world((u8) size, (u32) world);
		do {
			int ptr = receive_network();
			if (ptr == 0) {
				continue;
			}
			if (ptr == 1) {
				break;
			}
			if (ptr == 2) {
				// wat shouldnt happen
				continue;
			}
		} while (1);

		// Print out the world for debug
		draw_world();
		send_world();
		u16 cost = receive_solution();
		solve_world(size, world, 0, cost);
		do {
			int ptr = receive_network();
			if (ptr == 0) {
				continue;
			}
			if (ptr == 1) {
				// wat shouldnt happen
				continue;
			}
			if (ptr == 2) {
				if (solution_reply.answer == 0) {
					xil_printf("Correct!\r\n");
				} else if (solution_reply.answer == 1) {
					xil_printf("Incorrect: too long!\r\n");
				} else if (solution_reply.answer == 2) {
					xil_printf("Incorrect: too short!\r\n");

				}
				break;
			}
		} while (1);
	}
	return 0;
}
