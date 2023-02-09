
// Fanuc_Robot_Interface.cpp: 定義應用程式的類別表現方式。
//

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "Fanuc_Robot_Interface.h"
#include "MainFrm.h"

#include "Fanuc_Robot_InterfaceDoc.h"
#include "Fanuc_Robot_InterfaceView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFanucRobotInterfaceApp

BEGIN_MESSAGE_MAP(CFanucRobotInterfaceApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CFanucRobotInterfaceApp::OnAppAbout)
	// 依據文件命令的標準檔案
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
END_MESSAGE_MAP()


// CFanucRobotInterfaceApp 建構

CFanucRobotInterfaceApp::CFanucRobotInterfaceApp() noexcept
{

	// TODO: 以唯一的 ID 字串取代下面的應用程式 ID 字串; 建議的
	// 字串格式為 CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("FanucRobotInterface.AppID.NoVersion"));

	// TODO:  在此加入建構程式碼，
	// 將所有重要的初始設定加入 InitInstance 中
}

// 唯一一個 CFanucRobotInterfaceApp 物件

CFanucRobotInterfaceApp theApp;


// CFanucRobotInterfaceApp 初始化

BOOL CFanucRobotInterfaceApp::InitInstance()
{
	CWinApp::InitInstance();


	EnableTaskbarInteraction(FALSE);

	// 需要有 AfxInitRichEdit2() 才能使用 RichEdit 控制項
	// AfxInitRichEdit2();

	// 標準初始設定
	// 如果您不使用這些功能並且想減少
	// 最後完成的可執行檔大小，您可以
	// 從下列程式碼移除不需要的初始化常式，
	// 變更儲存設定值的登錄機碼
	// TODO: 您應該適度修改此字串
	// (例如，公司名稱或組織名稱)
	SetRegistryKey(_T("本機 AppWizard 所產生的應用程式"));
	LoadStdProfileSettings(4);  // 載入標準 INI 檔選項 (包含 MRU)


	// 登錄應用程式的文件範本。文件範本負責在文件、
	// 框架視窗與檢視間進行連接
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CFanucRobotInterfaceDoc),
		RUNTIME_CLASS(CMainFrame),       // 主 SDI 框架視窗
		RUNTIME_CLASS(CFanucRobotInterfaceView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// 剖析標準 Shell 命令、DDE、檔案開啟舊檔的命令列
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// 在命令列中指定的分派命令。如果已使用 
	// /RegServer、/Register、/Unregserver 或 /Unregister 啟動應用程式，將傳回 FALSE。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// 僅初始化一個視窗，所以顯示並更新該視窗
	m_pMainWnd->SetWindowText(L"Biomine Robot Interface Test");
	m_pMainWnd->MoveWindow(0,
		0,
		1900,
		950,
		TRUE);

	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	return TRUE;
}

// CFanucRobotInterfaceApp 訊息處理常式


// 對 App About 使用 CAboutDlg 對話方塊

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

// 程式碼實作
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// 執行對話方塊的應用程式命令
void CFanucRobotInterfaceApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CFanucRobotInterfaceApp 訊息處理常式

void CFanucRobotInterfaceApp::Load_Path_Setting()
{
	FILE* fp1;

	CFileFind	FileFind;
	CString		sFile_Path;
	CString		sFind_FileName;
	CString		sDir = L"C:\\Bionime\\Robot_Setting\\Path_*.*";
	
	if (FileFind.FindFile(sDir))
	{
		All_Path.clear();
		BOOL bFindFile = TRUE;

		while (bFindFile)
		{
			bFindFile = FileFind.FindNextFileW();

			if (FileFind.IsDots())					//如果是"."或".."繼續執行下一次迴圈
				continue;

			if (!FileFind.IsDirectory())	//If found object is file
			{
				sFind_FileName = FileFind.GetFileName();
				int strLength = sFind_FileName.GetLength();
				CString Path_Name = sFind_FileName.Mid(5, strLength-5-4);
				CString Path_FileName = L"C:\\Bionime\\Robot_Setting\\" + sFind_FileName;
				
				CString		sTitle;
				CString		sSubTitle;
				CString		sTemp;
				TCHAR		readbuf[128];
				
				bool Is_Sec_Ok = true;
				int Index = 0;
				std::vector<Robot_Pos> NewPath;
				while (Is_Sec_Ok)
				{
					Robot_Pos PosSetValue;					
					sTitle.Format(L"Position_%d", Index++);

					GetPrivateProfileString(sTitle, L"MotionType", L"", readbuf, sizeof(readbuf), Path_FileName);
					if (!_tcscmp(readbuf, _T("linear")))
						PosSetValue.Type = Linear_Motion;
					else if (!_tcscmp(readbuf, _T("joint")))
						PosSetValue.Type = Joint_Motion;
					else
						Is_Sec_Ok = false;
					
					GetPrivateProfileString(sTitle, L"Velocity", L"", readbuf, sizeof(readbuf), Path_FileName);
					PosSetValue.Velocity = _tstoi(readbuf);

					for (int i = Axis::X; i <= Axis::J6; i++)
					{
						sSubTitle.Format(L"P%d", i + 1);
						GetPrivateProfileString(sTitle, sSubTitle, L"", readbuf, sizeof(readbuf), Path_FileName);
						PosSetValue.Pos[i] = _tstof(readbuf);
						/*sTemp.Format(L"%f", PosSetValue.Pos[i]);
						AfxMessageBox(sTemp);*/
					}				
					
					if(Is_Sec_Ok)
						NewPath.push_back(PosSetValue);
				}	

				if (NewPath.size() > 0)
					All_Path[Path_Name] = NewPath;						
			}
		}
		FileFind.Close();		
	}	
}

void CFanucRobotInterfaceApp::Save_Path_Setting()
{
	CString		sFileName;
	CString		sTemp;
	CString		sTitle;
	CString		sSubTitle;

	CreateDirectory(L"C:\\Bionime\\", NULL);
	CreateDirectory(L"C:\\Bionime\\Robot_Setting", NULL);

	for (auto iter = All_Path.begin(); iter != All_Path.end(); ++iter)
	{
		sFileName.Format(L"C:\\Bionime\\Robot_Setting\\Path_" + iter->first + ".ini");

		CFileFind	Finder;
		CFile		File;
		if (Finder.FindFile(sFileName))
			File.Remove(sFileName);
		
		for (int i = 0; i < All_Path[iter->first].size(); i++)
		{
			sTitle.Format(L"Position_%d", i);
			
			if (All_Path[iter->first][i].Type == Linear_Motion)
				sTemp.Format(L"linear");
			else
				sTemp.Format(L"joint");
			WritePrivateProfileString(sTitle, L"MotionType", sTemp, sFileName);

			sTemp.Format(L"%d", All_Path[iter->first][i].Velocity);
			WritePrivateProfileString(sTitle, L"Velocity", sTemp, sFileName);

			for (int j = Axis::X; j <= Axis::J6; j++)
			{
				sTemp.Format(L"%.2f", All_Path[iter->first][i].Pos[j]);
				sSubTitle.Format(L"P%d", j + 1);
				WritePrivateProfileString(sTitle, sSubTitle, sTemp, sFileName);
			}
		}
	}
	
}

