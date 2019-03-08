#pragma once

// EoDlgPipeSymbol dialog

class EoDlgPipeSymbol : public CDialog {
	DECLARE_DYNAMIC(EoDlgPipeSymbol)

public:
	EoDlgPipeSymbol(CWnd* pParent = NULL);
	virtual ~EoDlgPipeSymbol();

// Dialog Data
	enum { IDD = IDD_PIPE_SYMBOL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

public:
	CListBox m_PipeSymbolsListBoxControl;
	int m_CurrentPipeSymbolIndex;

protected:
	DECLARE_MESSAGE_MAP()
};


