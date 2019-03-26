#include "stdafx.h"

EoDbCharacterCellDefinition::EoDbCharacterCellDefinition() : m_Height(.1), m_WidthFactor(1.) , m_RotationAngle(0.) , m_ObliqueAngle(0.) {
}
EoDbCharacterCellDefinition::EoDbCharacterCellDefinition(const EoDbCharacterCellDefinition& other) {
	m_Height = other.m_Height;
	m_WidthFactor = other.m_WidthFactor;
	m_RotationAngle = other.m_RotationAngle;
	m_ObliqueAngle = other.m_ObliqueAngle;
}
EoDbCharacterCellDefinition& EoDbCharacterCellDefinition::operator=(const EoDbCharacterCellDefinition& other) {
	m_Height = other.m_Height;
	m_WidthFactor = other.m_WidthFactor;
	m_RotationAngle = other.m_RotationAngle;
	m_ObliqueAngle = other.m_ObliqueAngle;

	return (*this);
}
double EoDbCharacterCellDefinition::WidthFactor() const {
	return (m_WidthFactor);
}
double EoDbCharacterCellDefinition::Height() const {
	return (m_Height);
}
double EoDbCharacterCellDefinition::ObliqueAngle() const {
	return (m_ObliqueAngle);
}
double EoDbCharacterCellDefinition::RotationAngle() const {
	return (m_RotationAngle);
}
void EoDbCharacterCellDefinition::SetWidthFactor(double widthFactor) {
	m_WidthFactor = widthFactor;
}
void EoDbCharacterCellDefinition::SetHeight(double height) {
	m_Height = height;
}
void EoDbCharacterCellDefinition::SetObliqueAngle(double obliqueAngle) {
	m_ObliqueAngle = obliqueAngle;
}
void EoDbCharacterCellDefinition::SetRotationAngle(double rotationAngle) {
	m_RotationAngle = rotationAngle;
}
