#pragma once

class EoDbCharacterCellDefinition;
class EoDbFile;

class EoGeReferenceSystem {
	OdGePoint3d	m_Origin;
	OdGeVector3d m_XDirection;
	OdGeVector3d m_YDirection;

public: // Constructors and destructor
	EoGeReferenceSystem();
	EoGeReferenceSystem(const OdGePoint3d& origin, EoDbCharacterCellDefinition& characterCellDefinition);
	EoGeReferenceSystem(const OdGePoint3d& origin, const OdGeVector3d& xDirection, const OdGeVector3d& yDirection);

	EoGeReferenceSystem(const EoGeReferenceSystem& cs);

	~EoGeReferenceSystem() {
	}
public: // Operators
	EoGeReferenceSystem& operator=(const EoGeReferenceSystem&);

public: // Methods
	void GetUnitNormal(OdGeVector3d& normal);
	OdGePoint3d Origin() const;
	void Read(EoDbFile& file);
	/// <summary>Takes the current reference directions and rescales using passed character cell state.</summary>
	void Rescale(EoDbCharacterCellDefinition& characterCellDefinition);
    double Rotation() const;
    void Set(const OdGePoint3d& origin, const OdGeVector3d& xDirection, const OdGeVector3d& yDirection);
	void SetOrigin(const OdGePoint3d& origin);
	void SetXDirection(const OdGeVector3d& xDirection);
	void SetYDirection(const OdGeVector3d& yDirection);
	void TransformBy(const EoGeMatrix3d& transformMatrix);
	EoGeMatrix3d TransformMatrix() const;
	void Write(EoDbFile& file) const;
	OdGeVector3d XDirection() const;
	OdGeVector3d YDirection() const;
};
