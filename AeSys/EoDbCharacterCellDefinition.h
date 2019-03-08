#pragma once

class EoDbCharacterCellDefinition {
	double m_Height;
	double m_WidthFactor;
	double m_RotationAngle;
	double m_ObliqueAngle;

public:

	EoDbCharacterCellDefinition();
	EoDbCharacterCellDefinition(const EoDbCharacterCellDefinition& other);

	EoDbCharacterCellDefinition& operator=(const EoDbCharacterCellDefinition& other);

	double WidthFactor() const;
	double Height() const;
	double ObliqueAngle() const;
	double RotationAngle() const;
	void SetWidthFactor(double widthFactor);
	void SetHeight(double height);
	void SetObliqueAngle(double obliqueAngle);
	void SetRotationAngle(double rotationAngle);
};
/// <summary>Produces the reference system vectors for a single charater cell.</summary>
void CharCellDef_EncdRefSys(const OdGeVector3d& normal, const EoDbCharacterCellDefinition& characterCellDefinition, OdGeVector3d& xAxisReference, OdGeVector3d& yAxisReference);
