#pragma once

// EoDlgSetHomePoint dialog
class EoDlgSetHomePoint : public CDialog {
DECLARE_DYNAMIC(EoDlgSetHomePoint)
	EoDlgSetHomePoint(CWnd* parent = nullptr);
	EoDlgSetHomePoint(AeSysView* activeView, CWnd* parent = nullptr);
	virtual ~EoDlgSetHomePoint();

	// Dialog Data
	enum { IDD = IDD_HOME_POINT_EDIT };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
	AeSysView* m_ActiveView;
	static OdGePoint3d m_CursorPosition;
public:
	CComboBox m_HomePointNames;
	CEdit m_X;
	CEdit m_Y;
	CEdit m_Z;
	void OnCbnEditupdateList();
protected:
DECLARE_MESSAGE_MAP()
};
