#include "pch.h"

#include "Robot_Ctrl.h"

#include <iostream> //for input, output
#include <consoleapi.h> //for AllocConsole()
#include <gcroot.h>
#include <conio.h>

# define ROBOT_DEBUG false
# define CYCLE_INTERVAL 10 //太短會讓Info或Motion一直擁有lock無釋放的時間,若無釋放則其他必須要取得lock才能執行的function無法執行

using namespace FRRJIf;
using namespace bridgeRobotIF;
using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

CWinThread* ProcessThread = NULL;

CWinThread* InfoThread = NULL;

enum OperateMode
{
	Teach = 0,
	Auto
};

struct Robot_Environment
{
	Robot_Environment(CString SetIP) {
		IP = SetIP;

		bridgeCommon::setEncode("Shift-Jis");
		pCore = gcnew bridgeCore(gcnew Core(bridgeCommon::getEncode()));
		pDataTable = gcnew bridgeDataTable(pCore->get_DataTable());
		pCurPos = pDataTable->AddCurPosUF(FRIF_DATA_TYPE::CURPOS, 1, 15); //argument2: 1 means world frame	
		pLibCore = gcnew bridgeLibCore(pCore->get_Snpx());
		pPosReg = pDataTable->AddPosReg(FRIF_DATA_TYPE::POSREG, 1, 1, 10);
		pAlarm = pDataTable->AddAlarm(FRIF_DATA_TYPE::ALARM_LIST, 100, 0);
		pNumReg = pDataTable->AddNumReg(FRRJIf::FRIF_DATA_TYPE::NUMREG_INT, 1, 5);
	}

	CString IP;
	bool Is_Connect = false;
	int  Index = -1;
	int Retry = 0;

	gcroot<bridgeCore^> pCore;
	gcroot<bridgeDataTable^> pDataTable;
	gcroot<bridgeDataCurPos^> pCurPos; //argument2: 1 means world frame	
	gcroot<bridgeLibCore^> pLibCore;
	gcroot<bridgeDataPosReg^> pPosReg;
	gcroot<bridgeDataAlarm^> pAlarm;
	gcroot<bridgeDataNumReg^> pNumReg;

	int MState = IDLE;
	int Pre_MState = IDLE;
	int Keep_MState = IDLE;
	bool IsNewMState = false;
	bool IsStop = false;
	bool IsPause = false;

	bool TeachMotionEnable = false;

	enum OperateMode
	{
		Teach = 0,
		Auto
	} _OperateMode = Teach;	 

	enum MotionMode
	{
		Single_Position = 0,
		Mutiple_Position
	} _MotionMode = Single_Position;

	struct RobotPosInfo {
		float Current_Coordinate[12]; //X,Y,Z.W.P.,R,J1~J6
		float Pre_Coordinate[12];
		float Goal_Coordinate[12];

		float E1;
		float E2;
		float E3;
		short C1;
		short C2;
		short C3;
		short C4;
		short C5;
		short C6;
		short C7;
		short User_Frame_World;
		short Tool_Frame;
		short ValidC;

		
		//float Goal_Joint[6];

		int Velocity;

		float J7_Angle;
		float J8_Angle;
		float J9_Angle;
		short User_Frame_Joint;
		short ValidJ;

		MotionType Moving_Type = Linear_Motion;

		RobotPosInfo()
		{
			for (int i = Axis::X; i <= Axis::J6; i++)
			{		
				Pre_Coordinate[i] = 0;
				Current_Coordinate[i] = 0;
				Goal_Coordinate[i] = 0;				
			}

			Velocity = 0;
		}
	} Position;

	struct RobotPath
	{		
		int Cur_Pos_Id;
		int Goal_Pos_Id;
		int Next_Pos_Id;
		std::vector<Robot_Pos> Path;
		bool Forward = true;
		RobotPath()
		{			
			Cur_Pos_Id = 0;
			Goal_Pos_Id = 0;
			Next_Pos_Id = 0;
		}
	} Current_Path;

	value struct RobotIO {
		static array<short>^ UO = gcnew array<short>(20) { 0 }; // robot->pc system signal
		static array<short>^ DO = gcnew array<short>(20) { 0 }; // robot->pc general signal
		static array<short>^ DI = gcnew array<short>(40) { 0 }; // pc->robot general signal
	} InOut;

	std::vector<AlarmInfo> Current_Alarm;
	std::vector<AlarmInfo> Pre_Alarm;	
};

Robot_Environment* Robot[5];

CCriticalSection CriticalSection;

HWND Parent_UI = NULL;

int Counter1 = 0;
int Counter2 = 0;

int Connect_Num = 0;

afx_msg void SendMsgToUI(InfoType Type, LPVOID Msg)
{
	SYSTEMTIME TimeTemp;
	GetLocalTime(&TimeTemp);
	CString Time;
	Time.Format(L"%02d:%02d:%02d   ", TimeTemp.wHour, TimeTemp.wMinute, TimeTemp.wSecond);

#if ROBOT_DEBUG
	static FILE* stream;

	if (stream == NULL)
	{
		//為呼叫程序配置新的主控台
		AllocConsole();

		//重定向輸出位置到命令列
		freopen_s(&stream, "CON", "w", stdout);
		freopen_s(&stream, "CON", "r", stdin);
		freopen_s(&stream, "CON", "r", stderr);
	}

	if (Type == MsgString)
	{
		String^ sMsg = gcnew String((wchar_t*)Msg);
		Console::WriteLine(sMsg);
	}
#endif

	if (Parent_UI != NULL)
	{
		RobotMsg DeliverMsg;
		DeliverMsg.Type = Type;
		DeliverMsg.Time = Time;

		if (Type == MsgString)
		{
			DeliverMsg.Content = (char*)Msg;
		}
		else
			DeliverMsg.Content = Msg;

		SendMessage(Parent_UI, ROBOT_MSG_UPDATE, (WPARAM)&DeliverMsg, 0);
	}
};

bool Robot_Init(int index)
{
	if (index >= 0 && Robot[index] != NULL)
	{
		Robot[index]->InOut.DI[0] = 0;
		Robot[index]->InOut.DI[1] = 0;
		Robot[index]->InOut.DI[2] = 0;
		Robot[index]->InOut.DI[7] = 0;
		Robot[index]->InOut.DI[8] = 0;

		if (Robot[index]->pLibCore->WriteSDI(1, Robot[index]->InOut.DI, Robot[index]->InOut.DI->Length) <= 0)
		{
			SendMsgToUI(InfoType::MsgString, L"Command Fail");
			return false;
		}

		Robot[index]->InOut.DI[0] = 1;
		Robot[index]->InOut.DI[1] = 1;
		Robot[index]->InOut.DI[2] = 1;
		Robot[index]->InOut.DI[7] = 1;
		Robot[index]->InOut.DI[3] = 1;
		Robot[index]->InOut.DI[4] = 1;

		if (Robot[index]->pLibCore->WriteSDI(1, Robot[index]->InOut.DI, Robot[index]->InOut.DI->Length) <= 0)
		{
			SendMsgToUI(InfoType::MsgString, L"Command Fail");
			return false;
		}

		Sleep(500);

		Robot[index]->InOut.DI[3] = 0;
		Robot[index]->InOut.DI[4] = 0;

		if (Robot[index]->pLibCore->WriteSDI(1, Robot[index]->InOut.DI, Robot[index]->InOut.DI->Length) <= 0)
		{
			SendMsgToUI(InfoType::MsgString, L"Command Fail");
			return false;
		}

		Sleep(900); //測試結果:700 ms比較能夠成功初始化手臂

		Robot[index]->InOut.DI[8] = 1;

		if (Robot[index]->pLibCore->WriteSDI(1, Robot[index]->InOut.DI, Robot[index]->InOut.DI->Length) <= 0)
		{
			SendMsgToUI(InfoType::MsgString, L"Command Fail");
			return false;
		}
	}
	else
	{
		SendMsgToUI(InfoType::MsgString, L"Robot is not exist, Initial Fail");
		return false;
	}

	return true;

	//預計新增確認初始化成功
}

void GoStep(int index, int step)
{
	Robot[index]->Pre_MState = Robot[index]->MState;
	Robot[index]->MState = step;
	Robot[index]->IsNewMState = true;	
};

void Refresh_Robot_Info(Robot_Environment* aRobot)
{
	//Sleep(500);  	

	//更新IO
	aRobot->pLibCore->ReadUO(1, aRobot->InOut.UO, aRobot->InOut.UO->Length);

	aRobot->pLibCore->ReadSDO(1, aRobot->InOut.DO, aRobot->InOut.DO->Length);

	//更新位置資訊
	aRobot->pDataTable->Refresh();

	if (aRobot->pCurPos->get_Valid())
	{
		aRobot->pCurPos->GetValueXyzwpr(aRobot->Position.Current_Coordinate[Axis::X],
			aRobot->Position.Current_Coordinate[Axis::Y],
			aRobot->Position.Current_Coordinate[Axis::Z],
			aRobot->Position.Current_Coordinate[Axis::W],
			aRobot->Position.Current_Coordinate[Axis::P],
			aRobot->Position.Current_Coordinate[Axis::R],
			aRobot->Position.E1, aRobot->Position.E2, aRobot->Position.E3, aRobot->Position.C1, aRobot->Position.C2, aRobot->Position.C3, aRobot->Position.C4, aRobot->Position.C5, aRobot->Position.C6, aRobot->Position.C7, aRobot->Position.User_Frame_World, aRobot->Position.Tool_Frame, aRobot->Position.ValidC);

		aRobot->pCurPos->GetValueJoint(aRobot->Position.Current_Coordinate[Axis::J1],
			aRobot->Position.Current_Coordinate[Axis::J2],
			aRobot->Position.Current_Coordinate[Axis::J3],
			aRobot->Position.Current_Coordinate[Axis::J4],
			aRobot->Position.Current_Coordinate[Axis::J5],
			aRobot->Position.Current_Coordinate[Axis::J6],
			aRobot->Position.J7_Angle, aRobot->Position.J8_Angle, aRobot->Position.J9_Angle, aRobot->Position.User_Frame_Joint, aRobot->Position.ValidJ);

		if (aRobot->Position.Current_Coordinate[Axis::J1] != aRobot->Position.Pre_Coordinate[Axis::J1] ||
			aRobot->Position.Current_Coordinate[Axis::J2] != aRobot->Position.Pre_Coordinate[Axis::J2] ||
			aRobot->Position.Current_Coordinate[Axis::J3] != aRobot->Position.Pre_Coordinate[Axis::J3] ||
			aRobot->Position.Current_Coordinate[Axis::J4] != aRobot->Position.Pre_Coordinate[Axis::J4] ||
			aRobot->Position.Current_Coordinate[Axis::J5] != aRobot->Position.Pre_Coordinate[Axis::J5] ||
			aRobot->Position.Current_Coordinate[Axis::J6] != aRobot->Position.Pre_Coordinate[Axis::J6])
		{
			Robot_Pos Position;
			Position.Index = aRobot->Index;
			for (int i = Axis::X; i <= Axis::J6; i++)
			{
				aRobot->Position.Pre_Coordinate[i] = aRobot->Position.Current_Coordinate[i];
				Position.Pos[i] = aRobot->Position.Current_Coordinate[i];
			}
			//SendMsgToUI(InfoType::Position, &Position);

			/*Robot_Pos Position2;
			Position2.Coor = Joint;
			Position2.Index = aRobot->Index;*/
			
			SendMsgToUI(InfoType::Position, &Position);
		}
	}

	/*Counter++;

	if (Counter * CYCLE_INTERVAL > 500)
	{
		FILE* fp1;
		fp1 = fopen("D:\\Test.csv", "w");

		fprintf(fp1, "XYZ, %f, %f, %f, %f, %f, %f\n", aRobot->Current_Position.XYZ[0], aRobot->Current_Position.XYZ[1], aRobot->Current_Position.XYZ[2], aRobot->Current_Position.XYZ[3], aRobot->Current_Position.XYZ[4], aRobot->Current_Position.XYZ[5]);
		fprintf(fp1, "JOINT, %f, %f, %f, %f, %f, %f\n", aRobot->Current_Position.Joint[0], aRobot->Current_Position.Joint[1], aRobot->Current_Position.Joint[2], aRobot->Current_Position.Joint[3], aRobot->Current_Position.Joint[4], aRobot->Current_Position.Joint[5]);
		fclose(fp1);

		Counter = 0;
	}*/

	//更新alarm history中的alarm,alarm history的alarm不等於現在正在發生的異常,而是曾經發生過的所有異常
	if (aRobot->pAlarm->get_Valid())
	{
		short AlarmID = 0, AlarmNumber = 0, CauseID = 0, CauseAlarmNumber = 0, Severity = 0, Year = 0, Month = 0, Day = 0, Hour = 0, Minute = 0, Second = 0;
		String^ AlarmMessage = "";
		String^ CauseAlarmMessage = "";
		String^ SeverityMessage = "";

		aRobot->Pre_Alarm.clear();

		if (aRobot->Current_Alarm.size() > 0)
		{
			for (int i = 0; i < aRobot->Current_Alarm.size(); i++)
			{
				aRobot->Pre_Alarm.push_back(aRobot->Current_Alarm[i]);
			}
		}

		aRobot->Current_Alarm.clear();
		bool New_Alarm = true;

		for (int i = 1; i <= 5; i++)
		{
			//從原廠library測試結果知道,函式呼叫時若有取得alarm history不代表alrm發生,只能代表alarm發生過
			//若沒取得alarm history代表呼叫ClearAlarm(0)到現在都沒有alarm發生
			aRobot->pAlarm->GetValue(i,
				AlarmID,
				AlarmNumber,
				CauseID,
				CauseAlarmNumber,
				Severity,
				Year,
				Month,
				Day,
				Hour,
				Minute,
				Second, AlarmMessage, CauseAlarmMessage, SeverityMessage);

			//AlarmID=Alarm種類, AlarmNumber=在這個Alarm種類下的其他Alarm
			int FullId = AlarmID * 100000 + AlarmNumber;

			if (AlarmID > 0 && AlarmNumber > 0)
			{
				New_Alarm = true;

				if (aRobot->Current_Alarm.size() > 0)
				{
					//過濾掉重複的alarm
					for (int j = 0; j < aRobot->Current_Alarm.size(); j++)
					{
						if (aRobot->Current_Alarm[j].AlarmID == FullId)
						{
							New_Alarm = false;
							break;
						}
					}
				}

				if (New_Alarm)
				{
					AlarmInfo Alarm;
					Alarm.AlarmID = FullId;
					Alarm.IsClear = !New_Alarm;
					Alarm.AlarmMessage = (CString)AlarmMessage;
					Alarm.CauseAlarmMessage = (CString)CauseAlarmMessage;
					Alarm.SeverityMessage = (CString)SeverityMessage;
					Alarm.Index = aRobot->Index;
					aRobot->Current_Alarm.push_back(Alarm);

					if (aRobot->Pre_Alarm.size() > 0)
					{
						for (int j = 0; j < aRobot->Pre_Alarm.size(); j++)
						{
							if (aRobot->Pre_Alarm[j].AlarmID == FullId)
							{
								New_Alarm = false;
								break;
							}
						}
					}

					if (New_Alarm)
					{
						Alarm.IsClear = !New_Alarm;
						SendMsgToUI(InfoType::Alarm, &Alarm);
					}
				}
			}
		}

		if (aRobot->Pre_Alarm.size() > 0)
		{
			bool Is_Clear = true;
			for (int i = 0; i < aRobot->Pre_Alarm.size(); i++)
			{
				Is_Clear = true;
				for (int j = 0; j < aRobot->Current_Alarm.size(); j++)
				{
					if (aRobot->Current_Alarm[j].AlarmID == aRobot->Pre_Alarm[i].AlarmID)
					{
						Is_Clear = false;
						break;
					}
				}

				if (Is_Clear)
				{
					aRobot->Pre_Alarm[i].IsClear = Is_Clear;
					SendMsgToUI(InfoType::Alarm, &aRobot->Pre_Alarm[i]);
				}
			}
		}
	}
};

//void Action_Process(Robot_Environment* aRobot)
//{
//	switch (aRobot->ActState)
//	{
//	case ACTION_IDLE:
//	{
//		if (aRobot->Pre_ActState != aRobot->ActState)
//		{
//			aRobot->Pre_ActState = aRobot->ActState;
//			SendMsgToUI(InfoType::MsgString, L"Act State = Action Idle");
//		}
//	}
//	break;
//	case START_MOVE://設定動作內容,觸發動作
//	{
//		if (aRobot->Pre_ActState != aRobot->ActState)
//		{
//			aRobot->Pre_ActState = aRobot->ActState;
//			SendMsgToUI(InfoType::MsgString, L"Act State = Start Move");
//		}
//
//		if (aRobot->InOut.DO[0] == 1) //確認手臂到位訊號是否觸發
//		{
//			Robot_Stop(aRobot->Index, true);
//			aRobot->ActState = ACTION_IDLE;
//			return;
//		}
//
//		/*LARGE_INTEGER ticksPerSecond;
//		LARGE_INTEGER start_tick;
//		LARGE_INTEGER end_tick;
//		double	elapsed;
//		QueryPerformanceFrequency(&ticksPerSecond);*/
//		//FILE* fCSV;
//		//fopen_s(&fCSV, "D:\\Counter.csv", "a");
//
//		if (aRobot->_MotionMode == Robot_Environment::Mutiple_Position)
//		{
//			if (aRobot->Current_Path.Goal_Pos_Id > aRobot->Current_Path.Cur_Pos_Id)
//			{
//				aRobot->Current_Path.Next_Pos_Id = aRobot->Current_Path.Cur_Pos_Id + 1;
//			}
//
//			if (aRobot->Current_Path.Goal_Pos_Id < aRobot->Current_Path.Cur_Pos_Id)
//			{
//				aRobot->Current_Path.Next_Pos_Id = aRobot->Current_Path.Cur_Pos_Id - 1;
//			}
//
//			//若起始位置等於終點位置則停止運動
//			if (aRobot->Current_Path.Goal_Pos_Id == aRobot->Current_Path.Cur_Pos_Id)
//			{
//				aRobot->ActState = ACTION_IDLE;
//				break;
//			}
//
//			if (aRobot->Current_Path.Forward)
//			{
//				aRobot->Current_Position.Moving_Type = aRobot->Current_Path.Path[aRobot->Current_Path.Next_Pos_Id].Type;
//				aRobot->Current_Position.Motion_Coordinate = aRobot->Current_Path.Path[aRobot->Current_Path.Next_Pos_Id].Coor;
//				aRobot->Current_Position.Velocity = aRobot->Current_Path.Path[aRobot->Current_Path.Next_Pos_Id].Velocity;
//			}
//			else
//			{
//				aRobot->Current_Position.Moving_Type = aRobot->Current_Path.Path[aRobot->Current_Path.Next_Pos_Id + 1].Type;
//				aRobot->Current_Position.Motion_Coordinate = aRobot->Current_Path.Path[aRobot->Current_Path.Next_Pos_Id + 1].Coor;
//				aRobot->Current_Position.Velocity = aRobot->Current_Path.Path[aRobot->Current_Path.Next_Pos_Id + 1].Velocity;
//			}
//
//			if (aRobot->Current_Position.Moving_Type == Linear_Motion)
//			{
//				for (int i = 0; i < 6; i++)
//					aRobot->Current_Position.Goal_XYZ[i] = aRobot->Current_Path.Path[aRobot->Current_Path.Next_Pos_Id].Pos[i];
//			}
//
//			if (aRobot->Current_Position.Moving_Type == Joint_Motion)
//			{
//				/*CString sTemp1;
//				sTemp1.Format(L"Cur %f  , %f, %f, %f, %f, %f", aRobot->Current_Path.Path[aRobot->Current_Path.Cur_Pos_Id].Pos[6], aRobot->Current_Path.Path[aRobot->Current_Path.Cur_Pos_Id].Pos[7], aRobot->Current_Path.Path[aRobot->Current_Path.Cur_Pos_Id].Pos[8], aRobot->Current_Path.Path[aRobot->Current_Path.Cur_Pos_Id].Pos[9], aRobot->Current_Path.Path[aRobot->Current_Path.Cur_Pos_Id].Pos[10], aRobot->Current_Path.Path[aRobot->Current_Path.Cur_Pos_Id].Pos[11]);
//				AfxMessageBox(sTemp1);*/
//
//				for (int i = 6; i < 12; i++)
//					aRobot->Current_Position.Goal_Joint[i - 6] = aRobot->Current_Path.Path[aRobot->Current_Path.Next_Pos_Id].Pos[i];
//
//				
//				/*sTemp1.Format(L"Next %f  , %f, %f, %f, %f, %f", aRobot->Current_Position.Goal_Joint[0], aRobot->Current_Position.Goal_Joint[1], aRobot->Current_Position.Goal_Joint[2], aRobot->Current_Position.Goal_Joint[3], aRobot->Current_Position.Goal_Joint[4], aRobot->Current_Position.Goal_Joint[5]);
//				AfxMessageBox(sTemp1);*/
//			}		
//		}
//
//		if (aRobot->Current_Position.Moving_Type == Linear_Motion)
//		{
//			array<int>^ lngValues = gcnew array< int>(100);
//			lngValues[0] = 1;
//
//			if (aRobot->_OperateMode == Robot_Environment::Auto)
//				lngValues[1] = aRobot->Current_Position.Velocity;
//			else
//				lngValues[1] = 30;
//			//QueryPerformanceCounter(&start_tick);
//			aRobot->pNumReg->SetValuesInt(aRobot->pNumReg->GetStartIndex(), lngValues, aRobot->pNumReg->GetEndIndex() - aRobot->pNumReg->GetStartIndex() + 1);
//			
//			aRobot->pPosReg->SetValueXyzwpr(aRobot->pPosReg->GetStartIndex(),
//				aRobot->Current_Position.Goal_XYZ[0],
//				aRobot->Current_Position.Goal_XYZ[1],
//				aRobot->Current_Position.Goal_XYZ[2],
//				aRobot->Current_Position.Goal_XYZ[3],
//				aRobot->Current_Position.Goal_XYZ[4],
//				aRobot->Current_Position.Goal_XYZ[5],
//				aRobot->Current_Position.E1, aRobot->Current_Position.E2, aRobot->Current_Position.E3, aRobot->Current_Position.C1, aRobot->Current_Position.C2, aRobot->Current_Position.C3, aRobot->Current_Position.C4, aRobot->Current_Position.C5, aRobot->Current_Position.C6, aRobot->Current_Position.C7, aRobot->Current_Position.User_Frame_World, aRobot->Current_Position.Tool_Frame);
//			
//			/*CString sTemp;
//			sTemp.Format(L"Current_Position.Goal_XYZ[3] = %f", aRobot->Current_Position.Goal_XYZ[3]);
//			AfxMessageBox(sTemp);
//
//			sTemp.Format(L"Current_Position.Goal_XYZ[5] = %f", aRobot->Current_Position.Goal_XYZ[5]);
//			AfxMessageBox(sTemp);*/
//			//QueryPerformanceCounter(&end_tick);
//			//elapsed = ((double)((end_tick.QuadPart - start_tick.QuadPart) * 1000.0) / (double)ticksPerSecond.QuadPart);
//			//fprintf(fCSV, "%lf,", elapsed);
//		}
//
//		if (aRobot->Current_Position.Moving_Type == Joint_Motion)
//		{
//			array<int>^ lngValues = gcnew array< int>(100);
//			lngValues[0] = 2;
//
//			if (aRobot->_OperateMode == Robot_Environment::Auto)
//				lngValues[1] = aRobot->Current_Position.Velocity;
//			else
//				lngValues[1] = 30;
//			//QueryPerformanceCounter(&start_tick);
//			aRobot->pNumReg->SetValuesInt(aRobot->pNumReg->GetStartIndex(), lngValues, aRobot->pNumReg->GetEndIndex() - aRobot->pNumReg->GetStartIndex() + 1);
//
//			aRobot->pPosReg->SetValueJoint(aRobot->pPosReg->GetStartIndex(),
//				aRobot->Current_Position.Goal_Joint[0],
//				aRobot->Current_Position.Goal_Joint[1],
//				aRobot->Current_Position.Goal_Joint[2],
//				aRobot->Current_Position.Goal_Joint[3],
//				aRobot->Current_Position.Goal_Joint[4],
//				aRobot->Current_Position.Goal_Joint[5],
//				aRobot->Current_Position.J7_Angle, aRobot->Current_Position.J8_Angle, aRobot->Current_Position.J9_Angle, aRobot->Current_Position.User_Frame_Joint, aRobot->Current_Position.Tool_Frame);
//			
//			/*CString sTemp1;
//			sTemp1.Format(L"%f  , %f, %f, %f, %f, %f", aRobot->Current_Position.Goal_Joint[0], aRobot->Current_Position.Goal_Joint[1], aRobot->Current_Position.Goal_Joint[2], aRobot->Current_Position.Goal_Joint[3], aRobot->Current_Position.Goal_Joint[4], aRobot->Current_Position.Goal_Joint[5]);
//			AfxMessageBox(sTemp1);*/
//			//QueryPerformanceCounter(&end_tick);
//			//elapsed = ((double)((end_tick.QuadPart - start_tick.QuadPart) * 1000.0) / (double)ticksPerSecond.QuadPart);
//			//fprintf(fCSV, "%lf,", elapsed);
//		}
//
//		//if (aRobot->Current_Position.Motion_Coordinate == Linear_Motion)
//		//{
//		//	//QueryPerformanceCounter(&start_tick);
//		//	aRobot->pPosReg->SetValueXyzwpr(aRobot->pPosReg->GetStartIndex(),
//		//		aRobot->Current_Position.Goal_XYZ[0],
//		//		aRobot->Current_Position.Goal_XYZ[1],
//		//		aRobot->Current_Position.Goal_XYZ[2],
//		//		aRobot->Current_Position.Goal_XYZ[3],
//		//		aRobot->Current_Position.Goal_XYZ[4],
//		//		aRobot->Current_Position.Goal_XYZ[5],
//		//		aRobot->Current_Position.E1, aRobot->Current_Position.E2, aRobot->Current_Position.E3, aRobot->Current_Position.C1, aRobot->Current_Position.C2, aRobot->Current_Position.C3, aRobot->Current_Position.C4, aRobot->Current_Position.C5, aRobot->Current_Position.C6, aRobot->Current_Position.C7, aRobot->Current_Position.User_Frame_World, aRobot->Current_Position.Tool_Frame);
//		//	//QueryPerformanceCounter(&end_tick);
//		//	//elapsed = ((double)((end_tick.QuadPart - start_tick.QuadPart) * 1000.0) / (double)ticksPerSecond.QuadPart);
//		//	//fprintf(fCSV, "%lf,", elapsed);
//		//}
//
//		//if (aRobot->Current_Position.Motion_Coordinate == Joint_Motion)
//		//{
//		//	//QueryPerformanceCounter(&start_tick);
//		//	aRobot->pPosReg->SetValueJoint(aRobot->pPosReg->GetStartIndex(),
//		//		aRobot->Current_Position.Goal_Joint[0],
//		//		aRobot->Current_Position.Goal_Joint[1],
//		//		aRobot->Current_Position.Goal_Joint[2],
//		//		aRobot->Current_Position.Goal_Joint[3],
//		//		aRobot->Current_Position.Goal_Joint[4],
//		//		aRobot->Current_Position.Goal_Joint[5],
//		//		aRobot->Current_Position.J7_Angle, aRobot->Current_Position.J8_Angle, aRobot->Current_Position.J9_Angle, aRobot->Current_Position.User_Frame_Joint, aRobot->Current_Position.Tool_Frame);
//		//	//QueryPerformanceCounter(&end_tick);
//		//	//elapsed = ((double)((end_tick.QuadPart - start_tick.QuadPart) * 1000.0) / (double)ticksPerSecond.QuadPart);
//		//	//fprintf(fCSV, "%lf,", elapsed);
//		//}
//
//		aRobot->InOut.DI[18] = 1;
//
//		if (aRobot->pLibCore->WriteSDI(1, aRobot->InOut.DI, aRobot->InOut.DI->Length) <= 0)
//		{
//			Robot_Stop(aRobot->Index, true);
//			aRobot->ActState = ACTION_IDLE;
//			return;
//		}
//
//		if (aRobot->_OperateMode == Robot_Environment::Auto)
//		{
//			CString sTemp;
//			sTemp.Format(L"Robot%d Go To %ls_%d", aRobot->Index, aRobot->Current_Path.Path_Id, aRobot->Current_Path.Next_Pos_Id);
//			char* cTemp;
//			cTemp = (char*)(LPCTSTR)sTemp;
//			SendMsgToUI(InfoType::MsgString, cTemp);
//		}
//
//		aRobot->ActState = WAIT_MOTION_FINISH;
//	}
//	break;
//	case WAIT_MOTION_FINISH://等待動作完成,確認是否繼續執行
//	{
//		if ((aRobot->_OperateMode == Robot_Environment::Teach && aRobot->TeachMotionEnable) ||
//			aRobot->_OperateMode == Robot_Environment::Auto)
//		{
//			if (aRobot->Pre_ActState != aRobot->ActState)
//			{
//				aRobot->Pre_ActState = aRobot->ActState;
//				SendMsgToUI(InfoType::MsgString, L"Act State = Wait Motion Finish");
//			}
//
//			if (aRobot->InOut.DO[0] == 1)//等待到位訊號
//			{
//				aRobot->InOut.DI[18] = 0; //關閉運動觸發訊號
//
//				if (aRobot->pLibCore->WriteSDI(1, aRobot->InOut.DI, aRobot->InOut.DI->Length) <= 0)
//				{
//					Robot_Stop(aRobot->Index, true);
//					aRobot->ActState = ACTION_IDLE;
//					return;
//				}
//				else
//					aRobot->ActState = WAIT_NEXT_MOTION;
//			}
//		}
//		else
//			Robot_Stop(aRobot->Index, true);
//	}
//	break;
//	case WAIT_NEXT_MOTION: //到位訊號反轉有時間差
//		if (aRobot->Pre_ActState != aRobot->ActState)
//		{
//			aRobot->Pre_ActState = aRobot->ActState;
//			SendMsgToUI(InfoType::MsgString, L"Act State = Wait Next Motion");
//		}
//
//		if (aRobot->InOut.DO[0] == 0)
//		{
//			if (aRobot->_MotionMode == Robot_Environment::Single_Position)
//			{
//				aRobot->ActState = ACTION_IDLE;
//			}
//
//			if (aRobot->_MotionMode == Robot_Environment::Mutiple_Position)
//			{
//				aRobot->Current_Path.Cur_Pos_Id = aRobot->Current_Path.Next_Pos_Id;
//
//				if (aRobot->Current_Path.Goal_Pos_Id == aRobot->Current_Path.Cur_Pos_Id)
//				{
//					aRobot->ActState = ACTION_IDLE;
//					SendMsgToUI(InfoType::MsgString, L"Arrive");
//				}
//				else
//					aRobot->ActState = START_MOVE;
//			}
//		}
//		break;
//	}
//};

void Machine_Process(Robot_Environment* aRobot)
{
	//確認暫停訊號是否觸發
	//if (aRobot->InOut.UO[2] == 0 &&  //robot程式執行中
	//	aRobot->InOut.UO[9] == 0 &&  //程式執行中或教導器操作中
	//	aRobot->InOut.UO[3] == 1)  //程式暫停
	//{
	//	if (aRobot->MState != PAUSE)
	//		Robot_Pause(aRobot->Index, true);
	//}

	//確認異常是否發生
	if (aRobot->InOut.UO[5] == 1)
	{
		if (aRobot->MState != STOP)
			GoStep(aRobot->Index, STOP);			
	}

	//確認PC與robot交握流程是否異常
	if (aRobot->InOut.DI[18] == 0 &&
		aRobot->InOut.DO[0] == 1 &&
		aRobot->MState == IDLE)
	{
		GoStep(aRobot->Index, STOP);
	}

	switch (aRobot->MState)
	{
	case IDLE:
		break;
#pragma region Run Process
	case START_MOVE:

		if (aRobot->InOut.DO[0] == 1) //確認手臂到位訊號是否關閉
		{
			aRobot->Retry++;

			if(aRobot->Retry* CYCLE_INTERVAL > 1000)
				Robot_Stop(aRobot->Index, true);			
		}
		else
		{
			/*LARGE_INTEGER ticksPerSecond;
			LARGE_INTEGER start_tick;
			LARGE_INTEGER end_tick;
			double	elapsed;
			QueryPerformanceFrequency(&ticksPerSecond);*/
			//FILE* fCSV;
			//fopen_s(&fCSV, "D:\\Counter.csv", "a");

			if (aRobot->_MotionMode == Robot_Environment::Mutiple_Position &&
				aRobot->Current_Path.Goal_Pos_Id == aRobot->Current_Path.Cur_Pos_Id)
			{
				GoStep(aRobot->Index, IDLE);
			}
			else
			{
				if (aRobot->_MotionMode == Robot_Environment::Mutiple_Position)
				{
					if (aRobot->Current_Path.Goal_Pos_Id > aRobot->Current_Path.Cur_Pos_Id)
					{
						aRobot->Current_Path.Next_Pos_Id = aRobot->Current_Path.Cur_Pos_Id + 1;
					}

					if (aRobot->Current_Path.Goal_Pos_Id < aRobot->Current_Path.Cur_Pos_Id)
					{
						aRobot->Current_Path.Next_Pos_Id = aRobot->Current_Path.Cur_Pos_Id - 1;
					}

					if (aRobot->Current_Path.Forward)
					{
						aRobot->Position.Moving_Type = aRobot->Current_Path.Path[aRobot->Current_Path.Next_Pos_Id].Type;
						aRobot->Position.Velocity = aRobot->Current_Path.Path[aRobot->Current_Path.Next_Pos_Id].Velocity;
					}
					else
					{
						aRobot->Position.Moving_Type = aRobot->Current_Path.Path[aRobot->Current_Path.Next_Pos_Id + 1].Type;
						aRobot->Position.Velocity = aRobot->Current_Path.Path[aRobot->Current_Path.Next_Pos_Id + 1].Velocity;
					}

					if (aRobot->Position.Moving_Type == Linear_Motion)
					{
						for (int i = Axis::X; i <= Axis::R; i++)
							aRobot->Position.Goal_Coordinate[i] = aRobot->Current_Path.Path[aRobot->Current_Path.Next_Pos_Id].Pos[i];
					}

					if (aRobot->Position.Moving_Type == Joint_Motion)
					{
						/*CString sTemp1;
						sTemp1.Format(L"Cur %f  , %f, %f, %f, %f, %f", aRobot->Current_Path.Path[aRobot->Current_Path.Cur_Pos_Id].Pos[6], aRobot->Current_Path.Path[aRobot->Current_Path.Cur_Pos_Id].Pos[7], aRobot->Current_Path.Path[aRobot->Current_Path.Cur_Pos_Id].Pos[8], aRobot->Current_Path.Path[aRobot->Current_Path.Cur_Pos_Id].Pos[9], aRobot->Current_Path.Path[aRobot->Current_Path.Cur_Pos_Id].Pos[10], aRobot->Current_Path.Path[aRobot->Current_Path.Cur_Pos_Id].Pos[11]);
						AfxMessageBox(sTemp1);*/

						for (int i = Axis::J1; i <= Axis::J6; i++)
							aRobot->Position.Goal_Coordinate[i] = aRobot->Current_Path.Path[aRobot->Current_Path.Next_Pos_Id].Pos[i];


						/*sTemp1.Format(L"Next %f  , %f, %f, %f, %f, %f", aRobot->Current_Position.Goal_Joint[0], aRobot->Current_Position.Goal_Joint[1], aRobot->Current_Position.Goal_Joint[2], aRobot->Current_Position.Goal_Joint[3], aRobot->Current_Position.Goal_Joint[4], aRobot->Current_Position.Goal_Joint[5]);
						AfxMessageBox(sTemp1);*/
					}
				}

				if (aRobot->Position.Moving_Type == Linear_Motion)
				{
					array<int>^ lngValues = gcnew array< int>(100);
					lngValues[0] = 1;

					if (aRobot->_OperateMode == Robot_Environment::Auto)
						lngValues[1] = aRobot->Position.Velocity;
					else
						lngValues[1] = 30;
					//QueryPerformanceCounter(&start_tick);
					if (aRobot->pNumReg->SetValuesInt(aRobot->pNumReg->GetStartIndex(), lngValues, aRobot->pNumReg->GetEndIndex() - aRobot->pNumReg->GetStartIndex() + 1))
					{
						if (aRobot->pPosReg->SetValueXyzwpr(aRobot->pPosReg->GetStartIndex(),
							aRobot->Position.Goal_Coordinate[Axis::X],
							aRobot->Position.Goal_Coordinate[Axis::Y],
							aRobot->Position.Goal_Coordinate[Axis::Z],
							aRobot->Position.Goal_Coordinate[Axis::W],
							aRobot->Position.Goal_Coordinate[Axis::P],
							aRobot->Position.Goal_Coordinate[Axis::R],
							aRobot->Position.E1, aRobot->Position.E2, aRobot->Position.E3, aRobot->Position.C1, aRobot->Position.C2, aRobot->Position.C3, aRobot->Position.C4, aRobot->Position.C5, aRobot->Position.C6, aRobot->Position.C7, aRobot->Position.User_Frame_World, aRobot->Position.Tool_Frame))
						{
							aRobot->InOut.DI[18] = 1;

							if (aRobot->pLibCore->WriteSDI(1, aRobot->InOut.DI, aRobot->InOut.DI->Length) > 0)
							{
								GoStep(aRobot->Index, WAIT_MOTION_FINISH);
							}
							else
							{
								aRobot->Retry++;

								if (aRobot->Retry > 15)
								{
									GoStep(aRobot->Index, STOP);
								}
							}
						}
						else
						{
							aRobot->Retry++;

							if (aRobot->Retry > 10)
							{
								GoStep(aRobot->Index, STOP);
							}
						}
					}
					else
					{
						aRobot->Retry++;

						if (aRobot->Retry > 5)
						{
							GoStep(aRobot->Index, STOP);
						}
					}

					

					/*CString sTemp;
					sTemp.Format(L"Current_Position.Goal_XYZ[3] = %f", aRobot->Current_Position.Goal_XYZ[3]);
					AfxMessageBox(sTemp);

					sTemp.Format(L"Current_Position.Goal_XYZ[5] = %f", aRobot->Current_Position.Goal_XYZ[5]);
					AfxMessageBox(sTemp);*/
					//QueryPerformanceCounter(&end_tick);
					//elapsed = ((double)((end_tick.QuadPart - start_tick.QuadPart) * 1000.0) / (double)ticksPerSecond.QuadPart);
					//fprintf(fCSV, "%lf,", elapsed);
				}

				if (aRobot->Position.Moving_Type == Joint_Motion)
				{
					array<int>^ lngValues = gcnew array< int>(100);
					lngValues[0] = 2;

					if (aRobot->_OperateMode == Robot_Environment::Auto)
						lngValues[1] = aRobot->Position.Velocity;
					else
						lngValues[1] = 30;
					//QueryPerformanceCounter(&start_tick);
					if (aRobot->pNumReg->SetValuesInt(aRobot->pNumReg->GetStartIndex(), lngValues, aRobot->pNumReg->GetEndIndex() - aRobot->pNumReg->GetStartIndex() + 1))
					{
						if (aRobot->pPosReg->SetValueJoint(aRobot->pPosReg->GetStartIndex(),
							aRobot->Position.Goal_Coordinate[Axis::J1],
							aRobot->Position.Goal_Coordinate[Axis::J2],
							aRobot->Position.Goal_Coordinate[Axis::J3],
							aRobot->Position.Goal_Coordinate[Axis::J4],
							aRobot->Position.Goal_Coordinate[Axis::J5],
							aRobot->Position.Goal_Coordinate[Axis::J6],
							aRobot->Position.J7_Angle, aRobot->Position.J8_Angle, aRobot->Position.J9_Angle, aRobot->Position.User_Frame_Joint, aRobot->Position.Tool_Frame))
						{
							aRobot->InOut.DI[18] = 1;

							if (aRobot->pLibCore->WriteSDI(1, aRobot->InOut.DI, aRobot->InOut.DI->Length) > 0)
							{
								GoStep(aRobot->Index, WAIT_MOTION_FINISH);
							}
							else
							{
								aRobot->Retry++;

								if (aRobot->Retry > 15)
								{
									GoStep(aRobot->Index, STOP);
								}
							}
						}
						else
						{
							aRobot->Retry++;

							if (aRobot->Retry > 10)
							{
								GoStep(aRobot->Index, STOP);
							}
						}
					}
					else
					{
						aRobot->Retry++;

						if (aRobot->Retry > 5)
						{
							GoStep(aRobot->Index, STOP);
						}
					}					
				}				
			}			
		}	

		break;
	case WAIT_MOTION_FINISH:

		if ((aRobot->_OperateMode == Robot_Environment::Teach && aRobot->TeachMotionEnable) ||
			aRobot->_OperateMode == Robot_Environment::Auto)
		{
			if (aRobot->InOut.DO[0] == 1)//等待到位訊號
			{
				aRobot->InOut.DI[18] = 0; //關閉運動觸發訊號

				if (aRobot->pLibCore->WriteSDI(1, aRobot->InOut.DI, aRobot->InOut.DI->Length) > 0)
				{
					GoStep(aRobot->Index, WAIT_NEXT_MOTION);					
				}
				else
				{
					aRobot->Retry++;

					if(aRobot->Retry > 10)
						GoStep(aRobot->Index, STOP);
				}
			}
			else
			{
				aRobot->Retry++;

				if (aRobot->Retry * CYCLE_INTERVAL > 2000)
				{
					GoStep(aRobot->Index, STOP);
				}
			}
		}
		else
			Robot_Stop(aRobot->Index, true);

		break;
	case WAIT_NEXT_MOTION:

		if (aRobot->InOut.DO[0] == 0)
		{
			if (aRobot->_MotionMode == Robot_Environment::Single_Position)
			{
				GoStep(aRobot->Index, IDLE);
			}

			if (aRobot->_MotionMode == Robot_Environment::Mutiple_Position)
			{
				aRobot->Current_Path.Cur_Pos_Id = aRobot->Current_Path.Next_Pos_Id;

				if (aRobot->Current_Path.Goal_Pos_Id == aRobot->Current_Path.Cur_Pos_Id)
				{
					GoStep(aRobot->Index, IDLE);					
				}
				else
					GoStep(aRobot->Index, START_MOVE);					
			}
		}
		else
		{
			aRobot->Retry++;

			if (aRobot->Retry * CYCLE_INTERVAL > 2000)
			{
				GoStep(aRobot->Index, STOP);
			}
		}

		break;
#pragma endregion
#pragma region STOP Process
	case STOP:

		//急停可由模組內部發現異常後觸發或外部主動觸發
		//第一次進入STOP,且IsStop==true,則觸發robot急停訊號
		aRobot->InOut.DI[0] = 0; //開啟急停訊號
		aRobot->InOut.DI[1] = 0; //強制中止Robot內部執行的程式
		aRobot->InOut.DI[2] = 0; //不允許機器人的動作
		aRobot->InOut.DI[7] = 0; //禁止手臂程式運轉
		aRobot->InOut.DI[8] = 0; //禁止手臂程式運轉
		aRobot->InOut.DI[18] = 0; //關閉運動觸發訊號

		if (aRobot->pLibCore->WriteSDI(1, aRobot->InOut.DI, aRobot->InOut.DI->Length) > 0)
			GoStep(aRobot->Index, WAIT_STOP_ON);
		
		break;
	case WAIT_STOP_ON:

		if(aRobot->InOut.UO[5] == 1)
			GoStep(aRobot->Index, WAIT_RESUME_STOP);
		else
		{
			aRobot->Retry++;

			if (aRobot->Retry * CYCLE_INTERVAL > 1000)
			{
				GoStep(aRobot->Index, STOP);
			}
		}

		break;
	case WAIT_RESUME_STOP:
		break;
	case RESUME_STOP:
		
		//IsStop==false且robot訊號異常off則嘗試reset alarm
		if (Robot_Init(aRobot->Index))
		{
			GoStep(aRobot->Index, WAIT_RESUME_STOP_SUCESS);
		}
		else
		{
			aRobot->Retry++;

			//異常情況仍然存在InOut.UO[5] == 1,必須重新由模組外下指令將IsStop變回false
			if (aRobot->Retry * CYCLE_INTERVAL > 3000)
			{
				Robot_Stop(aRobot->Index, true);
			}
		}		

		break;
	case WAIT_RESUME_STOP_SUCESS:

		//IsStop必須由外部變為false且robot異常訊號off,MState方可回到IDLE狀態後進行動作		
		if (aRobot->InOut.UO[0] == 1 &&
			aRobot->InOut.UO[1] == 1 &&
			aRobot->InOut.UO[2] == 1 &&
			aRobot->InOut.UO[9] == 1 &&
			aRobot->InOut.UO[3] == 0 &&
			aRobot->InOut.UO[4] == 0 &&
			aRobot->InOut.UO[5] == 0 &&
			aRobot->InOut.UO[6] == 0 &&
			aRobot->InOut.UO[7] == 0 &&
			aRobot->InOut.UO[8] == 0)
		{			
			aRobot->pCore->ClearAlarm(0); //reset alarm成功,清除alarm hitory中的alarm
			GoStep(aRobot->Index, IDLE);
		}

		break;
#pragma endregion
#pragma region PAUSE Process
	case PAUSE:

		aRobot->InOut.DI[1] = 0; //機器人減速至停止,暫停執行中的程式,

		if (aRobot->pLibCore->WriteSDI(1, aRobot->InOut.DI, aRobot->InOut.DI->Length) > 0)
		{
			aRobot->Keep_MState = aRobot->Pre_MState;

			GoStep(aRobot->Index, WAIT_PAUSE_ON);
		}
		else
		{
			aRobot->Retry++;

			if (aRobot->Retry > 10)
				Robot_Stop(aRobot->Index, true);
		}			

		break;
	case WAIT_PAUSE_ON:

		if (aRobot->InOut.UO[3] == 1)
		{
			GoStep(aRobot->Index, WAIT_RESUME_PAUSE);
		}
		else
		{
			aRobot->Retry++;

			if (aRobot->Retry * CYCLE_INTERVAL > 3000)
			{
				Robot_Stop(aRobot->Index, true);				
			}			
		}		

		break;
	case WAIT_RESUME_PAUSE:
		break;
	case RESUME_PAUSE:

		//解除暫停		
		aRobot->InOut.DI[1] = 1;
		aRobot->InOut.DI[5] = 1;  //重新執行程式,下緣觸發						

		if (aRobot->pLibCore->WriteSDI(1, aRobot->InOut.DI, aRobot->InOut.DI->Length) > 0)
		{
			Sleep(500);

			aRobot->InOut.DI[5] = 0;

			if (aRobot->pLibCore->WriteSDI(1, aRobot->InOut.DI, aRobot->InOut.DI->Length) > 0)
			{
				GoStep(aRobot->Index, WAIT_RESUME_PAUSE_SUCESS);
			}
			else
			{
				aRobot->Retry++;

				if (aRobot->Retry > 15)
					Robot_Stop(aRobot->Index, true);
			}
		}
		else
		{
			aRobot->Retry++;

			if (aRobot->Retry > 10)
				Robot_Stop(aRobot->Index, true);
		}

		break;	
	case WAIT_RESUME_PAUSE_SUCESS:

		if (aRobot->InOut.UO[3] == 0)
		{
			GoStep(aRobot->Index, aRobot->Keep_MState);
		}
		else
		{
			aRobot->Retry++;

			if (aRobot->Retry * CYCLE_INTERVAL > 3000)
			{
				GoStep(aRobot->Index, WAIT_RESUME_PAUSE);
			}			
		}

		break;
#pragma endregion
	}
}

//正常1, 2. 3, 10
//暫停1,2,4,5
//停止4,6

UINT Thread_Motion_Process(LPVOID param)
{
	SendMsgToUI(InfoType::MsgString, L"Start Robot Cycle");

	while (Connect_Num > 0)
	{
		/*LARGE_INTEGER ticksPerSecond;
		LARGE_INTEGER start_tick;
		LARGE_INTEGER end_tick;
		double	elapsed;
		QueryPerformanceFrequency(&ticksPerSecond);
		FILE* fCSV;
		fopen_s(&fCSV, "D:\\Motion.csv", "a");
		QueryPerformanceCounter(&start_tick);*/

		Sleep(CYCLE_INTERVAL);

#if ROBOT_DEBUG
		Counter1++;
		if (Counter1 % 50 == 0)
		{
			Console::WriteLine("Robot Motion Process Alive " + Counter1 / 10);
			if (Counter1 > 100)
				Counter1 = 0;
		}
#endif

		for (int i = 0; i < 5; i++)
		{
			if (Robot[i] != NULL && Robot[i]->Is_Connect)
			{
				Refresh_Robot_Info(Robot[i]);

				if (Robot[i]->IsNewMState)
				{
					Robot[i]->Pre_MState = Robot[i]->MState;
					Robot[i]->Retry = 0;
				}

				Machine_Process(Robot[i]);
			}
		}		
	}

	SendMsgToUI(InfoType::MsgString, L"Stop Robot Cycle");

	return 1;
};

int Add_Robot(CString IP)
{
	Robot_Environment* aRobot = new Robot_Environment(IP);

	for (int i = 0; i < 5; i++)
	{
		if (Robot[i] == NULL)
		{
			aRobot->Index = i;
			Robot[i] = aRobot;

			Robot[i]->pCore->set_TimeOutValue(1000);

			if (Robot[i]->Is_Connect = Robot[i]->pCore->Connect(gcnew String(Robot[i]->IP)))
			{
				if (Robot[i]->pLibCore->WriteSDI(1, Robot[i]->InOut.DI, 40) <= 0)
				{
					AfxMessageBox(L"Write Initial DI Signal Error");
					Remove_Robot(i);
					aRobot->Index = -1;
				}
				else
				{
					Connect_Num++;
					SendMsgToUI(InfoType::MsgString, L"Suceed To Connect Robot");
				}
			}
			else
			{
				SendMsgToUI(InfoType::MsgString, L"Fail To Connect Robot");
				aRobot->Index = -1;
			}
			
			break;
		}
	}

	if (aRobot->Index >= 0)
	{
		if (Robot_Init(aRobot->Index))
		{
			if (ProcessThread == NULL)
				ProcessThread = AfxBeginThread(Thread_Motion_Process, NULL, THREAD_PRIORITY_HIGHEST);

			SendMsgToUI(InfoType::MsgString, L"Sucess to Init Robot");
			return aRobot->Index;
		}
		else
		{
			SendMsgToUI(InfoType::MsgString, L"Fail to Init Robot");
			Robot[aRobot->Index] = NULL;
			delete aRobot;
			return -1;
		}
	}
	else
	{
		Robot[aRobot->Index] = NULL;
		delete aRobot;
		SendMsgToUI(InfoType::MsgString, L"Fail to Add Robot");
		return -1;
	}
}

bool Remove_Robot(int index)
{
	SendMsgToUI(InfoType::MsgString, L"Robot Disconnect");
	//UnActive_Robot();

	Robot[index]->Is_Connect = !Robot[index]->pCore->Disconnect();

	if (!Robot[index]->Is_Connect)
	{
		if ((bridgeCore^)Robot[index]->pCore != nullptr)
			delete Robot[index]->pCore;

		if ((bridgeLibCore^)Robot[index]->pLibCore != nullptr)
			delete Robot[index]->pLibCore;

		if ((bridgeDataTable^)Robot[index]->pDataTable != nullptr)
			delete Robot[index]->pDataTable;

		if ((bridgeDataCurPos^)Robot[index]->pCurPos != nullptr)
			delete Robot[index]->pCurPos;

		if ((bridgeDataAlarm^)Robot[index]->pAlarm != nullptr)
			delete Robot[index]->pAlarm;

		Connect_Num--;

		delete Robot[index];

		return true;
	}
	else
		return false;	
};

bool StepMove(int index, Axis axis, double step, bool forward)
{
	if (Robot[index]->MState == IDLE && Robot[index]->Is_Connect)
	{
		Robot[index]->_OperateMode = Robot_Environment::Teach;
		Robot[index]->_MotionMode = Robot_Environment::Single_Position;
		
		for (int i = Axis::X; i <= Axis::J6; i++)
		{
			Robot[index]->Position.Goal_Coordinate[i] = Robot[index]->Position.Current_Coordinate[i];			
		}		

		if (axis == Axis::X || axis == Axis::Y || axis == Axis::Z || axis == Axis::W || axis == Axis::P || axis == Axis::R)		
			Robot[index]->Position.Moving_Type = Linear_Motion;
		else
			Robot[index]->Position.Moving_Type = Joint_Motion;

		if (forward)
			Robot[index]->Position.Goal_Coordinate[axis] += step;
		else
			Robot[index]->Position.Goal_Coordinate[axis] -= step;		

		/*FILE* fp1;
		SYSTEMTIME TimeTemp;
		GetLocalTime(&TimeTemp);
		fopen_s(&fp1, "D:\\Move.txt", "a");
		fprintf(fp1, "%d:%d:%d:%d    X = %.2f Y=%.2f Z= %.2f\n", TimeTemp.wHour, TimeTemp.wMinute, TimeTemp.wSecond, TimeTemp.wMilliseconds, Goal_X, Goal_Y, Goal_Z);
		fclose(fp1);*/
		GoStep(index, START_MOVE);	

		return true;
	}
	else
		return false;
};

bool Move(int index, std::vector<Robot_Pos> Path, int dest_Pos_Num, bool forward)
{
	if (Robot[index]->Is_Connect)
	{
		if (Robot[index]->MState == IDLE)
		{
			for (int i = 0; i < Path.size(); i++)
			{
				if (Path[i].Type == NoneType)
				{
					AfxMessageBox(L"Set Coordinate Fail!!!");
					
					return false;
				}

				if (Path[i].Pos[Axis::X] == -999 ||
					Path[i].Pos[Axis::Y] == -999 ||
					Path[i].Pos[Axis::Z] == -999 ||
					Path[i].Pos[Axis::W] == -999 ||
					Path[i].Pos[Axis::P] == -999 ||
					Path[i].Pos[Axis::R] == -999 ||
					Path[i].Pos[Axis::J1] == -999 ||
					Path[i].Pos[Axis::J2] == -999 ||
					Path[i].Pos[Axis::J3] == -999 ||
					Path[i].Pos[Axis::J4] == -999 ||
					Path[i].Pos[Axis::J5] == -999 ||
					Path[i].Pos[Axis::J6] == -999)
				{
					AfxMessageBox(L"Set Postion Fail!!!");
					return false;
				}

				if (Path[i].Velocity < 0 || Path[i].Velocity > 100)
				{
					AfxMessageBox(L"Set Velocity Fail!!!");
					return false;
				}
			}

			Robot[index]->Current_Path.Path.clear();

			for (int i = 0; i < Path.size(); i++)
			{
				Robot_Pos TmpPath;
				TmpPath.Type = Path[i].Type;

				for (int j = Axis::X; j <= Axis::J6; j++)
					TmpPath.Pos[j] = Path[i].Pos[j];

				TmpPath.Velocity = Path[i].Velocity;
				Robot[index]->Current_Path.Path.push_back(TmpPath);

				/*CString sTemp;
				sTemp.Format(L"%d", i);
				AfxMessageBox(sTemp);*/
			}

			if (dest_Pos_Num >= 0 &&
				dest_Pos_Num <= Robot[index]->Current_Path.Path.size() - 1)
			{
				Robot[index]->Current_Path.Cur_Pos_Id = -1;

				CString sTemp;

				for (int i = 0; i < Robot[index]->Current_Path.Path.size(); i++)
				{
					if (fabs(Robot[index]->Current_Path.Path[i].Pos[Axis::X] - Robot[index]->Position.Current_Coordinate[Axis::X]) < 1 &&
						fabs(Robot[index]->Current_Path.Path[i].Pos[Axis::Y] - Robot[index]->Position.Current_Coordinate[Axis::Y]) < 1 &&
						fabs(Robot[index]->Current_Path.Path[i].Pos[Axis::Z] - Robot[index]->Position.Current_Coordinate[Axis::Z]) < 1 &&
						fabs(Robot[index]->Current_Path.Path[i].Pos[Axis::J1] - Robot[index]->Position.Current_Coordinate[Axis::J1]) < 0.5 &&
						fabs(Robot[index]->Current_Path.Path[i].Pos[Axis::J2] - Robot[index]->Position.Current_Coordinate[Axis::J2]) < 0.5 &&
						fabs(Robot[index]->Current_Path.Path[i].Pos[Axis::J3] - Robot[index]->Position.Current_Coordinate[Axis::J3]) < 0.5 &&
						fabs(Robot[index]->Current_Path.Path[i].Pos[Axis::J4] - Robot[index]->Position.Current_Coordinate[Axis::J4]) < 0.5 &&
						fabs(Robot[index]->Current_Path.Path[i].Pos[Axis::J5] - Robot[index]->Position.Current_Coordinate[Axis::J5]) < 0.5 &&
						fabs(Robot[index]->Current_Path.Path[i].Pos[Axis::J6] - Robot[index]->Position.Current_Coordinate[Axis::J6]) < 0.5)
					{
						//由於WPR的座標值為+180 ~ -180之間,座標值+180等同於-180,因此當作標值位於+-180左右,以額外的方式判斷
						//對於J1~J6則有極限限制,因此不會發生同一位置上可能是+180或-180
						
						float Limit1 = 0.0;
						float Limit2 = 0.0;
						bool WPR_OK[3] = {false, false, false};

						for (int j = Axis::W; j <= Axis::R; j++)
						{
							if (Robot[index]->Position.Current_Coordinate[j] + 0.5 >= 180) //假設當前位置為正,且接近+180時的判斷條件
							{
								Limit1 = Robot[index]->Position.Current_Coordinate[j] - 0.5;
								Limit2 = Robot[index]->Position.Current_Coordinate[j] + 0.5 - 180 - 180;
								if ((Robot[index]->Current_Path.Path[i].Pos[j] <= 180 && Robot[index]->Current_Path.Path[i].Pos[j] >= Limit1) ||
									(Robot[index]->Current_Path.Path[i].Pos[j] >= -180 && Robot[index]->Current_Path.Path[i].Pos[j] <= Limit2))
								{
									WPR_OK[j - Axis::W] = true;
								}
							}
							else if (Robot[index]->Position.Current_Coordinate[j] - 0.5 <= -180) //假設當前位置為負,且接近-180時的判斷條件
							{
								Limit1 = Robot[index]->Position.Current_Coordinate[j] + 0.5;
								Limit2 = Robot[index]->Position.Current_Coordinate[j] - 0.5 + 180 + 180;
								if ((Robot[index]->Current_Path.Path[i].Pos[j] >= -180 && Robot[index]->Current_Path.Path[i].Pos[j] <= Limit1) ||
									(Robot[index]->Current_Path.Path[i].Pos[j] <= 180 && Robot[index]->Current_Path.Path[i].Pos[j] >= Limit2))
								{
									WPR_OK[j - Axis::W] = true;
								}
							}
							else if (fabs(Robot[index]->Current_Path.Path[i].Pos[j] - Robot[index]->Position.Current_Coordinate[j]) < 0.5)
							{
								WPR_OK[j - Axis::W] = true;
							}
						}
						

						if (WPR_OK[0] && WPR_OK[1] && WPR_OK[2])
						{
							Robot[index]->Current_Path.Cur_Pos_Id = i;
							break;
						}
					}
				}

				if (Robot[index]->Current_Path.Cur_Pos_Id == -1)
				{
					CString sTemp;
					sTemp.Format(L"Robot%d Position is Wrong", index);
					AfxMessageBox(sTemp);

					return false;
				}
				else
				{
					Robot[index]->_MotionMode = Robot_Environment::Mutiple_Position;
					Robot[index]->Current_Path.Goal_Pos_Id = dest_Pos_Num;
					Robot[index]->Current_Path.Forward = forward;
					Robot[index]->Current_Path.Next_Pos_Id = 0;			
					GoStep(index, START_MOVE);

					return true;
				}							
			}
			else
				return false;
		}
		else
			return false;
	}
	else
		return false;	
};

void Robot_Stop(int index, bool ForceStop)
{
	if (ForceStop && 
		(Robot[index]->MState > STOP ||
		Robot[index]->MState < WAIT_RESUME_STOP_SUCESS))
	{
		GoStep(Robot[index]->Index, STOP);
	}

	if (!ForceStop && Robot[index]->MState == WAIT_RESUME_STOP)
		GoStep(Robot[index]->Index, RESUME_STOP);
};

void Robot_Pause(int index, bool ForcePause)
{
	if (ForcePause && 
		Robot[index]->MState < STOP && 
		Robot[index]->InOut.UO[3] == 0)
	{			
		GoStep(Robot[index]->Index, PAUSE);
	}

	if (!ForcePause &&
		Robot[index]->MState == WAIT_RESUME_PAUSE &&
		Robot[index]->InOut.UO[3] == 1)
	{			
		GoStep(Robot[index]->Index, RESUME_PAUSE);
	}	
};

void Robot_AssignUI(HWND View_Parent)
{
	Parent_UI = View_Parent;
};

void Robot_SetAuto(int index, bool isAuto)
{
	if (Robot[index] != NULL && Robot[index]->MState == IDLE)
	{
		if (isAuto)
			Robot[index]->_OperateMode = Robot_Environment::Auto;
		else
			Robot[index]->_OperateMode = Robot_Environment::Teach;
	}
};

void Robot_SetTeachEnable(int index, bool isEnable)
{
	CString sTemp;
	if (Robot[index] != NULL)
	{
		if (Robot[index]->TeachMotionEnable != isEnable)
		{
			Robot[index]->TeachMotionEnable = isEnable;
		}

		/*if (isEnable)
		{
			sTemp.Format(L"Auto");
			AfxMessageBox(sTemp);
		}
		else
		{
			sTemp.Format(L"Manual");
			AfxMessageBox(sTemp);
		}*/
		
		
	}
	

};

int Robot_Main_State(int index)
{
	int ret;
	CSingleLock wait(&CriticalSection);
	wait.Lock();
	ret = (int)Robot[index]->MState;
	wait.Unlock();
	return ret;
};









