#include "stdafx.h"
#include "resource.h"
#include "LTimeElapse.h"

#define ShowMessage(X) MessageBox(NULL,X,"发生错误",MB_OK|MB_TOPMOST|MB_ICONWARNING); \
	PostQuitMessage(0);

BOOL LTimeElapse::threadExit = FALSE;
BOOL LTimeElapse::bPause = FALSE;

LTimeElapse::LTimeElapse()
{
	shuffleNum = MAXNUM;
	tipsNum = MAXNUM;
	//bBackMidi = TRUE;
}

LTimeElapse::~LTimeElapse()
{
}

void LTimeElapse::LTimeElapseInit(HWND hParent, int totalSecond)
{
	this->hParent =hParent;
	this->totalSecond =totalSecond;
}

void LTimeElapse::SetTotalTime(int totalSecond)
{
	this->totalSecond = totalSecond;
}

void LTimeElapse::ShowElapseTime()
{
	HDC hDC;
	hDC=GetDC(hParent);
	SetTextColor(hDC,RGB(255,0,0));

	HFONT hfont,oldfont;
	hfont=CreateFont(45,0,0,0,0,0,0,0,0,0,0,0,0,FONTNAME);
	oldfont=(HFONT)SelectObject(hDC,hfont);

	char szTimeElapse[10];
	wsprintf((LPTSTR)szTimeElapse,"  %d  ",totalSecond);

	RECT rcDraw;
	rcDraw.top =10;
	rcDraw.bottom= 50;
	rcDraw.left=0;
	rcDraw.right =800;
	DrawText(hDC,(LPCTSTR)szTimeElapse,strlen(szTimeElapse),&rcDraw,DT_CENTER|DT_VCENTER|DT_SINGLELINE);

	SelectObject(hDC,oldfont);
	DeleteObject(hfont);
	ReleaseDC(hParent,hDC);
}

void LTimeElapse::Begin()
{
	HANDLE hThread;
	DWORD did;
	threadExit=FALSE;

	hThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadElapseTime,(LPVOID)this,0,&did);

	/*if(bBackMidi)
		PlayMidiSound();*/

	if(hThread)
	{
		CloseHandle(hThread);
	}
	else
	{
		ShowMessage("计时线程创建失败");
	}
}

void LTimeElapse::Stop()
{
	threadExit=TRUE;
	/*if(bBackMidi){
		mciSendCommand (OpenParms.wDeviceID,MCI_PAUSE,0,(DWORD)(LPVOID)&PlayParms);
		mciSendCommand (OpenParms.wDeviceID, MCI_CLOSE, NULL, NULL);
	}*/
}

void LTimeElapse::Pause()
{
	bPause=!bPause;
	/*if(bPause  && bBackMidi)
	{
		StopMidiSound();
	}
	else if(bBackMidi)
		PlayMidiSound();*/
}

/*
void LTimeElapse::StartMidiSound()
{
	bBackMidi=TRUE;
	OpenParms.lpstrDeviceType =(LPCSTR) MCI_DEVTYPE_SEQUENCER;//MIDI类型 
	OpenParms.lpstrElementName = (LPCSTR) "backsound"; 
	OpenParms.wDeviceID = 0; 
	OpenParms.dwCallback=0;

	MCIERROR mer;
	mer=mciSendCommand (NULL, MCI_OPEN,MCI_WAIT | MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT,(DWORD)(LPVOID) &OpenParms);
	if(!mer)
	{
		PlayParms.dwFrom = 0;
		mciSendCommand( OpenParms.wDeviceID,MCI_PLAY, MCI_FROM, (DWORD)(LPVOID)&PlayParms );	
	}
}

void LTimeElapse::PlayMidiSound()
{
	mciSendCommand(OpenParms.wDeviceID,MCI_PLAY,MCI_FROM,(DWORD)(LPVOID)&PlayParms);
}

void LTimeElapse::StopMidiSound()
{
	mciSendCommand (OpenParms.wDeviceID,MCI_PAUSE,0,(DWORD)(LPVOID)&PlayParms);
}

void LTimeElapse::CloseMidiSound()
{
	mciSendCommand (OpenParms.wDeviceID,MCI_PAUSE,0,(DWORD)(LPVOID)&PlayParms);
	mciSendCommand (OpenParms.wDeviceID, MCI_CLOSE, NULL, NULL);
	bBackMidi=FALSE;
}
*/

LRESULT CALLBACK LTimeElapse::ThreadElapseTime(LPARAM lParam)
{
	LTimeElapse *pElapse;
	pElapse=(LTimeElapse*)lParam;

	while(!threadExit && pElapse->totalSecond !=0)
	{
		if(!bPause)
		{
			pElapse->ShowElapseTime();
			pElapse->totalSecond --;
		}
		Sleep(100);
	}

	if(pElapse->totalSecond==0)
		SendMessage(pElapse->hParent,OUT_OF_TIME,0,0);

	return 0;
}

