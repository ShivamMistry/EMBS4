#include "toplevel.h"

#define X(index) (index >> 6)
#define Y(index) (index & 0x3F)
#define PACK(x,y) (((u12)x << 6) | y)

u6 width, height;
u12 pts[11];
u4 num_waypoints;
u12 src_node;
u12 curr_src;
u12 curr_dst;
node_t world[3601];
u4 pts_order[11];
u12 best_cost;
// Distance matrix
u12 dists[12][12];
u4 a[10], p[11];
u6 max_x, max_y, min_x, min_y;

u12 absolute(i13 val) {
	if (val < 0) {
		return (u12) -val;
	}
	return (u12) val;
}

u12 estimate(u12 start, u12 end) {
	return absolute((i13) ((i13) X(start) - (i13) X(end)))
			+ absolute((i13) ((i13) Y(start) - (i13) Y(end)));
}

u12 prev(u12 node) {
	node_t *n = &world[node];
	if (n->previous == P_NULL)
		return 4095;
	u6 x = X(node);
	u6 y = Y(node);
	if (n->previous == P_NORTH) {
		++y;
	} else if (n->previous == P_SOUTH) {
		--y;
	} else if (n->previous == P_WEST) {
		--x;
	} else if (n->previous == P_EAST) {
		++x;
	}
	return PACK(x, y);
}

u12 g_cost(u12 node) {
	if (node == curr_src)
		return 0;
	node_t *n = &world[node];
	if (n->previous == P_NULL)
		return 4095;
	u12 index = node;
	u12 cost = 0;
	while (n->previous != P_NULL) {
		index = prev(index);
		n = &world[index];
		++cost;
	}
	return cost;
}

u12 get_cost(u12 node, u12 dst) {
	return g_cost(node) + estimate(node, dst);
}
u12 smallest;
u12 smallest_cost;
u12 smallest_open_node(u12 dst) {
	smallest = 4095;
	smallest_cost = 4095;
	for (u6 x = min_x; x <= max_x; x++) {
		for (u6 y = min_y; y <= max_y; y++) {
			u12 i = PACK(x, y);
			if ((&world[i])->open) {
				u12 current_cost = get_cost(i, dst);
				if (current_cost <= smallest_cost) {
					smallest = i;
					smallest_cost = current_cost;
				}
			}

		}
	}
	return smallest;
}

void relax(u12 current, u12 node, u3 direction) {
	node_t *n = &world[node];
	if (n->wall || n->closed)
		return;
	u6 x = X(node), y = Y(node);
	if (x > max_x) max_x = x;
	if (y > max_y) max_y = y;
	if (x < min_x) min_x = x;
	if (y < min_y) min_y = y;
	u12 tentative_g = g_cost(current) + 1;
	u12 g = g_cost(node);
	if (tentative_g < g) {
		n->open = 1;
		n->previous = direction;
	}
}


u12 a_star() {
	u6 x,y;
	node_t *node;

	for (u6 x = 0; x < width; x++) {
		for (u6 y = 0; y < height; y++) {
			node = &world[PACK(x, y)];
			node->closed = 0;
			node->open = 0;
			node->previous = P_NULL;
		}

	}
	node_t *source = &world[curr_src];
	source->open = 1;
	source->previous = P_NULL;
	min_x = max_x = X(curr_src);
	min_y = max_y = Y(curr_src);
	u12 current;
	while (1) {
		current = smallest_open_node(curr_dst);
		if (current == 4095) break;
		if (current == curr_dst) {
			return g_cost(curr_dst);
		}
		node = &world[current];
		node->open = 0;
		node->closed = 1;
		// Look at the neighbours
		x = X(current);
		y = Y(current);
		// Look north
		if (y + 1 < height) {
			relax(current, PACK(x, y + 1), P_SOUTH);
		}
		// Look south
		if (y - 1 >= 0) {
			relax(current, PACK(x, y - 1), P_NORTH);
		}
		// Look east
		if (x + 1 < width) {
			relax(current, PACK(x+1, y), P_WEST);
		}
		// Look west
		if (x - 1 >= 0) {
			relax(current, PACK(x-1, y), P_EAST);
		}
	}
	return 4095;
}

void find_distances() {
	// Work out distances between each wpt
	for (u4 x = 0; x < num_waypoints; x++) {
		for (u4 y = x; y < num_waypoints; y++) {
			curr_src = x == 0 ? src_node : pts[x - 1];
			curr_dst = y == 0 ? src_node : pts[y - 1];
			if (curr_src == curr_dst) {
				// Distance is 0 to itself
				dists[x][y] = 0;
				dists[y][x] = 0;
			} else {

				u12 cost = a_star();
				if (cost == 4095)
					cost = 0;
				dists[x][y] = cost;
				dists[y][x] = cost;
			}
		}
	}
}

void try_perm(u4 *a) {
	u4 i, j;
	u4 dst;
	for (j = 0; j < ((num_waypoints - 2) >> 1) + 1; j++) {
		u12 cost = 0;
		u4 src = 0;
		for (i = 0; i < j; i++) {
			dst = a[i];
			u12 c = dists[src][dst];
			// Could not solve this one
			if (c == 4095)
				return;
			cost += c;
			src = dst;
		}
		dst = 1;
		u12 c = dists[src][dst];
		// Could not solve this one
		if (c == 4095)
			return;
		cost += c;
		src = dst;
		for (i = j + 1; i < num_waypoints - 1; i++) {
			dst = a[i - 1];
			u12 c = dists[src][dst];
			// Could not solve this one
			if (c == 4095)
				return;
			cost += c;
			src = dst;
		}
		cost += dists[dst][0];
		if (cost < best_cost) {
			best_cost = cost;
			for (i = 0; i < j; i++) {
				pts_order[i] = a[i];
			}
			pts_order[j] = 1;
			for (i = j + 1; i < num_waypoints - 1; i++) {
				pts_order[i] = a[i-1];
			}
		}
	}

}

void find_best_pts() {
	best_cost = 4095;
	// From: http://www.quickperm.org/01example.php
	u4 N = num_waypoints - 2;
	u4 i, j, tmp; // Upper Index i; Lower Index j
	for (i = 0; i < N; i++) {
		a[i] = i + 2;   // a[i] value is not revealed and can be arbitrary
		p[i] = i;
	}
	p[N] = N; // p[N] > 0 controls iteration and the index boundary for i
	try_perm(a);
	i = 1;	// setup first swap points to be 1 and 0 respectively (i & j)
	while (i < N) {
		p[i]--;             // decrease index "weight" for i by one
		j = (i & 0b1) * p[i];       // IF i is odd then j = p[i] otherwise j = 0
		tmp = a[j];             // swap(a[j], a[i])
		a[j] = a[i];
		a[i] = tmp;
		try_perm(a);
		i = 1;             // reset index i to 1 (assumed)
		while (!p[i])             // while (p[i] == 0)
		{
			p[i] = i;        // reset p[i] zero value
			i++;        // set new index value for i (increase by one)
		} // while(!p[i])
	} // while(i < N)
}

void reconstruct_path(hls::stream<u32> &output) {
	// Finds the path between the current pts array
	u12 parent;
	u12 cost = 0;
	curr_dst = src_node;
	for (u4 i = 0; i < num_waypoints - 1; i++) {
		curr_src = pts[pts_order[i] - 1];
		u12 c = a_star();
		if (c == 4095) {
			output.write(0);
			return;
		}
		output.write(c);
		parent = prev(curr_dst);
		while (parent != 4095) {
			output.write(X(parent));
			output.write(Y(parent));
			parent = prev(parent);
		}
		cost += c;
		curr_dst = curr_src;
	}
	curr_src = src_node;
	u12 this_cost = a_star();
	if (this_cost == 4095) {
		output.write(0);
		return;
	}
	output.write(this_cost);
	parent = prev(curr_dst);
	while (parent != 4095) {
		output.write(X(parent));
		output.write(Y(parent));
		parent = prev(parent);
	}
	output.write(0);
}

//Top-level function
void toplevel(hls::stream<u32> &input, hls::stream<u32> &output) {
#pragma HLS RESOURCE variable=input core=AXI4Stream
#pragma HLS RESOURCE variable=output core=AXI4Stream
#pragma HLS INTERFACE ap_ctrl_none port=return

	width = input.read();
	height = input.read();
	num_waypoints = input.read();
	u6 i, x, y;
	for (i = 0; i < num_waypoints; i++) {
		x = (u6) input.read();
		y = (u6) input.read();
		u12 index = PACK(x, y);
		if (i == 0) {
			src_node = index;
		} else {
			pts[i - 1] = index;
		}

	}
	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			u8 wall = input.read();
			node_t *node = &world[PACK(x, y)];
			node->wall = wall;
			node->closed = 0;
			node->open = 0;
			node->previous = P_NULL;
		}
	}
	find_distances();
	find_best_pts();
	reconstruct_path(output);
}
