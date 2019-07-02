#include "stdafx.h"
#include "DbSymUtl.h"
#include "EoDbFile.h"
#include "EoDbText.h"

double EoDbFontDefinition::CharacterSpacing() const noexcept {
	return m_CharacterSpacing;
}

EoDb::HorizontalAlignment EoDbFontDefinition::HorizontalAlignment() const noexcept {
	return m_HorizontalAlignment;
}

CString EoDbFontDefinition::FontName() const {
	return m_FontName;
}

EoDb::Precision EoDbFontDefinition::Precision() const noexcept {
	return m_Precision;
}

EoDb::Path EoDbFontDefinition::Path() const noexcept {
	return m_Path;
}

EoDb::VerticalAlignment EoDbFontDefinition::VerticalAlignment() const noexcept {
	return m_VerticalAlignment;
}

void EoDbFontDefinition::SetCharacterSpacing(const double spacing) noexcept {
	m_CharacterSpacing = spacing;
}

void EoDbFontDefinition::SetHorizontalAlignment(const EoDb::HorizontalAlignment horizontalAlignment) noexcept {
	m_HorizontalAlignment = horizontalAlignment;
}

void EoDbFontDefinition::SetFontName(const CString& fontName) {
	m_FontName = fontName;
}

void EoDbFontDefinition::SetPath(const EoDb::Path path) noexcept {
	m_Path = path;
}

void EoDbFontDefinition::SetPrecision(const EoDb::Precision precision) noexcept {
	m_Precision = precision;
}

void EoDbFontDefinition::SetVerticalAlignment(const EoDb::VerticalAlignment verticalAlignment) noexcept {
	m_VerticalAlignment = verticalAlignment;
}

CString EoDbFontDefinition::FormatHorizontalAlignment() const {
	std::vector<const wchar_t*> HorizontalAlignments {L"Left", L"Center", L"Right"};
	return m_HorizontalAlignment >= EoDb::kAlignLeft && m_HorizontalAlignment <= EoDb::kAlignRight ? HorizontalAlignments.at(m_HorizontalAlignment - 1) : L"Invalid!";
}

CString EoDbFontDefinition::FormatPath() const {
	std::vector<const wchar_t*> Path {L"Right", L"Left", L"Up", L"Down"};
	return m_Path >= EoDb::kPathRight && m_Path <= EoDb::kPathDown ? Path.at(m_Path) : L"Invalid!";
}

CString EoDbFontDefinition::FormatPrecision() const {
	std::vector<const wchar_t*> Precision {L"True Type", L"Stroke"};
	return m_Precision >= EoDb::kTrueType && m_Precision <= EoDb::kStrokeType ? Precision.at(m_Precision - 1) : L"Invalid!";
}

CString EoDbFontDefinition::FormatVerticalAlignment() const {
	std::vector<const wchar_t*> VerticalAlignment {L"Top", L"Middle", L"Bottom"};
	return m_VerticalAlignment >= EoDb::kAlignTop && m_VerticalAlignment <= EoDb::kAlignBottom ? VerticalAlignment.at(m_VerticalAlignment - 2) : L"Invalid!";
}

void EoDbFontDefinition::Read(EoDbFile& file) {
	file.Read(&m_Precision, sizeof(unsigned short));
	file.ReadString(m_FontName);
	file.Read(&m_Path, sizeof(unsigned short));
	file.Read(&m_HorizontalAlignment, sizeof(unsigned short));
	file.Read(&m_VerticalAlignment, sizeof(unsigned short));
	file.Read(&m_CharacterSpacing, sizeof(double));
}

void EoDbFontDefinition::Write(EoDbFile& file) const {
	file.Write(&m_Precision, sizeof(unsigned short));
	file.WriteString(m_FontName);
	file.Write(&m_Path, sizeof(unsigned short));
	file.Write(&m_HorizontalAlignment, sizeof(unsigned short));
	file.Write(&m_VerticalAlignment, sizeof(unsigned short));
	file.Write(&m_CharacterSpacing, sizeof(double));
}

void EoDbFontDefinition::SetTo(OdDbTextStyleTableRecordPtr textStyleTableRecord) noexcept {
	m_FontName = L"Simplex.psf";
	m_Precision = EoDb::kStrokeType;
	if (textStyleTableRecord->isShapeFile()) {
		TRACE1("TextStyle references shape library %s.\n", ( const wchar_t*) textStyleTableRecord->desc()->name());
	} else { // shx font file or windows (ttf) font file
		OdString TypeFace;
		bool Bold;
		bool Italic;
		int Charset;
		int PitchAndFamily;
		textStyleTableRecord->font(TypeFace, Bold, Italic, Charset, PitchAndFamily);
		if (TypeFace != L"") { // windows (ttf) file
			m_FontName = static_cast<const wchar_t*>(TypeFace);
			m_Precision = EoDb::kTrueType;
		}
	}
}

void EoDbFontDefinition::SetJustification(const OdDb::TextHorzMode horizontalMode, const OdDb::TextVertMode verticalMode) noexcept {
	m_HorizontalAlignment = EoDbText::ConvertHorizontalAlignment(horizontalMode);
	m_VerticalAlignment = EoDbText::ConvertVerticalAlignment(verticalMode);
}

void EoDbFontDefinition::SetJustification(const OdDbMText::AttachmentPoint attachmentPoint) noexcept {
	switch (attachmentPoint) {
		case OdDbMText::kTopLeft:
			m_HorizontalAlignment = EoDb::kAlignLeft;
			m_VerticalAlignment = EoDb::kAlignTop;
			break;
		case OdDbMText::kTopCenter: case OdDbMText::kTopAlign: case OdDbMText::kTopFit: case OdDbMText::kTopMid:
			m_HorizontalAlignment = EoDb::kAlignCenter;
			m_VerticalAlignment = EoDb::kAlignTop;
			break;
		case OdDbMText::kTopRight:
			m_HorizontalAlignment = EoDb::kAlignRight;
			m_VerticalAlignment = EoDb::kAlignTop;
			break;
		case OdDbMText::kMiddleLeft:
			m_HorizontalAlignment = EoDb::kAlignLeft;
			m_VerticalAlignment = EoDb::kAlignMiddle;
			break;
		case OdDbMText::kMiddleCenter: case OdDbMText::kMiddleAlign: case OdDbMText::kMiddleFit: case OdDbMText::kMiddleMid:
			m_HorizontalAlignment = EoDb::kAlignCenter;
			m_VerticalAlignment = EoDb::kAlignMiddle;
			break;
		case OdDbMText::kMiddleRight:
			m_HorizontalAlignment = EoDb::kAlignRight;
			m_VerticalAlignment = EoDb::kAlignMiddle;
			break;
		case OdDbMText::kBaseCenter: case OdDbMText::kBaseAlign: case OdDbMText::kBaseFit: case OdDbMText::kBaseMid: case OdDbMText::kBottomCenter: case OdDbMText::kBottomAlign: case OdDbMText::kBottomFit: case OdDbMText::kBottomMid:
			m_HorizontalAlignment = EoDb::kAlignCenter;
			m_VerticalAlignment = EoDb::kAlignBottom;
			break;
		case OdDbMText::kBaseRight: case OdDbMText::kBottomRight:
			m_HorizontalAlignment = EoDb::kAlignRight;
			m_VerticalAlignment = EoDb::kAlignBottom;
			break;
		case OdDbMText::kBaseLeft: case OdDbMText::kBottomLeft: default:
			m_HorizontalAlignment = EoDb::kAlignLeft;
			m_VerticalAlignment = EoDb::kAlignBottom;
	}
}
