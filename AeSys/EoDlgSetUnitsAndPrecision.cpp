#include "stdafx.h"
#include "AeSys.h"
#include "EoDlgSetUnitsAndPrecision.h"

IMPLEMENT_DYNAMIC(EoDlgSetUnitsAndPrecision, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetUnitsAndPrecision, CDialog)
		ON_BN_CLICKED(IDC_METRIC, &EoDlgSetUnitsAndPrecision::OnBnClickedMetric)
END_MESSAGE_MAP()

EoDlgSetUnitsAndPrecision::EoDlgSetUnitsAndPrecision(CWnd* parent)
	: CDialog(IDD, parent) {
}

void EoDlgSetUnitsAndPrecision::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_METRIC_UNITS, m_MetricUnitsListBoxControl);
	DDX_Text(dataExchange, IDC_PRECISION, precision);
}

BOOL EoDlgSetUnitsAndPrecision::OnInitDialog() {
	CDialog::OnInitDialog();
	const auto CheckButtonId {EoMin(IDC_ARCHITECTURAL + units, IDC_METRIC)};
	CheckRadioButton(IDC_ARCHITECTURAL, IDC_METRIC, CheckButtonId);
	const auto MetricUnits {AeSys::LoadStringResource(IDS_METRIC_UNITS)};
	auto Position {0};
	while (Position < MetricUnits.GetLength()) {
		auto UnitsItem {MetricUnits.Tokenize(L"\n", Position)};
		m_MetricUnitsListBoxControl.AddString(UnitsItem);
	}
	if (CheckButtonId == IDC_METRIC) {
		m_MetricUnitsListBoxControl.SetCurSel(units - AeSys::kMeters);
	}
	return TRUE;
}

void EoDlgSetUnitsAndPrecision::OnOK() {
	switch (GetCheckedRadioButton(IDC_ARCHITECTURAL, IDC_METRIC)) {
		case IDC_ARCHITECTURAL:
			units = AeSys::kArchitectural;
			break;
		case IDC_ENGINEERING:
			units = AeSys::kEngineering;
			break;
		case IDC_FEET:
			units = AeSys::kFeet;
			break;
		case IDC_INCHES:
			units = AeSys::kInches;
			break;
		default:
			switch (m_MetricUnitsListBoxControl.GetCurSel()) {
				case 0:
					units = AeSys::kMeters;
					break;
				case 1:
					units = AeSys::kMillimeters;
					break;
				case 2:
					units = AeSys::kCentimeters;
					break;
				case 3:
					units = AeSys::kDecimeters;
					break;
				default:
					units = AeSys::kKilometers;
			}
	}
	CDialog::OnOK();
}

void EoDlgSetUnitsAndPrecision::OnBnClickedMetric() {
	m_MetricUnitsListBoxControl.SetCurSel(AeSys::kCentimeters - AeSys::kMeters);
}
