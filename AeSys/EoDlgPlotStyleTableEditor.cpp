#include "stdafx.h"

#include "EoDlgPlotStyleTableEditor.h"
#include "EoDlgPlotStyleTableEditor_FormViewPropertyPage.h"
#include "EoDlgPlotStyleTableEditor_GeneralPropertyPage.h"

IMPLEMENT_DYNAMIC(EoDlgPlotStyleManager, CPropertySheet)

EoDlgPlotStyleManager::EoDlgPlotStyleManager(CWnd* parent)
	: CPropertySheet(L"Plot Style Table Editor", parent)
	, m_pPlotStyleTable(0) {
	m_psh.dwFlags |= PSH_NOAPPLYNOW;

	AddPage(&m_page1);
	AddPage(&m_page2);
}

EoDlgPlotStyleManager::~EoDlgPlotStyleManager() {
}

BEGIN_MESSAGE_MAP(EoDlgPlotStyleManager, CPropertySheet)
	ON_WM_CREATE()
END_MESSAGE_MAP()

int EoDlgPlotStyleManager::OnCreate(LPCREATESTRUCT createStructure) {
	if (CPropertySheet::OnCreate(createStructure) == -1) {
		return -1;
	}
	return 0;
}

BOOL EoDlgPlotStyleManager::OnInitDialog() {
	CPropertySheet::OnInitDialog();

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


const bool EoDlgPlotStyleManager::SetPlotStyleTable(OdPsPlotStyleTable* pPlotStyleTable) {
	if (!pPlotStyleTable) {
		return false;
	}
	m_pPlotStyleTable = pPlotStyleTable;

	m_pPsTabForPropertyPg = pPlotStyleTable->clone();
	OdPsPlotStyleTable* pPsTab = m_pPsTabForPropertyPg.get();

	return m_page1.SetPlotStyleTable(pPsTab) && m_page2.SetPlotStyleTable(pPsTab);
}

void EoDlgPlotStyleManager::SetFileBufPath(const OdString sFilePath) {
	m_page1.SetFileBufPath(sFilePath);
	m_page2.SetFileBufPath(sFilePath);
}

OdPsPlotStyleTablePtr EoDlgPlotStyleManager::GetPlotStyleTable() const {
	return m_pPsTabForPropertyPg;
}
