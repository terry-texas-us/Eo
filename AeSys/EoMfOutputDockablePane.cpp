#include "stdafx.h"

#include "AeSys.h"

#include "EoMfOutputDockablePane.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// COutputBar

BEGIN_MESSAGE_MAP(EoMfOutputDockablePane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int EoMfOutputDockablePane::OnCreate(LPCREATESTRUCT createStructure) {
	
	if (CDockablePane::OnCreate(createStructure) == - 1) { return - 1; }

	m_Font.CreateStockObject(DEFAULT_GUI_FONT);

	CRect EmptyRect;
	EmptyRect.SetRectEmpty();

	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_FLAT, EmptyRect, this, 1, CMFCTabCtrl::LOCATION_BOTTOM)) {
		TRACE0("Failed to create output tab window\n");
		return - 1;
	}
	const unsigned long SharedStyles {WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LBS_NOINTEGRALHEIGHT};

	if (!m_OutputMessagesList.Create(SharedStyles, EmptyRect, &m_wndTabs, 2) || !m_OutputReportsList.Create(SharedStyles, EmptyRect, &m_wndTabs, 4)) {
		TRACE0("Failed to create output windows\n");
		return - 1;
	}
	m_OutputMessagesList.SetFont(&m_Font);
	m_OutputReportsList.SetFont(&m_Font);

	// Attach list windows to tab:
	auto TabLabel {theApp.LoadStringResource(IDS_OUTPUT_MESSAGES)};
	m_wndTabs.AddTab(&m_OutputMessagesList, TabLabel);
	TabLabel = theApp.LoadStringResource(IDS_OUTPUT_REPORTS);
	m_wndTabs.AddTab(&m_OutputReportsList, TabLabel);

	// Dummy data
	m_OutputMessagesList.AddString(L"Message output is being displayed here.");
	m_OutputReportsList.AddString(L"Reports output is being displayed here.");

	return 0;
}

void EoMfOutputDockablePane::OnSize(unsigned type, int cx, int cy) {
	CDockablePane::OnSize(type, cx, cy);

	// Tab control should cover the whole client area:
	m_wndTabs.SetWindowPos(nullptr, - 1, - 1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
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
		Menu.LoadMenuW(IDR_OUTPUT_POPUP);

		auto SubMenu {Menu.GetSubMenu(0)};
		auto PopupMenu {new CMFCPopupMenu};

		if (!PopupMenu->Create(this, point.x, point.y, SubMenu->GetSafeHmenu(), FALSE, TRUE)) { return; }

		static_cast<CMDIFrameWndEx*>(AfxGetMainWnd())->OnShowPopupMenu(PopupMenu);
		UpdateDialogControls(this, FALSE);
	}
	SetFocus();
}
void EoMfOutputListBox::OnEditCopy() noexcept {
	::MessageBoxW(nullptr, L"Copy output", L"Testing", 0);
}

void EoMfOutputListBox::OnEditClear() noexcept {
	::MessageBoxW(nullptr, L"Clear output", L"Testing", 0);
}

void EoMfOutputListBox::OnViewOutput() {
	auto ParentBar {DYNAMIC_DOWNCAST(CDockablePane, GetOwner())};
	auto MainFrame {DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetTopLevelFrame())};

	if (MainFrame != nullptr && ParentBar != nullptr) {
		MainFrame->SetFocus();
		MainFrame->ShowPane(ParentBar, FALSE, FALSE, FALSE);
		MainFrame->RecalcLayout();

	}
}
