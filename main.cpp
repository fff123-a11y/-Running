/*
	天天酷跑开发日志
	1. 创建项目
	2. 导入素材
	3. 创建游戏界面
			选择图形库
			天天酷跑基于“easyX”图形库
			1)创建窗口
			2）加载背景——loadimage
				a. 3部分背景由远及近，以不同速度移动
				b. 循环滚动——背景图片都是两倍窗口长度，循环滚动
			3）实现背景——putimage
				问题：背景图片png透明呈黑色——tool文件中putimagePNG2函数
	4. 实现玩家奔跑
	5. 实现玩家跳跃——用户按键交互输入
	6. 实现随机乌龟
	7. 创建障碍物结构体数据类型
	8. 使用障碍物结构体后重新初始化
	9. 封装后多个障碍物的显示
	10. 实现英雄下蹲
	11. 实现障碍物“柱子”
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <vector>//cpp中的容器实现可变数组
#include "tools.h"

using namespace std;

#define WIN_WIDTH 1012
#define WIN_HEIGHT 396
#define HORIZON 348    //地平线
#define OBSTACLE_CNT 6//障碍总数
#define NUMOFBGIMG 3
#define NUMOFHEROIMG 12
#define NUMOFTORTOISEIMG 7
#define NUMOFLIONIMG 6
#define NUMOFCOLIMG 4
#define WIN_SCORE 1000
//全局变量，游戏背景数组
IMAGE bgimgs[NUMOFBGIMG];
int bgX[NUMOFBGIMG] = { 0 };
int bgSpeed[NUMOFBGIMG] = { 1, 2, 4 };
//英雄图片
IMAGE heroimgs[NUMOFHEROIMG];
int heroX;
int heroY;
int heroIndex;
bool heroJump;
bool heroDown;
//int jumpMaxHeight;
int jumpOffset;
bool update;//页面刷新标志位；表示是否需要马上刷新页面
//下蹲
IMAGE imgHeroDown[2];
int heroBlood;
int score;

typedef enum
{
	TORTOISE,//乌龟	0
	LION,//狮子		1
	COL1,//柱子
	COL2,
	COL3,
	COL4,
	OBS_TYPE_CNT//障碍物总数
}obstacle_type;
vector<vector<IMAGE>>obstacleImgs;//可变数组存放各类障碍的图片
typedef struct
{
	int type;//类型
	int imgIndex;
	int x, y;//坐标
	int speed;//速度
	int force;//杀伤力
	bool exist;
	bool hited;
	bool scoreCNT;
}obstacle;
obstacle obs[OBSTACLE_CNT];//障碍物总数组
IMAGE imgsNUM[10];

//游戏初始化
void init()
{
	//创建游戏窗口
	initgraph(WIN_WIDTH, WIN_HEIGHT);
	//加载游戏背景
	char filePath[64];
	heroBlood = 100;
	for (int i = 0; i < NUMOFBGIMG; i++)
	{
		//  ./res/bg001.png
		sprintf(filePath, "./res/bg%03d.png", i + 1);
		loadimage(&bgimgs[i], filePath);
	}
	//加载英雄奔跑
	for (int i = 0; i < NUMOFHEROIMG; i++)
	{
		sprintf(filePath, "./res/hero%d.png", i + 1);
		loadimage(&heroimgs[i], filePath);
	}
	//设置英雄初始位置
	heroX = (WIN_WIDTH - heroimgs[0].getwidth()) / 2;
	heroY = HORIZON - heroimgs[0].getheight();
	heroIndex = 0;
	//初始英雄跳跃参数
	heroJump = false;
	heroDown = false;
	jumpOffset = -16;
	update = true;

	//加载英雄下蹲
	for (int i = 0; i < 2; i++)
	{
		sprintf(filePath, "./res/d%d.png", i + 1);
		loadimage(&imgHeroDown[i], filePath);
	}

	//加载乌龟进入obstacle数组
	IMAGE torImg;
	vector<IMAGE> torImgArr;
	for (int i = 0; i < NUMOFTORTOISEIMG; i++)
	{
		sprintf(filePath, "./res/t%d.png", i + 1);
		loadimage(&torImg, filePath);
		torImgArr.push_back(torImg);
	}
	obstacleImgs.push_back(torImgArr);
	//加载狮子进入obstacle数组
	IMAGE lionImgs;
	vector<IMAGE> lionImgsArr;
	for (int i = 0; i < NUMOFLIONIMG; i++)
	{
		sprintf(filePath, "./res/p%d.png", i + 1);
		loadimage(&lionImgs, filePath);
		lionImgsArr.push_back(lionImgs);
	}
	obstacleImgs.push_back(lionImgsArr);
	//加载柱子进入obstacle数组
	IMAGE colImgs;
	for (int i = 0; i < NUMOFCOLIMG; i++)
	{
		vector<IMAGE> colImgsArr;
		sprintf(filePath, "./res/h%d.png", i + 1);
		loadimage(&colImgs, filePath, 63, 260, true);
		colImgsArr.push_back(colImgs);
		obstacleImgs.push_back(colImgsArr);
	}
	//初始化障碍物池，全初始化为不存在
	for (int i = 0; i < OBSTACLE_CNT; i++)
	{
		obs[i].exist = false;
	}
	//预加载音效
	preLoadSound("./res/hit.mp3");
	mciSendString("play ./res/bg.mp3 repeat", 0, 0, 0);
	score = 0;
	//加载数字图片
	for (int i = 0; i < 10; i++)
	{
		sprintf(filePath, "./res/num/%d.png", i);
		loadimage(&imgsNUM[i], filePath);

	}
}


//渲染背景
void updateBg()
{
	putimagePNG2(bgX[0], 0, &bgimgs[0]);
	putimagePNG2(bgX[1], 119, &bgimgs[1]);
	putimagePNG2(bgX[2], 330, &bgimgs[2]);
}

void creatObs()
{
	int i;
	for (i = 0; i < OBSTACLE_CNT; i++)//找未存在的障碍物
	{
		if (obs[i].exist == false)
		{
			break;
		}
	}
	if (i == OBSTACLE_CNT)//未找到,即所有障碍物都存在
	{
		return;
	}
	obs[i].exist = true;
	obs[i].hited = false;
	obs[i].scoreCNT = false;
	obs[i].imgIndex = 0;
	//obs[i].type = (obstacle_type)(rand() % OBS_TYPE_CNT);
	obs[i].type = rand() % 3;//三种类型障碍物
	if (i >= 1 && obs[i].type == LION && obs[i - 1].type == COL1 && obs[i - 1].x > WIN_WIDTH - 800)
	{
		obs[i].type == TORTOISE;
	}
	if (obs[i].type == COL1)//2~5
	{
		obs[i].type += rand() % 4;
	}
	obs[i].x = WIN_WIDTH;
	obs[i].y = HORIZON - obstacleImgs[obs[i].type][0].getheight();
	if (obs[i].type == TORTOISE)
	{
		obs[i].speed = 1;
		obs[i].force = 5;
	}
	else if (obs[i].type == LION)
	{
		obs[i].speed = 4;
		obs[i].force = 20;
	}
	else if (obs[i].type >= COL1 && obs[i].type <= COL4)
	{
		obs[i].speed = 0;
		obs[i].force = 20;
		obs[i].y = 0;
	}
}


void pictureChange()
{
	//背景1、2、3图移动
	for (int i = 0; i < NUMOFBGIMG; i++)
	{
		bgX[i] -= bgSpeed[i];
		if (bgX[i] <= -WIN_WIDTH)
		{
			bgX[i] = 0;
		}
	}
	//实现跳跃，跳跃时动画帧不变，即不摆腿
	if (heroJump)//跳跃
	{
		heroY += jumpOffset;
		jumpOffset += 1;
		if (heroY > HORIZON - heroimgs[0].getheight())//超过下顶点，归位，重置初始化
		{
			jumpOffset = -16;
			heroY = HORIZON - heroimgs[0].getheight();
			heroJump = false;
		}
	}
	else if (heroDown) //下蹲
	{
		static int cnt = 0;
		int delay[2] = { 8, 32 };
		cnt++;
		if (cnt >= delay[heroIndex])
		{
			cnt = 0;
			heroIndex++;
			if (heroIndex == 2)
			{
				heroIndex = 0;
				heroDown = false;
			}
		}
	}
	else//奔跑
	{
		//英雄奔跑动画帧变化
		heroIndex = (heroIndex + 1) % NUMOFHEROIMG;
	}

	//创建障碍物，初始化对应障碍物的参数
	static int frameCnt = 0;
	static int enemyFrequ = 50;//100~400
	//srand((unsigned int)time(NULL));
	frameCnt++;
	if (frameCnt > enemyFrequ)
	{
		frameCnt = 0;
		enemyFrequ = 50 + rand() % 201;
		creatObs();
	}

	//更新障碍物参数
	for (int i = 0; i < OBSTACLE_CNT; i++)
	{
		if (obs[i].exist)
		{
			obs[i].x -= obs[i].speed + bgSpeed[2];
			obs[i].imgIndex = (obs[i].imgIndex + 1) % obstacleImgs[obs[i].type].size();
		}
		if (obs[i].x < -obstacleImgs[obs[i].type][0].getwidth() * 2)
		{
			obs[i].exist = false;
		}
	}




	update = false;
}
//渲染障碍物
void updateEnemy()
{
	for (int i = 0; i < OBSTACLE_CNT; i++)
	{
		if (obs[i].exist)
		{
			putimagePNG2(obs[i].x, obs[i].y, WIN_WIDTH, &obstacleImgs[obs[i].type][obs[i].imgIndex]);
		}
	}

}

void updateHero()
{
	if (heroDown)//下蹲状态
	{
		int downY = HORIZON - imgHeroDown[heroIndex].getheight();
		putimagePNG2(heroX, downY, &imgHeroDown[heroIndex]);//渲染英雄跑步
	}
	else//非蹲，即跑步
	{
		putimagePNG2(heroX, heroY, &heroimgs[heroIndex]);//渲染英雄跑步
	}
}
void updateBloodBar()
{
	drawBloodBar(10, 10, 200, 10, 2, BLUE, DARKGRAY, RED, heroBlood / 100.0);
}
void updateScore()
{
	//将分数转为字符串，方便判断
	int x = 10;
	int y = 25;
	char scoreStr[8];
	sprintf(scoreStr, "%d", score);
	for (int i = 0; scoreStr[i]; i++)
	{
		int num = scoreStr[i] - '0';
		putimagePNG2(x, y, &imgsNUM[num]);
		x += imgsNUM[num].getwidth() + 5;
	}
}
void cheakHit()//碰撞检测
{
	for (int i = 0; i < OBSTACLE_CNT; i++)
	{
		if (obs[i].exist && !obs[i].hited)
		{
			int a1x, a1y, a2x, a2y;
			int off = 22;//偏移量
			if (heroDown)//下蹲状态
			{
				a1x = heroX + off;
				a1y = HORIZON - imgHeroDown[heroIndex].getheight() + off;
				a2x = heroX + imgHeroDown[heroIndex].getwidth() - off;
				a2y = HORIZON - off;
			}
			else//非蹲，奔跑、跳跃状态
			{
				a1x = heroX + off;
				a1y = heroY + off;
				a2x = heroX + heroimgs[0].getwidth() - off;
				a2y = heroY + heroimgs[0].getheight() - off;
			}
			int b1x = obs[i].x + off;
			int b1y = obs[i].y + off;
			int b2x = obs[i].x + obstacleImgs[obs[i].type][obs[i].imgIndex].getwidth() - off;
			int b2y = obs[i].y + obstacleImgs[obs[i].type][obs[i].imgIndex].getheight() - off;
			if (rectIntersect(a1x, a1y, a2x, a2y, b1x, b1y, b2x, b2y))
			{
				obs[i].hited = true;
				heroBlood -= obs[i].force;
				printf("剩余血量%d\n", heroBlood);
				playSound("./res/hit.mp3");
			}
		}
	}
}

void cheakGameOver()
{
	if (heroBlood <= 0)
	{
		loadimage(0, "./res/over.png");
		FlushBatchDraw();
		mciSendString("stop ./res/bg.mp3", 0, 0, 0);
		system("pause");
		//暂停后，直接开始下一句
		init();
		mciSendString("play ./res/bg.mp3 repeat", 0, 0, 0);
	}
}
void gameStart()
{
	loadimage(0, "./res/over.png");
	system("pause");
}
void cheakScore()
{
	for (int i = 0; i < OBSTACLE_CNT; i++)
	{
		if (obs[i].exist && obs[i].hited == false &&
			heroX > obs[i].x + obstacleImgs[obs[i].type][obs[i].imgIndex].getwidth() &&
			obs[i].scoreCNT == false)
		{
			score += 10;
			obs[i].scoreCNT = true;
			printf("分数：%d\n", score);
		}
	}
}

void cheakVictory()
{
	if (score >= WIN_SCORE)
	{
		FlushBatchDraw();
		Sleep(1000);
		loadimage(0, "./res/win.png");
		FlushBatchDraw();
		mciSendString("stop ./res/bg.mp3", 0, 0, 0);
		mciSendString("play ./res/win.mp3", 0, 0, 0);
		system("pause");
		//暂停后，直接开始下一局
		init();
		mciSendString("play ./res/bg.mp3 repeat", 0, 0, 0);
	}
}
//处理用户输入
void keyEvent()
{
	if (_kbhit())//当有按键输入是，返回真。将不会阻塞游戏进程
	{
		char ch = _getch();//无需按下回车键
		if (ch == ' ' || ch == 72 || ch == 'w' || ch == 'W')
		{
			heroJump = true;
			update = true;
		}
		else if (ch == 80 || ch == 's' || ch == 'S')
		{
			update = true;
			heroDown = true;
			heroIndex = 0;
		}
	}

}
int main()
{
	int timecnt = 0;
	init();
	gameStart();
	while (1)
	{
		keyEvent();
		//控制刷新速度
		timecnt += getDelay();
		if (timecnt > 30)
		{
			timecnt = 0;
			update = true;
		}
		if (update)
		{
			BeginBatchDraw();
			updateBg();
			updateHero();
			updateEnemy();
			updateBloodBar();
			updateScore();
			cheakVictory();
			EndBatchDraw();

			pictureChange();
			cheakHit();
			cheakGameOver();
			cheakScore();
		}

	}

	return 0;
}
