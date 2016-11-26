#include "network.h"

XEmacLite ether;
static u8 mac_address[] = { 0x00, 0x11, 0x22, 0x33, 0x00, 0x1F };
static u8 server_mac[] = { 0x00, 0x11, 0x22, 0x44, 0x00, 0x50 };

reply_world_t reply_world;
solution_reply_t solution_reply;

u8 buff[XEL_MAX_FRAME_SIZE];

void initialise_network() {
	XEmacLite_Config *etherconfig = XEmacLite_LookupConfig(
			XPAR_EMACLITE_0_DEVICE_ID);
	XEmacLite_CfgInitialize(&ether, etherconfig, etherconfig->BaseAddress);
	XEmacLite_SetMacAddress(&ether, mac_address);
}


u8 * read_u32(u8 *pos, u32* value) {
	u32 val = *pos++ << 24;
	val |= *pos++ << 16;
	val |= *pos++ << 8;
	*value = val | (*pos++);
	return pos;
}

int receive_network() {
	int recv = XEmacLite_Recv(&ether, buff);
	u8 *pos = buff;
	int i;
	// Read the dst mac addr, check if it is for us
	for (i = 0; i < 6; i++) {
		if (mac_address[i] != *pos++) {
			return 0;
		}
	}
	// Make sure the msg came from the server
	for (i = 0; i < 6; i++) {
		if (server_mac[i] != *pos++) {
			return 0;
		}
	}
	// Check the type
	int typeA = *pos++;
	int typeB = *pos++;
	if ((typeA << 8 | typeB) != 0x55AB) {
		return 0;
	}
	recv -= 14;
	int opcode = *pos++;
	// Unknown opcode
	if (opcode != 4 && opcode != 2) {

		return 0;
	}
	if (opcode == 4) {
		// Answer

		solution_reply.answer = *pos++;
		return 2;
	}
	else {
		// assert opcode == 2;
		// Reply
		u32 world_id;
		pos = read_u32(pos, &world_id);
		reply_world.world_id = world_id;
		reply_world.width = *pos++;
		reply_world.height = *pos++;
		reply_world.num_waypoints = *pos++;
		for (i = 0; i < reply_world.num_waypoints; i++) {
			waypoint_t *waypoint = &reply_world.waypoints[i];
			waypoint->x = *pos++;
			waypoint->y = *pos++;
		}
		reply_world.num_walls = *pos++;
		for (i = 0; i < reply_world.num_walls; i++) {
			wall_t *wall = &reply_world.walls[i];
			wall->x = *pos++;
			wall->y = *pos++;
			wall->direction = *pos++;
			wall->length = *pos++;
		}
		return 1;
	}
}

u8 * reset_buffer() {
	u8 *pos = buff;
	int i;
	// Destination MAC
	for (i = 0; i < 6; i++) {
		*(pos++) = server_mac[i];
	}
	// Our MAC
	for (i = 0; i < 6; i++) {
		*(pos++) = mac_address[i];
	}
	// Type
	*(pos++) = 0x55;
	*(pos++) = 0xAB;
	// This header is 14 bytes
	// Should probably clear the rest but that would be slow!
	return pos;
}

u8 * write_u32_be(u8 *pos, u32 val) {
	// Write each byte from the value, MSB first
	*(pos++) = (val >> 24) & 0xff;
	*(pos++) = (val >> 16) & 0xff;
	*(pos++) = (val >> 8) & 0xff;
	*(pos++) = val & 0xff;
	return pos;
}

void request_world(u8 size, u32 world_id) {
	u8 *pos = reset_buffer();
	*(pos++) = 1; // Write opcode
	*(pos++) = size; // world size
	pos = write_u32_be(pos, world_id);
	// This is 6 bytes
	// 6 + 14 = 20
	XEmacLite_Send(&ether, buff, 20);
}

void solve_world(u8 size, u32 world_id, u8 ignore_walls, u32 shortest_path) {
	u8 *pos = reset_buffer();
	*(pos++) = 3; // Write opcode
	*(pos++) = size; // World size
	pos = write_u32_be(pos, world_id); // World ID
	*(pos++) = ignore_walls;
	pos = write_u32_be(pos, shortest_path);
	// This is 25 bytes
	XEmacLite_Send(&ether, buff, 25);
}
