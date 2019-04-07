#pragma once

// EoDlgDrawOptions dialog

class EoDlgDrawOptions : public CDialog {
	DECLARE_DYNAMIC(EoDlgDrawOptions)

public:
	EoDlgDrawOptions(CWnd* parent = NULL);
	virtual ~EoDlgDrawOptions();

// Dialog Data
	enum { IDD = IDD_DRAW_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override;

public:
	afx_msg void OnBnClickedPen();
	afx_msg void OnBnClickedLine();
	afx_msg void OnBnClickedText();
	afx_msg void OnBnClickedFill();
	afx_msg void OnBnClickedConstraints();

protected:
	DECLARE_MESSAGE_MAP()
};
