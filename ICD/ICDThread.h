#pragma once
#include "stdafx.h"
#include "BaseTask.h"
#include "opencv\highgui.h"
#include "opencv\cv.h"

class CICDThread : public CBaseTask
{
	DECLARE_DYNCREATE(CICDThread)
public:
	CICDThread();
	~CICDThread();

	void OnMainThreadMessage(WPARAM wParam, LPARAM lParam);

	enum GET_MSG{
		MSG_FROM_MAIN = CBaseTask::MSG_NEXT_NO + 100,
		MSG_NEXT_NO,
	};

	// NOTIFY
	enum NOTIFY{
		NOTIFY_STATUS = CBaseTask::NOTIFY_NEXT_NO + 100,
		NOTIFY_CAPTURE_ERR,	// CAPTUREエラー
		NOTIFY_DETECT_ERR,	// DETECTエラー
		NOTIFY_CAPTURE_END,	// CAPTURE終了
		NOTIFY_DETECT_END,  // DETECT終了
	};
	/*
	struct RADIO_VALUE{
		static const int	CIRCLE_DETECT;
		static const int	SQUARE_DETECT;
		static const int	ENGLISH_DETECT;
	};
	*/

	void GetImage(IplImage** image);
	void IplToBmp(const IplImage* srcIplImage, CBitmap* BmpMapData);

	CString m_strDetectText;

protected:
	CBitmap m_cBitmap;
	IplImage* m_iCaptureImage;
	IplImage* m_iDetectImage;
	void Capture();
	struct RADIO_VALUE{
		static const int	CIRCLE_DETECT;
		static const int	SQUARE_DETECT;
		static const int	ENGLISH_DETECT;
	};

	DECLARE_MESSAGE_MAP()
	BOOL InitInstance();
	bool Init();
	int  ExitInstance();
	void DetectCircle(const IplImage* srcIplImage, IplImage** getOutputImage);
	void DetectSquare(const IplImage* srcIplImage, IplImage** getOutputImage);
	void DetectEngText(const IplImage* srcIplImage, IplImage** getOutputImage);



};

