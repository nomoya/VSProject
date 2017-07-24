#include "stdafx.h"
#include "MainThread.h"

IMPLEMENT_DYNCREATE(CMainThread, CBaseTask)

// ���b�Z�[�W
BEGIN_MESSAGE_MAP(CMainThread, CBaseTask)
	ON_THREAD_MESSAGE(MSG_FROM_GUI, OnGUIMessage)
	ON_THREAD_MESSAGE(MSG_FROM_ICD, OnICDMessage)
END_MESSAGE_MAP()

CMainThread::CMainThread()
{
}


CMainThread::~CMainThread()
{
}


/****************************************************************************
Name : InitInstance()
Desc : �J�n���ɌĂ΂��@
****************************************************************************/
BOOL CMainThread::InitInstance()
{
	// ������
	if (!Init()){
		// m_cStatus.bBusy = FALSE;
		// FALSE��Ԃ��ƁA�����I��ExitInstance���Ă΂�Ă��܂��̂ł܂���
		// ���������s���Ɏ��s���R���擾���邽��
		return(false);
		//return(true);
	}
	//m_cStatus.bInit = TRUE;
	//m_cStatus.bBusy = FALSE;
	//ChangeStatus();
	//NotifyGui(NOTIFY_INIT_OK, NULL);
	return(true);
}

/****************************************************************************
Name : Init()
Desc : �X���b�h��������
****************************************************************************/
bool CMainThread::Init()
{
	// �N���e�B�J���Z�N�V����������
	InitializeCriticalSection(&m_CriticalSection);

	// ���������X���b�h�쐬
	m_pICDThread = (CICDThread*)AfxBeginThread
		(
		RUNTIME_CLASS(CICDThread),
		THREAD_PRIORITY_ABOVE_NORMAL,
		0,
		CREATE_SUSPENDED	// �T�X�y���h��ԂŋN��
		);
	if (m_pICDThread == NULL){
		return(false);
	}
	m_pICDThread->m_bAutoDelete = FALSE;			    // �����j���h�~
	m_pICDThread->SetSendTask(this, MSG_FROM_ICD);	// ���b�Z�[�W�ݒ�
	m_pICDThread->ResumeThread();					    // �T�X�y���h����

	return(true);
}

/****************************************************************************
Name : ExitInstance()
Desc : �I�����ɌĂ΂��@
****************************************************************************/
int CMainThread::ExitInstance()
{
	DWORD	dwExitCode = STILL_ACTIVE;

	// CalcThread�I��
	if (m_pICDThread != NULL){
		// �����ĂĂ������I�ɒ�~
		//m_pCalcThread->Abort();
		// ��~�҂�
		//do{
		//	m_pCalcThread->GetStatus(cAAStatus);
		//} while (cAAStatus.bBusy);
		// �I�����b�Z�[�W�𑗂�
		::PostThreadMessage(m_pICDThread->m_nThreadID, WM_QUIT, 0, 0);
		// �^�X�N���I������܂ő҂�
		do{
			if (GetExitCodeThread(m_pICDThread->m_hThread, &dwExitCode) != TRUE){
				dwExitCode = STILL_ACTIVE;
			}
		} while (dwExitCode == STILL_ACTIVE);
		delete(m_pICDThread);
		m_pICDThread = NULL;
	}
	// �N���e�B�J���Z�N�V�����̔j��
	DeleteCriticalSection(&m_CriticalSection);

	return(CBaseTask::ExitInstance());
}


/****************************************************************************
Name : GetImage()
Desc : ���̃X���b�h�����̊֐��������ĉ摜���擾����@
****************************************************************************/
void CMainThread::GetImage(IplImage** image)
{
	EnterCriticalSection(&m_CriticalSection);//�N���e�B�J���Z�N�V�����擾
	{
		*image = cvCloneImage(m_iImage);
	}
	LeaveCriticalSection(&m_CriticalSection);//�N���e�B�J���Z�N�V�������
}


/****************************************************************************
Name : SendICDThreadMess()
Desc : ICD�X���b�h�Ƀ��b�Z�[�W�𑗐M
****************************************************************************/
void CMainThread::SendICDThreadMess(WPARAM wParam, LPARAM lParam)
{
	if (m_pICDThread != NULL){
		::PostThreadMessage(m_pICDThread->m_nThreadID, CICDThread::MSG_FROM_MAIN, wParam, lParam);
	}
}

/****************************************************************************
Name : OnGUIMessage()
Desc : GUI����̃��b�Z�[�W�n���h���@
****************************************************************************/
void CMainThread::OnGUIMessage(WPARAM wParam, LPARAM lParam)
{
	CString strLog;
	switch (wParam)
	{
	case WPRM_CAPTURE:
		SendICDThreadMess(wParam, lParam);
		break;
	case WPRM_DETECT:
		//strLog.Format(_T("wparam = [%d]"), wParam);
		//OutputDebugString(strLog);
		SendICDThreadMess(wParam, lParam);  //���̂܂܃��b�Z�[�W�𓊂���
		break;
	}

}

/****************************************************************************
Name : OnICDcMessage()
Desc : ICD�X���b�h����̃��b�Z�[�W�n���h���@
****************************************************************************/
void CMainThread::OnICDMessage(WPARAM wParam, LPARAM lParam)
{
	CString strLog;
	CImage img;
	switch (wParam)
	{
	case CICDThread::NOTIFY_CAPTURE_END:
		m_pICDThread->GetImage(&m_iImage);// ICDThread�ŃL���v�`�������摜���擾
		NotifyGui(wParam, lParam);
		break;
	case CICDThread::NOTIFY_DETECT_END:
		NotifyGui(wParam, lParam);
		break;
	}

}