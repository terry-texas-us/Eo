#include "stdafx.h"
#include "EoDlgSelectIsometricView.h"
IMPLEMENT_DYNAMIC(EoDlgSelectIsometricView, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSelectIsometricView, CDialog)
END_MESSAGE_MAP()

EoDlgSelectIsometricView::EoDlgSelectIsometricView(CWnd* parent)
	: CDialog(IDD, parent) {}

void EoDlgSelectIsometricView::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Radio(dataExchange, IDC_VIEW_ISO_LEFT, leftRight);
	DDX_Radio(dataExchange, IDC_VIEW_ISO_FRONT, frontBack);
	DDX_Radio(dataExchange, IDC_VIEW_ISO_ABOVE, aboveUnder);
}

// EoDlgSelectIsometricView message handlers
BOOL EoDlgSelectIsometricView::OnInitDialog() {
	CDialog::OnInitDialog();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
