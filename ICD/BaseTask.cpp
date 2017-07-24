// BaseTask.cpp : �����t�@�C��
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

	// �^�C�}�[
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
	EnterCriticalSection(&m_CriticalSection);//�N���e�B�J���Z�N�V�����擾
	{
		m_pPostMessageWnd = pWnd;
		m_unWndMsg = msg;
	}
	LeaveCriticalSection(&m_CriticalSection);//�N���e�B�J���Z�N�V�������
}
void CBaseTask::SetSendTask(CWinThread* pWinThread, const UINT msg)
{
	EnterCriticalSection(&m_CriticalSection);//�N���e�B�J���Z�N�V�����擾
	{
		m_pPostMessageThread = pWinThread;
		m_unThreadMsg = msg;
	}
	LeaveCriticalSection(&m_CriticalSection);//�N���e�B�J���Z�N�V�������
}
void CBaseTask::GetSendGui(CWnd** ppWnd, UINT &msg)
{
	EnterCriticalSection(&m_CriticalSection);//�N���e�B�J���Z�N�V�����擾
	{
		*(ppWnd) = m_pPostMessageWnd;
		msg = m_unWndMsg;
	}
	LeaveCriticalSection(&m_CriticalSection);//�N���e�B�J���Z�N�V�������
}
void CBaseTask::GetSendTask(CWinThread** ppWinThread, UINT &msg)
{
	EnterCriticalSection(&m_CriticalSection);//�N���e�B�J���Z�N�V�����擾
	{
		*(ppWinThread) = m_pPostMessageThread;
		msg = m_unThreadMsg;
	}
	LeaveCriticalSection(&m_CriticalSection);//�N���e�B�J���Z�N�V�������
}
//////////////////////////////////////////////////////////
//��ʂɃ��b�Z�[�W��POST���܂��B LPARAM �͎����̃|�C���^
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
//��ʂɃ��b�Z�[�W��POST���܂�
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

// ��ʂɕ������ʒm���܂�(GUI�ɂ������M���Ȃ��j
void CBaseTask::NotifyGuiStr(const CString& strTitle, const CString& strMess)
{
	NotifyGuiStr(CNotifyStr::STR_KIND_MESS, strTitle, strMess, 0);
}

// ��ʂɕ������ʒm���܂�
void CBaseTask::NotifyGuiStr(const int nKind, const CString& strTitle, const CString& strMess)
{
	NotifyGuiStr(nKind, strTitle, strMess, 0);
}
// ��ʂɕ������ʒm���܂�
void CBaseTask::NotifyGuiStr(const int nKind, const CString& strTitle, const CString& strMess, const int nCode)
{
	size_t nRetNum = 0;

	EnterCriticalSection(&m_CriticalSection);//�N���e�B�J���Z�N�V�����擾
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
	LeaveCriticalSection(&m_CriticalSection);//�N���e�B�J���Z�N�V�������
}

const size_t CBaseTask::GetNotifyStr(std::vector<CNotifyStr>& daNofiyStr)
{
	size_t nRet;
	EnterCriticalSection(&m_CriticalSection);//�N���e�B�J���Z�N�V�����擾
	{
		daNofiyStr = m_daNotifyStr;
		nRet = daNofiyStr.size();
		m_daNotifyStr.clear();
	}
	LeaveCriticalSection(&m_CriticalSection);//�N���e�B�J���Z�N�V�������
	return(nRet);
}

//�^�C�}�[�J�E���^�̍��̌v�Z���s���܂�
int CBaseTask::SubTime(const DWORD dwEnd, const DWORD dwStart)
{
	DWORD dwDuration;
	int nDuration;

	if (dwEnd < dwStart){
		//���肵�Ă���
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

// �^�C�}�[�J�E���^�̍��̌v�Z���s���܂�
int CBaseTask::SubTime(const DWORD dwStart)
{
	return(SubTime(timeGetTime(), dwStart));
}

// ���݂̃^�C�}�[���擾
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

// 2�̃^�C�}�[�̍���(ms)��Ԃ�
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

	//�ʒm
	NotifyGuiStr(strTitle, strMess);
}


void CBaseTask::SetErrorMess(const CString& strTitle, const CString& strSrc)
{
	CString strErrorMess = strSrc;
	//�G���[���Ƀ��O�ۑ�
	strErrorMess.Replace(_T('\r'), _T(' '));
	strErrorMess.Replace(_T('\n'), _T(' '));

	CCSLock csLock(&m_CriticalSection);

	m_strErrorMess = strErrorMess;

	//�ʒm
	NotifyGuiStr(CNotifyStr::STR_KIND_ERR, strTitle, strErrorMess);
}


////////////////////
//�T�v�F�x���������ʒm
//�����F�x��������
//����:m_sErrorMess�Ɍx���������ݒ肷��Ƌ��Ƀ��O�ɕۑ����܂�	
void CBaseTask::SetAlartMess(const CString& strTitle, const CString& strSrc)
{
	CString strErrorMess = strSrc;
	//�G���[���Ƀ��O�ۑ�
	strErrorMess.Replace(_T('\r'), _T(' '));
	strErrorMess.Replace(_T('\n'), _T(' '));

	//�ʒm
	NotifyGuiStr(CNotifyStr::STR_KIND_ALT, strTitle, strErrorMess);
}

void CBaseTask::SetErrorPop(const CString& strTitle, const CString& strSrc, const int nCode)
{
	CString strErrorMess = strSrc;
	//�G���[���Ƀ��O�ۑ�
	strErrorMess.Replace(_T('\r'), _T(' '));
	strErrorMess.Replace(_T('\n'), _T(' '));

	CCSLock csLock(&m_CriticalSection);

	m_strErrorMess = strErrorMess;

	//�ʒm
	NotifyGuiStr(CNotifyStr::STR_KIND_ERR_POP, strTitle, strErrorMess, nCode);
}


////////////////////
//�T�v�F�x���������ʒm
//�����F�x��������
//����:m_sErrorMess�Ɍx���������ݒ肷��Ƌ��Ƀ��O�ɕۑ����܂�	
void CBaseTask::SetAlartPop(const CString& strTitle, const CString& strSrc, const int nCode)
{
	CString strErrorMess = strSrc;
	//�G���[���Ƀ��O�ۑ�
	strErrorMess.Replace(_T('\r'), _T(' '));
	strErrorMess.Replace(_T('\n'), _T(' '));


	//�ʒm
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
		NULL, // �f�t�H���g����
		caErrorMessage,
		256,
		NULL
		);

	strErrorMessage.Format(_T("%s"), caErrorMessage);
	//���s���폜
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
		strTime = _T("����");
		nError = nRetCode / ALT;
	}
	else if (nRetCode >= ERR){
		strTime = _T("�ُ�");
		nError = nRetCode / ERR;
	}

	nReason = nError % ERS_OFS;
	nSrc = nError - nReason;


	if (nSrc == ERS_OTHER){
		strSrc = _T("�s���Ȍ�����");
	}
	else if (nSrc == ERS_NET){
		strSrc = _T("PC�ʐM��");
	}
	else if (nSrc == ERS_COM){
		strSrc = _T("COM PORT��");
	}
	else if (nSrc == ERS_FILE){
		strSrc = _T("PC��FILE��");
	}
	else if (nSrc == ERS_MEM){
		strSrc = _T("PC��MEMORY��");
	}
	else if (nSrc == ERS_USER){
		strSrc = _T("PC��USER�ݒ肪");
	}
	else if (nSrc == ERS_SOCK){
		strSrc = _T("SOCK�ʐM��");
	}
	else if (nSrc == ERS_TASK){
		strSrc = _T("�v���O������");
	}
	else if (nSrc == ERS_SET_OTHER){
		strSrc = _T("���̑��ݒ肪");
	}
	else if (nSrc == ERS_SET_APP){
		strSrc = _T("SOFTWARE�ݒ肪");
	}
	else if (nSrc == ERS_SET_MAC){
		strSrc = _T("���u�ݒ肪");
	}
	else if (nSrc == ERS_SET_RECIPE){
		strSrc = _T("RECIPE�ݒ肪");
	}
	else if (nSrc == ERS_HD){
		strSrc = _T("���̑��@�킪");
	}
	else if (nSrc == ERS_PLC){
		strSrc = _T("PLC��");
	}
	else if (nSrc == ERS_CTRL_PC){
		strSrc = _T("����PC��");
	}
	else if (nSrc == ERS_INS_PC){
		strSrc = _T("����PC��");
	}
	else if (nSrc == ERS_MANAGE_PC){
		strSrc = _T("����PC��");
	}
	else if (nSrc == ERS_SERVER){
		strSrc = _T("�T�[�o�[��");
	}
	else if (nSrc == ERS_AF){
		strSrc = _T("AF��");
	}
	else if (nSrc == ERS_LIGHT){
		strSrc = _T("�Ɩ���");
	}
	else if (nSrc == ERS_CAMERA){
		strSrc = _T("CAMERA��");
	}
	else if (nSrc == ERS_GRABBER){
		strSrc = _T("�B���{�[�h��");
	}
	else if (nSrc == ERS_UPS){
		strSrc = _T("UPS��");
	}
	else{ strSrc = _T("�s���Ȍ�����"); }

	if (nReason == ERR_OTHER){
		strReason = _T("��肪�������܂���");//���̑�
	}
	else if (nReason == ERR_ABORT){
		strReason = _T("���f");//���f
	}
	else if (nReason == ERR_CREATE){
		strReason = _T("�쐬���s");//�����Ȃ�
	}
	else if (nReason == ERR_CLOSED){
		strReason = _T("�J���Ă��Ȃ�");//�����Ȃ�
	}
	else if (nReason == ERR_OPEN){
		strReason = _T("�J���Ȃ�");//�J���Ȃ�
	}
	else if (nReason == ERR_READ){
		strReason = _T("�ǂݍ��ݎ��s");//�J���Ȃ�
	}
	else if (nReason == ERR_WRITE){
		strReason = _T("�������ݎ��s");//�����Ȃ�
	}
	else if (nReason == ERR_REMOVE){
		strReason = _T("�폜���s");//�����Ȃ�
	}
	else if (nReason == ERR_REJECTED){
		strReason = _T("���ۂ���܂���");//���ۂ��ꂽ
	}
	else if (nReason == ERR_STARVED){
		strReason = _T("�s�����܂���");//�s������
	}
	else if (nReason == ERR_OVERMUCH){
		strReason = _T("���߂���");//�ߑ�
	}
	else if (nReason == ERR_NOT_FOUND){
		strReason = _T("������܂���");//������Ȃ�
	}
	else if (nReason == ERR_MISSED){
		strReason = _T("���s���܂���");//���s
	}
	else if (nReason == ERR_BROKEN){
		strReason = _T("���܂���");//��ꂽ
	}
	else if (nReason == ERR_DATA_WRONG){
		strReason = _T("DATA�ُ킪����܂���");//�f�[�^�ُ�
	}
	else if (nReason == ERR_NO_RESPONSE){
		strReason = _T("��������");//�����Ȃ�
	}
	else if (nReason == ERR_TIMEOUT){
		strReason = _T("TIMEOUT");//�����Ȃ�
	}
	else if (nReason == ERR_DEGRADE){
		strReason = _T("�򉻂��܂���");//��
	}
	else{
		strReason = _T("��肪�������܂���");
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
		strReason = _T("had problem");//���̑�
	}
	else if (nReason == ERR_ABORT){
		strReason = _T("Aborted");//���f
	}
	else if (nReason == ERR_CLOSED){
		strReason = _T("closed");//�J���Ă��Ȃ�
	}
	else if (nReason == ERR_OPEN){
		strReason = _T("couldn't open");//�J���Ȃ�
	}
	else if (nReason == ERR_CREATE){
		strReason = _T("couldn't create");//�����Ȃ�
	}
	else if (nReason == ERR_WRITE){
		strReason = _T("couldn't write");//�����Ȃ�
	}
	else if (nReason == ERR_READ){
		strReason = _T("couldn't read");//�����Ȃ�
	}
	else if (nReason == ERR_REMOVE){
		strReason = _T("couldn't remove");//�����Ȃ�
	}
	else if (nReason == ERR_REJECTED){
		strReason = _T("rejected");//���ۂ��ꂽ
	}
	else if (nReason == ERR_STARVED){
		strReason = _T("starved");//�s������
	}
	else if (nReason == ERR_OVERMUCH){
		strReason = _T("over much");//�ߑ�
	}
	else if (nReason == ERR_NOT_FOUND){
		strReason = _T("not found");//������Ȃ�
	}
	else if (nReason == ERR_MISSED){
		strReason = _T("missed");//���s
	}
	else if (nReason == ERR_BROKEN){
		strReason = _T("broken");//��ꂽ
	}
	else if (nReason == ERR_DATA_WRONG){
		strReason = _T("had Bad Data");//�f�[�^�ُ�
	}
	else if (nReason == ERR_NO_RESPONSE){
		strReason = _T("no response");//�����Ȃ�
	}
	else if (nReason == ERR_TIMEOUT){
		strReason = _T("timeout");//�����Ȃ�
	}
	else if (nReason == ERR_DEGRADE){
		strReason = _T("degraded");//��
	}
	else{
		strReason = _T("had problem");
	}


	strRetCode = _T("[") + strTime + _T("]") + strSrc + _T(" ") + strReason;

	return(strRetCode);
}