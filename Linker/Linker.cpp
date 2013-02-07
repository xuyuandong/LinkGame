// Linker.cpp : Defines the entry point for the application.
/*
本程序还存在一些bug，就像我自己，不那么完美，
但这些bug不影响程序的运行和使用，就像人的缺点，不会使他被完全否定
谢谢使用
*/

#include "stdafx.h"
#include "Linker.h"
#include "LTimeElapse.h"
#include "LRegistry.h"
#include "LPicMatrix.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// TODO: My own Object 
TCHAR szAppName[] = TEXT ("Linker") ;
const int TimerID = 1;
IPicture *pPicture;
//
LTimeElapse myTimeElapse;
LRegistry myRegistry;
LPicMatrix myPicMatrix;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

//TODO: My own Functions
void LoadAndShowPicture(HWND hWnd,HDC hDC,LPCTSTR pRes);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// TODO: com库初始化
	CoInitialize(NULL);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_LINKER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LINKER));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// TODO: release object

	// TODO: unload com
	CoUninitialize();		

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LINKER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_LINKER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	// TODO: my object needed
	HMENU hMenu;
	POINT pPos;
	static BOOL bGameStarted = FALSE;
	BOOL bFound;

	switch (message)
	{
	case WM_CREATE:
		hMenu=GetMenu(hWnd);
		hMenu=GetSubMenu(hMenu,0);
		hMenu=GetSubMenu(hMenu,2);
		CheckMenuItem(hMenu,ID_SOUND,MF_BYCOMMAND|MF_CHECKED);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_START:
			myTimeElapse.LTimeElapseInit(hWnd, TOTALTIME);			//120 sec
			myPicMatrix.picMatrixInit(hWnd, IDB_BITMAP_SUBJECTS);
			bGameStarted = TRUE;
			InvalidateRect(hWnd,NULL,TRUE);
			UpdateWindow(hWnd);
			myTimeElapse.Begin();
			//myTimeElapse.InvalidIt(hWnd);
			break;
		case ID_SOUND:
			hMenu=GetMenu(hWnd);
			hMenu=GetSubMenu(hMenu,0);
			hMenu=GetSubMenu(hMenu,2);
			if(myPicMatrix.GetSound())
			{
				CheckMenuItem(hMenu,ID_SOUND,MF_BYCOMMAND|MF_UNCHECKED);
				myPicMatrix.SetSound(FALSE);
			}
			else
			{
				CheckMenuItem(hMenu,ID_SOUND,MF_BYCOMMAND|MF_CHECKED);
				myPicMatrix.SetSound(TRUE);
			}
			break;
		case ID_TIPS:
			SendMessage(hWnd,WM_KEYDOWN,(WPARAM)VK_F4,0);
			break;
		case ID_SHUFFLE:
			SendMessage(hWnd,WM_KEYDOWN,(WPARAM)VK_F5,0);
			break;
		case ID_PAUSE:
			SendMessage(hWnd,WM_KEYDOWN,(WPARAM)VK_F6,0);
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case STOP_TIME:
		if(bGameStarted)
		{
			myTimeElapse.Stop();
			hdc=GetDC(hWnd);
			LoadAndShowPicture(hWnd, hdc, (LPCTSTR)IDR_END);
			ReleaseDC(hWnd, hdc);
			Sleep(3000);
			bGameStarted = FALSE;
			InvalidateRect(hWnd,NULL,TRUE);
		}
		break;
	case OUT_OF_TIME:
		myTimeElapse.Stop();
		//
		MessageBox(hWnd,"失败了，你也太菜了","信息",MB_OK|MB_ICONWARNING);
		if(bGameStarted)
		{
			hdc=GetDC(hWnd);
			LoadAndShowPicture(hWnd, hdc, (LPCTSTR)IDR_END);
			ReleaseDC(hWnd, hdc);
			Sleep(3000);

			bGameStarted = FALSE;
			myTimeElapse.totalSecond =TOTALTIME;
			myTimeElapse.tipsNum = MAXNUM;
			myTimeElapse.shuffleNum = MAXNUM;

			InvalidateRect(hWnd,NULL,TRUE);
		}
		break;
	case WM_KEYDOWN:
		if(bGameStarted)
		{
			switch(wParam)
			{
			case VK_F5:
				if(myTimeElapse.shuffleNum<1)
				{
					MessageBox( hWnd, "洗牌提示已用完","信息", MB_OK );
					break;
				}
				myTimeElapse.shuffleNum--;

				myPicMatrix.Shuffle();
				InvalidateRect(hWnd,NULL,TRUE);

				if(myTimeElapse.totalSecond-100<0)
					myTimeElapse.totalSecond =0;
				else
					myTimeElapse.totalSecond -=100;

				break;
			case VK_F4:
				if(myTimeElapse.tipsNum<1)
				{
					MessageBox( hWnd, "链接提示已用完","信息", MB_OK );
					break;
				}
				myTimeElapse.tipsNum--;
				int id1, id2;
				bFound = myPicMatrix.autoSearchLink(/*&myPicMatrix,*/&id1,&id2);
				if(bFound)
				{
					myPicMatrix.tipLink( id1, id2 );

					if(myTimeElapse.totalSecond-50<0)
						myTimeElapse.totalSecond =0;
					else
						myTimeElapse.totalSecond -=50;
				}
				break;
			case VK_F6:
				myTimeElapse.Pause();
				if(myTimeElapse.bPause)
				{
					hdc=GetDC(hWnd);
					LoadAndShowPicture(hWnd, hdc, (LPCTSTR)IDR_PAUSE);
					ReleaseDC(hWnd,hdc);
				}
				else
				{
					LoadAndShowPicture(hWnd, NULL, (LPCTSTR)IDR_WELCOME);
					InvalidateRect(hWnd,NULL,TRUE);
				}
				break;
			}
		}
		break;
	case WM_LBUTTONDOWN:
		if( bGameStarted && !myTimeElapse.bPause )
		{
			pPos.x=LOWORD(lParam);
			pPos.y=HIWORD(lParam);
			myPicMatrix.pictureClick( pPos );
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		if(bGameStarted && !myTimeElapse.bPause)
		{
			myPicMatrix.picMatrixDraw( hdc );
		}
		else
		{
			//LoadAndShowPicture(hWnd, hdc, (LPCTSTR)IDB_BITMAP_WELCOME);
			LoadAndShowPicture(hWnd, hdc, (LPCTSTR)IDR_WELCOME);
		}
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

/*
void LoadAndShowPicture(HWND hWnd,HDC hDC,LPCTSTR pRes)
{
	HBITMAP hBitmap;
    BITMAP  bitmap;
	HDC hdcMem;

    hBitmap = LoadBitmap(hInst, pRes);
    GetObject(hBitmap, sizeof (BITMAP), &bitmap);

    hdcMem = CreateCompatibleDC(hDC);
    SelectObject(hdcMem, hBitmap);

	RECT rc;
	GetClientRect( hWnd, &rc );
    BitBlt(hDC, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
	//BitBlt(hDC, 0, 0, rc.right-rc.left, rc.bottom-rc.top, hdcMem, 0, 0, SRCCOPY);

    DeleteDC(hdcMem);
    DeleteObject(hBitmap);
}
*/

void LoadAndShowPicture(HWND hWnd,HDC hDC,LPCTSTR pRes)
{
	HRSRC hsrc;
	DWORD rsize;
	HGLOBAL hGlobal;
	HGLOBAL hGlobalrc;	
	LPVOID pData;
	LPVOID pDatarc;
	IStream *pStream;
	IPicture *pPic;
	HRESULT hresult;

	RECT rc;
	GetClientRect(hWnd, &rc);

	hsrc=FindResource((HMODULE)GetModuleHandle(NULL),(LPCTSTR)pRes,(LPCTSTR)"JPEG");
	if(hsrc)
	{
		rsize=SizeofResource((HMODULE)GetModuleHandle(NULL),hsrc);
		hGlobal=LoadResource((HMODULE)GetModuleHandle(NULL),hsrc);
		if(hGlobal)
		{
			hGlobalrc=GlobalAlloc(GMEM_MOVEABLE,rsize);
		
			pData=GlobalLock(hGlobal);
			pDatarc=GlobalLock(hGlobalrc);
			memcpy((void*)pDatarc,(const void*)pData,rsize);
			GlobalUnlock(hGlobalrc);
			GlobalUnlock(hGlobal);
			
			/*hresult=*/CreateStreamOnHGlobal(hGlobalrc,TRUE,&pStream);
						//装入图形文件     
			hresult=OleLoadPicture(pStream,0,FALSE,IID_IPicture,(LPVOID*)&pPic);
			//hresult=OleLoadPicture(pStream,rsize,TRUE,IID_IPicture,(LPVOID*)&pPic); 

			if( FAILED(hresult) )
				return ;
			
			OLE_XSIZE_HIMETRIC srcWidth;
			OLE_YSIZE_HIMETRIC srcHeight;
			pPic->get_Width(&srcWidth);
			pPic->get_Height(&srcHeight);
			
			pPic->Render(hDC, 0, 0, rc.right, rc.bottom, 0, srcHeight, srcWidth, -srcHeight, NULL);
			pPic->Release();
			pStream->Release();
		}
		GlobalFree( hGlobalrc );
	}
	FreeResource( hsrc );

}