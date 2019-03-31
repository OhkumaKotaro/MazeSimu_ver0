#include "Maze.h"

unsigned char step[MAZE_SIZE][MAZE_SIZE];
unsigned char goal_size=1;

void Maze_UpdatePosition(unsigned char dir,pos_t *pos) {
	if (dir>3)
	{
		dir = 3;
	}
	// 次の動作で向き、座標を更新する	
	if (dir == FRONT) {
		switch (pos->dir) {
		case NORTH:
			pos->y++;
			break;
		case EAST:
			pos->x++;
			break;
		case SOUTH:
			pos->y--;
			break;
		case WEST:
			pos->x--;
			break;
		default:
			break;
		}
	}
	else if (dir == LEFT) {
		switch (pos->dir) {
		case NORTH:
			pos->x--;
			pos->dir = WEST;
			break;
		case EAST:
			pos->y++;
			pos->dir = NORTH;
			break;
		case SOUTH:
			pos->x++;
			pos->dir = EAST;
			break;
		case WEST:
			pos->y--;
			pos->dir = SOUTH;
			break;
		default:
			break;
		}
	}
	else if (dir == RIGHT) {
		switch (pos->dir) {
		case NORTH:
			pos->x++;
			pos->dir = EAST;
			break;
		case EAST:
			pos->y--;
			pos->dir = SOUTH;
			break;
		case SOUTH:
			pos->x--;
			pos->dir = WEST;
			break;
		case WEST:
			pos->y++;
			pos->dir = NORTH;
			break;
		default:
			break;
		}
	}
	else if (dir == REAR) {
		switch (pos->dir) {
		case NORTH:
			pos->y--;
			pos->dir = SOUTH;
			break;
		case EAST:
			pos->x--;
			pos->dir = WEST;
			break;
		case SOUTH:
			pos->y++;
			pos->dir = NORTH;
			break;
		case WEST:
			pos->x++;
			pos->dir = EAST;
			break;
		default:
			break;
		}
	}
}

unsigned char Maze_GetWallData(unsigned char x, unsigned char y, unsigned char dir, wallData_t *wall) {
	uint16_t check_wall = 1;

	if (dir > 3) {
		dir = dir - 4;
	}

	if (dir == NORTH) {
		check_wall <<= x;
		check_wall &= wall->horizontal[y+1];
		if (check_wall != 0) {
			check_wall = 1;
		}
	}
	else if (dir == EAST) {
		check_wall <<= y;
		check_wall &= wall->vertical[x+1];
		if (check_wall != 0) {
			check_wall = 1;
		}
	}
	else if (dir == SOUTH) {
		check_wall <<= x;
		check_wall &= wall->horizontal[y];
		if (check_wall != 0) {
			check_wall = 1;
		}
	}
	else if (dir == WEST) {
		check_wall <<= y;
		check_wall &= wall->vertical[x];
		if (check_wall != 0) {
			check_wall = 1;
		}
	}

	if (check_wall == 1) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

void Search_UnknownWall(wallData_t *wall,unsigned char *tail,unsigned char pos[MAX_STEP]) {
	unsigned char buff_tail = *tail;
	//init step
	for (unsigned char i = 0; i < MAZE_SIZE; i++) {
		for (unsigned char j = 0; j < MAZE_SIZE; j++) {
			unsigned char tmp_step = 255;
			//north
			if ((wall->horizontal_known[j+1] & (0b1 << i)) == 0) {
				tmp_step = 0;
			}
			//south
			if (j > 0)
			{
				if ((wall->horizontal_known[j] & (0b1 << i)) == 0) {
					tmp_step = 0;
				}
			}
			//east
			if ((wall->vertical_known[i+1] & (0b1 << j)) == 0)
			{
				tmp_step = 0;
			}
			//west
			if (i > 0)
			{
				if ((wall->vertical_known[i] & (0b1 << j)) == 0) {
					tmp_step = 0;
				}

			}
			if (tmp_step == 0) {
				*(pos+ buff_tail) = (i<<4)|j;
				buff_tail++;
			}
			step[i][j] = tmp_step;
		}
	}
	*tail = buff_tail;
}

void Maze_UpdateStepMap(unsigned char *goal_flag,unsigned char gx, unsigned char gy, wallData_t *wall) {
	unsigned char head=0;
	unsigned char tail=0;
	unsigned char pos[MAX_STEP];
	
	//unsigned char step[MAZE_SIZE][MAZE_SIZE];

	for (unsigned char x = 0; x < MAZE_SIZE; x++)
	{
		for (unsigned char y = 0; y < MAZE_SIZE; y++)
		{
			step[x][y] = MAX_STEP;
		}
	}
	
	if (*goal_flag==1)
	{
		Search_UnknownWall(wall,&tail,pos);
	}
	else
	{
		if (goal_size == 4) {
			step[gx + 1][gy] = 0;
			step[gx + 1][gy + 1] = 0;
			step[gx][gy + 1] = 0;
			step[gx][gy] = 0;

			pos[tail] = gx << 4 | gy;
			tail++;
			pos[tail] = (gx + 1) << 4 | gy;
			tail++;
			pos[tail] = (gx + 1) << 4 | (gy + 1);
			tail++;
			pos[tail] = (gx + 1) << 4 | (gy + 1);
			tail++;
		}
		else {
			step[gx][gy] = 0;
			pos[tail] = (gx << 4) | gy;
			tail++;
		}
	}


	while (head!=tail) {
		unsigned int tmp;
		unsigned char y = pos[head]& 0b00001111;
		unsigned char x = (pos[head] & 0b11110000) >> 4;
		head++;
		//north
		if (y < MAZE_SIZE-1 ) {
			tmp = (wall->horizontal[y + 1] & (0b1 << x));
			if (tmp==0 && (step[x][y + 1] == MAX_STEP)) {
				step[x][y + 1] = step[x][y] + 1;
				pos[tail] = (x << 4) | (y + 1);
				tail++;
			}
		}
		//east
		if (x < MAZE_SIZE-1) {
			tmp = (wall->vertical[x + 1] & (0b1 << y));
			if (tmp == 0 && (step[x + 1][y] == MAX_STEP)) {
				step[x + 1][y] = step[x][y] + 1;
				pos[tail] = ((x + 1) << 4) | y;
				tail++;
			}
		}

		//south
		if (y > 0) {
			tmp = (wall->horizontal[y] & (0b1 << x));
			if (tmp == 0 && (step[x][y - 1] == MAX_STEP)) {
				step[x][y - 1] = step[x][y] + 1;
				pos[tail] = ((x << 4) | (y-1));
				tail++;
			}
		}

		//west
		if (x > 0) {
			tmp = (wall->vertical[x] & (0b1 << y));
			if (tmp == 0 && (step[x - 1][y] == MAX_STEP)) {
				step[x - 1][y] = step[x][y] + 1;
				pos[tail] = (((x - 1) << 4) | y);
				tail++;
			}
		}
	}
	if (step[0][0]==MAX_STEP)
	{
		*goal_flag = 2;
	}
}

unsigned char Maze_GetNextMotion(pos_t *mypos, wallData_t *wall) {
	unsigned char tmp_step = MAX_STEP; // 歩数
	unsigned char tmp_dir = REAR;	 // 方向
									 // 現在の向きに応じて場合分けし、 歩数が少ない方向を判断
									 // 迷路外に進むのとゴールがスタートマス以外の場合(0,0)に進むのを阻止

	unsigned char x = mypos->x;
	unsigned char y = mypos->y;

	switch (mypos->dir)
	{
	case NORTH:
		if (step[x][y + 1] < tmp_step)
		{
			if (y < MAZE_SIZE - 1)
			{
				if (((wall->horizontal[y+1] >> x) & 0b1) == FALSE)
				{
					tmp_step = step[x][y + 1];
					tmp_dir = FRONT;
				}
			}
		}
		if (step[x - 1][y] < tmp_step)
		{
			if (x > 0)
			{
				if (((wall->vertical[x] >> y) & 0b1) == FALSE)
				{
					tmp_step = step[x - 1][y];
					tmp_dir = LEFT;
				}
			}
		}
		if (step[x + 1][y] < tmp_step)
		{
			if (x < MAZE_SIZE - 1)
			{
				if (((wall->vertical[x+1] >> y) & 0b1) == FALSE)
				{
					tmp_step = step[x + 1][y];
					tmp_dir = RIGHT;
				}
			}
		}
		if (tmp_step == MAX_STEP || step[x][y - 1] < tmp_step)
		{
			if (((wall->horizontal[y+1] >> x) & 0b1) == FALSE)
			{
				tmp_dir = REAR;
			}
			else
			{
				tmp_dir = PIVO_REAR;
			}
		}
		break;
	case EAST:
		if (step[x + 1][y] < tmp_step)
		{
			if (x < MAZE_SIZE - 1)
			{
				if (((wall->vertical[x+1] >> y) & 0b1) == FALSE)
				{
					tmp_step = step[x + 1][y];
					tmp_dir = FRONT;
				}
			}
		}
		if (step[x][y + 1] < tmp_step)
		{
			if (y < MAZE_SIZE - 1)
			{
				if (((wall->horizontal[y+1] >> x) & 0b1) == FALSE)
				{
					tmp_step = step[x][y + 1];
					tmp_dir = LEFT;
				}
			}
		}
		if (step[x][y - 1] < tmp_step)
		{
			if (y > 0)
			{
				if (((wall->horizontal[y] >> x) & 0b1) == FALSE)
				{
					tmp_step = step[x][y - 1];
					tmp_dir = RIGHT;
				}
			}
		}
		if (tmp_step == MAX_STEP || step[x - 1][y] < tmp_step)
		{
			if (((wall->vertical[x+1] >> y) & 0b1) == FALSE)
			{
				tmp_dir = REAR;
			}
			else
			{
				tmp_dir = PIVO_REAR;
			}
		}
		break;
	case SOUTH:
		if (step[x][y - 1] < tmp_step)
		{
			if (y > 0)
			{
				if (((wall->horizontal[y] >> x) & 0b1) == FALSE)
				{
					tmp_step = step[x][y - 1];
					tmp_dir = FRONT;
				}
			}
		}
		if (step[x + 1][y] < tmp_step)
		{
			if (x < MAZE_SIZE - 1)
			{
				if (((wall->vertical[x+1] >> y) & 0b1) == FALSE)
				{
					tmp_step = step[x + 1][y];
					tmp_dir = LEFT;
				}
			}
		}
		if (step[x - 1][y] < tmp_step)
		{
			if (x > 0)
			{
				if (((wall->vertical[x] >> y) & 0b1) == FALSE)
				{
					tmp_step = step[x-1][y];
					tmp_dir = RIGHT;
				}
			}
		}
		if (tmp_step == MAX_STEP || step[x][y + 1] < tmp_step)
		{
			if (((wall->horizontal[y] >> x) & 0b1) == FALSE)
			{
				tmp_dir = REAR;
			}
			else
			{
				tmp_dir = PIVO_REAR;
			}
		}
		break;
	case WEST:
		if (step[x - 1][y] < tmp_step)
		{
			if (x > 0)
			{
				if (((wall->vertical[x] >> y) & 0b1) == FALSE)
				{
					tmp_step = step[x - 1][y];
					tmp_dir = FRONT;
				}
			}
		}
		if (step[x][y - 1] < tmp_step)
		{
			if (y > 0)
			{
				if (((wall->horizontal[y] >> x) & 0b1) == FALSE)
				{
					tmp_step = step[x][y - 1];
					tmp_dir = LEFT;
				}
			}
		}
		if (step[x][y + 1] < tmp_step)
		{
			if (y < MAZE_SIZE - 1)
			{
				if (((wall->horizontal[y+1] >> x) & 0b1) == FALSE)
				{
					tmp_step = step[x][y + 1];
					tmp_dir = RIGHT;
				}
			}
		}
		if (tmp_step == MAX_STEP || step[x + 1][y] < tmp_step)
		{
			if (((wall->vertical[x] >> y) & 0b1) == FALSE)
			{
				tmp_dir = REAR;
			}
			else
			{
				tmp_dir = PIVO_REAR;
			}
		}
		break;
	default:
		break;
	}
	return tmp_dir;
}

unsigned char Maze_GetStep(unsigned char x, unsigned char y) {
	return step[x][y];
}