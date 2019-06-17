#pragma once
#include "EoDlgPlotStyleTableEditor_FormViewPropertyPage.h"
#include "EoDlgPlotStyleTableEditor_GeneralPropertyPage.h"

class OdPsPlotStyleTable;

class EoDlgPlotStyleManager : public CPropertySheet {
    DECLARE_DYNCREATE(EoDlgPlotStyleManager)

    OdPsPlotStyleTable* m_pPlotStyleTable;
    OdPsPlotStyleTablePtr m_pPsTabForPropertyPg;

public:
    EoDlgPlotStyleManager(CWnd* pParentWnd = nullptr);
    bool SetPlotStyleTable(OdPsPlotStyleTable* pPlotStyleTable);
    void SetFileBufPath(const OdString sFilePath);
    OdPsPlotStyleTablePtr GetPlotStyleTable() const;

public: // Attributes
    EoDlgPlotStyleEditor_GeneralPropertyPage m_page1;
    EoDlgPlotStyleEditor_FormViewPropertyPage m_page2;

public: // Operations

    enum { IDD = IDD_PLOTSTYLE_MANAGER };

public:
    virtual ~EoDlgPlotStyleManager();
protected:
    int OnCreate(LPCREATESTRUCT createStructure);
    BOOL OnInitDialog() final;

    DECLARE_MESSAGE_MAP()
};
