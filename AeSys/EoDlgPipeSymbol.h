#pragma once

// EoDlgPipeSymbol dialog
class EoDlgPipeSymbol : public CDialog {
DECLARE_DYNAMIC(EoDlgPipeSymbol)
	EoDlgPipeSymbol(CWnd* parent = nullptr);
	virtual ~EoDlgPipeSymbol();

	// Dialog Data
	enum { IDD = IDD_PIPE_SYMBOL };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
public:
	CListBox m_PipeSymbolsListBoxControl;
	int m_CurrentPipeSymbolIndex;
protected:
DECLARE_MESSAGE_MAP()
};
