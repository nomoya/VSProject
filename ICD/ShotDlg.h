#pragma once
#include "afxwin.h"


// CShotDlg �_�C�A���O

class CShotDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CShotDlg)

public:
	CShotDlg(CWnd* pParent = NULL);   // �W���R���X�g���N�^�[
	virtual ~CShotDlg();
// �_�C�A���O �f�[�^
	enum { IDD = IDD_SHOT_DIALOG };
	CStatic m_cPicture_L;
	CStatic m_cPicture_R;

	// NOTIFY



protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
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
