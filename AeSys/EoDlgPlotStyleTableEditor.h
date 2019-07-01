#pragma once
#include "EoDlgPlotStyleTableEditor_FormViewPropertyPage.h"
#include "EoDlgPlotStyleTableEditor_GeneralPropertyPage.h"
class OdPsPlotStyleTable;

class EoDlgPlotStyleManager final : public CPropertySheet {
DECLARE_DYNCREATE(EoDlgPlotStyleManager)

private:
	EoDlgPlotStyleEditor_GeneralPropertyPage m_page1;
	EoDlgPlotStyleEditor_FormViewPropertyPage m_page2;
public:
	OdPsPlotStyleTable* plotStyleTable {nullptr};
	OdPsPlotStyleTablePtr plotStyleTableForPropertyPage;

	EoDlgPlotStyleManager(CWnd* parent = nullptr);

	bool SetPlotStyleTable(OdPsPlotStyleTable* plotStyleTable);

	void SetFileBufPath(const OdString& filePath);

	OdPsPlotStyleTablePtr GetPlotStyleTable() const;

	// Operations
	enum { IDD = IDD_PLOTSTYLE_MANAGER };

protected:
	int OnCreate(LPCREATESTRUCT createStructure); // hides non-virtual function of parent
	BOOL OnInitDialog() final;

DECLARE_MESSAGE_MAP()
};
