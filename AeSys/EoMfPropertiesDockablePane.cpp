#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysView.h"

#include "EoMfPropertiesDockablePane.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static wchar_t* TabsStyles[] = { L"None", L"Standard", L"Grouped", NULL };
static wchar_t* TabLocations[] = { L"On Bottom", L"On Top", NULL };

BEGIN_MESSAGE_MAP(EoMfPropertiesDockablePane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_WM_SIZE()
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_COMMAND(ID_PROPERTIES1, OnProperties1)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES1, OnUpdateProperties1)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
END_MESSAGE_MAP()

EoMfPropertiesDockablePane::EoMfPropertiesDockablePane() {
}
EoMfPropertiesDockablePane::~EoMfPropertiesDockablePane() {
}
int EoMfPropertiesDockablePane::OnCreate(LPCREATESTRUCT createStructure) {
	if (CDockablePane::OnCreate(createStructure) == - 1) {
		return - 1;
	}
	CRect EmptyRect;
	EmptyRect.SetRectEmpty();

	if (!m_wndObjectCombo.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, EmptyRect, this, 1)) {
		ATLTRACE2(atlTraceGeneral, 0, L"Failed to create Properties Combo\n");
		return - 1;
	}
	m_wndObjectCombo.AddString(L"Application");
	m_wndObjectCombo.AddString(L"Persistant");
	m_wndObjectCombo.SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)), TRUE);
	m_wndObjectCombo.SetCurSel(0);

	if (!m_PropertyGrid.Create(WS_VISIBLE | WS_CHILD, EmptyRect, this, 2)) {
		ATLTRACE2(atlTraceGeneral, 0, L"Failed to create Properties Grid \n");
		return - 1;
	}
	InitializePropertyGrid();
	SetWorkspaceTabsSubItemsState();

	m_PropertiesToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_PropertiesToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE, 0, 0, 0);
	m_PropertiesToolBar.CleanUpLockedImages();
	m_PropertiesToolBar.LoadBitmap(theApp.HighColorMode() ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE, 0, 0);

	m_PropertiesToolBar.SetPaneStyle(m_PropertiesToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_PropertiesToolBar.SetPaneStyle(m_PropertiesToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_PropertiesToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_PropertiesToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}
void EoMfPropertiesDockablePane::OnSetFocus(CWnd* oldWindow) {
	CDockablePane::OnSetFocus(oldWindow);
	m_PropertyGrid.SetFocus();
}
void EoMfPropertiesDockablePane::OnSettingChange(UINT flags, LPCWSTR section) {
	CDockablePane::OnSettingChange(flags, section);
	SetPropertyGridFont();
}
void EoMfPropertiesDockablePane::OnSize(UINT type, int cx, int cy) {
	CDockablePane::OnSize(type, cx, cy);
	AdjustLayout();
}
LRESULT EoMfPropertiesDockablePane::OnPropertyChanged(WPARAM, LPARAM lparam) {
	CMFCPropertyGridProperty* Property = (CMFCPropertyGridProperty*) lparam;

	BOOL ResetMDIChild = FALSE;

	switch (int(Property->GetData())) {
    case kTabsStyle: {
        CString TabStyle = (LPCWSTR) (_bstr_t) Property->GetValue();
        ResetMDIChild = TRUE;

        for (int i = 0; ::TabsStyles[i] != NULL; i++) {
            if (TabStyle == ::TabsStyles[i]) {
                switch (i) {
                case 0:
                    theApp.m_Options.m_nTabsStyle = EoApOptions::None;
                    break;

                case 1:
                    theApp.m_Options.m_nTabsStyle = EoApOptions::Standard;
                    break;

                case 2:
                    theApp.m_Options.m_nTabsStyle = EoApOptions::Grouped;
                    break;
                }
                break;
            }
        }
        SetWorkspaceTabsSubItemsState();
        break;
    }
    case kTabLocation: {
        CString TabLocation = (LPCWSTR) (_bstr_t) Property->GetValue();
        theApp.m_Options.m_MdiTabInfo.m_tabLocation = (TabLocation == TabLocations[0] ? CMFCTabCtrl::LOCATION_BOTTOM : CMFCTabCtrl::LOCATION_TOP);
        break;
    }
	case kTabsAutoColor:
		theApp.m_Options.m_MdiTabInfo.m_bAutoColor = Property->GetValue().boolVal == VARIANT_TRUE;
		break;

	case kTabIcons:
		theApp.m_Options.m_MdiTabInfo.m_bTabIcons = Property->GetValue().boolVal == VARIANT_TRUE;
		break;

    case kTabBorderSize: {
        const int nBorder = Property->GetValue().iVal;
        theApp.m_Options.m_MdiTabInfo.m_nTabBorderSize = min(8, max(0, nBorder));
        break;
    }
    case kActiveViewScale: {
        AeSysView* ActiveView = AeSysView::GetActiveView();
        ActiveView->SetWorldScale(Property->GetValue().dblVal);
        ActiveView->UpdateStateInformation(AeSysView::Scale);
        return LRESULT(0);
    }
	}
	theApp.UpdateMDITabs(ResetMDIChild);

	return LRESULT(0);
}

void EoMfPropertiesDockablePane::OnExpandAllProperties() {
	m_PropertyGrid.ExpandAll();
}
void EoMfPropertiesDockablePane::OnProperties1() {
	ATLTRACE2(atlTraceGeneral, 0, L"EoMfPropertiesDockablePane::OnProperties1\n");
}
void EoMfPropertiesDockablePane::OnSortProperties() {
	m_PropertyGrid.SetAlphabeticMode(!m_PropertyGrid.IsAlphabeticMode());
}
void EoMfPropertiesDockablePane::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */) {
	ATLTRACE2(atlTraceGeneral, 2, L"EoMfPropertiesDockablePane::OnUpdatExpandAllProperties\n");
}
void EoMfPropertiesDockablePane::OnUpdateProperties1(CCmdUI* /* pCmdUI */) {
	ATLTRACE2(atlTraceGeneral, 2, L"EoMfPropertiesDockablePane::OnUpdatProperties1\n");
}
void EoMfPropertiesDockablePane::OnUpdateSortProperties(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_PropertyGrid.IsAlphabeticMode());
}
void EoMfPropertiesDockablePane::AdjustLayout() {
	
	if (GetSafeHwnd() == NULL) { return; }

	CRect rectClient,rectCombo;
	GetClientRect(rectClient);

	m_wndObjectCombo.GetWindowRect(&rectCombo);

	const int cyCmb = rectCombo.Size().cy;
	const int cyTlb = m_PropertiesToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndObjectCombo.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), 200, SWP_NOACTIVATE | SWP_NOZORDER);
	m_PropertiesToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_PropertyGrid.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb + cyTlb, rectClient.Width(), rectClient.Height() -(cyCmb+cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);
}
void EoMfPropertiesDockablePane::InitializePropertyGrid() {
	SetPropertyGridFont();

	m_PropertyGrid.EnableHeaderCtrl(FALSE, L"Property", L"Value");
	m_PropertyGrid.EnableDescriptionArea(TRUE);

	m_PropertyGrid.SetVSDotNetLook(TRUE);
	m_PropertyGrid.SetGroupNameFullWidth(TRUE, TRUE);

	m_PropertyGrid.MarkModifiedProperties(TRUE, TRUE);

	CMFCPropertyGridProperty* WorkspaceTabsGroup = new CMFCPropertyGridProperty(L"Workspace Tabs");

	CMFCPropertyGridProperty* TabsStyle = new CMFCPropertyGridProperty(L"Tabs Style", L"", L"Set the Tabs Style to None, Standard, or Grouped", kTabsStyle, NULL, NULL, NULL);
	TabsStyle->AddOption(::TabsStyles[0], TRUE);
	TabsStyle->AddOption(::TabsStyles[1], TRUE);
	TabsStyle->AddOption(::TabsStyles[2], TRUE);
	TabsStyle->SetValue(::TabsStyles[theApp.m_Options.m_nTabsStyle]);
	TabsStyle->AllowEdit(FALSE);
	WorkspaceTabsGroup->AddSubItem(TabsStyle);

	CMFCPropertyGridProperty* TabLocation = new CMFCPropertyGridProperty(L"Tab Location", L"", L"Set the Tab Location to Top or Bottom", kTabLocation, NULL, NULL, NULL);
	TabLocation->AddOption(::TabLocations[0], TRUE);
	TabLocation->AddOption(::TabLocations[1], TRUE);
	TabLocation->SetValue(::TabLocations[theApp.m_Options.m_MdiTabInfo.m_tabLocation]);
	TabLocation->AllowEdit(FALSE);
	WorkspaceTabsGroup->AddSubItem(TabLocation);

	COleVariant TabsAutoColor((short)(theApp.m_Options.m_MdiTabInfo.m_bAutoColor == TRUE), VT_BOOL);
	WorkspaceTabsGroup->AddSubItem(new CMFCPropertyGridProperty(L"Tabs auto-color", TabsAutoColor, L"Set Workspace Tabs to use automatic color", kTabsAutoColor));

	COleVariant TabIcons((short)(theApp.m_Options.m_MdiTabInfo.m_bTabIcons == TRUE), VT_BOOL);
	WorkspaceTabsGroup->AddSubItem(new CMFCPropertyGridProperty(L"Tab icons", TabIcons, L"Show document icons on Workspace Tabs", kTabIcons));

	COleVariant TabBorderSize((short)(theApp.m_Options.m_MdiTabInfo.m_nTabBorderSize), VT_I2);
	CMFCPropertyGridProperty* BorderSize = new CMFCPropertyGridProperty(L"Border Size", TabBorderSize, L"Set Workspace border size from 0 to 8 pixels", kTabBorderSize);
	BorderSize->EnableSpinControl(TRUE, 0, 8);
	BorderSize->AllowEdit(FALSE);
	WorkspaceTabsGroup->AddSubItem(BorderSize);

	m_PropertyGrid.AddProperty(WorkspaceTabsGroup);

	AeSysView* ActiveView = AeSysView::GetActiveView();

	const double Scale = (ActiveView) ? ActiveView->WorldScale() : 1.;

	CMFCPropertyGridProperty* ActiveViewGroup = new CMFCPropertyGridProperty(L"Active View");
	CMFCPropertyGridProperty* WorldScaleProperty = new CMFCPropertyGridProperty(L"World Scale", (_variant_t) Scale, L"Specifies the world scale used in the Active View", kActiveViewScale);
	ActiveViewGroup->AddSubItem(WorldScaleProperty);
	ActiveViewGroup->AddSubItem(new CMFCPropertyGridProperty(L"Use True Type fonts", (_variant_t) true, L"Specifies that the Active View uses True Type fonts"));
	m_PropertyGrid.AddProperty(ActiveViewGroup);
	WorldScaleProperty->Enable(ActiveView != NULL);


	CMFCPropertyGridProperty* AppearanceGroup = new CMFCPropertyGridProperty(L"Appearance");

	AppearanceGroup->AddSubItem(new CMFCPropertyGridProperty(L"3D Look", (_variant_t) false, L"Specifies the window's font will be non-bold and controls will have a 3D border"));

	CMFCPropertyGridProperty* LengthUnits = new CMFCPropertyGridProperty(L"Length Units", L"Engineering", L"Specifies the units used to display lengths");
	LengthUnits->AddOption(L"Architectural", TRUE);
	LengthUnits->AddOption(L"Engineering", TRUE);
	LengthUnits->AddOption(L"Feet", TRUE);
	LengthUnits->AddOption(L"Inches", TRUE);
	LengthUnits->AddOption(L"Meters", TRUE);
	LengthUnits->AddOption(L"Millimeters", TRUE);
	LengthUnits->AddOption(L"Centimeters", TRUE);
	LengthUnits->AddOption(L"Decimeters", TRUE);
	LengthUnits->AddOption(L"Kilometers", TRUE);
	LengthUnits->AllowEdit(FALSE);
	AppearanceGroup->AddSubItem(LengthUnits);

	CMFCPropertyGridProperty* LengthPrecision = new CMFCPropertyGridProperty(L"Length Precision", (_variant_t) 8l, L"Specifies the precision used to display lengths");
	LengthPrecision->EnableSpinControl(TRUE, 0, 256);
	AppearanceGroup->AddSubItem(LengthPrecision);

	AppearanceGroup->AddSubItem(new CMFCPropertyGridProperty(L"Caption", (_variant_t) L"About", L"Specifies the text that will be displayed in the window's title bar"));

	m_PropertyGrid.AddProperty(AppearanceGroup);

	CMFCPropertyGridProperty* PointGrid = new CMFCPropertyGridProperty(L"Point Grid", 0, TRUE);

	CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(L"X", (_variant_t) 3., L"Specifies the point grid x spacing");
	PointGrid->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( L"Y", (_variant_t) 3., L"Specifies the point grid y spacing");
	PointGrid->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( L"Z", (_variant_t) 0., L"Specifies the point grid z spacing");
	PointGrid->AddSubItem(pProp);

	m_PropertyGrid.AddProperty(PointGrid);

	CMFCPropertyGridProperty* NoteGroup = new CMFCPropertyGridProperty(L"Note");

	LOGFONT lf;
	CFont* font = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	font->GetLogFont(&lf);

	lstrcpy(lf.lfFaceName, L"Arial");

	NoteGroup->AddSubItem(new CMFCPropertyGridFontProperty(L"Font", lf, CF_EFFECTS | CF_SCREENFONTS, L"Specifies the default font for the window"));
	NoteGroup->AddSubItem(new CMFCPropertyGridProperty(L"Use System Font", (_variant_t) true, L"Specifies that the window uses MS Shell Dlg font"));

	CMFCPropertyGridProperty* HorizontalAlignment = new CMFCPropertyGridProperty(L"Horizontal Alignment", L"Left", L"Specifies the horizontal alignment used for new notes");
	HorizontalAlignment->AddOption(L"Left", TRUE);
	HorizontalAlignment->AddOption(L"Center", TRUE);
	HorizontalAlignment->AddOption(L"Right", TRUE);
	HorizontalAlignment->AllowEdit(FALSE);
	NoteGroup->AddSubItem(HorizontalAlignment);

	CMFCPropertyGridProperty* VerticalAlignment = new CMFCPropertyGridProperty(L"Vertical Alignment", L"Bottom", L"Specifies the vertical alignment used for new notes");
	VerticalAlignment->AddOption(L"Bottom", TRUE);
	VerticalAlignment->AddOption(L"Middle", TRUE);
	VerticalAlignment->AddOption(L"Top", TRUE);
	HorizontalAlignment->AllowEdit(FALSE);
	NoteGroup->AddSubItem(VerticalAlignment);

	CMFCPropertyGridProperty* Path = new CMFCPropertyGridProperty(L"Path", L"Right", L"Specifies the text path used for new notes");
	Path->AddOption(L"Right", TRUE);
	Path->AddOption(L"Left", TRUE);
	Path->AddOption(L"Up", TRUE);
	Path->AddOption(L"Down", TRUE);
	Path->AllowEdit(FALSE);
	NoteGroup->AddSubItem(Path);

	m_PropertyGrid.AddProperty(NoteGroup);

	CMFCPropertyGridProperty* MiscGroup = new CMFCPropertyGridProperty(L"Misc");
	pProp = new CMFCPropertyGridProperty(L"(Name)", L"Application");
	pProp->Enable(FALSE);
	MiscGroup->AddSubItem(pProp);

	CMFCPropertyGridColorProperty* ColorProperty = new CMFCPropertyGridColorProperty(L"Window Color", RGB(210, 192, 254), NULL, L"Specifies the default window color");
	ColorProperty->EnableOtherButton(L"Other...");
	ColorProperty->EnableAutomaticButton(L"Default", ::GetSysColor(COLOR_3DFACE));
	MiscGroup->AddSubItem(ColorProperty);

	static wchar_t BASED_CODE szFilter[] = L"Icon Files(*.ico)|*.ico|All Files(*.*)|*.*||";
	MiscGroup->AddSubItem(new CMFCPropertyGridFileProperty(L"Icon", TRUE, L"", L"ico", 0, szFilter, L"Specifies the window icon"));

	MiscGroup->AddSubItem(new CMFCPropertyGridFileProperty(L"Shadow Folder Path", theApp.ShadowFolderPath(), 0, NULL));

	m_PropertyGrid.AddProperty(MiscGroup);
}
void EoMfPropertiesDockablePane::SetPropertyGridFont() {
	::DeleteObject(m_PropertyGridFont.Detach());

	LOGFONT LogFont;
	afxGlobalData.fontRegular.GetLogFont(&LogFont);

	NONCLIENTMETRICS Info;
	Info.cbSize = sizeof(Info);

	afxGlobalData.GetNonClientMetrics(Info);

	LogFont.lfHeight = Info.lfMenuFont.lfHeight;
	LogFont.lfWeight = Info.lfMenuFont.lfWeight;
	LogFont.lfItalic = Info.lfMenuFont.lfItalic;

	m_PropertyGridFont.CreateFontIndirect(&LogFont);

	m_PropertyGrid.SetFont(&m_PropertyGridFont);
}
void EoMfPropertiesDockablePane::SetWorkspaceTabsSubItemsState() {
	for (int i = 0; i < m_PropertyGrid.GetPropertyCount(); i++) {
		CMFCPropertyGridProperty* Property = m_PropertyGrid.GetProperty(i);
		ASSERT_VALID(Property);

		if (wcscmp(Property->GetName(), L"Workspace Tabs") == 0) {
			for (int SubItemIndex = 1; SubItemIndex < Property->GetSubItemsCount(); SubItemIndex++) {
				CMFCPropertyGridProperty* SubProperty = Property->GetSubItem(SubItemIndex);

				ASSERT_VALID(SubProperty);
				SubProperty->Enable(theApp.m_Options.m_nTabsStyle != EoApOptions::None);
			}
		}
	}
	if (m_PropertyGrid.GetSafeHwnd() != NULL) {
		m_PropertyGrid.RedrawWindow();
	}
}
