#pragma once

#include "DbMText.h"

#include "EoDb.h"

class EoDbFile;

class EoDbFontDefinition {
private:
	double m_CharacterSpacing;
	EoDb::Precision m_Precision;
	CString m_FontName;
	EoDb::Path m_Path;
	EoDb::HorizontalAlignment m_HorizontalAlignment;
	EoDb::VerticalAlignment m_VerticalAlignment;

public:
	EoDbFontDefinition();
	EoDbFontDefinition(const EoDbFontDefinition& other);

	EoDbFontDefinition& operator=(const EoDbFontDefinition& other) noexcept;

	CString FormatHorizonatlAlignment() const;
	CString FormatPath() const;
	CString FormatPrecision() const;
	CString FormatVerticalAlignment() const;

public:
	double CharacterSpacing() const noexcept;
	EoDb::HorizontalAlignment HorizontalAlignment() const noexcept;
	CString FontName() const;
	EoDb::Path Path() const noexcept;
	EoDb::Precision Precision() const noexcept;
	EoDb::VerticalAlignment VerticalAlignment() const noexcept;
	void SetCharacterSpacing(double spacing) noexcept;
	void SetFontName(const CString& fontName);
	void SetHorizontalAlignment(EoDb::HorizontalAlignment horizontalAlignment) noexcept;
	void SetPath(EoDb::Path path) noexcept;
	void SetPrecision(EoDb::Precision precision) noexcept;
	void SetVerticalAlignment(EoDb::VerticalAlignment verticalAlignment) noexcept;

	void Read(EoDbFile& file);
	void Write(EoDbFile& file) const;

	void EoDbFontDefinition::SetTo(OdDbTextStyleTableRecordPtr textStyleTableRecord) noexcept;
	void EoDbFontDefinition::SetJustification(OdDb::TextHorzMode horizontalMode, OdDb::TextVertMode verticalMode) noexcept;
	void EoDbFontDefinition::SetJustification(OdDbMText::AttachmentPoint attachmentPoint) noexcept;
};