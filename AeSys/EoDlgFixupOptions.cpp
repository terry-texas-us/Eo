#include "stdafx.h"
#include "EoDlgFixupOptions.h"
IMPLEMENT_DYNAMIC(EoDlgFixupOptions, CDialog)

BEGIN_MESSAGE_MAP(EoDlgFixupOptions, CDialog)
END_MESSAGE_MAP()

EoDlgFixupOptions::EoDlgFixupOptions(CWnd* parent)
	: CDialog(IDD, parent) {
}

void EoDlgFixupOptions::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Text(dataExchange, IDC_FIX_AX_TOL, axisTolerance);
	DDX_Text(dataExchange, IDC_FIX_SIZ, cornerSize);
}
