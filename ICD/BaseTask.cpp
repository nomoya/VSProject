// BaseTask.cpp : 実装ファイル
//

#include "stdafx.h"
#include "BaseTask.h"
#pragma comment(lib,"winmm.lib")


// CBaseTask

IMPLEMENT_DYNCREATE(CBaseTask, CWinThread)

CBaseTask::CBaseTask()
{
	m_pPostMessageWnd = NULL;
	m_unWndMsg = NULL;
	m_pPostMessageThread = NULL;
	m_unThreadMsg = NULL;


	InitializeCriticalSection(&m_CriticalSection);

	// タイマー
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_xQueryFreq);
}

CBaseTask::~CBaseTask()
{
	DeleteCriticalSection(&m_CriticalSection);
}

BOOL CBaseTask::InitInstance()
{
	return(TRUE);
}

int CBaseTask::ExitInstance()
{
	return(CWinThread::ExitInstance());
}

BEGIN_MESSAGE_MAP(CBaseTask, CWinThread)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
void CBaseTask::SetSendGui(CWnd* pWnd, const UINT msg)
{
	EnterCriticalSection(&m_CriticalSection);//クリティカルセクション取得
	{
		m_pPostMessageWnd = pWnd;
		m_unWndMsg = msg;
	}
	LeaveCriticalSection(&m_CriticalSection);//クリティカルセクション解放
}
void CBaseTask::SetSendTask(CWinThread* pWinThread, const UINT msg)
{
	EnterCriticalSection(&m_CriticalSection);//クリティカルセクション取得
	{
		m_pPostMessageThread = pWinThread;
		m_unThreadMsg = msg;
	}
	LeaveCriticalSection(&m_CriticalSection);//クリティカルセクション解放
}
void CBaseTask::GetSendGui(CWnd** ppWnd, UINT &msg)
{
	EnterCriticalSection(&m_CriticalSection);//クリティカルセクション取得
	{
		*(ppWnd) = m_pPostMessageWnd;
		msg = m_unWndMsg;
	}
	LeaveCriticalSection(&m_CriticalSection);//クリティカルセクション解放
}
void CBaseTask::GetSendTask(CWinThread** ppWinThread, UINT &msg)
{
	EnterCriticalSection(&m_CriticalSection);//クリティカルセクション取得
	{
		*(ppWinThread) = m_pPostMessageThread;
		msg = m_unThreadMsg;
	}
	LeaveCriticalSection(&m_CriticalSection);//クリティカルセクション解放
}
//////////////////////////////////////////////////////////
//上位にメッセージをPOSTします。 LPARAM は自分のポインタ
void CBaseTask::NotifyTask(const WPARAM wParam, const LPARAM lParam)
{
	CWinThread* pWinThread;
	UINT uMsg;
	GetSendTask(&pWinThread, uMsg);

	if (pWinThread != NULL && uMsg != 0){
		::PostThreadMessage
			(
			pWinThread->m_nThreadID,
			uMsg,
			wParam,
			lParam
			);
	}
}
//////////////////////////////////////////////////////////
//上位にメッセージをPOSTします
void CBaseTask::NotifyGui(const WPARAM wParam, const LPARAM lParam)
{
	CWnd* pWnd;
	UINT uMsg;
	GetSendGui(&pWnd, uMsg);

	if (pWnd != NULL && uMsg != 0){
		::PostMessage
			(
			pWnd->GetSafeHwnd(),
			uMsg,
			wParam,
			lParam
			);
	}
}

// 上位に文字列を通知します(GUIにしか送信しない）
void CBaseTask::NotifyGuiStr(const CString& strTitle, const CString& strMess)
{
	NotifyGuiStr(CNotifyStr::STR_KIND_MESS, strTitle, strMess, 0);
}

// 上位に文字列を通知します
void CBaseTask::NotifyGuiStr(const int nKind, const CString& strTitle, const CString& strMess)
{
	NotifyGuiStr(nKind, strTitle, strMess, 0);
}
// 上位に文字列を通知します
void CBaseTask::NotifyGuiStr(const int nKind, const CString& strTitle, const CString& strMess, const int nCode)
{
	size_t nRetNum = 0;

	EnterCriticalSection(&m_CriticalSection);//クリティカルセクション取得
	{
		nRetNum = m_daNotifyStr.size();

		CNotifyStr dtNotifyStr;
		dtNotifyStr.nKind = nKind;
		dtNotifyStr.strTitle = strTitle;
		dtNotifyStr.strMess = strMess;
		dtNotifyStr.nCode = nCode;

		if (nRetNum < NOTIFY_STR_MAX_NUM){
			m_daNotifyStr.push_back(dtNotifyStr);
			nRetNum = m_daNotifyStr.size();
		}
		NotifyGui(NOTIFY_GUI_STR, nRetNum);
	}
	LeaveCriticalSection(&m_CriticalSection);//クリティカルセクション解放
}

const size_t CBaseTask::GetNotifyStr(std::vector<CNotifyStr>& daNofiyStr)
{
	size_t nRet;
	EnterCriticalSection(&m_CriticalSection);//クリティカルセクション取得
	{
		daNofiyStr = m_daNotifyStr;
		nRet = daNofiyStr.size();
		m_daNotifyStr.clear();
	}
	LeaveCriticalSection(&m_CriticalSection);//クリティカルセクション解放
	return(nRet);
}

//タイマーカウンタの差の計算を行います
int CBaseTask::SubTime(const DWORD dwEnd, const DWORD dwStart)
{
	DWORD dwDuration;
	int nDuration;

	if (dwEnd < dwStart){
		//一回りしている
		dwDuration = ULONG_MAX - dwStart;;
		dwDuration += dwEnd;
	}
	else{
		dwDuration = dwEnd - dwStart;
	}
	if (dwDuration >= INT_MAX){
		nDuration = INT_MAX;
	}
	else{
		nDuration = dwDuration;
	}

	return(nDuration);
}

// タイマーカウンタの差の計算を行います
int CBaseTask::SubTime(const DWORD dwStart)
{
	return(SubTime(timeGetTime(), dwStart));
}

// 現在のタイマーを取得
LONGLONG CBaseTask::GetQueryTime()
{
	LONGLONG xNow;
	QueryPerformanceCounter((LARGE_INTEGER*)&xNow);
	return(xNow);
}

int	CBaseTask::SubQueryTime(const LONGLONG xStart)
{
	return(SubQueryTime(GetQueryTime(), xStart));
}

// 2つのタイマーの差分(ms)を返す
int	CBaseTask::SubQueryTime(const LONGLONG xEnd, const LONGLONG xStart)
{
	LONGLONG xSub;
	if (xEnd > xStart){
		xSub = xEnd - xStart;
	}
	else{
		xSub = _I64_MAX - xStart + xEnd;
	}
	double dSub;
	dSub = xSub / (m_xQueryFreq / 1000.0);//ms
	int nSub;
	nSub = (int)(dSub + 0.5);

	return(nSub);
}

void CBaseTask::QuerySleep(const int nMsec)
{
	LONGLONG xStart = GetQueryTime();
	int i;
	int f;
	if (nMsec < 0) return;

	if (nMsec > 20){
		Sleep(nMsec - 15);
	};

	while (1){
		for (i = 0; i < 65536; i++){
			f = i * rand();
		}
		if (SubQueryTime(GetQueryTime(), xStart) > nMsec) break;
	}
	return;
}


void CBaseTask::MsgLoopQuerySleep(const int nMsec)
{
	LONGLONG xStart = GetQueryTime();
	int i;
	int f;
	if (nMsec < 0) return;
	MSG msg;
	int nDutarion;
	while (1){
		nDutarion = SubQueryTime(xStart);
		if (nDutarion > nMsec){
			break;
		}
		else if (nDutarion - nMsec > 15){
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)){
				if (msg.message == WM_QUIT)	break;
				TranslateMessage(&msg);
				DispatchThreadMessageEx(&msg);
			}
			Sleep(10);
		}
		else{
			for (i = 0; i < 65536; i++){
				f = i * rand();
			}
		}
	}
	return;
}


void CBaseTask::SetMess(const CString& strTitle, const CString& strSrc)
{
	CString strMess = strSrc;
	strMess.Replace(_T('\r'), _T(' '));
	strMess.Replace(_T('\n'), _T(' '));

	//通知
	NotifyGuiStr(strTitle, strMess);
}


void CBaseTask::SetErrorMess(const CString& strTitle, const CString& strSrc)
{
	CString strErrorMess = strSrc;
	//エラー共にログ保存
	strErrorMess.Replace(_T('\r'), _T(' '));
	strErrorMess.Replace(_T('\n'), _T(' '));

	CCSLock csLock(&m_CriticalSection);

	m_strErrorMess = strErrorMess;

	//通知
	NotifyGuiStr(CNotifyStr::STR_KIND_ERR, strTitle, strErrorMess);
}


////////////////////
//概要：警告文字列を通知
//引数：警告文字列
//動作:m_sErrorMessに警告文字列を設定すると共にログに保存します	
void CBaseTask::SetAlartMess(const CString& strTitle, const CString& strSrc)
{
	CString strErrorMess = strSrc;
	//エラー共にログ保存
	strErrorMess.Replace(_T('\r'), _T(' '));
	strErrorMess.Replace(_T('\n'), _T(' '));

	//通知
	NotifyGuiStr(CNotifyStr::STR_KIND_ALT, strTitle, strErrorMess);
}

void CBaseTask::SetErrorPop(const CString& strTitle, const CString& strSrc, const int nCode)
{
	CString strErrorMess = strSrc;
	//エラー共にログ保存
	strErrorMess.Replace(_T('\r'), _T(' '));
	strErrorMess.Replace(_T('\n'), _T(' '));

	CCSLock csLock(&m_CriticalSection);

	m_strErrorMess = strErrorMess;

	//通知
	NotifyGuiStr(CNotifyStr::STR_KIND_ERR_POP, strTitle, strErrorMess, nCode);
}


////////////////////
//概要：警告文字列を通知
//引数：警告文字列
//動作:m_sErrorMessに警告文字列を設定すると共にログに保存します	
void CBaseTask::SetAlartPop(const CString& strTitle, const CString& strSrc, const int nCode)
{
	CString strErrorMess = strSrc;
	//エラー共にログ保存
	strErrorMess.Replace(_T('\r'), _T(' '));
	strErrorMess.Replace(_T('\n'), _T(' '));


	//通知
	NotifyGuiStr(CNotifyStr::STR_KIND_ALT_POP, strTitle, strErrorMess, nCode);
}
CString CBaseTask::LastSysErrString()
{
	CString strErrorMessage;
	DWORD dwErrorCode;
	dwErrorCode = ::GetLastError();

	return(SysErrString(dwErrorCode));
}

CString CBaseTask::SysErrString(const DWORD dwError)
{
	CString strErrorMessage;

	TCHAR caErrorMessage[256] = _T("");

	::FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		NULL, // デフォルト言語
		caErrorMessage,
		256,
		NULL
		);

	strErrorMessage.Format(_T("%s"), caErrorMessage);
	//改行を削除
	strErrorMessage.Replace(_T('\r'), _T(' ')); strErrorMessage.Replace(_T('\n'), _T(' '));
	return(strErrorMessage);
}

CString CBaseTask::ErrorString(const int nRetCode)
{
	int nError = 0;
	int nSrc, nReason;
	CString strRetCode, strTime, strSrc, strReason;

	strRetCode = _T("");
	if (nRetCode == OK){
		return(strRetCode);
	}
	else if (nRetCode <= ALT){
		strTime = _T("注意");
		nError = nRetCode / ALT;
	}
	else if (nRetCode >= ERR){
		strTime = _T("異常");
		nError = nRetCode / ERR;
	}

	nReason = nError % ERS_OFS;
	nSrc = nError - nReason;


	if (nSrc == ERS_OTHER){
		strSrc = _T("不明な原因で");
	}
	else if (nSrc == ERS_NET){
		strSrc = _T("PC通信が");
	}
	else if (nSrc == ERS_COM){
		strSrc = _T("COM PORTが");
	}
	else if (nSrc == ERS_FILE){
		strSrc = _T("PCのFILEが");
	}
	else if (nSrc == ERS_MEM){
		strSrc = _T("PCのMEMORYが");
	}
	else if (nSrc == ERS_USER){
		strSrc = _T("PCのUSER設定が");
	}
	else if (nSrc == ERS_SOCK){
		strSrc = _T("SOCK通信が");
	}
	else if (nSrc == ERS_TASK){
		strSrc = _T("プログラムが");
	}
	else if (nSrc == ERS_SET_OTHER){
		strSrc = _T("その他設定が");
	}
	else if (nSrc == ERS_SET_APP){
		strSrc = _T("SOFTWARE設定が");
	}
	else if (nSrc == ERS_SET_MAC){
		strSrc = _T("装置設定が");
	}
	else if (nSrc == ERS_SET_RECIPE){
		strSrc = _T("RECIPE設定が");
	}
	else if (nSrc == ERS_HD){
		strSrc = _T("その他機器が");
	}
	else if (nSrc == ERS_PLC){
		strSrc = _T("PLCが");
	}
	else if (nSrc == ERS_CTRL_PC){
		strSrc = _T("制御PCが");
	}
	else if (nSrc == ERS_INS_PC){
		strSrc = _T("検査PCが");
	}
	else if (nSrc == ERS_MANAGE_PC){
		strSrc = _T("統合PCが");
	}
	else if (nSrc == ERS_SERVER){
		strSrc = _T("サーバーが");
	}
	else if (nSrc == ERS_AF){
		strSrc = _T("AFが");
	}
	else if (nSrc == ERS_LIGHT){
		strSrc = _T("照明が");
	}
	else if (nSrc == ERS_CAMERA){
		strSrc = _T("CAMERAが");
	}
	else if (nSrc == ERS_GRABBER){
		strSrc = _T("撮像ボードが");
	}
	else if (nSrc == ERS_UPS){
		strSrc = _T("UPSが");
	}
	else{ strSrc = _T("不明な原因で"); }

	if (nReason == ERR_OTHER){
		strReason = _T("問題が発生しました");//その他
	}
	else if (nReason == ERR_ABORT){
		strReason = _T("中断");//中断
	}
	else if (nReason == ERR_CREATE){
		strReason = _T("作成失敗");//書けない
	}
	else if (nReason == ERR_CLOSED){
		strReason = _T("開いていない");//書けない
	}
	else if (nReason == ERR_OPEN){
		strReason = _T("開けない");//開けない
	}
	else if (nReason == ERR_READ){
		strReason = _T("読み込み失敗");//開けない
	}
	else if (nReason == ERR_WRITE){
		strReason = _T("書き込み失敗");//書けない
	}
	else if (nReason == ERR_REMOVE){
		strReason = _T("削除失敗");//消せない
	}
	else if (nReason == ERR_REJECTED){
		strReason = _T("拒否されました");//拒否された
	}
	else if (nReason == ERR_STARVED){
		strReason = _T("不足しました");//不足した
	}
	else if (nReason == ERR_OVERMUCH){
		strReason = _T("多過ぎる");//過多
	}
	else if (nReason == ERR_NOT_FOUND){
		strReason = _T("見つかりません");//見つからない
	}
	else if (nReason == ERR_MISSED){
		strReason = _T("失敗しました");//失敗
	}
	else if (nReason == ERR_BROKEN){
		strReason = _T("壊れました");//壊れた
	}
	else if (nReason == ERR_DATA_WRONG){
		strReason = _T("DATA異常がありました");//データ異常
	}
	else if (nReason == ERR_NO_RESPONSE){
		strReason = _T("応答無し");//応答なし
	}
	else if (nReason == ERR_TIMEOUT){
		strReason = _T("TIMEOUT");//応答なし
	}
	else if (nReason == ERR_DEGRADE){
		strReason = _T("劣化しました");//劣化
	}
	else{
		strReason = _T("問題が発生しました");
	}


	strRetCode = _T("[") + strTime + _T("]") + strSrc + _T(" ") + strReason;

	return(strRetCode);

}

CString  CBaseTask::ErrorStringA(const int nRetCode)
{
	int nError = 0;
	int nSrc, nReason;
	CString strRetCode, strTime, strSrc, strReason;

	strRetCode = _T("");
	if (nRetCode == OK){
		return(strRetCode);
	}
	else if (nRetCode <= ALT){
		strTime = _T("CAUTION");
		nError = nRetCode / ALT;
	}
	else if (nRetCode >= ERR){
		strTime = _T("ERROR");
		nError = nRetCode / ERR;
	}

	nReason = nError % ERS_OFS;
	nSrc = nError - nReason;


	if (nSrc == ERS_OTHER){
		strSrc = _T("Unknown");
	}
	else if (nSrc == ERS_NET){
		strSrc = _T("Ethrer");
	}
	else if (nSrc == ERS_COM){
		strSrc = _T("SerialPort");
	}
	else if (nSrc == ERS_FILE){
		strSrc = _T("PC File");
	}
	else if (nSrc == ERS_MEM){
		strSrc = _T("PC Memory");
	}
	else if (nSrc == ERS_USER){
		strSrc = _T("User Setting");
	}
	else if (nSrc == ERS_SOCK){
		strSrc = _T("TCIP Sock");
	}
	else if (nSrc == ERS_TASK){
		strSrc = _T("Program");
	}
	else if (nSrc == ERS_SET_OTHER){
		strSrc = _T("Other Settings");
	}
	else if (nSrc == ERS_SET_APP){
		strSrc = _T("Software Setting");
	}
	else if (nSrc == ERS_SET_MAC){
		strSrc = _T("Machine Setting");
	}
	else if (nSrc == ERS_SET_RECIPE){
		strSrc = _T("Recipe Setting");
	}
	else if (nSrc == ERS_HD){
		strSrc = _T("Other Devices");
	}
	else if (nSrc == ERS_PLC){
		strSrc = _T("PLC ");
	}
	else if (nSrc == ERS_CTRL_PC){
		strSrc = _T("Ctrl PC");
	}
	else if (nSrc == ERS_INS_PC){
		strSrc = _T("Ins PC");
	}
	else if (nSrc == ERS_MANAGE_PC){
		strSrc = _T("Manage PC");
	}
	else if (nSrc == ERS_SERVER){
		strSrc = _T("Server");
	}
	else if (nSrc == ERS_AF){
		strSrc = _T("AF");
	}
	else if (nSrc == ERS_LIGHT){
		strSrc = _T("Light");
	}
	else if (nSrc == ERS_CAMERA){
		strSrc = _T("CAMERA");
	}
	else if (nSrc == ERS_GRABBER){
		strSrc = _T("GRABBER");
	}
	else if (nSrc == ERS_UPS){
		strSrc = _T("UPS");
	}
	else if (nSrc == ERS_LOT_ID){
		strSrc = _T("LOT ID");
	}
	else{ strSrc = _T("Unknown"); }

	if (nReason == ERR_OTHER){
		strReason = _T("had problem");//その他
	}
	else if (nReason == ERR_ABORT){
		strReason = _T("Aborted");//中断
	}
	else if (nReason == ERR_CLOSED){
		strReason = _T("closed");//開いていない
	}
	else if (nReason == ERR_OPEN){
		strReason = _T("couldn't open");//開けない
	}
	else if (nReason == ERR_CREATE){
		strReason = _T("couldn't create");//書けない
	}
	else if (nReason == ERR_WRITE){
		strReason = _T("couldn't write");//書けない
	}
	else if (nReason == ERR_READ){
		strReason = _T("couldn't read");//書けない
	}
	else if (nReason == ERR_REMOVE){
		strReason = _T("couldn't remove");//消せない
	}
	else if (nReason == ERR_REJECTED){
		strReason = _T("rejected");//拒否された
	}
	else if (nReason == ERR_STARVED){
		strReason = _T("starved");//不足した
	}
	else if (nReason == ERR_OVERMUCH){
		strReason = _T("over much");//過多
	}
	else if (nReason == ERR_NOT_FOUND){
		strReason = _T("not found");//見つからない
	}
	else if (nReason == ERR_MISSED){
		strReason = _T("missed");//失敗
	}
	else if (nReason == ERR_BROKEN){
		strReason = _T("broken");//壊れた
	}
	else if (nReason == ERR_DATA_WRONG){
		strReason = _T("had Bad Data");//データ異常
	}
	else if (nReason == ERR_NO_RESPONSE){
		strReason = _T("no response");//応答なし
	}
	else if (nReason == ERR_TIMEOUT){
		strReason = _T("timeout");//応答なし
	}
	else if (nReason == ERR_DEGRADE){
		strReason = _T("degraded");//劣化
	}
	else{
		strReason = _T("had problem");
	}


	strRetCode = _T("[") + strTime + _T("]") + strSrc + _T(" ") + strReason;

	return(strRetCode);
}