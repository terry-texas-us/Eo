#include "stdafx.h"

#include "AeSysApp.h"
#include "AeSysView.h"

#include "DbDimStyleTable.h"
#include "DbDimStyleTableRecord.h"
#include "DbAlignedDimension.h"

#include "EoVaxFloat.h"

#include "EoGePolyline.h"

#include "EoDbFile.h"
#include "EoDbDimension.h"
#include "EoDbText.h"

OdUInt16 EoDbDimension::sm_wFlags = 0;

EoDbDimension::EoDbDimension()
	: m_TextColorIndex(1) {
}

EoDbDimension::EoDbDimension(const EoDbDimension& other) {
	m_ColorIndex = other.m_ColorIndex;
	m_LinetypeIndex = other.m_LinetypeIndex;
	m_Line = other.m_Line;

	m_TextColorIndex = other.m_TextColorIndex;
	m_FontDefinition = other.m_FontDefinition;
	m_ReferenceSystem = other.m_ReferenceSystem;
	m_strText = other.m_strText;
}

EoDbDimension::~EoDbDimension() {
}

const EoDbDimension& EoDbDimension::operator=(const EoDbDimension& other) {
	m_ColorIndex = other.m_ColorIndex;
	m_LinetypeIndex = other.m_LinetypeIndex;
	m_Line = other.m_Line;

	m_TextColorIndex = other.m_TextColorIndex;
	m_FontDefinition = other.m_FontDefinition;
	m_ReferenceSystem = other.m_ReferenceSystem;
	m_strText = other.m_strText;

	return (*this);
}

void EoDbDimension::AddReportToMessageList(const OdGePoint3d& point) const {
	double AngleInXYPlane = m_Line.AngleFromXAxis_xy();
	double Relationship;
	m_Line.ParametricRelationshipOf(point, Relationship);

	if (Relationship > .5) {
		AngleInXYPlane += PI;
	}
	AngleInXYPlane = fmod(AngleInXYPlane, TWOPI);

	const double Length = m_Line.length();

	CString Report(L"<Dimension>");
	Report += L" Color:" + FormatColorIndex();
	Report += L" Linetype:" + FormatLinetypeIndex();
	Report += L" [" + theApp.FormatLength(Length, theApp.GetUnits()) + L" @ " + theApp.FormatAngle(AngleInXYPlane) + L"]";
	theApp.AddStringToMessageList(Report);

	theApp.SetEngagedLength(Length);
	theApp.SetEngagedAngle(AngleInXYPlane);
}

void EoDbDimension::AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept {
	CMainFrame::InsertTreeViewControlItem(tree, parent, L"<Dimension>", this);
}

EoDbPrimitive* EoDbDimension::Clone(OdDbBlockTableRecordPtr blockTableRecord) const {
	OdDbAlignedDimensionPtr AlignedDimension = m_EntityObjectId.safeOpenObject()->clone();
	blockTableRecord->appendOdDbEntity(AlignedDimension);

	return EoDbDimension::Create(AlignedDimension);
}

void EoDbDimension::CutAt(const OdGePoint3d& point, EoDbGroup* newGroup) noexcept {
	EoGeLineSeg3d ln;

	if (m_Line.CutAt(point, ln) != 0) {
		EoDbDimension* DimensionPrimitive = new EoDbDimension(*this);

		DimensionPrimitive->m_Line = ln;
		DimensionPrimitive->SetDefaultNote();
		newGroup->AddTail(DimensionPrimitive);
	}
	SetDefaultNote();
}

void EoDbDimension::CutAt2Points(OdGePoint3d* points, EoDbGroupList* groups, EoDbGroupList* newGroups, OdDbDatabasePtr database) {
	EoDbDimension* pDim;
	double	dRel[2];

	m_Line.ParametricRelationshipOf(points[0], dRel[0]);
	m_Line.ParametricRelationshipOf(points[1], dRel[1]);

	if (dRel[0] <= DBL_EPSILON && dRel[1] >= 1. - DBL_EPSILON) {
		// Put entire dimension in trap
		pDim = this;
	} else { // Something gets cut
		pDim = new EoDbDimension(*this);
		if (dRel[0] > DBL_EPSILON && dRel[1] < 1. - DBL_EPSILON) { // Cut section out of middle
			pDim->SetStartPoint(points[1]);
			pDim->SetDefaultNote();
			EoDbGroup* Group = new EoDbGroup;
			Group->AddTail(pDim);
			groups->AddTail(Group);

			pDim = new EoDbDimension(*this);
			pDim->SetStartPoint(points[0]);
			pDim->SetEndPoint(points[1]);
			pDim->SetDefaultNote();
			m_Line.SetEndPoint(points[0]);
		} else if (dRel[1] < 1. - DBL_EPSILON) { // Cut in two and place begin section in trap
			pDim->SetEndPoint(points[1]);
			pDim->SetDefaultNote();
			m_Line.SetStartPoint(points[1]);
		} else { // Cut in two and place end section in trap
			pDim->SetStartPoint(points[0]);
			pDim->SetDefaultNote();
			m_Line.SetEndPoint(points[0]);
		}
		SetDefaultNote();
		EoDbGroup* Group = new EoDbGroup;
		Group->AddTail(this);
		groups->AddTail(Group);
	}
	EoDbGroup* NewGroup = new EoDbGroup;
	NewGroup->AddTail(pDim);
	newGroups->AddTail(NewGroup);
}

void EoDbDimension::Display(AeSysView* view, CDC* deviceContext) {
	OdInt16 ColorIndex = LogicalColorIndex();
	pstate.SetPen(view, deviceContext, ColorIndex, LogicalLinetypeIndex());
	m_Line.Display(view, deviceContext);

	ColorIndex = sm_HighlightColorIndex == 0 ? m_TextColorIndex : sm_HighlightColorIndex;
	if (ColorIndex == COLORINDEX_BYLAYER) {
		ColorIndex = sm_LayerColorIndex;
	} else if (ColorIndex == COLORINDEX_BYBLOCK) {
		ColorIndex = 7;
	}
	pstate.SetColorIndex(deviceContext, ColorIndex);

	const OdInt16 LinetypeIndex = pstate.LinetypeIndex();
	pstate.SetLinetypeIndexPs(deviceContext, 1);

	DisplayText(view, deviceContext, m_FontDefinition, m_ReferenceSystem, m_strText);

	pstate.SetLinetypeIndexPs(deviceContext, LinetypeIndex);
}

void EoDbDimension::FormatExtra(CString& extra) const {
	extra.Empty();
	extra += L"Dimension Color;" + FormatColorIndex() + L"\t";
	extra += L"Dimension Linetype;" + FormatLinetypeIndex() + L"\t";

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

void EoDbDimension::FormatGeometry(CString& geometry) const {
	OdGePoint3d Point;
	Point = m_Line.startPoint();
	CString PointString;
	PointString.Format(L"Start Point;%f;%f;%f\t", Point.x, Point.y, Point.z);
	geometry += PointString;
	Point = m_Line.endPoint();
	PointString.Format(L"End Point;%f;%f;%f\t", Point.x, Point.y, Point.z);
	geometry += PointString;

	EoGeReferenceSystem ReferenceSystem = m_ReferenceSystem;
	const OdGePoint3d Origin = ReferenceSystem.Origin();
	CString OriginString;
	OriginString.Format(L"Text Position;%f;%f;%f\t", Origin.x, Origin.y, Origin.z);
	geometry += OriginString;
	CString VectorString;
	VectorString.Format(L"Text X Axis;%f;%f;%f\t", ReferenceSystem.XDirection().x, ReferenceSystem.XDirection().y, ReferenceSystem.XDirection().z);
	geometry += VectorString;
	VectorString.Format(L"Text Y Axis;%f;%f;%f\t", ReferenceSystem.YDirection().x, ReferenceSystem.YDirection().y, ReferenceSystem.YDirection().z);
	geometry += VectorString;
}

void EoDbDimension::GetAllPoints(OdGePoint3dArray& points) const {
	points.clear();
	points.append(m_Line.startPoint());
	points.append(m_Line.endPoint());
}

// Determination of text extent.
void EoDbDimension::GetBoundingBox(OdGePoint3dArray& boundingBox, double spaceFactor) const {
	text_GetBoundingBox(m_FontDefinition, m_ReferenceSystem, m_strText.GetLength(), spaceFactor, boundingBox);
}

OdGePoint3d EoDbDimension::GetCtrlPt() const {
	return m_Line.midPoint();
}

void EoDbDimension::GetExtents(AeSysView* view, OdGeExtents3d& extents) const {
	extents.addPoint(m_Line.startPoint());
	extents.addPoint(m_Line.endPoint());
	// <tas="Add BoundingBox to extents"</tas>
}

OdGePoint3d EoDbDimension::GoToNxtCtrlPt() const {
	if (sm_ControlPointIndex == 0)
		sm_ControlPointIndex = 1;
	else if (sm_ControlPointIndex == 1) {
		sm_ControlPointIndex = 0;
	} else { // Initial rock .. jump to point at lower left or down if vertical
		const OdGePoint3d ptBeg = m_Line.startPoint();
		const OdGePoint3d ptEnd = m_Line.endPoint();

		if (ptEnd.x > ptBeg.x)
			sm_ControlPointIndex = 0;
		else if (ptEnd.x < ptBeg.x)
			sm_ControlPointIndex = 1;
		else if (ptEnd.y > ptBeg.y)
			sm_ControlPointIndex = 0;
		else
			sm_ControlPointIndex = 1;
	}
	return (sm_ControlPointIndex == 0 ? m_Line.startPoint() : m_Line.endPoint());
}

bool EoDbDimension::IsEqualTo(EoDbPrimitive * primitive) const noexcept {
	return false;
}

bool EoDbDimension::IsInView(AeSysView * view) const {
	EoGePoint4d pt[] = {EoGePoint4d(m_Line.startPoint(), 1.), EoGePoint4d(m_Line.endPoint(), 1.)};

	view->ModelViewTransformPoints(2, &pt[0]);

	return (EoGePoint4d::ClipLine(pt[0], pt[1]));
}

bool EoDbDimension::IsPointOnControlPoint(AeSysView * view, const EoGePoint4d & point) const {
	for (OdUInt16 ControlPointIndex = 0; ControlPointIndex < 2; ControlPointIndex++) {
		EoGePoint4d pt(ControlPointIndex == 0 ? m_Line.startPoint() : m_Line.endPoint(), 1.);

		view->ModelViewTransformPoint(pt);

		if (point.DistanceToPointXY(pt) < sm_SelectApertureSize)
			return true;
	}
	return false;
}

void EoDbDimension::ModifyState() noexcept {
	if ((sm_wFlags & 0x0001) != 0) {
		EoDbPrimitive::ModifyState();
	}
	if ((sm_wFlags & 0x0002) != 0) {
		m_FontDefinition = pstate.FontDefinition();
	}
}

const EoDbFontDefinition& EoDbDimension::FontDef() noexcept {
	return m_FontDefinition;
}

const EoGeLineSeg3d& EoDbDimension::Line() noexcept {
	return m_Line;
}

void EoDbDimension::GetPts(OdGePoint3d & ptBeg, OdGePoint3d & ptEnd) {
	ptBeg = m_Line.startPoint();
	ptEnd = m_Line.endPoint();
}

EoGeReferenceSystem EoDbDimension::ReferenceSystem() const {
	return (m_ReferenceSystem);
}

double EoDbDimension::Length() const {
	return m_Line.length();
}

double EoDbDimension::ParametricRelationshipOf(const OdGePoint3d & point) const {
	double dRel;
	m_Line.ParametricRelationshipOf(point, dRel);
	return dRel;
}

OdGePoint3d EoDbDimension::SelectAtControlPoint(AeSysView * view, const EoGePoint4d & point) const {
	sm_ControlPointIndex = SIZE_T_MAX;
	OdGePoint3d ControlPoint;

	double Aperture = sm_SelectApertureSize;

	for (OdUInt16 ControlPointIndex = 0; ControlPointIndex < 2; ControlPointIndex++) {
		EoGePoint4d pt(ControlPointIndex == 0 ? m_Line.startPoint() : m_Line.endPoint(), 1.);

		view->ModelViewTransformPoint(pt);

		const double Distance = point.DistanceToPointXY(pt);

		if (Distance < Aperture) {
			sm_ControlPointIndex = ControlPointIndex;
			ControlPoint = ControlPointIndex == 0 ? m_Line.startPoint() : m_Line.endPoint();
			Aperture = Distance;
		}
	}
	return ControlPoint;
}

bool EoDbDimension::SelectBy(const EoGePoint4d & point, AeSysView * view, OdGePoint3d & ptProj) const {
	sm_wFlags &= ~0x0003;

	EoGePoint4d pt[4];
	pt[0] = EoGePoint4d(m_Line.startPoint(), 1.);
	pt[1] = EoGePoint4d(m_Line.endPoint(), 1.);
	view->ModelViewTransformPoints(2, &pt[0]);

	EoGeLineSeg3d ln;
	ln.set(pt[0].Convert3d(), pt[1].Convert3d());
	if (ln.IsSelectedBy_xy(point.Convert3d(), view->SelectApertureSize(), ptProj, sm_RelationshipOfPoint)) {
		ptProj.z = ln.startPoint().z + sm_RelationshipOfPoint * (ln.endPoint().z - ln.startPoint().z);
		sm_wFlags |= 0x0001;
		return true;
	}
	OdGePoint3dArray ptsExt;
	GetBoundingBox(ptsExt, 0.);

	pt[0] = EoGePoint4d(ptsExt[0], 1.);
	pt[1] = EoGePoint4d(ptsExt[1], 1.);
	pt[2] = EoGePoint4d(ptsExt[2], 1.);
	pt[3] = EoGePoint4d(ptsExt[3], 1.);
	view->ModelViewTransformPoints(4, pt);

	for (size_t n = 0; n < 4; n++) {
		if (EoGeLineSeg3d(pt[n].Convert3d(), pt[(n + 1) % 4].Convert3d()).DirectedRelationshipOf(point.Convert3d()) < 0)
			return false;
	}
	ptProj = point.Convert3d();
	sm_wFlags |= 0x0002;
	return true;
}

bool EoDbDimension::SelectBy(const EoGeLineSeg3d & line, AeSysView * view, OdGePoint3dArray & intersections) {
	polyline::BeginLineStrip();
	polyline::SetVertex(m_Line.startPoint());
	polyline::SetVertex(m_Line.endPoint());

	return polyline::SelectBy(line, view, intersections);
}

bool EoDbDimension::SelectBy(const OdGePoint3d & lowerLeftCorner, const OdGePoint3d & upperRightCorner, AeSysView * view) const {
	polyline::BeginLineStrip();
	polyline::SetVertex(m_Line.startPoint());
	polyline::SetVertex(m_Line.endPoint());

	if (!polyline::SelectBy(lowerLeftCorner, upperRightCorner, view)) {
		OdGePoint3dArray Points;
		GetBoundingBox(Points, 0.);
		return polyline::SelectUsingRectangle(view, lowerLeftCorner, upperRightCorner, Points);
	}
	return true;
}

void EoDbDimension::SetStartPoint(const OdGePoint3d & startPoint) {
	m_Line.SetStartPoint(startPoint);
}

void EoDbDimension::SetText(const CString & str) {
	m_strText = str;
}

void EoDbDimension::SetTextColorIndex(OdInt16 colorIndex) noexcept {
	m_TextColorIndex = colorIndex;
}

void EoDbDimension::SetEndPoint(const OdGePoint3d & endPoint) {
	m_Line.SetEndPoint(endPoint);
}

const CString& EoDbDimension::Text() noexcept {
	return m_strText;
}

const OdInt16& EoDbDimension::TextColorIndex() noexcept {
	return m_TextColorIndex;
}

void EoDbDimension::SetDefaultNote() {
	const auto ActiveView {AeSysView::GetActiveView()};

	m_ReferenceSystem.SetOrigin(m_Line.midPoint());
	double dAng = 0.;
	const wchar_t cText0 = m_strText[0];
	if (cText0 != 'R' && cText0 != 'D') {
		dAng = m_Line.AngleFromXAxis_xy();
		double dDis = .075;
		if (dAng > HALF_PI + RADIAN && dAng < TWOPI - HALF_PI + RADIAN) {
			dAng -= PI;
			dDis = -dDis;
		}
		OdGePoint3d Origin;
		EoGeLineSeg3d(m_ReferenceSystem.Origin(), m_Line.endPoint()).ProjPtFrom_xy(0., dDis, Origin);
		m_ReferenceSystem.SetOrigin(Origin);
	}
	const OdGeVector3d vPlnNorm = ActiveView->CameraDirection();

	OdGeVector3d XDirection;
	OdGeVector3d YDirection;

	YDirection = ActiveView->ViewUp();
	YDirection.rotateBy(dAng, vPlnNorm);
	YDirection *= .1;
	XDirection = YDirection;
	XDirection.rotateBy(-HALF_PI, vPlnNorm);
	XDirection *= .6;

	m_ReferenceSystem.SetXDirection(XDirection);
	m_ReferenceSystem.SetYDirection(YDirection);

	AeSysApp::Units Units = theApp.GetUnits();
	if (Units == AeSysApp::kArchitectural) {
		Units = AeSysApp::kArchitecturalS;
	}
	m_strText = theApp.FormatLength(m_Line.length(), Units);
	if (cText0 == 'R' || cText0 == 'D') {
		m_strText = cText0 + m_strText;
	}
}

void EoDbDimension::SetFontDefinition(const EoDbFontDefinition & fontDefinition) {
	m_FontDefinition = fontDefinition;
}

void EoDbDimension::SetReferenceSystem(const EoGeReferenceSystem & referenceSystem) noexcept {
	m_ReferenceSystem = referenceSystem;
}

void EoDbDimension::SetTextHorizontalAlignment(EoDb::HorizontalAlignment horizontalAlignment) noexcept {
	m_FontDefinition.SetHorizontalAlignment(horizontalAlignment);
}

void EoDbDimension::SetTextVerticalAlignment(EoDb::VerticalAlignment verticalAlignment) noexcept {
	m_FontDefinition.SetVerticalAlignment(verticalAlignment);
}

void EoDbDimension::TransformBy(const EoGeMatrix3d & transformMatrix) {
	if ((sm_wFlags & 0x0001) != 0) {
		m_Line.SetStartPoint(transformMatrix * m_Line.startPoint());
		m_Line.SetEndPoint(transformMatrix * m_Line.endPoint());
	}
	if ((sm_wFlags & 0x0002) != 0) {
		m_ReferenceSystem.TransformBy(transformMatrix);
	}
}

void EoDbDimension::TranslateUsingMask(const OdGeVector3d & translate, const DWORD mask) {
	if ((mask & 1) == 1) {
		m_Line.SetStartPoint(m_Line.startPoint() + translate);
	}

	if ((mask & 2) == 2) {
		m_Line.SetEndPoint(m_Line.endPoint() + translate);
	}
	SetDefaultNote();
}

bool EoDbDimension::Write(EoDbFile & file) const {
	file.WriteUInt16(EoDb::kDimensionPrimitive);

	file.WriteInt16(m_ColorIndex);
	file.WriteInt16(m_LinetypeIndex);
	file.WritePoint3d(m_Line.startPoint());
	file.WritePoint3d(m_Line.endPoint());
	file.WriteInt16(m_TextColorIndex);
	m_FontDefinition.Write(file);
	m_ReferenceSystem.Write(file);
	file.WriteString(m_strText);

	return true;
}

void EoDbDimension::Write(CFile & file, OdUInt8 * buffer) const {
	OdUInt16 NumberOfCharacters = OdUInt16(m_strText.GetLength());

	buffer[3] = OdUInt8((118 + NumberOfCharacters) / 32);
	*((OdUInt16*) & buffer[4]) = OdUInt16(EoDb::kDimensionPrimitive);
	buffer[6] = OdInt8(m_ColorIndex == COLORINDEX_BYLAYER ? sm_LayerColorIndex : m_ColorIndex);
	buffer[7] = OdInt8(m_LinetypeIndex == LINETYPE_BYLAYER ? sm_LayerLinetypeIndex : m_LinetypeIndex);
	if (buffer[7] >= 16) buffer[7] = 2;

	((EoVaxPoint3d*) & buffer[8])->Convert(m_Line.startPoint());
	((EoVaxPoint3d*) & buffer[20])->Convert(m_Line.endPoint());

	buffer[32] = OdInt8(m_ColorIndex);
	buffer[33] = OdInt8(EoDb::kStrokeType);
	*((OdInt16*) & buffer[34]) = 0;
	((EoVaxFloat*) & buffer[36])->Convert(m_FontDefinition.CharacterSpacing());
	buffer[40] = OdInt8(m_FontDefinition.Path());
	buffer[41] = OdInt8(m_FontDefinition.HorizontalAlignment());
	buffer[42] = OdInt8(m_FontDefinition.VerticalAlignment());

	EoGeReferenceSystem ReferenceSystem = m_ReferenceSystem;

	((EoVaxPoint3d*) & buffer[43])->Convert(ReferenceSystem.Origin());
	((EoVaxVector3d*) & buffer[55])->Convert(ReferenceSystem.XDirection());
	((EoVaxVector3d*) & buffer[67])->Convert(ReferenceSystem.YDirection());

	*((OdInt16*) & buffer[79]) = NumberOfCharacters;
	size_t BufferOffset = 81;
	for (size_t CharacterIndex = 0; CharacterIndex < NumberOfCharacters; CharacterIndex++) {
		buffer[BufferOffset++] = OdUInt8(m_strText[CharacterIndex]);
	}
	file.Write(buffer, buffer[3] * 32);
}

EoDbDimension* EoDbDimension::Create(OdDbAlignedDimensionPtr& alignedDimension) {

	if (alignedDimension->dimBlockId()) {
		OdDbBlockTableRecordPtr Block = alignedDimension->dimBlockId().safeOpenObject(OdDb::kForRead);
		auto DimensionBlockName {Block->getName()};
	}
	const auto Measurement {alignedDimension->getMeasurement()};
	auto DimensionText {alignedDimension->dimensionText()};

	OdString FormattedMeasurement;
	if (Measurement >= 0.) {
		alignedDimension->formatMeasurement(FormattedMeasurement, Measurement, DimensionText);
	}
	const auto DimensionBlockPosition {alignedDimension->dimBlockPosition()};
	const auto DimensionBlockRotation {alignedDimension->dimBlockRotation()};
	const auto DimensionBlockScale {alignedDimension->dimBlockScale()};

	const auto TextPosition {alignedDimension->textPosition()};
	const auto TextRotation {alignedDimension->textRotation()};

	OdDbDimStyleTableRecordPtr DimensionStyle {alignedDimension->dimensionStyle().safeOpenObject(OdDb::kForRead)};

	const auto ExtensionLine1Linetype {alignedDimension->getDimExt1Linetype()};
	const auto ExtensionLine2Linetype {alignedDimension->getDimExt2Linetype()};
	const auto DimensionLineLinetype {alignedDimension->getDimLinetype()};
	const auto HorizontalRotation {alignedDimension->horizontalRotation()};
	const auto Elevation {alignedDimension->elevation()};
	const auto Normal {alignedDimension->normal()};
	const auto TextHeight {DimensionStyle->dimtxt()};

	const auto ExtensionLine1Point {alignedDimension->xLine1Point()};
	const auto ExtensionLine2Point {alignedDimension->xLine2Point()};
	const auto DimensionLinePoint {alignedDimension->dimLinePoint()};
	const auto Oblique {alignedDimension->oblique()};

	auto Dimension {new EoDbDimension()};
	Dimension->SetColorIndex(DimensionStyle->dimclrd().colorIndex());
	//Dimension->SetLinetypeIndex(LinetypeIndex);
	Dimension->SetStartPoint(ExtensionLine1Point);
	Dimension->SetEndPoint(ExtensionLine2Point);
	Dimension->SetTextColorIndex(DimensionStyle->dimclrt().colorIndex());

	EoDbFontDefinition FontDefinition;
	FontDefinition.SetHorizontalAlignment(EoDb::kAlignCenter);
	Dimension->SetFontDefinition(FontDefinition);

	auto XDirection {ExtensionLine2Point - ExtensionLine1Point};
	XDirection.normalize();
	auto YDirection {XDirection * TextHeight};
	YDirection.rotateBy(OdaPI2, Normal);
	XDirection *= TextHeight * .6;
	EoGeReferenceSystem ReferenceSystem;
	ReferenceSystem.Set(TextPosition, XDirection, YDirection);
	Dimension->SetReferenceSystem(ReferenceSystem);

	Dimension->SetText((LPCWSTR) FormattedMeasurement);

	return (Dimension);
}

OdDbAlignedDimensionPtr EoDbDimension::Create(OdDbBlockTableRecordPtr blockTableRecord) {
	auto AlignedDimension {OdDbAlignedDimension::createObject()};
	AlignedDimension->setDatabaseDefaults(blockTableRecord->database());

	blockTableRecord->appendOdDbEntity(AlignedDimension);
	AlignedDimension->setColorIndex(pstate.ColorIndex());

	const auto Linetype {EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex())};

	AlignedDimension->setLinetype(Linetype);

	return AlignedDimension;
}

OdDbAlignedDimensionPtr EoDbDimension::Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile & file) {
	const auto ColorIndex {file.ReadInt16()};
	const auto LinetypeIndex {file.ReadInt16()};
	const auto StartPoint {file.ReadPoint3d()};
	const auto EndPoint {file.ReadPoint3d()};
	const auto TextColorIndex {file.ReadInt16()};

	// <tas'"Font definition and reference system not directly used to set the dimension text properties."/>

	EoDbFontDefinition FontDefinition;
	FontDefinition.Read(file);
	EoGeReferenceSystem ReferenceSystem;
	ReferenceSystem.Read(file);
	
	// <tas="Any text override not used. The text is auto generated from the measured value."/>

	CString Text;
	file.ReadString(Text);

	auto Database {blockTableRecord->database()};

	auto AlignedDimension {OdDbAlignedDimension::createObject()};
	AlignedDimension->setDatabaseDefaults(Database);

	OdDbDimStyleTablePtr DimStyleTable = Database->getDimStyleTableId().safeOpenObject(OdDb::kForRead);
	auto DimStyleRecord {DimStyleTable->getAt(L"EoStandard")};
	AlignedDimension->setDimensionStyle(DimStyleRecord);

	blockTableRecord->appendOdDbEntity(AlignedDimension);

	AlignedDimension->setColorIndex(ColorIndex);

	const auto Linetype {EoDbPrimitive::LinetypeObjectFromIndex0(Database, LinetypeIndex)};

	AlignedDimension->setLinetype(Linetype);

	AlignedDimension->setXLine1Point(StartPoint);
	AlignedDimension->setXLine2Point(EndPoint);
	AlignedDimension->setDimLinePoint(EndPoint);
	AlignedDimension->measurement(); // initial compute of the measurement

	AlignedDimension->downgradeOpen();

	return (AlignedDimension);
}

OdDbAlignedDimensionPtr EoDbDimension::Create(OdDbBlockTableRecordPtr blockTableRecord, OdUInt8 * primitiveBuffer, int versionNumber) {
	const auto ColorIndex {OdInt16(primitiveBuffer[6])};
	const auto LinetypeIndex {OdInt16(primitiveBuffer[7])};
	EoGeLineSeg3d Line;
	Line.set(((EoVaxPoint3d*) & primitiveBuffer[8])->Convert(), ((EoVaxPoint3d*) & primitiveBuffer[20])->Convert());

	const auto TextColorIndex {OdInt16(primitiveBuffer[32])};
	EoDbFontDefinition FontDefinition;
	FontDefinition.SetFontName(L"Simplex.psf");
	FontDefinition.SetPrecision(EoDb::kStrokeType);
	FontDefinition.SetCharacterSpacing(((EoVaxFloat*) & primitiveBuffer[36])->Convert());

	switch (primitiveBuffer[40]) {
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
	switch (primitiveBuffer[41]) {
		case 3:
			FontDefinition.SetHorizontalAlignment(EoDb::kAlignRight);
			break;
		case 2:
			FontDefinition.SetHorizontalAlignment(EoDb::kAlignCenter);
			break;
		default:
			FontDefinition.SetHorizontalAlignment(EoDb::kAlignLeft);
	}
	switch (primitiveBuffer[42]) {
		case 2:
			FontDefinition.SetVerticalAlignment(EoDb::kAlignTop);
			break;
		case 3:
			FontDefinition.SetVerticalAlignment(EoDb::kAlignMiddle);
			break;
		default:
			FontDefinition.SetVerticalAlignment(EoDb::kAlignBottom);
	}
	EoGeReferenceSystem ReferenceSystem;
	ReferenceSystem.SetOrigin(((EoVaxPoint3d*) & primitiveBuffer[43])->Convert());
	ReferenceSystem.SetXDirection(((EoVaxVector3d*) & primitiveBuffer[55])->Convert());
	ReferenceSystem.SetYDirection(((EoVaxVector3d*) & primitiveBuffer[67])->Convert());

	OdInt16 TextLength = *((OdInt16*) & primitiveBuffer[79]);

	primitiveBuffer[81 + TextLength] = '\0';
	CString Text = CString((LPCSTR) & primitiveBuffer[81]);


	auto Database {blockTableRecord->database()};

	auto AlignedDimension {OdDbAlignedDimension::createObject()};
	AlignedDimension->setDatabaseDefaults(Database);

	OdDbDimStyleTablePtr DimStyleTable = Database->getDimStyleTableId().safeOpenObject(OdDb::kForRead);
	auto DimStyleRecord {DimStyleTable->getAt(L"EoStandard")};
	AlignedDimension->setDimensionStyle(DimStyleRecord);

	blockTableRecord->appendOdDbEntity(AlignedDimension);

	AlignedDimension->setColorIndex(ColorIndex);

	const auto Linetype {EoDbPrimitive::LinetypeObjectFromIndex0(Database, LinetypeIndex)};

	AlignedDimension->setLinetype(Linetype);

	AlignedDimension->setXLine1Point(Line.startPoint());
	AlignedDimension->setXLine2Point(Line.endPoint());
	AlignedDimension->setDimLinePoint(Line.endPoint());
	AlignedDimension->measurement(); // initial compute of the measurement

	AlignedDimension->downgradeOpen();

	return (AlignedDimension);
}
