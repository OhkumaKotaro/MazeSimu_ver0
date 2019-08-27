//include
#include "DxLib.h"
#include "MazeCon.h"
#include "stdint.h"
#include "stdio.h"
#include "Map.h"
#include "Maze.h"

#define WINDOW_SIZE 700//800
#define BOX_SIZE 42//48
#define POLL_SIZE 6//7

#define START_COLOR 0xd0d0d0
#define FRONT_COLOR 0xd0d0d0
#define SEARCH_COLOR 0x909090
#define ADJUST_COLOR 0xdd0000
#define T90_COLOR 0x0f0fff
#define T180_COLOR 0x00dddd
#define TV90_COLOR 0x00dd00
#define T45IN_COLOR 0xdddd00
#define T135IN_COLOR 0xdd00dd
#define T45OUT_COLOR 0xdddd00
#define T135OUT_COLOR 0xdd00dd
#define DIAGONAL_COLOR 0xd0d0d0
#define GOAL_COLOR 0xd0d0d0


//Private variables

//Private function prototypes
int MazeSimu(void);
void setSimlationWall_Init(wallData_t *wall, unsigned char select_maze);
void updateBoxPosition(int *box_x, int *box_y, int nextdir, pos_t mypos);
void DrawRootEx(wallData_t *wall,uint8_t gx,uint8_t gy);

// This program starts from "WinMain"
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int act_check = 1;

	while (act_check==1)
	{
		act_check = MazeSimu();
	}

	DxLib_End();

	return act_check;				// ソフトの終了 
}

int MazeSimu(void) {
	//ウインドウモードで起動
	ChangeWindowMode(TRUE);
	// 画面サイズは最大の 800 800 にしておく
	SetGraphMode(WINDOW_SIZE + 400, WINDOW_SIZE, 32);

	if (DxLib_Init() == -1)		// ＤＸライブラリ初期化処理
	{
		return -1;			// エラーが起きたら直ちに終了
	}

	int maze_number = 8;
	char title[128];
	int px = 0, py = 0, side_wall_x = POLL_SIZE, side_wall_y = BOX_SIZE, front_wall_x = BOX_SIZE, front_wall_y = POLL_SIZE;
	int box_x = 10, box_y = WINDOW_SIZE-BOX_SIZE-15, sx = BOX_SIZE-POLL_SIZE*2/*34*/, sy = BOX_SIZE - POLL_SIZE * 2/*34*/;
	unsigned char gx = 7, gy = 7;
	int select_map = 0;
	bool update = false;
	double time = 0.0;
	int check_mode = 0;
	int priority_select = 0;

	SetDrawScreen(DX_SCREEN_BACK);//描画先を裏画面にする

	while (!CheckHitKey(KEY_INPUT_LEFT))
	{
		//画面をクリア
		ClearDrawScreen();

		// 文字列の描画
		DrawFormatString(200, 200, GetColor(255, 255, 255), "マップを選択 [左矢印で次の動作に移る]\n 迷路選択 = %4d \n", select_map);

		if (CheckHitKey(KEY_INPUT_UP)) {
			select_map++;
			WaitTimer(200);
		}
		if (CheckHitKey(KEY_INPUT_DOWN)) {
			select_map--;
			WaitTimer(200);
		}

		if (select_map > 8) select_map = 0;
		if (select_map < 0)select_map = 8;

		ScreenFlip();//	メイン画面に転送

		//タイトルバーに表示するテキストの準備(Win32 APIのwsprintfA関数を使用)
		wsprintf(title, "迷路選択");

		//タイトルバーにテキストを表示する
		SetMainWindowText(title);

		//ウィンドウメッセージを処理
		if (ProcessMessage() == -1)break;
	}

	//迷路情報の読み込み
	wallData_t sim_maze;
	setSimlationWall_Init(&sim_maze, select_map);

	//壁情報を持った構造体を定義
	wallData_t wall_data;
	wallData_t wall_data_fast;

	//迷路アルゴリズムを呼び出す

	//ポジションを定義、初期化
	pos_t mypos;
	mypos.dir = 0;
	mypos.x = 0;
	mypos.y = 0;

	//マップを扱うクラスを定義、壁情報を初期化
	Map_Init(&wall_data);
	Map_InitFast(&wall_data_fast);

	//歩数マップの作製
	unsigned char nextdir = FRONT;
	//最初の区画は進んでいるものとする
	unsigned char flag_goal = 0;
	Maze_UpdatePosition(nextdir, &mypos);

	box_y -= BOX_SIZE;

	unsigned char n_wall, e_wall, w_wall, s_wall;

	//ゴールの座標を代入
	
	if (check_mode==0)
	{
		while (!CheckHitKey(KEY_INPUT_ESCAPE))
		{
			// 現在の座標の壁情報を読み込み、壁を探索データに追加する。
			n_wall = Maze_GetWallData(mypos.x, mypos.y, NORTH, &sim_maze);
			e_wall = Maze_GetWallData(mypos.x, mypos.y, EAST, &sim_maze);
			w_wall = Maze_GetWallData(mypos.x, mypos.y, WEST, &sim_maze);
			s_wall = Maze_GetWallData(mypos.x, mypos.y, SOUTH, &sim_maze);

			Map_addWall(&wall_data, &mypos, n_wall, e_wall, w_wall, s_wall);
			Map_addWall(&wall_data_fast, &mypos, n_wall, e_wall, w_wall, s_wall);

			Maze_UpdateStepMap(&flag_goal,gx, gy, &wall_data);
			nextdir = Maze_GetNextMotion(&mypos, &wall_data);

			updateBoxPosition(&box_x, &box_y, nextdir, mypos);

			// マシンの座標を更新する
			Maze_UpdatePosition(nextdir, &mypos);
			if (mypos.x==gx && mypos.y==gy)
			{
				flag_goal = 1;
			}

			//画面をクリア
			ClearDrawScreen();
			//ボックスを描画
			DrawBox(box_x, box_y, box_x + sx, box_y + sy, GetColor(0, 0, 255), TRUE);

			// 壁壁画関連
			for (int i = 0; i < 16; i++) {
				for (int j = 0; j < 16; j++) {
					py = (16 - 1 - j) * BOX_SIZE;
					px = i * BOX_SIZE;
					if (Maze_GetWallData(i, j, NORTH, &sim_maze) && !Maze_GetWallData(i, j, NORTH, &wall_data)) {
						DrawBox(px + POLL_SIZE, py, px + front_wall_x + POLL_SIZE, py + front_wall_y, GetColor(0, 255, 0), TRUE);
					}
					else if (Maze_GetWallData(i, j, NORTH, &wall_data)) {
						DrawBox(px + POLL_SIZE, py, px + front_wall_x + POLL_SIZE, py + front_wall_y, GetColor(255, 0, 0), TRUE);
					}

					if (Maze_GetWallData(i, j, WEST, &sim_maze) && !Maze_GetWallData(i, j, WEST, &wall_data)) {
						DrawBox(px, py, px + side_wall_x, py + side_wall_y, GetColor(0, 255, 0), TRUE);
					}
					else if (Maze_GetWallData(i, j, WEST, &wall_data)) {
						DrawBox(px, py, px + side_wall_x, py + side_wall_y, GetColor(255, 0, 0), TRUE);
					}

					// 歩数の情報を表示している
					DrawFormatString(px + 8, py + 12, GetColor(255, 0, 255), "%4d",Maze_GetStep(i,j));

					if (Maze_GetWallData(i, j, EAST, &sim_maze) && (!Maze_GetWallData(i, j, EAST, &wall_data))) {
						DrawBox(px + BOX_SIZE, py + POLL_SIZE, px + side_wall_x + BOX_SIZE, py + side_wall_y + POLL_SIZE, GetColor(0, 255, 0), TRUE);
					}
					else if (Maze_GetWallData(i, j, EAST, &wall_data)) {
						DrawBox(px + BOX_SIZE, py + POLL_SIZE, px + side_wall_x + BOX_SIZE, py + side_wall_y + POLL_SIZE, GetColor(255, 0, 0), TRUE);
					}

					if (Maze_GetWallData(i, j, SOUTH, &sim_maze) && !Maze_GetWallData(i, j, SOUTH, &wall_data)) {
						DrawBox(px, py + BOX_SIZE, px + front_wall_x, py + front_wall_y + BOX_SIZE, GetColor(0, 255, 0), TRUE);
					}
					else if (Maze_GetWallData(i, j, SOUTH, &wall_data)) {
						DrawBox(px, py + BOX_SIZE, px + front_wall_x, py + front_wall_y + BOX_SIZE, GetColor(255, 0, 0), TRUE);
					}

				}
			}
			//メイン画面に転送
			ScreenFlip();


			//タイトルバーに表示するテキストの準備(Win32 APIのwsprintfA関数を使用)
			wsprintf(title, "迷路のポジション[%2d][%2d] [ercを押せば終了]", mypos.x, mypos.y);


			//タイトルバーにテキストを表示する
			SetMainWindowText(title);

			//ウィンドウメッセージを処理
			if (ProcessMessage() == -1)break;

			//
			while (flag_goal==2)
			{
				if (CheckHitKey(KEY_INPUT_ESCAPE)) break;
			}

			//１ミリ秒待機
			WaitTimer(10);
		}
	}

	WaitTimer(100);
	ClearDrawScreen();
	ScreenFlip();
	//Map_Init(&wall_data);
	Maze_UpdateStepMapEx(&wall_data_fast, 7, 5, 7, 7);
	while (!CheckHitKey(KEY_INPUT_LEFT))
	{
		//画面をクリア
		ClearDrawScreen();
		//ボックスを描画
		DrawBox(box_x, box_y, box_x + sx, box_y + sy, GetColor(0, 0, 200), TRUE);

		// 壁壁画関連
		for (int i = 0; i < 16; i++) {
			for (int j = 0; j < 16; j++) {
				py = (16 - 1 - j) * BOX_SIZE;
				px = i * BOX_SIZE;
				if (Maze_GetWallData(i, j, NORTH, &sim_maze) && !Maze_GetWallData(i, j, NORTH, &wall_data)) {
					DrawBox(px + POLL_SIZE, py, px + front_wall_x + POLL_SIZE, py + front_wall_y, GetColor(0, 200, 0), TRUE);
				}
				else if (Maze_GetWallData(i, j, NORTH, &wall_data)) {
					DrawBox(px + POLL_SIZE, py, px + front_wall_x + POLL_SIZE, py + front_wall_y, GetColor(200, 0, 0), TRUE);
				}

				if (Maze_GetWallData(i, j, WEST, &sim_maze) && !Maze_GetWallData(i, j, WEST, &wall_data)) {
					DrawBox(px, py, px + side_wall_x, py + side_wall_y, GetColor(0, 200, 0), TRUE);
				}
				else if (Maze_GetWallData(i, j, WEST, &wall_data)) {
					DrawBox(px, py, px + side_wall_x, py + side_wall_y, GetColor(200, 0, 0), TRUE);
				}

				// 歩数の情報を表示している
				DrawFormatString(px + 8, py + 12, GetColor(100, 100, 100), "%d\r\n%d", Maze_GetStepEx_v(i, j),Maze_GetStepEx_h(i,j));

				if (Maze_GetWallData(i, j, EAST, &sim_maze) && (!Maze_GetWallData(i, j, EAST, &wall_data))) {
					DrawBox(px + BOX_SIZE, py + POLL_SIZE, px + side_wall_x + BOX_SIZE, py + side_wall_y + POLL_SIZE, GetColor(0, 200, 0), TRUE);
				}
				else if (Maze_GetWallData(i, j, EAST, &wall_data)) {
					DrawBox(px + BOX_SIZE, py + POLL_SIZE, px + side_wall_x + BOX_SIZE, py + side_wall_y + POLL_SIZE, GetColor(200, 0, 0), TRUE);
				}

				if (Maze_GetWallData(i, j, SOUTH, &sim_maze) && !Maze_GetWallData(i, j, SOUTH, &wall_data)) {
					DrawBox(px, py + BOX_SIZE, px + front_wall_x, py + front_wall_y + BOX_SIZE, GetColor(0, 200, 0), TRUE);
				}
				else if (Maze_GetWallData(i, j, SOUTH, &wall_data)) {
					DrawBox(px, py + BOX_SIZE, px + front_wall_x, py + front_wall_y + BOX_SIZE, GetColor(200, 0, 0), TRUE);
				}

			}
		}
		
		DrawRootEx(&wall_data_fast,7,7);
		//メイン画面に転送
		ScreenFlip();


		//タイトルバーに表示するテキストの準備(Win32 APIのwsprintfA関数を使用)
		wsprintf(title, "迷路のポジション[%2d][%2d] [ercを押せば終了]", mypos.x, mypos.y);


		//タイトルバーにテキストを表示する
		SetMainWindowText(title);

		//ウィンドウメッセージを処理
		if (ProcessMessage() == -1)break;

		//１ミリ秒待機
		WaitTimer(100);
	}

	return 0;
}


void setSimlationWall_Init(wallData_t *wall, unsigned char select_maze)
{
	// 迷路情報の代入
	pos_t pos;

	pos.dir = 0;
	pos.x = 0;
	pos.y = 0;

	uint8_t map[16][16];
	uint8_t map0[16][16] = { { 14,6,6,6,6,6,6,6,4,6,6,6,6,6,4,5 },{ 12,5,12,4,7,14,4,6,3,12,7,12,7,13,11,9 },{ 9,10,3,10,5,12,2,7,12,2,4,2,4,2,5,9 },{ 10,5,12,7,10,3,14,4,3,13,11,13,11,12,3,9 },{ 12,3,9,14,4,6,6,3,13,10,4,2,6,3,13,9 },{ 9,12,2,6,3,12,4,6,0,5,10,4,7,12,1,9 },{ 9,9,13,13,12,3,11,14,3,10,5,10,5,9,9,9 },{ 9,8,0,0,3,12,6,4,5,12,2,7,9,9,9,9 },{ 9,9,11,11,14,0,7,10,3,10,5,12,3,9,9,9 },{ 9,8,4,7,14,2,5,14,5,12,1,8,7,9,9,9 },{ 9,11,10,6,5,14,2,6,2,1,11,10,5,9,9,9 },{ 9,14,5,14,2,6,6,6,5,11,12,7,8,3,9,9 },{ 10,5,8,7,12,5,12,5,10,5,9,13,9,13,9,9 },{ 12,3,9,12,1,9,9,8,5,8,1,10,3,8,3,9 },{ 8,7,8,3,9,9,9,9,10,1,8,5,13,9,14,1 },{ 10,6,2,6,3,10,3,11,14,2,3,11,10,2,6,3 } };
	uint8_t map1[16][16] = { { 14,4,6,6,6,6,6,6,6,6,6,6,6,6,6,5 },{ 12,3,12,4,6,6,6,6,6,6,4,6,6,5,12,3 },{ 9,12,3,9,12,6,6,6,6,5,10,4,5,9,10,5 },{ 9,9,12,1,9,12,6,6,6,3,12,3,9,10,5,9 },{ 9,9,9,10,1,10,6,6,6,5,10,5,9,12,3,9 },{ 9,9,8,7,10,5,14,4,5,9,12,3,9,10,6,1 },{ 9,9,10,5,12,3,12,3,10,3,10,5,9,12,5,11 },{ 9,10,5,9,10,5,9,12,5,12,4,3,10,3,10,5 },{ 8,7,9,9,12,3,9,10,1,11,10,5,14,5,12,3 },{ 9,12,1,9,10,5,9,14,0,7,12,2,4,2,2,5 },{ 9,9,9,9,12,3,9,14,0,7,10,5,8,6,6,3 },{ 9,8,1,9,10,5,9,14,0,7,12,3,10,6,6,5 },{ 9,9,9,9,12,3,9,14,0,7,10,4,6,6,5,9 },{ 9,9,10,3,10,5,9,12,0,5,12,2,7,13,9,9 },{ 9,10,5,13,12,3,8,1,11,9,10,6,6,1,9,11 },{ 10,6,2,3,10,6,3,10,6,2,6,6,6,3,10,7 } };
	uint8_t map2[16][16] = { { 14,6,4,6,4,6,6,6,6,5,12,5,12,5,12,5 },{ 12,5,9,12,3,12,4,4,5,8,1,9,9,9,9,9 },{ 9,9,9,10,6,3,9,9,9,9,11,10,3,10,3,9 },{ 9,10,0,6,6,5,11,11,11,10,6,6,5,12,5,9 },{ 8,5,10,5,13,10,6,6,6,6,6,6,3,11,9,9 },{ 9,8,7,9,9,12,6,6,6,6,5,12,4,4,1,9 },{ 8,3,12,3,9,9,12,6,6,5,9,9,9,9,9,9 },{ 8,6,1,14,1,9,9,12,5,9,9,9,9,9,11,9 },{ 8,5,10,5,9,9,8,2,3,9,9,10,1,8,5,9 },{ 9,8,7,9,9,8,2,6,6,3,9,12,3,9,9,9 },{ 8,3,12,3,8,2,6,6,6,6,2,2,4,3,11,9 },{ 8,6,1,14,1,12,6,6,6,5,14,5,10,6,5,9 },{ 8,5,10,5,9,9,14,6,5,10,5,9,13,12,3,9 },{ 9,8,7,9,9,9,12,6,2,6,3,9,9,10,5,9 },{ 8,3,12,3,10,3,10,6,6,6,6,1,9,12,1,9 },{ 10,6,2,6,6,6,6,6,6,6,6,3,10,3,10,3 } };
	uint8_t map3[16][16] = { { 14,4,6,5,15,12,6,6,6,5,12,5,12,6,6,5 },{ 13,9,13,10,4,3,12,6,5,10,3,10,3,12,5,9 },{ 9,9,8,5,11,12,3,13,10,6,6,6,6,3,9,9 },{ 8,3,9,10,6,2,7,9,15,12,5,12,5,14,0,1 },{ 8,7,8,4,6,4,6,0,6,1,10,1,10,5,11,9 },{ 10,5,9,9,13,9,15,9,15,8,5,8,5,8,5,9 },{ 12,3,9,9,9,8,4,2,7,9,10,1,10,1,11,9 },{ 10,5,9,9,9,9,11,12,5,10,5,8,5,8,5,9 },{ 12,3,9,9,10,2,5,10,1,13,10,1,10,1,9,9 },{ 9,12,3,8,6,6,2,6,3,8,7,8,5,8,1,9 },{ 9,9,14,2,6,6,6,4,6,3,13,9,10,1,9,9 },{ 9,9,12,6,4,6,6,2,5,12,2,0,5,8,1,9 },{ 9,8,1,14,2,6,6,6,3,9,12,1,11,9,8,1 },{ 9,9,10,6,6,6,6,6,6,3,9,11,12,2,3,9 },{ 9,10,6,6,6,6,6,6,5,12,3,12,3,13,15,9 },{ 10,6,6,6,6,6,6,6,2,2,6,2,6,2,6,3 } };
	uint8_t map4[16][16] = { { 14,6,6,5,14,7,12,6,5,12,6,4,6,6,6,5 },{ 12,5,15,10,6,6,3,15,8,1,15,11,14,7,12,1 },{ 9,10,6,6,6,4,4,6,3,8,4,6,6,6,3,9 },{ 9,12,6,6,4,1,9,12,7,9,10,4,6,6,5,9 },{ 9,10,6,4,3,9,9,11,15,9,15,10,4,6,3,9 },{ 9,13,12,2,6,3,10,6,6,2,6,6,2,5,13,9 },{ 9,11,9,12,5,12,4,7,14,4,5,12,5,9,11,9 },{ 10,5,9,9,9,9,9,12,5,9,9,9,9,9,12,3 },{ 15,9,8,3,9,9,11,8,3,11,9,9,10,1,9,15 },{ 12,1,9,15,8,3,13,9,15,13,10,1,15,11,10,5 },{ 9,9,9,12,3,13,9,8,5,9,13,10,4,6,4,1 },{ 9,10,2,2,5,11,11,9,9,11,11,12,3,15,9,9 },{ 8,7,12,5,9,12,5,9,9,13,13,9,12,5,10,1 },{ 9,15,9,9,9,9,9,8,1,8,1,9,9,9,15,9 },{ 8,6,1,10,3,9,9,9,9,9,9,10,3,8,6,1 },{ 10,6,2,6,6,3,10,3,10,3,10,6,6,2,6,3 } };
	uint8_t map5[16][16] = { { 14,4,6,6,6,6,6,6,6,6,6,6,4,4,4,7 },{ 13,10,6,5,12,5,12,6,5,12,6,5,9,9,10,5 },{ 8,6,5,10,3,9,10,5,10,3,12,3,10,2,6,1 },{ 8,7,9,12,5,10,5,10,6,5,10,5,12,6,5,9 },{ 8,7,10,3,10,5,8,5,12,1,12,3,10,5,9,9 },{ 9,12,4,7,12,3,11,10,3,11,10,5,12,3,9,9 },{ 9,9,9,14,0,7,12,6,6,5,14,1,9,12,3,9 },{ 9,9,10,5,10,5,9,12,4,1,12,3,9,10,5,9 },{ 9,9,12,3,12,3,9,10,3,9,10,5,10,5,9,9 },{ 9,9,9,14,0,7,8,7,12,3,14,1,12,3,9,9 },{ 9,9,10,5,10,5,10,5,10,5,12,3,9,12,3,9 },{ 9,8,7,10,6,1,12,1,12,3,10,5,9,10,5,9 },{ 9,10,6,4,7,11,9,9,9,13,13,10,3,12,3,9 },{ 9,14,5,9,12,5,9,8,0,0,1,12,5,9,14,1 },{ 9,14,1,10,3,10,3,11,11,11,10,3,10,3,12,1 },{ 10,6,2,6,6,6,6,6,6,6,6,6,6,6,3,11 } };
	uint8_t map6[16][16] = { {14,6,4,6,6,6,6,6,6,6,6,6,6,6,6,5},{12,6,2,6,6,6,6,6,6,6,6,6,6,6,5,9},{9,12,6,5,12,6,6,6,5,12,6,6,6,5,9,9},{9,9,13,9,9,12,5,12,3,9,12,4,7,9,9,9},{9,9,9,9,9,9,10,3,13,10,3,10,5,9,9,9},{9,9,9,10,3,10,4,6,2,6,6,6,3,9,9,9},{9,10,2,6,6,5,10,6,6,6,6,5,12,1,9,9},{9,12,6,6,6,3,13,12,5,13,12,3,9,9,9,9},{9,9,13,13,12,6,1,10,1,8,3,12,3,11,9,9},{9,9,9,8,3,12,3,12,3,9,12,2,7,13,9,9},{9,9,8,3,12,3,12,1,12,3,10,6,5,8,1,9},{9,10,3,12,3,12,3,11,9,12,5,13,10,1,9,9},{9,13,12,3,12,0,6,5,9,9,9,8,5,9,9,9},{9,8,3,12,3,9,14,1,10,3,9,9,9,9,9,9},{9,10,6,2,6,2,6,2,6,7,10,3,10,3,9,9},{10,6,6,6,6,6,6,6,6,6,6,6,6,6,2,3} };
	uint8_t map7[16][16] = { {14,6,6,4,6,6,6,5,12,6,6,6,4,6,6,5},{14,6,5,9,12,6,5,9,9,14,4,6,1,12,5,9},{12,6,3,8,1,15,9,8,2,6,3,12,1,9,9,9},{8,6,4,1,10,5,10,2,4,6,6,3,10,3,9,9},{9,15,9,9,15,10,5,15,11,12,5,12,5,13,8,1},{9,12,3,10,6,5,8,5,12,3,9,9,10,2,3,9},{9,9,12,6,6,3,11,10,3,14,2,3,12,5,14,1},{8,3,10,6,6,5,15,12,5,12,6,5,9,8,5,9},{9,12,6,6,6,3,13,10,1,9,12,3,9,9,9,9},{9,9,14,6,5,12,2,5,9,9,10,6,3,9,9,9},{8,2,4,6,3,10,5,10,2,1,12,6,6,1,9,9},{9,12,3,14,4,5,10,6,5,9,10,6,5,9,10,3},{9,9,13,12,1,10,5,15,9,10,6,6,3,10,4,7},{9,9,8,1,9,15,10,5,10,6,6,6,5,15,8,7},{9,10,3,8,2,6,5,10,4,6,6,4,3,12,2,7},{10,6,6,3,14,7,10,6,3,15,14,2,6,2,6,7} };
	uint8_t map8[16][16] = { {14,6,5,12,4,4,6,6,6,6,4,4,5,12,6,7},{14,6,0,3,9,9,12,6,4,5,9,9,10,0,6,7},{14,4,2,6,3,9,8,5,9,9,9,10,6,2,4,7},{12,2,6,6,6,3,9,9,9,9,10,6,6,6,2,5},{9,12,6,6,6,6,3,9,9,10,6,6,6,6,5,9},{9,9,12,6,6,6,6,3,10,4,6,4,5,13,9,9},{9,9,9,12,5,12,4,7,14,0,5,9,9,9,9,9},{9,9,9,9,9,9,9,12,5,11,8,1,9,9,9,9},{9,9,9,9,9,9,9,10,2,5,11,8,0,3,9,9},{9,9,9,9,9,9,10,4,4,3,14,3,10,5,9,9},{9,9,9,9,9,9,12,1,9,12,5,12,5,9,9,9},{9,9,9,9,9,10,1,9,9,9,9,9,9,9,9,9},{9,9,9,9,9,12,0,1,9,9,9,9,9,9,9,9},{9,9,9,9,9,9,9,9,10,3,10,3,9,9,9,9},{9,9,10,3,8,2,2,1,14,6,6,6,2,3,9,9},{10,3,14,6,2,6,6,3,14,6,6,6,6,6,2,3} };
	uint8_t tempN, tempE, tempS, tempW;

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			switch (select_maze) {
			case 0: //2016 台湾 
				map[i][j] = map0[i][j];
				break;
			case 1:	//2016 全日本 決勝
				map[i][j] = map1[i][j];
				break;
			case 2:	//2016 学生大会
				map[i][j] = map2[i][j];
				break;
			case 3:	//2016 東日本
				map[i][j] = map3[i][j];
				break;
			case 4:	//2016全日本フレッシュマン予選
				map[i][j] = map4[i][j];
				break;

			case 5: // 2017 全日本大会 クラシック決勝
				map[i][j] = map5[i][j];
				break;

			case 6: // 2013 APEC
				map[i][j] = map6[i][j];
				break;

			case 7: // 2015 草の根
				map[i][j] = map7[i][j];
				break;

			case 8: // 2018全日本大会
				map[i][j] = map8[i][j];
				break;

			default:
				map[i][j] = map1[i][j];
				break;
			}
			//変換
			tempS = (map[i][j] & 0b1000) >> 3;	//南
			tempW = (map[i][j] & 0b0100) >> 2;	//西
			tempE = (map[i][j] & 0b0010) >> 1;	//東
			tempN = (map[i][j] & 0b0001) >> 0;	//北

			pos.x = i;
			pos.y = j;

			bool n_wall, e_wall, w_wall, s_wall;

			if (tempN == 1) {
				n_wall = true;
			}
			else {
				n_wall = false;
			}

			if (tempE == 1) {
				e_wall = true;
			}
			else {
				e_wall = false;
			}

			if (tempS == 1) {
				s_wall = true;
			}
			else {
				s_wall = false;
			}

			if (tempW == 1) {
				w_wall = true;
			}
			else {
				w_wall = false;
			}

			Map_addWall(wall, &pos, n_wall, e_wall, w_wall, s_wall);
		}
	}
}

void updateBoxPosition(int *box_x, int *box_y, int nextdir, pos_t mypos)
{
	if (nextdir == FRONT) {
		switch (mypos.dir) {
		case NORTH:
			*box_y -= BOX_SIZE;
			break;
		case EAST:
			*box_x += BOX_SIZE;
			break;
		case SOUTH:
			*box_y += BOX_SIZE;
			break;
		case WEST:
			*box_x -= BOX_SIZE;
			break;
		default:
			break;
		}
	}
	else if (nextdir == LEFT) {
		switch (mypos.dir) {
		case NORTH:
			*box_x -= BOX_SIZE;
			break;
		case EAST:
			*box_y -= BOX_SIZE;
			break;
		case SOUTH:
			*box_x += BOX_SIZE;
			break;
		case WEST:
			*box_y += BOX_SIZE;
			break;
		default:
			break;
		}
	}
	else if (nextdir == RIGHT) {
		switch (mypos.dir) {
		case NORTH:
			*box_x += BOX_SIZE;
			break;
		case EAST:
			*box_y += BOX_SIZE;
			break;
		case SOUTH:
			*box_x -= BOX_SIZE;
			break;
		case WEST:
			*box_y -= BOX_SIZE;
			break;
		default:
			break;
		}
	}
	else{
		switch (mypos.dir) {
		case NORTH:
			*box_y += BOX_SIZE;
			break;
		case EAST:
			*box_x -= BOX_SIZE;
			break;
		case SOUTH:
			*box_y -= BOX_SIZE;
			break;
		case WEST:
			*box_x += BOX_SIZE;
			break;
		default:
			break;
		}
	}
}

void DrawFront(volatile uint32_t *x, volatile uint32_t *y,uint8_t dir,uint16_t step,uint32_t color) {
	uint32_t x_buff = *x;
	uint32_t y_buff = *y;
	switch (dir)
	{
	case NORTH:
		*y -= ((BOX_SIZE / 2) * (uint32_t)step);
		break;
	case EAST:
		*x += ((BOX_SIZE / 2) * (uint32_t)step);
		break;
	case SOUTH:
		*y += ((BOX_SIZE / 2) * (uint32_t)step);
		break;
	case WEST:
		*x -= ((BOX_SIZE / 2) * (uint32_t)step);
		break;
	default:
		break;
	}
	DrawLine(x_buff, y_buff, *x, *y,color, 2);
}

void DrawLeft(volatile uint32_t *x, volatile uint32_t *y, uint8_t dir,uint32_t color) {
	uint32_t x_buff = *x;
	uint32_t y_buff = *y;
	switch (dir)
	{
	case NORTH:
		*x -= (BOX_SIZE / 2);
		*y -= (BOX_SIZE / 2);
		break;
	case EAST:
		*x += (BOX_SIZE / 2);
		*y -= (BOX_SIZE / 2);
		break;
	case SOUTH:
		*x += (BOX_SIZE / 2);
		*y += (BOX_SIZE / 2);
		break;
	case WEST:
		*x -= (BOX_SIZE / 2);
		*y += (BOX_SIZE / 2);
		break;
	default:
		break;
	}
	DrawLine(x_buff, y_buff, *x, *y, color, 2);
}

void DrawRight(volatile uint32_t *x, volatile uint32_t *y, uint8_t dir, uint32_t color) {
	uint32_t x_buff = *x;
	uint32_t y_buff = *y;
	switch (dir)
	{
	case NORTH:
		*x += (BOX_SIZE / 2);
		*y -= (BOX_SIZE / 2);
		break;
	case EAST:
		*x += (BOX_SIZE / 2);
		*y += (BOX_SIZE / 2);
		break;
	case SOUTH:
		*x -= (BOX_SIZE / 2);
		*y += (BOX_SIZE / 2);
		break;
	case WEST:
		*x -= (BOX_SIZE / 2);
		*y -= (BOX_SIZE / 2);
		break;
	default:
		break;
	}
	DrawLine(x_buff, y_buff, *x, *y, color, 2);
}

void DrawDiagonal(volatile uint32_t *x, volatile uint32_t *y, uint8_t dir,uint16_t step,uint32_t color) {
	switch (dir) {
	case NORTHWEST:
		for (int i= 0; i < step; i++) {
			DrawLeft(x, y, dir, color);
		}
		break;
	case SOUTHWEST:
		for (int i = 0; i < step; i++) {
			DrawLeft(x, y, dir, color);
		}
		break;
	case SOUTHEAST:
		for (int i = 0; i < step; i++) {
			DrawLeft(x, y, dir, color);
		}
		break;
	case NORTHEAST:
		for (int i = 0; i < step; i++) {
			DrawLeft(x, y, dir, color);
		}
		break;
	}
}

void DrawFastMotion(volatile uint32_t *x, volatile uint32_t *y, uint8_t *dir_origin, uint16_t motion) {
	uint8_t dir = *dir_origin;
	switch (motion&0xf)
	{
	case START:
		DrawFront(x, y, dir, (motion >> 4) + 1, START_COLOR);
		break;
	case FRONT:
		DrawFront(x, y, dir, motion >> 4, FRONT_COLOR);
		break;
	case ADJUST:
		DrawFront(x, y, dir, motion>>4, ADJUST_COLOR);
		break;
	case LEFT:
		switch (motion>>4)
		{
		case SEARCH:
			DrawLeft(x, y, dir, SEARCH_COLOR);
			if (dir<3)
			{
				dir++;
			}
			else {
				dir = 0;
			}
			
			break;
		case T_90:
			DrawFront(x, y, dir, 1, T90_COLOR);
			DrawLeft(x, y, dir, T90_COLOR);
			if (dir < 3)
			{
				dir++;
			}
			else {
				dir = 0;
			}
			DrawFront(x, y, dir, 1, T90_COLOR);
			break;
		case T_180:
			DrawFront(x, y, dir, 1, T180_COLOR);
			DrawLeft(x, y, dir, T180_COLOR);
			if (dir < 3)
			{
				dir++;
			}
			else {
				dir = 0;
			}
			DrawLeft(x, y, dir, T180_COLOR);
			if (dir < 3)
			{
				dir++;
			}
			else {
				dir = 0;
			}
			DrawFront(x, y, dir, 1, T180_COLOR);
			break;
		case T_45IN:
			DrawFront(x, y, dir, 1, T45IN_COLOR);
			DrawLeft(x, y, dir, T45IN_COLOR);
			break;
		case T_135IN:
			DrawFront(x, y, dir, 1, T135IN_COLOR);
			DrawLeft(x, y, dir, T135IN_COLOR);
			if (dir < 3)
			{
				dir++;
			}
			else {
				dir = 0;
			}
			DrawLeft(x, y, dir, T135IN_COLOR);
			break;
		case T_45OUT:
			DrawDiagonal(x, y, dir, 1, T45OUT_COLOR);
			if (dir < 3)
			{
				dir++;
			}
			else {
				dir = 0;
			}
			DrawFront(x, y, dir, 1, T45OUT_COLOR);
			break;
		case T_135OUT:
			DrawDiagonal(x, y, dir, 1, T135OUT_COLOR);
			if (dir < 3)
			{
				dir++;
			}
			else {
				dir = 0;
			}
			DrawDiagonal(x, y, dir, 1, T135OUT_COLOR);
			if (dir < 3)
			{
				dir++;
			}
			else {
				dir = 0;
			}
			DrawFront(x, y, dir, 1, T135OUT_COLOR);
			break;
		case T_V90:
			DrawDiagonal(x, y, dir, 1, TV90_COLOR);
			if (dir < 3)
			{
				dir++;
			}
			else {
				dir = 0;
			}
			DrawDiagonal(x, y, dir, 1, TV90_COLOR);
			break;
		default:
			break;
		}
		break;
	case RIGHT:
		switch (motion >> 4)
		{
		case SEARCH:
			DrawRight(x, y, dir, SEARCH_COLOR);
			if (dir > 0)
			{
				dir--;
			}
			else {
				dir = 3;
			}

			break;
		case T_90:
			DrawFront(x, y, dir, 1, T90_COLOR);
			DrawRight(x, y, dir, T90_COLOR);
			if (dir > 0)
			{
				dir--;
			}
			else {
				dir = 3;
			}
			DrawFront(x, y, dir, 1, T90_COLOR);
			break;
		case T_180:
			DrawFront(x, y, dir, 1, T180_COLOR);
			DrawRight(x, y, dir, T180_COLOR);
			if (dir > 0)
			{
				dir--;
			}
			else {
				dir = 3;
			}
			DrawRight(x, y, dir, T180_COLOR);
			if (dir > 0)
			{
				dir--;
			}
			else {
				dir = 3;
			}
			DrawFront(x, y, dir, 1, T180_COLOR);
			break;
		case T_45IN:
			DrawFront(x, y, dir, 1, T45IN_COLOR);
			DrawRight(x, y, dir, T45IN_COLOR);
			if (dir > 0)
			{
				dir --;
			}
			else {
				dir = 3;
			}
			break;
		case T_135IN:
			DrawFront(x, y, dir, 1, T135IN_COLOR);
			DrawRight(x, y, dir, T135IN_COLOR);
			if (dir > 0)
			{
				dir--;
			}
			else {
				dir = 3;
			}
			DrawRight(x, y, dir, T135IN_COLOR);
			if (dir > 0)
			{
				dir--;
			}
			else {
				dir = 3;
			}
			break;
		case T_45OUT:
			DrawDiagonal(x, y, dir, 1, T45OUT_COLOR);
			DrawFront(x, y, dir, 1, T45OUT_COLOR);
			break;
		case T_135OUT:
			DrawDiagonal(x, y, dir, 1, T135OUT_COLOR);
			if (dir > 0)
			{
				dir--;
			}
			else {
				dir = 3;
			}
			DrawDiagonal(x, y, dir, 1, T135OUT_COLOR);
			DrawFront(x, y, dir, 1, T135OUT_COLOR);
			break;
		case T_V90:
			DrawDiagonal(x, y, dir, 1, TV90_COLOR);
			if (dir > 0)
			{
				dir--;
			}
			else {
				dir = 3;
			}
			DrawDiagonal(x, y, dir, 1, TV90_COLOR);
			break;
		default:
			break;
		}
		break;
	case DIAGONAL:
		DrawDiagonal(x, y, dir, motion>>4, DIAGONAL_COLOR);
		break;
	case GOAL:
		DrawFront(x, y, dir, (motion >> 4) + 1, GOAL_COLOR);
		break;
	default:
		break;
	}
	*dir_origin = dir;
}

void DrawRootEx(wallData_t *wall,uint8_t gx,uint8_t gy) {
	pos_t pos;
	uint16_t motion[255];
	uint8_t head = 0;
	uint8_t tail = 0;
	volatile uint32_t draw_x = 28;
	volatile uint32_t draw_y = WINDOW_SIZE - 2*BOX_SIZE + 10;
	pos.dir = NORTH;
	pos.x = 0;
	pos.y = 1;
	motion[tail] = START;
	tail++;
	motion[tail] = 1 << 4 | FRONT;
	tail++;
	head+=2;
	while (pos.x!=gx || pos.y!=gy) {
		motion[tail] = Maze_GetNextMotionEx(&pos, wall);
		tail++;
		Maze_UpdatePosition(motion[head]&0xf, &pos);
		head++;
	}
	motion[tail] = GOAL;
	tail++;
	uint32_t velocity[255];
	Maze_Compress(FALSE, motion, velocity, &tail);
	DrawFormatString(WINDOW_SIZE, 20, 0xC000, "▼ Fast Map ▼\r\n");
	head = 0;
	uint16_t point_x = WINDOW_SIZE;
	uint16_t point_y = 20 + 15;

	draw_x = 25;
	draw_y = WINDOW_SIZE - BOX_SIZE + 15;
	uint8_t dir = 0;
	while (head != tail)
	{
		uint16_t buff = motion[head];
		DrawFastMotion(&draw_x, &draw_y, &dir, buff);
		switch (motion[head] & 0xf)
		{
		case START:
			DrawFormatString(point_x, point_y, 0xC000, "SART:%d",motion[head]>>4);
			break;
		case FRONT:
			DrawFormatString(point_x, point_y, 0xC000, "FRONT:%d", motion[head] >> 4);
			break;
		case LEFT:
			DrawFormatString(point_x, point_y, 0xC000, "LEFT:%d", motion[head] >> 4);
			break;
		case RIGHT:
			DrawFormatString(point_x, point_y, 0xC000, "RIGHT:%d", motion[head] >> 4);
			break;
		case DIAGONAL:
			DrawFormatString(point_x, point_y, 0xC000, "DIAGONAL:%d", motion[head] >> 4);
			break;
		case ADJUST:
			DrawFormatString(point_x, point_y, 0xC000, "ADJUST:%d", motion[head] >> 4);
			break;
		case GOAL:
			DrawFormatString(point_x, point_y, 0xC000, "GOAL:%d", motion[head] >> 4);
			break;
		default:
			break;
		}
		head++;
		point_y += 15;
		if (point_y > WINDOW_SIZE - 15) {
			point_y = 20 + 15;
			point_x += 200;
		}
	}
}