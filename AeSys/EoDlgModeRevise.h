#pragma once

// EoDlgModeRevise dialog

class EoDlgModeRevise : public CDialog {
    DECLARE_DYNAMIC(EoDlgModeRevise)

public:
    EoDlgModeRevise(CWnd* parent = NULL);
    virtual ~EoDlgModeRevise();

    // Dialog Data
    enum { IDD = IDD_ADD_NOTE };

protected:
    void DoDataExchange(CDataExchange* pDX) final;
    BOOL OnInitDialog() final;
    void OnOK() final;

    static EoDbFontDefinition sm_FontDefinition;
    static EoGeReferenceSystem sm_ReferenceSystem;
    static EoDbText* sm_TextPrimitive;

public:
    CEdit m_TextEditControl;
    /// <summary> Effectively resizes the edit control to use the entire client area of the dialog.</summary>
    /// <remarks> OnSize can be called before OnInitialUpdate so check is made for valid control window.</remarks>
    void OnSize(UINT nType, int cx, int cy);

protected:
    DECLARE_MESSAGE_MAP()
};
