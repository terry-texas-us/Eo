#pragma once
class EoDlgSelectGotoHomePoint final : public CDialog {
DECLARE_DYNAMIC(EoDlgSelectGotoHomePoint)

	EoDlgSelectGotoHomePoint(CWnd* parent = nullptr);

	EoDlgSelectGotoHomePoint(AeSysView* activeView, CWnd* parent = nullptr);

	enum { IDD = IDD_HOME_POINT_GO };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;

	BOOL OnInitDialog() final;

	void OnOK() final;

	AeSysView* m_ActiveView;
	CComboBox m_HomePointNames;
	CEdit m_X;
	CEdit m_Y;
	CEdit m_Z;

	void OnCbnEditupdateList();

	void OnCbnSelchangeList();

protected:
DECLARE_MESSAGE_MAP()
};
