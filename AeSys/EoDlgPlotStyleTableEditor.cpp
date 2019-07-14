#include "stdafx.h"
#include "EoDlgPlotStyleTableEditor.h"
#include "EoDlgPlotStyleTableEditor_FormViewPropertyPage.h"
#include "EoDlgPlotStyleTableEditor_GeneralPropertyPage.h"
IMPLEMENT_DYNAMIC(EoDlgPlotStyleManager, CPropertySheet)

EoDlgPlotStyleManager::EoDlgPlotStyleManager(CWnd* parent)
	: CPropertySheet(L"Plot Style Table Editor", parent) {
	m_psh.dwFlags |= PSH_NOAPPLYNOW;
	AddPage(&m_page1);
	AddPage(&m_page2);
}
#pragma warning(push)
#pragma warning(disable : 4191) // (level 3) 'operator': unsafe conversion from 'type_of_expression' to 'type_required'
BEGIN_MESSAGE_MAP(EoDlgPlotStyleManager, CPropertySheet)
		ON_WM_CREATE()
END_MESSAGE_MAP()
#pragma warning (pop)
int EoDlgPlotStyleManager::OnCreate(const LPCREATESTRUCT createStructure) {
	if (CPropertySheet::OnCreate(createStructure) == -1) {
		return -1;
	}
	return 0;
}

BOOL EoDlgPlotStyleManager::OnInitDialog() {
	CPropertySheet::OnInitDialog();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

bool EoDlgPlotStyleManager::SetPlotStyleTable(OdPsPlotStyleTable* plotStyleTable) {
	if (plotStyleTable == nullptr) {
		return false;
	}
	EoDlgPlotStyleManager::plotStyleTable = plotStyleTable;
	plotStyleTableForPropertyPage = plotStyleTable->clone();
	const auto PlotStyleTable {plotStyleTableForPropertyPage.get()};
	return m_page1.SetPlotStyleTable(PlotStyleTable) && m_page2.SetPlotStyleTable(PlotStyleTable);
}

void EoDlgPlotStyleManager::SetFileBufPath(const OdString& filePath) {
	m_page1.SetFileBufPath(filePath);
	m_page2.SetFileBufPath(filePath);
}

OdPsPlotStyleTablePtr EoDlgPlotStyleManager::GetPlotStyleTable() const {
	return plotStyleTableForPropertyPage;
}
