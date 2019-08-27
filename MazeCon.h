#ifndef __MAZECONF_H
#define __MAZECONF_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

	//define
#define NORTH 0
#define WEST 1
#define SOUTH 2
#define EAST 3

#define NORTHWEST 0
#define SOUTHWEST 1
#define SOUTHEAST 2
#define NORTHEAST 3


//search
#define VELO_S 400.0f
#define ACCEL_S 5000.0f
#define VELO_ANG_S 400.0f
#define ACCEL_ANG_S 8000.0f
#define VELO_ANG_S_SLALOM 600.0f
#define ACCEL_ANG_S_SLALOM 10000.0f

//fast
#define ACCEL_F 5000.0f
#define VELO_F 800.0f
#define VELO_ANG_D 400.0f
#define ACCEL_ANG_D 8000.0f

#define FRONT 0
#define LEFT 1
#define RIGHT 2
#define REAR 3
#define PIVO_REAR 4
#define START 5
#define GOAL 6
#define DIAGONAL 7 //90*sqrt(2)
#define ADJUST 8 

//fast turn code
#define SEARCH 0
#define T_45IN 1
#define T_135IN 2
#define T_90 3
#define T_180 4
#define T_45OUT 5
#define T_135OUT 6
#define T_V90 7

#define MAZE_SIZE 16
#define MAX_STEP 255



typedef struct
{
	uint16_t vertical[MAZE_SIZE+1];
	uint16_t horizontal[MAZE_SIZE+1];
	uint16_t vertical_known[MAZE_SIZE + 1];
	uint16_t horizontal_known[MAZE_SIZE + 1];
}wallData_t;

typedef struct
{
	unsigned char x;
	unsigned char y;
	unsigned char dir;
}pos_t;


#ifdef __cplusplus
}
#endif
#endif /* __MAZE_CONF_H */
