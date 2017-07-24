#include "opencv\highgui.h"
#include "opencv\cv.h"
#include "MainThread.h"
#include "ICDThread.h"
#include "ShotDlg.h"

// ICDDlg.h : �w�b�_�[ �t�@�C��
//

#pragma once



// CICDDlg �_�C�A���O
class CICDDlg : public CDialogEx
{

protected:
	HICON m_hIcon;

	// �������ꂽ�A���b�Z�[�W���蓖�Ċ֐�
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
	CICDDlg(CWnd* pParent = NULL);	// �W���R���X�g���N�^�[
	void SendMainThreadMess(WPARAM wParam, LPARAM lParam);

// �_�C�A���O �f�[�^
	enum { IDD = IDD_ICD_DIALOG };

	IplImage* m_iImage;
	void IplToBmp(const IplImage* srcIplImage, CBitmap* BmpMapData);
	CBitmap m_cBitmap;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �T�|�[�g


	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	bool CreateThread();
	LRESULT OnMainThreadMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedIcdmode();
	void CreateShotModeDialog();
	void DestroyShotModeDialog();



	
};
