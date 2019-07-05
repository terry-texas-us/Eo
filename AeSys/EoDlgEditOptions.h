#pragma once
class EoDlgEditOptions final : public CDialog {
DECLARE_DYNAMIC(EoDlgEditOptions)

	EoDlgEditOptions(CWnd* parent = nullptr);

	EoDlgEditOptions(AeSysView* view, CWnd* parent = nullptr);

	virtual ~EoDlgEditOptions();

	enum { IDD = IDD_EDIT_OPTIONS };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;

	BOOL OnInitDialog() final;

	void OnOK() final;

	AeSysView* m_ActiveView {nullptr};
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

DECLARE_MESSAGE_MAP()

public:
	double m_ScaleFactorX {0.0};
	double m_ScaleFactorY {0.0};
	double m_ScaleFactorZ {0.0};
	double m_EditModeRotationAngleX {0.0};
	double m_EditModeRotationAngleY {0.0};
	double m_EditModeRotationAngleZ {0.0};
};
