#include "stdafx.h"

#include "EoGsAbstractView.h"

EoGsAbstractView::EoGsAbstractView() {
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

void EoGsAbstractView::AdjustWindow(const double aspectRatio) noexcept {
	const double FieldWidth = m_FieldWidthMaximum - m_FieldWidthMinimum;
	const double FieldHeight = m_FieldHeightMaximum - m_FieldHeightMinimum;

	if (FieldWidth <= FLT_EPSILON || FieldHeight / FieldWidth > aspectRatio) {
		const double Adjustment = (FieldHeight / aspectRatio - FieldWidth) * 0.5;
		m_FieldWidthMinimum -= Adjustment;
		m_FieldWidthMaximum += Adjustment;
	} else {
		const double Adjustment = (FieldWidth * aspectRatio - FieldHeight) * 0.5;
		m_FieldHeightMinimum -= Adjustment;
		m_FieldHeightMaximum += Adjustment;
	}
}

double EoGsAbstractView::FarClipDistance() const noexcept {
	return m_FarClipDistance;
}

double EoGsAbstractView::FieldHeight() const noexcept {
	return (m_FieldHeightMaximum - m_FieldHeightMinimum);
}

double EoGsAbstractView::FieldHeightMaximum() const noexcept {
	return m_FieldHeightMaximum;
}

double EoGsAbstractView::FieldHeightMinimum() const noexcept {
	return m_FieldHeightMinimum;
}

double EoGsAbstractView::FieldWidth() const noexcept {
	return (m_FieldWidthMaximum - m_FieldWidthMinimum);
}

double EoGsAbstractView::FieldWidthMaximum() const noexcept {
	return m_FieldWidthMaximum;
}

double EoGsAbstractView::FieldWidthMinimum() const noexcept {
	return m_FieldWidthMinimum;
}

bool EoGsAbstractView::IsNearClipAtEyeOn() const noexcept {
	return ((m_ViewMode & AV_NEARCLIPPINGATEYE) == AV_NEARCLIPPINGATEYE);
}

bool EoGsAbstractView::IsNearClipOn() const noexcept {
	return ((m_ViewMode & AV_NEARCLIPPING) == AV_NEARCLIPPING);
}

bool EoGsAbstractView::IsFarClipOn() const noexcept {
	return ((m_ViewMode & AV_FARCLIPPING) == AV_FARCLIPPING);
}

bool EoGsAbstractView::IsPerspectiveOn() const noexcept {
	return ((m_ViewMode & AV_PERSPECTIVE) == AV_PERSPECTIVE);
}

double EoGsAbstractView::LensLength() const noexcept {
	return m_LensLength;
}

double EoGsAbstractView::NearClipDistance() const noexcept {
	return m_NearClipDistance;
}

OdGePoint3d EoGsAbstractView::Position() const noexcept {
	return m_Position;
}

OdGsView::RenderMode EoGsAbstractView::RenderMode() const noexcept {
	return m_RenderMode;
}

void EoGsAbstractView::SetFarClipDistance(const double distance) noexcept {
	m_FarClipDistance = distance;
}

void EoGsAbstractView::SetRenderMode(const OdGsView::RenderMode& renderMode) noexcept {
	m_RenderMode = renderMode;
}

void EoGsAbstractView::SetNearClipDistance(const double distance) noexcept {
	m_NearClipDistance = distance;
}

void EoGsAbstractView::SetLensLength(const double length) noexcept {
	m_LensLength = length;
}

void EoGsAbstractView::EnablePerspective(bool enabled) noexcept {
	if (enabled) {
		m_ViewMode |= AV_PERSPECTIVE;
	} else {
		m_ViewMode &= ~AV_PERSPECTIVE;
	}
}

void EoGsAbstractView::SetPosition_(const OdGePoint3d& position) noexcept {
	m_Position = position;
}

void EoGsAbstractView::SetProjectionPlaneField(double fieldWidth, double fieldHeight) noexcept {
	m_FieldWidthMinimum = - fieldWidth * 0.5;
	m_FieldHeightMinimum = - fieldHeight * 0.5;
	m_FieldWidthMaximum = m_FieldWidthMinimum + fieldWidth;
	m_FieldHeightMaximum = m_FieldHeightMinimum + fieldHeight;
}

void EoGsAbstractView::SetTarget(const OdGePoint3d& target) noexcept {
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

void EoGsAbstractView::SetProjectionPlaneField(const double uMin, const double vMin, const double uMax, const double vMax) noexcept {
	m_FieldWidthMinimum = uMin;
	m_FieldHeightMinimum = vMin;
	m_FieldWidthMaximum = uMax;
	m_FieldHeightMaximum = vMax;
}

OdGePoint3d EoGsAbstractView::Target() const noexcept {
	return m_Target;
}

OdGeVector3d EoGsAbstractView::ViewUp() const noexcept {
	return m_ViewUp;
}
