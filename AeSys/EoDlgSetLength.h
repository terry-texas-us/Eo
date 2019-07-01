#pragma once
class EoDlgSetLength final : public CDialog {
DECLARE_DYNAMIC(EoDlgSetLength)

	EoDlgSetLength(CWnd* parent = nullptr);

	enum { IDD = IDD_SET_LENGTH };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;

	BOOL OnInitDialog() final;

	void OnOK() final;

public:
	double length {0.0};
	CString title;
DECLARE_MESSAGE_MAP()
};
