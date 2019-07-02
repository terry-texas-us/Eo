#pragma once
#include "EoGeReferenceSystem.h"
#include "EoDbCharacterCellDefinition.h"
#include "EoDbFontDefinition.h"
#include "PrimState.h"

class CPrimState {
	short m_PointDisplayMode {0};
	short m_ColorIndex {0};
	short m_LinetypeIndex {0};
	short m_HatchInteriorStyle {0};
	unsigned m_HatchInteriorStyleIndex {0};
	EoDbFontDefinition m_FontDefinition;
	EoDbCharacterCellDefinition m_CharacterCellDefinition;
public:
	CPrimState& operator=(const CPrimState& other) noexcept;

	EoDbCharacterCellDefinition CharacterCellDefinition() const noexcept;

	short ColorIndex() const noexcept;

	EoDbFontDefinition FontDefinition() const noexcept;

	short HatchInteriorStyle() const noexcept;

	unsigned HatchInteriorStyleIndex() const noexcept;

	short LinetypeIndex() const noexcept;
	/// <summary>Manages a small set of pen definitions.</summary>
	void ManagePenResources(CDC& deviceContext, short colorIndex, int penWidth, short linetypeIndex);

	short PointDisplayMode() const noexcept;

	void Restore(CDC& deviceContext, int saveIndex);

	int Save();

	void SetCharacterCellDefinition(const EoDbCharacterCellDefinition& characterCellDefinition) noexcept;

	void SetColorIndex(CDC* deviceContext, short colorIndex);

	void SetFontDefinition(CDC* deviceContext, const EoDbFontDefinition& fontDefinition);

	void SetLinetypeIndexPs(CDC* deviceContext, short linetypeIndex);

	void SetPointDisplayMode(short pointDisplayMode) noexcept;

	void SetHatchInteriorStyle(short interiorStyle) noexcept;

	void SetHatchInteriorStyleIndex(unsigned styleIndex) noexcept;

	void SetPen(AeSysView* view, CDC* deviceContext, short colorIndex, short linetypeIndex) noexcept;

	int SetROP2(CDC& deviceContext, int drawMode);

	void SetTxtAlign(CDC* deviceContext, EoDb::HorizontalAlignment horizontalAlignment, EoDb::VerticalAlignment verticalAlignment);
};

extern CPrimState g_PrimitiveState;
