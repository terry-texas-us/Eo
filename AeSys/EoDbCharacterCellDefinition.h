#pragma once

class EoDbCharacterCellDefinition {
	double m_Height;
	double m_WidthFactor;
	double m_RotationAngle;
	double m_ObliqueAngle;

public:

	EoDbCharacterCellDefinition();
	EoDbCharacterCellDefinition(const EoDbCharacterCellDefinition& other) noexcept;

	EoDbCharacterCellDefinition& operator=(const EoDbCharacterCellDefinition& other) noexcept;

	double WidthFactor() const noexcept;
	double Height() const noexcept;
	double ObliqueAngle() const noexcept;
	double RotationAngle() const noexcept;
	void SetWidthFactor(double widthFactor) noexcept;
	void SetHeight(double height) noexcept;
	void SetObliqueAngle(double obliqueAngle) noexcept;
	void SetRotationAngle(double rotationAngle) noexcept;
};
