#pragma once

class EoDlgSetText final : public CDialog {
DECLARE_DYNAMIC(EoDlgSetText)
	EoDlgSetText(CWnd* parent = nullptr);
	virtual ~EoDlgSetText();

	enum { IDD = IDD_SET_TEXT };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
public:
	CString text;
	CString title;
protected:
DECLARE_MESSAGE_MAP()
};
