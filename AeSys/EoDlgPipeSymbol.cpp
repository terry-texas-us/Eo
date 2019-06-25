#include "stdafx.h"
#include "AeSys.h"
#include "EoDlgPipeSymbol.h"

// EoDlgPipeSymbol dialog
IMPLEMENT_DYNAMIC(EoDlgPipeSymbol, CDialog)

BEGIN_MESSAGE_MAP(EoDlgPipeSymbol, CDialog)
END_MESSAGE_MAP()

EoDlgPipeSymbol::EoDlgPipeSymbol(CWnd* parent)
	: CDialog(IDD, parent)
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
	const auto Names {AeSys::LoadStringResource(IDS_PIPE_SYMBOL_NAMES)};
	auto Position {0};
	while (Position < Names.GetLength()) {
		auto NamesItem {Names.Tokenize(L"\n", Position)};
		m_PipeSymbolsListBoxControl.AddString(NamesItem);
	}
	m_PipeSymbolsListBoxControl.SetCurSel(m_CurrentPipeSymbolIndex);
	return TRUE;
}

void EoDlgPipeSymbol::OnOK() {
	m_CurrentPipeSymbolIndex = m_PipeSymbolsListBoxControl.GetCurSel();
	CDialog::OnOK();
}
