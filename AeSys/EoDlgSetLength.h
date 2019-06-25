#pragma once

class EoDlgSetLength final : public CDialog {
DECLARE_DYNAMIC(EoDlgSetLength)
	EoDlgSetLength(CWnd* parent = nullptr);
	virtual ~EoDlgSetLength();

	enum { IDD = IDD_SET_LENGTH };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
public:
	double length {0.0};
	CString title;
protected:
DECLARE_MESSAGE_MAP()
};
