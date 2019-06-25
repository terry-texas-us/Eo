#pragma once

class EoDlgSetupConstraints final : public CDialog {
DECLARE_DYNAMIC(EoDlgSetupConstraints)
	EoDlgSetupConstraints(CWnd* parent = nullptr);
	EoDlgSetupConstraints(AeSysView* view, CWnd* parent = nullptr);
	virtual ~EoDlgSetupConstraints();

	enum { IDD = IDD_SETUP_CONSTRAINTS_GRID };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
	AeSysView* m_ActiveView {nullptr};
	CEdit m_GridXSnapSpacing;
	CEdit m_GridYSnapSpacing;
	CEdit m_GridZSnapSpacing;
	CEdit m_GridXPointSpacing;
	CEdit m_GridYPointSpacing;
	CEdit m_GridZPointSpacing;
	CEdit m_GridXLineSpacing;
	CEdit m_GridYLineSpacing;
	CEdit m_GridZLineSpacing;
	CButton m_GridSnapEnableButton;
	CButton m_GridDisplayButton;
	CButton m_GridLineDisplayButton;
	CEdit m_AxisInfluenceAngle;
	CEdit m_AxisZOffsetAngle;
DECLARE_MESSAGE_MAP()
};
