// ShotDlg.cpp : �����t�@�C��
//

#include "stdafx.h"
#include "ICD.h"
#include "ShotDlg.h"
#include "afxdialogex.h"
#include "MainThread.h"
#include "ICDDlg.h"


// CShotDlg �_�C�A���O

IMPLEMENT_DYNAMIC(CShotDlg, CDialogEx)

CShotDlg::CShotDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CShotDlg::IDD, pParent)
	, m_rbDetect(FALSE)

{

}

CShotDlg::~CShotDlg()
{
}

void CShotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PIC_L, m_cPicture_L);
	DDX_Control(pDX, IDC_PIC_R, m_cPicture_R);
	DDX_Radio(pDX, IDC_CIRLCE_RADIO, m_rbDetect);
	DDX_Control(pDX, IDC_EDIT, m_strEditMess);
}


BEGIN_MESSAGE_MAP(CShotDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CAPTURE_BUTTON, &CShotDlg::OnBnClickedCaptureButton)
	ON_BN_CLICKED(IDC_DETECT_BUTTON, &CShotDlg::OnBnClickedDetectButton)
END_MESSAGE_MAP()


// CShotDlg ���b�Z�[�W �n���h���[

/****************************************************************************
Name : SendMainThreadMess()
Desc : MainThread�Ƀ��b�Z�[�W�𑗐M
****************************************************************************/
void CShotDlg::SendMainThreadMess(WPARAM wParam, LPARAM lParam)
{
	((CICDDlg*)theApp.GetMainWnd())->SendMainThreadMess(wParam, lParam);

	/*
	if (m_pMainThread != NULL){
		::PostThreadMessage(m_pMainThread->m_nThreadID, CMainThread::MSG_FROM_GUI, wParam, lParam);
	}
	*/
}


void CShotDlg::OnBnClickedCaptureButton()
{
	SendMainThreadMess(CMainThread::WPRM_CAPTURE, NULL);
}


void CShotDlg::OnBnClickedDetectButton()
{
	UpdateData(); // �X�V���� m_rbDetect �ɔ��f
	SendMainThreadMess(CMainThread::WPRM_DETECT, m_rbDetect);
}


