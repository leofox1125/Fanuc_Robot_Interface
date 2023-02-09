
// Fanuc_Robot_InterfaceView.cpp: CFanucRobotInterfaceView 類別的實作
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以定義在實作預覽、縮圖和搜尋篩選條件處理常式的
// ATL 專案中，並允許與該專案共用文件程式碼。
#ifndef SHARED_HANDLERS
#include "Fanuc_Robot_Interface.h"
#endif

#include "Fanuc_Robot_InterfaceDoc.h"
#include "Fanuc_Robot_InterfaceView.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

bool Stop_Signal = false;
bool Pause_Signal = false;
int Robot_Index = 0;
bool IsEnable = false;

// CFanucRobotInterfaceView

IMPLEMENT_DYNCREATE(CFanucRobotInterfaceView, CFormView)

BEGIN_MESSAGE_MAP(CFanucRobotInterfaceView, CFormView)
	ON_BN_CLICKED(IDC_BTN_CON, &CFanucRobotInterfaceView::OnBnClickedBtnCon)
	ON_BN_CLICKED(IDC_BTN_CON2, &CFanucRobotInterfaceView::OnBnClickedBtnCon2)
	ON_BN_CLICKED(IDC_BUT_ACTIVE_ROBOT, &CFanucRobotInterfaceView::OnBnClickedButActiveRobot)
	ON_BN_CLICKED(IDC_BUT_FORWARD_X, &CFanucRobotInterfaceView::OnBnClickedButForwardX)
	ON_BN_CLICKED(IDC_BUT_BACKWARD_X2, &CFanucRobotInterfaceView::OnBnClickedButBackwardX2)
	ON_BN_CLICKED(IDC_BUT_BACKWARD_X, &CFanucRobotInterfaceView::OnBnClickedButBackwardX)
	ON_BN_CLICKED(IDC_BUT_BACKWARD_Y, &CFanucRobotInterfaceView::OnBnClickedButBackwardY)
	ON_BN_CLICKED(IDC_BUT_FORWARD_Y, &CFanucRobotInterfaceView::OnBnClickedButForwardY)
	ON_BN_CLICKED(IDC_BUT_BACKWARD_Z, &CFanucRobotInterfaceView::OnBnClickedButBackwardZ)
	ON_BN_CLICKED(IDC_BUT_FORWARD_Z, &CFanucRobotInterfaceView::OnBnClickedButForwardZ)
	ON_BN_CLICKED(IDC_BUT_BACKWARD_W, &CFanucRobotInterfaceView::OnBnClickedButBackwardW)
	ON_BN_CLICKED(IDC_BUT_FORWARD_W, &CFanucRobotInterfaceView::OnBnClickedButForwardW)
	ON_BN_CLICKED(IDC_BUT_BACKWARD_P, &CFanucRobotInterfaceView::OnBnClickedButBackwardP)
	ON_BN_CLICKED(IDC_BUT_FORWARD_P, &CFanucRobotInterfaceView::OnBnClickedButForwardP)
	ON_BN_CLICKED(IDC_BUT_BACKWARD_R, &CFanucRobotInterfaceView::OnBnClickedButBackwardR)
	ON_BN_CLICKED(IDC_BUT_FORWARD_R, &CFanucRobotInterfaceView::OnBnClickedButForwardR)
	ON_BN_CLICKED(IDC_BUT_BACKWARD_J1, &CFanucRobotInterfaceView::OnBnClickedButBackwardJ1)
	ON_BN_CLICKED(IDC_BUT_FORWARD_J1, &CFanucRobotInterfaceView::OnBnClickedButForwardJ1)
	ON_BN_CLICKED(IDC_BUT_BACKWARD_J2, &CFanucRobotInterfaceView::OnBnClickedButBackwardJ2)
	ON_BN_CLICKED(IDC_BUT_FORWARD_J2, &CFanucRobotInterfaceView::OnBnClickedButForwardJ2)
	ON_BN_CLICKED(IDC_BUT_BACKWARD_J3, &CFanucRobotInterfaceView::OnBnClickedButBackwardJ3)
	ON_BN_CLICKED(IDC_BUT_FORWARD_J3, &CFanucRobotInterfaceView::OnBnClickedButForwardJ3)
	ON_BN_CLICKED(IDC_BUT_BACKWARD_J4, &CFanucRobotInterfaceView::OnBnClickedButBackwardJ4)
	ON_BN_CLICKED(IDC_BUT_FORWARD_J4, &CFanucRobotInterfaceView::OnBnClickedButForwardJ4)
	ON_BN_CLICKED(IDC_BUT_BACKWARD_J5, &CFanucRobotInterfaceView::OnBnClickedButBackwardJ5)
	ON_BN_CLICKED(IDC_BUT_FORWARD_J5, &CFanucRobotInterfaceView::OnBnClickedButForwardJ5)
	ON_BN_CLICKED(IDC_BUT_BACKWARD_J6, &CFanucRobotInterfaceView::OnBnClickedButBackwardJ6)
	ON_BN_CLICKED(IDC_BUT_FORWARD_J6, &CFanucRobotInterfaceView::OnBnClickedButForwardJ6)
	ON_BN_CLICKED(IDC_BTN_DISCON, &CFanucRobotInterfaceView::OnBnClickedBtnDiscon)
	ON_BN_CLICKED(IDC_BUT_NEWPATH, &CFanucRobotInterfaceView::OnBnClickedButNewpath)
	ON_BN_CLICKED(IDC_BUT_INSERT_POS, &CFanucRobotInterfaceView::OnBnClickedButInsertPos)
	ON_BN_CLICKED(IDC_BUT_DELETE_POS, &CFanucRobotInterfaceView::OnBnClickedButDeletePos)
	ON_BN_CLICKED(IDC_BUT_SAVE_PATH, &CFanucRobotInterfaceView::OnBnClickedButSavePath)
	ON_BN_CLICKED(IDC_BUT_MOTION_TEST, &CFanucRobotInterfaceView::OnBnClickedButMotionTest)
	ON_BN_CLICKED(IDC_BUT_MOTION_TEST_BACKWARD, &CFanucRobotInterfaceView::OnBnClickedButMotionTestBackward)
	ON_BN_CLICKED(IDC_BUT_MOTION_TEST_FORWARD, &CFanucRobotInterfaceView::OnBnClickedButMotionTestForward)
	//ON_BN_CLICKED(IDC_BUTTON7, &CFanucRobotInterfaceView::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUT_STOP, &CFanucRobotInterfaceView::OnBnClickedButStop)
	//ON_BN_CLICKED(IDC_BUTTON8, &CFanucRobotInterfaceView::OnBnClickedButton8)
	ON_CBN_SELCHANGE(IDC_COMBO_PATH_NAME, &CFanucRobotInterfaceView::OnCbnSelchangeComboPathName)
	ON_MESSAGE(ROBOT_MSG_UPDATE, &CFanucRobotInterfaceView::Receive_Robot_Msg)
	ON_BN_CLICKED(IDC_BUT_ESTOP, &CFanucRobotInterfaceView::OnBnClickedButEstop)
	ON_BN_CLICKED(IDC_BUT_PAUSE, &CFanucRobotInterfaceView::OnBnClickedButPause)
	ON_WM_KEYDOWN()
	ON_BN_CLICKED(IDC_BUTTON_SETAUTO, &CFanucRobotInterfaceView::OnBnClickedButtonSetauto)
END_MESSAGE_MAP()

// CFanucRobotInterfaceView 建構/解構

CFanucRobotInterfaceView::CFanucRobotInterfaceView() noexcept
	: CFormView(IDD_FANUC_ROBOT_INTERFACE_FORM)
{
	// TODO: 在此加入建構程式碼

}

CFanucRobotInterfaceView::~CFanucRobotInterfaceView()
{
}

void CFanucRobotInterfaceView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BOOL CFanucRobotInterfaceView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此經由修改 CREATESTRUCT cs 
	// 達到修改視窗類別或樣式的目的

	return CFormView::PreCreateWindow(cs);
}

void CFanucRobotInterfaceView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();
	Robot_Is_Connect = false;

	CString sTemp;

	theApp.Load_Path_Setting();

	sTemp.Format(L"Linear Motion");
	((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->InsertString(-1, sTemp);
	((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE_SETUP))->InsertString(-1, sTemp);
	sTemp.Format(L"Joint Motion");
	((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->InsertString(-1, sTemp);
	((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE_SETUP))->InsertString(-1, sTemp);

	sTemp.Format(L"XYZ");
	((CComboBox*)GetDlgItem(IDC_COMBO_COOR_SYS_SETUP))->InsertString(-1, sTemp);
	sTemp.Format(L"Joint");
	((CComboBox*)GetDlgItem(IDC_COMBO_COOR_SYS_SETUP))->InsertString(-1, sTemp);

	((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_COMBO_COOR_SYS_SETUP))->SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE_SETUP))->SetCurSel(0);

	((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->InsertColumn(0, L"編號", LVCFMT_LEFT, 90, -1);
	((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->InsertColumn(1, L"位置", LVCFMT_LEFT,324, -1);
	((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->InsertColumn(2, L"速度", LVCFMT_LEFT, 80, -1);
	((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->InsertColumn(3, L"座標系", LVCFMT_LEFT, 80, -1);
	((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->InsertColumn(4, L"運動方式", LVCFMT_LEFT, 95, -1);

	for (auto iter = theApp.All_Path.begin(); iter != theApp.All_Path.end(); ++iter)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->AddString(iter->first);
	}

	((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->SetCurSel(0);

	CString PathName;
	((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->GetCurSel(), PathName);

	for (int i = 0; i < theApp.All_Path[PathName].size(); i++)
	{
		sTemp.Format(L"%d", i);
		((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->InsertItem(i, sTemp);
		
		sTemp.Format(L"%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf, %.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf",
			theApp.All_Path[PathName][i].Pos[Axis::X],
			theApp.All_Path[PathName][i].Pos[Axis::Y],
			theApp.All_Path[PathName][i].Pos[Axis::Z],
			theApp.All_Path[PathName][i].Pos[Axis::W],
			theApp.All_Path[PathName][i].Pos[Axis::P],
			theApp.All_Path[PathName][i].Pos[Axis::R],
			theApp.All_Path[PathName][i].Pos[Axis::J1],
			theApp.All_Path[PathName][i].Pos[Axis::J2],
			theApp.All_Path[PathName][i].Pos[Axis::J3],
			theApp.All_Path[PathName][i].Pos[Axis::J3],
			theApp.All_Path[PathName][i].Pos[Axis::J5],
			theApp.All_Path[PathName][i].Pos[Axis::J6]);
		((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->SetItemText(i, 1, sTemp);

		sTemp.Format(L"%d", theApp.All_Path[PathName][i].Velocity);
		((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->SetItemText(i, 2, sTemp);

		if (theApp.All_Path[PathName][i].Type == Linear_Motion)
			sTemp.Format(L"Linear Motion");
		else
			sTemp.Format(L"Joint Motion");
		((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->SetItemText(i, 3, sTemp);
	}

	Robot_AssignUI(this->GetSafeHwnd());

	
}


// CFanucRobotInterfaceView 診斷

#ifdef _DEBUG
void CFanucRobotInterfaceView::AssertValid() const
{
	CFormView::AssertValid();
}

void CFanucRobotInterfaceView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CFanucRobotInterfaceDoc* CFanucRobotInterfaceView::GetDocument() const // 內嵌非偵錯版本
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFanucRobotInterfaceDoc)));
	return (CFanucRobotInterfaceDoc*)m_pDocument;
}
#endif //_DEBUG


// CFanucRobotInterfaceView 訊息處理常式

void CFanucRobotInterfaceView::OnBnClickedBtnCon()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	//AssignUIDisplay((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO));
	CString IP;
	//IP.Format(L"192.168.18.100");
	IP = AfxGetApp()->GetProfileString(_T("Settings"), _T("HostName"), _T(""));
	Robot_Index = Add_Robot(L"192.168.18.100");
	if(Robot_Index >=0)
		Robot_Is_Connect = true;
}


void CFanucRobotInterfaceView::OnBnClickedBtnCon2()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	
}




void CFanucRobotInterfaceView::OnBnClickedButActiveRobot()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	
}


void CFanucRobotInterfaceView::OnBnClickedButForwardX()
{
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;
	
	// TODO: 在此加入控制項告知處理常式程式碼
	if(((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::X, atof(cTemp), Type), true)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButBackwardX2()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;


	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::X, atof(cTemp), Type), false)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButBackwardX()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::X, atof(cTemp), Type), false)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButBackwardY()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::Y, atof(cTemp), Type), false)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButForwardY()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::Y, atof(cTemp), Type), true)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButBackwardZ()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::Z, atof(cTemp), Type), false)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButForwardZ()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::Z, atof(cTemp), Type), true)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButBackwardW()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::W, atof(cTemp), Type), false)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButForwardW()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::W, atof(cTemp), Type), true)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButBackwardP()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::P, atof(cTemp), Type), false)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButForwardP()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::P, atof(cTemp), Type), true)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButBackwardR()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::R, atof(cTemp), Type), false)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButForwardR()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::R, atof(cTemp), Type), true)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButBackwardJ1()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::J1, atof(cTemp), Type), false)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButForwardJ1()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::J1, atof(cTemp), Type), true)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButBackwardJ2()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::J2, atof(cTemp), Type), false)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButForwardJ2()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::J2, atof(cTemp), Type), true)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButBackwardJ3()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::J3, atof(cTemp), Type), false)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButForwardJ3()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::J3, atof(cTemp), Type), true)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButBackwardJ4()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::J4, atof(cTemp), Type), false)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButForwardJ4()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::J4, atof(cTemp), Type), true)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButBackwardJ5()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::J5, atof(cTemp), Type), false)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButForwardJ5()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::J5, atof(cTemp), Type), true)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButBackwardJ6()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::J6, atof(cTemp), Type), false)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButForwardJ6()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString sTemp;
	char	cTemp[64];
	GetDlgItem(IDC_EDIT_MOV_INTERVAL)->GetWindowText(sTemp);
	sprintf_s(cTemp, "%ls", sTemp);
	MotionType Type = Linear_Motion;

	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 0)
		Type = Linear_Motion;
	if (((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE))->GetCurSel() == 1)
		Type = Joint_Motion;

	if (!StepMove(Robot_Index, Axis::J6, atof(cTemp), Type), true)
	{
		AfxMessageBox(_T("Robot is running"));
	}
}


void CFanucRobotInterfaceView::OnBnClickedButBackwardJ7()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	
}


void CFanucRobotInterfaceView::OnBnClickedButForwardJ7()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	
}


void CFanucRobotInterfaceView::OnBnClickedButBackwardJ8()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	
}


void CFanucRobotInterfaceView::OnBnClickedButForwardJ8()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	
}


void CFanucRobotInterfaceView::OnBnClickedButBackwardJ9()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	
}


void CFanucRobotInterfaceView::OnBnClickedButForwardJ9()
{
	// TODO: 在此加入控制項告知處理常式程式碼

}


void CFanucRobotInterfaceView::OnBnClickedBtnDiscon()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	Remove_Robot(Robot_Index);
}


void CFanucRobotInterfaceView::OnBnClickedButton3()
{
	// TODO: 在此加入控制項告知處理常式程式碼
}


void CFanucRobotInterfaceView::OnBnClickedButNewpath()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString NewPathName;
	GetDlgItem(IDC_EDIT_NEWPATH)->GetWindowText(NewPathName);
	if (NewPathName != L"")
	{
		vector<Robot_Pos> NewPath;
		theApp.All_Path.insert(pair <CString, vector < Robot_Pos>>(NewPathName, NewPath));
		for (auto iter = theApp.All_Path.begin(); iter != theApp.All_Path.end(); ++iter)
		{
			((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->AddString(iter->first);			
		}

		CString PathName;
		for (int i = 0; i < ((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->GetCount(); i++)
		{
			((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->GetLBText(i, PathName);
			if (PathName == NewPathName)
			{
				((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->SetCurSel(i);
				break;
			}
		}

		((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->DeleteAllItems();
		
	}
	else
		AfxMessageBox(L"Path Name is Wrong!!!");
}


void CFanucRobotInterfaceView::OnBnClickedButInsertPos()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	//((CComboBox*)GetDlgItem(IDC_COMBO2))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO2))->GetCurSel(), recipe_name);

	//AfxMessageBox(PathNum);
	CString sTemp;
	CString CoorSys;
	((CComboBox*)GetDlgItem(IDC_COMBO_COOR_SYS_SETUP))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO_COOR_SYS_SETUP))->GetCurSel(), CoorSys);

	CString Pos;

	GetDlgItem(IDC_EDIT_XPOS)->GetWindowTextW(sTemp);
	if (sTemp == L"")
	{
		AfxMessageBox(L"Current Position is Wrong!!!");
		return;
	}
	Pos = sTemp;
	GetDlgItem(IDC_EDIT_YPOS)->GetWindowTextW(sTemp);
	if (sTemp == L"")
	{
		AfxMessageBox(L"Current Position is Wrong!!!");
		return;
	}
	Pos = Pos + "," + sTemp;
	GetDlgItem(IDC_EDIT_ZPOS)->GetWindowTextW(sTemp);
	if (sTemp == L"")
	{
		AfxMessageBox(L"Current Position is Wrong!!!");
		return;
	}
	Pos = Pos + "," + sTemp;
	GetDlgItem(IDC_EDIT_WANGLE)->GetWindowTextW(sTemp);
	if (sTemp == L"")
	{
		AfxMessageBox(L"Current Position is Wrong!!!");
		return;
	}
	Pos = Pos + "," + sTemp;
	GetDlgItem(IDC_EDIT_PANGLE)->GetWindowTextW(sTemp);
	if (sTemp == L"")
	{
		AfxMessageBox(L"Current Position is Wrong!!!");
		return;
	}
	Pos = Pos + "," + sTemp;
	GetDlgItem(IDC_EDIT_RANGLE)->GetWindowTextW(sTemp);
	if (sTemp == L"")
	{
		AfxMessageBox(L"Current Position is Wrong!!!");
		return;
	}
	Pos = Pos + "," + sTemp;

	GetDlgItem(IDC_EDIT_J1)->GetWindowTextW(sTemp);
	if (sTemp == L"")
	{
		AfxMessageBox(L"Current Position is Wrong!!!");
		return;
	}
	Pos = Pos + "," + sTemp;
	GetDlgItem(IDC_EDIT_J2)->GetWindowTextW(sTemp);
	if (sTemp == L"")
	{
		AfxMessageBox(L"Current Position is Wrong!!!");
		return;
	}
	Pos = Pos + "," + sTemp;
	GetDlgItem(IDC_EDIT_J3)->GetWindowTextW(sTemp);
	if (sTemp == L"")
	{
		AfxMessageBox(L"Current Position is Wrong!!!");
		return;
	}
	Pos = Pos + "," + sTemp;
	GetDlgItem(IDC_EDIT_J4)->GetWindowTextW(sTemp);
	if (sTemp == L"")
	{
		AfxMessageBox(L"Current Position is Wrong!!!");
		return;
	}
	Pos = Pos + "," + sTemp;
	GetDlgItem(IDC_EDIT_J5)->GetWindowTextW(sTemp);
	if (sTemp == L"")
	{
		AfxMessageBox(L"Current Position is Wrong!!!");
		return;
	}
	Pos = Pos + "," + sTemp;
	GetDlgItem(IDC_EDIT_J6)->GetWindowTextW(sTemp);
	if (sTemp == L"")
	{
		AfxMessageBox(L"Current Position is Wrong!!!");
		return;
	}
	Pos = Pos + "," + sTemp;

	/*if (CoorSys == L"XYZ")
	{
		GetDlgItem(IDC_EDIT_XPOS)->GetWindowTextW(sTemp);
		if (sTemp == L"")
		{
			AfxMessageBox(L"Current Position is Wrong!!!");
			return;
		}
		Pos = sTemp;
		GetDlgItem(IDC_EDIT_YPOS)->GetWindowTextW(sTemp);
		if (sTemp == L"")
		{
			AfxMessageBox(L"Current Position is Wrong!!!");
			return;
		}
		Pos = Pos + "," + sTemp;
		GetDlgItem(IDC_EDIT_ZPOS)->GetWindowTextW(sTemp);
		if (sTemp == L"")
		{
			AfxMessageBox(L"Current Position is Wrong!!!");
			return;
		}
		Pos = Pos + "," + sTemp;
		GetDlgItem(IDC_EDIT_WANGLE)->GetWindowTextW(sTemp);
		if (sTemp == L"")
		{
			AfxMessageBox(L"Current Position is Wrong!!!");
			return;
		}
		Pos = Pos + "," + sTemp;
		GetDlgItem(IDC_EDIT_PANGLE)->GetWindowTextW(sTemp);
		if (sTemp == L"")
		{
			AfxMessageBox(L"Current Position is Wrong!!!");
			return;
		}
		Pos = Pos + "," + sTemp;
		GetDlgItem(IDC_EDIT_RANGLE)->GetWindowTextW(sTemp);
		if (sTemp == L"")
		{
			AfxMessageBox(L"Current Position is Wrong!!!");
			return;
		}
		Pos = Pos + "," + sTemp;
	}
	else if (CoorSys == L"Joint")
	{
		GetDlgItem(IDC_EDIT_J1)->GetWindowTextW(sTemp);
		if (sTemp == L"")
		{
			AfxMessageBox(L"Current Position is Wrong!!!");
			return;
		}
		Pos = sTemp;
		GetDlgItem(IDC_EDIT_J2)->GetWindowTextW(sTemp);
		if (sTemp == L"")
		{
			AfxMessageBox(L"Current Position is Wrong!!!");
			return;
		}
		Pos = Pos + "," + sTemp;
		GetDlgItem(IDC_EDIT_J3)->GetWindowTextW(sTemp);
		if (sTemp == L"")
		{
			AfxMessageBox(L"Current Position is Wrong!!!");
			return;
		}
		Pos = Pos + "," + sTemp;
		GetDlgItem(IDC_EDIT_J4)->GetWindowTextW(sTemp);
		if (sTemp == L"")
		{
			AfxMessageBox(L"Current Position is Wrong!!!");
			return;
		}
		Pos = Pos + "," + sTemp;
		GetDlgItem(IDC_EDIT_J5)->GetWindowTextW(sTemp);
		if (sTemp == L"")
		{
			AfxMessageBox(L"Current Position is Wrong!!!");
			return;
		}
		Pos = Pos + "," + sTemp;
		GetDlgItem(IDC_EDIT_J6)->GetWindowTextW(sTemp);
		if (sTemp == L"")
		{
			AfxMessageBox(L"Current Position is Wrong!!!");
			return;
		}
		Pos = Pos + "," + sTemp;
	}*/
	
	CString Velocity;
	GetDlgItem(IDC_EDIT_VELOCITY_SETUP)->GetWindowText(Velocity);	

	if (_ttoi(Velocity) > 100 || _ttoi(Velocity) <= 0)
	{
		AfxMessageBox(L"Setting Value of Velocity is Wrong!!!");
		return;
	}

	CString Motion;
	((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE_SETUP))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO_MOTION_TYPE_SETUP))->GetCurSel(), Motion);
	
	CString PathNum;
	GetDlgItem(IDC_EDIT_PATH_NUM)->GetWindowText(PathNum);

	/*CString sTemp1;
	sTemp1.Format(L"%d , %d", ((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->GetItemCount(), _ttoi(PathNum));
	AfxMessageBox(sTemp1);*/

	if (((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->GetItemCount() <= _ttoi(PathNum))
	{
		for (int i = ((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->GetItemCount(); i <= _ttoi(PathNum); i++)
		{
			sTemp.Format(L"%d", i);
			((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->InsertItem(i, sTemp);
		}
	}
	((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->SetItemText(_ttoi(PathNum), 1, Pos);
	((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->SetItemText(_ttoi(PathNum), 2, Velocity);
	((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->SetItemText(_ttoi(PathNum), 3, CoorSys);
	((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->SetItemText(_ttoi(PathNum), 4, Motion);
}


void CFanucRobotInterfaceView::OnBnClickedButDeletePos()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString PathNum;
	GetDlgItem(IDC_EDIT_PATH_NUM)->GetWindowText(PathNum);

	if (_ttoi(PathNum) + 1 <=  ((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->GetItemCount())
	{
		CString sTemp;
		
		TCHAR		writebuf[256];
		LVITEM lvi;
		lvi.mask = LVIF_TEXT;
		lvi.pszText = writebuf;
		lvi.cchTextMax = 256;

		for (int i = _ttoi(PathNum) + 1; i <= ((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->GetItemCount(); i++)
		{
			lvi.iItem = i;
			for (int j = 1; j < 5; j++)
			{
				lvi.iSubItem = j;
				((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->GetItem(&lvi);
				sTemp.Format(L"%s", writebuf);
				((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->SetItemText(i - 1, j, sTemp);
			}
		}
		((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->DeleteItem(((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->GetItemCount() - 1);
	}
}

void CFanucRobotInterfaceView::OnBnClickedButSavePath()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString PathName;
	((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->GetCurSel(), PathName);
	
	if (PathName != L"")
	{		
		CString sTemp;

		TCHAR		writebuf[256];
		LVITEM lvi;
		lvi.mask = LVIF_TEXT;
		lvi.pszText = writebuf;
		lvi.cchTextMax = 256;

		theApp.All_Path[PathName].clear();

		for (int i = 0; i < ((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->GetItemCount(); i++)
		{
			Robot_Pos TmpRobotPos;

			CString CoorSys;
			lvi.iItem = i;

			lvi.iSubItem = 3;
			((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->GetItem(&lvi);
			CoorSys.Format(L"%s", writebuf);

			lvi.iSubItem = 1;
			((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->GetItem(&lvi);
			
			for (int j = 0; j < 12; j++)
			{
				AfxExtractSubString(sTemp, writebuf, j, ',');
				TmpRobotPos.Pos[j] = _ttof(sTemp);				
			}	

			
			lvi.iSubItem = 2;
			((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->GetItem(&lvi);
			sTemp.Format(L"%s", writebuf);
			
			if (_ttoi(sTemp) > 0)
			{				
				TmpRobotPos.Velocity = _ttoi(sTemp);
			}			

			lvi.iSubItem = 4;
			((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->GetItem(&lvi);
			sTemp.Format(L"%s", writebuf);

			if (sTemp == L"Linear Motion")
			{				
				TmpRobotPos.Type = Linear_Motion;
			}

			if (sTemp == L"Joint Motion")
			{				
				TmpRobotPos.Type = Joint_Motion;
			}
			
			theApp.All_Path[PathName].push_back(TmpRobotPos);

			/*sTemp.Format(L"%d", i);

			AfxMessageBox(sTemp);*/
		}
	}
	else
		AfxMessageBox(L"Wrong Path Name!!!");

	theApp.Save_Path_Setting();
}


void CFanucRobotInterfaceView::OnBnClickedButMotionTest()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString PathName;
	((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->GetCurSel(), PathName);	
	
	if (!Move(Robot_Index, theApp.All_Path[PathName], 0, false))
	{
		AfxMessageBox(L"Move Command Fail!!!");
		return;
	}
}


void CFanucRobotInterfaceView::OnBnClickedButMotionTestBackward()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString PathName;
	((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->GetCurSel(), PathName);

	if (!Move(Robot_Index, theApp.All_Path[PathName], 0, false))
	{
		AfxMessageBox(L"Move Command Fail!!!");
		return;
	}
}


void CFanucRobotInterfaceView::OnBnClickedButMotionTestForward()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString PathName;
	((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->GetCurSel(), PathName);

	//AfxMessageBox(PathName);

	if (!Move(Robot_Index, theApp.All_Path[PathName], theApp.All_Path[PathName].size() - 1, true))
	{
		AfxMessageBox(L"Move Command Fail!!!");
		return;
	}
}

//void CFanucRobotInterfaceView::OnBnClickedButton7()
//{
//	// TODO: 在此加入控制項告知處理常式程式碼
//	AfxBeginThread(Test, this, THREAD_PRIORITY_NORMAL);
//}


void CFanucRobotInterfaceView::OnBnClickedButStop()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	Robot_Stop(Robot_Index, true);
}


//void CFanucRobotInterfaceView::OnBnClickedButton8()
//{
//	// TODO: 在此加入控制項告知處理常式程式碼
//	TestRobot();
//}


void CFanucRobotInterfaceView::OnCbnSelchangeComboPathName()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString PathName;
	CString sTemp;
	((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO_PATH_NAME))->GetCurSel(), PathName);

	((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->DeleteAllItems();

	for (int i = 0; i < theApp.All_Path[PathName].size(); i++)
	{
		sTemp.Format(L"%d", i);
		((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->InsertItem(i, sTemp);

		sTemp.Format(L"%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf, %.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf",
			theApp.All_Path[PathName][i].Pos[Axis::X],
			theApp.All_Path[PathName][i].Pos[Axis::Y],
			theApp.All_Path[PathName][i].Pos[Axis::Z],
			theApp.All_Path[PathName][i].Pos[Axis::W],
			theApp.All_Path[PathName][i].Pos[Axis::P],
			theApp.All_Path[PathName][i].Pos[Axis::R],
			theApp.All_Path[PathName][i].Pos[Axis::J1],
			theApp.All_Path[PathName][i].Pos[Axis::J2],
			theApp.All_Path[PathName][i].Pos[Axis::J3],
			theApp.All_Path[PathName][i].Pos[Axis::J4],
			theApp.All_Path[PathName][i].Pos[Axis::J5],
			theApp.All_Path[PathName][i].Pos[Axis::J6]);
		((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->SetItemText(i, 1, sTemp);

		sTemp.Format(L"%d", theApp.All_Path[PathName][i].Velocity);
		((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->SetItemText(i, 2, sTemp);

		if (theApp.All_Path[PathName][i].Type == Linear_Motion)
			sTemp.Format(L"Linear Motion");
		else
			sTemp.Format(L"Joint Motion");
		((CListCtrl*)GetDlgItem(IDC_LIST_PATH))->SetItemText(i, 3, sTemp);
	}
}

LRESULT CFanucRobotInterfaceView::Receive_Robot_Msg(WPARAM wParam, LPARAM lParam)
{
	RobotMsg* pRobotMsg;
	pRobotMsg = (RobotMsg*)wParam;

	if (((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO))->GetCount() > 100)
		((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO))->ResetContent();

	if (pRobotMsg->Type == MsgString)
	{
		CString iCotent;		
		iCotent.Format(L"%s", (char*)(pRobotMsg->Content));
		((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO))->InsertString(-1, pRobotMsg->Time + L" " + iCotent);
	}

	if (pRobotMsg->Type == Position)
	{
		CString TmpStr;

		TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[Axis::X]);
		GetDlgItem(IDC_EDIT_XPOS)->SetWindowTextW(TmpStr);
		TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[Axis::Y]);
		GetDlgItem(IDC_EDIT_YPOS)->SetWindowTextW(TmpStr);
		TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[Axis::Z]);
		GetDlgItem(IDC_EDIT_ZPOS)->SetWindowTextW(TmpStr);
		TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[Axis::W]);
		GetDlgItem(IDC_EDIT_WANGLE)->SetWindowTextW(TmpStr);
		TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[Axis::P]);
		GetDlgItem(IDC_EDIT_PANGLE)->SetWindowTextW(TmpStr);
		TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[Axis::R]);
		GetDlgItem(IDC_EDIT_RANGLE)->SetWindowTextW(TmpStr);
		TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[Axis::J1]);
		GetDlgItem(IDC_EDIT_J1)->SetWindowTextW(TmpStr);
		TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[Axis::J2]);
		GetDlgItem(IDC_EDIT_J2)->SetWindowTextW(TmpStr);
		TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[Axis::J3]);
		GetDlgItem(IDC_EDIT_J3)->SetWindowTextW(TmpStr);
		TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[Axis::J4]);
		GetDlgItem(IDC_EDIT_J4)->SetWindowTextW(TmpStr);
		TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[Axis::J5]);
		GetDlgItem(IDC_EDIT_J5)->SetWindowTextW(TmpStr);
		TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[Axis::J6]);
		GetDlgItem(IDC_EDIT_J6)->SetWindowTextW(TmpStr);

		/*if (((Robot_Pos*)pRobotMsg->Content)->Coor == Xyz)
		{
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[0]);
			GetDlgItem(IDC_EDIT_XPOS)->SetWindowTextW(TmpStr);
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[1]);
			GetDlgItem(IDC_EDIT_YPOS)->SetWindowTextW(TmpStr);
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[2]);
			GetDlgItem(IDC_EDIT_ZPOS)->SetWindowTextW(TmpStr);
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[3]);
			GetDlgItem(IDC_EDIT_WANGLE)->SetWindowTextW(TmpStr);
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[4]);
			GetDlgItem(IDC_EDIT_PANGLE)->SetWindowTextW(TmpStr);
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[5]);
			GetDlgItem(IDC_EDIT_RANGLE)->SetWindowTextW(TmpStr);
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[0]);
			GetDlgItem(IDC_EDIT_J1)->SetWindowTextW(TmpStr);
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[1]);
			GetDlgItem(IDC_EDIT_J2)->SetWindowTextW(TmpStr);
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[2]);
			GetDlgItem(IDC_EDIT_J3)->SetWindowTextW(TmpStr);
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[3]);
			GetDlgItem(IDC_EDIT_J4)->SetWindowTextW(TmpStr);
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[4]);
			GetDlgItem(IDC_EDIT_J5)->SetWindowTextW(TmpStr);
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[5]);
			GetDlgItem(IDC_EDIT_J6)->SetWindowTextW(TmpStr);
		}*/

		/*if (((Robot_Pos*)pRobotMsg->Content)->Coor == Joint)
		{
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[0]);
			GetDlgItem(IDC_EDIT_J1)->SetWindowTextW(TmpStr);
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[1]);
			GetDlgItem(IDC_EDIT_J2)->SetWindowTextW(TmpStr);
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[2]);
			GetDlgItem(IDC_EDIT_J3)->SetWindowTextW(TmpStr);
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[3]);
			GetDlgItem(IDC_EDIT_J4)->SetWindowTextW(TmpStr);
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[4]);
			GetDlgItem(IDC_EDIT_J5)->SetWindowTextW(TmpStr);
			TmpStr.Format(L"%.3f", ((Robot_Pos*)pRobotMsg->Content)->Pos[5]);
			GetDlgItem(IDC_EDIT_J6)->SetWindowTextW(TmpStr);
		}*/
	}

	if (pRobotMsg->Type == Alarm)
	{
		CString iCotent;
		if (!((AlarmInfo*)pRobotMsg->Content)->IsClear)
		{			
			((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO))->InsertString(-1, pRobotMsg->Time);
			iCotent.Format(L"-------------------------------Alarm-------------------------------");
			((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO))->InsertString(-1, iCotent);

			iCotent.Format(L"New Alarm Id : %u", ((AlarmInfo*)pRobotMsg->Content)->AlarmID);
			((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO))->InsertString(-1, iCotent);
			((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO))->InsertString(-1, ((AlarmInfo*)pRobotMsg->Content)->AlarmMessage);			

			iCotent.Format(L"----------------------------------------------------------------------");
			((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO))->InsertString(-1, iCotent);
		}
		else
		{
			((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO))->InsertString(-1, pRobotMsg->Time);
			iCotent.Format(L"-----------------------------Clear Alarm-----------------------------");
			((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO))->InsertString(-1, iCotent);

			iCotent.Format(L"Alarm Id : %u", ((AlarmInfo*)pRobotMsg->Content)->AlarmID);
			((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO))->InsertString(-1, iCotent);
			((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO))->InsertString(-1, ((AlarmInfo*)pRobotMsg->Content)->AlarmMessage);
			
			iCotent.Format(L"----------------------------------------------------------------------");
			((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO))->InsertString(-1, iCotent);
		}
	}

	if (pRobotMsg->Type == State)
	{
		CString iCotent;
		if (((MachineState*)pRobotMsg->Content)->MState == IDLE)
		{
			Stop_Signal = false;
			Pause_Signal = false;
			iCotent.Format(L"Robot State = IDLE");
			((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO))->InsertString(-1, pRobotMsg->Time + " " + iCotent);
		}

		/*if (((MachineState*)pRobotMsg->Content)->MState == RUN)
		{
			Stop_Signal = false;
			Pause_Signal = false;
			iCotent.Format(L"Robot State = RUN");
			((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO))->InsertString(-1, pRobotMsg->Time + " " + iCotent);
		}*/

		if (((MachineState*)pRobotMsg->Content)->MState == STOP)
		{
			Stop_Signal = true;
			Pause_Signal = false;
			iCotent.Format(L"Robot State = STOP");
			((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO))->InsertString(-1, pRobotMsg->Time + " " + iCotent);
		}

		if (((MachineState*)pRobotMsg->Content)->MState == PAUSE)
		{
			Stop_Signal = false;
			Pause_Signal = true;
			iCotent.Format(L"Robot State = PAUSE");
			((CListBox*)GetDlgItem(IDC_LIST_ROBOTINFO))->InsertString(-1, pRobotMsg->Time + " " + iCotent);
		}		
	}

	return LRESULT();
}


void CFanucRobotInterfaceView::OnBnClickedButEstop()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	Robot_Stop(Robot_Index, !Stop_Signal);
}


void CFanucRobotInterfaceView::OnBnClickedButPause()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	Robot_Pause(Robot_Index, !Pause_Signal);
}


void CFanucRobotInterfaceView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此加入您的訊息處理常式程式碼和 (或) 呼叫預設值
	CString sTemp;

	switch (nChar)
	{
	case VK_RETURN:
		sTemp.Format(L"VK_RETURN");
		AfxMessageBox(sTemp);
		break;
	case VK_F1:
		sTemp.Format(L"VK_F1");
		AfxMessageBox(sTemp);
		break;
	case VK_DELETE:
		sTemp.Format(L"VK_DELETE");
		AfxMessageBox(sTemp);
		break;		
	default:
		sTemp.Format(L"VK_NO");
		AfxMessageBox(sTemp);
	}

	CFormView::OnKeyDown(nChar, nRepCnt, nFlags);
}


BOOL CFanucRobotInterfaceView::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此加入特定的程式碼和 (或) 呼叫基底類別
	CString sTemp;
	if (pMsg->message == WM_KEYUP)

	{
		switch (pMsg->wParam)
		{
		case VK_SHIFT:
			Robot_SetTeachEnable(Robot_Index, false);
			sTemp.Format(L"Unable");
			GetDlgItem(IDC_BUTTON_SETAUTO)->SetWindowTextW(sTemp);
			
			break;
		default:
			
			break;
		}
	}

	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_SHIFT:
			Robot_SetTeachEnable(Robot_Index, true);
			sTemp.Format(L"Enable");
			GetDlgItem(IDC_BUTTON_SETAUTO)->SetWindowTextW(sTemp);

			break;
		default:

			break;
		}
	}

	return CFormView::PreTranslateMessage(pMsg);
}


void CFanucRobotInterfaceView::OnBnClickedButtonSetauto()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	IsEnable = !IsEnable;
	Robot_SetAuto(Robot_Index, IsEnable);
}
