#include "stdafx.h"

#include "EoDlgSetAngle.h"

// EoDlgSetAngle dialog

IMPLEMENT_DYNAMIC(EoDlgSetAngle, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetAngle, CDialog)
END_MESSAGE_MAP()

EoDlgSetAngle::EoDlgSetAngle(CWnd* parent)
	: CDialog(EoDlgSetAngle::IDD, parent)
	, m_dAngle(0) {
}

EoDlgSetAngle::~EoDlgSetAngle() {
}

void EoDlgSetAngle::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ANGLE, m_dAngle);
	DDV_MinMaxDouble(pDX, m_dAngle, -360., 360.);
}
BOOL EoDlgSetAngle::OnInitDialog() {
	CDialog::OnInitDialog();
	if (!m_strTitle.IsEmpty()) {
		SetWindowTextW(m_strTitle);
	}
	return TRUE;
}
