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
	HWND hWndVedio = nullptr;
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
	CHorizontalLayoutUI* camWnd; //摄像窗口
	CHorizontalLayoutUI* drawWnd; //检测灵敏度窗口
	CRichEditUI* pRich; //报警信息 pRich->AppendText()
	CEditUI* ip;  //ip ip->GetText()
	CEditUI* userName;  //用户名 用法同上
	CEditUI* password; //密码 用法同上
	CSliderUI* slider; //灵敏度滑条 slider->GetValue()
	CComboUI* algo_select; //算法选择 algo_select->GetText(),用lstrcpm比较,如if(lstrcmp(algo_select->GetText(), "算法1") == 0)
	CCheckBoxUI* if_inte_al; //开启智能算法 if_inte_al->IsSelected()，若要禁止修改则if_inte_al->SetEnabled(false);
	CCheckBoxUI* if_up_warning; //上传报警信息 用法同上
	CCheckBoxUI* if_alarm; //开启算法报警 用法同上
	CCheckBoxUI* if_show_dyna;//显示动检结果 用法同上



	/* 摄像机初始化按钮 */
	void camInit() {
		//以下为测试组件代码，请修改为函数逻辑
		
		pRich->AppendText("初始化按钮点击\n");
		pRich->AppendText(ip->GetText());
		pRich->AppendText(userName->GetText());
		pRich->AppendText(password->GetText());

		char temp[64];
		sprintf_s(temp, "\n灵敏度设置为:%d\n", slider->GetValue());
		pRich->AppendText(temp);

		if(lstrcmp(algo_select->GetText(), "算法1") == 0)
			sprintf_s(temp, "算法选择为:算法1\n");
		else 
			sprintf_s(temp, "算法选择为:算法2\n");
		pRich->AppendText(temp);
		
		if (if_inte_al->IsSelected()) {
			pRich->AppendText("开启智能算法被选中\n");
		}
		if (if_up_warning->IsSelected()) {
			pRich->AppendText("上传报警信息被选中\n");
		}
		if (if_alarm->IsSelected()) {
			pRich->AppendText("开启算法报警被选中\n");
		}
		if (if_show_dyna->IsSelected()) {
			pRich->AppendText("显示动检结果被选中\n");
		}
		if_inte_al->SetEnabled(false);
		
		
	}

	/* 播放画面按钮 */
	void playVedio() {
		
		pRich->AppendText("播放画面按钮被点击\n");
		
		/* 请在这段函数下方生成要播放的img并imshow("cam",ifmg); */
		imgCam = cv::imread("1.png", 1);
		imshow("cam",imgCam);
		
		
	}
	/* 设置重设算法区域按钮  */
	void setRegion() {
		pRich->AppendText("设置、重设算法识别区域按钮被点击\n");
	}


	/* 更新draw图像 */
	void updateDraw(int arr[]) {
		imgDraw.create(drawY, drawX, CV_8UC3);
		imgDraw.setTo(cv::Scalar(255, 255, 255));
		rectangle(imgDraw, cv::Point(20, 100 - arr[0]), cv::Point(40, drawY), cv::Scalar(200, 200, 100), -1);
		rectangle(imgDraw, cv::Point(50, 100 - arr[1]), cv::Point(70, drawY), cv::Scalar(200, 200, 100), -1);
		rectangle(imgDraw, cv::Point(80, 100 - arr[2]), cv::Point(100, drawY), cv::Scalar(200, 200, 100), -1);
		rectangle(imgDraw, cv::Point(110, 100 - arr[3]), cv::Point(130, drawY), cv::Scalar(200, 200, 100), -1);
		rectangle(imgDraw, cv::Point(140, 100 - arr[4]), cv::Point(160, drawY), cv::Scalar(200, 200, 100), -1);
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
			else if (msg.pSender->GetName() == _T("set_reset_region")) {
				setRegion();
			}
			
		}
		if (msg.sType != _T("scroll")) {
			pUI->SetPos(pUI->rc);
			pUI2->SetPos(pUI2->rc);
			imshow("cam", imgCam);
			imshow("draw", imgDraw);
		}
		

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
			slider = static_cast<CSliderUI*>(m_pm.FindControl(_T("spec_controlor"))); 
			algo_select = static_cast<CComboUI*>(m_pm.FindControl(_T("algo_select")));
			if_inte_al = static_cast<CCheckBoxUI*>(m_pm.FindControl(_T("if_inte_al")));
			if_up_warning = static_cast<CCheckBoxUI*>(m_pm.FindControl(_T("if_up_warning")));
			if_alarm = static_cast<CCheckBoxUI*>(m_pm.FindControl(_T("if_alarm")));
			if_show_dyna = static_cast<CCheckBoxUI*>(m_pm.FindControl(_T("if_show_dyna")));

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

			updateDraw(new int[]{60,20,80,40,20});
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


