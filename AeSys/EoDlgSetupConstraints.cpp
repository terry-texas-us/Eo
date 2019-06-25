#include "stdafx.h"
#include "AeSys.h"
#include "AeSysView.h"
#include "EoDlgSetupConstraints.h"

// EoDlgSetContraints dialog
IMPLEMENT_DYNAMIC(EoDlgSetupConstraints, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetupConstraints, CDialog)
END_MESSAGE_MAP()

EoDlgSetupConstraints::EoDlgSetupConstraints(CWnd* parent)
	: CDialog(IDD, parent)
	, m_ActiveView(nullptr) {
}

EoDlgSetupConstraints::EoDlgSetupConstraints(AeSysView* view, CWnd* parent)
	: CDialog(IDD, parent)
	, m_ActiveView(view) {
}

EoDlgSetupConstraints::~EoDlgSetupConstraints() {
}

void EoDlgSetupConstraints::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_USR_GRID_X_INT, m_GridXSnapSpacing);
	DDX_Control(pDX, IDC_USR_GRID_Y_INT, m_GridYSnapSpacing);
	DDX_Control(pDX, IDC_USR_GRID_Z_INT, m_GridZSnapSpacing);
	DDX_Control(pDX, IDC_GRID_DOT_INT_X, m_GridXPointSpacing);
	DDX_Control(pDX, IDC_GRID_DOT_INT_Y, m_GridYPointSpacing);
	DDX_Control(pDX, IDC_GRID_DOT_INT_Z, m_GridZPointSpacing);
	DDX_Control(pDX, IDC_GRID_LN_INT_X, m_GridXLineSpacing);
	DDX_Control(pDX, IDC_GRID_LN_INT_Y, m_GridYLineSpacing);
	DDX_Control(pDX, IDC_GRID_LN_INT_Z, m_GridZLineSpacing);
	DDX_Control(pDX, IDC_GRID_SNAP_ON, m_GridSnapEnableButton);
	DDX_Control(pDX, IDC_GRID_PTS_DIS_ON, m_GridDisplayButton);
	DDX_Control(pDX, IDC_GRID_LNS_DIS_ON, m_GridLineDisplayButton);
	DDX_Control(pDX, IDC_USR_AX_INF_ANG, m_AxisInfluenceAngle);
	DDX_Control(pDX, IDC_USR_AX_Z_OFF_ANG, m_AxisZOffsetAngle);
}

BOOL EoDlgSetupConstraints::OnInitDialog() {
	CDialog::OnInitDialog();
	const auto CurrentUnits {theApp.GetUnits()};
	double X;
	double Y;
	double Z;
	m_ActiveView->GetGridSnapSpacing(X, Y, Z);
	m_GridXSnapSpacing.SetWindowTextW(theApp.FormatLength(X, CurrentUnits, 12, 4));
	m_GridYSnapSpacing.SetWindowTextW(theApp.FormatLength(Y, CurrentUnits, 12, 4));
	m_GridZSnapSpacing.SetWindowTextW(theApp.FormatLength(Z, CurrentUnits, 12, 4));
	m_ActiveView->GetGridPointSpacing(X, Y, Z);
	m_GridXPointSpacing.SetWindowTextW(theApp.FormatLength(X, CurrentUnits, 12, 4));
	m_GridYPointSpacing.SetWindowTextW(theApp.FormatLength(Y, CurrentUnits, 12, 4));
	m_GridZPointSpacing.SetWindowTextW(theApp.FormatLength(Z, CurrentUnits, 12, 4));
	m_ActiveView->GetGridLineSpacing(X, Y, Z);
	m_GridXLineSpacing.SetWindowTextW(theApp.FormatLength(X, CurrentUnits, 12, 4));
	m_GridYLineSpacing.SetWindowTextW(theApp.FormatLength(Y, CurrentUnits, 12, 4));
	m_GridZLineSpacing.SetWindowTextW(theApp.FormatLength(Z, CurrentUnits, 12, 4));
	m_GridSnapEnableButton.SetCheck(m_ActiveView->GridSnap() ? BST_CHECKED : BST_UNCHECKED);
	m_GridDisplayButton.SetCheck(m_ActiveView->DisplayGridWithPoints() ? BST_CHECKED : BST_UNCHECKED);
	m_GridLineDisplayButton.SetCheck(m_ActiveView->DisplayGridWithLines() ? BST_CHECKED : BST_UNCHECKED);
	CString Text;
	Text.Format(L"%f", m_ActiveView->AxisConstraintInfluenceAngle());
	m_AxisInfluenceAngle.SetWindowTextW(Text);
	Text.Format(L"%f", m_ActiveView->AxisConstraintOffsetAngle());
	m_AxisZOffsetAngle.SetWindowTextW(Text);
	return TRUE;
}

void EoDlgSetupConstraints::OnOK() {
	const auto CurrentUnits {theApp.GetUnits()};
	wchar_t StringBuffer[32];
	m_GridXSnapSpacing.GetWindowTextW(StringBuffer, 32);
	auto X {AeSys::ParseLength(CurrentUnits, StringBuffer)};
	m_GridYSnapSpacing.GetWindowTextW(StringBuffer, 32);
	auto Y {AeSys::ParseLength(CurrentUnits, StringBuffer)};
	m_GridZSnapSpacing.GetWindowTextW(StringBuffer, 32);
	auto Z {AeSys::ParseLength(CurrentUnits, StringBuffer)};
	m_ActiveView->SetGridSnapSpacing(X, Y, Z);
	m_GridXPointSpacing.GetWindowTextW(StringBuffer, 32);
	X = AeSys::ParseLength(CurrentUnits, StringBuffer);
	m_GridYPointSpacing.GetWindowTextW(StringBuffer, 32);
	Y = AeSys::ParseLength(CurrentUnits, StringBuffer);
	m_GridZPointSpacing.GetWindowTextW(StringBuffer, 32);
	Z = AeSys::ParseLength(CurrentUnits, StringBuffer);
	m_ActiveView->SetGridPointSpacing(X, Y, Z);
	m_GridXLineSpacing.GetWindowTextW(StringBuffer, 32);
	X = AeSys::ParseLength(CurrentUnits, StringBuffer);
	m_GridYLineSpacing.GetWindowTextW(StringBuffer, 32);
	Y = AeSys::ParseLength(CurrentUnits, StringBuffer);
	m_GridZLineSpacing.GetWindowTextW(StringBuffer, 32);
	Z = AeSys::ParseLength(CurrentUnits, StringBuffer);
	m_ActiveView->SetGridLineSpacing(X, Y, Z);
	m_ActiveView->EnableGridSnap(m_GridSnapEnableButton.GetCheck() == BST_CHECKED);
	m_ActiveView->EnableDisplayGridWithPoints(m_GridDisplayButton.GetCheck() == BST_CHECKED);
	m_ActiveView->EnableDisplayGridWithLines(m_GridLineDisplayButton.GetCheck() == BST_CHECKED);
	m_AxisInfluenceAngle.GetWindowTextW(StringBuffer, 32);
	m_ActiveView->SetAxisConstraintInfluenceAngle(_wtof(StringBuffer));
	m_AxisZOffsetAngle.GetWindowTextW(StringBuffer, 32);
	m_ActiveView->SetAxisConstraintOffsetAngle(_wtof(StringBuffer));
	CDialog::OnOK();
}
