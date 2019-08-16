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

#define FRONT 0
#define LEFT 1
#define RIGHT 2
#define REAR 3
#define PIVO_REAR 4
#define START 5
#define GOAL 6

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
