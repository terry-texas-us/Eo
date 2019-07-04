#pragma once
class EoDbCharacterCellDefinition {
	double m_Height {0.1};
	double m_WidthFactor {1.0};
	double m_RotationAngle {0.0};
	double m_ObliqueAngle {0.0};
public:
	EoDbCharacterCellDefinition() = default;

	EoDbCharacterCellDefinition(const EoDbCharacterCellDefinition& other) noexcept;

	EoDbCharacterCellDefinition& operator=(const EoDbCharacterCellDefinition& other) = default;

	[[nodiscard]] double WidthFactor() const noexcept;

	[[nodiscard]] double Height() const noexcept;

	[[nodiscard]] double ObliqueAngle() const noexcept;

	[[nodiscard]] double RotationAngle() const noexcept;

	void SetWidthFactor(double widthFactor) noexcept;

	void SetHeight(double height) noexcept;

	void SetObliqueAngle(double obliqueAngle) noexcept;

	void SetRotationAngle(double rotationAngle) noexcept;
};
