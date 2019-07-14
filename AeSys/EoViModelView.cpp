#include "stdafx.h"
#include "AeSysView.h"

OdGeVector3d AeSysView::CameraDirection() const {
	auto Direction {m_ViewTransform.Position() - m_ViewTransform.Target()};
	Direction.normalize();
	return Direction;
}

OdGePoint3d AeSysView::CameraTarget() const noexcept {
	return m_ViewTransform.Target();
}

void AeSysView::CopyActiveModelViewToPreviousModelView() noexcept {
	m_PreviousViewTransform = m_ViewTransform;
}

void AeSysView::ExchangeActiveAndPreviousModelViews() {
	const auto ModelView {m_ViewTransform};
	m_ViewTransform = m_PreviousViewTransform;
	m_PreviousViewTransform = ModelView;
}

EoGsViewTransform AeSysView::PreviousModelView() const {
	return m_PreviousViewTransform;
}

void AeSysView::SetCameraPosition(const OdGeVector3d& direction) {
	const auto Position {m_ViewTransform.Target() + direction.normal() * m_ViewTransform.LensLength()};
	m_ViewTransform.SetPosition_(Position);
	m_ViewTransform.BuildTransformMatrix();
}

void AeSysView::SetCameraTarget(const OdGePoint3d& target) {
	m_ViewTransform.SetTarget(target);
	m_ViewTransform.BuildTransformMatrix();
}

void AeSysView::SetProjectionPlaneField(const double fieldWidth, const double fieldHeight) {
	m_ViewTransform.SetProjectionPlaneField(fieldWidth, fieldHeight);
	m_ViewTransform.BuildTransformMatrix();
}

void AeSysView::SetViewTransform(EoGsViewTransform& viewTransform) {
	m_ViewTransform = viewTransform;
	m_ViewTransform.BuildTransformMatrix();
}

void AeSysView::SetView(const OdGePoint3d& position, const OdGePoint3d& target, const OdGeVector3d& upVector, const double fieldWidth, const double fieldHeight) {
	m_ViewTransform.SetView(position, target, upVector, fieldWidth, fieldHeight);
	m_ViewTransform.BuildTransformMatrix();
}

void AeSysView::SetViewWindow(const double uMin, const double vMin, const double uMax, const double vMax) {
	m_ViewTransform.SetProjectionPlaneField(uMin, vMin, uMax, vMax);
	m_ViewTransform.BuildTransformMatrix();
}

void AeSysView::ModelViewGetViewport(EoGsViewport& viewport) const noexcept {
	viewport = m_Viewport;
}

EoGeMatrix3d AeSysView::ModelViewMatrix() const noexcept {
	return m_ViewTransform.Matrix();
}

double AeSysView::ZoomFactor() const noexcept {
	return ViewportWidthInInches() / m_ViewTransform.FieldWidth();
}

OdGeVector3d AeSysView::ViewUp() const noexcept {
	return m_ViewTransform.ViewUp();
}

void AeSysView::ModelViewInitialize() {
	OdGsViewPtr FirstView = m_LayoutHelper->viewAt(0);
	SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
}

void AeSysView::ModelTransformPoint(OdGePoint3d& point) const {
	if (m_ModelTransform.Depth() != 0) {
		point.transformBy(m_ModelTransform.ModelMatrix());
	}
}

void AeSysView::ModelViewTransformPoint(EoGePoint4d& point) const {
	auto Matrix {m_ViewTransform.Matrix()};
	if (m_ModelTransform.Depth() != 0) {
		Matrix.postMultBy(m_ModelTransform.ModelMatrix());
	}
	point.TransformBy(Matrix);
}

void AeSysView::ModelViewTransformPoints(EoGePoint4dArray& points) const {
	const auto NumberOfPoints {static_cast<int>(points.GetSize())};
	auto Matrix {m_ViewTransform.Matrix()};
	if (m_ModelTransform.Depth() != 0) {
		Matrix.postMultBy(m_ModelTransform.ModelMatrix());
	}
	for (auto PointIndex = 0; PointIndex < NumberOfPoints; PointIndex++) {
		points[PointIndex].TransformBy(Matrix);
	}
}

void AeSysView::ModelViewTransformPoints(const int numberOfPoints, EoGePoint4d* points) const {
	auto Matrix {m_ViewTransform.Matrix()};
	if (m_ModelTransform.Depth() != 0) {
		Matrix.postMultBy(m_ModelTransform.ModelMatrix());
	}
	for (auto PointIndex = 0; PointIndex < numberOfPoints; PointIndex++) {
		points[PointIndex].TransformBy(Matrix);
	}
}

void AeSysView::ModelViewTransformVector(OdGeVector3d& vector) const {
	auto Matrix {m_ViewTransform.Matrix()};
	if (m_ModelTransform.Depth() != 0) {
		Matrix.postMultBy(m_ModelTransform.ModelMatrix());
	}
	vector.transformBy(Matrix);
}
