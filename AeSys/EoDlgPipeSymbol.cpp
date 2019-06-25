#include "stdafx.h"
#include "AeSys.h"
#include "EoDlgPipeSymbol.h"

IMPLEMENT_DYNAMIC(EoDlgPipeSymbol, CDialog)

BEGIN_MESSAGE_MAP(EoDlgPipeSymbol, CDialog)
END_MESSAGE_MAP()

EoDlgPipeSymbol::EoDlgPipeSymbol(CWnd* parent)
	: CDialog(IDD, parent) {
}

EoDlgPipeSymbol::~EoDlgPipeSymbol() = default;

void EoDlgPipeSymbol::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_LIST, pipeSymbolsListBoxControl);
}

BOOL EoDlgPipeSymbol::OnInitDialog() {
	CDialog::OnInitDialog();
	const auto Names {AeSys::LoadStringResource(IDS_PIPE_SYMBOL_NAMES)};
	auto Position {0};
	while (Position < Names.GetLength()) {
		auto NamesItem {Names.Tokenize(L"\n", Position)};
		pipeSymbolsListBoxControl.AddString(NamesItem);
	}
	pipeSymbolsListBoxControl.SetCurSel(currentPipeSymbolIndex);
	return TRUE;
}

void EoDlgPipeSymbol::OnOK() {
	currentPipeSymbolIndex = pipeSymbolsListBoxControl.GetCurSel();
	CDialog::OnOK();
}
