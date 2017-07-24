#include "stdafx.h"
#include "stdlib.h"
#include "ICDThread.h"
#include "MainThread.h"



IMPLEMENT_DYNCREATE(CICDThread, CBaseTask)
// メッセージ
BEGIN_MESSAGE_MAP(CICDThread, CBaseTask)
	ON_THREAD_MESSAGE(MSG_FROM_MAIN, OnMainThreadMessage)
END_MESSAGE_MAP()

/*
const int CICDThread::RADIO_VALUE::CIRCLE_DETECT = 0;
const int CICDThread::RADIO_VALUE::SQUARE_DETECT = 1;
const int CICDThread::RADIO_VALUE::ENGLISH_DETECT = 2;
*/

CICDThread::CICDThread()
{

}


CICDThread::~CICDThread()
{
}


/****************************************************************************
Name : InitInstance()
Desc : 開始時に呼ばれる　
****************************************************************************/
BOOL CICDThread::InitInstance()
{
	

	//m_cStatus.bBusy = true;
	// 初期化
	if (!Init())
	{
		// 失敗
		//m_cStatus.bBusy = false;
		//m_cStatus.strErr = _T("初期化エラー");
		//ChangeStatus();
		//NotifyTask(NOTIFY_ERR, NULL);
		return(TRUE);	// FALSEを返すと終了してしまうのでTRUEを返す
	}

	// 成功
	//m_cStatus.bBusy = false;
	//ChangeStatus();
	//NotifyTask(NOTIFY_INIT_OK, NULL);
	//LOG(_T("初期化完了 ======================================"));
	return(TRUE);
}

/****************************************************************************
Name : Init()
Desc : スレッド初期処理
****************************************************************************/
bool CICDThread::Init()
{

	// クリティカルセクション初期化
	InitializeCriticalSection(&m_CriticalSection);
	return(true);
}

/****************************************************************************
Name : ExitInstance()
Desc : 終了時に呼ばれる　
****************************************************************************/
int CICDThread::ExitInstance()
{
	// クリティカルセクションの破棄
	DeleteCriticalSection(&m_CriticalSection);

	return(CBaseTask::ExitInstance());
}

/****************************************************************************
Name : GetImage()
Desc : 他のスレッドがこの関数をつかって画像を取得する　
****************************************************************************/
void CICDThread::GetImage(IplImage** image, int type)
{
	EnterCriticalSection(&m_CriticalSection);
	{
		if (type == 0)
		{
			*image = cvCloneImage(m_iCaptureImage);
		}
		else if (type == 1)
		{
			*image = cvCloneImage(m_iDetectImage);
		}
	}
	LeaveCriticalSection(&m_CriticalSection);
}



/****************************************************************************
Name : OnMainThreadMessage()
Desc : Mainスレッドからのメッセージハンドラ　
****************************************************************************/
void CICDThread::OnMainThreadMessage(WPARAM wParam, LPARAM lParam)
{
	CString strLog;
	CImage img;

	switch (wParam)
	{
	case CMainThread::WPRM_CAPTURE:
		// キャプチャ
		Capture();		
		NotifyTask(NOTIFY_CAPTURE_END, NULL);
		break;
	case CMainThread::WPRM_DETECT:
		
		switch (lParam)
		{
		case 0:
			DetectCircle(m_iCaptureImage, &m_iDetectImage);
			break;
		case 1:
			DetectSquare(m_iCaptureImage, &m_iDetectImage);
			break;
		case 2:
			
			break;
		}
		
		NotifyTask(NOTIFY_DETECT_END, NULL);
		break;

	}

}


/****************************************************************************
Name : Capture()
Desc : 接続しているウェブカメラでキャプチャを行う
****************************************************************************/
void CICDThread::Capture()
{

	// カメラから取り込む
	CvCapture *capture = cvCreateCameraCapture(0);

	//cvNamedWindow("debug", CV_WINDOW_AUTOSIZE);

	// カメラが見つからない
	if (capture == NULL)
	{
		NotifyTask(NOTIFY_CAPTURE_ERR, NULL);
		return;
	}
	//カメラのサイズを取り込みます。 cvGetCapturePropertyの戻り値はdoubleなので、intにキャスト
	CvSize size = cvSize
		(
		(int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH),
		(int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT)
		);

	CvSize normalsize = cvSize((int)size.width, (int)size.height);
	CvSize resizesize = cvSize((int)size.width, (int)size.height);

	m_iCaptureImage = cvCreateImage(normalsize, IPL_DEPTH_8U, 3);

	IplImage* CaptureFrame = cvCreateImage(normalsize, IPL_DEPTH_8U, 3);

	// 1ショットだけだとリソース解放時にエラー? とりあえず10ショット
	int count = 0;
	while (count < 10){
		try
		{
			CaptureFrame = cvQueryFrame(capture);
		}
		catch (std::exception ex)
		{
			// キャプチャ失敗
		}
		count++;
	}

	if (CaptureFrame == NULL)
	{
		NotifyTask(NOTIFY_CAPTURE_ERR, NULL);
		return;
	}

	//cvShowImage("debug", CaptureFrame);

	m_iCaptureImage = cvCloneImage(CaptureFrame);

	try{
		//リソースを開放
		cvReleaseCapture(&capture);
	}
	catch (std::exception ex)
	{
	
	}

}

/****************************************************************************
Name : IplToBmp()
Desc : IPLイメージをビットマップに変換　
****************************************************************************/
void CICDThread::IplToBmp(const IplImage* srcIplImage, CBitmap* BmpMapData)
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

	//カラーバッファ内容をBitmapに展開
	BmpMapData->CreateBitmap(Width, Height, 1, 32, ColorBuf);
	free(ColorBuf);
}


/****************************************************************************
Name : DetectCircle()
Desc : 円の検出
****************************************************************************/
void CICDThread::DetectCircle(const IplImage* srcIplImage, IplImage** getOutputImage){

	CvMemStorage *storage;
	CvSeq		 *circles = 0;
	float* p;

	IplImage *copy_img;
	copy_img = cvCloneImage(srcIplImage);

	IplImage* grayImg;
	grayImg = cvCreateImage(cvSize(srcIplImage->width, srcIplImage->height), IPL_DEPTH_8U, 1);

	// グレースケールに変換
	cvCvtColor(copy_img, grayImg, CV_BGR2GRAY);

	// 2値化する
	cvThreshold(grayImg, grayImg, 150, 255, CV_THRESH_BINARY);
	//cvShowImage("gray", grayImg);

	// 画像の平滑化（ハフ変換のための前処理）
	cvSmooth(grayImg, grayImg, CV_GAUSSIAN, 11, 11, 0, 0);
	storage = cvCreateMemStorage(0);

	// ハフ変換による円の検出と検出した円の描画
	circles = cvHoughCircles(grayImg, storage, CV_HOUGH_GRADIENT,
		1, 5, 20, 50, 0, MAX(grayImg->width, grayImg->height));

	for (int i = 0; i < circles->total; i++) {
		p = (float *)cvGetSeqElem(circles, i);
		cvCircle(copy_img, cvPoint(cvRound(p[0]), cvRound(p[1])), 3, CV_RGB(0, 255, 0), -1, 8, 0);
		cvCircle(copy_img, cvPoint(cvRound(p[0]), cvRound(p[1])), cvRound(p[2]), CV_RGB(255, 0, 0), 3, 8, 0);
	}
	//cvShowImage("Detect", copy_img);
	cvReleaseImage(&grayImg);

	// 結果を返す
	*getOutputImage = copy_img;
	//cvReleaseMemStorage(&storage);

}

/****************************************************************************
Name : DetectSquare()
Desc : 四角形の検出、ポリライン近似使用
****************************************************************************/
void CICDThread::DetectSquare(const IplImage* srcIplImage, IplImage** getOutputImage){
#define VIRTEX_COLOR    CV_RGB( 0, 0, 255)
	CvSeq *contours;
	IplImage *srcImg, *dstImg, *grayImg;
	CvMemStorage *contStorage = cvCreateMemStorage(0);
	CvTreeNodeIterator polyIterator;
	int found = 0;
	int i;
	CvPoint point;
	// ポリライン近似
	CvMemStorage *polyStorage = cvCreateMemStorage(0);
	CvSeq *polys, *poly;

	grayImg = cvCreateImage(cvSize(srcIplImage->width, srcIplImage->height), IPL_DEPTH_8U, 1);
	dstImg = cvCreateImage(cvSize(srcIplImage->width, srcIplImage->height), IPL_DEPTH_8U, 3);

	srcImg = cvCloneImage(srcIplImage);
	// グレイスケールに変換
	cvCvtColor(srcImg, grayImg, CV_RGB2GRAY);
	cvCopy(srcIplImage, dstImg);

	// 2値化する
	cvThreshold(grayImg, grayImg, 150, 255, CV_THRESH_BINARY);
	
	//cvShowImage("gray", grayImg);

	// 画像の平滑化（ハフ変換のための前処理）
	//cvSmooth(tmp, tmp, CV_GAUSSIAN, 11, 11, 0, 0);

	// Contour生成
	// CV_CHAIN_APPRO_NONE  ：輪郭全てを検索する
	// CV_CHAIN_APPRO_SIMPLE：折れ線の頂点のみを検索する
	found = cvFindContours(grayImg, contStorage, &contours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_L1);

	/*
	for (int i = 0; i < found; i++){
		cvDrawContours(dstImg, contours, VIRTEX_COLOR, VIRTEX_COLOR, 0, CV_AA, 8);
	}
	*/
	
	// ポリライン近似
	polys = cvApproxPoly(contours, sizeof(CvContour), polyStorage, CV_POLY_APPROX_DP, 10, 10);

	// イテレータの初期化
	cvInitTreeNodeIterator(&polyIterator, contours, 1000);

	// 画像4端は読み飛ばす
	(CvSeq *)cvNextTreeNode(&polyIterator);

	//ポリライン近似した４点のデータを見て回る
	while ((poly = (CvSeq *)cvNextTreeNode(&polyIterator)) != NULL)
	{
		//４点の領域が閾値以上のとき、その４点を描画
		if ((abs(cvContourArea(poly, CV_WHOLE_SEQ) > 20))){
			for (i = 0; i < poly->total; i++)
			{
				point = *(CvPoint*)cvGetSeqElem(poly, i);
				cvCircle(dstImg, point, 3, VIRTEX_COLOR, -1);
			}
		}
	}
	

	//cvShowImage("Detect", dstImg);

	// 結果を返す
	*getOutputImage = dstImg;
}

