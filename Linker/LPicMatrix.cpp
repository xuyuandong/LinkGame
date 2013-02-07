#include "stdafx.h"
#include "resource.h"
#include "LRegistry.h"
#include "LTimeElapse.h"
#include "LPicMatrix.h"

extern LTimeElapse myTimeElapse;

#define PICTURE_WIDTH 36
#define PICTURE_HEIGHT 36
#define PICTURE_SPACE 0

#define ShowMessage(X) MessageBox(NULL,X,"发生错误",MB_OK|MB_TOPMOST|MB_ICONWARNING); \
	PostQuitMessage(0);

HWND LPicMatrix::hParent =NULL;
PSBLOCK LPicMatrix::arrayPic =NULL;

LPicMatrix::LPicMatrix()
{
	bSound=TRUE;
	hitBoth = -1;
	turn_count = 10;
	length = 100;
	//preClick = -1;
}

LPicMatrix::~LPicMatrix()
{
	if(allThumb)
		DeleteObject(allThumb);
	if(tempDC)
		DeleteDC(tempDC);
	if(arrayPic)
		delete[] arrayPic;
	arrayPic=NULL;
}

void LPicMatrix::picMatrixInit(HWND hParent, long lThumb)
{
	bFirstDraw = TRUE;
	// re-check and destroy
	if(allThumb)
	{
		DeleteObject(allThumb);
		allThumb=NULL;
	}
	if(tempDC)
		DeleteDC(tempDC);
	if(arrayPic)
		delete[] arrayPic;

	arrayPic=NULL;

	if( !IsWindow(hParent) )
	{
		ShowMessage("父窗口句柄无效");
	}
	this->hParent =hParent;
	this->lThumb =lThumb;

	char szReadBuf[512];
	wsprintf( (LPTSTR)szReadBuf,"subjects.bmp" );

	allThumb=(HBITMAP)LoadImage(NULL,(LPCTSTR)szReadBuf,IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_DEFAULTSIZE);

	if(!allThumb)
		allThumb=LoadBitmap( (HINSTANCE)GetModuleHandle(NULL), (LPCTSTR)lThumb );
	if(!allThumb)
	{
		ShowMessage("装入图片库失败，可能图片不存在或内存不够，请关闭一些程序后再试");
	}

	BITMAP bm;
	GetObject( allThumb, sizeof(bm), (LPVOID)&bm );

	RECT rc;
	GetClientRect(hParent,&rc);
	startPoint.x=( (rc.right -rc.left) - (PICTURE_WIDTH*horzNum) )/2;
	startPoint.y=( (rc.bottom -rc.top) - (bm.bmHeight*vertNum) )/2;

	rcPicture_region.left=startPoint.x;
	rcPicture_region.top =startPoint.y;
	rcPicture_region.right =rcPicture_region.left+((PICTURE_WIDTH+PICTURE_SPACE)*horzNum);
	rcPicture_region.bottom =rcPicture_region.top+((PICTURE_HEIGHT+PICTURE_SPACE)*vertNum);

	leftEdge = startPoint.x - ( (PICTURE_WIDTH+PICTURE_SPACE)/2 );
	topEdge = startPoint.y - ( (PICTURE_HEIGHT+PICTURE_SPACE)/2 );
	rightEdge = rcPicture_region.right + ( (PICTURE_WIDTH+PICTURE_SPACE)/2 );
	bottomEdge = rcPicture_region.bottom + ( (PICTURE_HEIGHT+PICTURE_SPACE)/2 );

	
	arrayPic=new SBLOCK[ vertNum*horzNum ];
	memset( (void*)arrayPic, 0, sizeof(SBLOCK)*vertNum*horzNum);

	//图片资源随机初始化
	for(int i=0; i<vertNum*horzNum; i++)
			arrayPic[i].resourceid = i%picNum ;
	for(int i=0; i<10; i++)
		Shuffle();

	tempDC=NULL;

	DWORD tid;
	HANDLE hThread;
	hThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadWatch,(LPVOID)this,0,&tid);
	if(hThread)
		CloseHandle(hThread);
}

#define OUTOFUP(IDQ) 	IDQ<0
#define OUTOFLEFT(IDQ) 	(IDQ+1)%horzNum==0
#define OUTOFRIGHT(IDQ) (IDQ)%horzNum==0
#define OUTOFBOTTOM(IDQ) IDQ>(horzNum*vertNum-1)

#define IDXPOS(idpos) arrayPic[idpos].rc.left+((PICTURE_WIDTH+PICTURE_SPACE)/2)
#define IDYPOS(idpos) arrayPic[idpos].rc.top+((PICTURE_HEIGHT+PICTURE_SPACE)/2)

BOOL LPicMatrix::findPath(int id1, int id2)
{
	using namespace std;
	vector<int> container;
	BOOL bFound = FALSE;

	NODE node[vertNum*horzNum];
	for(int i=0; i<vertNum*horzNum; i++)
	{
		node[i].distance = 100;
		node[i].turn = 10;
	}
	node[id1].distance = 0;
	node[id1].path = -1;
	node[id1].turn = -1;

	container.push_back(id1);

	while( !container.empty() )
	{
		int source = container.back();
		container.pop_back();

		if(source==id2)
		{
			bFound = TRUE;
			//break;
		}

		//look up
		for(int d=1; d<vertNum; d++)
		{
			int tmpid = source - d*horzNum; 
			//合理性判断
			if( tmpid<0 )
				break ;
			if( (arrayPic[tmpid].resourceid!=-1) && (tmpid!=id2) )
				break ;
			//收集扩展节点
			if( (node[tmpid].turn>node[source].turn+1) || 
					( (node[tmpid].turn==node[source].turn+1)
						&& (node[tmpid].distance>node[source].distance+d) ) )
			{
				node[tmpid].turn = node[source].turn+1;
				node[tmpid].distance = node[source].distance+d;
				node[tmpid].path = source;
				if( node[tmpid].turn<3 )
					container.push_back( tmpid );
			}
		}
		//look down
		for(int d=1; d<vertNum; d++)
		{
			int tmpid = source + d*horzNum; 
			//合理性判断
			if( tmpid>vertNum*horzNum-1) 
				break ;
			if( (arrayPic[tmpid].resourceid!=-1) && (tmpid!=id2) )
				break ;
			//收集扩展节点
			if( (node[tmpid].turn>node[source].turn+1) || 
					( (node[tmpid].turn==node[source].turn+1)
						&& (node[tmpid].distance>node[source].distance+d) ) )
			{
				node[tmpid].turn = node[source].turn+1;
				node[tmpid].distance = node[source].distance+d;
				node[tmpid].path = source;
				if( node[tmpid].turn<3 )
					container.push_back( tmpid );
			}
		}
		//look left
		for(int d=1; d<horzNum; d++)
		{
			int tmpid = source - d;
			//合理性判断
			if( (tmpid+1)%horzNum==0) 
				break ;
			if( (arrayPic[tmpid].resourceid!=-1) && (tmpid!=id2) )
				break ;
			//收集扩展节点
			if( (node[tmpid].turn>node[source].turn+1) || 
					( (node[tmpid].turn==node[source].turn+1)
						&& (node[tmpid].distance>node[source].distance+d) ) )
			{
				node[tmpid].turn = node[source].turn+1;
				node[tmpid].distance = node[source].distance+d;
				node[tmpid].path = source;
				if( node[tmpid].turn<3 )
					container.push_back( tmpid );
			}
		}
		//look right
		for(int d=1; d<horzNum; d++)
		{
			int tmpid = source + d; 
			//合理性判断
			if( tmpid%horzNum==0 ) 
				break ;
			if( (arrayPic[tmpid].resourceid!=-1) && (tmpid!=id2) )
				break ;
			//收集扩展节点
			if( (node[tmpid].turn>node[source].turn+1) || 
					( (node[tmpid].turn==node[source].turn+1)
						&& (node[tmpid].distance>node[source].distance+d) ) )
			{
				node[tmpid].turn = node[source].turn+1;
				node[tmpid].distance = node[source].distance+d;
				node[tmpid].path = source;
				if( node[tmpid].turn<3 )
					container.push_back( tmpid );
			}
		}

	}

	//generate inner search results
	int corners = 10;
	int distance = 100;
	if(bFound)
	{
		corners = -1;
		int tmp = id2;
		while(node[tmp].path!=-1)
		{
			corners++;
			tmp = node[tmp].path ;
		}
		distance = node[id2].distance;
	}

	//connect by edges
	BOOL byEdge = FALSE;
	length = 100;
	if( connectToTop(id1, id2) || connectToBottom(id1, id2) 
		|| connectToLeft(id1, id2) || connectToRight(id1, id2) )
	{
		byEdge = TRUE;
		turn_count = 2;
		//length is already assigned in condition functions
	}

	if( byEdge )
	{
		if( bFound )
		{
			if( (corners<2)||(distance<length) )
			{
				turn_count = corners ;
				length = distance ;
				if(turn_count==1)
				{
					int one = node[id2].path;
					cornerOne.x = IDXPOS(one);
					cornerOne.y = IDYPOS(one);
					cornerTwo.x = IDXPOS(one);
					cornerTwo.y = IDYPOS(one);
				}
				if(turn_count==2)
				{
					int two = node[id2].path;
					int one = node[two].path;
					cornerOne.x = IDXPOS(one);
					cornerOne.y = IDYPOS(one);
					cornerTwo.x = IDXPOS(two);
					cornerTwo.y = IDYPOS(two);
				}
			}
		}
	}
	else
	{
		if( bFound )
		{
			turn_count = corners;
			length = distance;
			if(turn_count==1)
			{
				int one = node[id2].path;
				cornerOne.x = IDXPOS(one);
				cornerOne.y = IDYPOS(one);
				cornerTwo.x = IDXPOS(one);
				cornerTwo.y = IDYPOS(one);
			}
			if(turn_count==2)
			{
				int two = node[id2].path;
				int one = node[two].path;
				cornerOne.x = IDXPOS(one);
				cornerOne.y = IDYPOS(one);
				cornerTwo.x = IDXPOS(two);
				cornerTwo.y = IDYPOS(two);
			}
		}
	}
	
	return (bFound||byEdge);
}

BOOL LPicMatrix::connectToTop( int id1, int id2 )
{
	BOOL yes1 = FALSE;
	BOOL yes2 = FALSE;
	int s1 = id1;
	int s2 = id2;
	int dist1 = 0;
	int dist2 = 0;
	int dist ;
	while(true)
	{
		id1 = id1 - horzNum ;
		dist1++;
		if(id1<0)
		{
			yes1 = TRUE;
			break;
		}
		if(	arrayPic[id1].resourceid!=-1)
			break;
	}
	while(true)
	{
		id2 = id2 - horzNum ;
		dist2++;
		if(id2<0)
		{
			yes2 = TRUE;
			break;
		}
		if(	arrayPic[id2].resourceid!=-1)
			break;
	}

	if(yes1 && yes2)
	{
		dist = dist1 + dist2 + abs(id1-id2) ;
		if(dist<length)
			length = dist ;

		cornerOne.y=topEdge;
		cornerOne.x=IDXPOS(s1);
		cornerTwo.y=topEdge;
		cornerTwo.x=IDXPOS(s2);

		return TRUE;
	}

	return FALSE;
}

BOOL LPicMatrix::connectToBottom(int id1, int id2)
{
	BOOL yes1 = FALSE;
	BOOL yes2 = FALSE;
	int s1 = id1;
	int s2 = id2;
	int dist1 = 0;
	int dist2 = 0;
	int dist ;
	while(true)
	{
		id1 = id1 + horzNum ;
		dist1++;
		if(id1>vertNum*horzNum-1)
		{
			yes1 = TRUE;
			break;
		}
		if(	arrayPic[id1].resourceid!=-1)
			break;
	}
	while(true)
	{
		id2 = id2 + horzNum ;
		dist2++;
		if(id2>vertNum*horzNum-1)
		{
			yes2 = TRUE;
			break;
		}
		if(	arrayPic[id2].resourceid!=-1)
			break;
	}

	if(yes1 && yes2)
	{
		dist = dist1 + dist2 + abs(id1-id2) ;
		if(dist<length)
			length = dist ;

		cornerOne.y=bottomEdge;
		cornerOne.x=IDXPOS(s1);
		cornerTwo.y= bottomEdge;
		cornerTwo.x=IDXPOS(s2);

		return TRUE;
	}

	return FALSE;
}

BOOL LPicMatrix::connectToLeft(int id1, int id2)
{
	BOOL yes1 = FALSE;
	BOOL yes2 = FALSE;
	int s1 = id1;
	int s2 = id2;
	int dist1 = 0;
	int dist2 = 0;
	int dist ;
	while(true)
	{
		id1 = id1 - 1 ;
		dist1++;
		if( (id1+1)%horzNum==0 )
		{
			yes1 = TRUE;
			break;
		}
		if(	arrayPic[id1].resourceid!=-1)
			break;
	}
	while(true)
	{
		id2 = id2 - 1 ;
		dist2++;
		if( (id2+1)%horzNum==0 )
		{
			yes2 = TRUE;
			break;
		}
		if(	arrayPic[id2].resourceid!=-1)
			break;
	}

	if(yes1 && yes2)
	{
		dist = dist1 + dist2 + abs(id1-id2)%horzNum ;
		if(dist<length)
			length = dist ;

		cornerOne.x=leftEdge;
		cornerOne.y=IDYPOS(s1);
		cornerTwo.x=leftEdge;
		cornerTwo.y=IDYPOS(s2);

		return TRUE;
	}

	return FALSE;
}

BOOL LPicMatrix::connectToRight(int id1, int id2)
{
	BOOL yes1 = FALSE;
	BOOL yes2 = FALSE;
	int s1 = id1;
	int s2 = id2;
	int dist1 = 0;
	int dist2 = 0;
	int dist ;
	while(true)
	{
		id1 = id1 + 1 ;
		dist1++;
		if( id1%horzNum==0 )
		{
			yes1 = TRUE;
			break;
		}
		if(	arrayPic[id1].resourceid!=-1)
			break;
	}
	while(true)
	{
		id2 = id2 + 1 ;
		dist2++;
		if( id2%horzNum==0 )
		{
			yes2 = TRUE;
			break;
		}
		if(	arrayPic[id2].resourceid!=-1)
			break;
	}

	if(yes1 && yes2)
	{
		dist = dist1 + dist2 + abs(id1-id2)%horzNum ;
		if(dist<length)
			length = dist ;

		cornerOne.x=rightEdge;
		cornerOne.y=IDYPOS(s1);
		cornerTwo.x=rightEdge;
		cornerTwo.y=IDYPOS(s2);

		return TRUE;
	}

	return FALSE;
}

BOOL LPicMatrix::autoSearchLink(/*LPicMatrix *picm, */int *idOne, int *idTwo)
{
	using namespace std;
	vector<int> container;

	//临时存储，idOne和idTwo的候选
	int id1, id2;
	BOOL bFound = FALSE;

	NODE node[vertNum*horzNum];
	
	for(int i=0; i<vertNum*horzNum; i++)
	{
		int key = arrayPic[i].resourceid;
		if( key!=-1 )
		{
			for(int m=0; m<vertNum*horzNum; m++)
			{
				node[m].distance = 100;
				node[m].turn = 10;
			}

			id1 = i;
			node[id1].distance = 0;
			node[id1].turn = -1;

			container.push_back(id1);

			while( !container.empty() )
			{
				int source = container.back();
				container.pop_back();

				//search up
				for(int d=1; d<vertNum; d++)
				{
					int tmpid = source - d*horzNum; 
					//合理性判断
					if( tmpid<0 )
						break ;
					if( arrayPic[tmpid].resourceid!=-1)
					{
						if( (arrayPic[tmpid].resourceid!=key)||(tmpid==id1) )
							break ;
						else
						{
							bFound = TRUE;
							id2 = tmpid;
							break;
						}
					}
					//收集扩展节点
					if( (node[tmpid].turn>node[source].turn+1) || 
							( (node[tmpid].turn==node[source].turn+1)
								&& (node[tmpid].distance>node[source].distance+d) ) )
					{
						node[tmpid].turn = node[source].turn+1;
						node[tmpid].distance = node[source].distance+d;
						//node[tmpid].path = source;
						if( node[tmpid].turn<3 )
							container.push_back( tmpid );
					}
				}//end up

				if(bFound)
					break;

				//search down
				for(int d=1; d<vertNum; d++)
				{
					int tmpid = source + d*horzNum; 
					//合理性判断
					if( tmpid>vertNum*horzNum-1 )
						break ;
					if( arrayPic[tmpid].resourceid!=-1)
					{
						if( (arrayPic[tmpid].resourceid!=key)||(tmpid==id1) )
							break ;
						else
						{
							bFound = TRUE;
							id2 = tmpid;
							break;
						}
					}
					//收集扩展节点
					if( (node[tmpid].turn>node[source].turn+1) || 
							( (node[tmpid].turn==node[source].turn+1)
								&& (node[tmpid].distance>node[source].distance+d) ) )
					{
						node[tmpid].turn = node[source].turn+1;
						node[tmpid].distance = node[source].distance+d;
						//node[tmpid].path = source;
						if( node[tmpid].turn<3 )
							container.push_back( tmpid );
					}
				}//end down

				if(bFound)
					break;

				//search left
				for(int d=1; d<vertNum; d++)
				{
					int tmpid = source - d; 
					//合理性判断
					if( (tmpid+1)%horzNum==0 )
						break ;
					if( arrayPic[tmpid].resourceid!=-1)
					{
						if( (arrayPic[tmpid].resourceid!=key)||(tmpid==id1) )
							break ;
						else
						{
							bFound = TRUE;
							id2 = tmpid;
							break;
						}
					}
					//收集扩展节点
					if( (node[tmpid].turn>node[source].turn+1) || 
							( (node[tmpid].turn==node[source].turn+1)
								&& (node[tmpid].distance>node[source].distance+d) ) )
					{
						node[tmpid].turn = node[source].turn+1;
						node[tmpid].distance = node[source].distance+d;
						//node[tmpid].path = source;
						if( node[tmpid].turn<3 )
							container.push_back( tmpid );
					}
				}//end left

				if(bFound)
					break;

				//search right
				for(int d=1; d<vertNum; d++)
				{
					int tmpid = source + d; 
					//合理性判断
					if( tmpid%horzNum==0 )
						break ;
					if( arrayPic[tmpid].resourceid!=-1)
					{
						if( (arrayPic[tmpid].resourceid!=key)||(tmpid==id1) )
							break ;
						else
						{
							bFound = TRUE;
							id2 = tmpid;
							break;
						}
					}
					//收集扩展节点
					if( (node[tmpid].turn>node[source].turn+1) || 
							( (node[tmpid].turn==node[source].turn+1)
								&& (node[tmpid].distance>node[source].distance+d) ) )
					{
						node[tmpid].turn = node[source].turn+1;
						node[tmpid].distance = node[source].distance+d;
						//node[tmpid].path = source;
						if( node[tmpid].turn<3 )
							container.push_back( tmpid );
					}
				}//end right

				if(bFound)
					break;

			}//end while(empty)
			
			if(bFound)
				break;

			//search edge
			for(int k=0; k<vertNum*horzNum; k++)
			{
				if( (k!=id1)&&( arrayPic[k].resourceid==key) )
				{
					if( connectToTop(id1, k) || connectToBottom(id1, k) 
						|| connectToLeft(id1, k) || connectToRight(id1, k) )
					{
						id2 = k;
						bFound = TRUE;
						break;
					}
				}
			}

			if(bFound)
				break;

		}//end if(key!=-1)

	}//end for(all grid)

	if(bFound)
	{
		*idOne = id1;
		*idTwo = id2;
	}

	return bFound;
}

void LPicMatrix::pictureClick( POINT &ptPos )
{
	if( !PtInRect(&rcPicture_region,ptPos) )		
		return ;

	int x = ptPos.x - startPoint.x;
	int y = ptPos.y - startPoint.y;
	x=x/(PICTURE_WIDTH + PICTURE_SPACE);
	y=y/(PICTURE_HEIGHT + PICTURE_SPACE);

	curClick=(int)y*horzNum+(int)x;
	if((hitBoth!=-1)&&(curClick==preClick))
		return ;

	if( arrayPic[curClick].resourceid!=-1 && hitBoth!=-1 
			&& arrayPic[curClick].resourceid==arrayPic[preClick].resourceid 
				&& findPath(preClick, curClick) )
	{
		outputText("干得好");
		drawLinkLine( preClick, curClick );
		//输出消除声音
		playAction( (LPCTSTR)IDR_CLEAR );
		arrayPic[curClick].resourceid = arrayPic[preClick].resourceid = -1;
		hitBoth = -1;

		//是否还没打完
		BOOL bContinue=FALSE;
		for(int i=0; i<vertNum; i++)
			for(int j=0; j<horzNum; j++)
				if( arrayPic[i*horzNum+j].resourceid!=-1 )
				{
					bContinue=TRUE;
					break;
				}

		if( !bContinue )
		{
			myTimeElapse.Pause();
			MessageBox( hParent, "恭喜你，完成了游戏", "信息", MB_OK );

			myTimeElapse.totalSecond =TOTALTIME;
			myTimeElapse.tipsNum = MAXNUM;
			myTimeElapse.shuffleNum = MAXNUM;

			picMatrixInit( hParent, lThumb );
			InvalidateRect( hParent, NULL, TRUE );

			SendMessage( hParent, STOP_TIME, 0, 0 );
		}

		InvalidateRect(hParent, &arrayPic[preClick].rc, TRUE);
		InvalidateRect(hParent, &arrayPic[curClick].rc, TRUE);

		return ;
	}

	if( hitBoth == -1 && arrayPic[curClick].resourceid != -1 )
	{
		hitBoth = 1;
		preClick = curClick;
		//显示选中图片框框
		ParsePosAndInvalidate( preClick );
		//输出点击声音
		playAction( (LPCTSTR)IDR_CLICK );

		UpdateWindow(hParent);
		return ;
	}

	if( hitBoth != -1 && arrayPic[curClick].resourceid != -1 )
	{
		outputText("我晕，点错了");
		//把原来的焦点去掉, 设置新焦点
		ParsePosAndInvalidate( preClick, TRUE );
		//标记已选中一个图片
		hitBoth = 1;
		preClick = curClick;
		//显示选中图片框框
		ParsePosAndInvalidate( curClick );
		//输出点击声音
		playAction( (LPCTSTR)IDR_CLICK );
		//
		UpdateWindow(hParent);
		return ;
	}

	return ;
}

#define DRAW_EDGE(color,w,x,y,tox,toy) \
		hpen=CreatePen(PS_SOLID,w,color); \
		oldpen=(HPEN)SelectObject(hDC,hpen); \
		MoveToEx(hDC,x,y,NULL); \
		LineTo(hDC,tox,toy); \
		SelectObject(hDC,oldpen); \
		DeleteObject(hpen);

void LPicMatrix::picMatrixDraw(HDC hDC/*, PSBLOCK arrayPic*/)
{
	HBRUSH hbr;
	HPEN hpen, oldpen;
	RECT rcfill;

	if(!tempDC)
	{
		tempDC=CreateCompatibleDC(hDC);
		SelectObject(tempDC,allThumb);
	}

	int hPos=startPoint.x;
	int vPos=startPoint.y;
	for(int i=0; i<horzNum*vertNum; i++)
	{
		if(i!=0 && i%horzNum==0)
		{
			vPos+=PICTURE_HEIGHT+PICTURE_SPACE;
			hPos=startPoint.x;
		}

		//已经打掉了
		if(arrayPic[i].resourceid ==-1)
		{		
			if(bFirstDraw)
				SetRect(&arrayPic[i].rc, hPos-PICTURE_SPACE-3, vPos-PICTURE_SPACE-3, 
								PICTURE_WIDTH+hPos+PICTURE_SPACE+5, PICTURE_HEIGHT+vPos+PICTURE_SPACE+5);
			hPos+=PICTURE_WIDTH+PICTURE_SPACE;
			continue;
		}

		if(hitBoth!=-1 && preClick==i)
		{
			SetRect(&rcfill,hPos,vPos,PICTURE_WIDTH+hPos,PICTURE_HEIGHT+vPos);
			hbr=CreateSolidBrush(RGB(200,200,200));
			FillRect(hDC,&rcfill,hbr);
			DeleteObject(hbr);
		}
		else
		{
			SetRect(&rcfill,hPos,vPos,PICTURE_WIDTH+hPos,PICTURE_HEIGHT+vPos);
			hbr=CreateSolidBrush(RGB(249,254,214));
			FillRect(hDC,&rcfill,hbr);
			DeleteObject(hbr);
		}

		DRAW_EDGE(RGB(209,182,161),2,hPos,vPos,hPos,vPos+PICTURE_HEIGHT);
		DRAW_EDGE(RGB(209,182,161),2,hPos,vPos+PICTURE_HEIGHT,hPos+PICTURE_WIDTH,vPos+PICTURE_HEIGHT);
		DRAW_EDGE(RGB(255,255,255),1,hPos+PICTURE_WIDTH-2,vPos,hPos+PICTURE_WIDTH-2,vPos+PICTURE_HEIGHT);

		DRAW_EDGE(RGB(248,182,94),1,hPos+PICTURE_WIDTH,vPos,hPos+PICTURE_WIDTH+4,vPos+4);
		DRAW_EDGE(RGB(248,182,94),1,hPos+PICTURE_WIDTH,vPos+PICTURE_HEIGHT,hPos+PICTURE_WIDTH+4,vPos+PICTURE_HEIGHT+4);
		//fill
		DRAW_EDGE(RGB(242,203,66),1,hPos+PICTURE_WIDTH,vPos+1,hPos+PICTURE_WIDTH,vPos+PICTURE_HEIGHT+1);
		DRAW_EDGE(RGB(242,203,66),1,hPos+PICTURE_WIDTH+1,vPos+2,hPos+PICTURE_WIDTH+1,vPos+PICTURE_HEIGHT+2);
		DRAW_EDGE(RGB(242,203,66),1,hPos+PICTURE_WIDTH+2,vPos+3,hPos+PICTURE_WIDTH+2,vPos+PICTURE_HEIGHT+3);
		DRAW_EDGE(RGB(242,203,66),1,hPos+PICTURE_WIDTH+3,vPos+4,hPos+PICTURE_WIDTH+3,vPos+PICTURE_HEIGHT+4);
		DRAW_EDGE(RGB(242,203,66),1,hPos+PICTURE_WIDTH+4,vPos+5,hPos+PICTURE_WIDTH+4,vPos+PICTURE_HEIGHT+5);

		DRAW_EDGE(RGB(248,182,94),1,hPos,vPos+PICTURE_HEIGHT,hPos+4,vPos+PICTURE_HEIGHT+4);

		//fill
		DRAW_EDGE(RGB(238,162,64),1,hPos+1,vPos+PICTURE_HEIGHT,hPos+PICTURE_WIDTH+1,vPos+PICTURE_HEIGHT);
		DRAW_EDGE(RGB(238,162,64),1,hPos+2,vPos+PICTURE_HEIGHT+1,hPos+PICTURE_WIDTH+2,vPos+PICTURE_HEIGHT+1);
		DRAW_EDGE(RGB(238,162,64),1,hPos+3,vPos+PICTURE_HEIGHT+2,hPos+PICTURE_WIDTH+3,vPos+PICTURE_HEIGHT+2);
		DRAW_EDGE(RGB(238,162,64),1,hPos+4,vPos+PICTURE_HEIGHT+3,hPos+PICTURE_WIDTH+4,vPos+PICTURE_HEIGHT+3);
		DRAW_EDGE(RGB(238,162,64),1,hPos+5,vPos+PICTURE_HEIGHT+4,hPos+PICTURE_WIDTH+5,vPos+PICTURE_HEIGHT+4);

		BitBlt(hDC,hPos,vPos,PICTURE_WIDTH,PICTURE_HEIGHT,tempDC,arrayPic[i].resourceid*PICTURE_WIDTH,0,SRCAND);
		if(bFirstDraw)
			SetRect(&arrayPic[i].rc,hPos-PICTURE_SPACE-3,vPos-PICTURE_SPACE-3,PICTURE_WIDTH+hPos+PICTURE_SPACE+5,PICTURE_HEIGHT+vPos+PICTURE_SPACE+5);
		hPos+=PICTURE_WIDTH+PICTURE_SPACE;
	}

	bFirstDraw = FALSE;
}

void LPicMatrix::playAction(LPCTSTR pszRes)
{
	HRSRC hsrc;
	HGLOBAL hGlobal;
	LPVOID pData;
	if(bSound)
	{
		hsrc=FindResource(GetModuleHandle(NULL),(LPCTSTR)pszRes,(LPCTSTR)"WAVE");
		if(hsrc)
		{
			hGlobal=LoadResource(GetModuleHandle(NULL),hsrc);
			if(hGlobal)
			{
				pData=GlobalLock(hGlobal);
				PlaySound((LPCTSTR)pData,GetModuleHandle(NULL),SND_ASYNC|SND_NOWAIT|SND_MEMORY);
				GlobalUnlock(hGlobal);
			}
			FreeResource(hsrc);
		}
	}
}

void LPicMatrix::ParsePosAndInvalidate( int id, BOOL bClear )
{
	//	InvalidateRect(hParent,&arrayPic[id].rc,TRUE);
	int vPos;
	int hPos;
	BOOL bHaveNext;
	BOOL bHavePNext;

	HDC hDC;
	HBRUSH hbr;
	HPEN hpen,oldpen;
	RECT rcfill;

	hPos=arrayPic[id].rc.left +3;
	vPos=arrayPic[id].rc.top +3;		

	hDC=GetDC(hParent);
	if(!bClear){
		SetRect(&rcfill,hPos,vPos,PICTURE_WIDTH+hPos,PICTURE_HEIGHT+vPos);
		hbr=CreateSolidBrush(RGB(200,200,200));
		FillRect(hDC,&rcfill,hbr);
		DeleteObject(hbr);
	}else{
		SetRect(&rcfill,hPos,vPos,PICTURE_WIDTH+hPos,PICTURE_HEIGHT+vPos);
		hbr=CreateSolidBrush(RGB(249,254,214));
		FillRect(hDC,&rcfill,hbr);
		DeleteObject(hbr);
	}

	DRAW_EDGE(RGB(209,182,161),2,hPos,vPos,hPos,vPos+PICTURE_HEIGHT);
	DRAW_EDGE(RGB(209,182,161),2,hPos,vPos+PICTURE_HEIGHT,hPos+PICTURE_WIDTH,vPos+PICTURE_HEIGHT);
	DRAW_EDGE(RGB(255,255,255),1,hPos+PICTURE_WIDTH-2,vPos,hPos+PICTURE_WIDTH-2,vPos+PICTURE_HEIGHT);


	bHaveNext=bHavePNext=TRUE;
	if(id+1>horzNum*vertNum||(arrayPic[id+1].resourceid==-1)){
		bHaveNext=FALSE;
	}

	if((id+horzNum)<(horzNum*vertNum) && arrayPic[id+horzNum].resourceid ==-1){
		bHavePNext=FALSE;
	}

	if(!bHaveNext){
		DRAW_EDGE(RGB(248,182,94),1,hPos+PICTURE_WIDTH,vPos,hPos+PICTURE_WIDTH+4,vPos+4);
		if(!bHavePNext){
			DRAW_EDGE(RGB(248,182,94),1,hPos+PICTURE_WIDTH,vPos+PICTURE_HEIGHT,hPos+PICTURE_WIDTH+4,vPos+PICTURE_HEIGHT+4);
		}
		//fill
		if(!bHavePNext){
			DRAW_EDGE(RGB(242,203,66),1,hPos+PICTURE_WIDTH,vPos+1,hPos+PICTURE_WIDTH,vPos+PICTURE_HEIGHT+1);
			DRAW_EDGE(RGB(242,203,66),1,hPos+PICTURE_WIDTH+1,vPos+2,hPos+PICTURE_WIDTH+1,vPos+PICTURE_HEIGHT+2);
			DRAW_EDGE(RGB(242,203,66),1,hPos+PICTURE_WIDTH+2,vPos+3,hPos+PICTURE_WIDTH+2,vPos+PICTURE_HEIGHT+3);
			DRAW_EDGE(RGB(242,203,66),1,hPos+PICTURE_WIDTH+3,vPos+4,hPos+PICTURE_WIDTH+3,vPos+PICTURE_HEIGHT+4);
			DRAW_EDGE(RGB(242,203,66),1,hPos+PICTURE_WIDTH+4,vPos+5,hPos+PICTURE_WIDTH+4,vPos+PICTURE_HEIGHT+5);
		}else{
			DRAW_EDGE(RGB(242,203,66),1,hPos+PICTURE_WIDTH,vPos+1,hPos+PICTURE_WIDTH,vPos+PICTURE_HEIGHT);
			DRAW_EDGE(RGB(242,203,66),1,hPos+PICTURE_WIDTH+1,vPos+2,hPos+PICTURE_WIDTH+1,vPos+PICTURE_HEIGHT);
			DRAW_EDGE(RGB(242,203,66),1,hPos+PICTURE_WIDTH+2,vPos+3,hPos+PICTURE_WIDTH+2,vPos+PICTURE_HEIGHT);
			DRAW_EDGE(RGB(242,203,66),1,hPos+PICTURE_WIDTH+3,vPos+4,hPos+PICTURE_WIDTH+3,vPos+PICTURE_HEIGHT);
			DRAW_EDGE(RGB(242,203,66),1,hPos+PICTURE_WIDTH+4,vPos+5,hPos+PICTURE_WIDTH+4,vPos+PICTURE_HEIGHT);
		}
	}
//////////////

	if(!bHavePNext){
		DRAW_EDGE(RGB(248,182,94),1,hPos,vPos+PICTURE_HEIGHT,hPos+4,vPos+PICTURE_HEIGHT+4);
	//fill
		if(!bHaveNext){
			DRAW_EDGE(RGB(238,162,64),1,hPos+1,vPos+PICTURE_HEIGHT,hPos+PICTURE_WIDTH+1,vPos+PICTURE_HEIGHT);
			DRAW_EDGE(RGB(238,162,64),1,hPos+2,vPos+PICTURE_HEIGHT+1,hPos+PICTURE_WIDTH+2,vPos+PICTURE_HEIGHT+1);
			DRAW_EDGE(RGB(238,162,64),1,hPos+3,vPos+PICTURE_HEIGHT+2,hPos+PICTURE_WIDTH+3,vPos+PICTURE_HEIGHT+2);
			DRAW_EDGE(RGB(238,162,64),1,hPos+4,vPos+PICTURE_HEIGHT+3,hPos+PICTURE_WIDTH+4,vPos+PICTURE_HEIGHT+3);
			DRAW_EDGE(RGB(238,162,64),1,hPos+5,vPos+PICTURE_HEIGHT+4,hPos+PICTURE_WIDTH+5,vPos+PICTURE_HEIGHT+4);
		}else{
			DRAW_EDGE(RGB(238,162,64),1,hPos+1,vPos+PICTURE_HEIGHT,hPos+PICTURE_WIDTH,vPos+PICTURE_HEIGHT);
			DRAW_EDGE(RGB(238,162,64),1,hPos+2,vPos+PICTURE_HEIGHT+1,hPos+PICTURE_WIDTH,vPos+PICTURE_HEIGHT+1);
			DRAW_EDGE(RGB(238,162,64),1,hPos+3,vPos+PICTURE_HEIGHT+2,hPos+PICTURE_WIDTH,vPos+PICTURE_HEIGHT+2);
			DRAW_EDGE(RGB(238,162,64),1,hPos+4,vPos+PICTURE_HEIGHT+3,hPos+PICTURE_WIDTH,vPos+PICTURE_HEIGHT+3);
			DRAW_EDGE(RGB(238,162,64),1,hPos+5,vPos+PICTURE_HEIGHT+4,hPos+PICTURE_WIDTH,vPos+PICTURE_HEIGHT+4);
		}
	}
///////////////////

	BitBlt(hDC,hPos,vPos,PICTURE_WIDTH,PICTURE_HEIGHT,tempDC,arrayPic[id].resourceid*PICTURE_WIDTH,0,SRCAND);
	ReleaseDC(hParent,hDC);
}

void LPicMatrix::drawLinkLine(int preClick, int curClick)
{
	POINT ptStart, ptEnd, ptOne, ptTwo;

	ptStart.x=IDXPOS(preClick);
	ptStart.y=IDYPOS(preClick);
	ptEnd.x=IDXPOS(curClick);
	ptEnd.y=IDYPOS(curClick);

	if(turn_count==0)
	{
		memcpy((void*)&ptOne,(const void*)&ptStart,sizeof(POINT));
		memcpy((void*)&ptTwo,(const void*)&ptEnd,sizeof(POINT));
	}
	else
	{
		memcpy((void*)&ptOne,(const void*)&cornerOne,sizeof(POINT));
		memcpy((void*)&ptTwo,(const void*)&cornerTwo,sizeof(POINT));
	}

	HDC hDC;
	HPEN oldpen,newpen;
	hDC=GetDC(hParent);
	newpen=CreatePen(PS_INSIDEFRAME,3,RGB(0,255,255));
	oldpen=(HPEN)SelectObject(hDC,newpen);
	SetROP2(hDC,R2_XORPEN);

	for(int i=0; i<2; i++)
	{
		MoveToEx(hDC,ptStart.x,ptStart.y,NULL);
		LineTo(hDC,ptOne.x,ptOne.y);
		LineTo(hDC,ptTwo.x,ptTwo.y);
		LineTo(hDC,ptEnd.x,ptEnd.y);
		Sleep(100);
	}

	SelectObject(hDC,oldpen);
	DeleteObject(newpen);
	ReleaseDC(hParent,hDC);
}

void LPicMatrix::outputText( char *pszInfo )
{
	static int prelen=-1;	
	
	HDC hDC;
	hDC=GetDC(hParent);
	SetTextColor(hDC,RGB(0,0,255));

	HFONT hfont,oldfont;
	hfont=CreateFont(25,0,0,0,0,0,0,0,0,0,0,0,0,"宋体");	
	oldfont=(HFONT)SelectObject(hDC,hfont);

	RECT rc;
	if(prelen>6)
	{
		rc.left =rcPicture_region.left;
		rc.top =rcPicture_region.top -30;
		rc.right=rc.left +500;
		rc.bottom =rc.top +30;
		InvalidateRect(hParent,&rc,TRUE);
		UpdateWindow(hParent);
	}
	prelen=strlen(pszInfo);

	TextOut(hDC,rcPicture_region.left,rcPicture_region.top-30,pszInfo,strlen(pszInfo));

	SelectObject(hDC,oldfont);
	DeleteObject(hfont);
	ReleaseDC(hParent,hDC);
}

void LPicMatrix::Shuffle()
{
	time_t curtime; //记录当前时间
	time(&curtime);//取得当前时间
	srand(curtime);//用当前时间作种子，产生随机数

	int totalNum = horzNum*vertNum ;
	for(int i=0; i<vertNum; i++)
	{
		for(int j=0; j<horzNum; j++)
		{
			if( arrayPic[i*vertNum+j].resourceid!=-1 )
			{
				int index ;
				do{
					index = rand()%totalNum;
				}while( arrayPic[index].resourceid==-1 );

				int tmp = arrayPic[index].resourceid;
				arrayPic[index].resourceid = arrayPic[i*vertNum+j].resourceid ;
				arrayPic[i*vertNum+j].resourceid = tmp;
			}
		} 
	}

}

void LPicMatrix::tipLink( int id1, int id2 )
{
	POINT ptPos;

	ptPos.x=(arrayPic[id1].rc.right-arrayPic[id1].rc.left)/2+arrayPic[id1].rc.left;
	ptPos.y=(arrayPic[id1].rc.bottom -arrayPic[id1].rc.top)/2+arrayPic[id1].rc.top;

	pictureClick(ptPos);

	ptPos.x=(arrayPic[id2].rc.right-arrayPic[id2].rc.left)/2+arrayPic[id2].rc.left;
	ptPos.y=(arrayPic[id2].rc.bottom -arrayPic[id2].rc.top)/2+arrayPic[id2].rc.top;

	pictureClick(ptPos);
}

LRESULT CALLBACK LPicMatrix::ThreadWatch(LPARAM lParam)
{
	LPicMatrix *picm;
	picm = (LPicMatrix*)lParam;

	BOOL bFound;
	while(true)
	{
		int id1, id2;
		bFound =	picm->autoSearchLink( /*picm, */&id1, &id2 );
		//bFound = TRUE;

		if(bFound==FALSE && myTimeElapse.bPause ==FALSE)
		{
			picm->Shuffle();
			InvalidateRect(hParent,NULL,TRUE);
		}

		Sleep(3000);
	}

	return 0;
}

BOOL LPicMatrix::GetSound()
{
	return bSound;
}

void LPicMatrix::SetSound(BOOL bSound)
{
	this->bSound=bSound;
}