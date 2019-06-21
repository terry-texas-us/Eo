#include "stdafx.h"
#include "AeSys.h"

EoApOptions::EoApOptions() {
	m_nTabsStyle = Grouped;
	m_MdiTabInfo.m_tabLocation = CMFCTabCtrl::LOCATION_BOTTOM;
	m_MdiTabInfo.m_style = CMFCTabCtrl::STYLE_3D_VS2005;
	m_MdiTabInfo.m_bTabIcons = TRUE;
	m_MdiTabInfo.m_bTabCloseButton = FALSE;
	m_MdiTabInfo.m_bTabCustomTooltips = TRUE;
	m_MdiTabInfo.m_bAutoColor = FALSE;
	m_MdiTabInfo.m_bDocumentMenu = TRUE;
	m_MdiTabInfo.m_bEnableTabSwap = TRUE;
	m_MdiTabInfo.m_bFlatFrame = TRUE;
	m_MdiTabInfo.m_bActiveTabCloseButton = TRUE;
	m_MdiTabInfo.m_nTabBorderSize = 1;
	m_TabsContextMenu = true;
	m_DisableSetRedraw = true;
}

void EoApOptions::Load() {
	m_nTabsStyle = static_cast<TabsStyle>(theApp.GetInt(L"TabsStyle", Grouped));
	m_MdiTabInfo.m_tabLocation = static_cast<CMFCTabCtrl::Location>(theApp.GetInt(L"TabLocation", CMFCTabCtrl::LOCATION_BOTTOM));
	m_MdiTabInfo.m_style = static_cast<CMFCTabCtrl::Style>(theApp.GetInt(L"TabsAppearance", CMFCTabCtrl::STYLE_3D_VS2005));
	m_MdiTabInfo.m_bTabIcons = theApp.GetInt(L"TabIcons", TRUE);
	m_MdiTabInfo.m_bTabCloseButton = theApp.GetInt(L"TabCloseButton", FALSE);
	m_MdiTabInfo.m_bTabCustomTooltips = theApp.GetInt(L"CustomTooltips", TRUE);
	m_MdiTabInfo.m_bAutoColor = theApp.GetInt(L"AutoColor", FALSE);
	m_MdiTabInfo.m_bDocumentMenu = theApp.GetInt(L"DocumentMenu", TRUE);
	m_MdiTabInfo.m_bEnableTabSwap = theApp.GetInt(L"EnableTabSwap", TRUE);
	m_MdiTabInfo.m_bFlatFrame = theApp.GetInt(L"FlatFrame", TRUE);
	m_MdiTabInfo.m_bActiveTabCloseButton = theApp.GetInt(L"ActiveTabCloseButton", TRUE);
	m_MdiTabInfo.m_nTabBorderSize = theApp.GetInt(L"TabBorderSize", 1);
	m_TabsContextMenu = theApp.GetInt(L"TabsContextMenu", true);
	m_DisableSetRedraw = theApp.GetInt(L"DisableSetRedraw", true);
}

void EoApOptions::Save() {
	theApp.WriteInt(L"TabsStyle", m_nTabsStyle);
	theApp.WriteInt(L"TabLocation", m_MdiTabInfo.m_tabLocation);
	theApp.WriteInt(L"TabsAppearance", m_MdiTabInfo.m_style);
	theApp.WriteInt(L"TabIcons", m_MdiTabInfo.m_bTabIcons);
	theApp.WriteInt(L"TabCloseButton", m_MdiTabInfo.m_bTabCloseButton);
	theApp.WriteInt(L"CustomTooltips", m_MdiTabInfo.m_bTabCustomTooltips);
	theApp.WriteInt(L"AutoColor", m_MdiTabInfo.m_bAutoColor);
	theApp.WriteInt(L"DocumentMenu", m_MdiTabInfo.m_bDocumentMenu);
	theApp.WriteInt(L"EnableTabSwap", m_MdiTabInfo.m_bEnableTabSwap);
	theApp.WriteInt(L"FlatFrame", m_MdiTabInfo.m_bFlatFrame);
	theApp.WriteInt(L"ActiveTabCloseButton", m_MdiTabInfo.m_bActiveTabCloseButton);
	theApp.WriteInt(L"TabBorderSize", m_MdiTabInfo.m_nTabBorderSize);
	theApp.WriteInt(L"TabsContextMenu", m_TabsContextMenu);
	theApp.WriteInt(L"DisableSetRedraw", m_DisableSetRedraw);
}
