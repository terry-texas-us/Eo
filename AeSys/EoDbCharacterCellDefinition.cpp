#include "stdafx.h"

#include "EoDbCharacterCellDefinition.h"

EoDbCharacterCellDefinition::EoDbCharacterCellDefinition(const EoDbCharacterCellDefinition& other) noexcept {
	m_Height = other.m_Height;
	m_WidthFactor = other.m_WidthFactor;
	m_RotationAngle = other.m_RotationAngle;
	m_ObliqueAngle = other.m_ObliqueAngle;
}

double EoDbCharacterCellDefinition::WidthFactor() const noexcept {
	return m_WidthFactor;
}

double EoDbCharacterCellDefinition::Height() const noexcept {
	return m_Height;
}

double EoDbCharacterCellDefinition::ObliqueAngle() const noexcept {
	return m_ObliqueAngle;
}

double EoDbCharacterCellDefinition::RotationAngle() const noexcept {
	return m_RotationAngle;
}

void EoDbCharacterCellDefinition::SetWidthFactor(double widthFactor) noexcept {
	m_WidthFactor = widthFactor;
}

void EoDbCharacterCellDefinition::SetHeight(double height) noexcept {
	m_Height = height;
}

void EoDbCharacterCellDefinition::SetObliqueAngle(double obliqueAngle) noexcept {
	m_ObliqueAngle = obliqueAngle;
}

void EoDbCharacterCellDefinition::SetRotationAngle(double rotationAngle) noexcept {
	m_RotationAngle = rotationAngle;
}
