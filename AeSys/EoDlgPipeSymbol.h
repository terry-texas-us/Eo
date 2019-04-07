#pragma once

// EoDlgPipeSymbol dialog

class EoDlgPipeSymbol : public CDialog {
	DECLARE_DYNAMIC(EoDlgPipeSymbol)

public:
	EoDlgPipeSymbol(CWnd* parent = NULL);
	virtual ~EoDlgPipeSymbol();

// Dialog Data
	enum { IDD = IDD_PIPE_SYMBOL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override;

public:
	CListBox m_PipeSymbolsListBoxControl;
	int m_CurrentPipeSymbolIndex;

protected:
	DECLARE_MESSAGE_MAP()
};


