#pragma once

// EoDlgSetHomePoint dialog

class EoDlgSetHomePoint : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetHomePoint)

public:
	EoDlgSetHomePoint(CWnd* parent = NULL);
	EoDlgSetHomePoint(AeSysView* activeView, CWnd* parent = NULL);
	virtual ~EoDlgSetHomePoint();

// Dialog Data
	enum { IDD = IDD_HOME_POINT_EDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override;

	AeSysView* m_ActiveView;
	static OdGePoint3d m_CursorPosition;
public:
	CComboBox m_HomePointNames;
	CEdit m_X;
	CEdit m_Y;
	CEdit m_Z;

	afx_msg void OnCbnEditupdateList();

protected:
	DECLARE_MESSAGE_MAP()
};
