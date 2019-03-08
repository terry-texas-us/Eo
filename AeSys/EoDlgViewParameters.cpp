#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysView.h"
#include "EoDlgViewParameters.h"

// EoDlgViewParameters dialog

IMPLEMENT_DYNAMIC(EoDlgViewParameters, CDialog)

BEGIN_MESSAGE_MAP(EoDlgViewParameters, CDialog)
	ON_BN_CLICKED(IDC_APPLY, &EoDlgViewParameters::OnBnClickedApply)
	ON_EN_CHANGE(IDC_POSITION_X, &EoDlgViewParameters::OnEnChangePositionX)
	ON_EN_CHANGE(IDC_POSITION_Y, &EoDlgViewParameters::OnEnChangePositionY)
	ON_EN_CHANGE(IDC_POSITION_Z, &EoDlgViewParameters::OnEnChangePositionZ)
	ON_EN_CHANGE(IDC_TARGET_X, &EoDlgViewParameters::OnEnChangeTargetX)
	ON_EN_CHANGE(IDC_TARGET_Y, &EoDlgViewParameters::OnEnChangeTargetY)
	ON_EN_CHANGE(IDC_TARGET_Z, &EoDlgViewParameters::OnEnChangeTargetZ)
	ON_EN_CHANGE(IDC_FRONT_CLIP_DISTANCE, &EoDlgViewParameters::OnEnChangeFrontClipDistance)
	ON_EN_CHANGE(IDC_BACK_CLIP_DISTANCE, &EoDlgViewParameters::OnEnChangeBackClipDistance)
	ON_EN_CHANGE(IDC_LENS_LENGTH, &EoDlgViewParameters::OnEnChangeLensLength)
	ON_BN_CLICKED(IDC_PERSPECTIVE_PROJECTION, &EoDlgViewParameters::OnBnClickedPerspectiveProjection)
END_MESSAGE_MAP()

EoDlgViewParameters::EoDlgViewParameters(CWnd* pParent /*=NULL*/)
	: CDialog(EoDlgViewParameters::IDD, pParent), m_ModelView(0) {
}
EoDlgViewParameters::~EoDlgViewParameters() {
}
void EoDlgViewParameters::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_PERSPECTIVE_PROJECTION, m_PerspectiveProjection);
}
// EoDlgViewParameters message handlers

void EoDlgViewParameters::OnBnClickedApply() {
	AeSysView* ActiveView = AeSysView::GetActiveView();

	EoGsViewport Viewport;
	ActiveView->ModelViewGetViewport(Viewport);

	EoGsViewTransform* ModelView = (EoGsViewTransform*) m_ModelView;

	WCHAR szBuf[32];
	OdGePoint3d Position;
	GetDlgItemTextW(IDC_POSITION_X, (LPWSTR) szBuf, 32);
	Position.x = theApp.ParseLength(theApp.GetUnits(), szBuf);
	GetDlgItemTextW(IDC_POSITION_Y, (LPWSTR) szBuf, 32);
	Position.y = theApp.ParseLength(theApp.GetUnits(), szBuf);
	GetDlgItemTextW(IDC_POSITION_Z, (LPWSTR) szBuf, 32);
	Position.z = theApp.ParseLength(theApp.GetUnits(), szBuf);

	OdGePoint3d Target;
	GetDlgItemTextW(IDC_TARGET_X, (LPWSTR) szBuf, 32);
	Target.x = theApp.ParseLength(theApp.GetUnits(), szBuf);
	GetDlgItemTextW(IDC_TARGET_Y, (LPWSTR) szBuf, 32);
	Target.y = theApp.ParseLength(theApp.GetUnits(), szBuf);
	GetDlgItemTextW(IDC_TARGET_Z, (LPWSTR) szBuf, 32);
	Target.z = theApp.ParseLength(theApp.GetUnits(), szBuf);

	GetDlgItemTextW(IDC_FRONT_CLIP_DISTANCE, (LPWSTR) szBuf, 32);
	double NearClipDistance = theApp.ParseLength(theApp.GetUnits(), szBuf);
	GetDlgItemTextW(IDC_BACK_CLIP_DISTANCE, (LPWSTR) szBuf, 32);
	double FarClipDistance = theApp.ParseLength(theApp.GetUnits(), szBuf);

	GetDlgItemTextW(IDC_LENS_LENGTH, (LPWSTR) szBuf, 32);
	double LensLength = theApp.ParseLength(theApp.GetUnits(), szBuf);

	OdGeVector3d Direction = Position - Target;
	// <tas="Is the direction reversed?"</tas>

	OdGeVector3d UpVector = Direction.crossProduct(OdGeVector3d::kZAxis);
	UpVector = UpVector.crossProduct(Direction);

	if (UpVector.isZeroLength()) {
		UpVector = OdGeVector3d::kYAxis;
	}
	else {
		UpVector.normalize();
	}
	ModelView->SetLensLength(LensLength);
	ModelView->SetNearClipDistance(NearClipDistance);
	ModelView->SetFarClipDistance(FarClipDistance);
	ModelView->EnablePerspective(m_PerspectiveProjection == TRUE);

	double AspectRatio = Viewport.HeightInInches() / Viewport.WidthInInches();
	double FieldWidth(ModelView->FieldWidth());
	double FieldHeight(ModelView->FieldHeight());
	if (AspectRatio < FieldHeight / FieldWidth) {
		FieldWidth = FieldHeight / AspectRatio;
	}
	else {
		FieldHeight = FieldWidth * AspectRatio;
	}
	ModelView->SetView(Position, Target, UpVector, FieldWidth, FieldHeight);
	ModelView->BuildTransformMatrix();

	ActiveView->SetViewTransform(*ModelView);
	ActiveView->InvalidateRect(NULL, TRUE);

	GetDlgItem(IDC_APPLY)->EnableWindow(FALSE);
}
BOOL EoDlgViewParameters::OnInitDialog() {
	CDialog::OnInitDialog();

	EoGsViewTransform* ModelView = (EoGsViewTransform*) m_ModelView;

	AeSysApp::Units Units = max(theApp.GetUnits(), AeSysApp::kEngineering);
	SetDlgItemTextW(IDC_POSITION_X, theApp.FormatLength(ModelView->Position().x, Units));
	SetDlgItemTextW(IDC_POSITION_Y, theApp.FormatLength(ModelView->Position().y, Units));
	SetDlgItemTextW(IDC_POSITION_Z, theApp.FormatLength(ModelView->Position().z, Units));

	SetDlgItemTextW(IDC_TARGET_X, theApp.FormatLength(ModelView->Target().x, Units));
	SetDlgItemTextW(IDC_TARGET_Y, theApp.FormatLength(ModelView->Target().y, Units));
	SetDlgItemTextW(IDC_TARGET_Z, theApp.FormatLength(ModelView->Target().z, Units));

	SetDlgItemTextW(IDC_FRONT_CLIP_DISTANCE, theApp.FormatLength(ModelView->NearClipDistance(), Units));
	SetDlgItemTextW(IDC_BACK_CLIP_DISTANCE, theApp.FormatLength(ModelView->FarClipDistance(), Units));

	SetDlgItemTextW(IDC_LENS_LENGTH, theApp.FormatLength(ModelView->LensLength(), Units));

	GetDlgItem(IDC_APPLY)->EnableWindow(FALSE);

	return TRUE;
}
void EoDlgViewParameters::OnOK() {
	OnBnClickedApply();

	CDialog::OnOK();
}
void EoDlgViewParameters::OnEnChangePositionX() {
	GetDlgItem(IDC_APPLY)->EnableWindow(TRUE);
}
void EoDlgViewParameters::OnEnChangePositionY() {
	GetDlgItem(IDC_APPLY)->EnableWindow(TRUE);
}
void EoDlgViewParameters::OnEnChangePositionZ() {
	GetDlgItem(IDC_APPLY)->EnableWindow(TRUE);
}
void EoDlgViewParameters::OnEnChangeTargetX() {
	GetDlgItem(IDC_APPLY)->EnableWindow(TRUE);
}
void EoDlgViewParameters::OnEnChangeTargetY() {
	GetDlgItem(IDC_APPLY)->EnableWindow(TRUE);
}
void EoDlgViewParameters::OnEnChangeTargetZ() {
	GetDlgItem(IDC_APPLY)->EnableWindow(TRUE);
}
void EoDlgViewParameters::OnEnChangeFrontClipDistance() {
	GetDlgItem(IDC_APPLY)->EnableWindow(TRUE);
}
void EoDlgViewParameters::OnEnChangeBackClipDistance() {
	GetDlgItem(IDC_APPLY)->EnableWindow(TRUE);
}
void EoDlgViewParameters::OnEnChangeLensLength() {
	GetDlgItem(IDC_APPLY)->EnableWindow(TRUE);
}
void EoDlgViewParameters::OnBnClickedPerspectiveProjection() {
	GetDlgItem(IDC_APPLY)->EnableWindow(TRUE);
}
