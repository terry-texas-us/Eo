#include "stdafx.h"

#include "EoDlgSetScale.h"

// EoDlgSetScale dialog

IMPLEMENT_DYNAMIC(EoDlgSetScale, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetScale, CDialog)
END_MESSAGE_MAP()

EoDlgSetScale::EoDlgSetScale(CWnd* pParent /*=NULL*/) :
	CDialog(EoDlgSetScale::IDD, pParent), m_Scale(0) {
}
EoDlgSetScale::~EoDlgSetScale() {
}
void EoDlgSetScale::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SCALE, m_Scale);
	DDV_MinMaxDouble(pDX, m_Scale, .0001, 10000.);
}

