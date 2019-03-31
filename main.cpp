//include
#include "DxLib.h"
#include "MazeCon.h"
#include <stdint.h>
#include <stdio.h>
#include "Map.h"
#include "Maze.h"

#define BOX_SIZE 48

//Private variables

//Private function prototypes
int MazeSimu(void);
void setSimlationWall_Init(wallData_t *wall, unsigned char select_maze);
void updateBoxPosition(int *box_x, int *box_y, int nextdir, pos_t mypos);

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
	SetGraphMode(800, 800, 32);

	if (DxLib_Init() == -1)		// ＤＸライブラリ初期化処理
	{
		return -1;			// エラーが起きたら直ちに終了
	}

	int maze_number = 8;
	char title[128];
	int px = 0, py = 0, side_wall_x = 7, side_wall_y = 48, front_wall_x = 48, front_wall_y = 7;
	int box_x = 10, box_y = 732, sx = 34, sy = 34;
	int gx = 7, gy = 7;
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

	//迷路アルゴリズムを呼び出す

	//ポジションを定義、初期化
	pos_t mypos;
	mypos.dir = 0;
	mypos.x = 0;
	mypos.y = 0;

	//マップを扱うクラスを定義、壁情報を初期化
	Map_Init(&wall_data);

	//歩数マップの作製
	unsigned char nextdir = FRONT;
	//最初の区画は進んでいるものとする
	Maze_UpdatePosition(nextdir, &mypos);

	box_y -= BOX_SIZE;

	unsigned char n_wall, e_wall, w_wall, s_wall;
	unsigned char flag_goal = 0;
	
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

			Maze_UpdateStepMap(&flag_goal,gx, gy, &wall_data);
			nextdir = Maze_GetNextMotion(&mypos, &wall_data);

			updateBoxPosition(&box_x, &box_y, nextdir, mypos);

			// マシンの座標を更新する
			Maze_UpdatePosition(nextdir, &mypos);

			//画面をクリア
			ClearDrawScreen();
			//ボックスを描画
			DrawBox(box_x, box_y, box_x + sx, box_y + sy, GetColor(0, 0, 255), TRUE);

			// 壁壁画関連
			for (int i = 0; i < 16; i++) {
				for (int j = 0; j < 16; j++) {
					py = (16 - 1 - j) * 48;
					px = i * 48;
					if (Maze_GetWallData(i, j, NORTH, &sim_maze) && !Maze_GetWallData(i, j, NORTH, &wall_data)) {
						DrawBox(px + 7, py, px + front_wall_x + 7, py + front_wall_y, GetColor(0, 255, 0), TRUE);
					}
					else if (Maze_GetWallData(i, j, NORTH, &wall_data)) {
						DrawBox(px + 7, py, px + front_wall_x + 7, py + front_wall_y, GetColor(255, 0, 0), TRUE);
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
						DrawBox(px + BOX_SIZE, py + 7, px + side_wall_x + BOX_SIZE, py + side_wall_y + 7, GetColor(0, 255, 0), TRUE);
					}
					else if (Maze_GetWallData(i, j, EAST, &wall_data)) {
						DrawBox(px + BOX_SIZE, py + 7, px + side_wall_x + BOX_SIZE, py + side_wall_y + 7, GetColor(255, 0, 0), TRUE);
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

			// ゴールについたらスタート座標に戻るようにする
			if (mypos.x == 7 && mypos.y == 7) {
				flag_goal = 1;
			}


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
			WaitTimer(100);
		}
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