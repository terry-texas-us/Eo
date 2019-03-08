// EoDlgSelectIsometricView.cpp : implementation file
//

#include "stdafx.h"
#include "EoDlgSelectIsometricView.h"


// EoDlgSelectIsometricView dialog

IMPLEMENT_DYNAMIC(EoDlgSelectIsometricView, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSelectIsometricView, CDialog)
END_MESSAGE_MAP()

EoDlgSelectIsometricView::EoDlgSelectIsometricView(CWnd* pParent /*=NULL*/)
	: CDialog(EoDlgSelectIsometricView::IDD, pParent) {

}

EoDlgSelectIsometricView::~EoDlgSelectIsometricView() {
}

void EoDlgSelectIsometricView::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_VIEW_ISO_LEFT, m_LeftRight);
	DDX_Radio(pDX, IDC_VIEW_ISO_FRONT, m_FrontBack);
	DDX_Radio(pDX, IDC_VIEW_ISO_ABOVE, m_AboveUnder);
}

// EoDlgSelectIsometricView message handlers

BOOL EoDlgSelectIsometricView::OnInitDialog() {
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
