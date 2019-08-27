#ifndef __MAZE_H
#define __MAZE_H
#ifdef __cplusplus
extern "C"
{
#endif
#include"MazeCon.h"
#include <stdint.h>

#define FALSE 0
#define TRUE 1

	void Maze_UpdatePosition(uint16_t dir, pos_t *pos);
	unsigned char Maze_GetWallData(unsigned char x, unsigned char y, unsigned char dir, wallData_t *wall);
	void Maze_UpdateStepMap(unsigned char *goal_flag, unsigned char gx, unsigned char gy, wallData_t *wall);
	unsigned char Maze_GetNextMotion(pos_t *mypos, wallData_t *wall);
	unsigned char Maze_GetStep(unsigned char x, unsigned char y);
	void Maze_UpdateStepMapEx(wallData_t *wallDate, uint16_t weight_s, uint16_t weight_t, uint16_t gx, uint16_t gy);
	uint16_t Maze_GetNextMotionEx(pos_t *mypos, wallData_t *wall);
	uint16_t Maze_GetStepEx_h(uint8_t x, uint8_t y);
	uint16_t Maze_GetStepEx_v(uint8_t x, uint8_t y);
	void Maze_Compress(uint8_t mode_FastTurn, uint16_t *motion, uint32_t *velocity, uint8_t *origin_tail);
#ifdef __cplusplus
}
#endif
#endif