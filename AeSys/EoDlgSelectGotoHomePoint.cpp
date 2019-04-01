#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

#include "EoDlgSelectGotoHomePoint.h"

// EoDlgSelectGotoHomePoint dialog

IMPLEMENT_DYNAMIC(EoDlgSelectGotoHomePoint, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSelectGotoHomePoint, CDialog)
	ON_CBN_EDITUPDATE(IDC_LIST, &EoDlgSelectGotoHomePoint::OnCbnEditupdateList)
	ON_CBN_SELCHANGE(IDC_LIST, &EoDlgSelectGotoHomePoint::OnCbnSelchangeList)
END_MESSAGE_MAP()

EoDlgSelectGotoHomePoint::EoDlgSelectGotoHomePoint(CWnd* parent) 
    : CDialog(EoDlgSelectGotoHomePoint::IDD, parent)
    , m_ActiveView(0) {
}

EoDlgSelectGotoHomePoint::EoDlgSelectGotoHomePoint(AeSysView* activeView, CWnd* parent) 
    : CDialog(EoDlgSelectGotoHomePoint::IDD, parent)
    , m_ActiveView(activeView) {
}

EoDlgSelectGotoHomePoint::~EoDlgSelectGotoHomePoint() {
}

void EoDlgSelectGotoHomePoint::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_HomePointNames);
	DDX_Control(pDX, IDC_X, m_X);
	DDX_Control(pDX, IDC_Y, m_Y);
	DDX_Control(pDX, IDC_Z, m_Z);
}
BOOL EoDlgSelectGotoHomePoint::OnInitDialog() {
	CDialog::OnInitDialog();

	CString Names = theApp.LoadStringResource(IDS_HOME_POINT_GO_NAMES);
	m_HomePointNames.ResetContent();
	int Position = 0;
	while (Position < Names.GetLength()) {
		CString NamesItem = Names.Tokenize(L"\n", Position);
		m_HomePointNames.AddString(NamesItem);
	}
	m_HomePointNames.SetCurSel(9);

	OdGePoint3d Origin = m_ActiveView->GridOrigin();

	SetDlgItemTextW(IDC_X, theApp.FormatLength(Origin.x, max(theApp.GetUnits(), AeSysApp::kEngineering), 12, 4));
	SetDlgItemTextW(IDC_Y, theApp.FormatLength(Origin.y, max(theApp.GetUnits(), AeSysApp::kEngineering), 12, 4));
	SetDlgItemTextW(IDC_Z, theApp.FormatLength(Origin.z, max(theApp.GetUnits(), AeSysApp::kEngineering), 12, 4));

	return TRUE;
}
void EoDlgSelectGotoHomePoint::OnOK() {
	CDialog::OnOK();
}
void EoDlgSelectGotoHomePoint::OnCbnEditupdateList() {
	CString NamesItem;
	m_HomePointNames.GetWindowTextW(NamesItem);

	int NamesItemIndex = m_HomePointNames.FindString(- 1, NamesItem);

	if (NamesItemIndex != CB_ERR) {
		OdGePoint3d Point;
		switch (NamesItemIndex) {
		case 9:
			Point = m_ActiveView->GridOrigin();
			break;
		case 10:
			Point = AeSysDoc::GetDoc()->TrapPivotPoint();
			break;
		case 11:
			Point = m_ActiveView->CameraTarget();
			break;
		case 12:
			Point = OdGePoint3d::kOrigin;
			break;
		default:
			Point = theApp.HomePointGet(NamesItemIndex);
		}
		m_ActiveView->SetCursorPosition(Point);

		CDialog::OnOK();
	}
}
void EoDlgSelectGotoHomePoint::OnCbnSelchangeList() {
	int NamesItemIndex = m_HomePointNames.GetCurSel();

	if (NamesItemIndex != CB_ERR) {
		OdGePoint3d Point;
		switch (NamesItemIndex) {
		case 9:
			Point = m_ActiveView->GridOrigin();
			break;
		case 10:
			Point = AeSysDoc::GetDoc()->TrapPivotPoint();
			break;
		case 11:
			Point = m_ActiveView->CameraTarget();
			break;
		case 12:
			Point = OdGePoint3d::kOrigin;
			break;
		default:
			Point = theApp.HomePointGet(NamesItemIndex);
		}
		SetDlgItemTextW(IDC_X, theApp.FormatLength(Point.x, max(theApp.GetUnits(), AeSysApp::kEngineering), 12, 4));
		SetDlgItemTextW(IDC_Y, theApp.FormatLength(Point.y, max(theApp.GetUnits(), AeSysApp::kEngineering), 12, 4));
		SetDlgItemTextW(IDC_Z, theApp.FormatLength(Point.z, max(theApp.GetUnits(), AeSysApp::kEngineering), 12, 4));
	}
}
