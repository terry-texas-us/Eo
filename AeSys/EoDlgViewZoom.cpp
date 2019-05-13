#include "stdafx.h"
#include "EoDlgViewZoom.h"

// EoDlgViewZoom dialog

IMPLEMENT_DYNAMIC(EoDlgViewZoom, CDialog)

BEGIN_MESSAGE_MAP(EoDlgViewZoom, CDialog)
END_MESSAGE_MAP()

EoDlgViewZoom::EoDlgViewZoom(CWnd* parent)
	: CDialog(EoDlgViewZoom::IDD, parent)
	, m_ZoomFactor(1.) {
}

EoDlgViewZoom::~EoDlgViewZoom() {
}

void EoDlgViewZoom::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_RATIO, m_ZoomFactor);
	DDV_MinMaxDouble(pDX, m_ZoomFactor, 0.001, 999.);
}

BOOL EoDlgViewZoom::OnInitDialog() {
	m_ZoomFactor = EoRound(m_ZoomFactor, 3);
	const int Precision = (m_ZoomFactor >= 1.) ? 3 - int(log10(m_ZoomFactor)) - 1 : 3;
	CString FormatSpecification;
	FormatSpecification.Format(L"%%8.%if", Precision);
	CString ZoomFactor;
	ZoomFactor.Format(FormatSpecification, m_ZoomFactor);
	m_ZoomFactor = _wtof(ZoomFactor);

	CDialog::OnInitDialog();
	return TRUE;
}
// EoDlgViewZoom message handlers


