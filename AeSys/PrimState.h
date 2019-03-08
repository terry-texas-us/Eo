#pragma once

class CPrimState {
	EoInt16 m_PointDisplayMode;
	EoInt16 m_ColorIndex;
	EoInt16 m_LinetypeIndex;
	EoDbFontDefinition m_FontDefinition;
	EoDbCharacterCellDefinition m_CharacterCellDefinition;
	EoInt16 m_HatchInteriorStyle;
	size_t m_HatchInteriorStyleIndex;

public: // Constructors and destructor
public: // Operators
	const CPrimState& operator=(const CPrimState&);

public: // Methods
	EoDbCharacterCellDefinition CharacterCellDefinition() const;
	EoInt16 ColorIndex() const;
	EoDbFontDefinition FontDefinition() const;
	EoInt16 HatchInteriorStyle() const;
	size_t HatchInteriorStyleIndex() const;
	EoInt16 LinetypeIndex() const;
	/// <summary>Manages a small set of pen definitions.</summary>
	void ManagePenResources(CDC* deviceContext, EoInt16 colorIndex, int penWidth, EoInt16 linetypeIndex);
	EoInt16 PointDisplayMode() const;
	void Restore(CDC* deviceContext, int saveIndex);
	int Save();
	void SetCharacterCellDefinition(const EoDbCharacterCellDefinition& characterCellDefinition);
	void SetColorIndex(CDC* deviceContext, EoInt16 colorIndex);
	void SetFontDefinition(CDC* deviceContext, const EoDbFontDefinition& fontDefinition);
	void SetLinetypeIndex(CDC* deviceContext, EoInt16 linetypeIndex);
	void SetPointDisplayMode(EoInt16 pointDisplayMode);
	void SetHatchInteriorStyle(EoInt16 interiorStyle);
	void SetHatchInteriorStyleIndex(size_t styleIndex);
	void SetPen(AeSysView* view, CDC* deviceContext, EoInt16 colorIndex, EoInt16 linetypeIndex);
	int SetROP2(CDC* deviceContext, int drawMode);
	void SetTxtAlign(CDC* deviceContext, EoDb::HorizontalAlignment horizontalAlignment, EoDb::VerticalAlignment verticalAlignment);
};
extern CPrimState pstate;
