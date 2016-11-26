#ifndef __TOPLEVEL_H_
#define __TOPLEVEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <ap_int.h>
#include <hls_stream.h>


#define P_NULL 0b000
#define P_NORTH 0b001
#define P_SOUTH 0b010
#define P_EAST 0b011
#define P_WEST 0b100

//Typedefs
typedef ap_uint<8> u8;
typedef ap_uint<6> u6;
typedef ap_uint<1> u1;
typedef ap_uint<3> u3;
typedef ap_uint<4> u4;
typedef ap_int<13> i13;

typedef ap_uint<32> u32;
typedef ap_uint<12> u12;

typedef struct {
	u1 closed;
	u1 open;
	u1 wall;
	u3 previous; // null=000, north=001, south=010, east = 011, west = 100
} node_t;

//Prototypes
void toplevel(hls::stream<u32> &input, hls::stream<u32> &output);

#endif
