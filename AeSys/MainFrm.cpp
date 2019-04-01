#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"

#include "DbObjectContextCollection.h"
#include "DbObjectContextManager.h"

#include "EoCtrlFindComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

const int MaximumUserToolbars = 10;
const UINT FirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT LastUserToolBarId = FirstUserToolBarId + MaximumUserToolbars - 1;

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

static UINT Indicators[] = {
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

CMainFrame::CMainFrame() :
	m_CurrentProgress(0),
	m_InProgress(false) {
	m_ApplicationLook = theApp.GetInt(L"ApplicationLook", ID_VIEW_APPLOOK_OFF_2007_BLACK);
}
CMainFrame::~CMainFrame() {
}
int CMainFrame::OnCreate(LPCREATESTRUCT createStructure) {
	if (CMDIFrameWndEx::OnCreate(createStructure) == - 1) {
		return - 1;
	}
	OnApplicationLook(m_ApplicationLook);
	UpdateMDITabs(FALSE);

	if (!m_MenuBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE)) {
		ATLTRACE2(atlTraceGeneral, 0, L"Failed to create menubar\n");
		return - 1;
	}
	m_MenuBar.SetPaneStyle(m_MenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	CList<UINT, UINT> ProtectedCommands;
	ProtectedCommands.AddTail(ID_VIEW_ANNOTATIONSCALES);
	ProtectedCommands.AddTail(ID_TOOLS_REGISTEREDCOMMANDS);
	CMFCToolBarButton::SetProtectedCommands(ProtectedCommands);

	// Prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);
	const DWORD Style(WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	if (!m_StandardToolBar.CreateEx(this, TBSTYLE_FLAT, Style) || !m_StandardToolBar.LoadToolBar(theApp.HighColorMode() ? IDR_MAINFRAME_256 : IDR_MAINFRAME)) {
		ATLTRACE2(atlTraceGeneral, 0, L"Failed to create toolbar\n");
		return - 1;
	}
	m_StandardToolBar.SetWindowTextW(L"Standard");
	m_StandardToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, L"Customize...");

	InitUserToolbars(NULL, FirstUserToolBarId, LastUserToolBarId);

	if (!m_StatusBar.Create(this)) {
		ATLTRACE2(atlTraceGeneral, 0, L"Failed to create status bar\n");
		return - 1;
	}
	m_StatusBar.SetIndicators(Indicators, sizeof(Indicators) / sizeof(UINT));

	m_StatusBar.SetPaneStyle(nStatusIcon, SBPS_NOBORDERS);
	m_StatusBar.SetPaneStyle(nStatusInfo, SBPS_STRETCH | SBPS_NOBORDERS);
	m_StatusBar.SetPaneWidth(nStatusProgress, 96);

	if (!CreateDockablePanes()) {
		ATLTRACE2(atlTraceGeneral, 0, L"Failed to create dockable panes\n");
		return - 1;
	}
	m_MenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_StandardToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_PropertiesPane.EnableDocking(CBRS_ALIGN_ANY);
	m_OutputPane.EnableDocking(CBRS_ALIGN_ANY);

	EnableDocking(CBRS_ALIGN_ANY);

	DockPane(&m_MenuBar);
	DockPane(&m_StandardToolBar);
	DockPane(&m_PropertiesPane);
	DockPane(&m_OutputPane);

	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	EnableWindowsDialog(ID_WINDOW_MANAGER, IDS_WINDOWS_MANAGER, TRUE);

	// Enable automatic creation and management of the pop-up pane menu, which displays a list of application panes.
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, L"Customize...", ID_VIEW_TOOLBARS, FALSE, FALSE);
	EnableFullScreenMode(ID_VIEW_FULLSCREEN);
	EnableFullScreenMainMenu(TRUE);

	CMFCToolBar::EnableQuickCustomization();

	if (CMFCToolBar::GetUserImages() == NULL) { // load user-defined toolbar images
		if (m_UserImages.Load(L"res\\UserImages.bmp")) {
			m_UserImages.SetImageSize(CSize(16, 16), FALSE);
			CMFCToolBar::SetUserImages(&m_UserImages);
		}
	}
	// Shows the document name after thumbnail before the application name in a frame window title.
	ModifyStyle(0, FWS_PREFIXTITLE);

	return 0;
}
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& createStructure) {
	if (!CMDIFrameWndEx::PreCreateWindow(createStructure)) {
		return FALSE;
	}
	return TRUE;
}
BOOL CMainFrame::CreateDockablePanes() {
	const CSize DefaultSize(200, 200);

	const DWORD SharedStyles(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_FLOAT_MULTI);

	CString Caption = theApp.LoadStringResource(IDS_OUTPUT);
	if (!m_OutputPane.Create(Caption, this, DefaultSize, TRUE, ID_VIEW_OUTPUTWND, SharedStyles | CBRS_BOTTOM)) {
		ATLTRACE2(atlTraceGeneral, 0, L"Failed to create Output pane\n");
		return FALSE;
	}
	Caption = theApp.LoadStringResource(IDS_PROPERTIES);
	if (!m_PropertiesPane.Create(Caption, this, DefaultSize, TRUE, ID_VIEW_PROPERTIESWND, SharedStyles | CBRS_RIGHT)) {
		ATLTRACE2(atlTraceGeneral, 0, L"Failed to create Properties pane\n");
		return FALSE;
	}
	SetDockablePanesIcons(theApp.HighColorMode());
	return TRUE;
}
void CMainFrame::DrawColorBox(CDC& deviceContext, const RECT& itemRectangle, const OdCmColor& color) {
	CBrush Brush(RGB(color.red(), color.green(), color.blue()));
	CBrush* OldBrush = deviceContext.SelectObject(&Brush);
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
		CString ColorName = color.colorNameForDisplay();
		deviceContext.ExtTextOutW(ItemRectangle.left, itemRectangle.top + 1, ETO_CLIPPED, &itemRectangle, ColorName, ColorName.GetLength(), NULL);
	}
}
void CMainFrame::DrawLineWeight(CDC& deviceContext, const RECT& itemRectangle, const OdDb::LineWeight lineWeight) {
	const double PixelsPerLogicalMillimeter = static_cast<double>(deviceContext.GetDeviceCaps(LOGPIXELSY)) / EoMmPerInch;
	const int PixelWidth = (lineWeight <= 0) ? 0 : int((double(lineWeight) / 100. * PixelsPerLogicalMillimeter) + .5);

	LOGBRUSH Brush;
	Brush.lbStyle = BS_SOLID;
	Brush.lbColor = RGB(0x00, 0x00, 0x00);
	Brush.lbHatch = 0;

	CPen Pen(PS_SOLID | PS_GEOMETRIC | PS_ENDCAP_SQUARE, PixelWidth, &Brush);
	CPen* OldPen = deviceContext.SelectObject(&Pen);

	CRect ItemRectangle(itemRectangle);
	ItemRectangle.DeflateRect(2, 2);
	ItemRectangle.right = ItemRectangle.left + 40;

	deviceContext.MoveTo(ItemRectangle.left, ItemRectangle.CenterPoint().y);
	deviceContext.LineTo(ItemRectangle.right, ItemRectangle.CenterPoint().y);
	deviceContext.SelectObject(OldPen);

	ItemRectangle.SetRect(ItemRectangle.right + 8, itemRectangle.top, itemRectangle.right, itemRectangle.bottom);
	if (ItemRectangle.left <= itemRectangle.right) {
		OdString String = CMainFrame::StringByLineWeight(lineWeight, false);
		deviceContext.ExtTextOutW(ItemRectangle.left, ItemRectangle.top, ETO_CLIPPED, &itemRectangle, String, String.getLength(), NULL);
	}
}
void CMainFrame::DrawPlotStyle(CDC& deviceContext, const RECT& itemRectangle, const CString& textOut, const OdDbDatabasePtr database) {
	if (database->getPSTYLEMODE() == 1) {
		const COLORREF OldTextColor = deviceContext.SetTextColor(GetSysColor(COLOR_GRAYTEXT));
		deviceContext.ExtTextOutW(itemRectangle.left + 6, itemRectangle.top + 1, ETO_CLIPPED, &itemRectangle, textOut, textOut.GetLength(), NULL);
		deviceContext.SetTextColor(OldTextColor);
	}
	else {
		deviceContext.ExtTextOutW(itemRectangle.left + 6, itemRectangle.top + 1, ETO_CLIPPED, &itemRectangle, textOut, textOut.GetLength(), NULL);
	}
}
void CMainFrame::SetDockablePanesIcons(bool highColorMode) {
	const CSize SmallIconSize(::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	HINSTANCE ResourceHandle(::AfxGetResourceHandle());

	HICON PropertiesPaneIcon = (HICON) ::LoadImage(ResourceHandle, MAKEINTRESOURCE(highColorMode ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, SmallIconSize.cx, SmallIconSize.cy, 0);
	m_PropertiesPane.SetIcon(PropertiesPaneIcon, FALSE);

	HICON OutputPaneIcon = (HICON) ::LoadImage(ResourceHandle, MAKEINTRESOURCE(highColorMode ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, SmallIconSize.cx, SmallIconSize.cy, 0);
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
	CMFCToolBarsCustomizeDialog* Dialog = new CMFCToolBarsCustomizeDialog(this, TRUE);
	Dialog->EnableUserDefinedToolbars();

	// Setup combobox:
	Dialog->ReplaceButton(ID_EDIT_FIND, EoCtrlFindComboBox());

	Dialog->Create();
}
LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp, LPARAM name) {
	LRESULT Result = CMDIFrameWndEx::OnToolbarCreateNew(wp, name);
	if (Result == 0) {
		return 0;
	}
	CMFCToolBar* UserToolbar = (CMFCToolBar*) Result;
	ASSERT_VALID(UserToolbar);

	CString Customize = theApp.LoadStringResource(IDS_TOOLBAR_CUSTOMIZE);

	UserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, Customize);
	return Result;
}
LRESULT CMainFrame::OnToolbarReset(WPARAM toolbarResourceId, LPARAM lparam) {
	switch (toolbarResourceId) {
	case IDR_MAINFRAME:
	case IDR_MAINFRAME_256: {
			m_StandardToolBar.ReplaceButton(ID_EDIT_FIND, EoCtrlFindComboBox(), FALSE);
			break;
		}
	case IDR_PROPERTIES:
		break;
	}
	return 0;
}
void CMainFrame::OnApplicationLook(UINT look) {
	m_ApplicationLook = look;

	switch (m_ApplicationLook) {
	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		break;

	default:
		switch (m_ApplicationLook) {
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
		}
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
	}
	CDockingManager* DockingManager = GetDockingManager();
	ASSERT_VALID(DockingManager);
	DockingManager->AdjustPaneFrames();
	DockingManager->SetDockingMode(DT_SMART);

	RecalcLayout();
	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_FRAME | RDW_ERASE | RDW_UPDATENOW);

	theApp.WriteInt(L"ApplicationLook", m_ApplicationLook);
}
void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI) {
	pCmdUI->SetRadio(m_ApplicationLook == pCmdUI->m_nID);
}
BOOL CMainFrame::LoadFrame(UINT resourceId, DWORD defaultStyle, CWnd* parentWindow, CCreateContext* createContext) {
	if (!CMDIFrameWndEx::LoadFrame(resourceId, defaultStyle, parentWindow, createContext)) {
		return FALSE;
	}
	// Add some tools for example....
	CUserToolsManager* UserToolsManager = theApp.GetUserToolsManager();
	if (UserToolsManager != NULL && UserToolsManager->GetUserTools().IsEmpty()) {
		CUserTool* Tool1 = UserToolsManager->CreateNewTool();
		Tool1->m_strLabel = L"&Notepad";
		Tool1->SetCommand(L"notepad.exe");

		CUserTool* Tool2 = UserToolsManager->CreateNewTool();
		Tool2->m_strLabel = L"Paint &Brush";
		Tool2->SetCommand(L"mspaint.exe");

		CUserTool* Tool3 = UserToolsManager->CreateNewTool();
		Tool3->m_strLabel = L"Fanning, Fanning & Associates On-&Line";
		Tool3->SetCommand(L"http://www.fanningfanning.com");
	}

	// Enable customization button for all user toolbars
	CString Customize = theApp.LoadStringResource(IDS_TOOLBAR_CUSTOMIZE);

	for (int i = 0; i < MaximumUserToolbars; i ++) {
		CMFCToolBar* UserToolbar = GetUserToolBarByIndex(i);
		if (UserToolbar != NULL) {
			UserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, Customize);
		}
	}
	return TRUE;
}
LRESULT CMainFrame::OnToolbarContextMenu(WPARAM, LPARAM point) {
	CMenu PopupToolbarMenu;
	VERIFY(PopupToolbarMenu.LoadMenu(IDR_POPUP_TOOLBAR));

	CMenu* SubMenu = PopupToolbarMenu.GetSubMenu(0);
	ASSERT(SubMenu != NULL);

	if (SubMenu) {
		const CPoint Point(AFX_GET_X_LPARAM(point), AFX_GET_Y_LPARAM(point));

		CMFCPopupMenu* PopupMenu = new CMFCPopupMenu;
		PopupMenu->Create(this, Point.x, Point.y, SubMenu->Detach());
	}
	return 0;
}

void CMainFrame::ShowAnnotationScalesPopupMenu(CMFCPopupMenu* popupMenu) {
	CFrameWnd* ActiveChildWindow(GetActiveFrame());
	try {
		ENSURE(ActiveChildWindow);
		CDocument* ActiveDocument = ActiveChildWindow->GetActiveDocument();
		ENSURE(ActiveDocument);
		OdDbDatabasePtr Database = ((AeSysDoc*) ActiveDocument)->m_DatabasePtr;

		popupMenu->RemoveAllItems();

		OdDbObjectContextManagerPtr ContextManager(Database->objectContextManager());
		OdDbObjectContextCollection* ScalesCollection(ContextManager->contextCollection(ODDB_ANNOTATIONSCALES_COLLECTION));
		OdDbObjectContextCollectionIteratorPtr ScalesCollectionIterator = ScalesCollection->newIterator();

		size_t ScaleMenuPosition = 1;
		OdIntPtr CurrentScaleIdentifier = Database->getCANNOSCALE()->uniqueIdentifier();
		for (; !ScalesCollectionIterator->done() && ScaleMenuPosition < 100; ScalesCollectionIterator->next()) {
			OdString ScaleName = (LPWSTR)(LPCWSTR) ScalesCollectionIterator->getContext()->getName();
			OdIntPtr ScaleIdentifier = ScalesCollectionIterator->getContext()->uniqueIdentifier();

			CMFCToolBarMenuButton MenuButton(ScaleMenuPosition + _APS_NEXT_COMMAND_VALUE, NULL, - 1, ScaleName);

			if (ScaleIdentifier == CurrentScaleIdentifier) {
				MenuButton.SetStyle(TBBS_CHECKED);
			}
			popupMenu->InsertItem(MenuButton);

			ScaleMenuPosition++;
		}
	}
	catch (...) {
		theApp.AddStringToMessageList(L"Annotation scale popup menu construction failed");
	}
}
void CMainFrame::ShowRegisteredCommandsPopupMenu(CMFCPopupMenu* popupMenu) {
	try {
		popupMenu->RemoveAllItems();

		MENUITEMINFO MenuItemInfo;
		MenuItemInfo.cbSize = sizeof(MENUITEMINFO);
		MenuItemInfo.fMask = MIIM_DATA;

#ifdef DEV_COMMAND_CONSOLE
		OdEdCommandStackPtr CommandStack = ::odedRegCmds();
		bool bHasNoCommand = CommandStack->newIterator()->done();

		int CommandId = _APS_NEXT_COMMAND_VALUE + 100;
		if (!bHasNoCommand) {
			OdRxIteratorPtr CommandStackGroupIterator = CommandStack->newGroupIterator();
			while(!CommandStackGroupIterator->done()) {
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
					OdString CommandName(pCmd->globalName());
					GroupMenu.AppendMenuW(MF_STRING, CommandId, CommandName);
				
					MenuItemInfo.dwItemData = (LPARAM) pCmd.get();
					VERIFY(::SetMenuItemInfoW(GroupMenu.m_hMenu, CommandId, FALSE, &MenuItemInfo));

					GroupCommandIterator->next();
					CommandId++;
				}
				CMFCToolBarMenuButton MenuButton(UINT(- 1), GroupMenu.Detach(), - 1, GroupName);
				popupMenu->InsertItem(MenuButton);

				CommandStackGroupIterator->next();
				GroupName.empty();
			}
		}
#endif // DEV_COMMAND_CONSOLE
	}
	catch (...) {
		theApp.AddStringToMessageList(L"Registered commands popup menu construction failed");
	}
}
BOOL CMainFrame::OnShowPopupMenu(CMFCPopupMenu* popupMenu) {
	CMDIFrameWndEx::OnShowPopupMenu(popupMenu);
	if (popupMenu != NULL) {
		if (popupMenu->GetMenuBar()->CommandToIndex(ID_VECTORIZE_CLEARMENU) >= 0) {
			if (CMFCToolBar::IsCustomizeMode()) {
				return FALSE;
			}
			CRegKey RegistryKey;
			RegistryKey.Create(HKEY_CURRENT_USER, L"Software\\Engineers Office\\AeSys\\options\\vectorizers");

			DWORD VectorizerIndex(0);

			CString VectorizerPath;
			DWORD PathSize;
			for(;;) {
				PathSize = _MAX_FNAME + _MAX_EXT;
				const DWORD ReturnValue = ::RegEnumValueW(RegistryKey, VectorizerIndex, VectorizerPath.GetBuffer(PathSize), &PathSize, NULL, NULL, NULL, NULL);
				VectorizerPath.ReleaseBuffer();
				if (ReturnValue != ERROR_SUCCESS) {
					break;
				}
				else {
					CMFCToolBarMenuButton MenuButton(VectorizerIndex + ID_VECTORIZER_FIRST, NULL, - 1, VectorizerPath);

					if (theApp.recentGsDevicePath().iCompare((LPCWSTR) VectorizerPath) == 0) {
						MenuButton.SetStyle(TBBS_CHECKED);
					}
					popupMenu->InsertItem(MenuButton, VectorizerIndex++);
				}
			}
		}
		if (popupMenu->GetMenuBar()->CommandToIndex(ID_TOOLS_REGISTEREDCOMMANDS) >= 0) {
			if (CMFCToolBar::IsCustomizeMode()) {
				return FALSE;
			}
			ShowRegisteredCommandsPopupMenu(popupMenu);
		}
		if (popupMenu->GetMenuBar()->CommandToIndex(ID_VIEW_TOOLBARS) >= 0) {
		if (CMFCToolBar::IsCustomizeMode()) {
			return FALSE;
		}
		popupMenu->RemoveAllItems();

		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_POPUP_TOOLBAR));

		CMenu* PopupSubMenu = menu.GetSubMenu(0);
		ASSERT(PopupSubMenu != NULL);

		if (PopupSubMenu) {
			popupMenu->GetMenuBar()->ImportFromMenu(*PopupSubMenu, TRUE);
		}
	}
		if (popupMenu->GetMenuBar()->CommandToIndex(ID_VIEW_ANNOTATIONSCALES) >= 0) {
			if (CMFCToolBar::IsCustomizeMode()) {
				return FALSE;
			}
			ShowAnnotationScalesPopupMenu(popupMenu);
		}
	}
	return TRUE;
}
void CMainFrame::UpdateMDITabs(BOOL resetMDIChild) {
	switch (theApp.m_Options.m_nTabsStyle) {
	case EoApOptions::None: {
			int MDITabsType;

			if (AreMDITabs(&MDITabsType)) {
				if (MDITabsType == 1) {
					EnableMDITabs(FALSE);
				}
				else if (MDITabsType == 2) {
					const CMDITabInfo TabInfo; // ignored when tabbed groups are disabled

					EnableMDITabbedGroups(FALSE, TabInfo);
				}
			}
			else {
				HWND ActiveWnd = (HWND) m_wndClientArea.SendMessage(WM_MDIGETACTIVE);
				m_wndClientArea.PostMessage(WM_MDICASCADE);
				::BringWindowToTop(ActiveWnd);
			}
			break;
		}
	case EoApOptions::Standard: {
			HWND ActiveWnd = (HWND) m_wndClientArea.SendMessage(WM_MDIGETACTIVE);
			m_wndClientArea.PostMessage(WM_MDIMAXIMIZE, LPARAM(ActiveWnd), 0L);
			::BringWindowToTop(ActiveWnd);

			EnableMDITabs(TRUE,
				theApp.m_Options.m_MdiTabInfo.m_bTabIcons,
				theApp.m_Options.m_MdiTabInfo.m_tabLocation,
				theApp.m_Options.m_MdiTabInfo.m_bTabCloseButton,
				theApp.m_Options.m_MdiTabInfo.m_style,
				theApp.m_Options.m_MdiTabInfo.m_bTabCustomTooltips,
				theApp.m_Options.m_MdiTabInfo.m_bActiveTabCloseButton);

			GetMDITabs().EnableAutoColor(theApp.m_Options.m_MdiTabInfo.m_bAutoColor);
			GetMDITabs().EnableTabDocumentsMenu(theApp.m_Options.m_MdiTabInfo.m_bDocumentMenu);
			GetMDITabs().EnableTabSwap(theApp.m_Options.m_MdiTabInfo.m_bEnableTabSwap);
			GetMDITabs().SetTabBorderSize(theApp.m_Options.m_MdiTabInfo.m_nTabBorderSize);
			GetMDITabs().SetFlatFrame(theApp.m_Options.m_MdiTabInfo.m_bFlatFrame);
			break;
		}
	case EoApOptions::Grouped: {
			HWND ActiveWnd = (HWND) m_wndClientArea.SendMessage(WM_MDIGETACTIVE);
			m_wndClientArea.PostMessage(WM_MDIMAXIMIZE, LPARAM(ActiveWnd), 0L);
			::BringWindowToTop(ActiveWnd);

			EnableMDITabbedGroups(TRUE, theApp.m_Options.m_MdiTabInfo);
			break;
		}
	}
	CList<UINT, UINT> lstCommands;
	if (AreMDITabs(NULL)) {
		lstCommands.AddTail(ID_WINDOW_ARRANGE);
		lstCommands.AddTail(ID_WINDOW_CASCADE);
		lstCommands.AddTail(ID_WINDOW_TILE_HORZ);
		lstCommands.AddTail(ID_WINDOW_TILE_VERT);
	}
	CMFCToolBar::SetNonPermittedCommands(lstCommands);
	if (resetMDIChild) {
		const BOOL bMaximize = theApp.m_Options.m_nTabsStyle != EoApOptions::None;

		HWND hwndT = ::GetWindow(m_hWndMDIClient, GW_CHILD);
		while (hwndT != NULL) {
			CMDIChildWndEx* pFrame = DYNAMIC_DOWNCAST(CMDIChildWndEx, CWnd::FromHandle(hwndT));
			if (pFrame != NULL) {
				ASSERT_VALID(pFrame);
				if (bMaximize) {
					pFrame->ModifyStyle(WS_SYSMENU, 0);
				}
				else {
					pFrame->ModifyStyle(0, WS_SYSMENU);
					pFrame->ShowWindow(SW_RESTORE);

					// Force a resize to happen on all the "restored" MDI child windows
					CRect rectFrame;
					pFrame->GetWindowRect(rectFrame);
					pFrame->SetWindowPos(NULL, - 1, - 1, rectFrame.Width() + 1, rectFrame.Height(), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
					pFrame->SetWindowPos(NULL, - 1, - 1, rectFrame.Width(), rectFrame.Height(), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
				}
			}
			hwndT = ::GetWindow(hwndT, GW_HWNDNEXT);
		}
		if (bMaximize) {
			m_MenuBar.SetMaximizeMode(FALSE);
		}
	}
	if (m_PropertiesPane.IsAutoHideMode()) {
		m_PropertiesPane.BringWindowToTop();
		CPaneDivider* Divider = m_PropertiesPane.GetDefaultPaneDivider();
		if (Divider != NULL) {
			Divider->BringWindowToTop();
		}
	}
	CMDIFrameWndEx::m_bDisableSetRedraw = theApp.m_Options.m_bDisableSetRedraw;

	RecalcLayout();
	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}

// CMainFrame message handlers

BOOL CMainFrame::OnShowMDITabContextMenu(CPoint point, DWORD dwAllowedItems, BOOL bDrop) {
	if (bDrop || !theApp.m_Options.m_bTabsContextMenu) {
		return FALSE;
	}
	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_POPUP_MDITABS));

	CMenu* PopupSubMenu = menu.GetSubMenu(0);
	ASSERT(PopupSubMenu != NULL);

	if (PopupSubMenu) {
		if ((dwAllowedItems & AFX_MDI_CAN_BE_DOCKED) == 0) {
			PopupSubMenu->DeleteMenu(ID_MDI_TABBED, MF_BYCOMMAND);
		}
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;
		if (pPopupMenu) {
			pPopupMenu->SetAutoDestroy(FALSE);
			pPopupMenu->Create (this, point.x, point.y, PopupSubMenu->GetSafeHmenu());
		}
	}
	return TRUE;
}
LRESULT CMainFrame::OnGetTabToolTip(WPARAM /*wp*/, LPARAM lp) {
	CMFCTabToolTipInfo* pInfo = (CMFCTabToolTipInfo*) lp;
	ASSERT (pInfo != NULL);

	if (pInfo) {
		ASSERT_VALID(pInfo->m_pTabWnd);
		if (!pInfo->m_pTabWnd->IsMDITab()) {
			return 0;
		}
		pInfo->m_strText.Format(L"Tab #%d Custom Tooltip", pInfo->m_nTabIndex + 1);
	}
	return 0;
}
void CMainFrame::OnMdiTabbed() {
	CMDIChildWndEx* pMDIChild = DYNAMIC_DOWNCAST(CMDIChildWndEx, MDIGetActive());
	if (pMDIChild == NULL) {
		ASSERT (FALSE);
		return;
	}
	TabbedDocumentToControlBar(pMDIChild);
}
void CMainFrame::OnUpdateMdiTabbed(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck();
}
void CMainFrame::OnDestroy() {
	ATLTRACE2(atlTraceGeneral, 1, L"CMainFrame::OnDestroy() - Entering\n");

	PostQuitMessage(0); 		// Force WM_QUIT message to terminate message loop
}

void CMainFrame::SetStatusPaneTextAt(int index, LPCWSTR newText) {
	m_StatusBar.SetPaneText(index, newText);
}
void CMainFrame::SetStatusPaneTextColorAt(int index, COLORREF textColor) {
	m_StatusBar.SetPaneTextColor(index, textColor);
}
static UINT_PTR TimerId = 2;

void CMainFrame::OnStartProgress(void) {
	if (m_InProgress) {
		KillTimer(TimerId);
		m_StatusBar.EnablePaneProgressBar(nStatusProgress, - 1);

		m_InProgress = false;

		return;
	}
	m_StatusBar.EnablePaneProgressBar(nStatusProgress, 100);

	m_CurrentProgress = 0;
	m_InProgress = true;

	TimerId = SetTimer(2, 1, NULL);
}
void CMainFrame::OnTimer(UINT_PTR nIDEvent) {
	ATLTRACE2(atlTraceGeneral, 0, L"CMainFrame::OnTimer(%i)\n", nIDEvent);

	if (nIDEvent == TimerId) {
		m_CurrentProgress += 10;

		if (m_CurrentProgress > 100) {
			m_CurrentProgress = 0;
		}
		m_StatusBar.SetPaneProgress (nStatusProgress, m_CurrentProgress);
	}
}
void CMainFrame::OnViewFullScreen(void) {
	ShowFullScreen();
}
CMFCToolBarComboBoxButton* CMainFrame::GetFindCombo(void) {
	CMFCToolBarComboBoxButton* FoundCombo = NULL;

	CObList ButtonsList;
	if (CMFCToolBar::GetCommandButtons(ID_EDIT_FIND_COMBO, ButtonsList) > 0) {
		for (POSITION Position = ButtonsList.GetHeadPosition(); FoundCombo == NULL && Position != NULL; ) {
			CMFCToolBarComboBoxButton* Combo = DYNAMIC_DOWNCAST(CMFCToolBarComboBoxButton, ButtonsList.GetNext(Position));

			if (Combo != NULL && Combo->GetEditCtrl()->GetSafeHwnd() == ::GetFocus()) {
				FoundCombo = Combo;
			}
		}
	}
	return FoundCombo;
}
HTREEITEM CMainFrame::InsertTreeViewControlItem(HWND tree, HTREEITEM parent, LPWSTR text, LPCVOID object) {
	TV_INSERTSTRUCT tvIS;
	tvIS.hParent = parent;
	tvIS.hInsertAfter = TVI_LAST;
	tvIS.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvIS.item.hItem = NULL;
	tvIS.item.iImage = 0;

	tvIS.item.pszText = (LPWSTR) text;
	tvIS.item.lParam = (LPARAM) object;
	return TreeView_InsertItem(tree, &tvIS);
}

OdDb::LineWeight CMainFrame::LineWeightByIndex(char lineWeight) {
	switch(lineWeight) {
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
	}
	return OdDb::kLnWtByLayer;
}
CString CMainFrame::StringByLineWeight(int lineWeight, bool lineWeightByIndex) {
	if (lineWeightByIndex) {
		lineWeight = LineWeightByIndex(char(lineWeight));
	}
	CString LineWeightText = L"";
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
		LineWeightText.Format(L"%1.2f mm", (float) lineWeight / 100);
	}
	return LineWeightText;
}

