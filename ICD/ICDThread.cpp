#include "stdafx.h"
#include "stdlib.h"
#include "ICDThread.h"
#include "MainThread.h"
#include "baseapi.h"
#include "strngs.h"
#include "resultiterator.h"


IMPLEMENT_DYNCREATE(CICDThread, CBaseTask)
// ���b�Z�[�W
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
Desc : �J�n���ɌĂ΂��@
****************************************************************************/
BOOL CICDThread::InitInstance()
{
	

	//m_cStatus.bBusy = true;
	// ������
	if (!Init())
	{
		// ���s
		//m_cStatus.bBusy = false;
		//m_cStatus.strErr = _T("�������G���[");
		//ChangeStatus();
		//NotifyTask(NOTIFY_ERR, NULL);
		return(TRUE);	// FALSE��Ԃ��ƏI�����Ă��܂��̂�TRUE��Ԃ�
	}

	// ����
	//m_cStatus.bBusy = false;
	//ChangeStatus();
	//NotifyTask(NOTIFY_INIT_OK, NULL);
	//LOG(_T("���������� ======================================"));
	return(TRUE);
}

/****************************************************************************
Name : Init()
Desc : �X���b�h��������
****************************************************************************/
bool CICDThread::Init()
{

	// �N���e�B�J���Z�N�V����������
	InitializeCriticalSection(&m_CriticalSection);
	return(true);
}

/****************************************************************************
Name : ExitInstance()
Desc : �I�����ɌĂ΂��@
****************************************************************************/
int CICDThread::ExitInstance()
{
	// �N���e�B�J���Z�N�V�����̔j��
	DeleteCriticalSection(&m_CriticalSection);

	return(CBaseTask::ExitInstance());
}

/****************************************************************************
Name : GetImage()
Desc : ���̃X���b�h�����̊֐��������ĉ摜���擾����@
****************************************************************************/
void CICDThread::GetImage(IplImage** image)
{
	EnterCriticalSection(&m_CriticalSection);
	{
		*image = cvCloneImage(m_iCaptureImage);
	}
	LeaveCriticalSection(&m_CriticalSection);
}

/****************************************************************************
Name : OnMainThreadMessage()
Desc : Main�X���b�h����̃��b�Z�[�W�n���h���@
****************************************************************************/
void CICDThread::OnMainThreadMessage(WPARAM wParam, LPARAM lParam)
{
	CString strLog;
	CImage img;

	switch (wParam)
	{
	case CMainThread::WPRM_CAPTURE:
		// �L���v�`��
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
			DetectEngText(m_iCaptureImage, &m_iDetectImage);
			break;
		}
		
		NotifyTask(NOTIFY_DETECT_END, NULL);
		break;

	}

}


/****************************************************************************
Name : Capture()
Desc : �ڑ����Ă���E�F�u�J�����ŃL���v�`�����s��
****************************************************************************/
void CICDThread::Capture()
{

	// �J���������荞��
	CvCapture *capture = cvCreateCameraCapture(0);

	//cvNamedWindow("debug", CV_WINDOW_AUTOSIZE);

	// �J������������Ȃ�
	if (capture == NULL)
	{
		NotifyTask(NOTIFY_CAPTURE_ERR, NULL);
		return;
	}
	//�J�����̃T�C�Y����荞�݂܂��B cvGetCaptureProperty�̖߂�l��double�Ȃ̂ŁAint�ɃL���X�g
	CvSize size = cvSize
		(
		(int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH),
		(int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT)
		);

	CvSize normalsize = cvSize((int)size.width, (int)size.height);
	CvSize resizesize = cvSize((int)size.width, (int)size.height);

	m_iCaptureImage = cvCreateImage(normalsize, IPL_DEPTH_8U, 3);

	IplImage* CaptureFrame = cvCreateImage(normalsize, IPL_DEPTH_8U, 3);

	// 1�V���b�g�������ƃ��\�[�X������ɃG���[? �Ƃ肠����10�V���b�g
	int count = 0;
	while (count < 10){
		try
		{
			CaptureFrame = cvQueryFrame(capture);
		}
		catch (std::exception ex)
		{
			// �L���v�`�����s
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
		//���\�[�X���J��
		cvReleaseCapture(&capture);
	}
	catch (std::exception ex)
	{
	
	}

}

/****************************************************************************
Name : IplToBmp()
Desc : IPL�C���[�W���r�b�g�}�b�v�ɕϊ��@
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

	//�J���[�o�b�t�@���e��Bitmap�ɓW�J
	BmpMapData->CreateBitmap(Width, Height, 1, 32, ColorBuf);
	free(ColorBuf);
}


/****************************************************************************
Name : DetectCircle()
Desc : �~�̌��o
****************************************************************************/
void CICDThread::DetectCircle(const IplImage* srcIplImage, IplImage** getOutputImage){

	CvMemStorage *storage;
	CvSeq		 *circles = 0;
	float* p;

	IplImage *copy_img;
	copy_img = cvCloneImage(srcIplImage);

	IplImage* grayImg;
	grayImg = cvCreateImage(cvSize(srcIplImage->width, srcIplImage->height), IPL_DEPTH_8U, 1);

	// �O���[�X�P�[���ɕϊ�
	cvCvtColor(copy_img, grayImg, CV_BGR2GRAY);

	// 2�l������
	cvThreshold(grayImg, grayImg, 150, 255, CV_THRESH_BINARY);
	cvShowImage("gray", grayImg);

	// �摜�̕������i�n�t�ϊ��̂��߂̑O�����j
	cvSmooth(grayImg, grayImg, CV_GAUSSIAN, 11, 11, 0, 0);
	storage = cvCreateMemStorage(0);

	// �n�t�ϊ��ɂ��~�̌��o�ƌ��o�����~�̕`��
	circles = cvHoughCircles(grayImg, storage, CV_HOUGH_GRADIENT,
		1, 5, 20, 50, 0, MAX(grayImg->width, grayImg->height));

	for (int i = 0; i < circles->total; i++) {
		p = (float *)cvGetSeqElem(circles, i);
		cvCircle(copy_img, cvPoint(cvRound(p[0]), cvRound(p[1])), 3, CV_RGB(0, 255, 0), -1, 8, 0);
		cvCircle(copy_img, cvPoint(cvRound(p[0]), cvRound(p[1])), cvRound(p[2]), CV_RGB(255, 0, 0), 3, 8, 0);
	}
	cvShowImage("Detect", copy_img);
	cvReleaseImage(&grayImg);

	// ���ʂ�Ԃ�
	*getOutputImage = grayImg;
	//cvReleaseMemStorage(&storage);

}

/****************************************************************************
Name : DetectSquare()
Desc : �l�p�`�̌��o�A�|�����C���ߎ��g�p
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
	// �|�����C���ߎ�
	CvMemStorage *polyStorage = cvCreateMemStorage(0);
	CvSeq *polys, *poly;

	grayImg = cvCreateImage(cvSize(srcIplImage->width, srcIplImage->height), IPL_DEPTH_8U, 1);
	dstImg = cvCreateImage(cvSize(srcIplImage->width, srcIplImage->height), IPL_DEPTH_8U, 3);

	srcImg = cvCloneImage(srcIplImage);
	// �O���C�X�P�[���ɕϊ�
	cvCvtColor(srcImg, grayImg, CV_RGB2GRAY);
	cvCopy(srcIplImage, dstImg);

	// 2�l������
	cvThreshold(grayImg, grayImg, 150, 255, CV_THRESH_BINARY);
	
	cvShowImage("gray", grayImg);

	// �摜�̕������i�n�t�ϊ��̂��߂̑O�����j
	//cvSmooth(tmp, tmp, CV_GAUSSIAN, 11, 11, 0, 0);

	// Contour����
	// CV_CHAIN_APPRO_NONE  �F�֊s�S�Ă���������
	// CV_CHAIN_APPRO_SIMPLE�F�܂���̒��_�݂̂���������
	found = cvFindContours(grayImg, contStorage, &contours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_L1);

	/*
	for (int i = 0; i < found; i++){
		cvDrawContours(dstImg, contours, VIRTEX_COLOR, VIRTEX_COLOR, 0, CV_AA, 8);
	}
	*/
	
	// �|�����C���ߎ�
	polys = cvApproxPoly(contours, sizeof(CvContour), polyStorage, CV_POLY_APPROX_DP, 10, 10);

	// �C�e���[�^�̏�����
	cvInitTreeNodeIterator(&polyIterator, contours, 1000);

	// �摜4�[�͓ǂݔ�΂�
	(CvSeq *)cvNextTreeNode(&polyIterator);

	//�|�����C���ߎ������S�_�̃f�[�^�����ĉ��
	while ((poly = (CvSeq *)cvNextTreeNode(&polyIterator)) != NULL)
	{
		//�S�_�̗̈悪臒l�ȏ�̂Ƃ��A���̂S�_��`��
		if ((abs(cvContourArea(poly, CV_WHOLE_SEQ) > 20))){
			for (i = 0; i < poly->total; i++)
			{
				point = *(CvPoint*)cvGetSeqElem(poly, i);
				cvCircle(dstImg, point, 3, VIRTEX_COLOR, -1);
			}
		}
	}
	

	cvShowImage("Detect", dstImg);

	// ���ʂ�Ԃ�
	*getOutputImage = dstImg;
}


/****************************************************************************
Name : DetectEngText()
Desc : �p���e�L�X�g�̌��o
�Q�l:http://pg.blog.jp/archives/2825800.html
****************************************************************************/
void CICDThread::DetectEngText(const IplImage* srcIplImage, IplImage** getOutputImage){

	STRING output;
	// �o�͐��tmp�摜
	IplImage *dst = cvCloneImage(srcIplImage);

	// �t�H���g�ݒ�
	CvFont font;
	CvSize text_size;
	CString strMess;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1, 1, 0, 1, 4);

	// API�̏����Ə�����
	tesseract::TessBaseAPI tessBaseApi;
	int nRet = tessBaseApi.Init(
		"C:/Users/t/Documents/Visual Studio 2013/Projects/ImageProcessing/Release/",// �f�[�^�p�X (tessdata �̐e�f�B���N�g����) tessdata/�͂���Ȃ� 
		"eng");             // ���� "eng", "jpn" �Ȃ�

	// �F�����镶�����i��i�z���C�g���X�g�j
	tessBaseApi.SetVariable("tessedit_char_whitelist", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

	// bmp�ۑ�
	CImage img;
	IplToBmp(srcIplImage, &m_cBitmap);
	img.Attach(m_cBitmap);
	img.Save(_T("bmptest.bmp"));

	// �����̔F��
	bool success = tessBaseApi.ProcessPages(
		"C:\\Users\\t\\Documents\\Visual Studio 2013\\Projects\\ICD\\Release\\bmptest.bmp",// ���̓t�@�C����
		NULL,					// ���g���Cconfig�t�@�C����
		0,						// �^�C���A�E�g�l(msec) (0 �͎w��Ȃ�)
		&output);				// �o�͕�����


	
	tesseract::ResultIterator* ri = tessBaseApi.GetIterator();
	tesseract::PageIteratorLevel level = tesseract::RIL_WORD;

	
	if (ri != 0) {
		do {
			const char* word = ri->GetUTF8Text(level);
			if (word == NULL || strlen(word) == 0) {
				continue;
			}

			int x1, y1, x2, y2;
			ri->BoundingBox(level, &x1, &y1, &x2, &y2);
			float conf = ri->Confidence(level);
			
			//std::string text = UTF8toSJIS(word);
			char cTemp[256];
			sprintf_s(cTemp, "(%d, %d)-(%d, %d) : %.1f%% : %c \n", x1, y1, x2, y2, conf, word);
			strMess += cTemp;

		} while (ri->Next(level));
	}

	//char *text = new char[strMess.GetLength() + 1];
	//strcpy(text, strMess);
	
	m_strDetectText = strMess;

	/*
	LPCSTR lpString = output.string();
	sprintf_s(text, "[%s]", lpString);
	*/

	// �ǂݎ������������摜�ɒ���t����
	//cvGetTextSize(text, &font, &text_size, 0);
	//cvPutText(dst, text, cvPoint(0, text_size.height), &font, CV_RGB(255, 0, 0));

	cvShowImage("Detect", dst);

	// ���ʂ�Ԃ�
	*getOutputImage = dst;

}

std::string UTF8toSJIS(const char* src) {

	// UTF8 -> UTF16
	int lenghtUnicode = MultiByteToWideChar(CP_UTF8, 0, src, strlen(src) + 1, NULL, 0);
	wchar_t* bufUnicode = new wchar_t[lenghtUnicode];
	MultiByteToWideChar(CP_UTF8, 0, src, strlen(src) + 1, bufUnicode, lenghtUnicode);

	// UTF16 -> ShiftJis
	int lengthSJis = WideCharToMultiByte(CP_THREAD_ACP, 0, bufUnicode, -1, NULL, 0, NULL, NULL);
	char* bufShiftJis = new char[lengthSJis];
	WideCharToMultiByte(CP_THREAD_ACP, 0, bufUnicode, lenghtUnicode + 1, bufShiftJis, lengthSJis, NULL, NULL);

	std::string strSJis(bufShiftJis);

	delete bufUnicode;
	delete bufShiftJis;

	return strSJis;
}

