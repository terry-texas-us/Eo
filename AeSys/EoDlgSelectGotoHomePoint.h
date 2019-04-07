#pragma once

// EoDlgSelectGotHomePoint dialog

class EoDlgSelectGotoHomePoint : public CDialog {
	DECLARE_DYNAMIC(EoDlgSelectGotoHomePoint)

public:
	EoDlgSelectGotoHomePoint(CWnd* parent = NULL);
	EoDlgSelectGotoHomePoint(AeSysView* currentView, CWnd* parent = NULL);
	virtual ~EoDlgSelectGotoHomePoint();

// Dialog Data
	enum { IDD = IDD_HOME_POINT_GO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override;

	AeSysView* m_ActiveView;
public:
	CComboBox m_HomePointNames;
	CEdit m_X;
	CEdit m_Y;
	CEdit m_Z;

	afx_msg void OnCbnEditupdateList();
	afx_msg void OnCbnSelchangeList();

protected:
	DECLARE_MESSAGE_MAP()
};


