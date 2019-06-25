#include "stdafx.h"
#include "EoDlgSetScale.h"

IMPLEMENT_DYNAMIC(EoDlgSetScale, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetScale, CDialog)
END_MESSAGE_MAP()

EoDlgSetScale::EoDlgSetScale(CWnd* parent)
	: CDialog(IDD, parent) {
}

EoDlgSetScale::~EoDlgSetScale() = default;

void EoDlgSetScale::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Text(dataExchange, IDC_SCALE, scale);
	DDV_MinMaxDouble(dataExchange, scale, .0001, 10000.);
}
