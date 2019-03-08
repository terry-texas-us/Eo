#pragma once

// EoDlgModeLetter dialog

class EoDlgModeLetter : public CDialog {
	DECLARE_DYNAMIC(EoDlgModeLetter)

public:
	EoDlgModeLetter(CWnd* pParent = NULL);
	virtual ~EoDlgModeLetter();

// Dialog Data
	enum { IDD = IDD_ADD_NOTE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	static OdGePoint3d m_Point;
public:
	CEdit m_TextEditControl;
	/// <summary> Effectively resizes the edit control to use the entire client area of the dialog.</summary>
	/// <remarks> OnSize can be called before OnInitialUpdate so check is made for valid control window.</remarks>
	afx_msg void OnSize(UINT nType, int cx, int cy);

protected:
	DECLARE_MESSAGE_MAP()
};
