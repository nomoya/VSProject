#pragma once
#include "afxwin.h"


// CShotDlg ダイアログ

class CShotDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CShotDlg)

public:
	CShotDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CShotDlg();
// ダイアログ データ
	enum { IDD = IDD_SHOT_DIALOG };
	CStatic m_cPicture_L;
	CStatic m_cPicture_R;

	// NOTIFY



protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	void SendMainThreadMess(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCaptureButton();
	afx_msg void OnBnClickedDetectButton();
private:
	int m_rbDetect;

public:
	CEdit m_strEditMess;
};
