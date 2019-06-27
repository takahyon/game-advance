#include <stdio.h>
#include <windows.h>
#include <math.h>
#include <time.h>

#define	MAP_WIDTH			17			// 盤の横
#define	MAP_LENGTH			17			// 盤の縦

#define TOTAL_PIECE_NUMBER	16			// 駒の数
#define MOVABLE_PIECENO		7			// 動かせる駒の数

#define PLAYER				0			// プレイヤー
#define COMPUTER			1			// COM

// マスの色
#define COLOR_BLOCK			0			// 移動不可能のマスは黒
#define COLOR_SAND			14*16		// 砂地のマスは黄色
#define COLOR_GRASS			2*16		// 草地のマスは緑
#define COLOR_ROCK			12*16		// 岩場のマスは赤
#define COLOR_RESET			15			// マスの色をリセット
#define COLOR_RANGE			9*16		// 移動範囲のマスは青
#define MOVED_COLOR			13			// 動いた駒は紫

// 駒のID
#define COM_CASTLE_ID		1
#define COM_KNIGHT_ID		2
#define COM_SOLDIER1_ID		3
#define COM_SOLDIER2_ID		4
#define COM_SOLDIER3_ID		5
#define COM_SOLDIER4_ID		6
#define COM_ARCHER1_ID		7
#define COM_ARCHER2_ID		8
#define PLAYER_CASTLE_ID	9
#define PLAYER_KNIGHT_ID	10
#define PLAYER_SOLDIER1_ID	11
#define PLAYER_SOLDIER2_ID	12
#define PLAYER_SOLDIER3_ID	13
#define PLAYER_SOLDIER4_ID	14
#define PLAYER_ARCHER1_ID	15
#define PLAYER_ARCHER2_ID	16

// 駒のヒットポイント
#define CASTLE_HITPOINT		150
#define KNIGHT_HITPOINT		100
#define SOLDIER_HITPOINT	80
#define ARCHER_HITPOINT		50

// 駒の攻撃力
#define CASTLE_ATTACK		90
#define KNIGHT_ATTACK		65
#define SOLDIER_ATTACK		50
#define ARCHER_ATTACK		40

// 駒の防御力
#define CASTLE_DEFENSE		30
#define KNIGHT_DEFENSE		50
#define SOLDIER_DEFENSE		40
#define ARCHER_DEFENSE		35

// 駒の移動力
#define CASTLE_MOVPOWER		1
#define KNIGHT_MOVPOWER		8
#define SOLDIER_MOVPOWER	5
#define ARCHER_MOVPOWER		5

// 駒の状況を保管するためのマスク
#define ALIVE_MASK			0x0001
#define MOVED_MASK			0x0002
#define COM_PIECE_MASK		0x0004

// 地形によっての移動力
#define SAND_ABILITY		0
#define GRASS_ABILITY		5
#define ROCK_ABILITY		30

// ゲームの状況
#define COM_TO_MOVE			1
#define PLAYER_TO_MOVE		2
#define COM_WINS			3
#define PLAYER_WINS			4

#define WEAK_CASTLE			50		// 弱い城の場合に回復できない
#define WEAK_PIECE			25		// コマが弱くなっている
#define RECOVERY_RATE		10		// 回復率

#define MIN_DISTANCE		0		// 最短距離
#define MAX_DISTANCE		99		// 最大距離

#define ATTACK_CASTLE		1		// プレイヤーの城を攻撃する
#define RETREAT_TO_CASTLE	2		// 自分の城に退却する
#define DANGER_DISTANCE		7		// 敵のコマが自分の城に近い距離

#define HOT_SPOT_BONUS			4	// 弱い敵がいろので行こう！
#define SOFT_SPOT_BONUS			2	// 少し弱い敵がいるので行っても良い
#define VERY_BAD_SPOT_BONUS		-4	// 強い敵がいるので逃げろ！
#define BAD_SPOT_BONUS			-2	// 少し強い敵がいるので逃げた方が良い
#define EVEN_SPOT_BONUS			1	// 自分と同じHPをもっている敵がいろので行っても良いかも
#define MOVE_OPP_CASTLE_BONUS	4	// 敵の城に行こう！
#define MOVE_OWN_CASTLE_BONUS	1	// 自分の城を守っても良いかも
#define JOIN_FORCES_BONUS		1	// 連携で動きましょう

// 地形情報のための定数
enum{
	SQ_BLOCK = 0,		// 移動不能
	SQ_GRASS,			// 草地
	SQ_SAND,			// 砂地
	SQ_ROCK,			// 岩場
	SQ_END
};

//駒の種類
enum{
	PIECE_CASTLE = 0,	// 城
	PIECE_SOLDIER,		// 歩兵
	PIECE_ARCHER,		// 弓兵
	PIECE_KNIGHT,		// 騎兵
	PIECE_END
};

// 駒の情報
typedef struct Piece {
	int	x;				// x座標
	int y;				// y座標
	int	hitPoint;		// ヒットポイント
	int	movPow;			// 移動力
	int	attack;			// 攻撃力
	int	defense;		// 防御力
	int	pieceType;		// 駒の種類
	int	status;			// 状態
} PIECE;

//------------------------------------------------------------地図データ
const int terrainMap[MAP_WIDTH][MAP_LENGTH] = {
	SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK,
	SQ_BLOCK, SQ_SAND, SQ_SAND, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_ROCK, SQ_SAND, SQ_ROCK, SQ_BLOCK, SQ_BLOCK, SQ_ROCK, SQ_SAND, SQ_SAND, SQ_SAND, SQ_SAND, SQ_BLOCK,
	SQ_BLOCK, SQ_SAND, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_ROCK, SQ_SAND, SQ_ROCK, SQ_ROCK, SQ_ROCK, SQ_SAND, SQ_SAND, SQ_SAND, SQ_SAND, SQ_GRASS, SQ_BLOCK,
	SQ_BLOCK, SQ_SAND, SQ_GRASS, SQ_GRASS, SQ_BLOCK, SQ_SAND, SQ_GRASS, SQ_SAND, SQ_SAND, SQ_SAND, SQ_SAND, SQ_SAND, SQ_BLOCK, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_BLOCK,
	SQ_BLOCK, SQ_GRASS, SQ_GRASS, SQ_ROCK, SQ_BLOCK, SQ_SAND, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_SAND, SQ_SAND, SQ_SAND, SQ_BLOCK, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_BLOCK,
	SQ_BLOCK, SQ_ROCK, SQ_GRASS, SQ_ROCK, SQ_ROCK, SQ_SAND, SQ_SAND, SQ_GRASS, SQ_GRASS, SQ_SAND, SQ_SAND, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_BLOCK,
	SQ_BLOCK, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_ROCK, SQ_GRASS, SQ_ROCK, SQ_GRASS, SQ_GRASS, SQ_SAND, SQ_SAND, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_BLOCK,
	SQ_BLOCK, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_ROCK, SQ_SAND, SQ_SAND, SQ_SAND, SQ_SAND, SQ_ROCK, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_BLOCK,
	SQ_BLOCK, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_ROCK, SQ_ROCK, SQ_SAND, SQ_SAND, SQ_SAND, SQ_ROCK, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_BLOCK,
	SQ_BLOCK, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_ROCK, SQ_ROCK, SQ_ROCK, SQ_SAND, SQ_BLOCK, SQ_SAND, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_BLOCK,
	SQ_BLOCK, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_BLOCK, SQ_GRASS, SQ_SAND, SQ_SAND, SQ_SAND, SQ_BLOCK, SQ_GRASS, SQ_BLOCK, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_BLOCK,
	SQ_BLOCK, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_BLOCK, SQ_GRASS, SQ_SAND, SQ_SAND, SQ_SAND, SQ_SAND, SQ_GRASS, SQ_BLOCK, SQ_BLOCK, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_BLOCK,
	SQ_BLOCK, SQ_GRASS, SQ_GRASS, SQ_SAND, SQ_BLOCK, SQ_SAND, SQ_SAND, SQ_SAND, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_BLOCK,
	SQ_BLOCK, SQ_GRASS, SQ_SAND, SQ_SAND, SQ_SAND, SQ_SAND, SQ_SAND, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_BLOCK,
	SQ_BLOCK, SQ_SAND, SQ_SAND, SQ_SAND, SQ_GRASS, SQ_ROCK, SQ_SAND, SQ_SAND, SQ_BLOCK, SQ_BLOCK, SQ_SAND, SQ_BLOCK, SQ_ROCK, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_BLOCK,
	SQ_BLOCK, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_GRASS, SQ_ROCK, SQ_SAND, SQ_SAND, SQ_SAND, SQ_SAND, SQ_ROCK, SQ_BLOCK, SQ_ROCK, SQ_ROCK, SQ_SAND, SQ_BLOCK, SQ_BLOCK,
	SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK, SQ_BLOCK
};

int pieceMap[MAP_LENGTH][MAP_WIDTH];	//	駒の場所
PIECE gamePieces[TOTAL_PIECE_NUMBER + 1];	// 駒の情報の管理

int gameStatus;				// ゲームの状況（手番、終了）
int piecesToMove;			// まだ動かしていない駒の数
int playerAlivePieceNo;		// プレイヤー側の生きている駒の数
int comAlivePieceNo;		// COM側の生きている駒の数
int playerCastleDestroyed;	// プレイヤーの城が崩壊かどうか
int comCastleDestroyed;		// COMの城が崩壊かどうか

int	moveRange[MAP_LENGTH][MAP_WIDTH];	//移動可能範囲などのチェック用
int	tmpMap[MAP_LENGTH][MAP_WIDTH];

//-----------------------------------------------------------------------------移動に必要なコスト
int	moveCost[PIECE_END][SQ_END + 1] = {
	{ 99, 99, 99, 99 },		//城は動けない
	{ 99, 1, 1, 2 },		//歩兵
	{ 99, 1, 2, 4 },		//弓兵
	{ 99, 1, 3, 4 },		//騎兵
};

//-----------------------------------------------------------------------------駒の相性
//　（城・騎兵・歩兵・弓兵）相性
int	pieceCompatibility[PIECE_END][PIECE_END] = {
	{ 0, 0, 0, 15 },		//城
	{ 0, 0, -25, 25 },		//騎兵
	{ 0, 25, 0, -50 },		//歩兵
	{ -15, -25, 50, 0 }, 	//弓兵	
};

//地形効果の値（進入不能・草地・砂地・岩場）
int	groundPower[SQ_END] = { 0, 5, 0, 30 };

// 関数の宣言
void initPieces();
void showMapWithPieces();
void showSquareInfo(int, int, int);
void displayCoordinates(int, int);
void selectAndMovePlayerPiece();
void getPlayerPiece(int *, int *);

void findMoveRange(int, int, const PIECE *, int);
void excludePiece(int, int);
void checkRange(int, int, int, int, int[][MAP_WIDTH]);
void checkDistance(int, int, int, int, int[][MAP_WIDTH]);
void fillMap(int[][MAP_WIDTH], int);
void showMoveRange();
void showPieceStatus();
void getPlayerDestination(int, int *, int *);
int flip(int);
void nextToCastleRecovery(int);
int getFullHitPointsForPiece(int);
void resolveBattle(PIECE *, int);
int selectOpponent(PIECE *, int);
void fightBattle(PIECE *, int);
int updateBattleHitPoints(int, int, int, int);

void moveComPieces();
void decideComPieceDestination(int, int *, int *);
void cpuAttackRetreat(int, int, int *, int *);
void addPieceInfoToMoveMap(int, int[][MAP_WIDTH]);

int main()
{

	char c, userSelection;

	srand((unsigned)time(NULL));	//ランダマイズ（乱数の初期化）

	initPieces();	// 駒の情報を初期化

	gameStatus = PLAYER_TO_MOVE;	// プレイヤーの手番
	playerAlivePieceNo = comAlivePieceNo = MOVABLE_PIECENO;	// すべての駒は生きている
	playerCastleDestroyed = comCastleDestroyed = 0;			// 城はまだ崩壊されていない
	piecesToMove = playerAlivePieceNo;	// プレイヤーがまだ動かしていない駒の数

	printf("===================================\n");
	printf("Simple console based strategy game.\n");
	printf("===================================\n");

	while (1)
	{
		showMapWithPieces();	// 盤を表示

		if (gameStatus == PLAYER_TO_MOVE) // プレイヤーの手番
		{
			// メーン　メニュ
			printf("Possible actions:\n");
			printf("1) Select piece to move\n");
			printf("2) Show complete piece status\n");
			printf("N) Start new game\n");
			printf("Q) Quit program\n");
			printf("Please enter your selection: ");

			c = getchar();
			userSelection = c;

			while ((c = getchar()) != '\n')
				;

			switch (userSelection)
			{
			case '1':
				selectAndMovePlayerPiece();	// プレイヤーの駒を動かす
				break;
			case '2':
				showPieceStatus();	// 駒の情報を表示
				break;
			case 'N':	// ゲームを再スタート
				initPieces();
				playerAlivePieceNo = comAlivePieceNo = MOVABLE_PIECENO;
				playerCastleDestroyed = comCastleDestroyed = 0;
				piecesToMove = playerAlivePieceNo;
				break;
			case 'Q':	// プログラムを終了
				return 0;
			default:
				break;
			}

		}
		else if (gameStatus == COM_WINS) // COMの勝ち宣言と初期化
		{
			if (playerCastleDestroyed)
				printf("\n\nI DESTROYED YOUR CASTLE!\n");
			else if (playerAlivePieceNo == 0)
				printf("\n\nI KILLED YOUR ARMY!\n");
			printf("\n\nYOU LOSE!!\n\n");
			printf("Want to try again?\n\n");
			initPieces();
			gameStatus = PLAYER_TO_MOVE;
			playerAlivePieceNo = comAlivePieceNo = MOVABLE_PIECENO;
			playerCastleDestroyed = comCastleDestroyed = 0;
			piecesToMove = playerAlivePieceNo;
		}
		else if (gameStatus == PLAYER_WINS) // COMの負け宣言と初期化
		{
			if (comCastleDestroyed)
				printf("\n\nYou destroyed my castle...\n");
			else if (comAlivePieceNo == 0)
				printf("\n\nYou killed my army...\n");
			printf("\n\nYOU WIN!!\n\n");
			printf("Want to try again?\n\n");
			initPieces();
			gameStatus = PLAYER_TO_MOVE;
			playerAlivePieceNo = comAlivePieceNo = MOVABLE_PIECENO;
			playerCastleDestroyed = comCastleDestroyed = 0;
			piecesToMove = playerAlivePieceNo;
		}
		else {
			moveComPieces(); // COMの駒を動かす
		}

	}
	return 0;
}

// 駒の場所と特徴を初期化
void initPieces()
{

	int x, y;

	for (x = 1; x < MAP_LENGTH - 1; x++)
	for (y = 1; y < MAP_WIDTH - 1; y++)
		pieceMap[x][y] = 0;

	// COMの駒
	pieceMap[8][1] = COM_CASTLE_ID;									// この駒の場所
	gamePieces[COM_CASTLE_ID].x = 8;									// この駒の場所を駒情報にも保管
	gamePieces[COM_CASTLE_ID].y = 1;									// この駒の場所を駒情報にも保管
	gamePieces[COM_CASTLE_ID].hitPoint = CASTLE_HITPOINT;				// この駒のヒットポイント
	gamePieces[COM_CASTLE_ID].pieceType = PIECE_CASTLE;				// この駒の種類
	gamePieces[COM_CASTLE_ID].attack = CASTLE_ATTACK;					// この駒の攻撃力
	gamePieces[COM_CASTLE_ID].defense = CASTLE_DEFENSE;				// この駒の防御力
	gamePieces[COM_CASTLE_ID].movPow = CASTLE_MOVPOWER;				// この駒の移動力
	gamePieces[COM_CASTLE_ID].status = ALIVE_MASK | COM_PIECE_MASK;	// この駒は生きているCOMの駒、城は動けない

	pieceMap[8][2] = COM_KNIGHT_ID;
	gamePieces[COM_KNIGHT_ID].x = 8;
	gamePieces[COM_KNIGHT_ID].y = 2;
	gamePieces[COM_KNIGHT_ID].hitPoint = KNIGHT_HITPOINT;
	gamePieces[COM_KNIGHT_ID].pieceType = PIECE_KNIGHT;
	gamePieces[COM_KNIGHT_ID].attack = KNIGHT_ATTACK;
	gamePieces[COM_KNIGHT_ID].defense = KNIGHT_DEFENSE;
	gamePieces[COM_KNIGHT_ID].movPow = KNIGHT_MOVPOWER;
	gamePieces[COM_KNIGHT_ID].status = ALIVE_MASK | MOVED_MASK | COM_PIECE_MASK;	// この駒は生きているCOMの駒、最初はプレイヤーの手番ので「動いた」状況

	pieceMap[6][2] = COM_SOLDIER1_ID;
	gamePieces[COM_SOLDIER1_ID].x = 6;
	gamePieces[COM_SOLDIER1_ID].y = 2;
	gamePieces[COM_SOLDIER1_ID].hitPoint = SOLDIER_HITPOINT;
	gamePieces[COM_SOLDIER1_ID].pieceType = PIECE_SOLDIER;
	gamePieces[COM_SOLDIER1_ID].attack = SOLDIER_ATTACK;
	gamePieces[COM_SOLDIER1_ID].defense = SOLDIER_DEFENSE;
	gamePieces[COM_SOLDIER1_ID].movPow = SOLDIER_MOVPOWER;
	gamePieces[COM_SOLDIER1_ID].status = ALIVE_MASK | MOVED_MASK | COM_PIECE_MASK;

	pieceMap[7][2] = COM_SOLDIER2_ID;
	gamePieces[COM_SOLDIER2_ID].x = 7;
	gamePieces[COM_SOLDIER2_ID].y = 2;
	gamePieces[COM_SOLDIER2_ID].hitPoint = SOLDIER_HITPOINT;
	gamePieces[COM_SOLDIER2_ID].pieceType = PIECE_SOLDIER;
	gamePieces[COM_SOLDIER2_ID].attack = SOLDIER_ATTACK;
	gamePieces[COM_SOLDIER2_ID].defense = SOLDIER_DEFENSE;
	gamePieces[COM_SOLDIER2_ID].movPow = SOLDIER_MOVPOWER;
	gamePieces[COM_SOLDIER2_ID].status = ALIVE_MASK | MOVED_MASK | COM_PIECE_MASK;

	pieceMap[9][2] = COM_SOLDIER3_ID;
	gamePieces[COM_SOLDIER3_ID].x = 9;
	gamePieces[COM_SOLDIER3_ID].y = 2;
	gamePieces[COM_SOLDIER3_ID].hitPoint = SOLDIER_HITPOINT;
	gamePieces[COM_SOLDIER3_ID].pieceType = PIECE_SOLDIER;
	gamePieces[COM_SOLDIER3_ID].attack = SOLDIER_ATTACK;
	gamePieces[COM_SOLDIER3_ID].defense = SOLDIER_DEFENSE;
	gamePieces[COM_SOLDIER3_ID].movPow = SOLDIER_MOVPOWER;
	gamePieces[COM_SOLDIER3_ID].status = ALIVE_MASK | MOVED_MASK | COM_PIECE_MASK;

	pieceMap[10][2] = COM_SOLDIER4_ID;
	gamePieces[COM_SOLDIER4_ID].x = 10;
	gamePieces[COM_SOLDIER4_ID].y = 2;
	gamePieces[COM_SOLDIER4_ID].hitPoint = SOLDIER_HITPOINT;
	gamePieces[COM_SOLDIER4_ID].pieceType = PIECE_SOLDIER;
	gamePieces[COM_SOLDIER4_ID].attack = SOLDIER_ATTACK;
	gamePieces[COM_SOLDIER4_ID].defense = SOLDIER_DEFENSE;
	gamePieces[COM_SOLDIER4_ID].movPow = SOLDIER_MOVPOWER;
	gamePieces[COM_SOLDIER4_ID].status = ALIVE_MASK | MOVED_MASK | COM_PIECE_MASK;

	pieceMap[7][1] = COM_ARCHER1_ID;
	gamePieces[COM_ARCHER1_ID].x = 7;
	gamePieces[COM_ARCHER1_ID].y = 1;
	gamePieces[COM_ARCHER1_ID].hitPoint = ARCHER_HITPOINT;
	gamePieces[COM_ARCHER1_ID].pieceType = PIECE_ARCHER;
	gamePieces[COM_ARCHER1_ID].attack = ARCHER_ATTACK;
	gamePieces[COM_ARCHER1_ID].defense = ARCHER_DEFENSE;
	gamePieces[COM_ARCHER1_ID].movPow = ARCHER_MOVPOWER;
	gamePieces[COM_ARCHER1_ID].status = ALIVE_MASK | MOVED_MASK | COM_PIECE_MASK;

	pieceMap[9][1] = COM_ARCHER2_ID;
	gamePieces[COM_ARCHER2_ID].x = 9;
	gamePieces[COM_ARCHER2_ID].y = 1;
	gamePieces[COM_ARCHER2_ID].hitPoint = ARCHER_HITPOINT;
	gamePieces[COM_ARCHER2_ID].pieceType = PIECE_ARCHER;
	gamePieces[COM_ARCHER2_ID].attack = ARCHER_ATTACK;
	gamePieces[COM_ARCHER2_ID].defense = ARCHER_DEFENSE;
	gamePieces[COM_ARCHER2_ID].movPow = ARCHER_MOVPOWER;
	gamePieces[COM_ARCHER2_ID].status = ALIVE_MASK | MOVED_MASK | COM_PIECE_MASK;

	// プレイヤーの駒
	pieceMap[8][15] = PLAYER_CASTLE_ID;
	gamePieces[PLAYER_CASTLE_ID].x = 8;
	gamePieces[PLAYER_CASTLE_ID].y = 15;
	gamePieces[PLAYER_CASTLE_ID].hitPoint = CASTLE_HITPOINT;
	gamePieces[PLAYER_CASTLE_ID].pieceType = PIECE_CASTLE;
	gamePieces[PLAYER_CASTLE_ID].attack = CASTLE_ATTACK;
	gamePieces[PLAYER_CASTLE_ID].defense = CASTLE_DEFENSE;
	gamePieces[PLAYER_CASTLE_ID].movPow = CASTLE_MOVPOWER;
	gamePieces[PLAYER_CASTLE_ID].status = ALIVE_MASK;

	pieceMap[8][14] = PLAYER_KNIGHT_ID;
	gamePieces[PLAYER_KNIGHT_ID].x = 8;
	gamePieces[PLAYER_KNIGHT_ID].y = 14;
	gamePieces[PLAYER_KNIGHT_ID].hitPoint = KNIGHT_HITPOINT;
	gamePieces[PLAYER_KNIGHT_ID].pieceType = PIECE_KNIGHT;
	gamePieces[PLAYER_KNIGHT_ID].attack = KNIGHT_ATTACK;
	gamePieces[PLAYER_KNIGHT_ID].defense = KNIGHT_DEFENSE;
	gamePieces[PLAYER_KNIGHT_ID].movPow = KNIGHT_MOVPOWER;
	gamePieces[PLAYER_KNIGHT_ID].status = ALIVE_MASK;

	pieceMap[6][14] = PLAYER_SOLDIER1_ID;
	gamePieces[PLAYER_SOLDIER1_ID].x = 6;
	gamePieces[PLAYER_SOLDIER1_ID].y = 14;
	gamePieces[PLAYER_SOLDIER1_ID].hitPoint = SOLDIER_HITPOINT;
	gamePieces[PLAYER_SOLDIER1_ID].pieceType = PIECE_SOLDIER;
	gamePieces[PLAYER_SOLDIER1_ID].attack = SOLDIER_ATTACK;
	gamePieces[PLAYER_SOLDIER1_ID].defense = SOLDIER_DEFENSE;
	gamePieces[PLAYER_SOLDIER1_ID].movPow = SOLDIER_MOVPOWER;
	gamePieces[PLAYER_SOLDIER1_ID].status = ALIVE_MASK;

	pieceMap[7][14] = PLAYER_SOLDIER2_ID;
	gamePieces[PLAYER_SOLDIER2_ID].x = 7;
	gamePieces[PLAYER_SOLDIER2_ID].y = 14;
	gamePieces[PLAYER_SOLDIER2_ID].hitPoint = SOLDIER_HITPOINT;
	gamePieces[PLAYER_SOLDIER2_ID].pieceType = PIECE_SOLDIER;
	gamePieces[PLAYER_SOLDIER2_ID].attack = SOLDIER_ATTACK;
	gamePieces[PLAYER_SOLDIER2_ID].defense = SOLDIER_DEFENSE;
	gamePieces[PLAYER_SOLDIER2_ID].movPow = SOLDIER_MOVPOWER;
	gamePieces[PLAYER_SOLDIER2_ID].status = ALIVE_MASK;

	pieceMap[9][14] = PLAYER_SOLDIER3_ID;
	gamePieces[PLAYER_SOLDIER3_ID].x = 9;
	gamePieces[PLAYER_SOLDIER3_ID].y = 14;
	gamePieces[PLAYER_SOLDIER3_ID].hitPoint = SOLDIER_HITPOINT;
	gamePieces[PLAYER_SOLDIER3_ID].pieceType = PIECE_SOLDIER;
	gamePieces[PLAYER_SOLDIER3_ID].attack = SOLDIER_ATTACK;
	gamePieces[PLAYER_SOLDIER3_ID].defense = SOLDIER_DEFENSE;
	gamePieces[PLAYER_SOLDIER3_ID].movPow = SOLDIER_MOVPOWER;
	gamePieces[PLAYER_SOLDIER3_ID].status = ALIVE_MASK;

	pieceMap[10][14] = PLAYER_SOLDIER4_ID;
	gamePieces[PLAYER_SOLDIER4_ID].x = 10;
	gamePieces[PLAYER_SOLDIER4_ID].y = 14;
	gamePieces[PLAYER_SOLDIER4_ID].hitPoint = SOLDIER_HITPOINT;
	gamePieces[PLAYER_SOLDIER4_ID].pieceType = PIECE_SOLDIER;
	gamePieces[PLAYER_SOLDIER4_ID].attack = SOLDIER_ATTACK;
	gamePieces[PLAYER_SOLDIER4_ID].defense = SOLDIER_DEFENSE;
	gamePieces[PLAYER_SOLDIER4_ID].movPow = SOLDIER_MOVPOWER;
	gamePieces[PLAYER_SOLDIER4_ID].status = ALIVE_MASK;

	pieceMap[7][15] = PLAYER_ARCHER1_ID;
	gamePieces[PLAYER_ARCHER1_ID].x = 7;
	gamePieces[PLAYER_ARCHER1_ID].y = 15;
	gamePieces[PLAYER_ARCHER1_ID].hitPoint = ARCHER_HITPOINT;
	gamePieces[PLAYER_ARCHER1_ID].pieceType = PIECE_ARCHER;
	gamePieces[PLAYER_ARCHER1_ID].attack = ARCHER_ATTACK;
	gamePieces[PLAYER_ARCHER1_ID].defense = ARCHER_DEFENSE;
	gamePieces[PLAYER_ARCHER1_ID].movPow = ARCHER_MOVPOWER;
	gamePieces[PLAYER_ARCHER1_ID].status = ALIVE_MASK;

	pieceMap[9][15] = PLAYER_ARCHER2_ID;
	gamePieces[PLAYER_ARCHER2_ID].x = 9;
	gamePieces[PLAYER_ARCHER2_ID].y = 15;
	gamePieces[PLAYER_ARCHER2_ID].hitPoint = ARCHER_HITPOINT;
	gamePieces[PLAYER_ARCHER2_ID].pieceType = PIECE_ARCHER;
	gamePieces[PLAYER_ARCHER2_ID].attack = ARCHER_ATTACK;
	gamePieces[PLAYER_ARCHER2_ID].defense = ARCHER_DEFENSE;
	gamePieces[PLAYER_ARCHER2_ID].movPow = ARCHER_MOVPOWER;
	gamePieces[PLAYER_ARCHER2_ID].status = ALIVE_MASK;

}

//　地図の表示
void showMapWithPieces()
{

	int x, y;

	printf("   1 2 3 4 5 6 7 8 9 A B C D E F\n");
	for (y = 1; y < MAP_WIDTH - 1; y++)
	{

		if (y < 10)
			printf(" %d", y);
		else
			printf(" %c", 'A' + (y - 10));

		for (x = 1; x < MAP_LENGTH - 1; x++)
		{
			if (terrainMap[x][y] == SQ_BLOCK)
				showSquareInfo(x, y, COLOR_BLOCK);	// 移動不能のマス
			else if (terrainMap[x][y] == SQ_GRASS)
				showSquareInfo(x, y, COLOR_GRASS);	// 草地のマス
			else if (terrainMap[x][y] == SQ_SAND)
				showSquareInfo(x, y, COLOR_SAND);		// 砂地のマス
			else if (terrainMap[x][y] == SQ_ROCK)
				showSquareInfo(x, y, COLOR_ROCK);		// 岩場のマス
			else
				printf("  ");
		}

		printf("\n");
	}
}


//　地図の表示、可能な移動範囲を表示
void showMoveRange()
{

	int x, y;

	printf("   1 2 3 4 5 6 7 8 9 A B C D E F\n");
	for (y = 1; y < MAP_WIDTH - 1; y++)
	{

		if (y < 10)
			printf(" %d", y);
		else
			printf(" %c", 'A' + (y - 10));

		for (x = 1; x < MAP_LENGTH - 1; x++)
		{
			if (moveRange[x][y] > 0 && moveRange[x][y] < 99)
				showSquareInfo(x, y, COLOR_RANGE);
			else if (terrainMap[x][y] == SQ_BLOCK)
				showSquareInfo(x, y, COLOR_BLOCK);
			else if (terrainMap[x][y] == SQ_GRASS)
				showSquareInfo(x, y, COLOR_GRASS);
			else if (terrainMap[x][y] == SQ_SAND)
				showSquareInfo(x, y, COLOR_SAND);
			else if (terrainMap[x][y] == SQ_ROCK)
				showSquareInfo(x, y, COLOR_ROCK);
			else
				printf("  ");
		}

		printf("\n");
	}
}


//　マスの表示
void showSquareInfo(int x, int y, int color)
{
	HANDLE  hConsole;

	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);		// マスの色のハンドル

	SetConsoleTextAttribute(hConsole, COLOR_RESET);	// マスの色初期化

	if (pieceMap[x][y] == 0) // 空きマス
	{
		SetConsoleTextAttribute(hConsole, color);		// マスの文字の色は黒、背景の色はcolor
		printf("  ");		// 空きマスので文字がない
	}
	else
	{
		if (gamePieces[pieceMap[x][y]].status & ALIVE_MASK)
		{
			// このマスに生きている駒があるので表示
			switch (gamePieces[pieceMap[x][y]].pieceType)
			{
			case PIECE_CASTLE:
				if (gamePieces[pieceMap[x][y]].status & MOVED_MASK)
				{
					// 動かした駒ので色は別
					SetConsoleTextAttribute(hConsole, MOVED_COLOR + color);	// マスの文字の色はMOVED_COLOR、背景の色はcolor
					if (gamePieces[pieceMap[x][y]].status & COM_PIECE_MASK)
						printf(" c");		// COMの駒は小文字
					else
						printf(" C");		// プレイヤーの駒は大文字
				}
				else
				{
					SetConsoleTextAttribute(hConsole, color);		// マスの文字の色は黒、背景の色はcolor
					if (gamePieces[pieceMap[x][y]].status & COM_PIECE_MASK)
						printf(" c");		// COMの駒は小文字
					else
						printf(" C");		// プレイヤーの駒は大文字
				}
				break;
			case PIECE_ARCHER:
				if (gamePieces[pieceMap[x][y]].status & MOVED_MASK)
				{
					// 動かした駒ので色は別
					SetConsoleTextAttribute(hConsole, MOVED_COLOR + color);	// マスの文字の色はMOVED_COLOR、背景の色はcolor
					if (gamePieces[pieceMap[x][y]].status & COM_PIECE_MASK)
						printf(" a");		// COMの駒は小文字
					else
						printf(" A");		// プレイヤーの駒は大文字
				}
				else
				{
					SetConsoleTextAttribute(hConsole, color);		// マスの文字の色は黒、背景の色はcolor
					if (gamePieces[pieceMap[x][y]].status & COM_PIECE_MASK)
						printf(" a");		// COMの駒は小文字
					else
						printf(" A");		// プレイヤーの駒は大文字
				}
				break;
			case PIECE_SOLDIER:
				if (gamePieces[pieceMap[x][y]].status & MOVED_MASK)
				{
					// 動かした駒ので色は別
					SetConsoleTextAttribute(hConsole, MOVED_COLOR + color);	// マスの文字の色はMOVED_COLOR、背景の色はcolor
					if (gamePieces[pieceMap[x][y]].status & COM_PIECE_MASK)
						printf(" s");		// COMの駒は小文字
					else
						printf(" S");		// プレイヤーの駒は大文字
				}
				else
				{
					SetConsoleTextAttribute(hConsole, color);		// マスの文字の色は黒、背景の色はcolor
					if (gamePieces[pieceMap[x][y]].status & COM_PIECE_MASK)
						printf(" s");		// COMの駒は小文字
					else
						printf(" S");		// プレイヤーの駒は大文字
				}
				break;
			case PIECE_KNIGHT:
				if (gamePieces[pieceMap[x][y]].status & MOVED_MASK)
				{
					// 動かした駒ので色は別
					SetConsoleTextAttribute(hConsole, MOVED_COLOR + color);	// マスの文字の色はMOVED_COLOR、背景の色はcolor
					if (gamePieces[pieceMap[x][y]].status & COM_PIECE_MASK)
						printf(" k");		// COMの駒は小文字
					else
						printf(" K");		// プレイヤーの駒は大文字
				}
				else
				{
					SetConsoleTextAttribute(hConsole, color);		// マスの文字の色は黒、背景の色はcolor
					if (gamePieces[pieceMap[x][y]].status & COM_PIECE_MASK)
						printf(" k");		// COMの駒は小文字
					else
						printf(" K");		// プレイヤーの駒は大文字
				}
				break;
			default:
				printf("  ");
				break;
			}
		}
	}
	SetConsoleTextAttribute(hConsole, COLOR_RESET);	// マスの色初期化
}

// すべての駒の情報を表示
void showPieceStatus()
{
	char x, y;

	for (int pieceId = 1; pieceId <= TOTAL_PIECE_NUMBER; pieceId++)
	{
		if (gamePieces[pieceId].status & ALIVE_MASK)
		{

			if (gamePieces[pieceId].x < 10)
				x = '0' + gamePieces[pieceId].x;
			else
				x = 'A' + (gamePieces[pieceId].x - 10);

			if (gamePieces[pieceId].y < 10)
				y = '0' + gamePieces[pieceId].y;
			else
				y = 'A' + (gamePieces[pieceId].y - 10);

			if (gamePieces[pieceId].status & COM_PIECE_MASK)
			{
				switch (gamePieces[pieceId].pieceType)
				{
				case PIECE_CASTLE:
					printf(" c %c%c HP:%d, At:%d, Df:%d, Mp:%d\n",
						x, y, gamePieces[pieceId].hitPoint,
						gamePieces[pieceId].attack, gamePieces[pieceId].defense, gamePieces[pieceId].movPow);
					break;
				case PIECE_ARCHER:
					printf(" a %c%c HP:%d, At:%d, Df:%d, Mp:%d\n",
						x, y, gamePieces[pieceId].hitPoint,
						gamePieces[pieceId].attack, gamePieces[pieceId].defense, gamePieces[pieceId].movPow);
					break;
				case PIECE_KNIGHT:
					printf(" k %c%c HP:%d, At:%d, Df:%d, Mp:%d\n",
						x, y, gamePieces[pieceId].hitPoint,
						gamePieces[pieceId].attack, gamePieces[pieceId].defense, gamePieces[pieceId].movPow);
					break;
				case PIECE_SOLDIER:
					printf(" s %c%c HP:%d, At:%d, Df:%d, Mp:%d\n",
						x, y, gamePieces[pieceId].hitPoint,
						gamePieces[pieceId].attack, gamePieces[pieceId].defense, gamePieces[pieceId].movPow);
					break;
				default:
					break;
				}
			}
			else
			{
				switch (gamePieces[pieceId].pieceType)
				{
				case PIECE_CASTLE:
					printf(" C %c%c HP:%d, At:%d, Df:%d, Mp:%d\n",
						x, y, gamePieces[pieceId].hitPoint,
						gamePieces[pieceId].attack, gamePieces[pieceId].defense, gamePieces[pieceId].movPow);
					break;
				case PIECE_ARCHER:
					printf(" A %c%c HP:%d, At:%d, Df:%d, Mp:%d\n",
						x, y, gamePieces[pieceId].hitPoint,
						gamePieces[pieceId].attack, gamePieces[pieceId].defense, gamePieces[pieceId].movPow);
					break;
				case PIECE_KNIGHT:
					printf(" K %c%c HP:%d, At:%d, Df:%d, Mp:%d\n",
						x, y, gamePieces[pieceId].hitPoint,
						gamePieces[pieceId].attack, gamePieces[pieceId].defense, gamePieces[pieceId].movPow);
					break;
				case PIECE_SOLDIER:
					printf(" S %c%c HP:%d, At:%d, Df:%d, Mp:%d\n",
						x, y, gamePieces[pieceId].hitPoint,
						gamePieces[pieceId].attack, gamePieces[pieceId].defense, gamePieces[pieceId].movPow);
					break;
				default:
					break;
				}
			}
		}
	}

}

// 座標を表示
void displayCoordinates(int x, int y)
{
	char cx, cy;

	if (x < 10)
		cx = '0' + x;
	else
		cx = 'A' + (x - 10);

	if (y < 10)
		cy = '0' + y;
	else
		cy = 'A' + (y - 10);

	printf("%c%c", cx, cy);
}

// プレイヤーの駒移動処理
void selectAndMovePlayerPiece()
{

	int x, y, dx, dy, pieceId;

	getPlayerPiece(&x, &y);		// プレイヤーが動かしたい駒の場所を獲得

	pieceId = pieceMap[x][y];

	if (gamePieces[pieceId].status & MOVED_MASK)
	{
		printf("You already moved this piece.\n");	//	既に動いた駒
	}
	else
	{
		// この駒の移動範囲を計算と表示
		fillMap(moveRange, 0);
		findMoveRange(x, y, &gamePieces[pieceId], PLAYER);
		showMoveRange();

		getPlayerDestination(pieceId, &dx, &dy);		// プレイヤーから移動先の場所を獲得

		// 駒の移動関係情報を更新
		pieceMap[dx][dy] = pieceMap[x][y];
		gamePieces[pieceId].x = dx;
		gamePieces[pieceId].y = dy;
		gamePieces[pieceId].status = gamePieces[pieceId].status | MOVED_MASK;
		if (x != dx || y != dy)
			pieceMap[x][y] = 0;
		resolveBattle(&gamePieces[pieceId], PLAYER);	// 戦う相手を選択してからバトル

		if (gameStatus != PLAYER_WINS) // バトルの結果でゲームオーバーになってない場合
		{
			piecesToMove--;
			if (piecesToMove == 0)
			{
				nextToCastleRecovery(COMPUTER);		// 城の隣の駒を回復

				// プレイヤーはすべての駒を動かしたのでCOMの手番に移る
				gameStatus = COM_TO_MOVE;
				piecesToMove = comAlivePieceNo;

				// COMの駒を「動いていない」状況に設定
				for (pieceId = 1; pieceId <= TOTAL_PIECE_NUMBER; pieceId++)
				{
					if (gamePieces[pieceId].status & COM_PIECE_MASK)
						gamePieces[pieceId].status &= ~MOVED_MASK;
				}

			}

		}
	}
}

// プレイヤーが動かしたい駒の場所を確保
void getPlayerPiece(int *x, int *y)
{

	int tempX, tempY, legalX, legalY, legalPiece;
	int i;
	char c;
	char buf[256];

	legalX = legalY = legalPiece = 0;
	tempX = tempY = 0;
	// 正しい駒を獲得できるまでユーザに情報を要求
	while (!legalPiece)
	{
		// 正しいxまで繰り返し
		while (!legalX)
		{
			printf("Enter x coordinate of piece (1-F): ");
			c = getchar();
			for (i = 0; c != '\n'; i++)
			{
				buf[i] = c;
				c = getchar();
			}
			buf[i] = '\0';
			if (buf[0] >= '1' && buf[0] <= '9')
			{
				tempX = (buf[0] - '1') + 1;
				legalX = 1;
			}
			else if (buf[0] >= 'A' && buf[0] <= 'F')
			{
				tempX = (buf[0] - 'A') + 10;
				legalX = 1;
			}
			else
			{
				printf("The x-coordinate should be between 1 and F.\n");
				tempX = 0;
			}
		}
		// 正しいyまで繰り返し
		while (!legalY)
		{
			printf("Enter y coordinate of piece (1-F): ");
			c = getchar();
			for (i = 0; c != '\n'; i++)
			{
				buf[i] = c;
				c = getchar();
			}
			buf[i] = '\0';
			if (buf[0] >= '1' && buf[0] <= '9')
			{
				tempY = (buf[0] - '1') + 1;
				legalY = 1;
			}
			else if (buf[0] >= 'A' && buf[0] <= 'F')
			{
				tempY = (buf[0] - 'A') + 10;
				legalY = 1;
			}
			else
			{
				printf("The y-coordinate should be between 1 and F.\n");
				tempY = 0;
			}
		}
		if (pieceMap[tempX][tempY] == 0)
		{
			// 選んだマスに駒がない
			printf("No piece on this square. Please try again.\n");
			legalX = 0;
			legalY = 0;
			tempX = 0;
			tempY = 0;
		}
		else if (gamePieces[pieceMap[tempX][tempY]].pieceType == PIECE_CASTLE)
		{
			// 城は動けない
			printf("Cannot move castle. Please try again.\n");
			legalX = 0;
			legalY = 0;
			tempX = 0;
			tempY = 0;
		}
		else if (gamePieces[pieceMap[tempX][tempY]].status & COM_PIECE_MASK)
		{
			// COMの駒は選べない
			printf("This is not your piece. Please try again.\n");
			legalX = 0;
			legalY = 0;
			tempX = 0;
			tempY = 0;
		}
		else if (gamePieces[pieceMap[tempX][tempY]].status & MOVED_MASK)
		{
			// 既に動いた駒は選べない
			printf("You already moved this piece. Please try again.\n");
			legalX = 0;
			legalY = 0;
			tempX = 0;
			tempY = 0;
		}
		else
		{
			// 正しい駒
			legalPiece = 1;
			*x = tempX;
			*y = tempY;
		}
	}

}

// 駒の移動先を確保
void getPlayerDestination(int pieceId, int *x, int *y)
{

	int tempX, tempY, legalX, legalY, legalMove;
	int i;
	char c;
	char buf[256];

	legalX = legalY = legalMove = 0;
	tempX = tempY = 0;

	// 正しい移動先まで繰り返し
	while (!legalMove)
	{
		// 正しいxまで繰り返し
		while (!legalX)
		{
			printf("Enter x coordinate of destination square (1-F): ");
			c = getchar();
			for (i = 0; c != '\n'; i++)
			{
				buf[i] = c;
				c = getchar();
			}
			buf[i] = '\0';
			if (buf[0] >= '1' && buf[0] <= '9')
			{
				tempX = (buf[0] - '1') + 1;
				legalX = 1;
			}
			else if (buf[0] >= 'A' && buf[0] <= 'F')
			{
				tempX = (buf[0] - 'A') + 10;
				legalX = 1;
			}
			else
			{
				printf("The x-coordinate should be between 1 and F.\n");
				tempX = 0;
			}
		}
		// 正しいyまで繰り返し
		while (!legalY)
		{
			printf("Enter y coordinate of destination square (1-F): ");
			c = getchar();
			for (i = 0; c != '\n'; i++)
			{
				buf[i] = c;
				c = getchar();
			}
			buf[i] = '\0';
			if (buf[0] >= '1' && buf[0] <= '9')
			{
				tempY = (buf[0] - '1') + 1;
				legalY = 1;
			}
			else if (buf[0] >= 'A' && buf[0] <= 'F')
			{
				tempY = (buf[0] - 'A') + 10;
				legalY = 1;
			}
			else
			{
				printf("The y-coordinate should be between 1 and F.\n");
				tempY = 0;
			}
		}
		if (moveRange[tempX][tempY] == 0)
		{
			// 選んだ移動先は移動範囲外
			printf("Cannot move piece to this square.\n");
			legalX = 0;
			legalY = 0;
			tempX = 0;
			tempY = 0;
		}
		else if (pieceMap[tempX][tempY] != 0 && pieceMap[tempX][tempY] != pieceId)
		{
			// 選んだ移動先に駒がある
			printf("There is already a piece on this square.\n");
			legalX = 0;
			legalY = 0;
			tempX = 0;
			tempY = 0;
		}
		else
		{
			// 正しい移動先
			legalMove = 1;
			*x = tempX;
			*y = tempY;
		}
	}
}

// 手番を逆する
int flip(int to_move)
{
	if (to_move == PLAYER)
		return COMPUTER;
	else
		return PLAYER;
}

// 駒の最高HPを返す関数
int getFullHitPointsForPiece(int piece)
{
	switch (piece)
	{
	case PIECE_CASTLE: return CASTLE_HITPOINT;
	case PIECE_KNIGHT: return KNIGHT_HITPOINT;
	case PIECE_SOLDIER: return SOLDIER_HITPOINT;
	case PIECE_ARCHER: return ARCHER_HITPOINT;
	default: return 0;
	}
}

// 城から四つ方向の相方駒は城からHPもらって、回復する
void nextToCastleRecovery(int side)
{
	int castleId, castleX, castleY, tempId, oldHp;

	if (side == COMPUTER)
		castleId = COM_CASTLE_ID;
	else
		castleId = PLAYER_CASTLE_ID;

	// 城のHPがWEAK_CASTLEを割っていたら回復しない
	if (gamePieces[castleId].hitPoint > WEAK_CASTLE)
	{

		castleX = gamePieces[castleId].x;
		castleY = gamePieces[castleId].y;

		// 駒を回復させてみて、もとのHPと変化していなかったら（回復済みだったら）城のHPは減らさない
		if (castleX > 1)
		{
			tempId = pieceMap[castleX - 1][castleY];
			if (tempId != 0 && (gamePieces[tempId].status & ALIVE_MASK) &&
				((side == COMPUTER && (gamePieces[tempId].status & COM_PIECE_MASK)) ||
				(side == PLAYER && !(gamePieces[tempId].status & COM_PIECE_MASK))))
			{
				oldHp = gamePieces[tempId].hitPoint;
				gamePieces[tempId].hitPoint = getFullHitPointsForPiece(gamePieces[tempId].pieceType);
				if (oldHp < gamePieces[tempId].hitPoint)
				{
					gamePieces[castleId].hitPoint -= (gamePieces[tempId].hitPoint - oldHp) / RECOVERY_RATE + 1;
					printf("Piece on ");
					displayCoordinates(castleX - 1, castleY);
					printf(" has recovered health. New castle HP: %d\n", gamePieces[castleId].hitPoint);
				}
			}
		}

		if (castleX < MAP_WIDTH - 2)
		{
			tempId = pieceMap[castleX + 1][castleY];
			if (tempId != 0 && (gamePieces[tempId].status & ALIVE_MASK) &&
				((side == COMPUTER && (gamePieces[tempId].status & COM_PIECE_MASK)) ||
				(side == PLAYER && !(gamePieces[tempId].status & COM_PIECE_MASK))))
			{
				oldHp = gamePieces[tempId].hitPoint;
				gamePieces[tempId].hitPoint = getFullHitPointsForPiece(gamePieces[tempId].pieceType);
				if (oldHp < gamePieces[tempId].hitPoint)
				{
					gamePieces[castleId].hitPoint -= (gamePieces[tempId].hitPoint - oldHp) / RECOVERY_RATE + 1;
					printf("Piece on ");
					displayCoordinates(castleX + 1, castleY);
					printf(" has recovered health. New castle HP: %d\n", gamePieces[castleId].hitPoint);
				}
			}
		}

		if (castleY > 1)
		{
			tempId = pieceMap[castleX][castleY - 1];
			if (tempId != 0 && (gamePieces[tempId].status & ALIVE_MASK) &&
				((side == COMPUTER && (gamePieces[tempId].status & COM_PIECE_MASK)) ||
				(side == PLAYER && !(gamePieces[tempId].status & COM_PIECE_MASK))))
			{
				oldHp = gamePieces[tempId].hitPoint;
				gamePieces[tempId].hitPoint = getFullHitPointsForPiece(gamePieces[tempId].pieceType);
				if (oldHp < gamePieces[tempId].hitPoint)
				{
					gamePieces[castleId].hitPoint -= (gamePieces[tempId].hitPoint - oldHp) / RECOVERY_RATE + 1;
					printf("Piece on ");
					displayCoordinates(castleX, castleY - 1);
					printf(" has recovered health. New castle HP: %d\n", gamePieces[castleId].hitPoint);
				}
			}
		}

		if (castleY < MAP_LENGTH - 2)
		{
			tempId = pieceMap[castleX][castleY + 1];
			if (tempId != 0 && (gamePieces[tempId].status & ALIVE_MASK) &&
				((side == COMPUTER && (gamePieces[tempId].status & COM_PIECE_MASK)) ||
				(side == PLAYER && !(gamePieces[tempId].status & COM_PIECE_MASK))))
			{
				oldHp = gamePieces[tempId].hitPoint;
				gamePieces[tempId].hitPoint = getFullHitPointsForPiece(gamePieces[tempId].pieceType);
				if (oldHp < gamePieces[tempId].hitPoint)
				{
					gamePieces[castleId].hitPoint -= (gamePieces[tempId].hitPoint - oldHp) / RECOVERY_RATE + 1;
					printf("Piece on ");
					displayCoordinates(castleX, castleY + 1);
					printf(" has recovered health. New castle HP: %d\n", gamePieces[castleId].hitPoint);
				}
			}
		}

	}

}

// 戦う相手を選択してからバトル。城を崩壊、相手の駒を全部破ったらゲームオーバー
void resolveBattle(PIECE *piece, int side)
{
	int oppId;

	oppId = selectOpponent(piece, side);		// 戦う相手を選択

	if (oppId != 0)
	{
		fightBattle(piece, oppId);				// バトル

		// 城を崩壊、相手の駒を全部破ったらゲームオーバー
		if (playerAlivePieceNo == 0 || playerCastleDestroyed)
			gameStatus = COM_WINS;
		else if (comAlivePieceNo == 0 || comCastleDestroyed)
			gameStatus = PLAYER_WINS;
	}

}

// 戦う相手を選択
int selectOpponent(PIECE *piece, int side)
{
	int myX = piece->x;
	int myY = piece->y;
	int oppId[4];
	int oppNo = 0;
	int legalId;
	char c, buf[80];
	int i, tempId;

	if (side == PLAYER)
	{
		// 手番はプレイヤーので敵を選択してもらう
		if (myX < MAP_WIDTH - 2)
		{
			// 右隣に敵を確認
			oppId[oppNo] = pieceMap[myX + 1][myY];
			if (oppId[oppNo] && gamePieces[oppId[oppNo]].status & COM_PIECE_MASK)
			{
				// 敵候補の情報を表示
				printf("Battle opponent %d at ", oppNo + 1);
				displayCoordinates(myX + 1, myY);
				printf(", MyHP: %d, OppHP: %d, Attack: %d, Defense: %d\n",
					piece->hitPoint, gamePieces[oppId[oppNo]].hitPoint, gamePieces[oppId[oppNo]].attack, gamePieces[oppId[oppNo]].defense);
				oppNo++;
			}
		}
		if (myX > 1)
		{
			// 左隣に敵を確認
			oppId[oppNo] = pieceMap[myX - 1][myY];
			if (oppId[oppNo] && gamePieces[oppId[oppNo]].status & COM_PIECE_MASK)
			{
				// 敵候補の情報を表示
				printf("Battle opponent %d at ", oppNo + 1);
				displayCoordinates(myX - 1, myY);
				printf(", MyHP: %d, OppHP: %d, Attack: %d, Defense: %d\n",
					piece->hitPoint, gamePieces[oppId[oppNo]].hitPoint, gamePieces[oppId[oppNo]].attack, gamePieces[oppId[oppNo]].defense);
				oppNo++;
			}
		}
		if (myY < MAP_LENGTH - 2)
		{
			// 下隣に敵を確認
			oppId[oppNo] = pieceMap[myX][myY + 1];
			if (oppId[oppNo] && gamePieces[oppId[oppNo]].status & COM_PIECE_MASK)
			{
				// 敵候補の情報を表示
				printf("Battle opponent %d at ", oppNo + 1);
				displayCoordinates(myX, myY + 1);
				printf(", MyHP: %d, OppHP: %d, Attack: %d, Defense: %d\n",
					piece->hitPoint, gamePieces[oppId[oppNo]].hitPoint, gamePieces[oppId[oppNo]].attack, gamePieces[oppId[oppNo]].defense);
				oppNo++;
			}
		}
		if (myY > 1)
		{
			// 下隣に敵を確認
			oppId[oppNo] = pieceMap[myX][myY - 1];
			if (oppId[oppNo] && gamePieces[oppId[oppNo]].status & COM_PIECE_MASK)
			{
				// 敵候補の情報を表示
				printf("Battle opponent %d at ", oppNo + 1);
				displayCoordinates(myX, myY - 1);
				printf(", MyHP: %d, OppHP: %d, Attack: %d, Defense: %d\n",
					piece->hitPoint, gamePieces[oppId[oppNo]].hitPoint, gamePieces[oppId[oppNo]].attack, gamePieces[oppId[oppNo]].defense);
				oppNo++;
			}
		}
		if (oppNo != 0)
		{
			// プレイヤーに敵を選んでもらう
			legalId = 0;
			while (!legalId)
			{
				printf("Select battle opponent number: ");
				c = getchar();
				for (i = 0; c != '\n'; i++)
				{
					buf[i] = c;
					c = getchar();
				}
				buf[i] = '\0';
				if (buf[0] >= '1' && buf[0] <= '9')
				{
					tempId = (buf[0] - '1') + 1;
					if (tempId <= oppNo)
						return oppId[tempId - 1];
					else
						printf("Incorrect battle opponent number, please try again\n");
				}
				else
					printf("Incorrect battle opponent number, please try again\n");
			}
		}
	}
	else
	{
		// 手番はCOMのでこの部分もAIになれるところがあるが、現在は一番体力が低い相手を敵にする
		if (myX < MAP_WIDTH - 2)
		{
			// 右隣に敵を確認
			oppId[oppNo] = pieceMap[myX + 1][myY];
			if (oppId[oppNo] && !(gamePieces[oppId[oppNo]].status & COM_PIECE_MASK))
				oppNo++;
		}
		if (myX > 1)
		{
			// 左隣に敵を確認
			oppId[oppNo] = pieceMap[myX - 1][myY];
			if (oppId[oppNo] && !(gamePieces[oppId[oppNo]].status & COM_PIECE_MASK))
				oppNo++;
		}
		if (myY < MAP_LENGTH - 2)
		{
			// 下隣に敵を確認
			oppId[oppNo] = pieceMap[myX][myY + 1];
			if (oppId[oppNo] && !(gamePieces[oppId[oppNo]].status & COM_PIECE_MASK))
				oppNo++;
		}
		if (myY > 1)
		{
			// 下隣に敵を確認
			oppId[oppNo] = pieceMap[myX][myY - 1];
			if (oppId[oppNo] && !(gamePieces[oppId[oppNo]].status & COM_PIECE_MASK))
				oppNo++;
		}
		if (oppNo != 0)
		{
			// 体力が低い敵を選ぶ
			tempId = oppId[0];
			for (i = 1; i < oppNo; i++)
			{
				if (gamePieces[oppId[i]].hitPoint < gamePieces[tempId].hitPoint)
					tempId = oppId[i];
			}
			return tempId;

		}
	}

	return 0;
}

// バトル
void fightBattle(PIECE *piece, int oppId)
{
	printf("Starting battle between ");
	displayCoordinates(piece->x, piece->y);
	printf(" and ");
	displayCoordinates(gamePieces[oppId].x, gamePieces[oppId].y);
	printf("...\n");

	// 攻撃側の先制攻撃
	gamePieces[oppId].hitPoint -= updateBattleHitPoints(piece->attack, gamePieces[oppId].defense,
		pieceCompatibility[piece->pieceType][gamePieces[oppId].pieceType], groundPower[terrainMap[gamePieces[oppId].x][gamePieces[oppId].y]]);

	printf("Attacked piece hitpoints after attack: %d\n", gamePieces[oppId].hitPoint);

	if (gamePieces[oppId].hitPoint <= 0)
	{
		// 攻撃された駒が死んだ
		printf("Attacked piece died in battle\n");
		if (gamePieces[oppId].pieceType == PIECE_CASTLE)
		{
			if (gamePieces[oppId].status & COM_PIECE_MASK)
				comCastleDestroyed = 1;
			else
				playerCastleDestroyed = 1;
		}
		else
		{
			if (gamePieces[oppId].status & COM_PIECE_MASK)
				comAlivePieceNo--;
			else
				playerAlivePieceNo--;
		}
		pieceMap[gamePieces[oppId].x][gamePieces[oppId].y] = 0;
		gamePieces[oppId].status &= ~ALIVE_MASK;
	}
	else
	{
		//死ななかったら反撃（反撃は攻撃の半分）
		piece->hitPoint -= updateBattleHitPoints(gamePieces[oppId].attack / 2, piece->defense,
			pieceCompatibility[piece->pieceType][gamePieces[oppId].pieceType], groundPower[terrainMap[piece->x][piece->y]]);

		printf("Attacking piece hitpoints after counter attack: %d\n", piece->hitPoint);

		if (piece->hitPoint <= 0)
		{
			// 攻撃した駒が死んだ（城は攻撃しないので関係ない）
			printf("Attacking piece died in battle\n");
			if (piece->status & COM_PIECE_MASK)
				comAlivePieceNo--;
			else
				playerAlivePieceNo--;
			pieceMap[piece->x][piece->y] = 0;
			piece->status &= ~ALIVE_MASK;
		}
	}
}

// バトルのときにHPを更新計算
int updateBattleHitPoints(int attackerAtPow, int defenderDefPow, int attackerDefenderCmp, int defenderGroundPower)
{
	int	atPow, defPow;

	//攻撃力・防御力の算出
	attackerAtPow += ((rand() % 10) - 5);			//ばらつきを持たせる
	atPow = attackerAtPow + (attackerAtPow * attackerDefenderCmp) / 100;	//％なので100で割る
	defPow = defenderDefPow + (defenderDefPow * defenderGroundPower) / 100;

	switch (rand() % 20)
	{
	case 0:		//ファンブル
		atPow /= 2;
		break;

	case 1:		//クリティカル
		atPow *= 2;
		break;
	}

	atPow += (atPow - defPow);
	if (atPow < 0){ atPow = 0; }						//ゼロ以下なら０に

	return atPow;
}

//　与えられた位置からの移動可能範囲をg_range配列内に収める
void findMoveRange(int startX, int startY, const PIECE *aPiece, int turnSide)
{
	moveRange[startX][startY] = aPiece->movPow;

	excludePiece(flip(turnSide), 99);	// 敵方の駒を迂回させるため、マップ移動最大値を詰め込む
	checkRange(startX, startY, aPiece->movPow, aPiece->pieceType, moveRange);
	excludePiece(flip(turnSide), 0);	// 敵方の駒の位置を移動可能範囲から除外する

}

//　選択範囲から駒を除外する
void excludePiece(int turnSide, int fillInt)
{
	int	i0;

	for (i0 = 1; i0 <= TOTAL_PIECE_NUMBER; i0++)
	{
		if (((gamePieces[i0].status & COM_PIECE_MASK) == turnSide))
		{
			moveRange[gamePieces[i0].x][gamePieces[i0].y] = fillInt;
		}
	}
}

//　再帰法で移動可能範囲をチェックする。配列の添え字と関数の引数が逆になっていることに要注意。
void checkRange(int startX, int startY, int leftPow, int pieceType, int aMap[][MAP_WIDTH])
{
	int	i0;

	aMap[startX][startY] = leftPow;	/*残り移動力*/

	i0 = leftPow - moveCost[pieceType][terrainMap[startX][startY - 1]];/*上*/
	if (aMap[startX][startY - 1] < i0)
	{
		checkRange(startX, startY - 1, i0, pieceType, aMap);
	}

	i0 = leftPow - moveCost[pieceType][terrainMap[startX][startY + 1]];/*下*/
	if (aMap[startX][startY + 1] < i0)
	{
		checkRange(startX, startY + 1, i0, pieceType, aMap);
	}

	i0 = leftPow - moveCost[pieceType][terrainMap[startX - 1][startY]];/*右*/
	if (aMap[startX - 1][startY] < i0)
	{
		checkRange(startX - 1, startY, i0, pieceType, aMap);
	}

	i0 = leftPow - moveCost[pieceType][terrainMap[startX + 1][startY]];/*左*/
	if (aMap[startX + 1][startY] < i0)
	{
		checkRange(startX + 1, startY, i0, pieceType, aMap);
	}
}

//　再帰法で目標地点からの所要歩数を調べて地図配列に収める
void checkDistance(int startX, int startY, int cnsPow, int pieceType, int aMap[][MAP_WIDTH])
{
	int	i0;

	aMap[startX][startY] = cnsPow;	/*ここまで費やした移動コストを書き込む*/

	i0 = cnsPow + moveCost[pieceType][terrainMap[startX][startY - 1]];/*上*/
	if (aMap[startX][startY - 1] > i0)
	{
		checkDistance(startX, startY - 1, i0, pieceType, aMap);
	}

	i0 = cnsPow + moveCost[pieceType][terrainMap[startX][startY + 1]];/*下*/
	if (aMap[startX][startY + 1] > i0)
	{
		checkDistance(startX, startY + 1, i0, pieceType, aMap);
	}

	i0 = cnsPow + moveCost[pieceType][terrainMap[startX - 1][startY]];/*右*/
	if (aMap[startX - 1][startY] > i0)
	{
		checkDistance(startX - 1, startY, i0, pieceType, aMap);
	}

	i0 = cnsPow + moveCost[pieceType][terrainMap[startX + 1][startY]];/*左*/
	if (aMap[startX + 1][startY] > i0)
	{
		checkDistance(startX + 1, startY, i0, pieceType, aMap);
	}
}

//　選択範囲構造体の初期化
void fillMap(int aMap[][MAP_WIDTH], int fillInt)
{
	int	i0, j0, k0 = 0;

	for (i0 = 1; i0 < MAP_LENGTH - 1; i0++)
	{
		for (j0 = 1; j0 < MAP_WIDTH - 1; j0++)
		{
			aMap[i0][j0] = fillInt;
		}
	}
}

// COMの駒を動かす
void moveComPieces()
{
	int pieceId, sx, sy, dx, dy;

	printf("Starting computer turn...\n");

	for (pieceId = 1; pieceId <= TOTAL_PIECE_NUMBER; pieceId++)
	{

		if (gamePieces[pieceId].status & COM_PIECE_MASK && gamePieces[pieceId].pieceType != PIECE_CASTLE &&
			gamePieces[pieceId].status & ALIVE_MASK)
		{
			// この駒の移動範囲を計算
			fillMap(moveRange, MIN_DISTANCE);
			sx = gamePieces[pieceId].x;
			sy = gamePieces[pieceId].y;
			findMoveRange(sx, sy, &gamePieces[pieceId], COMPUTER);

			decideComPieceDestination(pieceId, &dx, &dy);		// ←これはAIの部分！

			// 駒の移動関係情報を更新
			pieceMap[dx][dy] = pieceMap[sx][sy];
			gamePieces[pieceId].x = dx;
			gamePieces[pieceId].y = dy;
			gamePieces[pieceId].status = gamePieces[pieceId].status | MOVED_MASK;
			if (sx != dx || sy != dy)
				pieceMap[sx][sy] = 0;
			resolveBattle(&gamePieces[pieceId], COMPUTER);	// 戦う相手を選択してからバトル
			if (gameStatus == COM_WINS)	// バトルの結果でゲームオーバーになった場合
				break;

			piecesToMove--;
			if (piecesToMove == 0)
				break;	// すべての駒を動かした
		}
	}

	if (gameStatus != COM_WINS)	// バトルの結果でゲームオーバーになっていない場合
	{
		printf("Computer turn finished. Your turn.\n");

		nextToCastleRecovery(PLAYER);		// 城の隣の駒を回復

		gameStatus = PLAYER_TO_MOVE;		// プレイヤーの手番
		piecesToMove = playerAlivePieceNo;	// 動かす駒の数

		// プレイヤーの駒を「動いていない」状況に設定
		for (pieceId = 1; pieceId <= TOTAL_PIECE_NUMBER; pieceId++)
		{
			if (!(gamePieces[pieceId].status & COM_PIECE_MASK))
				gamePieces[pieceId].status &= ~MOVED_MASK;
		}

	}

}

// COMの駒の移動先を決める
void decideComPieceDestination(int pieceId, int *dx, int *dy)
{
	int i0;
	int castleNum, nearDist;

	castleNum = COM_CASTLE_ID;
	nearDist = MAX_DISTANCE;

	//自分の城に一番近い敵の駒の距離
	//注：すべての駒にチェックする理由は途中で自分の城に近いプレイヤーの駒は攻め落としたこともありえるから
	for (i0 = 1; i0 <= TOTAL_PIECE_NUMBER; i0++)
	{
		if (!(gamePieces[i0].status & COM_PIECE_MASK) &&
			gamePieces[i0].status & ALIVE_MASK)
		{
			int dist;
			dist = abs(gamePieces[castleNum].x - gamePieces[i0].x) + abs(gamePieces[castleNum].y - gamePieces[i0].y);
			if (dist < nearDist)
			{
				nearDist = dist;
			}
		}
	}
	//
	// 残りヒットポイントで方針を決める
	//
	// コマが弱くなって、自分の城で回復できれば退却する
	if ((gamePieces[pieceId].hitPoint < WEAK_PIECE) && (gamePieces[castleNum].hitPoint > WEAK_CASTLE))
	{
		cpuAttackRetreat(pieceId, RETREAT_TO_CASTLE, dx, dy);
	}
	//自分の城がピンチだったら守る
	else if (nearDist < DANGER_DISTANCE)
	{
		cpuAttackRetreat(pieceId, RETREAT_TO_CASTLE, dx, dy);
	}
	// 敵の城を攻め落としに行く
	else
	{
		cpuAttackRetreat(pieceId, ATTACK_CASTLE, dx, dy);
	}
}

// 引数のAttackOrRetreatがATTACK_CASTLEであれば、駒をプレイヤーの城の方向に向かう
// 一方、引数のAttackOrRetreatがRETREAT_TO_CASTLEであれば、駒は自分の城に退却する
void cpuAttackRetreat(int pieceId, int AttackOrRetreat, int *resultX, int *resultY)
{
	int	advMap[MAP_WIDTH][MAP_LENGTH];			//有利度マップ
	int	i0, j0;										//カウンタとか
	int castleNum;									//敵の城のID、ターゲットになる駒のID
	int	tmpX, tmpY, tarX, tarY, minCost;

	//敵の城の背番号を得る
	//突撃するなら敵の城を、退却するなら自分の城を目標にする
	if (AttackOrRetreat == ATTACK_CASTLE)
		castleNum = PLAYER_CASTLE_ID;
	else if (AttackOrRetreat == RETREAT_TO_CASTLE)
		castleNum = COM_CASTLE_ID;

	tarX = gamePieces[castleNum].x;
	tarY = gamePieces[castleNum].y;

	//マップの初期化
	fillMap(tmpMap, MAX_DISTANCE);
	fillMap(moveRange, MIN_DISTANCE);

	//テンポラリを最大歩数で埋める。
	fillMap(advMap, MAX_DISTANCE);

	//移動コスト探索が配列の範囲を逸脱しないように、外周を0にしておく。
	for (i0 = 0; i0 < MAP_WIDTH; i0++)
	{
		advMap[i0][0] = MIN_DISTANCE;
		advMap[i0][MAP_LENGTH - 1] = MIN_DISTANCE;
	}
	for (i0 = 0; i0 < MAP_LENGTH; i0++)
	{
		advMap[0][i0] = MIN_DISTANCE;
		advMap[MAP_WIDTH - 1][i0] = MIN_DISTANCE;
	}

	//敵の城からの移動コストを有利度マップに書き込む
	checkDistance(tarX, tarY, MIN_DISTANCE, gamePieces[pieceId].pieceType, advMap);

	//敵の駒と自分のコマの位置情報を加味する
	addPieceInfoToMoveMap(pieceId, advMap);

	//後で最も低いコストの位置を調べるから、外周は最大値で埋めておく
	for (i0 = 0; i0 < MAP_WIDTH; i0++)
	{
		advMap[i0][0] = MAX_DISTANCE;
		advMap[i0][MAP_LENGTH - 1] = MAX_DISTANCE;
	}
	for (i0 = 0; i0 < MAP_LENGTH; i0++)
	{
		advMap[0][i0] = MAX_DISTANCE;
		advMap[MAP_WIDTH - 1][i0] = MAX_DISTANCE;
	}

	//移動可能範囲の探索
	findMoveRange(gamePieces[pieceId].x, gamePieces[pieceId].y, &gamePieces[pieceId], COMPUTER);

	//駒のある位置には移動できない
	for (i0 = 1; i0 <= TOTAL_PIECE_NUMBER; i0++)
	{
		if (gamePieces[i0].status & ALIVE_MASK)
		{
			tmpMap[gamePieces[i0].x][gamePieces[i0].y] = MIN_DISTANCE;
		}
	}

	//移動できない位置から自分自身の位置は除外する（周囲を囲まれたときの問題を回避）
	tmpMap[gamePieces[pieceId].x][gamePieces[pieceId].y] = MAX_DISTANCE;

	//最低コストに最大値を入れておく
	minCost = MAX_DISTANCE;
	tmpX = -1;
	tmpY = -1;

	//移動範囲の中で有利度が最も高い点（有利度マップ中のスコアが最も低い点）を探す
	for (i0 = 1; i0 < MAP_WIDTH - 1; i0++)
	{
		for (j0 = 1; j0 < MAP_LENGTH - 1; j0++)
		{
			if ((tmpMap[i0][j0] != MIN_DISTANCE) &&
				(moveRange[i0][j0] != MIN_DISTANCE) &&
				(minCost > advMap[i0][j0]))
			{
				minCost = advMap[i0][j0];
				tmpX = i0;
				tmpY = j0;
			}
		}
	}

	//結果を返す
	*resultX = tmpX;
	*resultY = tmpY;
}

// 自分とそれ以外の敵のコマのヒットポイントを比べて、自分のヒットポイントが多かったら、
// ヒットポイントの少ないコマを狙うように移動し、逆であれば、避けるように移動する
// また、CPU側の他のコマと連携するように移動する
// 自分と敵のコマの情報を利用して、行くべきところのaMapにある数字を低くして、
// 行かない方が良いところのaMapにある数字を高くする
void addPieceInfoToMoveMap(int pieceId, int aMap[][MAP_WIDTH])
{
	int i0;

	for (i0 = 1; i0 <= TOTAL_PIECE_NUMBER; i0++)
	{
		//駒は生きてる？
		if (gamePieces[i0].status & ALIVE_MASK)
		{
			int	oppInfoBonus = 0;			//敵のコマを狙うか避けるかのための変数
			int moveTogetherBonus = 0;		//CPU側の他のコマと連携で移動のための変数

			int	myHP, oppHP;
			int	tmpX, tmpY;

			//ヒットポイントを取り込む
			myHP = gamePieces[pieceId].hitPoint;
			oppHP = gamePieces[i0].hitPoint;
			//駒の位置を取り込む
			tmpX = gamePieces[i0].x;
			tmpY = gamePieces[i0].y;

			// プレイヤーの駒？
			if (!(gamePieces[i0].status & COM_PIECE_MASK))
			{
				if (gamePieces[i0].pieceType != PIECE_CASTLE)
				{
					//自分と相手のコマのヒットポイントを比較して、自分が有利ならばボーナスを高く、
					//相手が有利あならばボーナスを低く設定
					if (myHP > oppHP * 2)
						oppInfoBonus = HOT_SPOT_BONUS;	// 弱い敵を狙う
					else if (myHP > oppHP)
						oppInfoBonus = SOFT_SPOT_BONUS;	// 少し弱い敵を狙う
					else if (myHP * 2 < oppHP)
						oppInfoBonus = VERY_BAD_SPOT_BONUS;	// 強い敵から逃げる
					else if (myHP < oppHP)
						oppInfoBonus = BAD_SPOT_BONUS;		// 少し強い敵ならば逃げて良いかも
					else
						oppInfoBonus = EVEN_SPOT_BONUS;		// 同じHPの敵は行っても良い
				}
				else
					oppInfoBonus = MOVE_OPP_CASTLE_BONUS;	//敵の城へ行こう！
			}
			else if (gamePieces[i0].pieceType == PIECE_CASTLE)
				oppInfoBonus = MOVE_OWN_CASTLE_BONUS;		//自分の城の方向に移動するも悪くない
			else
				moveTogetherBonus = JOIN_FORCES_BONUS;		//自分のコマと連携して移動も悪くない

			//有利度をマップから減算する（スコア低い＝有利）
			// まずは敵のコマの隣の有利度を更新（弱い敵を狙って、強い敵を避けるため
			// 注意：XとYは逆に見えて分かりにくいだが、あっています
			aMap[tmpY - 1][tmpX] -= oppInfoBonus;
			aMap[tmpY + 1][tmpX] -= oppInfoBonus;
			aMap[tmpY][tmpX - 1] -= oppInfoBonus;
			aMap[tmpY][tmpX + 1] -= oppInfoBonus;
			// 次に、自分のコマの斜めの有利度を更新（連携移動のため）
			aMap[tmpY - 1][tmpX - 1] -= moveTogetherBonus;
			aMap[tmpY + 1][tmpX - 1] -= moveTogetherBonus;
			aMap[tmpY - 1][tmpX + 1] -= moveTogetherBonus;
			aMap[tmpY + 1][tmpX + 1] -= moveTogetherBonus;
		}
	}

}

