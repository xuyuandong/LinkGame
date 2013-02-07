// ltimeelapse.h
#pragma once

#define TOTALTIME 3500
#define MAXNUM 2

class LTimeElapse
{
public:
	LTimeElapse();
	~LTimeElapse();
	void LTimeElapseInit(HWND hParent,int totalSecond);

public:
	void Begin();
	void Stop();
	void Pause();
	void InvalidIt(HWND hParent);

	void SetTotalTime(int totalSecond);
	void ShowElapseTime();
	static LRESULT CALLBACK	ThreadElapseTime(LPARAM lParam);
		
	/*void StartMidiSound();
	void PlayMidiSound();
	void StopMidiSound();
	void CloseMidiSound();*/

public:
	int totalSecond;
	int tipsNum;
	int shuffleNum;
	static BOOL bPause;

private:
	HWND hParent;
	HBITMAP hBitmap;

	//BOOL bBackMidi;

	static BOOL threadExit;
	MCI_PLAY_PARMS PlayParms;
	MCI_OPEN_PARMS OpenParms; 

};