#pragma once

// EoDlgEditOptions dialog

class EoDlgEditOptions : public CDialog {
    DECLARE_DYNAMIC(EoDlgEditOptions)

public:
    EoDlgEditOptions(CWnd* parent = nullptr);
    EoDlgEditOptions(AeSysView* view, CWnd* parent = nullptr);
    virtual ~EoDlgEditOptions();

    // Dialog Data
    enum { IDD = IDD_EDIT_OPTIONS };

protected:
    void DoDataExchange(CDataExchange* pDX) final;
    BOOL OnInitDialog() final;
    void OnOK() final;

    AeSysView* m_ActiveView;

public:
    CEdit m_RotationXEditControl;
    CEdit m_RotationYEditControl;
    CEdit m_RotationZEditControl;
    CEdit m_SizingXEditControl;
    CEdit m_SizingYEditControl;
    CEdit m_SizingZEditControl;
    CButton m_MirrorXButton;
    CButton m_MirrorYButton;
    CButton m_MirrorZButton;

    void OnEditOpRotation();
    void OnEditOpMirroring();
    void OnEditOpSizing();
    void OnBnClickedEditOpMirX();
    void OnBnClickedEditOpMirY();
    void OnBnClickedEditOpMirZ();

protected:
    DECLARE_MESSAGE_MAP()
public:
    double m_ScaleFactorX;
    double m_ScaleFactorY;
    double m_ScaleFactorZ;
    double m_EditModeRotationAngleX;
    double m_EditModeRotationAngleY;
    double m_EditModeRotationAngleZ;
};
