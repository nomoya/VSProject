#include "stdafx.h"
#include "MainThread.h"

IMPLEMENT_DYNCREATE(CMainThread, CBaseTask)

// メッセージ
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
Desc : 開始時に呼ばれる　
****************************************************************************/
BOOL CMainThread::InitInstance()
{
	// 初期化
	if (!Init()){
		// m_cStatus.bBusy = FALSE;
		// FALSEを返すと、自動的にExitInstanceが呼ばれてしまうのでまずい
		// 初期化失敗時に失敗理由を取得するため
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
Desc : スレッド初期処理
****************************************************************************/
bool CMainThread::Init()
{
	// クリティカルセクション初期化
	InitializeCriticalSection(&m_CriticalSection);

	// 内部処理スレッド作成
	m_pICDThread = (CICDThread*)AfxBeginThread
		(
		RUNTIME_CLASS(CICDThread),
		THREAD_PRIORITY_ABOVE_NORMAL,
		0,
		CREATE_SUSPENDED	// サスペンド状態で起動
		);
	if (m_pICDThread == NULL){
		return(false);
	}
	m_pICDThread->m_bAutoDelete = FALSE;			    // 自動破棄防止
	m_pICDThread->SetSendTask(this, MSG_FROM_ICD);	// メッセージ設定
	m_pICDThread->ResumeThread();					    // サスペンド解除

	return(true);
}

/****************************************************************************
Name : ExitInstance()
Desc : 終了時に呼ばれる　
****************************************************************************/
int CMainThread::ExitInstance()
{
	DWORD	dwExitCode = STILL_ACTIVE;

	// CalcThread終了
	if (m_pICDThread != NULL){
		// 動いてても強制的に停止
		//m_pCalcThread->Abort();
		// 停止待ち
		//do{
		//	m_pCalcThread->GetStatus(cAAStatus);
		//} while (cAAStatus.bBusy);
		// 終了メッセージを送る
		::PostThreadMessage(m_pICDThread->m_nThreadID, WM_QUIT, 0, 0);
		// タスクが終了するまで待つ
		do{
			if (GetExitCodeThread(m_pICDThread->m_hThread, &dwExitCode) != TRUE){
				dwExitCode = STILL_ACTIVE;
			}
		} while (dwExitCode == STILL_ACTIVE);
		delete(m_pICDThread);
		m_pICDThread = NULL;
	}
	// クリティカルセクションの破棄
	DeleteCriticalSection(&m_CriticalSection);

	return(CBaseTask::ExitInstance());
}


/****************************************************************************
Name : GetImage()
Desc : 他のスレッドがこの関数をつかって画像を取得する　
****************************************************************************/
void CMainThread::GetImage(IplImage** image)
{
	EnterCriticalSection(&m_CriticalSection);//クリティカルセクション取得
	{
		*image = cvCloneImage(m_iImage);
	}
	LeaveCriticalSection(&m_CriticalSection);//クリティカルセクション解放
}


/****************************************************************************
Name : SendICDThreadMess()
Desc : ICDスレッドにメッセージを送信
****************************************************************************/
void CMainThread::SendICDThreadMess(WPARAM wParam, LPARAM lParam)
{
	if (m_pICDThread != NULL){
		::PostThreadMessage(m_pICDThread->m_nThreadID, CICDThread::MSG_FROM_MAIN, wParam, lParam);
	}
}

/****************************************************************************
Name : OnGUIMessage()
Desc : GUIからのメッセージハンドラ　
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
		SendICDThreadMess(wParam, lParam);  //そのままメッセージを投げる
		break;
	}

}

/****************************************************************************
Name : OnICDcMessage()
Desc : ICDスレッドからのメッセージハンドラ　
****************************************************************************/
void CMainThread::OnICDMessage(WPARAM wParam, LPARAM lParam)
{
	CString strLog;
	CImage img;
	switch (wParam)
	{
	case CICDThread::NOTIFY_CAPTURE_END:
		m_pICDThread->GetImage(&m_iImage);// ICDThreadでキャプチャした画像を取得
		NotifyGui(wParam, lParam);
		break;
	case CICDThread::NOTIFY_DETECT_END:
		NotifyGui(wParam, lParam);
		break;
	}

}