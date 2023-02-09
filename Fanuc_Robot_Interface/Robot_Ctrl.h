#pragma once

#include "pch.h"
#include <vector>


//using namespace std;

#define		ROBOT_MSG_UPDATE				(WM_USER+1000)   

enum MainState
{
	IDLE = 0,
	START_MOVE,
	WAIT_MOTION_FINISH,
	WAIT_NEXT_MOTION,
	STOP,
	WAIT_STOP_ON,
	WAIT_RESUME_STOP,
	RESUME_STOP,
	WAIT_RESUME_STOP_SUCESS,
	PAUSE,
	WAIT_PAUSE_ON,
	WAIT_RESUME_PAUSE,
	RESUME_PAUSE,
	WAIT_RESUME_PAUSE_SUCESS
};

enum InfoType
{
	Alarm,
	Position,
	State,
	MsgString
};

struct RobotMsg {
	CString Time;
	InfoType Type;
	LPVOID Content;
};

enum Axis
{
	X = 0,
	Y,
	Z,
	W,
	P,
	R,
	J1,
	J2,
	J3,
	J4,
	J5,
	J6	
};

enum MotionType
{
	Linear_Motion = 0,
	Joint_Motion,
	NoneType
};

struct Robot_Pos {
	float Pos[12]; //依序為 X, Y, Z, W, P, R , J1, J2, J3, J4, J5, J6	
	MotionType Type;
	int Velocity;
	int Index;
	Robot_Pos() {
		for (int i = Axis::X; i <= Axis::J6; i++)
			Pos[i] = -999;
		Type = NoneType;
		Velocity = 0;
	}
};

struct AlarmInfo {
	int AlarmID;
	bool IsClear;
	int Index;
	CString AlarmMessage;
	CString CauseAlarmMessage;
	CString SeverityMessage;
};

struct MachineState {
	int Index;
	MainState MState;	
};

//---Connect---
int Add_Robot(CString IP); //return robot index, if index < 0, add fail
bool Remove_Robot(int index);

//---moiton---
bool Move(int index, std::vector<Robot_Pos> ThePath, int dest_Pos_Num, bool forward);
bool StepMove(int index, Axis axis, double step, bool forward);
void Robot_Stop(int index, bool ForceStop); //true 則 stop robot, false則解除stop
void Robot_Pause(int index, bool ForcePause);
int Robot_Main_State(int index);
void Robot_SetTeachEnable(int index, bool isEnable);
void Robot_SetAuto(int index, bool isAuto);

//---UI---
void Robot_AssignUI(HWND View_Parent);