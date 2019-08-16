#include "Map.h"
#include "MazeCon.h"
#include <stdint.h>

void Map_addWall(wallData_t *wall, pos_t *pos, unsigned char n_wall, unsigned char e_wall, unsigned char w_wall, unsigned char s_wall) {
	unsigned char x = pos->x;
	unsigned char y = pos->y;

	if (n_wall) {
		//Add north wall
		uint16_t data = 1;
		data <<= x;
		wall->horizontal[y+1] |= data;
		wall->horizontal_known[y+1] |= data;
	}
	else {
		//Remobe north wall
		uint16_t data = 1;
		data <<= x;
		wall->horizontal[y+1] &= ~data;
		wall->horizontal_known[y+1] |= data;
	}

	if (e_wall) {
		//Add east wall
		uint16_t data = 1;
		data <<= y;
		wall->vertical[x+1] |= data;
		wall->vertical_known[x+1] |= data;
	}
	else {
		//Remove east wall
		uint16_t data = 1;
		data <<= y;
		wall->vertical[x+1] &= ~data;
		wall->vertical_known[x+1] |= data;
	}

	if (s_wall) {
		//Add south wall
		uint16_t data = 1;
		data <<= x;
		wall->horizontal[y] |= data;
		wall->horizontal_known[y] |= data;
	}
	else {
		//Remobe south wall
		uint16_t data = 1;
		data <<= x;
		wall->horizontal[y] &= ~data;
		wall->horizontal_known[y] |= data;
	}

	if (w_wall) {
		//Add west wall
		uint16_t data = 1;
		data <<= y;
		wall->vertical[x] |= data;
		wall->vertical_known[x] |= data;
	}
	else {
		//Remobe weat wall
		uint16_t data = 1;
		data <<= y;
		wall->vertical[x] &= ~data;
		wall->vertical_known[x] |= data;
	}
}

void Map_Init(wallData_t *wall) {
	for (unsigned char i = 0; i <= MAZE_SIZE; i++)
	{
		wall->horizontal[i] = 0;
		wall->vertical[i] = 0;
		wall->horizontal_known[i] = 0;
		wall->vertical_known[i] = 0;
	}
	//外壁をセット
	wall->horizontal[0] = 0xffff;
	wall->vertical[0] = 0xffff;
	wall->horizontal[MAZE_SIZE] = 0xffff;
	wall->vertical[MAZE_SIZE] = 0xffff;
	//start横の壁をセットする
	wall->vertical[1] = 0x0001;
	wall->vertical_known[1] = 0x0001;
	//外壁を探査済みにする
	wall->horizontal_known[0] = 0xffff;
	wall->vertical_known[0] = 0xffff;
	wall->horizontal_known[MAZE_SIZE] = 0xffff;
	wall->vertical_known[MAZE_SIZE] = 0xffff;
}

void Map_InitFast(wallData_t *wall) {
	for (unsigned char i = 0; i <= MAZE_SIZE; i++)
	{
		wall->horizontal[i] = 0xffff;
		wall->vertical[i] = 0xffff;
		wall->horizontal_known[i] = 0;
		wall->vertical_known[i] = 0;
	}
	//外壁をセット
	wall->horizontal[0] = 0xffff;
	wall->vertical[0] = 0xffff;
	wall->horizontal[MAZE_SIZE] = 0xffff;
	wall->vertical[MAZE_SIZE] = 0xffff;
	//start横の壁をセットする
	wall->vertical_known[1] = 0x0001;
	//外壁を探査済みにする
	wall->horizontal_known[0] = 0xffff;
	wall->vertical_known[0] = 0xffff;
	wall->horizontal_known[MAZE_SIZE] = 0xffff;
	wall->vertical_known[MAZE_SIZE] = 0xffff;
}