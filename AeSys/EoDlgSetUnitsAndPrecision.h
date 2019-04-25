#pragma once

// EoDlgSetUnitsAndPrecision dialog

class EoDlgSetUnitsAndPrecision : public CDialog {
    DECLARE_DYNAMIC(EoDlgSetUnitsAndPrecision)

public:
    EoDlgSetUnitsAndPrecision(CWnd* parent = NULL);
    virtual ~EoDlgSetUnitsAndPrecision();

    // Dialog Data
    enum { IDD = IDD_UNITS };

protected:
    void DoDataExchange(CDataExchange* pDX) final;
    BOOL OnInitDialog() final;
    void OnOK() final;
public:
    CListBox m_MetricUnitsListBoxControl;
    AeSysApp::Units m_Units;
    int m_Precision;

    void OnBnClickedMetric();

protected:
    DECLARE_MESSAGE_MAP()
};


