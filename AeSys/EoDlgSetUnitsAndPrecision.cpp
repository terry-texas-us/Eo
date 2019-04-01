#include "stdafx.h"
#include "AeSysApp.h"

#include "EoDlgSetUnitsAndPrecision.h"

// EoDlgSetUnitsAndPrecision dialog

IMPLEMENT_DYNAMIC(EoDlgSetUnitsAndPrecision, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetUnitsAndPrecision, CDialog)
	ON_BN_CLICKED(IDC_METRIC, &EoDlgSetUnitsAndPrecision::OnBnClickedMetric)
END_MESSAGE_MAP()

EoDlgSetUnitsAndPrecision::EoDlgSetUnitsAndPrecision(CWnd* parent) 
    : CDialog(EoDlgSetUnitsAndPrecision::IDD, parent)
    , m_Units(AeSysApp::kInches)
    , m_Precision(8) {
}

EoDlgSetUnitsAndPrecision::~EoDlgSetUnitsAndPrecision() {
}

void EoDlgSetUnitsAndPrecision::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_METRIC_UNITS, m_MetricUnitsListBoxControl);
	DDX_Text(pDX, IDC_PRECISION, m_Precision);
}

BOOL EoDlgSetUnitsAndPrecision::OnInitDialog() {
	CDialog::OnInitDialog();

	const int CheckButtonId = EoMin(IDC_ARCHITECTURAL + m_Units, IDC_METRIC);
	CheckRadioButton(IDC_ARCHITECTURAL, IDC_METRIC, CheckButtonId);

	CString MetricUnits = theApp.LoadStringResource(IDS_METRIC_UNITS);
	int Position = 0;
	while (Position < MetricUnits.GetLength()) {
		CString UnitsItem = MetricUnits.Tokenize(L"\n", Position);
		m_MetricUnitsListBoxControl.AddString(UnitsItem);
	}
	if (CheckButtonId == IDC_METRIC) {
		m_MetricUnitsListBoxControl.SetCurSel(m_Units - AeSysApp::kMeters);
	}
	return TRUE;
}
void EoDlgSetUnitsAndPrecision::OnOK() {
	switch (GetCheckedRadioButton(IDC_ARCHITECTURAL, IDC_METRIC)) {
	case IDC_ARCHITECTURAL:
		m_Units = AeSysApp::kArchitectural;
		break;
	case IDC_ENGINEERING:
		m_Units = AeSysApp::kEngineering;
		break;
	case IDC_FEET:
		m_Units = AeSysApp::kFeet;
		break;
	case IDC_INCHES:
		m_Units = AeSysApp::kInches;
		break;
	default:
		switch (m_MetricUnitsListBoxControl.GetCurSel()) {
		case 0:
			m_Units = AeSysApp::kMeters;
			break;
		case 1:
			m_Units = AeSysApp::kMillimeters;
			break;
		case 2:
			m_Units = AeSysApp::kCentimeters;
			break;
		case 3:
			m_Units = AeSysApp::kDecimeters;
			break;
		default:
			m_Units = AeSysApp::kKilometers;
		}
	}
	CDialog::OnOK();
}
void EoDlgSetUnitsAndPrecision::OnBnClickedMetric() {
	m_MetricUnitsListBoxControl.SetCurSel(AeSysApp::kCentimeters - AeSysApp::kMeters);
}
