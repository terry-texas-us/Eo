#include "stdafx.h"

#include "EoGsAbstractView.h"

EoGsAbstractView::EoGsAbstractView() {
	m_ViewMode = 0; // Parallel projection, No front clipping, No back clipping, 
	m_RenderMode = OdGsView::k2DOptimized;

	m_Elevation = 0.;
	m_LensLength = 50.;

	m_Position = OdGePoint3d::kOrigin + OdGeVector3d::kZAxis * m_LensLength;
	SetTarget(OdGePoint3d::kOrigin);
	SetViewUp(OdGeVector3d::kYAxis);
	m_TwistAngle = 0.;
	m_FieldWidthMinimum = - 0.5;
	m_FieldHeightMinimum = - 0.5;
	m_FieldWidthMaximum = 0.5;
	m_FieldHeightMaximum = 0.5;
	m_NearClipDistance = 20.;
	m_FarClipDistance = 100.;
}
EoGsAbstractView::EoGsAbstractView(const EoGsAbstractView& other) {
	m_ViewMode = other.m_ViewMode;
	m_RenderMode = other.m_RenderMode;

	m_Elevation = other.m_Elevation;
	m_LensLength = other.m_LensLength;

	m_Position = other.m_Position;
	m_Target = other.m_Target;
	m_ViewUp = other.m_ViewUp;
	m_TwistAngle = other.m_TwistAngle;
	m_FieldWidthMinimum = other.m_FieldWidthMinimum;
	m_FieldHeightMinimum = other.m_FieldHeightMinimum;
	m_FieldWidthMaximum = other.m_FieldWidthMaximum;
	m_FieldHeightMaximum = other.m_FieldHeightMaximum;
	m_NearClipDistance = other.m_NearClipDistance;
	m_FarClipDistance = other.m_FarClipDistance;
}
EoGsAbstractView::~EoGsAbstractView() {
}

EoGsAbstractView& EoGsAbstractView::operator=(const EoGsAbstractView& other) {
	m_ViewMode = other.m_ViewMode;
	m_RenderMode = other.m_RenderMode;

	m_Elevation = other.m_Elevation;
	m_LensLength = other.m_LensLength;

	m_Position = other.m_Position;
	m_Target = other.m_Target;
	m_ViewUp = other.m_ViewUp;
	m_TwistAngle = other.m_TwistAngle;
	m_FieldWidthMinimum = other.m_FieldWidthMinimum;
	m_FieldHeightMinimum = other.m_FieldHeightMinimum;
	m_FieldWidthMaximum = other.m_FieldWidthMaximum;
	m_FieldHeightMaximum = other.m_FieldHeightMaximum;
	m_NearClipDistance = other.m_NearClipDistance;
	m_FarClipDistance = other.m_FarClipDistance;

	return *this;
}

void EoGsAbstractView::AdjustWindow(const double aspectRatio) {
	const double FieldWidth = m_FieldWidthMaximum - m_FieldWidthMinimum;
	const double FieldHeight = m_FieldHeightMaximum - m_FieldHeightMinimum;

	if (FieldWidth <= FLT_EPSILON || FieldHeight / FieldWidth > aspectRatio) {
		const double Adjustment = (FieldHeight / aspectRatio - FieldWidth) * 0.5;
		m_FieldWidthMinimum -= Adjustment;
		m_FieldWidthMaximum += Adjustment;
	}
	else {
		const double Adjustment = (FieldWidth * aspectRatio - FieldHeight) * 0.5;
		m_FieldHeightMinimum -= Adjustment;
		m_FieldHeightMaximum += Adjustment;
	}
}
double EoGsAbstractView::FarClipDistance(void) const {
	return m_FarClipDistance;
}
double EoGsAbstractView::FieldHeight() const {
	return (m_FieldHeightMaximum - m_FieldHeightMinimum);
}
double EoGsAbstractView::FieldHeightMaximum() const {
	return m_FieldHeightMaximum;
}
double EoGsAbstractView::FieldHeightMinimum() const {
	return m_FieldHeightMinimum;
}
double EoGsAbstractView::FieldWidth() const {
	return (m_FieldWidthMaximum - m_FieldWidthMinimum);
}
double EoGsAbstractView::FieldWidthMaximum() const {
	return m_FieldWidthMaximum;
}
double EoGsAbstractView::FieldWidthMinimum() const {
	return m_FieldWidthMinimum;
}
bool EoGsAbstractView::IsNearClipAtEyeOn() const {
	return ((m_ViewMode & AV_NEARCLIPPINGATEYE) == AV_NEARCLIPPINGATEYE);
}
bool EoGsAbstractView::IsNearClipOn() const {
	return ((m_ViewMode & AV_NEARCLIPPING) == AV_NEARCLIPPING);
}
bool EoGsAbstractView::IsFarClipOn() const {
	return ((m_ViewMode & AV_FARCLIPPING) == AV_FARCLIPPING);
}
bool EoGsAbstractView::IsPerspectiveOn() const {
	return ((m_ViewMode & AV_PERSPECTIVE) == AV_PERSPECTIVE);
}
double EoGsAbstractView::LensLength() const {
	return m_LensLength;
}
double EoGsAbstractView::NearClipDistance(void) const {
	return m_NearClipDistance;
}
OdGePoint3d EoGsAbstractView::Position(void) const {
	return m_Position;
}
OdGsView::RenderMode EoGsAbstractView::RenderMode() const {
	return m_RenderMode;
}
void EoGsAbstractView::SetFarClipDistance(const double distance) {
	m_FarClipDistance = distance;
}
void EoGsAbstractView::SetRenderMode(const OdGsView::RenderMode& renderMode) {
	m_RenderMode = renderMode;
}
void EoGsAbstractView::SetNearClipDistance(const double distance) {
	m_NearClipDistance = distance;
}
void EoGsAbstractView::SetLensLength(const double length) {
	m_LensLength = length;
}
void EoGsAbstractView::EnablePerspective(bool enabled) {
	if (enabled) {
		m_ViewMode |= AV_PERSPECTIVE;
	}
	else {
		m_ViewMode &= ~AV_PERSPECTIVE;
	}
}
void EoGsAbstractView::SetPosition_(const OdGePoint3d& position) {
	m_Position = position;
}
void EoGsAbstractView::SetProjectionPlaneField(double fieldWidth, double fieldHeight) {
	m_FieldWidthMinimum = - fieldWidth * 0.5;
	m_FieldHeightMinimum = - fieldHeight * 0.5;
	m_FieldWidthMaximum = m_FieldWidthMinimum + fieldWidth;
	m_FieldHeightMaximum = m_FieldHeightMinimum + fieldHeight;
}
void EoGsAbstractView::SetTarget(const OdGePoint3d& target) {
	m_Target = target;
}
void EoGsAbstractView::SetView(const OdGePoint3d& position, const OdGePoint3d& target, const OdGeVector3d& viewUp, double fieldWidth, double fieldHeight) {
	m_Position = position;
	m_Target = target;
	SetViewUp(viewUp);
	m_FieldWidthMinimum = - fieldWidth * 0.5;
	m_FieldHeightMinimum = - fieldHeight * 0.5;
	m_FieldWidthMaximum = m_FieldWidthMinimum + fieldWidth;
	m_FieldHeightMaximum = m_FieldHeightMinimum + fieldHeight;
}
void EoGsAbstractView::SetViewUp(const OdGeVector3d& viewUp) {
	if (!viewUp.isZeroLength()) {
		m_ViewUp = viewUp.normal();
	}
}
void EoGsAbstractView::SetProjectionPlaneField(const double uMin, const double vMin, const double uMax, const double vMax) {
	m_FieldWidthMinimum = uMin;
	m_FieldHeightMinimum = vMin;
	m_FieldWidthMaximum = uMax;
	m_FieldHeightMaximum = vMax;
}
OdGePoint3d EoGsAbstractView::Target(void) const {
	return m_Target;
}
OdGeVector3d EoGsAbstractView::ViewUp() const {
	return m_ViewUp;
}
