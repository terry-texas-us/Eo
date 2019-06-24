#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "PrimState.h"
#include "EoGePolyline.h"
#include "EoVaxFloat.h"
#include "EoDbFile.h"
#include "EoDbJobFile.h"
#include "EoDlgTrapModify.h"
IMPLEMENT_DYNAMIC(EoDbText, EoDbPrimitive)

EoDbText::EoDbText(const EoDbText& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;
	m_ColorIndex = other.m_ColorIndex;
	m_FontDefinition = other.m_FontDefinition;
	m_ReferenceSystem = other.m_ReferenceSystem;
	m_strText = other.m_strText;
}

EoDbText& EoDbText::operator=(const EoDbText& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;
	m_ColorIndex = other.m_ColorIndex;
	m_FontDefinition = other.m_FontDefinition;
	m_ReferenceSystem = other.m_ReferenceSystem;
	m_strText = other.m_strText;
	return *this;
}

void EoDbText::AddReportToMessageList(const OdGePoint3d& point) const {
	CString Report(L"<Text>");
	Report += L" Color:" + FormatColorIndex();
	Report += L" Font:" + m_FontDefinition.FontName();
	Report += L" Precision:" + m_FontDefinition.FormatPrecision();
	Report += L" Path:" + m_FontDefinition.FormatPath();
	Report += L" Alignment:(" + m_FontDefinition.FormatHorizontalAlignment() + L"," + m_FontDefinition.FormatVerticalAlignment() + L")";
	theApp.AddStringToMessageList(Report);
}

void EoDbText::AddToTreeViewControl(const HWND tree, const HTREEITEM parent) const noexcept {
	CMainFrame::InsertTreeViewControlItem(tree, parent, L"<Text>", this);
}

EoDbPrimitive* EoDbText::Clone(OdDbBlockTableRecordPtr blockTableRecord) const {
	OdDbTextPtr Text = m_EntityObjectId.safeOpenObject()->clone();
	blockTableRecord->appendOdDbEntity(Text);
	return Create(Text);
}

void EoDbText::Display(AeSysView* view, CDC* deviceContext) {
	const auto ColorIndex {LogicalColorIndex()};
	g_PrimitiveState.SetColorIndex(deviceContext, ColorIndex);
	const auto LinetypeIndex {g_PrimitiveState.LinetypeIndex()};
	g_PrimitiveState.SetLinetypeIndexPs(deviceContext, 1);
	DisplayText(view, deviceContext, m_FontDefinition, m_ReferenceSystem, m_strText);
	g_PrimitiveState.SetLinetypeIndexPs(deviceContext, LinetypeIndex);
}

EoDbFontDefinition EoDbText::FontDefinition() const {
	return m_FontDefinition;
}

void EoDbText::FormatExtra(CString& extra) const {
	extra.Empty();
	extra += L"Color;" + FormatColorIndex() + L"\t";
	extra += L"Font;" + m_FontDefinition.FontName() + L"\t";
	extra += L"Precision;" + m_FontDefinition.FormatPrecision() + L"\t";
	extra += L"Path;" + m_FontDefinition.FormatPath() + L"\t";
	extra += L"Alignment;(" + m_FontDefinition.FormatHorizontalAlignment() + L"," + m_FontDefinition.FormatVerticalAlignment() + L")\t";
	CString Spacing;
	Spacing.Format(L"Spacing;%f\t", m_FontDefinition.CharacterSpacing());
	extra += Spacing;
	CString Length;
	Length.Format(L"Number of Characters;%d\t", m_strText.GetLength());
	extra += Length;
	extra += L"Text;" + m_strText;
}

void EoDbText::FormatGeometry(CString& geometry) const {
	const auto ReferenceSystem {m_ReferenceSystem};
	const auto Origin {ReferenceSystem.Origin()};
	CString OriginString;
	OriginString.Format(L"Origin;%f;%f;%f\t", Origin.x, Origin.y, Origin.z);
	geometry += OriginString;
	CString VectorString;
	VectorString.Format(L"X Axis;%f;%f;%f\t", ReferenceSystem.XDirection().x, ReferenceSystem.XDirection().y, ReferenceSystem.XDirection().z);
	geometry += VectorString;
	VectorString.Format(L"Y Axis;%f;%f;%f\t", ReferenceSystem.YDirection().x, ReferenceSystem.YDirection().y, ReferenceSystem.YDirection().z);
	geometry += VectorString;
}

void EoDbText::GetAllPoints(OdGePoint3dArray& points) const {
	points.clear();
	points.append(m_ReferenceSystem.Origin());
}

void EoDbText::GetBoundingBox(OdGePoint3dArray& boundingBox, const double spaceFactor) const {
	const auto Length {TextLengthSansFormattingCharacters(m_strText)};
	text_GetBoundingBox(m_FontDefinition, m_ReferenceSystem, Length, spaceFactor, boundingBox);
}

OdGePoint3d EoDbText::GetCtrlPt() const noexcept {
	return m_ReferenceSystem.Origin();
}

void EoDbText::GetExtents(AeSysView* view, OdGeExtents3d& extents) const {
	OdGePoint3dArray BoundingBox;
	text_GetBoundingBox(m_FontDefinition, m_ReferenceSystem, m_strText.GetLength(), 0.0, BoundingBox);
	for (const auto& Point : BoundingBox) {
		extents.addPoint(Point);
	}
}

OdGePoint3d EoDbText::GoToNxtCtrlPt() const noexcept {
	return m_ReferenceSystem.Origin();
}

bool EoDbText::IsEqualTo(EoDbPrimitive* primitive) const noexcept {
	return false;
}

bool EoDbText::IsInView(AeSysView* view) const {
	EoGePoint4d pt[2];
	OdGePoint3dArray BoundingBox;
	text_GetBoundingBox(m_FontDefinition, m_ReferenceSystem, m_strText.GetLength(), 0.0, BoundingBox);
	for (unsigned n = 0; n <= 2;) {
		pt[0] = EoGePoint4d(BoundingBox[n++], 1.0);
		pt[1] = EoGePoint4d(BoundingBox[n++], 1.0);
		view->ModelViewTransformPoints(2, pt);
		if (EoGePoint4d::ClipLine(pt[0], pt[1])) return true;
	}
	return false;
}

bool EoDbText::IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	EoGePoint4d pt(m_ReferenceSystem.Origin(), 1.0);
	view->ModelViewTransformPoint(pt);
	return point.DistanceToPointXY(pt) < sm_SelectApertureSize ? true : false;
}

void EoDbText::ModifyNotes(const EoDbFontDefinition& fontDefinition, const EoDbCharacterCellDefinition& characterCellDefinition, const int iAtt) {
	if (iAtt == TM_TEXT_ALL) {
		m_ColorIndex = g_PrimitiveState.ColorIndex();
		m_FontDefinition = fontDefinition;
		m_ReferenceSystem.Rescale(characterCellDefinition);
	} else if (iAtt == TM_TEXT_FONT) {
		m_FontDefinition.SetFontName(fontDefinition.FontName());
		m_FontDefinition.SetPrecision(fontDefinition.Precision());
	} else if (iAtt == TM_TEXT_HEIGHT) {
		m_FontDefinition.SetCharacterSpacing(fontDefinition.CharacterSpacing());
		m_FontDefinition.SetPath(fontDefinition.Path());
		m_ReferenceSystem.Rescale(characterCellDefinition);
	}
}

void EoDbText::ModifyState() noexcept {
	EoDbPrimitive::ModifyState();
	m_FontDefinition = g_PrimitiveState.FontDefinition();
	const auto CharacterCellDefinition = g_PrimitiveState.CharacterCellDefinition();
	m_ReferenceSystem.Rescale(CharacterCellDefinition);
}

OdGePoint3d EoDbText::Position() const noexcept {
	return m_ReferenceSystem.Origin();
}

EoGeReferenceSystem EoDbText::ReferenceSystem() const {
	return m_ReferenceSystem;
}

double EoDbText::Rotation() const {
	const auto HorizontalAxis {ReferenceSystem().XDirection()};
	auto Angle {0.0};
	Angle = atan2(HorizontalAxis.y, HorizontalAxis.x); // -pi to pi radians
	if (Angle < 0.0) {
		Angle += Oda2PI;
	}
	return Angle;
}

OdGePoint3d EoDbText::SelectAtControlPoint(AeSysView*, const EoGePoint4d& point) const {
	sm_ControlPointIndex = USHRT_MAX;
	return point.Convert3d();
}

bool EoDbText::SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const {
	OdGePoint3dArray Points;
	text_GetBoundingBox(m_FontDefinition, m_ReferenceSystem, m_strText.GetLength(), 0.0, Points);
	return polyline::SelectUsingRectangle(view, lowerLeftCorner, upperRightCorner, Points);
}

bool EoDbText::SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& projectedPoint) const {
	if (m_strText.GetLength() == 0) { return false; }
	OdGePoint3dArray BoundingBox;
	text_GetBoundingBox(m_FontDefinition, m_ReferenceSystem, m_strText.GetLength(), 0.0, BoundingBox);
	EoGePoint4d pt0[] = {EoGePoint4d(BoundingBox[0], 1.0), EoGePoint4d(BoundingBox[1], 1.0), EoGePoint4d(BoundingBox[2], 1.0), EoGePoint4d(BoundingBox[3], 1.0)};
	view->ModelViewTransformPoints(4, pt0);
	for (unsigned n = 0; n < 4; n++) {

		if (EoGeLineSeg3d(pt0[n].Convert3d(), pt0[(n + 1) % 4].Convert3d()).DirectedRelationshipOf(point.Convert3d()) < 0) { return false; }
	}
	projectedPoint = point.Convert3d();
	return true;
}

bool EoDbText::SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view, OdGePoint3dArray& intersections) {
	const CRuntimeClass* PrimitiveClass = GetRuntimeClass();
	theApp.AddStringToMessageList(L"Selection by line segment not implemented for <%s>\n", CString(PrimitiveClass->m_lpszClassName));
	return false;
}

void EoDbText::SetFontDefinition(const EoDbFontDefinition& fontDefinition) noexcept {
	m_FontDefinition = fontDefinition;
}

void EoDbText::SetReferenceSystem(const EoGeReferenceSystem& referenceSystem) noexcept {
	m_ReferenceSystem = referenceSystem;
}

void EoDbText::SetText(const CString& text) {
	m_strText = text;
}

const CString& EoDbText::Text() noexcept {
	return m_strText;
}

void EoDbText::TransformBy(const EoGeMatrix3d& transformMatrix) {
	m_ReferenceSystem.TransformBy(transformMatrix);
}

void EoDbText::TranslateUsingMask(const OdGeVector3d& translate, const unsigned long mask) {

	if (mask != 0) { m_ReferenceSystem.SetOrigin(m_ReferenceSystem.Origin() + translate); }
}

bool EoDbText::Write(EoDbFile& file) const {
	file.WriteUInt16(EoDb::kTextPrimitive);
	file.WriteInt16(m_ColorIndex);
	file.WriteInt16(m_LinetypeIndex);
	m_FontDefinition.Write(file);
	m_ReferenceSystem.Write(file);
	file.WriteString(m_strText);
	return true;
}

void EoDbText::Write(CFile& file, unsigned char* buffer) const {
	const unsigned short NumberOfCharacters = static_cast<unsigned short>(m_strText.GetLength());
	buffer[3] = static_cast<unsigned char>((86 + NumberOfCharacters) / 32);
	*reinterpret_cast<unsigned short*>(& buffer[4]) = static_cast<unsigned short>(EoDb::kTextPrimitive);
	buffer[6] = static_cast<unsigned char>(m_ColorIndex == COLORINDEX_BYLAYER ? sm_LayerColorIndex : m_ColorIndex);
	buffer[7] = static_cast<unsigned char>(m_FontDefinition.Precision());
	*reinterpret_cast<short*>(& buffer[8]) = 0;
	reinterpret_cast<EoVaxFloat*>(& buffer[10])->Convert(m_FontDefinition.CharacterSpacing());
	buffer[14] = static_cast<unsigned char>(m_FontDefinition.Path());
	buffer[15] = static_cast<unsigned char>(m_FontDefinition.HorizontalAlignment());
	buffer[16] = static_cast<unsigned char>(m_FontDefinition.VerticalAlignment());
	const auto ReferenceSystem {m_ReferenceSystem};
	reinterpret_cast<EoVaxPoint3d*>(& buffer[17])->Convert(ReferenceSystem.Origin());
	reinterpret_cast<EoVaxVector3d*>(& buffer[29])->Convert(ReferenceSystem.XDirection());
	reinterpret_cast<EoVaxVector3d*>(& buffer[41])->Convert(ReferenceSystem.YDirection());

	// <tas="Stacked fractions (\Snum/den;) are not being converted to legacy format (^/num/den^)"/>
	*reinterpret_cast<unsigned short*>(& buffer[53]) = NumberOfCharacters;
	unsigned BufferOffset = 55;
	for (unsigned CharacterIndex = 0; CharacterIndex < NumberOfCharacters; CharacterIndex++) {
		buffer[BufferOffset++] = static_cast<unsigned char>(m_strText[static_cast<int>(CharacterIndex)]);
	}
	file.Write(buffer, buffer[3] * 32u);
}

EoDb::HorizontalAlignment EoDbText::ConvertHorizontalAlignment(const OdDb::TextHorzMode horizontalMode) noexcept {
	auto HorizontalAlignment {EoDb::kAlignLeft};
	switch (horizontalMode) {
		case OdDb::kTextMid: case OdDb::kTextCenter:
			HorizontalAlignment = EoDb::kAlignCenter;
			break;
		case OdDb::kTextRight: case OdDb::kTextAlign: case OdDb::kTextFit:
			HorizontalAlignment = EoDb::kAlignRight;
			break;
		case OdDb::kTextLeft: default:
			HorizontalAlignment = EoDb::kAlignLeft;
	}
	return HorizontalAlignment;
}

EoDb::VerticalAlignment EoDbText::ConvertVerticalAlignment(const OdDb::TextVertMode verticalMode) noexcept {
	auto VerticalAlignment {EoDb::kAlignBottom};
	switch (verticalMode) {
		case OdDb::kTextVertMid:
			VerticalAlignment = EoDb::kAlignMiddle;
			break;
		case OdDb::kTextTop:
			VerticalAlignment = EoDb::kAlignTop;
			break;
		case OdDb::kTextBase: case OdDb::kTextBottom: default:
			VerticalAlignment = EoDb::kAlignBottom;
	}
	return VerticalAlignment;
}

OdDb::TextHorzMode EoDbText::ConvertHorizontalMode(const unsigned horizontalAlignment) noexcept {
	auto HorizontalMode {OdDb::kTextLeft};
	switch (horizontalAlignment) {
		case EoDb::kAlignCenter:
			HorizontalMode = OdDb::kTextCenter;
			break;
		case EoDb::kAlignRight:
			HorizontalMode = OdDb::kTextRight;
			break;
		default: // kAlignLeft
			HorizontalMode = OdDb::kTextLeft;
	}
	return HorizontalMode;
}

OdDb::TextVertMode EoDbText::ConvertVerticalMode(const unsigned verticalAlignment) noexcept {
	auto VerticalMode {OdDb::kTextBottom};
	switch (verticalAlignment) {
		case EoDb::kAlignMiddle:
			VerticalMode = OdDb::kTextVertMid;
			break;
		case EoDb::kAlignTop:
			VerticalMode = OdDb::kTextTop;
			break;
		default: // kAlignBottom
			VerticalMode = OdDb::kTextBase;
	}
	return VerticalMode;
}

OdDbTextPtr EoDbText::Create(OdDbBlockTableRecordPtr& blockTableRecord, EoDbFile& file) {
	auto Text {OdDbText::createObject()};
	Text->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(Text);
	Text->setColorIndex(static_cast<unsigned short>(file.ReadInt16())); /* short LinetypeIndex = */
	file.ReadInt16();

	// <tas="Precision, FontName, and Path defined in the Text Style which is currently using the default EoStandard. This closely matches the Simplex.psf stroke font.">
	unsigned short Precision {EoDb::kStrokeType};
	file.Read(&Precision, sizeof(unsigned short));
	OdString FontName;
	file.ReadString(FontName);
	unsigned short Path {EoDb::kPathRight};
	file.Read(&Path, sizeof(unsigned short));
	// </tas>
	Text->setHorizontalMode(ConvertHorizontalMode(file.ReadUInt16()));
	Text->setVerticalMode(ConvertVerticalMode(file.ReadUInt16()));
	auto CharacterSpacing {0.0};
	file.Read(&CharacterSpacing, sizeof(double));
	EoGeReferenceSystem ReferenceSystem;
	ReferenceSystem.Read(file);
	Text->setPosition(ReferenceSystem.Origin());
	Text->setHeight(ReferenceSystem.YDirection().length());
	Text->setRotation(ReferenceSystem.Rotation());
	Text->setAlignmentPoint(Text->position());
	OdString TextString;
	file.ReadString(TextString);
	Text->setTextString(TextString);
	return Text;
}

OdDbTextPtr EoDbText::Create(OdDbBlockTableRecordPtr blockTableRecord, unsigned char* primitiveBuffer, const int versionNumber) {
	short ColorIndex {1};
	EoDbFontDefinition FontDefinition;
	EoGeReferenceSystem ReferenceSystem;
	OdString TextString;
	FontDefinition.SetPrecision(EoDb::kStrokeType);
	FontDefinition.SetFontName(L"Simplex.psf");
	if (versionNumber == 1) {
		ColorIndex = short(primitiveBuffer[4] & 0x000f);
		FontDefinition.SetCharacterSpacing(reinterpret_cast<EoVaxFloat*>(& primitiveBuffer[36])->Convert());
		FontDefinition.SetCharacterSpacing(min(max(FontDefinition.CharacterSpacing(), 0.0), 4.));
		const auto d {reinterpret_cast<EoVaxFloat*>(&primitiveBuffer[40])->Convert()};
		switch (int(fmod(d, 10.))) {
			case 3:
				FontDefinition.SetPath(EoDb::kPathDown);
				break;
			case 2:
				FontDefinition.SetPath(EoDb::kPathUp);
				break;
			case 1:
				FontDefinition.SetPath(EoDb::kPathLeft);
				break;
			default:
				FontDefinition.SetPath(EoDb::kPathRight);
		}
		switch (int(fmod(d / 10., 10.))) {
			case 3:
				FontDefinition.SetHorizontalAlignment(EoDb::kAlignRight);
				break;
			case 2:
				FontDefinition.SetHorizontalAlignment(EoDb::kAlignCenter);
				break;
			default:
				FontDefinition.SetHorizontalAlignment(EoDb::kAlignLeft);
		}
		switch (int(d / 100.)) {
			case 2:
				FontDefinition.SetVerticalAlignment(EoDb::kAlignTop);
				break;
			case 3:
				FontDefinition.SetVerticalAlignment(EoDb::kAlignMiddle);
				break;
			default:
				FontDefinition.SetVerticalAlignment(EoDb::kAlignBottom);
		}
		ReferenceSystem.SetOrigin(reinterpret_cast<EoVaxPoint3d*>(& primitiveBuffer[8])->Convert() * 1.e-3);
		auto dChrHgt {reinterpret_cast<EoVaxFloat*>(&primitiveBuffer[20])->Convert()};
		dChrHgt = min(max(dChrHgt, .01e3), 100.e3);
		auto dChrExpFac {reinterpret_cast<EoVaxFloat*>(&primitiveBuffer[24])->Convert()};
		dChrExpFac = min(max(dChrExpFac, 0.0), 10.);
		ReferenceSystem.SetXDirection(OdGeVector3d(0.6 * dChrHgt * dChrExpFac, 0.0, 0.0) * 1.e-3);
		ReferenceSystem.SetYDirection(OdGeVector3d(0.0, dChrHgt, 0.0) * 1.e-3);
		auto Angle {reinterpret_cast<EoVaxFloat*>(&primitiveBuffer[28])->Convert()};
		Angle = min(max(Angle, -Oda2PI), Oda2PI);
		if (fabs(Angle) > FLT_EPSILON) {
			auto XDirection {ReferenceSystem.XDirection()};
			XDirection = XDirection.rotateBy(Angle, OdGeVector3d::kZAxis);
			ReferenceSystem.SetXDirection(XDirection);
			auto YDirection {ReferenceSystem.YDirection()};
			YDirection = YDirection.rotateBy(Angle, OdGeVector3d::kZAxis);
			ReferenceSystem.SetYDirection(YDirection);
		}
		char* NextToken = nullptr;
		auto pChr {strtok_s(reinterpret_cast<char*>(&primitiveBuffer[44]), "\\", &NextToken)};
		if (pChr == nullptr) {
			TextString = L"EoDbJobFile.PrimText error: Missing string terminator.";
		} else if (strlen(pChr) > 132) {
			TextString = L"EoDbJobFile.PrimText error: Text too long.";
		} else {
			while (*pChr != 0) {
				if (!isprint(*pChr)) * pChr = '.';
				pChr++;
			}
			TextString = reinterpret_cast<LPCSTR>(&primitiveBuffer[44]);
		}
	} else {
		ColorIndex = short(primitiveBuffer[6]);
		FontDefinition.SetCharacterSpacing(reinterpret_cast<EoVaxFloat*>(& primitiveBuffer[10])->Convert());
		switch (primitiveBuffer[14]) {
			case 3:
				FontDefinition.SetPath(EoDb::kPathDown);
				break;
			case 2:
				FontDefinition.SetPath(EoDb::kPathUp);
				break;
			case 1:
				FontDefinition.SetPath(EoDb::kPathLeft);
				break;
			default:
				FontDefinition.SetPath(EoDb::kPathRight);
		}
		switch (primitiveBuffer[15]) {
			case 3:
				FontDefinition.SetHorizontalAlignment(EoDb::kAlignRight);
				break;
			case 2:
				FontDefinition.SetHorizontalAlignment(EoDb::kAlignCenter);
				break;
			default:
				FontDefinition.SetHorizontalAlignment(EoDb::kAlignLeft);
		}
		switch (primitiveBuffer[16]) {
			case 2:
				FontDefinition.SetVerticalAlignment(EoDb::kAlignTop);
				break;
			case 3:
				FontDefinition.SetVerticalAlignment(EoDb::kAlignMiddle);
				break;
			default:
				FontDefinition.SetVerticalAlignment(EoDb::kAlignBottom);
		}
		ReferenceSystem.SetOrigin(reinterpret_cast<EoVaxPoint3d*>(& primitiveBuffer[17])->Convert());
		ReferenceSystem.SetXDirection(reinterpret_cast<EoVaxVector3d*>(& primitiveBuffer[29])->Convert());
		ReferenceSystem.SetYDirection(reinterpret_cast<EoVaxVector3d*>(& primitiveBuffer[41])->Convert());
		const auto TextLength {*reinterpret_cast<short*>(&primitiveBuffer[53])};
		primitiveBuffer[55 + TextLength] = '\0';
		TextString = reinterpret_cast<LPCSTR>(& primitiveBuffer[55]);
	}
	EoDbJobFile::ConvertFormattingCharacters(TextString);
	auto Text {OdDbText::createObject()};
	Text->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(Text);
	Text->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Text->setHorizontalMode(ConvertHorizontalMode(FontDefinition.HorizontalAlignment()));
	Text->setVerticalMode(ConvertVerticalMode(FontDefinition.VerticalAlignment()));
	Text->setPosition(ReferenceSystem.Origin());
	Text->setHeight(ReferenceSystem.YDirection().length());
	Text->setRotation(ReferenceSystem.Rotation());
	Text->setAlignmentPoint(Text->position());
	Text->setTextString(TextString);
	return Text;
}

OdDbTextPtr EoDbText::Create(OdDbBlockTableRecordPtr& blockTableRecord, const OdGePoint3d& position, const OdString& textString) {
	auto Text {OdDbText::createObject()};
	Text->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(Text);
	Text->setColorIndex(static_cast<unsigned short>(g_PrimitiveState.ColorIndex()));
	Text->setPosition(position);
	Text->setTextString(textString);
	return Text;
}

OdDbMTextPtr EoDbText::CreateM(OdDbBlockTableRecordPtr& blockTableRecord, OdString text) {
	auto MText {OdDbMText::createObject()};
	MText->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(MText);
	MText->setColorIndex(static_cast<unsigned short>(g_PrimitiveState.ColorIndex()));
	MText->setContents(text);
	return MText;
}

EoDbText* EoDbText::Create(OdDbTextPtr& text) {
	auto Text {new EoDbText()};
	Text->SetEntityObjectId(text->objectId());
	Text->SetColorIndex(static_cast<short>(text->colorIndex()));
	Text->SetLinetypeIndex(static_cast<short>(EoDbLinetypeTable::LegacyLinetypeIndex(text->linetype())));
	const OdDbTextStyleTableRecordPtr TextStyleTableRecordPtr {text->textStyle().safeOpenObject(OdDb::kForRead)};
	EoDbFontDefinition FontDefinition;
	FontDefinition.SetTo(TextStyleTableRecordPtr);
	FontDefinition.SetJustification(text->horizontalMode(), text->verticalMode());
	auto AlignmentPoint {text->position()};
	if (FontDefinition.HorizontalAlignment() != EoDb::kAlignLeft || FontDefinition.VerticalAlignment() != EoDb::kAlignBottom) {
		AlignmentPoint = text->alignmentPoint();
	}
	EoDbCharacterCellDefinition CharacterCellDefinition;
	CharacterCellDefinition.SetHeight(text->height());
	CharacterCellDefinition.SetWidthFactor(text->widthFactor());
	CharacterCellDefinition.SetRotationAngle(text->rotation());
	CharacterCellDefinition.SetObliqueAngle(text->oblique());
	const EoGeReferenceSystem ReferenceSystem(AlignmentPoint, text->normal(), CharacterCellDefinition);
	Text->SetFontDefinition(FontDefinition);
	Text->SetReferenceSystem(ReferenceSystem);
	Text->SetText(static_cast<const wchar_t*>(text->textString()));
	return Text;
}

EoDbText* EoDbText::Create(OdDbMTextPtr& text) {
	auto Text {new EoDbText()};
	Text->SetEntityObjectId(text->objectId());
	Text->SetColorIndex(static_cast<short>(text->colorIndex()));
	Text->SetLinetypeIndex(static_cast<short>(EoDbLinetypeTable::LegacyLinetypeIndex(text->linetype())));
	const OdDbTextStyleTableRecordPtr TextStyleTableRecordPtr {text->textStyle().safeOpenObject(OdDb::kForRead)};
	EoDbFontDefinition FontDefinition;
	FontDefinition.SetTo(TextStyleTableRecordPtr);
	FontDefinition.SetJustification(text->attachment());

	// <tas="defines the bounding rectangle size for paragraph features"</tas>
	//    double Width = text->width();
	//    double Height = text->height();
	const auto AlignmentPoint {text->location()};
	EoDbCharacterCellDefinition CharacterCellDefinition;
	CharacterCellDefinition.SetHeight(text->textHeight());
	//    CharacterCellDefinition.SetWidthFactor(text->??);
	CharacterCellDefinition.SetRotationAngle(text->rotation());
	//    CharacterCellDefinition.SetObliqueAngle(text->oblique());
	const EoGeReferenceSystem ReferenceSystem {AlignmentPoint, text->normal(), CharacterCellDefinition};
	Text->SetFontDefinition(FontDefinition);
	Text->SetReferenceSystem(ReferenceSystem);
	Text->SetText(static_cast<const wchar_t*>(text->contents()));
	return Text;
}

bool HasFormattingCharacters(const CString& text) {
	for (auto i = 0; i < text.GetLength() - 1; i++) {
		if (text[i] == '\\') {
			switch (text[i + 1]) { // Parameter Meaning
				case 'P': // Hard line break
					//case '~':	// Non-breaking space
					//case '/':	// Single backslash; otherwise used as an escape character
					//case '{':	// Single opening curly bracket; otherwise used as block begin
					//case '}':	// Single closing curly bracket; otherwise used as block end
				case 'A': // Change alignment: bottom (0), center (1)  or top (2)
					//case 'C':	// ACI color number				Change character color
				case 'F': // Change to a different font: acad - \FArial.shx; windows - \FArial|b1|i0|c0|p34
				case 'f':
					//case 'H':	// Change text height: New height or relative height followed by an x
					//case 'L': // Start underlining
					//case 'l': // End underlining
					//case 'O': // Start overlining
					//case 'o': // End overlining
					//case 'T':	// Change kerning, i.e. character spacing
					//case 'W':	// Change character width, i.e X scaling
				case 'S':	// Stacked text or fractions: the S is followed by two text segments separated by a / (fraction bar) or ^ (no fraction bar)
					return true;
			}
		}
	}
	return false;
}

int FontEscapementAngle(const OdGeVector3d& xAxis) noexcept {
	auto Angle {0.0};
	Angle = atan2(xAxis.y, xAxis.x); // -pi to pi radians
	if (Angle < 0.0) {
		Angle += Oda2PI;
	}
	return EoRound(EoToDegree(Angle) * 10.);
}

OdGePoint3d CalculateInsertionPoint(const EoDbFontDefinition& fontDefinition, const int numberOfCharacters) noexcept {
	auto InsertionPoint {OdGePoint3d::kOrigin};
	if (numberOfCharacters > 0) {
		const auto dTxtExt {double(numberOfCharacters) + (double(numberOfCharacters) - 1.0) * (0.32 + fontDefinition.CharacterSpacing()) / 0.6};
		if (fontDefinition.Path() == EoDb::kPathRight || fontDefinition.Path() == EoDb::kPathLeft) {
			if (fontDefinition.Path() == EoDb::kPathRight) {
				if (fontDefinition.HorizontalAlignment() == EoDb::kAlignCenter) InsertionPoint.x = -dTxtExt * 0.5;
				else if (fontDefinition.HorizontalAlignment() == EoDb::kAlignRight) InsertionPoint.x = -dTxtExt;
			} else {
				if (fontDefinition.HorizontalAlignment() == EoDb::kAlignLeft) InsertionPoint.x = dTxtExt;
				else if (fontDefinition.HorizontalAlignment() == EoDb::kAlignCenter) InsertionPoint.x = dTxtExt * 0.5;
				InsertionPoint.x = InsertionPoint.x - 1.0;
			}
			if (fontDefinition.VerticalAlignment() == EoDb::kAlignMiddle) InsertionPoint.y = -.5;
			else if (fontDefinition.VerticalAlignment() == EoDb::kAlignTop) InsertionPoint.y = -1.0;
		} else if (fontDefinition.Path() == EoDb::kPathDown || fontDefinition.Path() == EoDb::kPathUp) {
			if (fontDefinition.HorizontalAlignment() == EoDb::kAlignCenter) InsertionPoint.x = -.5;
			else if (fontDefinition.HorizontalAlignment() == EoDb::kAlignRight) InsertionPoint.x = -1.0;
			if (fontDefinition.Path() == EoDb::kPathUp) {
				if (fontDefinition.VerticalAlignment() == EoDb::kAlignMiddle) InsertionPoint.y = -dTxtExt * 0.5;
				else if (fontDefinition.VerticalAlignment() == EoDb::kAlignTop) InsertionPoint.y = -dTxtExt;
			} else {
				if (fontDefinition.VerticalAlignment() == EoDb::kAlignBottom) InsertionPoint.y = dTxtExt;
				else if (fontDefinition.VerticalAlignment() == EoDb::kAlignMiddle) InsertionPoint.y = dTxtExt * 0.5;
				InsertionPoint.y = InsertionPoint.y - 1.0;
			}
		}
	}
	return InsertionPoint;
}

// <summary>Determines the number of characters in the text excluding formatting characters.</summary>
/* <issues>
	Designed for single line of text. For Multiple lines the length should be of the longest line in the paragraph if the length is to be used for horizontal alignment points.
	Block delimiters '{' and '}' and other formatting codes not being handled.
</issues> */
int TextLengthSansFormattingCharacters(const CString& text) {
	auto Length {text.GetLength()};
	auto CurrentPosition {0};
	while (CurrentPosition < text.GetLength()) {
		auto c {text[CurrentPosition++]};
		if (c == '\\') {
			c = text[CurrentPosition];
			if (c == 'A') {
				const auto EndSemicolon {text.Find(';', CurrentPosition)};
				if (EndSemicolon != -1 && EndSemicolon == CurrentPosition + 2) {
					Length -= 4;
					CurrentPosition = EndSemicolon + 1;
				}
			} else if (c == 'F' || c == 'f') {
				const auto EndSemicolon {text.Find(';', CurrentPosition)};
				if (EndSemicolon != -1) {
					const auto FormatLength {EndSemicolon - CurrentPosition};
					Length -= FormatLength + 2;
					CurrentPosition = EndSemicolon + 1;
				}
			} else if (c == 'P') {
				Length -= 2;
				CurrentPosition++;
			} else if (c == 'S') {
				const auto EndSemicolon {text.Find(';', CurrentPosition)};
				if (EndSemicolon != -1) {
					auto TextSegmentDelimiter {text.Find('/', CurrentPosition)};
					if (TextSegmentDelimiter == -1) TextSegmentDelimiter = text.Find('^', CurrentPosition);
					if (TextSegmentDelimiter != -1 && TextSegmentDelimiter < EndSemicolon) {
						Length -= 4;
						CurrentPosition = EndSemicolon + 1;
					}
				}
			}
		}
	}
	return Length;
}

void DisplayText(AeSysView* view, CDC* deviceContext, EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, const CString& text) {
	if (text.IsEmpty()) { return; }
	if (HasFormattingCharacters(text)) {
		DisplayTextWithFormattingCharacters(view, deviceContext, fontDefinition, referenceSystem, text);
		return;
	}
	auto ReferenceSystem {referenceSystem};
	auto InsertionPoint {CalculateInsertionPoint(fontDefinition, text.GetLength())};
	InsertionPoint.transformBy(EoGeMatrix3d::ReferenceSystemToWorld(referenceSystem));
	ReferenceSystem.SetOrigin(InsertionPoint);
	auto NumberOfCharactersToDisplay {0};
	auto StartPosition {0};
	auto CurrentPosition {StartPosition};
	while (CurrentPosition < text.GetLength()) {
		const auto c {text[CurrentPosition++]};
		if (c == '\r' && text[CurrentPosition] == '\n') {
			DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, StartPosition, NumberOfCharactersToDisplay, text);
			ReferenceSystem.SetOrigin(InsertionPoint);
			ReferenceSystem.SetOrigin(text_GetNewLinePos(fontDefinition, ReferenceSystem, 1.0, 0));
			InsertionPoint = ReferenceSystem.Origin();
			StartPosition += 2 + NumberOfCharactersToDisplay;
			CurrentPosition = StartPosition;
			NumberOfCharactersToDisplay = 0;
		} else {
			NumberOfCharactersToDisplay++;
		}
	}
	DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, StartPosition, NumberOfCharactersToDisplay, text);
}

void DisplayTextSegment(AeSysView* view, CDC* deviceContext, EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, const int startPosition, const int numberOfCharacters, const CString& text) {
	if (deviceContext != nullptr && fontDefinition.Precision() == EoDb::kTrueType && view->ViewTrueTypeFonts()) {
		auto XDirection {referenceSystem.XDirection()};
		auto YDirection {referenceSystem.YDirection()};
		view->ModelViewTransformVector(XDirection);
		view->ModelViewTransformVector(YDirection);
		auto PlaneNormal {XDirection.crossProduct(YDirection)};
		if (PlaneNormal.isZeroLength()) { return; }
		PlaneNormal.normalize();
		if (PlaneNormal.isEqualTo(OdGeVector3d::kZAxis)) {
			if (DisplayTextSegmentUsingTrueTypeFont(view, deviceContext, fontDefinition, referenceSystem, startPosition, numberOfCharacters, text)) { return; }
		}
	}
	DisplayTextSegmentUsingStrokeFont(view, deviceContext, fontDefinition, referenceSystem, startPosition, numberOfCharacters, text);
}

void DisplayTextSegmentUsingStrokeFont(AeSysView* view, CDC* deviceContext, EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, const int startPosition, const int numberOfCharacters, const CString& text) {
	if (numberOfCharacters == 0) { return; }
	const long* plStrokeFontDef = reinterpret_cast<long*>(theApp.SimplexStrokeFont());
	if (plStrokeFontDef == nullptr) { return; }
	const auto tm {EoGeMatrix3d::ReferenceSystemToWorld(referenceSystem)};
	const auto plStrokeChrDef {plStrokeFontDef + 96};
	const auto CharacterSpacing {1. + (0.32 + fontDefinition.CharacterSpacing()) / 0.6};
	auto ptStroke {OdGePoint3d::kOrigin};
	auto ptChrPos {ptStroke};
	auto n {startPosition};
	while (n < startPosition + numberOfCharacters) {
		polyline::BeginLineStrip();
		int Character = text.GetAt(n);
		if (Character < 32 || Character > 126) Character = '.';
		for (auto i = static_cast<int>(plStrokeFontDef[Character - 32]); i <= plStrokeFontDef[Character - 32 + 1] - 1; i++) {
			auto iY {static_cast<int>(plStrokeChrDef[i - 1] % 4096L)};
			if ((iY & 2048) != 0) { iY = -(iY - 2048); }
			auto iX {static_cast<int>(plStrokeChrDef[i - 1] / 4096L % 4096L)};
			if ((iX & 2048) != 0) { iX = -(iX - 2048); }
			ptStroke += OdGeVector3d(.01 / 0.6 * iX, .01 * iY, 0.0);
			if (plStrokeChrDef[i - 1] / 16777216 == 5) {
				polyline::__End(view, deviceContext, 1);
				polyline::BeginLineStrip();
			}
			polyline::SetVertex(tm * ptStroke);
		}
		polyline::__End(view, deviceContext, 1);
		switch (fontDefinition.Path()) {
			case EoDb::kPathLeft:
				ptChrPos.x -= CharacterSpacing;
				break;
			case EoDb::kPathUp:
				ptChrPos.y += CharacterSpacing;
				break;
			case EoDb::kPathDown:
				ptChrPos.y -= CharacterSpacing;
				break;
			case EoDb::kPathRight: default:
				ptChrPos.x += CharacterSpacing;
		}
		ptStroke = ptChrPos;
		n++;
	}
}

bool DisplayTextUsingWindowsFontOutline(CDC* deviceContext, const int x, const int y, const CString& text) {
	deviceContext->BeginPath();
	deviceContext->TextOutW(x, y, text);
	deviceContext->EndPath();
	auto NumberOfPointsInPath {deviceContext->GetPath(nullptr, nullptr, 0)};
	if (NumberOfPointsInPath == 0) { return true; }

	// Allocate memory to hold points and stroke types from the path.
	LPPOINT Points {nullptr};
	unsigned char* Types {nullptr};
	try {
		Points = new POINT[static_cast<unsigned>(NumberOfPointsInPath)];
		Types = new unsigned char[static_cast<unsigned>(NumberOfPointsInPath)];
	} catch (CException* Exception) {
		delete[] Points;
		Points = nullptr;
		delete[] Types;
		Types = nullptr;
		Exception->Delete();
	}
	if (Points == nullptr || Types == nullptr) { return true; }

	// Now that we have the memory, really get the path data.
	NumberOfPointsInPath = deviceContext->GetPath(Points, Types, NumberOfPointsInPath);
	if (NumberOfPointsInPath != -1) { deviceContext->PolyDraw(Points, Types, NumberOfPointsInPath); }
	delete[] Points;
	delete[] Types;
	return true;
}

bool DisplayTextSegmentUsingTrueTypeFont(AeSysView* view, CDC* deviceContext, EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, const int startPosition, const int numberOfCharacters, const CString& text) {
	if (numberOfCharacters <= 0) { return true; }
	const auto ReferenceSystemToWorldTransform {EoGeMatrix3d::ReferenceSystemToWorld(referenceSystem)};
	auto Origin {OdGePoint3d::kOrigin};
	EoGePoint4d StartPoint(Origin.transformBy(ReferenceSystemToWorldTransform), 1.0);
	view->ModelViewTransformPoint(StartPoint);
	const auto ProjectedStartPoint {view->DoViewportProjection(StartPoint)};
	EoGePoint4d ptsBox[3];
	OdGePoint3d TopLeft(0.0, 1.0, 0.0);
	TopLeft.transformBy(ReferenceSystemToWorldTransform);
	ptsBox[1] = EoGePoint4d(TopLeft, 1.0);
	view->ModelViewTransformPoint(ptsBox[1]);
	OdGePoint3d BottomRight(1.0, 0.0, 0.0);
	BottomRight.transformBy(ReferenceSystemToWorldTransform);
	ptsBox[2] = EoGePoint4d(BottomRight, 1.0);
	view->ModelViewTransformPoint(ptsBox[2]);
	CPoint pnt[4];
	pnt[1] = view->DoViewportProjection(ptsBox[1]);
	pnt[2] = view->DoViewportProjection(ptsBox[2]);
	const OdGeVector3d vX(static_cast<double>(pnt[2].x) - static_cast<double>(ProjectedStartPoint.x), static_cast<double>(pnt[2].y) - static_cast<double>(ProjectedStartPoint.y), 0.0);
	const OdGeVector3d vY(static_cast<double>(pnt[1].x) - static_cast<double>(ProjectedStartPoint.x), static_cast<double>(pnt[1].y) - static_cast<double>(ProjectedStartPoint.y), 0.0);
	const auto Height = vY.length();
	if (Height == 0.0) { return true; }
	LOGFONT FontAttributes;
	memset(&FontAttributes, 0, sizeof FontAttributes);
	FontAttributes.lfHeight = -EoRound(1.33 * Height);
	FontAttributes.lfEscapement = -FontEscapementAngle(vX);
	FontAttributes.lfOrientation = FontAttributes.lfEscapement;
	FontAttributes.lfWeight = FW_NORMAL;
	wcscpy_s(FontAttributes.lfFaceName, LF_FACESIZE, fontDefinition.FontName());
	CFont Font;
	Font.CreateFontIndirectW(&FontAttributes);
	const auto OldFont {deviceContext->SelectObject(&Font)};
	const auto TextAlign {deviceContext->SetTextAlign(TA_LEFT | TA_BASELINE)};
	const auto BackgroundMode {deviceContext->SetBkMode(TRANSPARENT)};
	deviceContext->TextOutW(ProjectedStartPoint.x, ProjectedStartPoint.y, text.Mid(startPosition, numberOfCharacters));

	//	DisplayTextUsingWindowsFontOutline(deviceContext, ProjectedStartPoint.x, ProjectedStartPoint.y, text.Mid(startPosition, numberOfCharacters));
	deviceContext->SetBkMode(BackgroundMode);
	deviceContext->SetTextAlign(TextAlign);
	deviceContext->SelectObject(OldFont);
	return true;
}

void DisplayTextWithFormattingCharacters(AeSysView* view, CDC* deviceContext, EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, const CString& text) {
	auto ReferenceSystem {referenceSystem};
	const auto Length {TextLengthSansFormattingCharacters(text)};
	auto InsertionPoint {CalculateInsertionPoint(fontDefinition, Length)};
	InsertionPoint.transformBy(EoGeMatrix3d::ReferenceSystemToWorld(referenceSystem));
	ReferenceSystem.SetOrigin(InsertionPoint);
	auto NumberOfCharactersToDisplay {0};
	auto StartPosition {0};
	auto CurrentPosition {StartPosition};
	while (CurrentPosition < text.GetLength()) {
		auto c {text[CurrentPosition++]};
		if (c != '\\') {
			NumberOfCharactersToDisplay++;
		} else {
			c = text[CurrentPosition];
			if (c == 'F' || c == 'f') { // Change to a different font. For shx fonts: \FTxt.shx. For windows fonts: \FSimplex|b1|i0|c0|p34;
				const auto EndSemicolon {text.Find(';', CurrentPosition)};
				if (EndSemicolon != -1) {
					if (CurrentPosition + 1 < EndSemicolon) {
						DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, StartPosition, NumberOfCharactersToDisplay, text);
						StartPosition = EndSemicolon + 1;
						CurrentPosition = StartPosition;
						NumberOfCharactersToDisplay = 0;
					}
				}
			} else if (c == 'P') { // Hard line break
				if (CurrentPosition < text.GetLength()) {
					DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, StartPosition, NumberOfCharactersToDisplay, text);
					ReferenceSystem.SetOrigin(InsertionPoint);
					ReferenceSystem.SetOrigin(text_GetNewLinePos(fontDefinition, ReferenceSystem, 1.0, 0));
					InsertionPoint = ReferenceSystem.Origin();
					StartPosition += 2 + NumberOfCharactersToDisplay;
					CurrentPosition = StartPosition;
					NumberOfCharactersToDisplay = 0;
				}
			} else if (c == 'A') { // Change alignment to bottom, center middle
				const auto EndSemicolon {text.Find(';', CurrentPosition)};
				if (EndSemicolon != -1) {
					if (CurrentPosition + 1 < EndSemicolon) {
						const auto Parameter {text[CurrentPosition + 1]};
						if (Parameter >= '0' && Parameter <= '2') {
							if (NumberOfCharactersToDisplay > 0) { // display text segment preceding the formatting
								DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, StartPosition, NumberOfCharactersToDisplay, text);

								// Offset the line position left of current position
								ReferenceSystem.SetOrigin(text_GetNewLinePos(fontDefinition, referenceSystem, 0.0, NumberOfCharactersToDisplay * (1 + 0.32 / 0.6)));
								InsertionPoint = ReferenceSystem.Origin();
							}
							StartPosition = EndSemicolon + 1;
							CurrentPosition = StartPosition;
							NumberOfCharactersToDisplay = 0;
							if (Parameter == '1') {
								ReferenceSystem.SetOrigin(text_GetNewLinePos(fontDefinition, ReferenceSystem, 0.5, 0.0));
							} else if (Parameter == '2') {
								ReferenceSystem.SetOrigin(text_GetNewLinePos(fontDefinition, ReferenceSystem, -.5, 0.0));
							}
						}
					}
				}
			} else if (c == 'S') { // Stacked text or fractions
				const auto EndSemicolon {text.Find(';', CurrentPosition)};
				if (EndSemicolon != -1) {
					auto TextSegmentDelimiter {text.Find('/', CurrentPosition)};
					if (TextSegmentDelimiter == -1) TextSegmentDelimiter = text.Find('^', CurrentPosition);
					if (TextSegmentDelimiter != -1 && TextSegmentDelimiter < EndSemicolon) {
						if (NumberOfCharactersToDisplay > 0) { // display text segment preceding the formatting
							DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, StartPosition, NumberOfCharactersToDisplay, text);
							StartPosition += NumberOfCharactersToDisplay;
						}
						// Offset the line position up and conditionally left of current position
						ReferenceSystem.SetOrigin(text_GetNewLinePos(fontDefinition, ReferenceSystem, -.35, NumberOfCharactersToDisplay * (1 + 0.32 / 0.6)));
						InsertionPoint = ReferenceSystem.Origin();
						StartPosition += 2; // skip the formatting characters
						NumberOfCharactersToDisplay = TextSegmentDelimiter - StartPosition;
						if (NumberOfCharactersToDisplay > 0) { // Display superscripted text segment
							DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, StartPosition, NumberOfCharactersToDisplay, text);
							StartPosition += NumberOfCharactersToDisplay;
						}
						// Offset the line position back down left
						ReferenceSystem.SetOrigin(text_GetNewLinePos(fontDefinition, ReferenceSystem, .35, NumberOfCharactersToDisplay * (1 + 0.32 / 0.6) - 0.72));
						InsertionPoint = ReferenceSystem.Origin();
						if (text[TextSegmentDelimiter] == '/') { // display the text segment delimiter
							DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, TextSegmentDelimiter, 1, text);
						}
						StartPosition = TextSegmentDelimiter + 1;
						NumberOfCharactersToDisplay = EndSemicolon - StartPosition;
						//Offset the line position down
						ReferenceSystem.SetOrigin(text_GetNewLinePos(fontDefinition, ReferenceSystem, .35, .72));
						InsertionPoint = ReferenceSystem.Origin();
						if (NumberOfCharactersToDisplay > 0) { // Display subscripted text segment
							DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, StartPosition, NumberOfCharactersToDisplay, text);
							StartPosition += NumberOfCharactersToDisplay;
						}
						ReferenceSystem.SetOrigin(text_GetNewLinePos(fontDefinition, ReferenceSystem, -.35, NumberOfCharactersToDisplay * (1 + 0.32 / 0.6)));
						InsertionPoint = ReferenceSystem.Origin();
						NumberOfCharactersToDisplay = 0;
						StartPosition = EndSemicolon + 1;
						CurrentPosition = StartPosition;
					}
				}
			}
		}
	}
	DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, StartPosition, NumberOfCharactersToDisplay, text);
}

void text_GetBoundingBox(const EoDbFontDefinition& fontDefinition, const EoGeReferenceSystem& referenceSystem, const int numberOfCharacters, const double spaceFactor, OdGePoint3dArray& boundingBox) {
	boundingBox.setLogicalLength(4);
	if (numberOfCharacters > 0) {
		const auto tm {EoGeMatrix3d::ReferenceSystemToWorld(referenceSystem)};
		auto TextHeight {1.0};
		auto TextWidth {1.0};
		const auto CharacterSpacing {(0.32 + fontDefinition.CharacterSpacing()) / 0.6};
		const auto d {double(numberOfCharacters) + CharacterSpacing * (double(numberOfCharacters) - 1.0)};
		if (fontDefinition.Path() == EoDb::kPathRight || fontDefinition.Path() == EoDb::kPathLeft) {
			TextWidth = d;
		} else {
			TextHeight = d;
		}
		boundingBox.setAll(OdGePoint3d::kOrigin);
		if (fontDefinition.HorizontalAlignment() == EoDb::kAlignLeft) {
			boundingBox[2].x = TextWidth;
		} else if (fontDefinition.HorizontalAlignment() == EoDb::kAlignCenter) {
			boundingBox[0].x = -TextWidth * 0.5;
			boundingBox[2].x = boundingBox[0].x + TextWidth;
		} else {
			boundingBox[0].x = -TextWidth;
		}
		if (fontDefinition.VerticalAlignment() == EoDb::kAlignTop) {
			boundingBox[0].y = -TextHeight;
		} else if (fontDefinition.VerticalAlignment() == EoDb::kAlignMiddle) {
			boundingBox[0].y = -TextHeight * 0.5;
			boundingBox[2].y = boundingBox[0].y + TextHeight;
		} else {
			boundingBox[2].y = TextHeight;
		}
		if (spaceFactor > DBL_EPSILON) {
			boundingBox[0].x -= spaceFactor / 0.6;
			boundingBox[0].y -= spaceFactor;
			boundingBox[2].x += spaceFactor / 0.6;
			boundingBox[2].y += spaceFactor;
		}
		boundingBox[1].x = boundingBox[2].x;
		boundingBox[1].y = boundingBox[0].y;
		boundingBox[3].x = boundingBox[0].x;
		boundingBox[3].y = boundingBox[2].y;
		for (unsigned n = 0; n < 4; n++) {
			boundingBox[n].transformBy(tm);
		}
	} else {
		boundingBox.setAll(referenceSystem.Origin());
	}
}

OdGePoint3d text_GetNewLinePos(EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, const double lineSpaceFactor, const double characterSpaceFactor) {
	auto pt {referenceSystem.Origin()};
	auto vPath {referenceSystem.XDirection()};
	const auto YDirection {referenceSystem.YDirection()};
	if (fontDefinition.Path() == EoDb::kPathRight || fontDefinition.Path() == EoDb::kPathLeft) {
		pt += vPath * characterSpaceFactor;
		OdGeVector3d vRefNorm;
		referenceSystem.GetUnitNormal(vRefNorm);
		vPath.normalize();
		vPath *= -(YDirection.length() * lineSpaceFactor);
		vPath.rotateBy(OdaPI2, vRefNorm);
	}
	return pt + vPath * 1.5;
}
