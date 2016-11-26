#ifndef NETWORK_H_
#define NETWORK_H_
#include "xil_types.h"
#include "xuartlite_l.h"
#include "xemaclite.h"

#pragma pack(push, 1)

typedef struct {
	u8 x;
	u8 y;
} waypoint_t;

typedef struct {
	u8 x;
	u8 y;
	u8 direction;
	u8 length;
} wall_t;

typedef struct {
	u32 world_id;
	u8 width;
	u8 height;
	u8 num_waypoints;
	waypoint_t waypoints[12];
	u8 num_walls;
	wall_t walls[20];
} reply_world_t;

typedef struct {
	u8 answer;
} solution_reply_t;

#pragma pack(pop)

extern reply_world_t reply_world;
extern solution_reply_t solution_reply;

void initialise_network();

int receive_network();

u8 * reset_buffer();

void request_world(u8 size, u32 world_id);

void solve_world(u8 size, u32 world_id, u8 ignore_walls, u32 shortest_path);

#endif /* NETWORK_H_ */
