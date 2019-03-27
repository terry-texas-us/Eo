#include "stdafx.h"

EoDbFontDefinition::EoDbFontDefinition()
	: m_Precision(EoDb::kEoTrueType), m_FontName(L"Simplex"), m_Path(EoDb::kPathRight), m_HorizontalAlignment(EoDb::kAlignLeft), m_VerticalAlignment(EoDb::kAlignBottom), m_CharacterSpacing(0.) {
}
EoDbFontDefinition::EoDbFontDefinition(const EoDbFontDefinition& other) {
	m_Precision = other.m_Precision;
	m_FontName = other.m_FontName;
	m_Path = other.m_Path;
	m_HorizontalAlignment = other.m_HorizontalAlignment;
	m_VerticalAlignment = other.m_VerticalAlignment;
	m_CharacterSpacing = other.m_CharacterSpacing;
}
EoDbFontDefinition& EoDbFontDefinition::operator=(const EoDbFontDefinition& other) {
	m_Precision = other.m_Precision;
	m_FontName = other.m_FontName;
	m_Path = other.m_Path;
	m_HorizontalAlignment = other.m_HorizontalAlignment;
	m_VerticalAlignment = other.m_VerticalAlignment;
	m_CharacterSpacing = other.m_CharacterSpacing;
	return (*this);
}
double EoDbFontDefinition::CharacterSpacing() const {
	return m_CharacterSpacing;
}
EoDb::HorizontalAlignment EoDbFontDefinition::HorizontalAlignment() const {
	return m_HorizontalAlignment;
}
CString EoDbFontDefinition::FontName() const {
	return m_FontName;
}
EoDb::Precision EoDbFontDefinition::Precision() const {
	return m_Precision;
}
EoDb::Path EoDbFontDefinition::Path() const {
	return m_Path;
}
EoDb::VerticalAlignment EoDbFontDefinition::VerticalAlignment() const {
	return m_VerticalAlignment;
}
void EoDbFontDefinition::SetCharacterSpacing(double spacing) {
	m_CharacterSpacing = spacing;
}
void EoDbFontDefinition::SetHorizontalAlignment(EoDb::HorizontalAlignment horizontalAlignment) {
	m_HorizontalAlignment = horizontalAlignment;
}
void EoDbFontDefinition::SetFontName(const CString& fontName) {
	m_FontName = fontName;
}
void EoDbFontDefinition::SetPath(EoDb::Path path) {
	m_Path = path;
}
void EoDbFontDefinition::SetPrecision(EoDb::Precision precision) {
	m_Precision = precision;
}
void EoDbFontDefinition::SetVerticalAlignment(EoDb::VerticalAlignment verticalAlignment) {
	m_VerticalAlignment = verticalAlignment;
}
CString EoDbFontDefinition::FormatHorizonatlAlignment() const {
	CString strAlign[] = {L"Left", L"Center", L"Right"};
	return (m_HorizontalAlignment >= EoDb::kAlignLeft && m_HorizontalAlignment <= EoDb::kAlignRight) ? strAlign[m_HorizontalAlignment - 1] : L"Invalid!";
}
CString EoDbFontDefinition::FormatPath() const {
	CString strPath[] = {L"Right", L"Left", L"Up", L"Down"};
	return (m_Path >= EoDb::kPathRight && m_Path <= EoDb::kPathDown) ? strPath[m_Path] : L"Invalid!";
}
CString EoDbFontDefinition::FormatPrecision() const {
	CString strPrec[] = {L"True Type", L"Stroke"};
	return (m_Precision >= EoDb::kEoTrueType && m_Precision <= EoDb::kStrokeType) ? strPrec[m_Precision - 1] : L"Invalid!";
}
CString EoDbFontDefinition::FormatVerticalAlignment() const {
	CString strAlign[] = {L"Top", L"Middle", L"Bottom"};
	return (m_VerticalAlignment >= EoDb::kAlignTop && m_VerticalAlignment <= EoDb::kAlignBottom) ? strAlign[m_VerticalAlignment - 2] : L"Invalid!";
}
void EoDbFontDefinition::Read(EoDbFile& file) {
	file.Read(&m_Precision, sizeof(EoUInt16));
	file.ReadString(m_FontName);
	file.Read(&m_Path, sizeof(EoUInt16));
	file.Read(&m_HorizontalAlignment, sizeof(EoUInt16));
	file.Read(&m_VerticalAlignment, sizeof(EoUInt16));
	file.Read(&m_CharacterSpacing, sizeof(double));
}
void EoDbFontDefinition::Write(EoDbFile& file) const {
	file.Write(&m_Precision, sizeof(EoUInt16));
	file.WriteString(m_FontName);
	file.Write(&m_Path, sizeof(EoUInt16));
	file.Write(&m_HorizontalAlignment, sizeof(EoUInt16));
	file.Write(&m_VerticalAlignment, sizeof(EoUInt16));
	file.Write(&m_CharacterSpacing, sizeof(double));
}

void EoDbFontDefinition::SetTo(OdDbTextStyleTableRecordPtr textStyleTableRecord) {
    m_FontName = L"Simplex.psf";
    m_Precision = EoDb::kStrokeType;

    if (textStyleTableRecord->isShapeFile()) {
        ATLTRACE2(atlTraceGeneral, 2, L"TextStyle references shape library %s.\n", (LPCWSTR) textStyleTableRecord->desc()->name());
    } else { // shx font file or windows (ttf) font file
        OdString TypeFace;
        bool Bold;
        bool Italic;
        int Charset;
        int PitchAndFamily;

        textStyleTableRecord->font(TypeFace, Bold, Italic, Charset, PitchAndFamily);

        if (TypeFace != L"") { // windows (ttf) file
            m_FontName = (LPCWSTR) TypeFace;
            m_Precision = EoDb::kEoTrueType;
        }
    }
}

void EoDbFontDefinition::SetJustification(OdDb::TextHorzMode horizontalMode, OdDb::TextVertMode verticalMode) {
    m_HorizontalAlignment = EoDbText::ConvertHorizontalAlignment(horizontalMode);;
    m_VerticalAlignment = EoDbText::ConvertVerticalAlignment(verticalMode);
}

void EoDbFontDefinition::SetJustification(OdDbMText::AttachmentPoint attachmentPoint) {
    switch (attachmentPoint) {
        case OdDbMText::kTopLeft:
            m_HorizontalAlignment = EoDb::kAlignLeft;
            m_VerticalAlignment = EoDb::kAlignTop;
            break;
        case OdDbMText::kTopCenter:
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
        case OdDbMText::kMiddleCenter:
            m_HorizontalAlignment = EoDb::kAlignCenter;
            m_VerticalAlignment = EoDb::kAlignMiddle;
            break;
        case OdDbMText::kMiddleRight:
            m_HorizontalAlignment = EoDb::kAlignRight;
            m_VerticalAlignment = EoDb::kAlignMiddle;
            break;
        case OdDbMText::kBottomCenter:
            m_HorizontalAlignment = EoDb::kAlignCenter;
            m_VerticalAlignment = EoDb::kAlignBottom;
            break;
        case OdDbMText::kBottomRight:
            m_HorizontalAlignment = EoDb::kAlignRight;
            m_VerticalAlignment = EoDb::kAlignBottom;
            break;
        default:
            m_HorizontalAlignment = EoDb::kAlignLeft;
            m_VerticalAlignment = EoDb::kAlignBottom;
    }
}



