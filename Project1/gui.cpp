// Project1.cpp : 定义应用程序的入口点。
//

#include "gui.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>
#include "framework.h"
#include "Uilib.h"


using namespace DuiLib;


class CWndUI : public CControlUI
{
public:
	RECT rc;

	CWndUI(int x, int y, int w, int h) : m_hWnd(NULL) 
	{
		rc.left = x;
		rc.top = y;
		rc.right = w + x;
		rc.bottom = y + h;
	}

	virtual void SetInternVisible(bool bVisible = true)
	{
		__super::SetInternVisible(bVisible);
		::ShowWindow(m_hWnd, bVisible);
	}

	virtual void SetPos(RECT rc)
	{
		this->rc = rc;
		__super::SetPos(rc);
		::SetWindowPos(m_hWnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	BOOL Attach(HWND hWndNew)
	{
		if (!::IsWindow(hWndNew))
		{
			return FALSE;
		}

		m_hWnd = hWndNew;
		return TRUE;
	}

	HWND Detach()
	{
		HWND hWnd = m_hWnd;
		m_hWnd = NULL;
		return hWnd;
	}

	HWND GetHwnd()
	{
		HWND hWnd = m_hWnd;
		return hWnd;
	}

protected:
	HWND m_hWnd;
};



class CFrameWindowWnd : public CWindowWnd, public INotifyUI
{
public:
	CFrameWindowWnd() { };
	LPCTSTR GetWindowClassName() const { return _T("UIMainFrame"); };
	UINT GetClassStyle() const { return CS_DBLCLKS; }; // UI_CLASSSTYLE_FRAME |
	void OnFinalMessage(HWND /*hWnd*/) { delete this; };

	CPaintManagerUI m_pm;
	CRichEditUI* pRich;
	CEditUI* ip;
	CEditUI* userName;
	CEditUI* password;
	CHorizontalLayoutUI* camWnd;
	HWND hWndVedio = nullptr;
	CHorizontalLayoutUI* drawWnd;
	HWND hWndDraw = nullptr;
	bool flag = false;
	int camX = 790;
	int camY = 332;
	int drawX = 180;
	int drawY = 120;
	cv::Mat imgCam;
	cv::Mat imgDraw;
	CWndUI* pUI;
	CWndUI* pUI2;
	

	/* 摄像机初始化按钮 */
	void camInit() {
		//以下为测试组件代码，请修改为函数逻辑
		updateDraw();
		pRich->AppendText("初始化按钮被点击\n");
		
		
	}

	/* 播放画面按钮 */
	void playVedio() {
		
		pRich->AppendText("播放画面按钮被点击\n");
		
		/* 请在这段函数下方生成要播放的img并imshow("cam",img); */
		imgCam = cv::imread("1.png", 1);
		//imgCam.rows;
		//cv::cv2CopyMakeBorder(imgCam, imgCam, 0 ,0, 0, 0,CV_HAL_BORDER_CONSTANT, value=[0,0,0]);
		imshow("cam",imgCam);
	}

	void updateDraw() {
		imgDraw.create(drawY, drawX, CV_8UC3);
		imgDraw.setTo(cv::Scalar(255, 255, 255));
		rectangle(imgDraw, cv::Point(20, 50), cv::Point(40, drawY), cv::Scalar(200, 200, 100), -1);
		rectangle(imgDraw, cv::Point(50, 100), cv::Point(70, drawY), cv::Scalar(200, 200, 100), -1);
		rectangle(imgDraw, cv::Point(80, 100), cv::Point(100, drawY), cv::Scalar(200, 200, 100), -1);
		rectangle(imgDraw, cv::Point(110, 100), cv::Point(130, drawY), cv::Scalar(200, 200, 100), -1);
		rectangle(imgDraw, cv::Point(140, 100), cv::Point(160, drawY), cv::Scalar(200, 200, 100), -1);
		imshow("draw", imgDraw);
	}

	void Notify(TNotifyUI& msg)
	{

		if (msg.sType == _T("click")) {
			//pRich->AppendText(msg.pSender->GetClass());
			if (msg.pSender->GetName() == _T("camInitBtn")) {
				camInit();
			}
			else if (msg.pSender->GetName() == _T("playBtn")) {
				playVedio();
			}
		}
		pUI->SetPos(pUI->rc);
		pUI2->SetPos(pUI2->rc);
		imshow("cam", imgCam);
		imshow("draw", imgDraw);

	}
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_CREATE) {
			m_pm.Init(m_hWnd);
			CDialogBuilder builder;
			CControlUI* pRoot = builder.Create(_T("theme/main_wnd_frame.xml"), (LPCTSTR)0, NULL, &m_pm);
			ASSERT(pRoot && "Failed to parse XML");
			m_pm.AttachDialog(pRoot);
			m_pm.AddNotifier(this);
			pRich = static_cast<CRichEditUI*>(m_pm.FindControl(_T("richEdit")));
			ip = static_cast<CEditUI*>(m_pm.FindControl(_T("ip")));
			userName = static_cast<CEditUI*>(m_pm.FindControl(_T("userName")));
			password = static_cast<CEditUI*>(m_pm.FindControl(_T("password")));
			camWnd = static_cast<CHorizontalLayoutUI*>(m_pm.FindControl(_T("camMedia")));
			drawWnd = static_cast<CHorizontalLayoutUI*>(m_pm.FindControl(_T("draw")));

			pUI = new CWndUI(10,10,camX,camY);
			camWnd->RemoveAll();
			camWnd->Add(pUI);
			hWndVedio = CreateWindow(_T("STATIC"), _T("摄像机画面"), WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 10, 10, camX, camY,
				m_pm.GetPaintWindow(), (HMENU)0, NULL, NULL);
			pUI->Attach(hWndVedio);
			pUI->SetEnabled(false);
			
			
			pUI2 = new CWndUI(310,401,drawX,drawY);
			drawWnd->RemoveAll();
			drawWnd->Add(pUI2);
			hWndDraw = CreateWindow(_T("STATIC"), _T("柱状图"), WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 310, 401, drawX, drawY,
				m_pm.GetPaintWindow(), (HMENU)0, NULL, NULL);
			pUI2->Attach(hWndDraw);
			pUI2->SetEnabled(false);

			cv::namedWindow("cam", CV_WINDOW_AUTOSIZE);
			HWND hWnd = (HWND)cvGetWindowHandle("cam");
			HWND hParent = ::GetParent(hWnd);
			::SetParent(hWnd, hWndVedio);
			::ShowWindow(hParent, SW_HIDE);

			cv::namedWindow("draw", CV_WINDOW_AUTOSIZE);
			hWnd = (HWND)cvGetWindowHandle("draw");
			hParent = ::GetParent(hWnd);
			::SetParent(hWnd, hWndDraw);
			::ShowWindow(hParent, SW_HIDE);

			imgDraw.create(drawY, drawX, CV_8UC3);
			imgCam.create(camY, camX, CV_8UC3);
			imgDraw.setTo(cv::Scalar(255, 255, 255));
			imgCam.setTo(cv::Scalar(255,255, 255));
			imshow("draw", imgDraw);
			imshow("cam", imgCam);

			return 0;
		}
		else if (uMsg == WM_DESTROY) {
			cv::destroyAllWindows();
			::PostQuitMessage(0);
		}
		else if (uMsg == WM_SIZE) {
			return 0;
		}
		LRESULT lRes = 0;
		if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;

		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}


};



// 程序入口及 Duilib 初始化部分
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR
	/*lpCmdLine*/, int nCmdShow)
{
	//::MessageBox(NULL, _T("110"), _T("提示2"), 0);;
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());
	CFrameWindowWnd* pFrame = new CFrameWindowWnd();
	if (pFrame == NULL) return 0;
	pFrame->Create(NULL, _T("YREC边坡踏岸识别"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	pFrame->ShowWindow(true);
	CPaintManagerUI::MessageLoop();	
	

	return 0;
}


