#pragma once

// EoDlgAnnotateOptions dialog

class EoDlgAnnotateOptions : public CDialog {
    DECLARE_DYNAMIC(EoDlgAnnotateOptions)

public:
    EoDlgAnnotateOptions(CWnd* parent = NULL);
    EoDlgAnnotateOptions(AeSysView* view, CWnd* parent = NULL);
    
    ~EoDlgAnnotateOptions();

    // Dialog Data
    enum { IDD = IDD_ANNOTATE_OPTIONS };

protected:
    void DoDataExchange(CDataExchange* pDX) final;
    BOOL OnInitDialog() final;
    void OnOK() final;

    AeSysView* m_ActiveView;
public:
    CComboBox m_EndItemTypeComboBox;

    double m_GapSpaceFactor;
    double m_CircleRadius;
    double m_EndItemSize;
    double m_BubbleRadius;
    int m_NumberOfSides;
    CString m_DefaultText;

protected:
    DECLARE_MESSAGE_MAP()
};


