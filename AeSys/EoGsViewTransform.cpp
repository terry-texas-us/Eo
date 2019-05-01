#include "stdafx.h"

#include "EoGsViewTransform.h"

EoGsViewTransform::EoGsViewTransform() {
}
EoGsViewTransform::EoGsViewTransform(const EoGsViewTransform& other)
	: EoGsAbstractView(other) {
	m_Matrix = other.m_Matrix;
	m_ViewMatrix = other.m_ViewMatrix;
	m_ProjectionMatrix = other.m_ProjectionMatrix;
}
EoGsViewTransform::~EoGsViewTransform() {
}

EoGsViewTransform& EoGsViewTransform::operator=(const EoGsViewTransform& other) noexcept {
	EoGsAbstractView::operator=(other);
	m_Matrix = other.m_Matrix;
	m_ViewMatrix = other.m_ViewMatrix;
	m_ProjectionMatrix = other.m_ProjectionMatrix;

	return *this;
}
void EoGsViewTransform::BuildTransformMatrix() {
	m_Matrix.SetToViewTransform(Position(), Target(), ViewUp());
	if (IsPerspectiveOn()) {
		m_ProjectionMatrix.SetToPerspectiveProjection(m_FieldWidthMinimum, m_FieldWidthMaximum, m_FieldHeightMinimum, m_FieldHeightMaximum, m_NearClipDistance, m_FarClipDistance);
	}
	else {
		m_ProjectionMatrix.SetToParallelProjection(m_FieldWidthMinimum, m_FieldWidthMaximum, m_FieldHeightMinimum, m_FieldHeightMaximum, m_NearClipDistance, m_FarClipDistance);
	}
	m_ViewMatrix = m_Matrix;
	m_Matrix.preMultBy(m_ProjectionMatrix);
}
EoGeMatrix3d EoGsViewTransform::Matrix() const noexcept {
	return m_Matrix;
}
