#pragma once

// EoDlgSelectGotHomePoint dialog

class EoDlgSelectGotoHomePoint : public CDialog {
    DECLARE_DYNAMIC(EoDlgSelectGotoHomePoint)

public:
    EoDlgSelectGotoHomePoint(CWnd* parent = nullptr);
    EoDlgSelectGotoHomePoint(AeSysView* currentView, CWnd* parent = nullptr);
    virtual ~EoDlgSelectGotoHomePoint();

    // Dialog Data
    enum { IDD = IDD_HOME_POINT_GO };

protected:
    void DoDataExchange(CDataExchange* pDX) final;
    BOOL OnInitDialog() final;
    void OnOK() final;

    AeSysView* m_ActiveView;
public:
    CComboBox m_HomePointNames;
    CEdit m_X;
    CEdit m_Y;
    CEdit m_Z;

    void OnCbnEditupdateList();
    void OnCbnSelchangeList();

protected:
    DECLARE_MESSAGE_MAP()
};


