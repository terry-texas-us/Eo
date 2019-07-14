#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDlgSetHomePoint.h"
IMPLEMENT_DYNAMIC(EoDlgSetHomePoint, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetHomePoint, CDialog)
		ON_CBN_EDITUPDATE(IDC_LIST, &EoDlgSetHomePoint::OnCbnEditUpdateList)
END_MESSAGE_MAP()
OdGePoint3d EoDlgSetHomePoint::m_CursorPosition {0.0, 0.0, 0.0};

EoDlgSetHomePoint::EoDlgSetHomePoint(CWnd* parent)
	: CDialog(IDD, parent) {}

EoDlgSetHomePoint::EoDlgSetHomePoint(AeSysView* activeView, CWnd* parent)
	: CDialog(IDD, parent)
	, m_ActiveView(activeView) {}

EoDlgSetHomePoint::~EoDlgSetHomePoint() = default;

void EoDlgSetHomePoint::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_LIST, homePointNames);
	DDX_Control(dataExchange, IDC_X, x);
	DDX_Control(dataExchange, IDC_Y, y);
	DDX_Control(dataExchange, IDC_Z, z);
}

BOOL EoDlgSetHomePoint::OnInitDialog() {
	CDialog::OnInitDialog();
	const auto Names {AeSys::LoadStringResource(IDS_HOME_POINT_SET_NAMES)};
	homePointNames.ResetContent();
	auto Position {0};
	while (Position < Names.GetLength()) {
		auto NamesItem {Names.Tokenize(L"\n", Position)};
		homePointNames.AddString(NamesItem);
	}
	homePointNames.SetCurSel(9);
	m_CursorPosition = AeSys::GetCursorPosition();
	SetDlgItemTextW(IDC_X, theApp.FormatLength(m_CursorPosition.x, max(theApp.GetUnits(), AeSys::kEngineering), 12, 4));
	SetDlgItemTextW(IDC_Y, theApp.FormatLength(m_CursorPosition.y, max(theApp.GetUnits(), AeSys::kEngineering), 12, 4));
	SetDlgItemTextW(IDC_Z, theApp.FormatLength(m_CursorPosition.z, max(theApp.GetUnits(), AeSys::kEngineering), 12, 4));
	return TRUE;
}

void EoDlgSetHomePoint::OnOK() {
	wchar_t StringBuffer[32];
	const auto CurrentUnits {theApp.GetUnits()};
	x.GetWindowTextW(StringBuffer, 32);
	m_CursorPosition.x = AeSys::ParseLength(CurrentUnits, StringBuffer);
	y.GetWindowTextW(StringBuffer, 32);
	m_CursorPosition.y = AeSys::ParseLength(CurrentUnits, StringBuffer);
	z.GetWindowTextW(StringBuffer, 32);
	m_CursorPosition.z = AeSys::ParseLength(CurrentUnits, StringBuffer);
	const auto NamesItemIndex {homePointNames.GetCurSel()};
	if (NamesItemIndex != CB_ERR) {
		switch (NamesItemIndex) {
			case 9:
				m_ActiveView->SetGridOrigin(m_CursorPosition);
				break;
			case 10:
				AeSysDoc::GetDoc()->SetTrapPivotPoint(m_CursorPosition);
				break;
			case 11:
				m_ActiveView->SetCameraTarget(m_CursorPosition);
				break;
			default:
				theApp.HomePointSave(NamesItemIndex, m_CursorPosition);
		}
		CDialog::OnOK();
	}
}

void EoDlgSetHomePoint::OnCbnEditUpdateList() {
	CString NamesItem;
	homePointNames.GetWindowTextW(NamesItem);
	const auto NamesItemIndex {homePointNames.FindString(-1, NamesItem)};
	if (NamesItemIndex != CB_ERR) {
		switch (NamesItemIndex) {
			case 9:
				m_ActiveView->SetGridOrigin(m_CursorPosition);
				break;
			case 10:
				AeSysDoc::GetDoc()->SetTrapPivotPoint(m_CursorPosition);
				break;
			case 11:
				m_ActiveView->SetCameraTarget(m_CursorPosition);
				break;
			default:
				theApp.HomePointSave(NamesItemIndex, m_CursorPosition);
		}
		CDialog::OnOK();
	}
}
