
// Fanuc_Robot_Interface.h: Fanuc_Robot_Interface 應用程式的主要標頭檔
//
#pragma once

#ifndef __AFXWIN_H__
	#error "對 PCH 在包含此檔案前先包含 'pch.h'"
#endif

#include "resource.h"       // 主要符號
#include "Robot_Ctrl.h"
#include <map>



// CFanucRobotInterfaceApp:
// 查看 Fanuc_Robot_Interface.cpp 以了解此類別的實作
//

class CFanucRobotInterfaceApp : public CWinApp
{
public:
	CFanucRobotInterfaceApp() noexcept;


// 覆寫
public:
	virtual BOOL InitInstance();

// 程式碼實作
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()	

//Robot參數
public:
	void Load_Path_Setting();
	void Save_Path_Setting();
public:
	std::map<CString, std::vector<Robot_Pos>> All_Path;
};

extern CFanucRobotInterfaceApp theApp;
