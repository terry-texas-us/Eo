#include "stdafx.h"
#include "AeSys.h"
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

EoDlgViewParameters::EoDlgViewParameters(CWnd* parent)
	: CDialog(IDD, parent)
	, m_PerspectiveProjection(FALSE)
	, m_ModelView(0) {
}

EoDlgViewParameters::~EoDlgViewParameters() {
}

void EoDlgViewParameters::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_PERSPECTIVE_PROJECTION, m_PerspectiveProjection);
}

// EoDlgViewParameters message handlers
void EoDlgViewParameters::OnBnClickedApply() {
	auto ActiveView {AeSysView::GetActiveView()};
	EoGsViewport Viewport;
	ActiveView->ModelViewGetViewport(Viewport);
	auto ModelView {reinterpret_cast<EoGsViewTransform*>(m_ModelView)};
	wchar_t String[32];
	OdGePoint3d Position;
	GetDlgItemTextW(IDC_POSITION_X, String, 32);
	Position.x = AeSys::ParseLength(theApp.GetUnits(), String);
	GetDlgItemTextW(IDC_POSITION_Y, String, 32);
	Position.y = AeSys::ParseLength(theApp.GetUnits(), String);
	GetDlgItemTextW(IDC_POSITION_Z, String, 32);
	Position.z = AeSys::ParseLength(theApp.GetUnits(), String);
	OdGePoint3d Target;
	GetDlgItemTextW(IDC_TARGET_X, String, 32);
	Target.x = AeSys::ParseLength(theApp.GetUnits(), String);
	GetDlgItemTextW(IDC_TARGET_Y, String, 32);
	Target.y = AeSys::ParseLength(theApp.GetUnits(), String);
	GetDlgItemTextW(IDC_TARGET_Z, String, 32);
	Target.z = AeSys::ParseLength(theApp.GetUnits(), String);
	GetDlgItemTextW(IDC_FRONT_CLIP_DISTANCE, String, 32);
	const auto NearClipDistance {AeSys::ParseLength(theApp.GetUnits(), String)};
	GetDlgItemTextW(IDC_BACK_CLIP_DISTANCE, String, 32);
	const auto FarClipDistance {AeSys::ParseLength(theApp.GetUnits(), String)};
	GetDlgItemTextW(IDC_LENS_LENGTH, String, 32);
	const auto LensLength {AeSys::ParseLength(theApp.GetUnits(), String)};
	const auto Direction {Position - Target};
	// <tas="Is the direction reversed?"</tas>
	auto UpVector {Direction.crossProduct(OdGeVector3d::kZAxis)};
	UpVector = UpVector.crossProduct(Direction);
	if (UpVector.isZeroLength()) {
		UpVector = OdGeVector3d::kYAxis;
	} else {
		UpVector.normalize();
	}
	ModelView->SetLensLength(LensLength);
	ModelView->SetNearClipDistance(NearClipDistance);
	ModelView->SetFarClipDistance(FarClipDistance);
	ModelView->EnablePerspective(m_PerspectiveProjection == TRUE);
	const auto AspectRatio {Viewport.HeightInInches() / Viewport.WidthInInches()};
	auto FieldWidth {ModelView->FieldWidth()};
	auto FieldHeight {ModelView->FieldHeight()};
	if (AspectRatio < FieldHeight / FieldWidth) {
		FieldWidth = FieldHeight / AspectRatio;
	} else {
		FieldHeight = FieldWidth * AspectRatio;
	}
	ModelView->SetView(Position, Target, UpVector, FieldWidth, FieldHeight);
	ModelView->BuildTransformMatrix();
	ActiveView->SetViewTransform(*ModelView);
	ActiveView->InvalidateRect(nullptr);
	GetDlgItem(IDC_APPLY)->EnableWindow(FALSE);
}

BOOL EoDlgViewParameters::OnInitDialog() {
	CDialog::OnInitDialog();
	const EoGsViewTransform* ModelView = reinterpret_cast<EoGsViewTransform*>(m_ModelView);
	const auto Units {max(theApp.GetUnits(), AeSys::kEngineering)};
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
