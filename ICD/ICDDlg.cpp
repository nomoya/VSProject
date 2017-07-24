
// ICDDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "ICD.h"
#include "ICDDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CICDDlg ダイアログ

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


// CICDDlg メッセージ ハンドラー

BOOL CICDDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	// TODO: 初期化をここに追加します。

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CICDDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR CICDDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/****************************************************************************
Name : OnCreate()
Desc : メインフレームの構築
****************************************************************************/
int CICDDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	/// ----------------------------------------------------- スレッド生成
	if (!CreateThread()){
		AfxMessageBox(_T("スレッド生成中にエラーが発生しました"));
		return(-1);
	}
	return(0);
}

/****************************************************************************
Name : OnClose()
Desc : ウィンドウを閉じる時に呼ばれる
****************************************************************************/
void CICDDlg::OnClose()
{
	if (IDOK != MessageBox(_T("本当に終了しますか？"), _T("終了確認"), MB_OKCANCEL)){
		return;
	}
	// 終了処理
	Close();
	// 規定の終了処理
	CDialogEx::OnClose();
}



/****************************************************************************
Name : CreateThread()
Desc : MainThreadを構築する
****************************************************************************/
bool CICDDlg::CreateThread()
{
	// MainThread作成
	m_pMainThread = (CMainThread*)AfxBeginThread
		(
		RUNTIME_CLASS(CMainThread),
		THREAD_PRIORITY_NORMAL,		// 優先度
		0,
		CREATE_SUSPENDED			// サスペンド状態で起動
		);
	if (m_pMainThread == NULL){

		return(false);
	}
	m_pMainThread->m_bAutoDelete = FALSE;						// 自動破棄防止
	m_pMainThread->SetSendGui(this, CMainThread::MSG_FROM_GUI);	// メッセージ設定
	m_pMainThread->ResumeThread();								// サスペンド解除
	return(true);
}

/****************************************************************************
Name : Close()
Desc : もろもろの終了処理
****************************************************************************/
void CICDDlg::Close()
{

	DWORD dwExitCode = STILL_ACTIVE;
	// MainThread終了
	if (m_pMainThread != NULL){
		// 終了メッセージを送る
		::PostThreadMessage(m_pMainThread->m_nThreadID, WM_QUIT, 0, 0);
		// タスクが終了するまで待つ
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
Desc : MainThreadにメッセージを送信
****************************************************************************/
void CICDDlg::SendMainThreadMess(WPARAM wParam, LPARAM lParam)
{
	if (m_pMainThread != NULL){
		::PostThreadMessage(m_pMainThread->m_nThreadID, CMainThread::MSG_FROM_GUI, wParam, lParam);
	}
}

/****************************************************************************
Name : OnMainThreadMessage()
Desc : メインスレッドからのメッセージハンドラ　
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
Desc : ICD Modeボタン押下
****************************************************************************/
void CICDDlg::OnBnClickedIcdmode()
{
	CreateShotModeDialog();
}


/****************************************************************************
Name : CreateShotModeDialog()
Desc : ShotModeダイアログを構築
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
Desc : ShotModeダイアログを破棄
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
Desc : IPLイメージをビットマップに変換　
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


	//カラーバッファ内容をBitmapに展開
	BmpMapData->CreateBitmap(Width, Height, 1, 32, ColorBuf);
	free(ColorBuf);
}

/****************************************************************************
Name : ResizeGraphView
Desc : picture controlリサイズ　
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



