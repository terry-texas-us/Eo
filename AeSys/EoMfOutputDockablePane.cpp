#include "stdafx.h"

#include "AeSysApp.h"

#include "EoMfOutputDockablePane.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// COutputBar

EoMfOutputDockablePane::EoMfOutputDockablePane() {
}
EoMfOutputDockablePane::~EoMfOutputDockablePane() {
}
BEGIN_MESSAGE_MAP(EoMfOutputDockablePane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int EoMfOutputDockablePane::OnCreate(LPCREATESTRUCT createStructure) {
	if (CDockablePane::OnCreate(createStructure) == - 1) {
		return - 1;
	}
	m_Font.CreateStockObject(DEFAULT_GUI_FONT);

	CRect EmptyRect;
	EmptyRect.SetRectEmpty();

	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_FLAT, EmptyRect, this, 1, CMFCTabCtrl::LOCATION_BOTTOM)) {
		ATLTRACE2(atlTraceGeneral, 0, L"Failed to create output tab window\n");
		return - 1;
	}
	const DWORD SharedStyles = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LBS_NOINTEGRALHEIGHT;

	if (!m_OutputMessagesList.Create(SharedStyles, EmptyRect, &m_wndTabs, 2) || !m_OutputReportsList.Create(SharedStyles, EmptyRect, &m_wndTabs, 4)) {
		ATLTRACE2(atlTraceGeneral, 0, L"Failed to create output windows\n");
		return - 1;
	}
	m_OutputMessagesList.SetFont(&m_Font);
	m_OutputReportsList.SetFont(&m_Font);

	// Attach list windows to tab:
	CString TabLabel = theApp.LoadStringResource(IDS_OUTPUT_MESSAGES);
	m_wndTabs.AddTab(&m_OutputMessagesList, TabLabel);
	TabLabel = theApp.LoadStringResource(IDS_OUTPUT_REPORTS);
	m_wndTabs.AddTab(&m_OutputReportsList, TabLabel);

	// Dummy data
	m_OutputMessagesList.AddString(L"Message output is being displayed here.");
	m_OutputReportsList.AddString(L"Reports output is being displayed here.");

	return 0;
}
void EoMfOutputDockablePane::OnSize(UINT type, int cx, int cy) {
	CDockablePane::OnSize(type, cx, cy);

	// Tab control should cover the whole client area:
	m_wndTabs.SetWindowPos(NULL, - 1, - 1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}
EoMfOutputListBox::EoMfOutputListBox() {
}
EoMfOutputListBox::~EoMfOutputListBox() {
}
BEGIN_MESSAGE_MAP(EoMfOutputListBox, CListBox)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()

// EoMfOutputListBox message handlers

void EoMfOutputListBox::OnContextMenu(CWnd* /* window */, CPoint point) {
	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx))) {
		CMenu Menu;
		Menu.LoadMenu(IDR_OUTPUT_POPUP);

		CMenu* SubMenu = Menu.GetSubMenu(0);
		CMFCPopupMenu* PopupMenu = new CMFCPopupMenu;

		if (!PopupMenu->Create(this, point.x, point.y, SubMenu->GetSafeHmenu(), FALSE, TRUE)) {
			return;
		}
		((CMDIFrameWndEx*) AfxGetMainWnd())->OnShowPopupMenu(PopupMenu);
		UpdateDialogControls(this, FALSE);
	}
	SetFocus();
}
void EoMfOutputListBox::OnEditCopy() noexcept {
	::MessageBoxW(0, L"Copy output", L"Testing", 0);
}

void EoMfOutputListBox::OnEditClear() noexcept {
	::MessageBoxW(0, L"Clear output", L"Testing", 0);
}

void EoMfOutputListBox::OnViewOutput() {
	CDockablePane* pParentBar = DYNAMIC_DOWNCAST(CDockablePane, GetOwner());
	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetTopLevelFrame());

	if (pMainFrame != NULL && pParentBar != NULL) {
		pMainFrame->SetFocus();
		pMainFrame->ShowPane(pParentBar, FALSE, FALSE, FALSE);
		pMainFrame->RecalcLayout();

	}
}
