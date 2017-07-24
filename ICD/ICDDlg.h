#include "opencv\highgui.h"
#include "opencv\cv.h"
#include "MainThread.h"
#include "ICDThread.h"
#include "ShotDlg.h"

// ICDDlg.h : ヘッダー ファイル
//

#pragma once



// CICDDlg ダイアログ
class CICDDlg : public CDialogEx
{

protected:
	HICON m_hIcon;

	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	void OnClose();
	void Close();
	CMainThread* m_pMainThread;
	CICDThread*  m_pICDThread;

	CShotDlg* m_pDlgShotMode;
	void ResizeGraphView(CStatic& view, int width, int height);

public:
	CICDDlg(CWnd* pParent = NULL);	// 標準コンストラクター
	void SendMainThreadMess(WPARAM wParam, LPARAM lParam);

// ダイアログ データ
	enum { IDD = IDD_ICD_DIALOG };

	IplImage* m_iImage;
	void IplToBmp(const IplImage* srcIplImage, CBitmap* BmpMapData);
	CBitmap m_cBitmap;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート


	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	bool CreateThread();
	LRESULT OnMainThreadMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedIcdmode();
	void CreateShotModeDialog();
	void DestroyShotModeDialog();



	
};
