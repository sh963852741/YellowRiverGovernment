// Project1.cpp : 定义应用程序的入口点。
//
#ifndef gui
#define gui
#include "resource.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>
#include "framework.h"
#include "Uilib.h"
#include <dhnetsdk.h>
#include <avglobal.h>
#include <dhconfigsdk.h>
#include "OpticalFlowInterface.cpp"
#include "CameraPictureGetter.cpp"
#include "CameraController.cpp"


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
	//CWndUI* pUI2;
	CHorizontalLayoutUI* camWnd; //摄像窗口
	//CHorizontalLayoutUI* drawWnd; //检测灵敏度窗口
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
	Mat frame_arr[2];
	boolean isShowing = true;
	CameraPictureGetter pg;

	/*
	172.16.19.213
	admin
	IO0n2a4G
	*/
	/* 摄像机初始化按钮 */
	void camInit() {
		char tempIp[32];
		char tempUserName[64];
		char tempPassword[64];
		sprintf_s(tempIp, "%s", ip->GetText().GetData());
		sprintf_s(tempUserName, "%s", userName->GetText().GetData());
		sprintf_s(tempPassword, "%s", password->GetText().GetData());
		if (!pg.CameraPictureGetterInit(tempIp, tempUserName, tempPassword)) {
			::MessageBox(NULL, _T("初始化相机失败"), _T("提示"), 0);;
		}
		else {
			::MessageBox(NULL, _T("初始化相机成功"), _T("提示"), 0);;
		}


		if_inte_al->SetEnabled(false);
		slider->SetEnabled(true);
		algo_select->SetEnabled(true);
		if_up_warning->SetEnabled(true);
		if_alarm->SetEnabled(true);
		if_show_dyna->SetEnabled(true);
		
		


		//char temp[64];
		//sprintf_s(temp, "\n灵敏度设置为:%d\n", slider->GetValue());
		//pRich->AppendText(temp);

		//if(lstrcmp(algo_select->GetText(), "算法1") == 0)
		//	sprintf_s(temp, "算法选择为:算法1\n");
		//else 
		//	sprintf_s(temp, "算法选择为:算法2\n");
		//pRich->AppendText(temp);
	}

	/* 播放画面按钮 */
	void playVedio() {
		if (!pg.netSKD_inited()) {
			::MessageBox(NULL, _T("请先初始化相机"), _T("提示"), 0);;
			return;
		}
		SetTimer(m_hWnd, 1, 300, NULL);
	}

	/* 设置重设算法区域按钮  */
	void setRegion() {
		if_inte_al->SetEnabled(true);
		if (frame_arr[0].empty()) {
			::MessageBox(NULL, _T("请先播放画面"), _T("提示"), 0);;
			return;
		}
		::MessageBox(NULL, _T("请在选中区域后按任意键关闭两个窗口，不要点右上角关闭按钮"), _T("提示"), 0);;
		isShowing = false;
		makeMask(frame_arr[1]);
		isShowing = true;
	}


	///* 更新draw图像 */
	//void updateDraw(int arr[]) {
	//	imgDraw.create(drawY, drawX, CV_8UC3);
	//	imgDraw.setTo(cv::Scalar(255, 255, 255));
	//	rectangle(imgDraw, cv::Point(20, drawY - arr[0]), cv::Point(40, drawY), cv::Scalar(200, 200, 100), -1);
	//	rectangle(imgDraw, cv::Point(50, drawY - arr[1]), cv::Point(70, drawY), cv::Scalar(200, 200, 100), -1);
	//	rectangle(imgDraw, cv::Point(80, drawY - arr[2]), cv::Point(100, drawY), cv::Scalar(200, 200, 100), -1);
	//	rectangle(imgDraw, cv::Point(110, drawY - arr[3]), cv::Point(130, drawY), cv::Scalar(200, 200, 100), -1);
	//	rectangle(imgDraw, cv::Point(140, drawY - arr[4]), cv::Point(160, drawY), cv::Scalar(200, 200, 100), -1);
	//	imshow("draw", imgDraw);
	//}

	/* 开启智能算法 */
	void click_inte_al() { 

		bool canConfig = if_inte_al->IsSelected();
		ip->SetEnabled(canConfig);
		userName->SetEnabled(canConfig);
		password->SetEnabled(canConfig);
		slider->SetEnabled(canConfig);
		algo_select->SetEnabled(canConfig);
		if_up_warning->SetEnabled(canConfig);
		if_alarm->SetEnabled(canConfig);
		if_show_dyna->SetEnabled(canConfig);
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
			else if (msg.pSender->GetName() == _T("if_inte_al")) {
				click_inte_al();
			}
			
		}
		if (msg.sType != _T("scroll")) {
			pUI->SetPos(pUI->rc);
			//pUI2->SetPos(pUI2->rc);
			imshow("cam", imgCam);
			//imshow("draw", imgDraw);
		}

	}

	void updateFrame() {
		if (!isShowing)
			return;
		pg >> frame_arr;
		if (frame_arr[0].empty()) {
			return;
		}
		if (if_inte_al->IsSelected()) { //开启智能算法
			opticalFlow(frame_arr, if_show_dyna->IsSelected());
		}
		else {
			if (cv::getWindowProperty("识别结果", WND_PROP_VISIBLE) == 1)
				cv::destroyWindow("识别结果");
			if (cv::getWindowProperty("动检结果", WND_PROP_VISIBLE) == 1)
				cv::destroyWindow("动检结果");
		}
		int nCols = 578;
		int nRows = frame_arr[1].rows * nCols / frame_arr[1].cols;
		imgCam = Mat(nRows, nCols, frame_arr[1].type());
		resize(frame_arr[1], imgCam, imgCam.size(), 0, 0, INTER_LINEAR);
		imshow("cam", imgCam);
	}


	virtual LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		//如果是自己定义的计时器id时处理,因为同一个窗口可以定义很多个定时器
		if (wParam == 1) {
			updateFrame();
		}
		//下面设置为false,意思是消息继续传递不拦截,否则程序中有滚动条时因为这里拦截就会出现拖动失效的情况
		bHandled = false;
		return 0;
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
			//drawWnd = static_cast<CHorizontalLayoutUI*>(m_pm.FindControl(_T("draw")));
			slider = static_cast<CSliderUI*>(m_pm.FindControl(_T("spec_controlor"))); 
			algo_select = static_cast<CComboUI*>(m_pm.FindControl(_T("algo_select")));
			if_inte_al = static_cast<CCheckBoxUI*>(m_pm.FindControl(_T("if_inte_al")));
			if_up_warning = static_cast<CCheckBoxUI*>(m_pm.FindControl(_T("if_up_warning")));
			if_alarm = static_cast<CCheckBoxUI*>(m_pm.FindControl(_T("if_alarm")));
			if_show_dyna = static_cast<CCheckBoxUI*>(m_pm.FindControl(_T("if_show_dyna")));
			slider->SetEnabled(false);
			algo_select->SetEnabled(false);
			if_up_warning->SetEnabled(false);
			if_alarm->SetEnabled(false);
			if_show_dyna->SetEnabled(false);
			if_inte_al->SetEnabled(false);
			pUI = new CWndUI(10,10,camX,camY);
			camWnd->RemoveAll();
			camWnd->Add(pUI);
			hWndVedio = CreateWindow(_T("STATIC"), _T("摄像机画面"), WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 10, 10, camX, camY,
				m_pm.GetPaintWindow(), (HMENU)0, NULL, NULL);
			pUI->Attach(hWndVedio);
			pUI->SetEnabled(false);
			
			
			//pUI2 = new CWndUI(310,401,drawX,drawY);
			//drawWnd->RemoveAll();
			//drawWnd->Add(pUI2);
			//hWndDraw = CreateWindow(_T("STATIC"), _T("柱状图"), WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 310, 401, drawX, drawY,
			//	m_pm.GetPaintWindow(), (HMENU)0, NULL, NULL);
			//pUI2->Attach(hWndDraw);
			//pUI2->SetEnabled(false);

			cv::namedWindow("cam", CV_WINDOW_AUTOSIZE);
			HWND hWnd = (HWND)cvGetWindowHandle("cam");
			HWND hParent = ::GetParent(hWnd);
			::SetParent(hWnd, hWndVedio);
			::ShowWindow(hParent, SW_HIDE);

			//cv::namedWindow("draw", CV_WINDOW_AUTOSIZE);
			//hWnd = (HWND)cvGetWindowHandle("draw");
			//hParent = ::GetParent(hWnd);
			//::SetParent(hWnd, hWndDraw);
			//::ShowWindow(hParent, SW_HIDE);

			//imgDraw.create(drawY, drawX, CV_8UC3);
			imgCam.create(camY, camX, CV_8UC3);
			//imgDraw.setTo(cv::Scalar(255, 255, 255));
			imgCam.setTo(cv::Scalar(255,255, 255));

			//updateDraw(new int[]{0,20,80,40,100});
			
				
			ip->SetText("172.16.19.213");
			userName->SetText("admin");
			password->SetText("IO0n2a4G");
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
		BOOL bHandled = false;
		if (uMsg == WM_TIMER) {
			lRes = OnTimer(uMsg, wParam, lParam, bHandled);
		}
		if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;

		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}


};


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




#endif