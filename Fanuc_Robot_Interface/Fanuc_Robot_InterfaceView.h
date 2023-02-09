
// Fanuc_Robot_InterfaceView.h: CFanucRobotInterfaceView 類別的介面
//

class CFanucRobotInterfaceView : public CFormView
{
protected: // 僅從序列化建立
	CFanucRobotInterfaceView() noexcept;
	DECLARE_DYNCREATE(CFanucRobotInterfaceView)

public:
#ifdef AFX_DESIGN_TIME
	enum{ IDD = IDD_FANUC_ROBOT_INTERFACE_FORM };
#endif

// 屬性
public:
	CFanucRobotInterfaceDoc* GetDocument() const;

// 作業
public:

// 覆寫
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援
	virtual void OnInitialUpdate(); // 建構後第一次呼叫

// 程式碼實作
public:
	virtual ~CFanucRobotInterfaceView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif	

protected:

// 產生的訊息對應函式
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnCon();
	afx_msg void OnBnClickedBtnCon2();

	bool Robot_Is_Connect;
	afx_msg void OnBnClickedButActiveRobot();
	afx_msg void OnBnClickedButForwardX();
	afx_msg void OnBnClickedButBackwardX2();
	afx_msg void OnBnClickedButBackwardX();
	afx_msg void OnBnClickedButBackwardY();
	afx_msg void OnBnClickedButForwardY();
	afx_msg void OnBnClickedButBackwardZ();
	afx_msg void OnBnClickedButForwardZ();
	afx_msg void OnBnClickedButBackwardW();
	afx_msg void OnBnClickedButForwardW();
	afx_msg void OnBnClickedButBackwardP();
	afx_msg void OnBnClickedButForwardP();
	afx_msg void OnBnClickedButBackwardR();
	afx_msg void OnBnClickedButForwardR();
	afx_msg void OnBnClickedButBackwardJ1();
	afx_msg void OnBnClickedButForwardJ1();
	afx_msg void OnBnClickedButBackwardJ2();
	afx_msg void OnBnClickedButForwardJ2();
	afx_msg void OnBnClickedButBackwardJ3();
	afx_msg void OnBnClickedButForwardJ3();
	afx_msg void OnBnClickedButBackwardJ4();
	afx_msg void OnBnClickedButForwardJ4();
	afx_msg void OnBnClickedButBackwardJ5();
	afx_msg void OnBnClickedButForwardJ5();
	afx_msg void OnBnClickedButBackwardJ6();
	afx_msg void OnBnClickedButForwardJ6();
	afx_msg void OnBnClickedButBackwardJ7();
	afx_msg void OnBnClickedButForwardJ7();
	afx_msg void OnBnClickedButBackwardJ8();
	afx_msg void OnBnClickedButForwardJ8();
	afx_msg void OnBnClickedButBackwardJ9();
	afx_msg void OnBnClickedButForwardJ9();
	afx_msg void OnBnClickedBtnDiscon();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButNewpath();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButInsertPos();
	afx_msg void OnBnClickedButDeletePos();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButSavePath();
	afx_msg void OnBnClickedButMotionTest();
	afx_msg void OnBnClickedButMotionTestBackward();
	afx_msg void OnBnClickedButMotionTestForward();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButStop();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnCbnSelchangeComboPathName();
	afx_msg LRESULT Receive_Robot_Msg(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButEstop();
	afx_msg void OnBnClickedButPause();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonSetauto();
};

#ifndef _DEBUG  // 對 Fanuc_Robot_InterfaceView.cpp 中的版本進行偵錯
inline CFanucRobotInterfaceDoc* CFanucRobotInterfaceView::GetDocument() const
   { return reinterpret_cast<CFanucRobotInterfaceDoc*>(m_pDocument); }
#endif



