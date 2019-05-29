#pragma once

#include "EoGeReferenceSystem.h"
#include "EoDbCharacterCellDefinition.h"
#include "EoDbFontDefinition.h"
#include "PrimState.h"

class CPrimState {
	OdInt16 m_PointDisplayMode;
	OdInt16 m_ColorIndex;
	OdInt16 m_LinetypeIndex;
	EoDbFontDefinition m_FontDefinition;
	EoDbCharacterCellDefinition m_CharacterCellDefinition;
	OdInt16 m_HatchInteriorStyle;
	unsigned m_HatchInteriorStyleIndex;

public: // Constructors and destructor

	const CPrimState& operator=(const CPrimState& other) noexcept;

public: // Methods

	EoDbCharacterCellDefinition CharacterCellDefinition() const noexcept;
	OdInt16 ColorIndex() const noexcept;
	EoDbFontDefinition FontDefinition() const noexcept;
	OdInt16 HatchInteriorStyle() const noexcept;
	unsigned HatchInteriorStyleIndex() const noexcept;
	OdInt16 LinetypeIndex() const noexcept;
	/// <summary>Manages a small set of pen definitions.</summary>
	void ManagePenResources(CDC* deviceContext, OdInt16 colorIndex, int penWidth, OdInt16 linetypeIndex);
	OdInt16 PointDisplayMode() const noexcept;
	void Restore(CDC* deviceContext, int saveIndex);
	int Save();
	void SetCharacterCellDefinition(const EoDbCharacterCellDefinition& characterCellDefinition) noexcept;
	void SetColorIndex(CDC* deviceContext, OdInt16 colorIndex);
	void SetFontDefinition(CDC* deviceContext, const EoDbFontDefinition& fontDefinition);
	void SetLinetypeIndexPs(CDC* deviceContext, OdInt16 linetypeIndex);
	void SetPointDisplayMode(OdInt16 pointDisplayMode) noexcept;
	void SetHatchInteriorStyle(OdInt16 interiorStyle) noexcept;
	void SetHatchInteriorStyleIndex(unsigned styleIndex) noexcept;
	void SetPen(AeSysView* view, CDC* deviceContext, OdInt16 colorIndex, OdInt16 linetypeIndex) noexcept;
	int SetROP2(CDC* deviceContext, int drawMode);
	void SetTxtAlign(CDC* deviceContext, EoDb::HorizontalAlignment horizontalAlignment, EoDb::VerticalAlignment verticalAlignment);
};
extern CPrimState pstate;
