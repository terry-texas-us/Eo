#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include <DbObjectContextCollection.h>
#include <DbObjectContextManager.h>
#include "EoCtrlFindComboBox.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CMainFrame
IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

const int gc_MaximumUserToolbars = 10;
const unsigned gc_FirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const unsigned gc_LastUserToolBarId = gc_FirstUserToolBarId + gc_MaximumUserToolbars - 1;
BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
		ON_WM_CREATE()
		ON_WM_DESTROY()
		ON_WM_TIMER()
		ON_WM_MDIACTIVATE()
		ON_COMMAND(ID_HELP_FINDER, &CMDIFrameWndEx::OnHelpFinder)
		ON_COMMAND(ID_HELP, &CMDIFrameWndEx::OnHelp)
		ON_COMMAND(ID_CONTEXT_HELP, &CMDIFrameWndEx::OnContextHelp)
		ON_COMMAND(ID_DEFAULT_HELP, &CMDIFrameWndEx::OnHelpFinder)
		ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
		ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
		ON_COMMAND(ID_VIEW_FULLSCREEN, OnViewFullScreen)
		ON_COMMAND(ID_MDI_TABBED, OnMdiTabbed)
		ON_UPDATE_COMMAND_UI(ID_MDI_TABBED, OnUpdateMdiTabbed)
		ON_REGISTERED_MESSAGE(AFX_WM_TOOLBARMENU, OnToolbarContextMenu)
		ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
		ON_REGISTERED_MESSAGE(AFX_WM_ON_GET_TAB_TOOLTIP, OnGetTabToolTip)
		ON_REGISTERED_MESSAGE(AFX_WM_RESETTOOLBAR, OnToolbarReset)
		ON_COMMAND_RANGE(ID_VIEW_APPLOOK_OFF_2007_BLUE, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
		ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_OFF_2007_BLUE, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
END_MESSAGE_MAP()
static unsigned Indicators[] = {
ID_INDICATOR_ICON, // status icon
ID_SEPARATOR, // status line indicator
ID_INDICATOR_PROGRESS, // progress bar
ID_OP0,
ID_OP1,
ID_OP2,
ID_OP3,
ID_OP4,
ID_OP5,
ID_OP6,
ID_OP7,
ID_OP8,
ID_OP9,
};

// CMainFrame construction/destruction
CMainFrame::CMainFrame() {
	theApp.applicationLook = static_cast<unsigned>(theApp.GetInt(L"ApplicationLook", ID_VIEW_APPLOOK_OFF_2007_BLACK));
}

int CMainFrame::OnCreate(LPCREATESTRUCT createStructure) {
	if (CMDIFrameWndEx::OnCreate(createStructure) == -1) { return -1; }
	UpdateMdiTabs(FALSE);
	if (m_MenuBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE) == 0) {
		TRACE0("Failed to create menubar\n");
		return -1;
	}
	m_MenuBar.SetPaneStyle(m_MenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);
	CList<unsigned, unsigned> ProtectedCommands;
	ProtectedCommands.AddTail(ID_VIEW_ANNOTATIONSCALES);
	ProtectedCommands.AddTail(ID_TOOLS_REGISTEREDCOMMANDS);
	CMFCToolBarButton::SetProtectedCommands(ProtectedCommands);

	// Prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);
	const unsigned long Style {WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC};
	if (m_StandardToolBar.CreateEx(this, TBSTYLE_FLAT, Style) == 0 || m_StandardToolBar.LoadToolBar(static_cast<unsigned>(theApp.HighColorMode() ? IDR_MAINFRAME_256 : IDR_MAINFRAME)) == 0) {
		TRACE0("Failed to create toolbar\n");
		return -1;
	}
	m_StandardToolBar.SetWindowTextW(L"Standard");
	m_StandardToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, L"Customize...");
	InitUserToolbars(nullptr, gc_FirstUserToolBarId, gc_LastUserToolBarId);
	if (m_StatusBar.Create(this) == 0) {
		TRACE0("Failed to create status bar\n");
		return -1;
	}
	m_StatusBar.SetIndicators(Indicators, sizeof Indicators / sizeof(unsigned));
	m_StatusBar.SetPaneStyle(gc_StatusIcon, SBPS_NOBORDERS);
	m_StatusBar.SetPaneStyle(gc_StatusInfo, SBPS_STRETCH | SBPS_NOBORDERS);
	m_StatusBar.SetPaneWidth(gc_StatusProgress, 96);
	m_MenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_StandardToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_MenuBar);
	DockPane(&m_StandardToolBar);
	CDockingManager::SetDockingMode(DT_SMART);
	EnableAutoHidePanes(CBRS_ALIGN_ANY);
	if (CreateDockingWindows() == 0) {
		TRACE0("Failed to create docking windows\n");
		return -1;
	}
	m_OutputPane.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_OutputPane);
	m_PropertiesPane.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_PropertiesPane);
	OnApplicationLook(theApp.applicationLook);
	EnableWindowsDialog(ID_WINDOW_MANAGER, IDS_WINDOWS_MANAGER, TRUE);

	// Enable automatic creation and management of the pop-up pane menu, which displays a list of application panes.
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, L"Customize...", ID_VIEW_TOOLBARS);
	EnableFullScreenMode(ID_VIEW_FULLSCREEN);
	EnableFullScreenMainMenu(TRUE);
	CMFCToolBar::EnableQuickCustomization();
	if (CMFCToolBar::GetUserImages() == nullptr) { // load user-defined toolbar images
		if (m_UserImages.Load(L"res\\UserImages.bmp") != 0) {
			m_UserImages.SetImageSize(CSize(16, 16), FALSE);
			CMFCToolBar::SetUserImages(&m_UserImages);
		}
	}
	// Shows the document name after thumbnail before the application name in a frame window title.
	ModifyStyle(0, FWS_PREFIXTITLE);
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& createStructure) {
	if (CMDIFrameWndEx::PreCreateWindow(createStructure) == 0) { return FALSE; }
	return TRUE;
}

BOOL CMainFrame::CreateDockingWindows() {
	const CSize DefaultSize(200, 200);
	const unsigned long SharedStyles {WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_FLOAT_MULTI};
	auto Caption {AeSys::LoadStringResource(IDS_OUTPUT)};
	if (m_OutputPane.Create(Caption, this, DefaultSize, TRUE, ID_VIEW_OUTPUTWND, SharedStyles | CBRS_BOTTOM) == 0) {
		TRACE0("Failed to create Output pane\n");
		return FALSE;
	}
	Caption = AeSys::LoadStringResource(IDS_PROPERTIES);
	if (m_PropertiesPane.Create(Caption, this, DefaultSize, TRUE, ID_VIEW_PROPERTIESWND, SharedStyles | CBRS_RIGHT) == 0) {
		TRACE0("Failed to create Properties pane\n");
		return FALSE;
	}
	SetDockablePanesIcons(theApp.HighColorMode());
	return TRUE;
}

void CMainFrame::DrawColorBox(CDC& deviceContext, const RECT& itemRectangle, const OdCmColor& color) {
	CBrush Brush(RGB(color.red(), color.green(), color.blue()));
	const auto OldBrush {deviceContext.SelectObject(&Brush)};
	CRect ItemRectangle(itemRectangle);
	ItemRectangle.DeflateRect(1, 1);
	ItemRectangle.right = ItemRectangle.left + ItemRectangle.Height();
	deviceContext.FillRect(&ItemRectangle, &Brush);
	deviceContext.SelectObject(OldBrush);
	CBrush FrameBrush;
	FrameBrush.CreateSysColorBrush(COLOR_WINDOWFRAME);
	deviceContext.FrameRect(&ItemRectangle, &FrameBrush);
	ItemRectangle.SetRect(ItemRectangle.right + 4, itemRectangle.top, itemRectangle.right, itemRectangle.bottom);
	if (ItemRectangle.left <= itemRectangle.right) {
		const auto ColorName {color.colorNameForDisplay()};
		deviceContext.ExtTextOutW(ItemRectangle.left, itemRectangle.top + 1, ETO_CLIPPED, &itemRectangle, ColorName, static_cast<unsigned>(ColorName.getLength()), nullptr);
	}
}

void CMainFrame::DrawLineWeight(CDC& deviceContext, const RECT& itemRectangle, const OdDb::LineWeight lineWeight) {
	const auto PixelsPerLogicalMillimeter {static_cast<double>(deviceContext.GetDeviceCaps(LOGPIXELSY)) / kMmPerInch};
	const auto PixelWidth {lineWeight <= 0 ? 0 : lround(double(lineWeight) / 100.0 * PixelsPerLogicalMillimeter)};
	LOGBRUSH Brush;
	Brush.lbStyle = BS_SOLID;
	Brush.lbColor = RGB(0x00, 0x00, 0x00);
	Brush.lbHatch = 0;
	CPen Pen(PS_SOLID | PS_GEOMETRIC | PS_ENDCAP_SQUARE, PixelWidth, &Brush);
	const auto OldPen {deviceContext.SelectObject(&Pen)};
	CRect ItemRectangle(itemRectangle);
	ItemRectangle.DeflateRect(2, 2);
	ItemRectangle.right = ItemRectangle.left + 40;
	deviceContext.MoveTo(ItemRectangle.left, ItemRectangle.CenterPoint().y);
	deviceContext.LineTo(ItemRectangle.right, ItemRectangle.CenterPoint().y);
	deviceContext.SelectObject(OldPen);
	ItemRectangle.SetRect(ItemRectangle.right + 8, itemRectangle.top, itemRectangle.right, itemRectangle.bottom);
	if (ItemRectangle.left <= itemRectangle.right) {
		const auto String {StringByLineWeight(lineWeight, false)};
		deviceContext.ExtTextOutW(ItemRectangle.left, ItemRectangle.top, ETO_CLIPPED, &itemRectangle, String, static_cast<unsigned>(String.getLength()), nullptr);
	}
}

void CMainFrame::DrawPlotStyle(CDC& deviceContext, const RECT& itemRectangle, const OdString& textOut, const OdDbDatabasePtr& database) {
	if (static_cast<int>(database->getPSTYLEMODE()) == 1) {
		const auto OldTextColor {deviceContext.SetTextColor(GetSysColor(COLOR_GRAYTEXT))};
		deviceContext.ExtTextOutW(itemRectangle.left + 6, itemRectangle.top + 1, ETO_CLIPPED, &itemRectangle, textOut, static_cast<unsigned>(textOut.getLength()), nullptr);
		deviceContext.SetTextColor(OldTextColor);
	} else {
		deviceContext.ExtTextOutW(itemRectangle.left + 6, itemRectangle.top + 1, ETO_CLIPPED, &itemRectangle, textOut, static_cast<unsigned>(textOut.getLength()), nullptr);
	}
}

void CMainFrame::SetDockablePanesIcons(const bool highColorMode) {
	const auto PropertiesPaneIcon {
	static_cast<HICON>(LoadImageW(AfxGetResourceHandle(), MAKEINTRESOURCEW(highColorMode ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0))
	};
	m_PropertiesPane.SetIcon(PropertiesPaneIcon, FALSE);
	const auto OutputPaneIcon {
	static_cast<HICON>(LoadImageW(AfxGetResourceHandle(), MAKEINTRESOURCEW(highColorMode ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0))
	};
	m_OutputPane.SetIcon(OutputPaneIcon, FALSE);
	UpdateMDITabbedBarsIcons();
}

// CMainFrame diagnostics
#ifdef _DEBUG
void CMainFrame::AssertValid() const {
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const {
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG
void CMainFrame::OnWindowManager() {
	ShowWindowsDialog();
}

void CMainFrame::OnViewCustomize() {
	auto Dialog {new CMFCToolBarsCustomizeDialog(this, TRUE)};
	Dialog->EnableUserDefinedToolbars();

	// Setup combobox:
	Dialog->ReplaceButton(ID_EDIT_FIND, EoCtrlFindComboBox());
	Dialog->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(const WPARAM wp, const LPARAM name) {
	const auto Result {CMDIFrameWndEx::OnToolbarCreateNew(wp, name)};
	if (Result == 0) { return 0; }
	auto UserToolbar {reinterpret_cast<CMFCToolBar*>(Result)};
	ASSERT_VALID(UserToolbar);
	const auto Customize {AeSys::LoadStringResource(IDS_TOOLBAR_CUSTOMIZE)};
	UserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, Customize);
	return Result;
}

LRESULT CMainFrame::OnToolbarReset(const WPARAM toolbarResourceId, LPARAM lparam) {
	switch (toolbarResourceId) {
		case IDR_MAINFRAME: case IDR_MAINFRAME_256: {
			m_StandardToolBar.ReplaceButton(ID_EDIT_FIND, EoCtrlFindComboBox(), FALSE);
			break;
		}
		case IDR_PROPERTIES:
			break;
		default: ;
	}
	return 0;
}

void CMainFrame::OnApplicationLook(const unsigned look) {
	theApp.applicationLook = look;
	switch (theApp.applicationLook) {
		case ID_VIEW_APPLOOK_WINDOWS_7:
			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
			break;
		default:
			switch (theApp.applicationLook) {
				case ID_VIEW_APPLOOK_OFF_2007_BLUE:
					CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
					break;
				case ID_VIEW_APPLOOK_OFF_2007_BLACK:
					CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
					break;
				case ID_VIEW_APPLOOK_OFF_2007_AQUA:
					CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
					break;
				case ID_VIEW_APPLOOK_OFF_2007_SILVER:
					CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
					break;
				default: ;
			}
			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
	}
	auto DockingManager {GetDockingManager()};
	ASSERT_VALID(DockingManager);
	DockingManager->AdjustPaneFrames();
	CDockingManager::SetDockingMode(DT_SMART);
	RecalcLayout();
	RedrawWindow(nullptr, nullptr, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);
	theApp.WriteInt(L"ApplicationLook", static_cast<int>(theApp.applicationLook));
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* commandUserInterface) {
	commandUserInterface->SetRadio(static_cast<BOOL>(theApp.applicationLook == commandUserInterface->m_nID));
}

BOOL CMainFrame::LoadFrame(const unsigned resourceId, const unsigned long defaultStyle, CWnd* parentWindow, CCreateContext* createContext) {
	if (CMDIFrameWndEx::LoadFrame(resourceId, defaultStyle, parentWindow, createContext) == 0) { return FALSE; }

	// Add some tools for example....
	auto UserToolsManager {theApp.GetUserToolsManager()};
	if (UserToolsManager != nullptr && UserToolsManager->GetUserTools().IsEmpty() != 0) {
		auto Tool1 {UserToolsManager->CreateNewTool()};
		Tool1->m_strLabel = L"&Notepad";
		Tool1->SetCommand(L"notepad.exe");
		auto Tool2 {UserToolsManager->CreateNewTool()};
		Tool2->m_strLabel = L"Paint &Brush";
		Tool2->SetCommand(L"mspaint.exe");
		auto Tool3 {UserToolsManager->CreateNewTool()};
		Tool3->m_strLabel = L"Fanning, Fanning & Associates On-&Line";
		Tool3->SetCommand(L"http://www.fanningfanning.com");
	}

	// Enable customization button for all user toolbars
	const auto Customize {AeSys::LoadStringResource(IDS_TOOLBAR_CUSTOMIZE)};
	for (auto i = 0; i < gc_MaximumUserToolbars; i++) {
		auto UserToolbar {GetUserToolBarByIndex(i)};
		if (UserToolbar != nullptr) {
			UserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, Customize);
		}
	}
	return TRUE;
}

LRESULT CMainFrame::OnToolbarContextMenu(WPARAM, const LPARAM point) {
	CMenu PopupToolbarMenu;
	VERIFY(PopupToolbarMenu.LoadMenuW(IDR_POPUP_TOOLBAR));
	auto SubMenu {PopupToolbarMenu.GetSubMenu(0)};
	ASSERT(SubMenu != nullptr);
	if (SubMenu != nullptr) {
		const CPoint Point(AFX_GET_X_LPARAM(point), AFX_GET_Y_LPARAM(point));
		auto PopupMenu {new CMFCPopupMenu};
		PopupMenu->Create(this, Point.x, Point.y, SubMenu->Detach());
	}
	return 0;
}

void CMainFrame::ShowAnnotationScalesPopupMenu(CMFCPopupMenu* popupMenu) {
	auto ActiveChildWindow {GetActiveFrame()};
	try {
		ENSURE(ActiveChildWindow);
		const auto ActiveDocument {ActiveChildWindow->GetActiveDocument()};
		ENSURE(ActiveDocument);
		auto Database {dynamic_cast<AeSysDoc*>(ActiveDocument)->m_DatabasePtr};
		popupMenu->RemoveAllItems();
		auto ContextManager {Database->objectContextManager()};
		const auto ScalesCollection {ContextManager->contextCollection(ODDB_ANNOTATIONSCALES_COLLECTION)};
		auto ScalesCollectionIterator {ScalesCollection->newIterator()};
		unsigned ScaleMenuPosition {1};
		const auto CurrentScaleIdentifier {Database->getCANNOSCALE()->uniqueIdentifier()};
		for (; !ScalesCollectionIterator->done() && ScaleMenuPosition < 100; ScalesCollectionIterator->next()) {
			auto ScaleName {ScalesCollectionIterator->getContext()->getName()};
			const auto ScaleIdentifier {ScalesCollectionIterator->getContext()->uniqueIdentifier()};
			CMFCToolBarMenuButton MenuButton(ScaleMenuPosition + _APS_NEXT_COMMAND_VALUE, nullptr, -1, ScaleName);
			if (ScaleIdentifier == CurrentScaleIdentifier) {
				MenuButton.SetStyle(TBBS_CHECKED);
			}
			popupMenu->InsertItem(MenuButton);
			ScaleMenuPosition++;
		}
	} catch (...) {
		AeSys::AddStringToMessageList(L"Annotation scale popup menu construction failed");
	}
}

// <command_console>
void CMainFrame::ShowRegisteredCommandsPopupMenu(CMFCPopupMenu* popupMenu) {
	try {
		popupMenu->RemoveAllItems();
		MENUITEMINFO MenuItemInfo;
		MenuItemInfo.cbSize = sizeof(MENUITEMINFO);
		MenuItemInfo.fMask = MIIM_DATA;
		auto CommandStack {odedRegCmds()};
		auto bHasNoCommand {CommandStack->newIterator()->done()};
		auto CommandId {_APS_NEXT_COMMAND_VALUE + 100};
		if (!bHasNoCommand) {
			auto CommandStackGroupIterator {CommandStack->newGroupIterator()};
			while (!CommandStackGroupIterator->done()) {
				OdRxDictionaryPtr Group = CommandStackGroupIterator->object();
				CMenu GroupMenu;
				GroupMenu.CreateMenu();
				OdRxIteratorPtr GroupCommandIterator = Group->newIterator(OdRx::kDictSorted);
				OdString GroupName;
				while (!GroupCommandIterator->done()) {
					OdEdCommandPtr pCmd = GroupCommandIterator->object().get();
					if (GroupName.isEmpty()) {
						GroupName = pCmd->groupName();
					}
					auto CommandName {pCmd->globalName()};
					GroupMenu.AppendMenuW(MF_STRING, static_cast<unsigned>(CommandId), CommandName);
					MenuItemInfo.dwItemData = reinterpret_cast<LPARAM>(pCmd.get());
					SetMenuItemInfoW(GroupMenu.m_hMenu, static_cast<unsigned>(CommandId), FALSE, &MenuItemInfo);
					GroupCommandIterator->next();
					CommandId++;
				}
				CMFCToolBarMenuButton MenuButton(unsigned(-1), GroupMenu.Detach(), -1, GroupName);
				popupMenu->InsertItem(MenuButton);
				CommandStackGroupIterator->next();
				GroupName.empty();
			}
		}
	} catch (...) {
		AeSys::AddStringToMessageList(L"Registered commands popup menu construction failed");
	}
}
// </command_console>
BOOL CMainFrame::OnShowPopupMenu(CMFCPopupMenu* popupMenu) {
	CMDIFrameWndEx::OnShowPopupMenu(popupMenu);
	if (popupMenu != nullptr) {
		if (popupMenu->GetMenuBar()->CommandToIndex(ID_VECTORIZERTYPE_CLEARMENU) >= 0) {
			if (CMFCToolBar::IsCustomizeMode() != 0) { return FALSE; }
			CRegKey RegistryKey;
			RegistryKey.Create(HKEY_CURRENT_USER, L"Software\\Engineers Office\\AeSys\\options\\vectorizers");
			unsigned long VectorizerIndex {0};
			CString VectorizerPath;
			unsigned long PathSize;
			for (;;) {
				PathSize = _MAX_FNAME + _MAX_EXT;
				const auto ReturnValue {RegEnumValueW(RegistryKey, VectorizerIndex, VectorizerPath.GetBuffer(static_cast<int>(PathSize)), &PathSize, nullptr, nullptr, nullptr, nullptr)};
				VectorizerPath.ReleaseBuffer();
				if (ReturnValue != ERROR_SUCCESS) { break; }
				CMFCToolBarMenuButton MenuButton(VectorizerIndex + ID_VECTORIZER_FIRST, nullptr, -1, VectorizerPath);
				if (theApp.RecentGsDevicePath().iCompare(static_cast<const wchar_t*>(VectorizerPath)) == 0) {
					MenuButton.SetStyle(TBBS_CHECKED);
				}
				popupMenu->InsertItem(MenuButton, static_cast<int>(VectorizerIndex++));
			}
		}
		if (popupMenu->GetMenuBar()->CommandToIndex(ID_TOOLS_REGISTEREDCOMMANDS) >= 0) {
			if (CMFCToolBar::IsCustomizeMode() != 0) { return FALSE; }
			ShowRegisteredCommandsPopupMenu(popupMenu);
		}
		if (popupMenu->GetMenuBar()->CommandToIndex(ID_VIEW_TOOLBARS) >= 0) {
			if (CMFCToolBar::IsCustomizeMode() != 0) { return FALSE; }
			popupMenu->RemoveAllItems();
			CMenu PopupToolbarMenu;
			VERIFY(PopupToolbarMenu.LoadMenuW(IDR_POPUP_TOOLBAR));
			const auto PopupSubMenu {PopupToolbarMenu.GetSubMenu(0)};
			ASSERT(PopupSubMenu != nullptr);
			if (PopupSubMenu != nullptr) {
				popupMenu->GetMenuBar()->ImportFromMenu(*PopupSubMenu, TRUE);
			}
		}
		if (popupMenu->GetMenuBar()->CommandToIndex(ID_VIEW_ANNOTATIONSCALES) >= 0) {
			if (CMFCToolBar::IsCustomizeMode() != 0) { return FALSE; }
			ShowAnnotationScalesPopupMenu(popupMenu);
		}
	}
	return TRUE;
}

void CMainFrame::UpdateMdiTabs(const BOOL resetMdiChild) {
	switch (theApp.applicationOptions.tabsStyle) {
		case EoApOptions::kNone: {
			int MdiTabsType;
			if (AreMDITabs(&MdiTabsType) != 0) {
				if (MdiTabsType == 1) {
					EnableMDITabs(FALSE);
				} else if (MdiTabsType == 2) {
					const CMDITabInfo TabInfo; // ignored when tabbed groups are disabled
					EnableMDITabbedGroups(FALSE, TabInfo);
				}
			} else {
				const auto ActiveWnd {reinterpret_cast<HWND>(m_wndClientArea.SendMessage(WM_MDIGETACTIVE))};
				m_wndClientArea.PostMessageW(WM_MDICASCADE);
				::BringWindowToTop(ActiveWnd);
			}
			break;
		}
		case EoApOptions::kStandard: {
			auto ActiveWnd {reinterpret_cast<HWND>(m_wndClientArea.SendMessage(WM_MDIGETACTIVE))};
			m_wndClientArea.PostMessageW(WM_MDIMAXIMIZE, WPARAM(ActiveWnd), 0L);
			::BringWindowToTop(ActiveWnd);
			EnableMDITabs(TRUE, theApp.applicationOptions.mdiTabInfo.m_bTabIcons, theApp.applicationOptions.mdiTabInfo.m_tabLocation, theApp.applicationOptions.mdiTabInfo.m_bTabCloseButton, theApp.applicationOptions.mdiTabInfo.m_style, theApp.applicationOptions.mdiTabInfo.m_bTabCustomTooltips, theApp.applicationOptions.mdiTabInfo.m_bActiveTabCloseButton);
			GetMDITabs().EnableAutoColor(theApp.applicationOptions.mdiTabInfo.m_bAutoColor);
			GetMDITabs().EnableTabDocumentsMenu(theApp.applicationOptions.mdiTabInfo.m_bDocumentMenu);
			GetMDITabs().EnableTabSwap(theApp.applicationOptions.mdiTabInfo.m_bEnableTabSwap);
			GetMDITabs().SetTabBorderSize(theApp.applicationOptions.mdiTabInfo.m_nTabBorderSize);
			GetMDITabs().SetFlatFrame(theApp.applicationOptions.mdiTabInfo.m_bFlatFrame);
			break;
		}
		case EoApOptions::kGrouped: {
			auto ActiveWnd {reinterpret_cast<HWND>(m_wndClientArea.SendMessage(WM_MDIGETACTIVE))};
			m_wndClientArea.PostMessageW(WM_MDIMAXIMIZE, WPARAM(ActiveWnd), 0L);
			::BringWindowToTop(ActiveWnd);
			EnableMDITabbedGroups(TRUE, theApp.applicationOptions.mdiTabInfo);
			break;
		}
	}
	CList<unsigned, unsigned> lstCommands;
	if (AreMDITabs() != 0) {
		lstCommands.AddTail(ID_WINDOW_ARRANGE);
		lstCommands.AddTail(ID_WINDOW_CASCADE);
		lstCommands.AddTail(ID_WINDOW_TILE_HORZ);
		lstCommands.AddTail(ID_WINDOW_TILE_VERT);
	}
	CMFCToolBar::SetNonPermittedCommands(lstCommands);
	if (resetMdiChild != 0) {
		const auto Maximize {theApp.applicationOptions.tabsStyle != EoApOptions::kNone};
		auto hwndT {::GetWindow(m_hWndMDIClient, GW_CHILD)};
		while (hwndT != nullptr) {
			auto pFrame {DYNAMIC_DOWNCAST(CMDIChildWndEx, CWnd::FromHandle(hwndT))};
			if (pFrame != nullptr) {
				ASSERT_VALID(pFrame);
				if (Maximize) {
					pFrame->ModifyStyle(WS_SYSMENU, 0);
				} else {
					pFrame->ModifyStyle(0, WS_SYSMENU);
					pFrame->ShowWindow(SW_RESTORE);

					// Force a resize to happen on all the "restored" MDI child windows
					CRect rectFrame;
					pFrame->GetWindowRect(rectFrame);
					pFrame->SetWindowPos(nullptr, -1, -1, rectFrame.Width() + 1, rectFrame.Height(), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
					pFrame->SetWindowPos(nullptr, -1, -1, rectFrame.Width(), rectFrame.Height(), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
				}
			}
			hwndT = ::GetWindow(hwndT, GW_HWNDNEXT);
		}
		if (Maximize) { m_MenuBar.SetMaximizeMode(FALSE); }
	}
	if (m_PropertiesPane.IsAutoHideMode() != 0) {
		m_PropertiesPane.BringWindowToTop();
		auto Divider {m_PropertiesPane.GetDefaultPaneDivider()};
		if (Divider != nullptr) { Divider->BringWindowToTop(); }
	}
	m_bDisableSetRedraw = static_cast<BOOL>(theApp.applicationOptions.disableSetRedraw);
	RecalcLayout();
	RedrawWindow(nullptr, nullptr, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}

// CMainFrame message handlers
BOOL CMainFrame::OnShowMDITabContextMenu(const CPoint point, const unsigned long allowedItems, const BOOL drop) {
	if (drop != 0 || !theApp.applicationOptions.tabsContextMenu) {
		return FALSE;
	}
	CMenu menu;
	VERIFY(menu.LoadMenuW(IDR_POPUP_MDITABS));
	auto PopupSubMenu {menu.GetSubMenu(0)};
	ASSERT(PopupSubMenu != nullptr);
	if (PopupSubMenu != nullptr) {
		if ((allowedItems & AFX_MDI_CAN_BE_DOCKED) == 0) {
			PopupSubMenu->DeleteMenu(ID_MDI_TABBED, MF_BYCOMMAND);
		}
		auto PopupMenu {new CMFCPopupMenu};
		if (PopupMenu != nullptr) {
			PopupMenu->SetAutoDestroy(FALSE);
			PopupMenu->Create(this, point.x, point.y, PopupSubMenu->GetSafeHmenu());
		}
	}
	return TRUE;
}

LRESULT CMainFrame::OnGetTabToolTip(WPARAM /*wp*/, const LPARAM lp) {
	auto TabToolTipInfo {reinterpret_cast<CMFCTabToolTipInfo*>(lp)};
	ASSERT(TabToolTipInfo != nullptr);
	if (TabToolTipInfo != nullptr) {
		ASSERT_VALID(TabToolTipInfo->m_pTabWnd);
		if (TabToolTipInfo->m_pTabWnd->IsMDITab() == 0) { return 0; }
		TabToolTipInfo->m_strText.Format(L"Tab #%d Custom Tooltip", TabToolTipInfo->m_nTabIndex + 1);
	}
	return 0;
}

void CMainFrame::OnMdiTabbed() {
	const auto pMDIChild {DYNAMIC_DOWNCAST(CMDIChildWndEx, MDIGetActive())};
	if (pMDIChild == nullptr) {
		ASSERT(FALSE);
		return;
	}
	TabbedDocumentToControlBar(pMDIChild);
}

void CMainFrame::OnUpdateMdiTabbed(CCmdUI* commandUserInterface) {
	commandUserInterface->SetCheck();
}

void CMainFrame::OnDestroy() {
	PostQuitMessage(0); 		// Force WM_QUIT message to terminate message loop
}

void CMainFrame::SetStatusPaneTextAt(const int index, const wchar_t* newText) {
	m_StatusBar.SetPaneText(index, newText);
}

void CMainFrame::SetStatusPaneTextColorAt(const int index, const COLORREF textColor) {
	m_StatusBar.SetPaneTextColor(index, textColor);
}

static UINT_PTR TimerId = 2;

void CMainFrame::OnStartProgress() {
	if (m_InProgress) {
		KillTimer(TimerId);
		m_StatusBar.EnablePaneProgressBar(gc_StatusProgress, -1);
		m_InProgress = false;
		return;
	}
	m_StatusBar.EnablePaneProgressBar(gc_StatusProgress, 100);
	m_CurrentProgress = 0;
	m_InProgress = true;
	TimerId = SetTimer(2, 1, nullptr);
}

void CMainFrame::OnTimer(const UINT_PTR nIDEvent) {
	if (nIDEvent == TimerId) {
		m_CurrentProgress += 10;
		if (m_CurrentProgress > 100) {
			m_CurrentProgress = 0;
		}
		m_StatusBar.SetPaneProgress(gc_StatusProgress, m_CurrentProgress);
	}
}

void CMainFrame::OnViewFullScreen() {
	ShowFullScreen();
}

CMFCToolBarComboBoxButton* CMainFrame::GetFindCombo() {
	CMFCToolBarComboBoxButton* FoundCombo = nullptr;
	CObList ButtonsList;
	if (CMFCToolBar::GetCommandButtons(ID_EDIT_FIND_COMBO, ButtonsList) > 0) {
		for (auto Position = ButtonsList.GetHeadPosition(); FoundCombo == nullptr && Position != nullptr;) {
			auto Combo {DYNAMIC_DOWNCAST(CMFCToolBarComboBoxButton, ButtonsList.GetNext(Position))};
			if (Combo != nullptr && Combo->GetEditCtrl()->GetSafeHwnd() == ::GetFocus()) {
				FoundCombo = Combo;
			}
		}
	}
	return FoundCombo;
}

HTREEITEM CMainFrame::InsertTreeViewControlItem(HWND tree, HTREEITEM parent, const wchar_t* text, LPCVOID object) noexcept {
	TVINSERTSTRUCTW tvIS;
	tvIS.hParent = parent;
	tvIS.hInsertAfter = TVI_LAST;
	tvIS.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvIS.item.hItem = nullptr;
	tvIS.item.iImage = 0;
	wchar_t Text[32];
	wcscpy(Text, text);
	tvIS.item.pszText = Text;
	tvIS.item.lParam = reinterpret_cast<LPARAM>(object);
	return TreeView_InsertItem(tree, &tvIS);
}

OdDb::LineWeight CMainFrame::LineWeightByIndex(const char lineWeight) noexcept {
	switch (lineWeight) {
		case 0:
			return OdDb::kLnWt000;
		case 1:
			return OdDb::kLnWt005;
		case 2:
			return OdDb::kLnWt009;
		case 3:
			return OdDb::kLnWt013;
		case 4:
			return OdDb::kLnWt015;
		case 5:
			return OdDb::kLnWt018;
		case 6:
			return OdDb::kLnWt020;
		case 7:
			return OdDb::kLnWt025;
		case 8:
			return OdDb::kLnWt030;
		case 9:
			return OdDb::kLnWt035;
		case 10:
			return OdDb::kLnWt040;
		case 11:
			return OdDb::kLnWt050;
		case 12:
			return OdDb::kLnWt053;
		case 13:
			return OdDb::kLnWt060;
		case 14:
			return OdDb::kLnWt070;
		case 15:
			return OdDb::kLnWt080;
		case 16:
			return OdDb::kLnWt090;
		case 17:
			return OdDb::kLnWt100;
		case 18:
			return OdDb::kLnWt106;
		case 19:
			return OdDb::kLnWt120;
		case 20:
			return OdDb::kLnWt140;
		case 21:
			return OdDb::kLnWt158;
		case 22:
			return OdDb::kLnWt200;
		case 23:
			return OdDb::kLnWt211;
		case 30:
			return OdDb::kLnWtByBlock;
		case 31:
			return OdDb::kLnWtByLwDefault;
		default: ;
	}
	return OdDb::kLnWtByLayer;
}

OdString CMainFrame::StringByLineWeight(int lineWeight, const bool lineWeightByIndex) {
	if (lineWeightByIndex) {
		lineWeight = LineWeightByIndex(gsl::narrow_cast<char>(lineWeight));
	}
	OdString LineWeightText {L""};
	switch (lineWeight) {
		case OdDb::kLnWtByLayer:
			LineWeightText = L"Layer";
			break;
		case OdDb::kLnWtByBlock:
			LineWeightText = L"Block";
			break;
		case OdDb::kLnWtByLwDefault:
			LineWeightText = L"Default";
			break;
		default:
			LineWeightText.format(L"%1.2f mm", static_cast<float>(lineWeight) / 100);
	}
	return LineWeightText;
}
