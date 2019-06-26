#pragma once
#include "EoGePoint3d.h"
class AeSysView;
class EoDbCharacterCellDefinition;
class EoDbFile;

class EoGeReferenceSystem {
	OdGePoint3d m_Origin {OdGePoint3d::kOrigin};
	OdGeVector3d m_XDirection {OdGeVector3d::kXAxis};
	OdGeVector3d m_YDirection {OdGeVector3d::kYAxis};
public:
	EoGeReferenceSystem() = default;
	EoGeReferenceSystem(const OdGePoint3d& origin, AeSysView* view, const EoDbCharacterCellDefinition& characterCellDefinition);
	EoGeReferenceSystem(const OdGePoint3d& origin, const OdGeVector3d& xDirection, const OdGeVector3d& yDirection) noexcept;
	EoGeReferenceSystem(const OdGePoint3d& origin, const OdGeVector3d& normal, const EoDbCharacterCellDefinition& characterCellDefinition);
	EoGeReferenceSystem(const EoGeReferenceSystem& other);
	EoGeReferenceSystem& operator=(const EoGeReferenceSystem& other) = default;
	~EoGeReferenceSystem() = default;

	void GetUnitNormal(OdGeVector3d& normal);
	[[nodiscard]] OdGePoint3d Origin() const noexcept;
	void Read(EoDbFile& file);
	/// <summary>Takes the current reference directions and rescales using passed character cell state.</summary>
	void Rescale(const EoDbCharacterCellDefinition& characterCellDefinition);
	[[nodiscard]] double Rotation() const noexcept;
	void Set(const OdGePoint3d& origin, const OdGeVector3d& xDirection, const OdGeVector3d& yDirection) noexcept;
	void SetOrigin(const OdGePoint3d& origin) noexcept;
	void SetXDirection(const OdGeVector3d& xDirection) noexcept;
	void SetYDirection(const OdGeVector3d& yDirection) noexcept;
	void TransformBy(const EoGeMatrix3d& transformMatrix);
	[[nodiscard]] EoGeMatrix3d TransformMatrix() const;
	void Write(EoDbFile& file) const;
	[[nodiscard]] OdGeVector3d XDirection() const noexcept;
	[[nodiscard]] OdGeVector3d YDirection() const noexcept;
};
