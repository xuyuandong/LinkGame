// lpicmatrix.h
#pragma once

#include "stdafx.h"

 /**************************************
	picmatrix[M][N]Ϊ�ƾ���	
	��[1][1]��ʼ����[M-2][N-2]��Ч;
	��ΧһȦΪԤ�������·��;
	ֵ��0Ϊ���ƣ��������֮�������ĵ�״̬;
	����ֵ��Ϊ�Ƶ������ֵͬ���Ʒ������;  
 ***************************************/

const int horzNum = 16;				//������Ŀ
const int vertNum = 9;				//������Ŀ
const int picNum = 24;			//��ͬͼ������Ŀ

//ͼƬ���� -1:������ >=0:���� 
typedef struct
{
	long resourceid;	//װҪ�����ͼƬID
	RECT rc;
} SBLOCK, *PSBLOCK ;

typedef struct 
{
	int path; //ǰ��
	int turn; //
	int distance;
} NODE;

//���������ƾ�����
class LPicMatrix
{
private:
	int curClick, preClick;
	int hitBoth;
	//·����ת�����
	int turn_count;
	//��¼·������
	int length;
	//��¼��������·���յ�
	POINT cornerOne, cornerTwo;
	
	static HWND hParent;
	HBITMAP allThumb;
	POINT startPoint;	//���

	int lThumb;
	static SBLOCK *arrayPic;
	HDC tempDC;			//�����THUMB�洢�ĵط�
	RECT rcPicture_region;	//ͼƬ��ռ��λ�ÿռ�

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