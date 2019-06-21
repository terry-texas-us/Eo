#include "stdafx.h"
#include "AeSysView.h"
#include "EoDbFile.h"

EoGeReferenceSystem::EoGeReferenceSystem(const OdGePoint3d& origin, AeSysView* view, const EoDbCharacterCellDefinition& characterCellDefinition)
	: m_Origin(origin) {
	const auto PlaneNormal {view->CameraDirection()};
	m_YDirection = view->ViewUp();
	m_YDirection.rotateBy(characterCellDefinition.RotationAngle(), PlaneNormal);
	m_XDirection = m_YDirection;
	m_XDirection.rotateBy(-OdaPI2, PlaneNormal);
	m_YDirection.rotateBy(characterCellDefinition.ObliqueAngle(), PlaneNormal);
	m_XDirection *= 0.6 * characterCellDefinition.Height() * characterCellDefinition.WidthFactor();
	m_YDirection *= characterCellDefinition.Height();
}

EoGeReferenceSystem::EoGeReferenceSystem(const OdGePoint3d& origin, const OdGeVector3d& xDirection, const OdGeVector3d& yDirection) noexcept
	: m_Origin(origin)
	, m_XDirection(xDirection)
	, m_YDirection(yDirection) {
}

EoGeReferenceSystem::EoGeReferenceSystem(const OdGePoint3d& origin, const OdGeVector3d& normal, const EoDbCharacterCellDefinition& characterCellDefinition)
	: m_Origin(origin) {
	m_XDirection = ComputeArbitraryAxis(normal);
	m_XDirection.rotateBy(characterCellDefinition.RotationAngle(), normal);
	m_YDirection = normal.crossProduct(m_XDirection);
	m_XDirection *= 0.6 * characterCellDefinition.Height() * characterCellDefinition.WidthFactor();
	m_YDirection.rotateBy(characterCellDefinition.ObliqueAngle(), normal);
	m_YDirection *= characterCellDefinition.Height();
}

EoGeReferenceSystem::EoGeReferenceSystem(const EoGeReferenceSystem& other) {
	m_Origin = other.m_Origin;
	m_XDirection = other.m_XDirection;
	m_YDirection = other.m_YDirection;
}

// <tas="Likely misuse of .normal"/>
void EoGeReferenceSystem::GetUnitNormal(OdGeVector3d& normal) {
	normal = m_XDirection.crossProduct(m_YDirection).normal();
}

OdGePoint3d EoGeReferenceSystem::Origin() const noexcept {
	return m_Origin;
}

EoGeMatrix3d EoGeReferenceSystem::TransformMatrix() const {
	EoGeMatrix3d Matrix;
	Matrix.setToWorldToPlane(OdGePlane(m_Origin, m_XDirection, m_YDirection));
	return Matrix;
}

void EoGeReferenceSystem::Read(EoDbFile& file) {
	m_Origin = file.ReadPoint3d();
	m_XDirection = file.ReadVector3d();
	m_YDirection = file.ReadVector3d();
}

void EoGeReferenceSystem::Rescale(const EoDbCharacterCellDefinition& characterCellDefinition) {
	OdGeVector3d PlaneNormal;
	GetUnitNormal(PlaneNormal);
	m_XDirection.normalize();
	m_YDirection = m_XDirection;
	m_YDirection.rotateBy(OdaPI2 + characterCellDefinition.ObliqueAngle(), PlaneNormal);
	m_XDirection *= 0.6 * characterCellDefinition.Height() * characterCellDefinition.WidthFactor();
	m_YDirection *= characterCellDefinition.Height();
}

double EoGeReferenceSystem::Rotation() const noexcept {
	const auto HorizontalAxis {m_XDirection};
	auto Angle {0.0};
	Angle = atan2(HorizontalAxis.y, HorizontalAxis.x); // -pi to pi radians
	if (Angle < 0.0) {
		Angle += Oda2PI;
	}
	return Angle;
}

void EoGeReferenceSystem::Set(const OdGePoint3d& origin, const OdGeVector3d& xDirection, const OdGeVector3d& yDirection) noexcept {
	m_Origin = origin;
	m_XDirection = xDirection;
	m_YDirection = yDirection;
}

void EoGeReferenceSystem::SetOrigin(const OdGePoint3d& origin) noexcept {
	m_Origin = origin;
}

void EoGeReferenceSystem::SetXDirection(const OdGeVector3d& xDirection) noexcept {
	m_XDirection = xDirection;
}

void EoGeReferenceSystem::SetYDirection(const OdGeVector3d& yDirection) noexcept {
	m_YDirection = yDirection;
}

void EoGeReferenceSystem::TransformBy(const EoGeMatrix3d& transformMatrix) {
	m_Origin.transformBy(transformMatrix);
	m_XDirection.transformBy(transformMatrix);
	m_YDirection.transformBy(transformMatrix);
}

void EoGeReferenceSystem::Write(EoDbFile& file) const {
	file.WritePoint3d(m_Origin);
	file.WriteVector3d(m_XDirection);
	file.WriteVector3d(m_YDirection);
}

OdGeVector3d EoGeReferenceSystem::XDirection() const noexcept {
	return m_XDirection;
}

OdGeVector3d EoGeReferenceSystem::YDirection() const noexcept {
	return m_YDirection;
}
