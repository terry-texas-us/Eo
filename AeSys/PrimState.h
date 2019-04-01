#pragma once

class CPrimState {
	OdInt16 m_PointDisplayMode;
	OdInt16 m_ColorIndex;
	OdInt16 m_LinetypeIndex;
	EoDbFontDefinition m_FontDefinition;
	EoDbCharacterCellDefinition m_CharacterCellDefinition;
	OdInt16 m_HatchInteriorStyle;
	size_t m_HatchInteriorStyleIndex;

public: // Constructors and destructor
public: // Operators
	const CPrimState& operator=(const CPrimState&);

public: // Methods
	EoDbCharacterCellDefinition CharacterCellDefinition() const;
	OdInt16 ColorIndex() const;
	EoDbFontDefinition FontDefinition() const;
	OdInt16 HatchInteriorStyle() const;
	size_t HatchInteriorStyleIndex() const;
	OdInt16 LinetypeIndex() const;
	/// <summary>Manages a small set of pen definitions.</summary>
	void ManagePenResources(CDC* deviceContext, OdInt16 colorIndex, int penWidth, OdInt16 linetypeIndex);
	OdInt16 PointDisplayMode() const;
	void Restore(CDC* deviceContext, int saveIndex);
	int Save();
	void SetCharacterCellDefinition(const EoDbCharacterCellDefinition& characterCellDefinition);
	void SetColorIndex(CDC* deviceContext, OdInt16 colorIndex);
	void SetFontDefinition(CDC* deviceContext, const EoDbFontDefinition& fontDefinition);
	void SetLinetypeIndex(CDC* deviceContext, OdInt16 linetypeIndex);
	void SetPointDisplayMode(OdInt16 pointDisplayMode);
	void SetHatchInteriorStyle(OdInt16 interiorStyle);
	void SetHatchInteriorStyleIndex(size_t styleIndex);
	void SetPen(AeSysView* view, CDC* deviceContext, OdInt16 colorIndex, OdInt16 linetypeIndex);
	int SetROP2(CDC* deviceContext, int drawMode);
	void SetTxtAlign(CDC* deviceContext, EoDb::HorizontalAlignment horizontalAlignment, EoDb::VerticalAlignment verticalAlignment);
};
extern CPrimState pstate;
