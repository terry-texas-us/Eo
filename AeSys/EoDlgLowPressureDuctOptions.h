#pragma once

// EoDlgLowPressureDuctOptions dialog

class EoDlgLowPressureDuctOptions : public CDialog {
    DECLARE_DYNAMIC(EoDlgLowPressureDuctOptions)

public:
    EoDlgLowPressureDuctOptions(CWnd* parent = nullptr);
    virtual ~EoDlgLowPressureDuctOptions();

    // Dialog Data
    enum { IDD = IDD_DLGPROC_LPD_OPTIONS };

protected:
    void DoDataExchange(CDataExchange* pDX) final;
    BOOL OnInitDialog() final;

public:
    double m_Width;
    double m_Depth;
    double m_RadiusFactor;
    bool m_GenerateVanes;
    int m_Justification;
    bool m_BeginWithTransition;

    void OnBnClickedOk();
    void OnBnClickedGenVanes() noexcept;
    void OnEnChangeWidth();

protected:
    DECLARE_MESSAGE_MAP()
};
