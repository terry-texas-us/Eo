#include "stdafx.h"
#include "EoDlgViewZoom.h"

IMPLEMENT_DYNAMIC(EoDlgViewZoom, CDialog)

BEGIN_MESSAGE_MAP(EoDlgViewZoom, CDialog)
END_MESSAGE_MAP()

EoDlgViewZoom::EoDlgViewZoom(CWnd* parent)
	: CDialog(IDD, parent) {
}

EoDlgViewZoom::~EoDlgViewZoom() = default;

void EoDlgViewZoom::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Text(dataExchange, IDC_RATIO, m_ZoomFactor);
	DDV_MinMaxDouble(dataExchange, m_ZoomFactor, 0.001, 999.0);
}

BOOL EoDlgViewZoom::OnInitDialog() {
	m_ZoomFactor = EoRound(m_ZoomFactor, 3);
	const auto Precision {m_ZoomFactor >= 1.0 ? 3 - int(log10(m_ZoomFactor)) - 1 : 3};
	CString FormatSpecification;
	FormatSpecification.Format(L"%%8.%if", Precision);
	CString ZoomFactor;
	ZoomFactor.Format(FormatSpecification, m_ZoomFactor);
	m_ZoomFactor = _wtof(ZoomFactor);
	CDialog::OnInitDialog();
	return TRUE;
}
