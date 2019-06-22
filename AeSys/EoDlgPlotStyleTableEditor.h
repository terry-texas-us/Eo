#pragma once
#include "EoDlgPlotStyleTableEditor_FormViewPropertyPage.h"
#include "EoDlgPlotStyleTableEditor_GeneralPropertyPage.h"
class OdPsPlotStyleTable;

class EoDlgPlotStyleManager final : public CPropertySheet {
DECLARE_DYNCREATE(EoDlgPlotStyleManager)
	OdPsPlotStyleTable* m_pPlotStyleTable;
	OdPsPlotStyleTablePtr m_pPsTabForPropertyPg;
	EoDlgPlotStyleManager(CWnd* parent = nullptr);
	bool SetPlotStyleTable(OdPsPlotStyleTable* pPlotStyleTable);
	void SetFileBufPath(OdString sFilePath);
	[[nodiscard]] OdPsPlotStyleTablePtr GetPlotStyleTable() const;

	// Attributes
	EoDlgPlotStyleEditor_GeneralPropertyPage m_page1;
	EoDlgPlotStyleEditor_FormViewPropertyPage m_page2;

	// Operations
	enum { IDD = IDD_PLOTSTYLE_MANAGER };

	virtual ~EoDlgPlotStyleManager();
protected:
	int OnCreate(LPCREATESTRUCT createStructure);
	BOOL OnInitDialog() final;
DECLARE_MESSAGE_MAP()
};
