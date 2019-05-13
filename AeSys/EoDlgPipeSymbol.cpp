#include "stdafx.h"
#include "AeSysApp.h"

#include "EoDlgPipeSymbol.h"

// EoDlgPipeSymbol dialog

IMPLEMENT_DYNAMIC(EoDlgPipeSymbol, CDialog)

BEGIN_MESSAGE_MAP(EoDlgPipeSymbol, CDialog)
END_MESSAGE_MAP()

EoDlgPipeSymbol::EoDlgPipeSymbol(CWnd* parent)
	: CDialog(EoDlgPipeSymbol::IDD, parent)
	, m_CurrentPipeSymbolIndex(0) {
}

EoDlgPipeSymbol::~EoDlgPipeSymbol() {
}

void EoDlgPipeSymbol::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_PipeSymbolsListBoxControl);
}
BOOL EoDlgPipeSymbol::OnInitDialog() {
	CDialog::OnInitDialog();

	CString Names = theApp.LoadStringResource(IDS_PIPE_SYMBOL_NAMES);
	int Position = 0;
	while (Position < Names.GetLength()) {
		CString NamesItem = Names.Tokenize(L"\n", Position);
		m_PipeSymbolsListBoxControl.AddString(NamesItem);
	}
	m_PipeSymbolsListBoxControl.SetCurSel(m_CurrentPipeSymbolIndex);

	return TRUE;
}
void EoDlgPipeSymbol::OnOK() {
	m_CurrentPipeSymbolIndex = m_PipeSymbolsListBoxControl.GetCurSel();

	CDialog::OnOK();
}
