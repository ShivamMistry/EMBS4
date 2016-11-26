#ifndef VGA_H_
#define VGA_H_
#include "xparameters.h"
#include "xil_types.h"
#include "xuartlite_l.h"
#include "network.h"

void draw_world();
void draw_solution(int x, int y, int, int, int direction);

#endif /* VGA_H_ */
