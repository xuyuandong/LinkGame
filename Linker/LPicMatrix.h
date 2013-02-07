// lpicmatrix.h
#pragma once

#include "stdafx.h"

 /**************************************
	picmatrix[M][N]为牌矩阵；	
	从[1][1]开始，到[M-2][N-2]有效;
	外围一圈为预留的配对路径;
	值＝0为无牌，即牌配对之后被消掉的的状态;
	其他值即为牌的类别，相同值的牌方可配对;  
 ***************************************/

const int horzNum = 16;				//横向数目
const int vertNum = 9;				//纵向数目
const int picNum = 24;			//不同图案的数目

//图片矩阵， -1:消掉， >=0:存在 
typedef struct
{
	long resourceid;	//装要载入的图片ID
	RECT rc;
} SBLOCK, *PSBLOCK ;

typedef struct 
{
	int path; //前驱
	int turn; //
	int distance;
} NODE;

//连连看的牌矩阵类
class LPicMatrix
{
private:
	int curClick, preClick;
	int hitBoth;
	//路径的转弯次数
	int turn_count;
	//记录路径长度
	int length;
	//记录搜索到的路径拐点
	POINT cornerOne, cornerTwo;
	
	static HWND hParent;
	HBITMAP allThumb;
	POINT startPoint;	//起点

	int lThumb;
	static SBLOCK *arrayPic;
	HDC tempDC;			//整体的THUMB存储的地方
	RECT rcPicture_region;	//图片所占的位置空间

	int leftEdge;
	int rightEdge;
	int topEdge;
	int bottomEdge;

	BOOL bFirstDraw;
	BOOL bSound;
  
public:
	LPicMatrix();
	~LPicMatrix();

	void picMatrixInit(HWND hParent, long lThumb);
	
	// display process
	void picMatrixDraw(HDC hDC/*, PSBLOCK arrayPic = arrayPic*/);

	void ParsePosAndInvalidate(int id, BOOL bClear=FALSE);
	void drawLinkLine(int preClick, int curClick);
	void outputText(char * pszInfo);

	// sound process
	void playAction(LPCTSTR pszRes);
	
	// action
	void Shuffle();
	void pictureClick(POINT &ptPos);
	BOOL autoSearchLink(/*LPicMatrix *picm, */int *idOne, int *idTwo);
	BOOL findPath( int id1, int id2 );
	BOOL connectToTop( int id1, int id2 );
	BOOL connectToBottom( int id1, int id2 );
	BOOL connectToLeft( int id1, int id2 );
	BOOL connectToRight( int id1, int id2 );
	void tipLink(int id1, int id2);

	BOOL GetSound();
	void SetSound(BOOL bSound);

	static LRESULT CALLBACK ThreadWatch(LPARAM lParam);
};