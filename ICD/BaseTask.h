#pragma once

#include <afxmt.h>
#include <mmsystem.h>
#include <vector>
//#include <afxwin.h>

// CBaseTask

// @(s)
// CBaseTask スレッド
// CWinThread派生の上位通知関数を追加した版（色々なクラスの派生元）
// このクラスの派生を作るときは一旦MFCでCWinThread派生で作成しCWinThreadをCBaseTaskに置換してください
/////////////////////////////////////////////////////////////////////////////
class CCSLock
{
public:
	CCSLock(CRITICAL_SECTION *cs) :m_pcs(cs){
		if (m_pcs != NULL){
			EnterCriticalSection(m_pcs);
		}
	};
	void SetCritiCalSection(CRITICAL_SECTION *p){
		if (p != NULL){
			m_pcs = p;
			EnterCriticalSection(m_pcs);
		}
	}

	~CCSLock(void){
		if (m_pcs != NULL){
			LeaveCriticalSection(m_pcs);
		}
	};

private:
	CRITICAL_SECTION* m_pcs;
};

class CBaseStatus
{
public:
	CBaseStatus(void){ Clear(); };
	~CBaseStatus(void){};

	//状態
	CString strStatus;

	//状況
	BOOL	blBusy;			//タスク処理中
	BOOL	blAbort;			//中断処理中
	WPARAM	wParam;
	LPARAM	lParam;
	UINT	uMsg;
	int		nLastRet;
	CString strErrorMess;

	void Clear()
	{
		blBusy = FALSE;
		blAbort = FALSE;
		wParam = NULL;
		lParam = NULL;
		uMsg = NULL;
		nLastRet = NULL;
		strErrorMess = _T("");
		strStatus = _T("");
	};


	BOOL operator==(const CBaseStatus &src)
	{
		if (strStatus != src.strStatus)	return(FALSE);
		if (blBusy != src.blBusy)	return(FALSE);
		if (blAbort != src.blAbort)	return(FALSE);
		if (wParam != src.wParam)	return(FALSE);
		if (lParam != src.lParam)	return(FALSE);
		if (uMsg != src.uMsg)	return(FALSE);
		if (nLastRet != src.nLastRet)	return(FALSE);

		return(TRUE);
	};
	BOOL operator!=(const CBaseStatus &src)
	{
		return(!(operator==(src)));
	};

};

class CNotifyStr
{
public:
	CNotifyStr(void){ Clear(); };
	CNotifyStr(const int nKindT, const CString& strT, const CString& strM)
	{
		nKind = nKindT;
		strTitle = strT;
		strMess = strM;
	};

	CNotifyStr(const int nKindT, const CString& strT, const CString& strM, const int nC)
	{
		nKind = nKindT;
		strTitle = strT;
		strMess = strM;
		nCode = nC;
	};

	~CNotifyStr(void){};

	//状態
	CString	strTitle;
	CString strMess;
	int		nKind;
	int		nCode;

	enum STR_KIND
	{
		STR_KIND_MESS = 0,
		STR_KIND_ALT,
		STR_KIND_ERR,
		STR_KIND_ALT_POP,
		STR_KIND_ERR_POP,
	};


	void Clear()
	{
		nKind = STR_KIND_MESS;
		strTitle = _T("");
		strMess = _T("");
		nCode = 0;
	};
};

class CBaseTask : public CWinThread
{
	DECLARE_DYNCREATE(CBaseTask)


public:
	// 戻り値
	enum RET_CODE
	{
		ALT = -1,	// 警告
		OK = 0, 	// OK
		ERR = 1,	// 何かエラー
	};

	enum ERR_REASON
	{
		ERR_OTHER = 1,
		ERR_ABORT,
		ERR_CLOSED,			//開いていない
		ERR_OPEN,			//開けない
		ERR_CREATE,			//開けない
		ERR_READ,			//開けない
		ERR_WRITE,			//書けない
		ERR_REMOVE,			//消せない
		ERR_REJECTED,		//拒否された
		ERR_STARVED,		//不足した
		ERR_OVERMUCH,		//過多
		ERR_NOT_FOUND,		//見つからない
		ERR_MISSED,			//失敗
		ERR_BROKEN,			//壊れた
		ERR_DATA_WRONG,		//データ異常
		ERR_NO_RESPONSE,	//応答なし
		ERR_TIMEOUT,		//時間切れ
		ERR_DEGRADE,		//劣化
		ERR_NEXT_NO,
	};

	enum ERR_SRC
	{
		ERS_OFS = 100,
		ERS_OTHER = 1,
		ERS_NET,
		ERS_COM,
		ERS_FILE,
		ERS_MEM,
		ERS_USER,
		ERS_SOCK,
		ERS_TASK,
		ERS_ARG,

		ERS_SET_OTHER,
		ERS_SET_APP,
		ERS_SET_MAC,
		ERS_SET_RECIPE,

		ERS_HD,
		ERS_PLC,
		ERS_CTRL_PC,
		ERS_INS_PC,
		ERS_MANAGE_PC,
		ERS_SERVER,
		ERS_AF,
		ERS_LIGHT,
		ERS_CAMERA,
		ERS_UPS,
		ERS_LOT_ID,
		ERS_GRABBER,
		ERS_NEXT_NO,
	};

	int Err(const int nErr, const int nErs){ return(ERR*(nErr + nErs*ERS_OFS)); };
	int Alt(const int nErr, const int nErs){ return(ALT*(nErr + nErs*ERS_OFS)); };


	enum NOTIFY_GUI
	{
		NOTIFY_GUI_STR = 1,//GUIに文字列通知
		NOTIFY_NEXT_NO,
		NOTIFY_STR_MAX_NUM = 256,
	};

	enum GET_MSG
	{
		MSG_NEXT_NO = WM_APP,
	};


	CBaseTask();
	virtual ~CBaseTask();
	virtual BOOL InitInstance();
	virtual int  ExitInstance();


	//上位クラス(通知先）の設定
	virtual void SetSendGui(CWnd* pWnd, const UINT msg);
	virtual void SetSendTask(CWinThread* pWinThread, const UINT msg);

	virtual void GetSendGui(CWnd** ppWnd, UINT &msg);
	virtual void GetSendTask(CWinThread** ppWinThread, UINT &msg);

	CString ErrorMess()
	{
		CCSLock csLock(&m_CriticalSection);
		return(m_strErrorMess);
	}

	CString LastSysErrString();
	CString SysErrString(const DWORD dwError);
	const size_t GetNotifyStr(std::vector<CNotifyStr>& daNofiyStr);


	virtual void SetMess(const CString& strTitle, const CString& strMess);
	virtual void SetErrorMess(const CString& strTitle, const CString& strErrorMess);
	virtual void SetAlartMess(const CString& strTitle, const CString& strErrorMess);
	virtual void SetErrorPop(const CString& strTitle, const CString& strErrorMess, const int nCode);
	virtual void SetAlartPop(const CString& strTitle, const CString& strErrorMess, const int nCode);

	CString  ErrorString(const int nRetCode);
	CString  ErrorStringA(const int nRetCode);
protected:


	// GUIに通知する関連
	virtual void NotifyGui(const WPARAM wParam, const LPARAM lParam);
	// メッセージを送る先
	CWnd*	m_pPostMessageWnd;
	// 送るメッセージ
	UINT	m_unWndMsg;


	// GUIに文字列を送る
	// NOTIFY_STR
	virtual void NotifyGuiStr(const CString& strTitle, const CString& strMess);
	// NOTIFY_STR
	virtual void NotifyGuiStr(const int nKind, const CString& strTitle, const CString& strMess);
	// NOTIFY_STR
	virtual void NotifyGuiStr(const int nKind, const CString& strTitle, const CString& strMess, const int nCode);
	virtual void NotifyTask(const WPARAM wParam, const LPARAM lParam);



	std::vector<CNotifyStr>	m_daNotifyStr;
	CString					m_strErrorMess;
	// メッセージを送る先
	CWinThread*				m_pPostMessageThread;
	// 送るメッセージ
	UINT					m_unThreadMsg;
	// クリティカルセクション
	CRITICAL_SECTION		m_CriticalSection;

	int SubTime(const DWORD dwEnd, const DWORD dwStart);
	int SubTime(const DWORD dwStart);

	// QueryCounter
	LONGLONG	m_xQueryFreq;
	// 現在のタイマーを取得
	LONGLONG	GetQueryTime();
	// 2つのタイマーの差分(ms)を返す
	int	SubQueryTime(const LONGLONG xEnd, const LONGLONG xStart);
	//2つのタイマーの差分(ms)を返す
	int	SubQueryTime(const LONGLONG xStart);

	void	QuerySleep(const int nMsec);
	void	MsgLoopQuerySleep(const int nMsec);

protected:
	DECLARE_MESSAGE_MAP()
};


