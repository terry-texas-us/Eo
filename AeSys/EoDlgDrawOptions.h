#pragma once

class EoDlgDrawOptions final : public CDialog {
DECLARE_DYNAMIC(EoDlgDrawOptions)
	EoDlgDrawOptions(CWnd* parent = nullptr);

	enum { IDD = IDD_DRAW_OPTIONS };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
public:
	void OnBnClickedPen();
	void OnBnClickedLine();
	void OnBnClickedText();
	void OnBnClickedFill();
	void OnBnClickedConstraints();
protected:
DECLARE_MESSAGE_MAP()
};
