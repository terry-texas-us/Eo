#pragma once
#include "EoDlgPlotStyleTableEditor_FormViewPropertyPage.h"
#include "EoDlgPlotStyleTableEditor_GeneralPropertyPage.h"

class OdPsPlotStyleTable;

class EoDlgPlotStyleManager : public CPropertySheet {
	DECLARE_DYNCREATE(EoDlgPlotStyleManager)

	OdPsPlotStyleTable* m_pPlotStyleTable;
	OdPsPlotStyleTablePtr m_pPsTabForPropertyPg;

public: // Construction
	EoDlgPlotStyleManager(CWnd* pParentWnd = NULL);
	const bool SetPlotStyleTable(OdPsPlotStyleTable* pPlotStyleTable);
	void SetFileBufPath(const OdString sFilePath);
	OdPsPlotStyleTablePtr GetPlotStyleTable() const;

public: // Attributes
	EoDlgPlotStyleEditor_GeneralPropertyPage m_page1;
	EoDlgPlotStyleEditor_FormViewPropertyPage m_page2;

public: // Operations

	enum { IDD = IDD_PLOTSTYLE_MANAGER };

public:
	virtual ~ EoDlgPlotStyleManager();
protected:
	afx_msg int OnCreate(LPCREATESTRUCT createStructure);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
