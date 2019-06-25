#include "stdafx.h"
#include "AeSys.h"
#include "EoDlgSetUnitsAndPrecision.h"

// EoDlgSetUnitsAndPrecision dialog
IMPLEMENT_DYNAMIC(EoDlgSetUnitsAndPrecision, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetUnitsAndPrecision, CDialog)
		ON_BN_CLICKED(IDC_METRIC, &EoDlgSetUnitsAndPrecision::OnBnClickedMetric)
END_MESSAGE_MAP()

EoDlgSetUnitsAndPrecision::EoDlgSetUnitsAndPrecision(CWnd* parent)
	: CDialog(IDD, parent)
	, m_Units(AeSys::kInches)
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
	const auto CheckButtonId {EoMin(IDC_ARCHITECTURAL + m_Units, IDC_METRIC)};
	CheckRadioButton(IDC_ARCHITECTURAL, IDC_METRIC, CheckButtonId);
	const auto MetricUnits {AeSys::LoadStringResource(IDS_METRIC_UNITS)};
	auto Position {0};
	while (Position < MetricUnits.GetLength()) {
		auto UnitsItem {MetricUnits.Tokenize(L"\n", Position)};
		m_MetricUnitsListBoxControl.AddString(UnitsItem);
	}
	if (CheckButtonId == IDC_METRIC) {
		m_MetricUnitsListBoxControl.SetCurSel(m_Units - AeSys::kMeters);
	}
	return TRUE;
}

void EoDlgSetUnitsAndPrecision::OnOK() {
	switch (GetCheckedRadioButton(IDC_ARCHITECTURAL, IDC_METRIC)) {
		case IDC_ARCHITECTURAL:
			m_Units = AeSys::kArchitectural;
			break;
		case IDC_ENGINEERING:
			m_Units = AeSys::kEngineering;
			break;
		case IDC_FEET:
			m_Units = AeSys::kFeet;
			break;
		case IDC_INCHES:
			m_Units = AeSys::kInches;
			break;
		default:
			switch (m_MetricUnitsListBoxControl.GetCurSel()) {
				case 0:
					m_Units = AeSys::kMeters;
					break;
				case 1:
					m_Units = AeSys::kMillimeters;
					break;
				case 2:
					m_Units = AeSys::kCentimeters;
					break;
				case 3:
					m_Units = AeSys::kDecimeters;
					break;
				default:
					m_Units = AeSys::kKilometers;
			}
	}
	CDialog::OnOK();
}

void EoDlgSetUnitsAndPrecision::OnBnClickedMetric() {
	m_MetricUnitsListBoxControl.SetCurSel(AeSys::kCentimeters - AeSys::kMeters);
}
