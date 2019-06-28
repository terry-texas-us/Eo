#pragma once
#include "DbMText.h"
#include "EoDb.h"
class EoDbFile;

class EoDbFontDefinition {
	double m_CharacterSpacing {0.0};
	EoDb::Precision m_Precision {EoDb::kTrueType};
	CString m_FontName {L"Simplex"};
	EoDb::Path m_Path {EoDb::kPathRight};
	EoDb::HorizontalAlignment m_HorizontalAlignment {EoDb::kAlignLeft};
	EoDb::VerticalAlignment m_VerticalAlignment {EoDb::kAlignBottom};
public:
	EoDbFontDefinition() = default;
	EoDbFontDefinition(const EoDbFontDefinition& other);
	EoDbFontDefinition& operator=(const EoDbFontDefinition& other) = default;
	CString FormatHorizontalAlignment() const;
	CString FormatPath() const;
	CString FormatPrecision() const;
	CString FormatVerticalAlignment() const;
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
	void SetTo(OdDbTextStyleTableRecordPtr textStyleTableRecord) noexcept;
	void SetJustification(OdDb::TextHorzMode horizontalMode, OdDb::TextVertMode verticalMode) noexcept;
	void SetJustification(OdDbMText::AttachmentPoint attachmentPoint) noexcept;
};
