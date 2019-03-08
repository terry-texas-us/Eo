#pragma once

// EoDlgEditOptions dialog

class EoDlgEditOptions : public CDialog {
	DECLARE_DYNAMIC(EoDlgEditOptions)

public:
	EoDlgEditOptions(CWnd* pParent = NULL);
	EoDlgEditOptions(AeSysView* view, CWnd* pParent = NULL);
	virtual ~EoDlgEditOptions();

// Dialog Data
	enum { IDD = IDD_EDIT_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

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

	afx_msg void OnEditOpRotation();
	afx_msg void OnEditOpMirroring();
	afx_msg void OnEditOpSizing();
	afx_msg void OnBnClickedEditOpMirX();
	afx_msg void OnBnClickedEditOpMirY();
	afx_msg void OnBnClickedEditOpMirZ();

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
