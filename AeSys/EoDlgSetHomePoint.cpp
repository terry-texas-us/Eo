#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

#include "EoDlgSetHomePoint.h"

// EoDlgSetHomePoint dialog

IMPLEMENT_DYNAMIC(EoDlgSetHomePoint, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetHomePoint, CDialog)
	ON_CBN_EDITUPDATE(IDC_LIST, &EoDlgSetHomePoint::OnCbnEditupdateList)
END_MESSAGE_MAP()

OdGePoint3d EoDlgSetHomePoint::m_CursorPosition = OdGePoint3d::kOrigin;

EoDlgSetHomePoint::EoDlgSetHomePoint(CWnd* parent) 
    : CDialog(EoDlgSetHomePoint::IDD, parent)
    , m_ActiveView(0) {
}

EoDlgSetHomePoint::EoDlgSetHomePoint(AeSysView* activeView, CWnd* parent) 
    : CDialog(EoDlgSetHomePoint::IDD, parent)
    , m_ActiveView(activeView) {
}

EoDlgSetHomePoint::~EoDlgSetHomePoint() {
}

void EoDlgSetHomePoint::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_HomePointNames);
	DDX_Control(pDX, IDC_X, m_X);
	DDX_Control(pDX, IDC_Y, m_Y);
	DDX_Control(pDX, IDC_Z, m_Z);
}
BOOL EoDlgSetHomePoint::OnInitDialog() {
	CDialog::OnInitDialog();

	CString Names = theApp.LoadStringResource(IDS_HOME_POINT_SET_NAMES);
	m_HomePointNames.ResetContent();
	int Position = 0;
	while (Position < Names.GetLength()) {
		CString NamesItem = Names.Tokenize(L"\n", Position);
		m_HomePointNames.AddString(NamesItem);
	}
	m_HomePointNames.SetCurSel(9);

	m_CursorPosition = theApp.GetCursorPosition();

	SetDlgItemTextW(IDC_X, theApp.FormatLength(m_CursorPosition.x, max(theApp.GetUnits(), AeSysApp::kEngineering), 12, 4));
	SetDlgItemTextW(IDC_Y, theApp.FormatLength(m_CursorPosition.y, max(theApp.GetUnits(), AeSysApp::kEngineering), 12, 4));
	SetDlgItemTextW(IDC_Z, theApp.FormatLength(m_CursorPosition.z, max(theApp.GetUnits(), AeSysApp::kEngineering), 12, 4));

	return TRUE;
}
void EoDlgSetHomePoint::OnOK() {
	wchar_t szBuf[32];

	const AeSysApp::Units CurrentUnits = theApp.GetUnits();

	m_X.GetWindowTextW(szBuf, 32);
	m_CursorPosition.x = theApp.ParseLength(CurrentUnits, szBuf);
	m_Y.GetWindowTextW(szBuf, 32);
	m_CursorPosition.y = theApp.ParseLength(CurrentUnits, szBuf);
	m_Z.GetWindowTextW(szBuf, 32);
	m_CursorPosition.z = theApp.ParseLength(CurrentUnits, szBuf);

	const int NamesItemIndex = m_HomePointNames.GetCurSel();

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
void EoDlgSetHomePoint::OnCbnEditupdateList() {
	CString NamesItem;
	m_HomePointNames.GetWindowTextW(NamesItem);
	const int NamesItemIndex = m_HomePointNames.FindString(- 1, NamesItem);

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
