#include "stdafx.h"

#include "EoDlgPipeOptions.h"

// EoDlgPipeOptions dialog

IMPLEMENT_DYNAMIC(EoDlgPipeOptions, CDialog)

BEGIN_MESSAGE_MAP(EoDlgPipeOptions, CDialog)
END_MESSAGE_MAP()

EoDlgPipeOptions::EoDlgPipeOptions(CWnd* parent) 
    : CDialog(EoDlgPipeOptions::IDD, parent)
    , m_PipeTicSize(0)
    , m_PipeRiseDropRadius(0) {
}

EoDlgPipeOptions::~EoDlgPipeOptions() {
}

void EoDlgPipeOptions::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
}
BOOL EoDlgPipeOptions::OnInitDialog() {
	CDialog::OnInitDialog();

	return TRUE;
}
void EoDlgPipeOptions::OnOK() {
	CDialog::OnOK();
}
