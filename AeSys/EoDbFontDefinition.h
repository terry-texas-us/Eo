#pragma once

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

	EoDbFontDefinition& operator=(const EoDbFontDefinition& other);

	CString FormatHorizonatlAlignment() const;
	CString FormatPath() const;
	CString FormatPrecision() const;
	CString FormatVerticalAlignment() const;

public:
	double CharacterSpacing() const;
	EoDb::HorizontalAlignment HorizontalAlignment() const;
	CString FontName() const;
	EoDb::Path Path() const;
	EoDb::Precision Precision() const;
	EoDb::VerticalAlignment VerticalAlignment() const;
	void SetCharacterSpacing(double spacing);
	void SetFontName(const CString& fontName);
	void SetHorizontalAlignment(EoDb::HorizontalAlignment horizontalAlignment);
	void SetPath(EoDb::Path path);
	void SetPrecision(EoDb::Precision precision);
	void SetVerticalAlignment(EoDb::VerticalAlignment verticalAlignment);

	void Read(EoDbFile& file);
	void Write(EoDbFile& file) const;

    void EoDbFontDefinition::SetTo(OdDbTextStyleTableRecordPtr textStyleTableRecord);
    void EoDbFontDefinition::SetJustification(OdDb::TextHorzMode horizontalMode, OdDb::TextVertMode verticalMode);
    void EoDbFontDefinition::SetJustification(OdDbMText::AttachmentPoint attachmentPoint);
};