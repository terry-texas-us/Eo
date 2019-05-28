#pragma once

#include "Ps/PlotStyles.h"

struct EoLineweightData {
    int m_OldIdx;
    int m_NewIdx;
    double m_Value;
    EoLineweightData() : m_OldIdx(-1), m_NewIdx(-1), m_Value(0.0) {};
};

class EoDlgPlotStyleEditLineweight : public CDialog {
    DECLARE_DYNAMIC(EoDlgPlotStyleEditLineweight)

    OdPsPlotStyleTable* m_PlotStyleTable;
    CImageList m_ListCtrlImages;
    size_t m_InitialSelection;
    EoLineweightData* m_LineweightData;

public:
    EoDlgPlotStyleEditLineweight(CWnd* parent = NULL);
    virtual ~EoDlgPlotStyleEditLineweight() {}

    enum { IDD = IDD_PLOTSTYLE_LINEWEIGHT };

    CListCtrl m_LineweightsListCtrl;
    CButton m_MillimetrsButton;
    CButton m_InchesButton;

protected:
    BOOL DestroyWindow() final;
    void DoDataExchange(CDataExchange* pDX) final;
    BOOL OnInitDialog() final;
    void OnOK() final;

protected:
    void InitializeListCtrl();
    const int InsertLineweightAt(int index, const CString& lineweight, const bool isUse);
    void InitializeLineweightsListCtrlImages();

public:
    const bool SetPlotStyleTable(OdPsPlotStyleTable* plotStyleTable) noexcept;
    void SetUnitIntoList(const bool isInchUnits);
    void SetInitialSelection(int selection) noexcept;

protected:
    void OnRadioMillimetrs();
    void OnRadioInches();
    void OnButtonEditlineweight();
    void OnButtonSortlineweight();
    void OnEndlabeleditListLineweight(NMHDR* pNMHDR, LRESULT* result);

    DECLARE_MESSAGE_MAP()
};
