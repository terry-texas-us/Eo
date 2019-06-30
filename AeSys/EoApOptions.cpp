#include "stdafx.h"
#include "AeSys.h"

EoApOptions::EoApOptions() {
	tabsStyle = kGrouped;
	mdiTabInfo.m_tabLocation = CMFCTabCtrl::LOCATION_BOTTOM;
	mdiTabInfo.m_style = CMFCTabCtrl::STYLE_3D_VS2005;
	mdiTabInfo.m_bTabIcons = TRUE;
	mdiTabInfo.m_bTabCloseButton = FALSE;
	mdiTabInfo.m_bTabCustomTooltips = TRUE;
	mdiTabInfo.m_bAutoColor = FALSE;
	mdiTabInfo.m_bDocumentMenu = TRUE;
	mdiTabInfo.m_bEnableTabSwap = TRUE;
	mdiTabInfo.m_bFlatFrame = TRUE;
	mdiTabInfo.m_bActiveTabCloseButton = TRUE;
	mdiTabInfo.m_nTabBorderSize = 1;
	tabsContextMenu = true;
	disableSetRedraw = true;
}

void EoApOptions::Load() {
	tabsStyle = static_cast<TabsStyle>(theApp.GetInt(L"TabsStyle", kGrouped));
	mdiTabInfo.m_tabLocation = static_cast<CMFCTabCtrl::Location>(theApp.GetInt(L"TabLocation", CMFCTabCtrl::LOCATION_BOTTOM));
	mdiTabInfo.m_style = static_cast<CMFCTabCtrl::Style>(theApp.GetInt(L"TabsAppearance", CMFCTabCtrl::STYLE_3D_VS2005));
	mdiTabInfo.m_bTabIcons = theApp.GetInt(L"TabIcons", TRUE);
	mdiTabInfo.m_bTabCloseButton = theApp.GetInt(L"TabCloseButton", FALSE);
	mdiTabInfo.m_bTabCustomTooltips = theApp.GetInt(L"CustomTooltips", TRUE);
	mdiTabInfo.m_bAutoColor = theApp.GetInt(L"AutoColor", FALSE);
	mdiTabInfo.m_bDocumentMenu = theApp.GetInt(L"DocumentMenu", TRUE);
	mdiTabInfo.m_bEnableTabSwap = theApp.GetInt(L"EnableTabSwap", TRUE);
	mdiTabInfo.m_bFlatFrame = theApp.GetInt(L"FlatFrame", TRUE);
	mdiTabInfo.m_bActiveTabCloseButton = theApp.GetInt(L"ActiveTabCloseButton", TRUE);
	mdiTabInfo.m_nTabBorderSize = theApp.GetInt(L"TabBorderSize", 1);
	tabsContextMenu = theApp.GetInt(L"TabsContextMenu", true);
	disableSetRedraw = theApp.GetInt(L"DisableSetRedraw", true);
}

void EoApOptions::Save() {
	theApp.WriteInt(L"TabsStyle", tabsStyle);
	theApp.WriteInt(L"TabLocation", mdiTabInfo.m_tabLocation);
	theApp.WriteInt(L"TabsAppearance", mdiTabInfo.m_style);
	theApp.WriteInt(L"TabIcons", mdiTabInfo.m_bTabIcons);
	theApp.WriteInt(L"TabCloseButton", mdiTabInfo.m_bTabCloseButton);
	theApp.WriteInt(L"CustomTooltips", mdiTabInfo.m_bTabCustomTooltips);
	theApp.WriteInt(L"AutoColor", mdiTabInfo.m_bAutoColor);
	theApp.WriteInt(L"DocumentMenu", mdiTabInfo.m_bDocumentMenu);
	theApp.WriteInt(L"EnableTabSwap", mdiTabInfo.m_bEnableTabSwap);
	theApp.WriteInt(L"FlatFrame", mdiTabInfo.m_bFlatFrame);
	theApp.WriteInt(L"ActiveTabCloseButton", mdiTabInfo.m_bActiveTabCloseButton);
	theApp.WriteInt(L"TabBorderSize", mdiTabInfo.m_nTabBorderSize);
	theApp.WriteInt(L"TabsContextMenu", tabsContextMenu);
	theApp.WriteInt(L"DisableSetRedraw", disableSetRedraw);
}
