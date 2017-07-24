#pragma once
#include "stdafx.h"
#include "BaseTask.h"
#include "ICDThread.h"


class CMainThread : public CBaseTask
{
	DECLARE_DYNCREATE(CMainThread)
public:
	CMainThread();
	virtual ~CMainThread();
	virtual BOOL	InitInstance();
	virtual int		ExitInstance();
	
	enum GET_MSG{
		MSG_FROM_GUI = CBaseTask::MSG_NEXT_NO + 50,
		MSG_FROM_ICD,
		MSG_NEXT_NO,
	};

	enum WPRM_MAIN{
		WPRM_START = 0,
		WPRM_CAPTURE,
		WPRM_DETECT,
	};

	IplImage* m_iImage;

	void GetImage(IplImage** image);

protected:
	CICDThread* m_pICDThread;
	DECLARE_MESSAGE_MAP()
	void OnGUIMessage(WPARAM wParam, LPARAM lParam);
	void OnICDMessage(WPARAM wParam, LPARAM lParam);
	void SendICDThreadMess(WPARAM wParam, LPARAM lParam);
	bool Init();


};