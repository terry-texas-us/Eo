#pragma once

// EoDlgModeLetter dialog
class EoDlgModeLetter : public CDialog {
DECLARE_DYNAMIC(EoDlgModeLetter)
	EoDlgModeLetter(CWnd* parent = nullptr);
	virtual ~EoDlgModeLetter();

	// Dialog Data
	enum { IDD = IDD_ADD_NOTE };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
	static OdGePoint3d m_Point;
public:
	CEdit m_TextEditControl;
	/// <summary> Effectively resizes the edit control to use the entire client area of the dialog.</summary>
	/// <remarks> OnSize can be called before OnInitialUpdate so check is made for valid control window.</remarks>
	void OnSize(unsigned type, int cx, int cy);
protected:
DECLARE_MESSAGE_MAP()
};
