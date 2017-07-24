
// ICDDlg.cpp : �����t�@�C��
//

#include "stdafx.h"
#include "ICD.h"
#include "ICDDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CICDDlg �_�C�A���O

//IMPLEMENT_DYNCREATE(CICDThread, CBaseTask)

CICDDlg::CICDDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CICDDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pDlgShotMode = NULL;
}

void CICDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CICDDlg, CDialogEx)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_ICDMODE, &CICDDlg::OnBnClickedIcdmode)
	ON_MESSAGE(CMainThread::MSG_FROM_GUI, OnMainThreadMessage)
END_MESSAGE_MAP()


// CICDDlg ���b�Z�[�W �n���h���[

BOOL CICDDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���̃_�C�A���O�̃A�C�R����ݒ肵�܂��B�A�v���P�[�V�����̃��C�� �E�B���h�E���_�C�A���O�łȂ��ꍇ�A
	//  Framework �́A���̐ݒ�������I�ɍs���܂��B
	SetIcon(m_hIcon, TRUE);			// �傫���A�C�R���̐ݒ�
	SetIcon(m_hIcon, FALSE);		// �������A�C�R���̐ݒ�

	// TODO: �������������ɒǉ����܂��B

	return TRUE;  // �t�H�[�J�X���R���g���[���ɐݒ肵���ꍇ�������ATRUE ��Ԃ��܂��B
}

// �_�C�A���O�ɍŏ����{�^����ǉ�����ꍇ�A�A�C�R����`�悷�邽�߂�
//  ���̃R�[�h���K�v�ł��B�h�L�������g/�r���[ ���f�����g�� MFC �A�v���P�[�V�����̏ꍇ�A
//  ����́AFramework �ɂ���Ď����I�ɐݒ肳��܂��B

void CICDDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �`��̃f�o�C�X �R���e�L�X�g

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// �N���C�A���g�̎l�p�`�̈���̒���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �A�C�R���̕`��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ���[�U�[���ŏ��������E�B���h�E���h���b�O���Ă���Ƃ��ɕ\������J�[�\�����擾���邽�߂ɁA
//  �V�X�e�������̊֐����Ăяo���܂��B
HCURSOR CICDDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/****************************************************************************
Name : OnCreate()
Desc : ���C���t���[���̍\�z
****************************************************************************/
int CICDDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	/// ----------------------------------------------------- �X���b�h����
	if (!CreateThread()){
		AfxMessageBox(_T("�X���b�h�������ɃG���[���������܂���"));
		return(-1);
	}
	return(0);
}

/****************************************************************************
Name : OnClose()
Desc : �E�B���h�E����鎞�ɌĂ΂��
****************************************************************************/
void CICDDlg::OnClose()
{
	if (IDOK != MessageBox(_T("�{���ɏI�����܂����H"), _T("�I���m�F"), MB_OKCANCEL)){
		return;
	}
	// �I������
	Close();
	// �K��̏I������
	CDialogEx::OnClose();
}



/****************************************************************************
Name : CreateThread()
Desc : MainThread���\�z����
****************************************************************************/
bool CICDDlg::CreateThread()
{
	// MainThread�쐬
	m_pMainThread = (CMainThread*)AfxBeginThread
		(
		RUNTIME_CLASS(CMainThread),
		THREAD_PRIORITY_NORMAL,		// �D��x
		0,
		CREATE_SUSPENDED			// �T�X�y���h��ԂŋN��
		);
	if (m_pMainThread == NULL){

		return(false);
	}
	m_pMainThread->m_bAutoDelete = FALSE;						// �����j���h�~
	m_pMainThread->SetSendGui(this, CMainThread::MSG_FROM_GUI);	// ���b�Z�[�W�ݒ�
	m_pMainThread->ResumeThread();								// �T�X�y���h����
	return(true);
}

/****************************************************************************
Name : Close()
Desc : �������̏I������
****************************************************************************/
void CICDDlg::Close()
{

	DWORD dwExitCode = STILL_ACTIVE;
	// MainThread�I��
	if (m_pMainThread != NULL){
		// �I�����b�Z�[�W�𑗂�
		::PostThreadMessage(m_pMainThread->m_nThreadID, WM_QUIT, 0, 0);
		// �^�X�N���I������܂ő҂�
		do{
			if (GetExitCodeThread(m_pMainThread->m_hThread, &dwExitCode) != TRUE){
				dwExitCode = STILL_ACTIVE;
			}
		} while (dwExitCode == STILL_ACTIVE);
		delete(m_pMainThread);
		m_pMainThread = NULL;
	}
}


/****************************************************************************
Name : SendMainThreadMess()
Desc : MainThread�Ƀ��b�Z�[�W�𑗐M
****************************************************************************/
void CICDDlg::SendMainThreadMess(WPARAM wParam, LPARAM lParam)
{
	if (m_pMainThread != NULL){
		::PostThreadMessage(m_pMainThread->m_nThreadID, CMainThread::MSG_FROM_GUI, wParam, lParam);
	}
}

/****************************************************************************
Name : OnMainThreadMessage()
Desc : ���C���X���b�h����̃��b�Z�[�W�n���h���@
****************************************************************************/
LRESULT CICDDlg::OnMainThreadMessage(WPARAM wParam, LPARAM lParam)
{
	CString strLog;
	CImage img;
	switch (wParam)
	{
	case CICDThread::NOTIFY_CAPTURE_END:
		m_pMainThread->GetImage(&m_iImage);
		IplToBmp(m_iImage, &m_cBitmap);
		img.Attach(m_cBitmap);
		img.Save(_T("bmptestcapture.bmp"));
		//ResizeGraphView(m_pDlgShotMode->m_cPicture_L, 500, 550);

		m_pDlgShotMode->m_cPicture_L.SetBitmap(m_cBitmap);
		Invalidate();
		break;
	case CICDThread::NOTIFY_DETECT_END:
		m_pMainThread->GetImage(&m_iImage);
		IplToBmp(m_iImage, &m_cBitmap);
		img.Attach(m_cBitmap);
		img.Save(_T("bmptestdetect.bmp"));
		//ResizeGraphView(m_pDlgShotMode->m_cPicture_L, 500, 550);

		m_pDlgShotMode->m_cPicture_R.SetBitmap(m_cBitmap);
		Invalidate();
		
		//m_pDlgShotMode->m_strEditMess.SetWindowTextW((LPCTSTR)m_pICDThread->m_strDetectText);
		UpdateData(FALSE);
		break;
	}


	return(0);
}

/****************************************************************************
Name : OnBnCLickedIcdmode()
Desc : ICD Mode�{�^������
****************************************************************************/
void CICDDlg::OnBnClickedIcdmode()
{
	CreateShotModeDialog();
}


/****************************************************************************
Name : CreateShotModeDialog()
Desc : ShotMode�_�C�A���O���\�z
****************************************************************************/
void CICDDlg::CreateShotModeDialog()
{
	if (m_pDlgShotMode != NULL)
	{
		DestroyShotModeDialog();
	}

	m_pDlgShotMode = new CShotDlg(this);
	m_pDlgShotMode->Create(m_pDlgShotMode->IDD, this);
	m_pDlgShotMode->ShowWindow(SW_SHOW);
}

/****************************************************************************
Name : DestroyShotModeDialog()
Desc : ShotMode�_�C�A���O��j��
****************************************************************************/
void CICDDlg::DestroyShotModeDialog()
{
	if (m_pDlgShotMode != NULL)
	{
		m_pDlgShotMode->DestroyWindow();
	}
}





/****************************************************************************
Name : IplToBmp()
Desc : IPL�C���[�W���r�b�g�}�b�v�ɕϊ��@
****************************************************************************/
void CICDDlg::IplToBmp(const IplImage* srcIplImage, CBitmap* BmpMapData)
{
	int	Width = srcIplImage->width;
	int	Height = srcIplImage->height;
	int	WidthStep = srcIplImage->widthStep;


	unsigned char*	ColorBuf = (unsigned char*)calloc(sizeof(char), (Width * Height * 4));

	for (int y = 0; y<Height; y++){
		for (int x = 0; x<Width; x++){
			ColorBuf[(Width * 4 * y) + (x * 4 + 0)] = srcIplImage->imageData[(WidthStep * y) + (x * 3 + 0)];	// Blue
			ColorBuf[(Width * 4 * y) + (x * 4 + 1)] = srcIplImage->imageData[(WidthStep * y) + (x * 3 + 1)];	// Green
			ColorBuf[(Width * 4 * y) + (x * 4 + 2)] = srcIplImage->imageData[(WidthStep * y) + (x * 3 + 2)];	// Red
		}
	}


	//�J���[�o�b�t�@���e��Bitmap�ɓW�J
	BmpMapData->CreateBitmap(Width, Height, 1, 32, ColorBuf);
	free(ColorBuf);
}

/****************************************************************************
Name : ResizeGraphView
Desc : picture control���T�C�Y�@
****************************************************************************/
void CICDDlg::ResizeGraphView(CStatic& view, int width, int height)
{
	CRect myrcView;

	view.GetClientRect(&myrcView);

	int nWInc = width - m_iImage->width; // myrcView.Width();
	int nHInc = height - m_iImage->height; // myrcView.Height();


	CRect rcViewWnd;
	view.GetClientRect(&rcViewWnd);
	//view.ScreenToClient(&rcViewWnd);
	rcViewWnd.right += nWInc;
	rcViewWnd.bottom += nHInc;

	//m_pDlgShotMode->MoveWindow(&rcThis);
	view.MoveWindow(&rcViewWnd);

}



