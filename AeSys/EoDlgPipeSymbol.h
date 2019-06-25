#pragma once

class EoDlgPipeSymbol final : public CDialog {
DECLARE_DYNAMIC(EoDlgPipeSymbol)
	EoDlgPipeSymbol(CWnd* parent = nullptr);
	virtual ~EoDlgPipeSymbol();

	enum { IDD = IDD_PIPE_SYMBOL };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
public:
	CListBox pipeSymbolsListBoxControl;
	int currentPipeSymbolIndex {0};
protected:
DECLARE_MESSAGE_MAP()
};
