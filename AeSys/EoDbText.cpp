#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

#include "EoDlgTrapModify.h"

EoDbText::EoDbText() {
}

EoDbText::EoDbText(const EoDbText& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;

	m_ColorIndex = other.m_ColorIndex;
	m_FontDefinition = other.m_FontDefinition;
	m_ReferenceSystem = other.m_ReferenceSystem;
	m_strText = other.m_strText;
}

EoDbText::~EoDbText() {
}

const EoDbText& EoDbText::operator=(const EoDbText& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;

	m_ColorIndex = other.m_ColorIndex;
	m_FontDefinition = other.m_FontDefinition;
	m_ReferenceSystem = other.m_ReferenceSystem;
	m_strText = other.m_strText;

	return (*this);
}

void EoDbText::AddReportToMessageList(const OdGePoint3d& point) const {
	CString Report(L"<Text>");
	Report += L" Color:" + FormatColorIndex();
	Report += L" Font:" + m_FontDefinition.FontName();
	Report += L" Precision:" + m_FontDefinition.FormatPrecision();
	Report += L" Path:" + m_FontDefinition.FormatPath();
	Report += L" Alignment:(" + m_FontDefinition.FormatHorizonatlAlignment() + L"," + m_FontDefinition.FormatVerticalAlignment() + L")";

	theApp.AddStringToMessageList(Report);
}

void EoDbText::AddToTreeViewControl(HWND tree, HTREEITEM parent) const {
	CMainFrame::InsertTreeViewControlItem(tree, parent, L"<Text>", this);
}

void EoDbText::AssociateWith(OdDbBlockTableRecordPtr blockTableRecord) {
	OdDbTextPtr TextEntity = OdDbText::createObject();
	blockTableRecord->appendOdDbEntity(TextEntity);
	TextEntity->setDatabaseDefaults();

	SetEntityObjectId(TextEntity->objectId());
	TextEntity->setColorIndex(m_ColorIndex);
	SetHorizontalMode(m_FontDefinition.HorizontalAlignment());
	SetVerticalMode(m_FontDefinition.VerticalAlignment());
	TextEntity->setPosition(m_ReferenceSystem.Origin());
	TextEntity->setTextString((LPCWSTR) m_strText);
	TextEntity->setHeight(m_ReferenceSystem.YDirection().length());
	TextEntity->setRotation(Rotation());
	TextEntity->setAlignmentPoint(TextEntity->position());
}

void EoDbText::ConvertFormattingCharacters() {
	for (int i = 0; i < m_strText.GetLength() - 1; i++) {
		if (m_strText[i] == '^') {
			if (m_strText[i + 1] == '/') { // Fractions
				int EndCaret = m_strText.Find('^', i + 1);

				if (EndCaret != - 1) {
					int FractionBar = m_strText.Find('/', i + 2);
					if (FractionBar != - 1 && FractionBar < EndCaret) {
						m_strText.SetAt(i++, '\\');
						m_strText.SetAt(i, 'S');
						m_strText.SetAt(EndCaret, ';');
						i = EndCaret;
					}
				}
			}
		}
	}
}

EoDbPrimitive* EoDbText::Clone(OdDbDatabasePtr database) const {
	return (EoDbText::Create(*this, database));
}

void EoDbText::Display(AeSysView* view, CDC* deviceContext) {
	EoInt16 ColorIndex = LogicalColorIndex();
	pstate.SetColorIndex(deviceContext, ColorIndex);

	EoInt16 LinetypeIndex = pstate.LinetypeIndex();
	pstate.SetLinetypeIndex(deviceContext, 1);

	DisplayText(view, deviceContext, m_FontDefinition, m_ReferenceSystem, m_strText);
	pstate.SetLinetypeIndex(deviceContext, LinetypeIndex);
}

EoDbFontDefinition EoDbText::FontDefinition() const {
	return (m_FontDefinition);
}

void EoDbText::FormatExtra(CString& extra) const {
	extra.Empty();
	extra += L"Color;" + FormatColorIndex() + L"\t";
	extra += L"Font;" + m_FontDefinition.FontName() + L"\t";
	extra += L"Precision;" + m_FontDefinition.FormatPrecision() + L"\t";
	extra += L"Path;" + m_FontDefinition.FormatPath() + L"\t";
	extra += L"Alignment;(" + m_FontDefinition.FormatHorizonatlAlignment() + L"," + m_FontDefinition.FormatVerticalAlignment() + L")\t";
	CString Spacing;
	Spacing.Format(L"Spacing;%f\t", m_FontDefinition.CharacterSpacing());
	extra += Spacing;
	CString Length;
	Length.Format(L"Number of Characters;%d\t", m_strText.GetLength());
	extra += Length;
	extra += L"Text;" + m_strText;
}

void EoDbText::FormatGeometry(CString& geometry) const {
	EoGeReferenceSystem ReferenceSystem = m_ReferenceSystem;
	OdGePoint3d Origin = ReferenceSystem.Origin();
	CString OriginString;
	OriginString.Format(L"Origin;%f;%f;%f\t", Origin.x, Origin.y, Origin.z);
	geometry +=  OriginString;
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

void EoDbText::GetBoundingBox(OdGePoint3dArray& boundingBox, double spaceFactor) const {
	int Length = LengthSansFormattingCharacters(m_strText);
	text_GetBoundingBox(m_FontDefinition, m_ReferenceSystem, Length, spaceFactor, boundingBox);
}

OdGePoint3d EoDbText::GetCtrlPt() const {
	return m_ReferenceSystem.Origin();
}

void EoDbText::GetExtents(AeSysView* view, OdGeExtents3d& extents) const {
	OdGePoint3dArray BoundingBox;

	text_GetBoundingBox(m_FontDefinition, m_ReferenceSystem, m_strText.GetLength(), 0., BoundingBox);

	for (EoUInt16 w = 0; w < BoundingBox.size(); w++) {
		extents.addPoint(BoundingBox[w]);
	}
}

OdGePoint3d	EoDbText::GoToNxtCtrlPt() const {
	return (m_ReferenceSystem.Origin());
}

bool EoDbText::Is(EoUInt16 type) const {
	return type == EoDb::kTextPrimitive;
}

bool EoDbText::IsEqualTo(EoDbPrimitive* primitive) const {
	return false;
}

bool EoDbText::IsInView(AeSysView* view) const {
	EoGePoint4d pt[2];

	OdGePoint3dArray BoundingBox;

	text_GetBoundingBox(m_FontDefinition, m_ReferenceSystem, m_strText.GetLength(), 0., BoundingBox);

	for (size_t n = 0; n <= 2; ) {
		pt[0] = EoGePoint4d(BoundingBox[n++], 1.);
		pt[1] = EoGePoint4d(BoundingBox[n++], 1.);

		view->ModelViewTransformPoints(2, pt);

		if (EoGePoint4d::ClipLine(pt[0], pt[1]))
			return true;
	}
	return false;
}

bool EoDbText::IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	EoGePoint4d pt(m_ReferenceSystem.Origin(), 1.);
	view->ModelViewTransformPoint(pt);

	return ((point.DistanceToPointXY(pt) < sm_SelectApertureSize) ? true : false);
}

void EoDbText::ModifyNotes(EoDbFontDefinition& fontDefinition, EoDbCharacterCellDefinition& characterCellDefinition, int iAtt) {
	if (iAtt == TM_TEXT_ALL) {
		m_ColorIndex = pstate.ColorIndex();
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

void EoDbText::ModifyState() {
	EoDbPrimitive::ModifyState();

	m_FontDefinition = pstate.FontDefinition();

	EoDbCharacterCellDefinition CharacterCellDefinition = pstate.CharacterCellDefinition();

	m_ReferenceSystem.Rescale(CharacterCellDefinition);
}

OdGePoint3d EoDbText::Position() const {
	return m_ReferenceSystem.Origin();
}

EoGeReferenceSystem EoDbText::ReferenceSystem() const {
	return (m_ReferenceSystem);
}

double EoDbText::Rotation() const {
	OdGeVector3d HorizontalAxis = ReferenceSystem().XDirection();

	double Angle = 0.;

	Angle = atan2(HorizontalAxis.y, HorizontalAxis.x); // -pi to pi radians
	if (Angle < 0.) {
		Angle  += TWOPI;
	}
	return (Angle);
}

OdGePoint3d EoDbText::SelectAtControlPoint(AeSysView*, const EoGePoint4d& point) const {
	sm_ControlPointIndex = USHRT_MAX;
	return (point.Convert3d());
}

bool EoDbText::SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const {
	OdGePoint3dArray Points;
	text_GetBoundingBox(m_FontDefinition, m_ReferenceSystem, m_strText.GetLength(), 0., Points);
	return polyline::SelectUsingRectangle(view, lowerLeftCorner, upperRightCorner, Points);
}

bool EoDbText::SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& ptProj) const {
	if (m_strText.GetLength() == 0) {
		return false;
	}
	OdGePoint3dArray BoundingBox;

	text_GetBoundingBox(m_FontDefinition, m_ReferenceSystem, m_strText.GetLength(), 0., BoundingBox);

	EoGePoint4d pt0[] = {EoGePoint4d(BoundingBox[0], 1.), EoGePoint4d(BoundingBox[1], 1.), EoGePoint4d(BoundingBox[2], 1.), EoGePoint4d(BoundingBox[3], 1.)};

	view->ModelViewTransformPoints(4, pt0);

	for (size_t n = 0; n < 4; n++) {
		if (EoGeLineSeg3d(pt0[n].Convert3d(), pt0[(n + 1) % 4].Convert3d()).DirectedRelationshipOf(point.Convert3d()) < 0) {
			return false;
	}
	}
	ptProj = point.Convert3d();

	return true;
}

void EoDbText::SetFontDefinition(const EoDbFontDefinition& fontDefinition) {
	m_FontDefinition = fontDefinition;
}

void EoDbText::SetHorizontalMode(HorizontalAlignment horizontalAlignment) {
	m_FontDefinition.SetHorizontalAlignment(horizontalAlignment);
	if (!m_EntityObjectId.isNull()) {
		OdDbTextPtr Text = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		switch (horizontalAlignment) {
		case EoDb::kAlignCenter:
			Text->setHorizontalMode(OdDb::kTextCenter);
			break;
		case EoDb::kAlignRight:
			Text->setHorizontalMode(OdDb::kTextRight);
			break;
		default:
			Text->setHorizontalMode(OdDb::kTextLeft);
		}
	}
}

void EoDbText::SetReferenceSystem(const EoGeReferenceSystem& referenceSystem) {
	m_ReferenceSystem = referenceSystem;
}

EoDbText& EoDbText::SetTo(const EoDbFontDefinition& fontDefinition, const EoGeReferenceSystem& referenceSystem, const CString& text) {
	// <tas="Text created with "\r\n" (ctrl-enter) not correctly displayed by dwg text entity"</tas>
	m_FontDefinition = fontDefinition;
	m_ReferenceSystem = referenceSystem;
	m_strText = text;

	SetHorizontalMode(fontDefinition.HorizontalAlignment());
	SetVerticalMode(fontDefinition.VerticalAlignment());
	if (!m_EntityObjectId.isNull()) {
		OdDbTextPtr Text = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		Text->setPosition(referenceSystem.Origin());
		Text->setTextString((LPCWSTR) text);
		Text->setHeight(referenceSystem.YDirection().length());
		Text->setRotation(Rotation());
		Text->setAlignmentPoint(Text->position());
	}
	return (*this);
}

void EoDbText::SetText(const CString& text) {
	m_strText = text;
}

void EoDbText::SetVerticalMode(VerticalAlignment verticalAlignment) {
	m_FontDefinition.SetVerticalAlignment(verticalAlignment);
	if (!m_EntityObjectId.isNull()) {
		OdDbTextPtr Text = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		switch (verticalAlignment) {
		case EoDb::kAlignTop:
			Text->setVerticalMode(OdDb::kTextTop);
			break;
		case EoDb::kAlignMiddle:
			Text->setVerticalMode(OdDb::kTextVertMid);
			break;
		default:
			Text->setVerticalMode(OdDb::kTextBase);
		}
	}
}

const CString& EoDbText::Text() {
	return m_strText;
}

void EoDbText::TransformBy(const EoGeMatrix3d& transformMatrix) {
	m_ReferenceSystem.TransformBy(transformMatrix);
}

void EoDbText::TranslateUsingMask(const OdGeVector3d& translate, const DWORD mask) {
	if (mask != 0)
		m_ReferenceSystem.SetOrigin(m_ReferenceSystem.Origin() + translate);
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

void EoDbText::Write(CFile& file, EoByte* buffer) const {
	EoUInt16 NumberOfCharacters = EoUInt16(m_strText.GetLength());

	buffer[3] = EoSbyte((86 + NumberOfCharacters) / 32);
	*((EoUInt16*) &buffer[4]) = EoUInt16(EoDb::kTextPrimitive);
	buffer[6] = EoSbyte(m_ColorIndex == COLORINDEX_BYLAYER ? sm_LayerColorIndex : m_ColorIndex);
	buffer[7] = EoSbyte(m_FontDefinition.Precision());
	*((EoInt16*) &buffer[8]) = 0;
	((EoVaxFloat*) &buffer[10])->Convert(m_FontDefinition.CharacterSpacing());
	buffer[14] = EoSbyte(m_FontDefinition.Path());
	buffer[15] = EoSbyte(m_FontDefinition.HorizontalAlignment());
	buffer[16] = EoSbyte(m_FontDefinition.VerticalAlignment());

	EoGeReferenceSystem ReferenceSystem = m_ReferenceSystem;
	((EoVaxPoint3d*) &buffer[17])->Convert(ReferenceSystem.Origin());
	((EoVaxVector3d*) &buffer[29])->Convert(ReferenceSystem.XDirection());
	((EoVaxVector3d*) &buffer[41])->Convert(ReferenceSystem.YDirection());

	*((EoUInt16*) &buffer[53]) = NumberOfCharacters;
	size_t BufferOffset = 55;
	for (size_t CharacterIndex = 0; CharacterIndex < NumberOfCharacters; CharacterIndex++) {
		buffer[BufferOffset++] = EoByte(m_strText[CharacterIndex]);
	}
	file.Write(buffer, buffer[3] * 32);
}

EoDbText* EoDbText::ConstructFrom(EoDbFile& file) {
	EoInt16 ColorIndex = file.ReadInt16();
	/* EoInt16 LinetypeIndex = */ file.ReadInt16();
	EoDbFontDefinition FontDefinition;
	FontDefinition.Read(file);
	EoGeReferenceSystem ReferenceSystem;
	ReferenceSystem.Read(file);
	CString TextString;
	file.ReadString(TextString);
	EoDbText* Text = new EoDbText();
	Text->SetColorIndex(ColorIndex);
	Text->SetTo(FontDefinition, ReferenceSystem, TextString);
	Text->ConvertFormattingCharacters();
	return (Text);
}

EoDbText* EoDbText::ConstructFrom(EoByte* primitiveBuffer, int versionNumber) {
	EoInt16 ColorIndex;
	EoDbFontDefinition FontDefinition;
	EoGeReferenceSystem ReferenceSystem;
	CString Text;

	FontDefinition.SetPrecision(EoDb::kStrokeType);
	FontDefinition.SetFontName(L"Simplex.psf");

	if (versionNumber == 1) {
		ColorIndex = EoInt16(primitiveBuffer[4] & 0x000f);
		FontDefinition.SetCharacterSpacing(((EoVaxFloat*) &primitiveBuffer[36])->Convert());
		FontDefinition.SetCharacterSpacing(min(max(FontDefinition.CharacterSpacing(), 0.), 4.));

		double d = ((EoVaxFloat*) &primitiveBuffer[40])->Convert();

		switch (int(fmod(d, 10.))) {
		case 3:
			FontDefinition.SetPath(kPathDown);
			break;
		case 2:
			FontDefinition.SetPath(kPathUp);
			break;
		case 1:
			FontDefinition.SetPath(kPathLeft);
			break;
		default:
			FontDefinition.SetPath(kPathRight);
		}
		switch (int(fmod(d / 10., 10.))) {
		case 3:
			FontDefinition.SetHorizontalAlignment(kAlignRight);
			break;
		case 2:
			FontDefinition.SetHorizontalAlignment(kAlignCenter);
			break;
		default:
			FontDefinition.SetHorizontalAlignment(kAlignLeft);
		}
		switch (int(d / 100.)) {
		case 2:
			FontDefinition.SetVerticalAlignment(kAlignTop);
			break;
		case 3:
			FontDefinition.SetVerticalAlignment(kAlignMiddle);
			break;
		default:
			FontDefinition.SetVerticalAlignment(kAlignBottom);
		}
		ReferenceSystem.SetOrigin(((EoVaxPoint3d*) &primitiveBuffer[8])->Convert() * 1.e-3);

		double dChrHgt = ((EoVaxFloat*) &primitiveBuffer[20])->Convert();
		dChrHgt = min(max(dChrHgt, .01e3), 100.e3);

		double dChrExpFac = ((EoVaxFloat*) &primitiveBuffer[24])->Convert();
		dChrExpFac = min(max(dChrExpFac, 0.), 10.);

		ReferenceSystem.SetXDirection(OdGeVector3d(0.6 * dChrHgt * dChrExpFac, 0., 0.) * 1.e-3);
		ReferenceSystem.SetYDirection(OdGeVector3d(0., dChrHgt, 0.) * 1.e-3);

		double Angle = ((EoVaxFloat*) &primitiveBuffer[28])->Convert();
		Angle = min(max(Angle, - TWOPI), TWOPI);

		if (fabs(Angle) > FLT_EPSILON) {
			OdGeVector3d XDirection(ReferenceSystem.XDirection());
			XDirection = XDirection.rotateBy(Angle, OdGeVector3d::kZAxis);
			ReferenceSystem.SetXDirection(XDirection);
			OdGeVector3d YDirection(ReferenceSystem.YDirection());
			YDirection = YDirection.rotateBy(Angle, OdGeVector3d::kZAxis);
			ReferenceSystem.SetYDirection(YDirection);
		}
		char* NextToken = NULL;
		char* pChr = strtok_s((char*) &primitiveBuffer[44], "\\", &NextToken);

		if (pChr == 0) {
			Text = L"EoDbJobFile.PrimText error: Missing string terminator.";
		}
		else if (strlen(pChr) > 132) {
			Text = L"EoDbJobFile.PrimText error: Text too long.";
		} else {
			while (*pChr != 0) {
				if (!isprint(*pChr)) *pChr = '.';
				pChr++;
			}
			Text = &primitiveBuffer[44];
		}
	}
	else {
		ColorIndex = EoInt16(primitiveBuffer[6]);
		FontDefinition.SetCharacterSpacing(((EoVaxFloat*) &primitiveBuffer[10])->Convert());
		switch (primitiveBuffer[14]) {
		case 3:
			FontDefinition.SetPath(kPathDown);
			break;
		case 2:
			FontDefinition.SetPath(kPathUp);
			break;
		case 1:
			FontDefinition.SetPath(kPathLeft);
			break;
		default:
			FontDefinition.SetPath(kPathRight);
		}
		switch (primitiveBuffer[15]) {
		case 3:
			FontDefinition.SetHorizontalAlignment(kAlignRight);
			break;
		case 2:
			FontDefinition.SetHorizontalAlignment(kAlignCenter);
			break;
		default:
			FontDefinition.SetHorizontalAlignment(kAlignLeft);
		}
		switch (primitiveBuffer[16]) {
		case 2:
			FontDefinition.SetVerticalAlignment(kAlignTop);
			break;
		case 3:
			FontDefinition.SetVerticalAlignment(kAlignMiddle);
			break;
		default:
			FontDefinition.SetVerticalAlignment(kAlignBottom);
		}
		ReferenceSystem.SetOrigin(((EoVaxPoint3d*) &primitiveBuffer[17])->Convert());
		ReferenceSystem.SetXDirection(((EoVaxVector3d*) &primitiveBuffer[29])->Convert());
		ReferenceSystem.SetYDirection(((EoVaxVector3d*) &primitiveBuffer[41])->Convert());

		EoInt16 TextLength = *((EoInt16*) &primitiveBuffer[53]);
		primitiveBuffer[55 + TextLength] = '\0';
		Text = CString((LPCSTR) &primitiveBuffer[55]);
	}
	EoDbText* TextPrimitive = new EoDbText();
	TextPrimitive->SetColorIndex(ColorIndex);
	TextPrimitive->SetFontDefinition(FontDefinition);
	TextPrimitive->SetReferenceSystem(ReferenceSystem);
	TextPrimitive->SetText(Text);

	return (TextPrimitive);
}

EoDbText* EoDbText::Create(OdDbDatabasePtr database) {
	OdDbBlockTableRecordPtr BlockTableRecord = database->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

	OdDbTextPtr TextEntity = OdDbText::createObject();
	TextEntity->setDatabaseDefaults(database);
	BlockTableRecord->appendOdDbEntity(TextEntity);
	
	EoDbText* Text = new EoDbText();
	Text->SetEntityObjectId(TextEntity->objectId());
	
	Text->SetColorIndex(pstate.ColorIndex());
	
	return Text;
}

EoDbText* EoDbText::Create(const EoDbText& other, OdDbDatabasePtr database) {
	OdDbBlockTableRecordPtr BlockTableRecord = database->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
	OdDbTextPtr TextEntity = other.EntityObjectId().safeOpenObject()->clone();
	BlockTableRecord->appendOdDbEntity(TextEntity);

	EoDbText* Text = new EoDbText(other);
	Text->SetEntityObjectId(TextEntity->objectId());

	return Text;
}

EoDb::HorizontalAlignment EoDbText::ConvertHorizontalAlignment(const OdDb::TextHorzMode horizontalMode) {
	EoDb::HorizontalAlignment HorizontalAlignment;

	switch (horizontalMode) {
		case OdDb::kTextMid:
		case OdDb::kTextCenter:
			HorizontalAlignment = EoDb::kAlignCenter;
			break;

		case OdDb::kTextRight:
		case OdDb::kTextAlign:
		case OdDb::kTextFit:
			HorizontalAlignment = EoDb::kAlignRight;
			break;

		default: // OdDb::kTextLeft
			HorizontalAlignment = EoDb::kAlignLeft;
	}
	return HorizontalAlignment;

}

EoDb::VerticalAlignment EoDbText::ConvertVerticalAlignment(const OdDb::TextVertMode verticalMode) {
	EoDb::VerticalAlignment VerticalAlignment;

	switch (verticalMode) {
		case OdDb::kTextVertMid:
			VerticalAlignment = EoDb::kAlignMiddle;
			break;

		case OdDb::kTextTop:
			VerticalAlignment = EoDb::kAlignTop;
			break;

		default: // OdDb::kTextBottom & OdDb::kTextBase
			VerticalAlignment = EoDb::kAlignBottom;
	}
	return VerticalAlignment;
}

EoDbText* EoDbText::Create(OdDbTextPtr text) {

	EoDbText* Text = new EoDbText();
	Text->SetEntityObjectId(text->objectId());
	Text->SetColorIndex(text->colorIndex());
	Text->SetLinetypeIndex(EoDbLinetypeTable::LegacyLinetypeIndex(text->linetype()));

	OdDbObjectId TextStyleObjectId = text->textStyle();
	OdDbTextStyleTableRecordPtr TextStyleTableRecordPtr = TextStyleObjectId.safeOpenObject(OdDb::kForRead);

	OdString FileName(L"Simplex.psf");
	EoDb::Precision Precision(EoDb::kStrokeType);

	if (TextStyleTableRecordPtr->isShapeFile()) {
		ATLTRACE2(atlTraceGeneral, 2, L"TextStyle references shape library %s.\n", (LPCWSTR) TextStyleTableRecordPtr->desc()->name());
	}
	else { // shx font file or windows (ttf) font file
		OdString TypeFace;
		bool Bold;
		bool Italic;
		int Charset;
		int PitchAndFamily;

		TextStyleTableRecordPtr->font(TypeFace, Bold, Italic, Charset, PitchAndFamily);

		if (TypeFace != L"") { // windows (ttf) file
			FileName = TypeFace;
			Precision = EoDb::kEoTrueType;
		}
	}

	EoDb::HorizontalAlignment HorizontalAlignment = EoDbText::ConvertHorizontalAlignment(text->horizontalMode());
	EoDb::VerticalAlignment VerticalAlignment = EoDbText::ConvertVerticalAlignment(text->verticalMode());

	OdGePoint3d AlignmentPoint = text->position();
	if (HorizontalAlignment != EoDb::kAlignLeft || VerticalAlignment != EoDb::kAlignBottom)
		AlignmentPoint = text->alignmentPoint();

	EoDbFontDefinition FontDefinition;
	FontDefinition.SetPrecision(Precision);
	FontDefinition.SetFontName((LPCWSTR) FileName);
	FontDefinition.SetHorizontalAlignment(HorizontalAlignment);
	FontDefinition.SetVerticalAlignment(VerticalAlignment);

	EoDbCharacterCellDefinition CharacterCellDefinition;
	CharacterCellDefinition.SetHeight(text->height());
	CharacterCellDefinition.SetWidthFactor(text->widthFactor());
	CharacterCellDefinition.SetRotationAngle(text->rotation());
	CharacterCellDefinition.SetObliqueAngle(text->oblique());

	OdGeVector3d XDirection;
	OdGeVector3d YDirection;
	CharCellDef_EncdRefSys(text->normal(), CharacterCellDefinition, XDirection, YDirection);

	EoGeReferenceSystem ReferenceSystem(AlignmentPoint, XDirection, YDirection);

	Text->SetFontDefinition(FontDefinition);
	Text->SetReferenceSystem(ReferenceSystem);
	Text->SetText((LPCWSTR) text->textString());

	return Text;
}

bool HasFormattingCharacters(const CString& text) {
	for (int i = 0; i < text.GetLength() - 1; i++) {
		if (text[i] == '\\') {
			switch (text[i + 1]) { // Parameter Meaning
				case 'P': // Hard line break
				//case '~':	// Nonbreaking space
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
				case 'S':	// Stacked text or fractions: the S is follwed by two text segments separated by a / (fraction bar) or ^ (no fraction bar)
					return true;
			}
		}
	}
	return false;
}

int FontEscapementAngle(const OdGeVector3d& xAxis) {
	double Angle = 0.;

	Angle = atan2(xAxis.y, xAxis.x); // -pi to pi radians
	if (Angle < 0.) {
		Angle  += TWOPI;
	}
	return EoRound(EoToDegree(Angle) * 10.);
}

OdGePoint3d CalculateInsertionPoint(EoDbFontDefinition& fontDefinition, int numberOfCharacters) {
	OdGePoint3d InsertionPoint(OdGePoint3d::kOrigin);

	if (numberOfCharacters > 0) {
		double dTxtExt = numberOfCharacters + (numberOfCharacters - 1) * (.32 + fontDefinition.CharacterSpacing()) / .6;

		if (fontDefinition.Path() == EoDb::kPathRight || fontDefinition.Path() == EoDb::kPathLeft) {
			if (fontDefinition.Path() == EoDb::kPathRight) {
				if (fontDefinition.HorizontalAlignment() == EoDb::kAlignCenter)
					InsertionPoint.x = -dTxtExt * .5;
				else if (fontDefinition.HorizontalAlignment() == EoDb::kAlignRight)
					InsertionPoint.x = -dTxtExt;
			}
			else {
				if (fontDefinition.HorizontalAlignment() == EoDb::kAlignLeft)
					InsertionPoint.x = dTxtExt;
				else if (fontDefinition.HorizontalAlignment() == EoDb::kAlignCenter)
					InsertionPoint.x = dTxtExt * .5;
				InsertionPoint.x = InsertionPoint.x - 1.;
			}
			if (fontDefinition.VerticalAlignment() == EoDb::kAlignMiddle)
				InsertionPoint.y = -.5;
			else if (fontDefinition.VerticalAlignment() == EoDb::kAlignTop)
				InsertionPoint.y = -1.;
		}
		else if (fontDefinition.Path() == EoDb::kPathDown || fontDefinition.Path() == EoDb::kPathUp) {
			if (fontDefinition.HorizontalAlignment() == EoDb::kAlignCenter)
				InsertionPoint.x = -.5;
			else if (fontDefinition.HorizontalAlignment() == EoDb::kAlignRight)
				InsertionPoint.x = -1.;
			if (fontDefinition.Path() == EoDb::kPathUp) {
				if (fontDefinition.VerticalAlignment() == EoDb::kAlignMiddle)
					InsertionPoint.y = -dTxtExt * .5;
				else if (fontDefinition.VerticalAlignment() == EoDb::kAlignTop)
					InsertionPoint.y = -dTxtExt;
			}
			else {
				if (fontDefinition.VerticalAlignment() == EoDb::kAlignBottom)
					InsertionPoint.y = dTxtExt;
				else if (fontDefinition.VerticalAlignment() == EoDb::kAlignMiddle)
					InsertionPoint.y = dTxtExt * .5;
				InsertionPoint.y = InsertionPoint.y - 1.;
			}
		}
	}
	return InsertionPoint;
}

int LengthSansFormattingCharacters(const CString& text) {
	int Length = text.GetLength();
	int CurrentPosition = 0;

	while (CurrentPosition < text.GetLength()) {
		wchar_t c = text[CurrentPosition++];
		if (c == '\\') {
			c = text[CurrentPosition];
			if (c == 'A') {
				int EndSemicolon = text.Find(';', CurrentPosition);
				if (EndSemicolon != - 1 && EndSemicolon == CurrentPosition + 2) {
					Length -= 4;
					CurrentPosition = EndSemicolon + 1;
				}
			} else if (c == 'F' || c == 'f') {
				int EndSemicolon = text.Find(';', CurrentPosition);
				if (EndSemicolon != - 1) {
					int FormatLength = EndSemicolon - CurrentPosition;
					Length -= FormatLength + 2;
					CurrentPosition = EndSemicolon + 1;
				}
			} else if (c == 'P') {
				Length -= 2;
				CurrentPosition++;
			} else if (c == 'S') {
				int EndSemicolon = text.Find(';', CurrentPosition);
				if (EndSemicolon != - 1) {
					int TextSegmentDelimiter = text.Find('/', CurrentPosition);
					if (TextSegmentDelimiter == - 1)
						TextSegmentDelimiter = text.Find('^', CurrentPosition);

					if (TextSegmentDelimiter != - 1 && TextSegmentDelimiter < EndSemicolon) {
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
	if (text.IsEmpty())
		return;

	if (HasFormattingCharacters(text)) {
		DisplayTextWithFormattingCharacters(view, deviceContext, fontDefinition, referenceSystem, text);
		return;
	}
	EoGeReferenceSystem ReferenceSystem = referenceSystem;

	OdGePoint3d InsertionPoint = CalculateInsertionPoint(fontDefinition, text.GetLength());
	InsertionPoint.transformBy(EoGeMatrix3d::ReferenceSystemToWorld(referenceSystem));
	ReferenceSystem.SetOrigin(InsertionPoint);

	int NumberOfCharactersToDisplay = 0;
	int StartPosition = 0;
	int CurrentPosition = StartPosition;
	while (CurrentPosition < text.GetLength()) {
		wchar_t c = text[CurrentPosition++];

		if (c == '\r' && text[CurrentPosition] == '\n') {
			DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, StartPosition, NumberOfCharactersToDisplay, text);

			ReferenceSystem.SetOrigin(InsertionPoint);
			ReferenceSystem.SetOrigin(text_GetNewLinePos(fontDefinition, ReferenceSystem, 1., 0));
			InsertionPoint = ReferenceSystem.Origin();

			StartPosition += 2 + NumberOfCharactersToDisplay;
			CurrentPosition = StartPosition;
			NumberOfCharactersToDisplay = 0;
		}
		else {
			NumberOfCharactersToDisplay++;
		}
	}
	DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, StartPosition, NumberOfCharactersToDisplay, text);
}

void DisplayTextSegment(AeSysView* view, CDC* deviceContext, EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, int startPosition, int numberOfCharacters, const CString& text) {
	if (deviceContext != 0 && fontDefinition.Precision() == EoDb::kEoTrueType && view->ViewTrueTypeFonts()) {
		OdGeVector3d XDirection(referenceSystem.XDirection());
		OdGeVector3d YDirection(referenceSystem.YDirection());

		view->ModelViewTransformVector(XDirection);
		view->ModelViewTransformVector(YDirection);

		OdGeVector3d PlaneNormal = XDirection.crossProduct(YDirection);
		if (PlaneNormal.isZeroLength()) return;
		PlaneNormal.normalize();
		if (PlaneNormal.isEqualTo(OdGeVector3d::kZAxis)) {
			if (DisplayTextSegmentUsingTrueTypeFont(view, deviceContext, fontDefinition, referenceSystem, startPosition, numberOfCharacters, text)) return;
		}
	}
	DisplayTextSegmentUsingStrokeFont(view, deviceContext, fontDefinition, referenceSystem, startPosition, numberOfCharacters, text);
}

void DisplayTextSegmentUsingStrokeFont(AeSysView* view, CDC* deviceContext, EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, int startPosition, int numberOfCharacters, const CString& text) {
	if (numberOfCharacters == 0) return;

	long* plStrokeFontDef = (long*) theApp.SimplexStrokeFont();
	if (plStrokeFontDef == 0) return;

	OdGeMatrix3d tm = EoGeMatrix3d::ReferenceSystemToWorld(referenceSystem);

	long* plStrokeChrDef = plStrokeFontDef + 96;
	double dChrSpac = 1. + (.32 + fontDefinition.CharacterSpacing()) / .6;

	OdGePoint3d ptStroke = OdGePoint3d::kOrigin;
	OdGePoint3d ptChrPos = ptStroke;
	OdGePoint3d ptLinePos = ptChrPos;

	int n = startPosition;

	while (n < startPosition + numberOfCharacters) {
		polyline::BeginLineStrip();

		int Character = text.GetAt(n);
		if (Character < 32 || Character > 126) Character = '.';

		for (int i = (int) plStrokeFontDef[Character - 32]; i <= plStrokeFontDef[Character - 32 + 1] - 1; i++) {
			int iY = (int) (plStrokeChrDef[i - 1] % 4096L);
			if ((iY & 2048) != 0)
				iY = -(iY - 2048);
			int iX = (int) ((plStrokeChrDef[i - 1] / 4096L) % 4096L);
			if ((iX & 2048) != 0)
				iX = -(iX - 2048);

			ptStroke += OdGeVector3d(.01 / .6 * iX, .01 * iY, 0.);

			if (plStrokeChrDef[i - 1] / 16777216 == 5) {
				polyline::__End(view, deviceContext, 1);
				polyline::BeginLineStrip();
			}
			polyline::SetVertex(tm * ptStroke);
		}
		polyline::__End(view, deviceContext, 1);

		switch (fontDefinition.Path()) {
			case EoDb::kPathLeft:
				ptChrPos.x -= dChrSpac;
				break;
			case EoDb::kPathUp:
				ptChrPos.y += dChrSpac;
				break;
			case EoDb::kPathDown:
				ptChrPos.y -= dChrSpac;
				break;
			default:
				ptChrPos.x += dChrSpac;
		}
		ptStroke = ptChrPos;
		n++;
	}
}

bool DisplayTextUsingWindowsFontOutline(CDC* deviceContext, int x, int y, const CString& text) {
	deviceContext->BeginPath();
	deviceContext->TextOutW(x, y, text);
	deviceContext->EndPath();

	int nNumPts = deviceContext->GetPath(NULL, NULL, 0);
	if (nNumPts == 0) {
		return true;
	}

	// Allocate memory to hold points and stroke types from the path.
	LPPOINT lpPoints = NULL;
	LPBYTE lpTypes = NULL;
	try
	{
		lpPoints = new POINT[nNumPts];
		lpTypes = new BYTE[nNumPts];
	}
	catch (CException* pe)
	{
		delete[] lpPoints;
		lpPoints = NULL;
		delete[] lpTypes;
		lpTypes = NULL;
		pe->Delete();
	}
	if (lpPoints == NULL || lpTypes == NULL) {
		return true;
	}
	// Now that we have the memory, really get the path data.
	nNumPts = deviceContext->GetPath(lpPoints, lpTypes, nNumPts);

	// If it worked, draw the lines.

	if (nNumPts != -1) {
		deviceContext->PolyDraw(lpPoints, lpTypes, nNumPts);
	}

	// Release the memory we used
	delete[] lpPoints;
	delete[] lpTypes;
	return true;
}

bool DisplayTextSegmentUsingTrueTypeFont(AeSysView* view, CDC* deviceContext, EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, int startPosition, int numberOfCharacters, const CString& text) {
	if (numberOfCharacters <= 0)
		return true;

	OdGeMatrix3d tm = EoGeMatrix3d::ReferenceSystemToWorld(referenceSystem);

	OdGePoint3d Origin = OdGePoint3d::kOrigin;
	EoGePoint4d StartPoint(Origin.transformBy(tm), 1.);
	view->ModelViewTransformPoint(StartPoint);
	CPoint ProjectedStartPoint = view->DoViewportProjection(StartPoint);

	EoGePoint4d ptsBox[3];

	OdGePoint3d TopLeft(0., 1., 0.);
	TopLeft.transformBy(tm);
	ptsBox[1] = EoGePoint4d(TopLeft, 1.);
	view->ModelViewTransformPoint(ptsBox[1]);
	
	OdGePoint3d BottomRight(1., 0., 0.);
	BottomRight.transformBy(tm);
	ptsBox[2] = EoGePoint4d(BottomRight, 1.);
	view->ModelViewTransformPoint(ptsBox[2]);

	CPoint pnt[4];

	pnt[1] = view->DoViewportProjection(ptsBox[1]);
	pnt[2] = view->DoViewportProjection(ptsBox[2]);

	OdGeVector3d vX(double(pnt[2].x - ProjectedStartPoint.x), double(pnt[2].y - ProjectedStartPoint.y), 0.);
	OdGeVector3d vY(double(pnt[1].x - ProjectedStartPoint.x), double(pnt[1].y - ProjectedStartPoint.y), 0.);

	double dHeight = vY.length();
	if (dHeight == 0.) {
		return true;
	}
	LOGFONT logfont;
	memset(&logfont, 0, sizeof(logfont));
	logfont.lfHeight = - EoRound(1.33 * dHeight);
	logfont.lfEscapement = - FontEscapementAngle(vX);
	logfont.lfOrientation = logfont.lfEscapement;
	logfont.lfWeight = FW_NORMAL;
	wcscpy_s(logfont.lfFaceName, LF_FACESIZE, fontDefinition.FontName());

	CFont font;
	font.CreateFontIndirect(&logfont);
	CFont* pfntold = (CFont*) deviceContext->SelectObject(&font);
	UINT uTextAlign = deviceContext->SetTextAlign(TA_LEFT | TA_BASELINE);	
	int iBkMode = deviceContext->SetBkMode(TRANSPARENT);
	
	deviceContext->TextOutW(ProjectedStartPoint.x, ProjectedStartPoint.y, text.Mid(startPosition));
	
//	DisplayTextUsingWindowsFontOutline(deviceContext, ProjectedStartPoint.x, ProjectedStartPoint.y, text.Mid(startPosition));

	deviceContext->SetBkMode(iBkMode);
	deviceContext->SetTextAlign(uTextAlign);
	deviceContext->SelectObject(pfntold);

	return true;
}

void DisplayTextWithFormattingCharacters(AeSysView* view, CDC* deviceContext, EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, const CString& text) {
	EoGeReferenceSystem ReferenceSystem = referenceSystem;

	OdGeMatrix3d tm = EoGeMatrix3d::ReferenceSystemToWorld(referenceSystem);

	int Length = LengthSansFormattingCharacters(text);

	OdGePoint3d InsertionPoint = CalculateInsertionPoint(fontDefinition, Length);
	InsertionPoint.transformBy(tm);
	ReferenceSystem.SetOrigin(InsertionPoint);

	int NumberOfCharactersToDisplay = 0;
	int StartPosition = 0;
	int CurrentPosition = StartPosition;

	while (CurrentPosition < text.GetLength()) {
		wchar_t c = text[CurrentPosition++];
		if (c != '\\') {
			NumberOfCharactersToDisplay++;
		}
		else {
			c = text[CurrentPosition];
			if (c == 'F' || c == 'f') {
				int EndSemicolon = text.Find(';', CurrentPosition);
				if (EndSemicolon != - 1) {
					if (CurrentPosition + 1 < EndSemicolon) {
						DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, StartPosition, NumberOfCharactersToDisplay, text);
						
						StartPosition = EndSemicolon + 1;
						CurrentPosition = StartPosition;
						NumberOfCharactersToDisplay = 0;
					}
				}
			} else if (c == 'P') { // Hard line bresk
				if (CurrentPosition < text.GetLength()) {
					DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, StartPosition, NumberOfCharactersToDisplay, text);

					ReferenceSystem.SetOrigin(InsertionPoint);
					ReferenceSystem.SetOrigin(text_GetNewLinePos(fontDefinition, ReferenceSystem, 1., 0));
					InsertionPoint = ReferenceSystem.Origin();
					StartPosition += 2 + NumberOfCharactersToDisplay;
					CurrentPosition = StartPosition;
					NumberOfCharactersToDisplay = 0;
				}
			} else if (c == 'A') { // Change alignment to bottom, center middle
				int EndSemicolon = text.Find(';', CurrentPosition);
				if (EndSemicolon != - 1) {
					if (CurrentPosition + 1 < EndSemicolon) {
						wchar_t Parameter = text[CurrentPosition + 1];
						if (Parameter >= '0' && Parameter <= '2') {
							if (NumberOfCharactersToDisplay > 0) { // display text segment preceding the formatting
								DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, StartPosition, NumberOfCharactersToDisplay, text);

								// Offset the line position left of current position
								ReferenceSystem.SetOrigin(text_GetNewLinePos(fontDefinition, referenceSystem, 0., NumberOfCharactersToDisplay * (1 + .32 / .6)));
								InsertionPoint = ReferenceSystem.Origin();
							}
							StartPosition = EndSemicolon + 1;
							CurrentPosition = StartPosition;
							NumberOfCharactersToDisplay = text.GetLength() - StartPosition;

							if (Parameter == '1') {
								ReferenceSystem.SetOrigin(text_GetNewLinePos(fontDefinition, ReferenceSystem, .5, 0.));
							} else if (Parameter == '2') {
								ReferenceSystem.SetOrigin(text_GetNewLinePos(fontDefinition, ReferenceSystem, - .5, 0.));
							}
							DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, StartPosition, NumberOfCharactersToDisplay, text);
							return;
						}
					}
				}
			} else if (c == 'S') { // Stacked text or fractions
				int EndSemicolon = text.Find(';', CurrentPosition);
				if (EndSemicolon != - 1) {
					int TextSegmentDelimiter = text.Find('/', CurrentPosition);
					if (TextSegmentDelimiter == - 1)
						TextSegmentDelimiter = text.Find('^', CurrentPosition);

					if (TextSegmentDelimiter != - 1 && TextSegmentDelimiter < EndSemicolon) {
						if (NumberOfCharactersToDisplay > 0) { // display text segment preceding the formatting
							DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, StartPosition, NumberOfCharactersToDisplay, text);
							StartPosition += NumberOfCharactersToDisplay;
						}
						// Offset the line position up and conditionally left of current position
						ReferenceSystem.SetOrigin(text_GetNewLinePos(fontDefinition, ReferenceSystem, - .35, NumberOfCharactersToDisplay * (1 + .32 / .6)));
						InsertionPoint = ReferenceSystem.Origin();
						StartPosition += 2; // skip the formatting characters
						NumberOfCharactersToDisplay = TextSegmentDelimiter - StartPosition;
						if (NumberOfCharactersToDisplay > 0) { // Display superscripted text segment
							DisplayTextSegment(view, deviceContext, fontDefinition, ReferenceSystem, StartPosition, NumberOfCharactersToDisplay, text);
							StartPosition += NumberOfCharactersToDisplay;
						}
						// Offset the line position back down left
						ReferenceSystem.SetOrigin(text_GetNewLinePos(fontDefinition, ReferenceSystem, .35, NumberOfCharactersToDisplay * (1 + .32 / .6) - .72));
						InsertionPoint = ReferenceSystem.Origin();

						if (text[TextSegmentDelimiter] == '/') { // display the text segment delimitier
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
						ReferenceSystem.SetOrigin(text_GetNewLinePos(fontDefinition, ReferenceSystem, - .35, NumberOfCharactersToDisplay * (1 + .32 / .6)));
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
void text_GetBoundingBox(const EoDbFontDefinition& fontDefinition, const EoGeReferenceSystem& referenceSystem, int numberOfCharacters, double spaceFactor, OdGePoint3dArray& boundingBox) {
	boundingBox.setLogicalLength(4);
	if (numberOfCharacters > 0) {
		OdGeMatrix3d tm = EoGeMatrix3d::ReferenceSystemToWorld(referenceSystem);

		double TextHeight = 1.;
		double TextWidth = 1.;

		double CharacterSpacing = (.32 + fontDefinition.CharacterSpacing()) / .6;
		double d = (double) numberOfCharacters + CharacterSpacing * (double) (numberOfCharacters - 1);

		if (fontDefinition.Path() == EoDb::kPathRight || fontDefinition.Path() == EoDb::kPathLeft) {
			TextWidth = d;
		}
		else {
			TextHeight = d;
		}
		boundingBox.setAll(OdGePoint3d::kOrigin);

		if (fontDefinition.HorizontalAlignment() == EoDb::kAlignLeft) {
			boundingBox[2].x = TextWidth;
		} else if (fontDefinition.HorizontalAlignment() == EoDb::kAlignCenter) {
			boundingBox[0].x = -TextWidth * .5;
			boundingBox[2].x = boundingBox[0].x + TextWidth;
		} else {
			boundingBox[0].x = -TextWidth;
		}
		if (fontDefinition.VerticalAlignment() == EoDb::kAlignTop) {
			boundingBox[0].y = -TextHeight;
		} else if (fontDefinition.VerticalAlignment() == EoDb::kAlignMiddle) {
			boundingBox[0].y = -TextHeight * .5;
			boundingBox[2].y = boundingBox[0].y + TextHeight;
		} else {
			boundingBox[2].y = TextHeight;
		}
		if (spaceFactor > DBL_EPSILON) {
			boundingBox[0].x -= spaceFactor / .6;
			boundingBox[0].y -= spaceFactor;
			boundingBox[2].x += spaceFactor / .6;
			boundingBox[2].y += spaceFactor;
		}
		boundingBox[1].x = boundingBox[2].x;
		boundingBox[1].y = boundingBox[0].y;
		boundingBox[3].x = boundingBox[0].x;
		boundingBox[3].y = boundingBox[2].y;

		for (int n = 0; n < 4; n++) {
			boundingBox[n].transformBy(tm);
		}
	}
	else {
		boundingBox.setAll(referenceSystem.Origin());
	}
}
OdGePoint3d text_GetNewLinePos(EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, double dLineSpaceFac, double dChrSpaceFac) {

	OdGePoint3d pt = referenceSystem.Origin();
	OdGeVector3d vPath = referenceSystem.XDirection();
	OdGeVector3d YDirection = referenceSystem.YDirection();

	if (fontDefinition.Path() == EoDb::kPathRight || fontDefinition.Path() == EoDb::kPathLeft) {
		pt += vPath * dChrSpaceFac;
		OdGeVector3d vRefNorm;
		referenceSystem.GetUnitNormal(vRefNorm);

		vPath.normalize();
		vPath *= - (YDirection.length() * dLineSpaceFac);
		vPath.rotateBy(HALF_PI, vRefNorm);
	}
	return (pt + (vPath * 1.5));
}