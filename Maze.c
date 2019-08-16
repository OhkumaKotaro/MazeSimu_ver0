
#include "Maze.h"
#include "stdint.h"
#include "MazeCon.h"
//dx library
#include "stdio.h"

#define MAX_STEP_EX 0xffff

unsigned char step[MAZE_SIZE][MAZE_SIZE];
volatile uint16_t stepEx_h[MAZE_SIZE+1][MAZE_SIZE+1];
volatile uint16_t stepEx_v[MAZE_SIZE+1][MAZE_SIZE+1];
unsigned char goal_size = 1;

void Maze_UpdatePosition(unsigned char dir, pos_t *pos) {
	if (dir > 3)
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
		check_wall &= wall->horizontal[y + 1];
		if (check_wall != 0) {
			check_wall = 1;
		}
	}
	else if (dir == EAST) {
		check_wall <<= y;
		check_wall &= wall->vertical[x + 1];
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

void Search_UnknownWall(wallData_t *wall, unsigned char *tail, unsigned char pos[MAX_STEP]) {
	unsigned char buff_tail = *tail;
	//init step
	for (unsigned char i = 0; i < MAZE_SIZE; i++) {
		for (unsigned char j = 0; j < MAZE_SIZE; j++) {
			unsigned char tmp_step = 255;
			//north
			if ((wall->horizontal_known[j + 1] & (0b1 << i)) == 0) {
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
			if ((wall->vertical_known[i + 1] & (0b1 << j)) == 0)
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
				*(pos + buff_tail) = (i << 4) | j;
				buff_tail++;
			}
			step[i][j] = tmp_step;
		}
	}
	*tail = buff_tail;
}

void Maze_UpdateStepMap(unsigned char *goal_flag, unsigned char gx, unsigned char gy, wallData_t *wall)
{
	unsigned char head = 0;
	unsigned char tail = 0;
	unsigned char pos[MAX_STEP];

	//unsigned char step[MAZE_SIZE][MAZE_SIZE];

	for (unsigned char x = 0; x < MAZE_SIZE; x++)
	{
		for (unsigned char y = 0; y < MAZE_SIZE; y++)
		{
			step[x][y] = MAX_STEP;
		}
	}

	if (*goal_flag == 1)
	{
		Search_UnknownWall(wall, &tail, pos);
	}
	else
	{
		if (goal_size == 4)
		{
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
		else
		{
			step[gx][gy] = 0;
			pos[tail] = (gx << 4) | gy;
			tail++;
		}
	}

	while (head < tail)
	{
		unsigned int tmp;
		unsigned char y = pos[head] & 0b00001111;
		unsigned char x = (pos[head] & 0b11110000) >> 4;
		head++;
		//north
		if (y < MAZE_SIZE - 1)
		{
			tmp = (wall->horizontal[y + 1] & (0b1 << x));
			if (tmp == 0 && (step[x][y + 1] == MAX_STEP))
			{
				step[x][y + 1] = step[x][y] + 1;
				pos[tail] = (x << 4) | (y + 1);
				tail++;
			}
		}
		//east
		if (x < MAZE_SIZE - 1)
		{
			tmp = (wall->vertical[x + 1] & (0b1 << y));
			if (tmp == 0 && (step[x + 1][y] == MAX_STEP))
			{
				step[x + 1][y] = step[x][y] + 1;
				pos[tail] = ((x + 1) << 4) | y;
				tail++;
			}
		}

		//south
		if (y > 0)
		{
			tmp = (wall->horizontal[y] & (0b1 << x));
			if (tmp == 0 && (step[x][y - 1] == MAX_STEP))
			{
				step[x][y - 1] = step[x][y] + 1;
				pos[tail] = ((x << 4) | (y - 1));
				tail++;
			}
		}

		//west
		if (x > 0)
		{
			tmp = (wall->vertical[x] & (0b1 << y));
			if (tmp == 0 && (step[x - 1][y] == MAX_STEP))
			{
				step[x - 1][y] = step[x][y] + 1;
				pos[tail] = (((x - 1) << 4) | y);
				tail++;
			}
		}
	}
	if (step[0][0] == MAX_STEP)
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
				if (((wall->horizontal[y + 1] >> x) & 0b1) == FALSE)
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
				if (((wall->vertical[x + 1] >> y) & 0b1) == FALSE)
				{
					tmp_step = step[x + 1][y];
					tmp_dir = RIGHT;
				}
			}
		}
		if (tmp_step == MAX_STEP || step[x][y - 1] < tmp_step)
		{
			if (((wall->horizontal[y + 1] >> x) & 0b1) == FALSE)
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
				if (((wall->vertical[x + 1] >> y) & 0b1) == FALSE)
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
				if (((wall->horizontal[y + 1] >> x) & 0b1) == FALSE)
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
			if (((wall->vertical[x + 1] >> y) & 0b1) == FALSE)
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
				if (((wall->vertical[x + 1] >> y) & 0b1) == FALSE)
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
					tmp_step = step[x - 1][y];
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
				if (((wall->horizontal[y + 1] >> x) & 0b1) == FALSE)
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


void Plan_Root(unsigned char motion[MAX_STEP],wallData_t wall,unsigned char *tail)
{
	unsigned char flag_goal_is = FALSE;
	pos_t mypos;
	unsigned char buff_tail=0;

	mypos.x = 0;
	mypos.y = 1;
	mypos.dir = NORTH;
	*(motion+buff_tail) = START;
	buff_tail++;

	while (flag_goal_is == FALSE)
	{
		unsigned char tmp;
		tmp = Maze_GetNextMotion(&mypos, &wall);
		Maze_UpdatePosition (tmp,&mypos);
		*(motion+buff_tail) = tmp;
		buff_tail++;
		if (step[mypos.x][mypos.y]==0)
		{
			flag_goal_is = TRUE;
			*(motion+buff_tail) = GOAL;
			buff_tail++;
		}
	}

	*tail = buff_tail;
}

void Plan_Compress(unsigned char a_qmotion[MAX_STEP],unsigned char *head,unsigned char *tail)
{
	unsigned char a_head = 0;
	unsigned char a_tail = *tail;
	unsigned char b_tail = 0;
	unsigned char motion;
	unsigned char b_qmotion[MAX_STEP];


	motion = *a_qmotion;
	a_head++;

	while (a_head != a_tail)
	{
		unsigned char buff = 0;
		switch (motion)
		{
		case START: //5
			motion = *(a_qmotion + a_head);
			a_head++;
			while (motion == FRONT && a_head != a_tail)
			{
				buff++;
				motion = *(a_qmotion + a_head);
				a_head++;
			}
			buff = START << 4 | buff;
			b_qmotion[b_tail] = buff;
			b_tail++;
			break;
		case FRONT:
			while (motion == FRONT && a_head != a_tail)
			{
				buff++;
				motion = *(a_qmotion + a_head);
				a_head++;
			}
			if (motion == GOAL)
			{
				buff = GOAL << 4 | buff;
				b_qmotion[b_tail] = buff;
				b_tail++;
			}
			else
			{
				buff = FRONT << 4 | buff;
				b_qmotion[b_tail] = buff;
				b_tail++;
			}
			break;
		case LEFT:
			buff = LEFT << 4;
			b_qmotion[b_tail] = buff;
			b_tail++;
			motion = *(a_qmotion + a_head);
			a_head++;
			if (motion == GOAL) {
				buff = GOAL << 4;
				b_qmotion[b_tail] = buff;
				b_tail++;
			}
			break;
		case RIGHT:
			buff = RIGHT << 4;
			b_qmotion[b_tail] = buff;
			b_tail++;
			motion = *(a_qmotion + a_head);
			a_head++;
			if (motion == GOAL) {
				buff = GOAL << 4;
				b_qmotion[b_tail] = buff;
				b_tail++;
			}
			break;
		default:
			break;
		}
	}
	a_qmotion = b_qmotion;
}

void Maze_UpdateStepMapEx(wallData_t *wallDate, uint16_t weight_s, uint16_t weight_t, uint16_t gx, uint16_t gy) {
	volatile uint16_t head_h = 0;
	volatile uint16_t head_v = 0;
	volatile uint16_t tail_h = 0;
	volatile uint16_t tail_v = 0;
	volatile uint16_t x_h[0xffff];
	volatile uint16_t y_h[0xffff];
	volatile uint16_t x_v[0xffff];
	volatile uint16_t y_v[0xffff];
	//init step
	wallDate->horizontal[0] = 0xffff;
	wallDate->horizontal[16] = 0xffff;
	wallDate->vertical[0] = 0xffff;
	wallDate->vertical[16] = 0xffff;
	for (uint8_t i = 0; i <= MAZE_SIZE; i++)
	{
		for (uint8_t j = 0; j <= MAZE_SIZE; j++) {
			stepEx_h[i][j] = MAX_STEP_EX;
			stepEx_v[i][j] = MAX_STEP_EX;
		}

	}
	//add goal step
	if (goal_size == 1)
	{
		//horizontal step
		if ((wallDate->horizontal[gy] & (0b1 << gx)) == FALSE) {
			stepEx_h[gx][gy] = 0;
			x_h[tail_h] = gx;
			y_h[tail_h] = gy;
			tail_h++;
		}
		if ((wallDate->horizontal[gy + 1] & (0b1 << gx)) == FALSE) {
			stepEx_h[gx][gy + 1] = 0;
			x_h[tail_h] = gx;
			y_h[tail_h] = gy + 1;
			tail_h++;
		}
		//vertical step
		if ((wallDate->vertical[gx] & (0b1 << gy)) == FALSE) {
			stepEx_v[gx][gy] = 0;
			x_v[tail_v] = gx;
			y_v[tail_v] = gy;
			tail_v++;
		}
		if ((wallDate->vertical[gx + 1] & (0b1 << gy)) == FALSE) {
			stepEx_v[gx + 1][gy] = 0;
			x_v[tail_v] = gx + 1;
			y_v[tail_v] = gy;
			tail_v++;
		}
	}
	//update stepEx
	while (head_h != tail_h || head_v != tail_v)
	{
		//printf("head_h:%d\ttail_h:%d\thead_v:%d\ttail_v:%d\r\n", head_h, tail_h, head_v, tail_v);
		//holizontal
		if (head_h != tail_h) {
			//holizontal->holizontal
			if (stepEx_h[x_h[head_h]][y_h[head_h]] != MAX_STEP_EX) {
				if (y_h[head_h] < (MAZE_SIZE - 1)) {
					if ((wallDate->horizontal[y_h[head_h] + 1] & (0b1 << x_h[head_h])) == FALSE) {
						if (stepEx_h[x_h[head_h]][y_h[head_h] + 1] /*== MAX_STEP_EX*/> stepEx_h[x_h[head_h]][y_h[head_h]] + weight_s) {
							stepEx_h[x_h[head_h]][y_h[head_h] + 1] = stepEx_h[x_h[head_h]][y_h[head_h]] + weight_s;
							uint16_t x_buff = x_h[head_h];
							uint16_t y_buff = y_h[head_h] + 1;
							x_h[tail_h] = x_buff;
							y_h[tail_h] = y_buff;
							tail_h++;
						}
					}
				}
				if (y_h[head_h] > 0) {
					if ((wallDate->horizontal[y_h[head_h] - 1] & (0b1 << x_h[head_h])) == FALSE) {
						if (stepEx_h[x_h[head_h]][y_h[head_h] - 1] /*== MAX_STEP_EX*/> stepEx_h[x_h[head_h]][y_h[head_h]] + weight_s) {
							stepEx_h[x_h[head_h]][y_h[head_h] - 1] = stepEx_h[x_h[head_h]][y_h[head_h]] + weight_s;
							uint16_t x_buff = x_h[head_h];
							uint16_t y_buff = y_h[head_h] - 1;
							x_h[tail_h] = x_buff;
							y_h[tail_h] = y_buff;
							tail_h++;
						}
					}
				}
				//holizontal->vertical
				if (x_h[head_h] > 0) {
					if ((wallDate->vertical[x_h[head_h]] & (0b1 << y_h[head_h])) == FALSE) {
						if (stepEx_v[x_h[head_h]][y_h[head_h]] /*== MAX_STEP_EX*/> stepEx_h[x_h[head_h]][y_h[head_h]] + weight_t) {
							stepEx_v[x_h[head_h]][y_h[head_h]] = stepEx_h[x_h[head_h]][y_h[head_h]] + weight_t;
							uint16_t x_buff = x_h[head_h];
							uint16_t y_buff = y_h[head_h];
							x_v[tail_v] = x_buff;
							y_v[tail_v] = y_buff;
							tail_v++;
						}
					}
				}
				if (y_h[head_h] > 0) {
					if ((wallDate->vertical[x_h[head_h]] & (0b1 << (y_h[head_h] - 1))) == FALSE) {
						if (stepEx_v[x_h[head_h]][y_h[head_h] - 1] /*== MAX_STEP_EX*/> stepEx_h[x_h[head_h]][y_h[head_h]] + weight_t) {
							stepEx_v[x_h[head_h]][y_h[head_h] - 1] = stepEx_h[x_h[head_h]][y_h[head_h]] + weight_t;
							uint16_t x_buff = x_h[head_h];
							uint16_t y_buff = y_h[head_h] - 1;
							x_v[tail_v] = x_buff;
							y_v[tail_v] = y_buff;
							tail_v++;
						}
					}
				}
				if (x_h[head_h] < (MAZE_SIZE - 1)) {
					if ((wallDate->vertical[x_h[head_h] + 1] & (0b1 << (y_h[head_h]))) == FALSE) {
						if (stepEx_v[x_h[head_h] + 1][y_h[head_h]] /*== MAX_STEP_EX*/> stepEx_h[x_h[head_h]][y_h[head_h]] + weight_t) {
							stepEx_v[x_h[head_h] + 1][y_h[head_h]] = stepEx_h[x_h[head_h]][y_h[head_h]] + weight_t;
							uint16_t x_buff= x_h[head_h] + 1;
							uint16_t y_buff= y_h[head_h];
							x_v[tail_v] = x_buff;
							y_v[tail_v] = y_buff;
							tail_v++;
						}
					}
				}
				if ((y_h[head_h] > 0) && (x_h[head_h] < (MAZE_SIZE - 1))) {
					if ((wallDate->vertical[x_h[head_h] + 1] & (0b1 << (y_h[head_h] - 1))) == FALSE) {
						if (stepEx_v[x_h[head_h] + 1][y_h[head_h] - 1] == MAX_STEP_EX/*> stepEx_h[x_h[head_h]][y_h[head_h]] + weight_t*/) {
							stepEx_v[x_h[head_h] + 1][y_h[head_h] - 1] = stepEx_h[x_h[head_h]][y_h[head_h]] + weight_t;
							uint16_t x_buff = x_h[head_h] + 1;
							uint16_t y_buff = y_h[head_h] - 1;
							x_v[tail_v] = x_buff;
							y_v[tail_v] = y_buff;
							tail_v++;
						}
					}
				}
			}
			head_h++;
		}
		//vertical
		if (head_v != tail_v) {
			if (stepEx_v[x_v[head_v]][y_v[head_v]] != MAX_STEP_EX) {
				//vertical->vertical
				if (x_v[head_v] < MAZE_SIZE - 1) {
					if ((wallDate->vertical[x_v[head_v] + 1] & (0b1 << y_v[head_v])) == FALSE) {
						if (stepEx_v[x_v[head_v] + 1][y_v[head_v]] /*== MAX_STEP_EX*/> stepEx_v[x_v[head_v]][y_v[head_v]] + weight_s) {
							stepEx_v[x_v[head_v] + 1][y_v[head_v]] = stepEx_v[x_v[head_v]][y_v[head_v]] + weight_s;
							uint16_t x_buff = x_v[head_v] + 1;
							uint16_t y_buff = y_v[head_v];
							x_v[tail_v] = x_buff;
							y_v[tail_v] = y_buff;
							tail_v++;
						}
					}
				}
				if (x_v[head_v] > 0) {
					if ((wallDate->vertical[x_v[head_v] - 1] & (0b1 << y_v[head_v])) == FALSE) {
						if (stepEx_v[x_v[head_v] - 1][y_v[head_v]] /*== MAX_STEP_EX */> stepEx_v[x_v[head_v]][y_v[head_v]] + weight_s) {
							stepEx_v[x_v[head_v] - 1][y_v[head_v]] = stepEx_v[x_v[head_v]][y_v[head_v]] + weight_s;
							uint16_t x_buff = x_v[head_v] - 1;
							uint16_t y_buff = y_v[head_v];
							x_v[tail_v] = x_buff;
							y_v[tail_v] = y_buff;
							tail_v++;
						}
					}
				}
				//vertical->horizontal
				if (y_v[head_v] > 0) {
					if ((wallDate->horizontal[y_v[head_v]] & (0b1 << x_v[head_v])) == FALSE) {
						if (stepEx_h[x_v[head_v]][y_v[head_v]] /*== MAX_STEP_EX */> stepEx_v[x_v[head_v]][y_v[head_v]] + weight_t) {
							stepEx_h[x_v[head_v]][y_v[head_v]] = stepEx_v[x_v[head_v]][y_v[head_v]] + weight_t;
							uint16_t x_buff = x_v[head_v];
							uint16_t y_buff = y_v[head_v];
							x_h[tail_h] = x_buff;
							y_h[tail_h] = y_buff;
							tail_h++;
						}
					}
				}
				if (x_v[head_v] > 0) {
					if ((wallDate->horizontal[y_v[head_v]] & (0b1 << (x_v[head_v] - 1))) == FALSE) {
						if (stepEx_h[x_v[head_v] - 1][y_v[head_v]] /*== MAX_STEP_EX */> stepEx_v[x_v[head_v]][y_v[head_v]] + weight_t) {
							stepEx_h[x_v[head_v] - 1][y_v[head_v]] = stepEx_v[x_v[head_v]][y_v[head_v]] + weight_t;
							uint16_t x_buff = x_v[head_v] - 1;
							uint16_t y_buff = y_v[head_v];
							x_h[tail_h] = x_buff;
							y_h[tail_h] = y_buff;
							tail_h++;
						}
					}
				}
				if (y_v[head_v] < (MAZE_SIZE - 1)) {
					if ((wallDate->horizontal[y_v[head_v] + 1] & (0b1 << x_v[head_v])) == FALSE) {
						if (stepEx_h[x_v[head_v]][y_v[head_v] + 1] /*== MAX_STEP_EX */> stepEx_v[x_v[head_v]][y_v[head_v]] + weight_t) {
							stepEx_h[x_v[head_v]][y_v[head_v] + 1] = stepEx_v[x_v[head_v]][y_v[head_v]] + weight_t;
							uint16_t x_buff = x_v[head_v];
							uint16_t y_buff = y_v[head_v] + 1;
							x_h[tail_h] = x_buff;
							y_h[tail_h] = y_buff;
							tail_h++;
						}
					}
				}
				if (((y_v[head_v] < (MAZE_SIZE - 1)) && (x_v[head_v] > 0))) {
					if ((wallDate->horizontal[y_v[head_v] + 1] & (0b1 << (x_v[head_v] - 1))) == FALSE) {
						if (stepEx_h[x_v[head_v] - 1][y_v[head_v] + 1] /*== MAX_STEP_EX*/ > stepEx_v[x_v[head_v]][y_v[head_v]] + weight_t) {
							stepEx_h[x_v[head_v] - 1][y_v[head_v] + 1] = stepEx_v[x_v[head_v]][y_v[head_v]] + weight_t;
							uint16_t x_buff = x_v[head_v] - 1;
							uint16_t y_buff = y_v[head_v] + 1;
							x_h[tail_h] = x_buff;
							y_h[tail_h] = y_buff;
							tail_h++;
						}
					}
				}
			}
			head_v++;
		}
	}
}


uint8_t Maze_GetNextMotionEx(pos_t *mypos, wallData_t *wall) {
	volatile uint16_t tmp_step = 0xffff; // 歩数
	uint8_t tmp_dir = 3;	 // 方向
									 // 現在の向きに応じて場合分けし、 歩数が少ない方向を判断
									 // 迷路外に進むのとゴールがスタートマス以外の場合(0,0)に進むのを阻止
	uint8_t x = mypos->x;
	uint8_t y = mypos->y;

	switch (mypos->dir)
	{
	case NORTH:
		if (stepEx_h[x][y + 1] < tmp_step)
		{
			if (y < MAZE_SIZE - 1)
			{
				if (((wall->horizontal[y + 1] >> x) & 0b1) == FALSE)
				{
					tmp_step = stepEx_h[x][y + 1];
					tmp_dir = FRONT;
				}
			}
		}
		if (stepEx_v[x][y] < tmp_step)
		{
			if (x > 0)
			{
				if (((wall->vertical[x] >> y) & 0b1) == FALSE)
				{
					tmp_step = stepEx_v[x][y];
					tmp_dir = LEFT;
				}
			}
		}
		if (stepEx_v[x + 1][y] < tmp_step)
		{
			if (x < MAZE_SIZE - 1)
			{
				if (((wall->vertical[x + 1] >> y) & 0b1) == FALSE)
				{
					tmp_step = stepEx_v[x + 1][y];
					tmp_dir = RIGHT;
				}
			}
		}
		break;
	case EAST:
		if (stepEx_v[x + 1][y] < tmp_step)
		{
			if (x < MAZE_SIZE - 1)
			{
				if (((wall->vertical[x + 1] >> y) & 0b1) == FALSE)
				{
					tmp_step = stepEx_v[x + 1][y];
					tmp_dir = FRONT;
				}
			}
		}
		if (stepEx_h[x][y+1] < tmp_step)
		{
			if (y < MAZE_SIZE - 1)
			{
				if (((wall->horizontal[y + 1] >> x) & 0b1) == FALSE)
				{
					tmp_step = stepEx_h[x][y + 1];
					tmp_dir = LEFT;
				}
			}
		}
		if (stepEx_h[x][y] < tmp_step)
		{
			if (y > 0)
			{
				if (((wall->horizontal[y] >> x) & 0b1) == FALSE)
				{
					tmp_step = stepEx_h[x][y];
					tmp_dir = RIGHT;
				}
			}
		}
		break;
	case SOUTH:
		if (stepEx_h[x][y] < tmp_step)
		{
			if (y > 0)
			{
				if (((wall->horizontal[y] >> x) & 0b1) == FALSE)
				{
					tmp_step = stepEx_h[x][y];
					tmp_dir = FRONT;
				}
			}
		}
		if (stepEx_v[x + 1][y] < tmp_step)
		{
			if (x < MAZE_SIZE - 1)
			{
				if (((wall->vertical[x + 1] >> y) & 0b1) == FALSE)
				{
					tmp_step = stepEx_v[x + 1][y];
					tmp_dir = LEFT;
				}
			}
		}
		if (stepEx_v[x][y] < tmp_step)
		{
			if (x > 0)
			{
				if (((wall->vertical[x] >> y) & 0b1) == FALSE)
				{
					tmp_step = stepEx_v[x][y];
					tmp_dir = RIGHT;
				}
			}
		}
		break;
	case WEST:
		if (stepEx_v[x][y] < tmp_step)
		{
			if (x > 0)
			{
				if (((wall->vertical[x] >> y) & 0b1) == FALSE)
				{
					tmp_step = stepEx_v[x][y];
					tmp_dir = FRONT;
				}
			}
		}
		if (stepEx_h[x][y] < tmp_step)
		{
			if (y > 0)
			{
				if (((wall->horizontal[y] >> x) & 0b1) == FALSE)
				{
					tmp_step = stepEx_h[x][y];
					tmp_dir = LEFT;
				}
			}
		}
		if (stepEx_h[x][y + 1] < tmp_step)
		{
			if (y < MAZE_SIZE - 1)
			{
				if (((wall->horizontal[y + 1] >> x) & 0b1) == FALSE)
				{
					tmp_step = stepEx_h[x][y + 1];
					tmp_dir = RIGHT;
				}
			}
		}
		break;
	default:
		break;
	}
	return tmp_dir;
}

uint16_t Maze_GetStepEx_h(uint8_t x, uint8_t y) {
	return stepEx_h[x][y];
}

uint16_t Maze_GetStepEx_v(uint8_t x, uint8_t y) {
	return stepEx_v[x][y];
}