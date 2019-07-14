#include "stdafx.h"
#include <DbBlockTableRecord.h>
#include "AeSys.h"
#include "AeSysView.h"
#include "PrimState.h"
#include <DbDimStyleTable.h>
#include <DbDimStyleTableRecord.h>
#include <DbAlignedDimension.h>
#include "EoVaxFloat.h"
#include "EoGePolyline.h"
#include "EoDbFile.h"
#include "EoDbDimension.h"
#include "EoDbText.h"
IMPLEMENT_DYNAMIC(EoDbDimension, EoDbPrimitive)

unsigned short EoDbDimension::sm_wFlags = 0;

EoDbDimension::EoDbDimension(const EoDbDimension& other) {
	m_ColorIndex = other.m_ColorIndex;
	m_LinetypeIndex = other.m_LinetypeIndex;
	m_Line = other.m_Line;
	m_TextColorIndex = other.m_TextColorIndex;
	m_FontDefinition = other.m_FontDefinition;
	m_ReferenceSystem = other.m_ReferenceSystem;
	m_strText = other.m_strText;
}

EoDbDimension& EoDbDimension::operator=(const EoDbDimension& other) {
	m_ColorIndex = other.m_ColorIndex;
	m_LinetypeIndex = other.m_LinetypeIndex;
	m_Line = other.m_Line;
	m_TextColorIndex = other.m_TextColorIndex;
	m_FontDefinition = other.m_FontDefinition;
	m_ReferenceSystem = other.m_ReferenceSystem;
	m_strText = other.m_strText;
	return *this;
}

void EoDbDimension::AddReportToMessageList(const OdGePoint3d& point) const {
	auto AngleInXYPlane {m_Line.AngleFromXAxis_xy()};
	double Relationship;
	m_Line.ParametricRelationshipOf(point, Relationship);
	if (Relationship > 0.5) {
		AngleInXYPlane += OdaPI;
	}
	AngleInXYPlane = fmod(AngleInXYPlane, Oda2PI);
	const auto Length {m_Line.length()};
	CString Report(L"<Dimension>");
	Report += L" Color:" + FormatColorIndex();
	Report += L" Linetype:" + FormatLinetypeIndex();
	Report += L" [" + theApp.FormatLength(Length, theApp.GetUnits()) + L" @ " + AeSys::FormatAngle(AngleInXYPlane) + L"]";
	AeSys::AddStringToMessageList(Report);
	theApp.SetEngagedLength(Length);
	theApp.SetEngagedAngle(AngleInXYPlane);
}

void EoDbDimension::AddToTreeViewControl(const HWND tree, const HTREEITEM parent) const noexcept {
	CMainFrame::InsertTreeViewControlItem(tree, parent, L"<Dimension>", this);
}

EoDbPrimitive* EoDbDimension::Clone(OdDbBlockTableRecordPtr blockTableRecord) const {
	OdDbAlignedDimensionPtr AlignedDimension = m_EntityObjectId.safeOpenObject()->clone();
	blockTableRecord->appendOdDbEntity(AlignedDimension);
	return Create(AlignedDimension);
}

void EoDbDimension::CutAt(const OdGePoint3d& point, EoDbGroup* newGroup) {
	EoGeLineSeg3d ln;
	if (m_Line.CutAt(point, ln) != 0) {
		auto DimensionPrimitive {new EoDbDimension(*this)};
		DimensionPrimitive->m_Line = ln;
		DimensionPrimitive->SetDefaultNote();
		newGroup->AddTail(DimensionPrimitive);
	}
	SetDefaultNote();
}

void EoDbDimension::CutAt2Points(OdGePoint3d* points, EoDbGroupList* groups, EoDbGroupList* newGroups, OdDbDatabasePtr /*database*/) {
	EoDbDimension* Dimension;
	double dRel[2];
	m_Line.ParametricRelationshipOf(points[0], dRel[0]);
	m_Line.ParametricRelationshipOf(points[1], dRel[1]);
	if (dRel[0] <= DBL_EPSILON && dRel[1] >= 1. - DBL_EPSILON) { // Put entire dimension in trap
		Dimension = this;
	} else { // Something gets cut
		Dimension = new EoDbDimension(*this);
		if (dRel[0] > DBL_EPSILON && dRel[1] < 1. - DBL_EPSILON) { // Cut section out of middle
			Dimension->SetStartPoint(points[1]);
			Dimension->SetDefaultNote();
			auto Group {new EoDbGroup};
			Group->AddTail(Dimension);
			groups->AddTail(Group);
			Dimension = new EoDbDimension(*this);
			Dimension->SetStartPoint(points[0]);
			Dimension->SetEndPoint(points[1]);
			Dimension->SetDefaultNote();
			m_Line.SetEndPoint(points[0]);
		} else if (dRel[1] < 1. - DBL_EPSILON) { // Cut in two and place begin section in trap
			Dimension->SetEndPoint(points[1]);
			Dimension->SetDefaultNote();
			m_Line.SetStartPoint(points[1]);
		} else { // Cut in two and place end section in trap
			Dimension->SetStartPoint(points[0]);
			Dimension->SetDefaultNote();
			m_Line.SetEndPoint(points[0]);
		}
		SetDefaultNote();
		auto Group {new EoDbGroup};
		Group->AddTail(this);
		groups->AddTail(Group);
	}
	auto NewGroup {new EoDbGroup};
	NewGroup->AddTail(Dimension);
	newGroups->AddTail(NewGroup);
}

void EoDbDimension::Display(AeSysView* view, CDC* deviceContext) {
	auto ColorIndex {LogicalColorIndex()};
	g_PrimitiveState.SetPen(view, deviceContext, ColorIndex, LogicalLinetypeIndex());
	m_Line.Display(view, deviceContext);
	ColorIndex = ms_HighlightColorIndex == 0 ? m_TextColorIndex : ms_HighlightColorIndex;
	if (ColorIndex == mc_ColorindexBylayer) {
		ColorIndex = ms_LayerColorIndex;
	} else if (ColorIndex == mc_ColorindexByblock) {
		ColorIndex = 7;
	}
	g_PrimitiveState.SetColorIndex(deviceContext, ColorIndex);
	const auto LinetypeIndex {g_PrimitiveState.LinetypeIndex()};
	g_PrimitiveState.SetLinetypeIndexPs(deviceContext, 1);
	DisplayText(view, deviceContext, m_FontDefinition, m_ReferenceSystem, m_strText);
	g_PrimitiveState.SetLinetypeIndexPs(deviceContext, LinetypeIndex);
}

void EoDbDimension::FormatExtra(CString& extra) const {
	extra.Empty();
	extra += L"Dimension Color;" + FormatColorIndex() + L"\t";
	extra += L"Dimension Linetype;" + FormatLinetypeIndex() + L"\t";
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
	extra += L"Text;" + m_strText + L"\t";
}

void EoDbDimension::FormatGeometry(CString& geometry) const {
	auto Point {m_Line.startPoint()};
	CString PointString;
	PointString.Format(L"Start Point;%f;%f;%f\t", Point.x, Point.y, Point.z);
	geometry += PointString;
	Point = m_Line.endPoint();
	PointString.Format(L"End Point;%f;%f;%f\t", Point.x, Point.y, Point.z);
	geometry += PointString;
	const auto ReferenceSystem {m_ReferenceSystem};
	const auto Origin {ReferenceSystem.Origin()};
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
void EoDbDimension::GetBoundingBox(OdGePoint3dArray& boundingBox, const double spaceFactor) const {
	text_GetBoundingBox(m_FontDefinition, m_ReferenceSystem, m_strText.GetLength(), spaceFactor, boundingBox);
}

OdGePoint3d EoDbDimension::GetCtrlPt() const {
	return m_Line.midPoint();
}

void EoDbDimension::GetExtents(AeSysView* /*view*/, OdGeExtents3d& extents) const {
	extents.addPoint(m_Line.startPoint());
	extents.addPoint(m_Line.endPoint());
	// <tas="Add BoundingBox to extents"</tas>
}

OdGePoint3d EoDbDimension::GoToNxtCtrlPt() const {
	if (ms_ControlPointIndex == 0) { 
		ms_ControlPointIndex = 1;
	} else if (ms_ControlPointIndex == 1) {
		ms_ControlPointIndex = 0;
	} else { // Initial rock .. jump to point at lower left or down if vertical
		const auto StartPoint {m_Line.startPoint()};
		const auto EndPoint {m_Line.endPoint()};
		if (EndPoint.x > StartPoint.x) {
			ms_ControlPointIndex = 0;
		} else if (EndPoint.x < StartPoint.x) {
			ms_ControlPointIndex = 1;
		} else if (EndPoint.y > StartPoint.y) {
			ms_ControlPointIndex = 0;
		} else {
			ms_ControlPointIndex = 1;
		}
	}
	return ms_ControlPointIndex == 0 ? m_Line.startPoint() : m_Line.endPoint();
}

bool EoDbDimension::IsEqualTo(EoDbPrimitive* /*primitive*/) const noexcept {
	return false;
}

bool EoDbDimension::IsInView(AeSysView* view) const {
	EoGePoint4d pt[] = {EoGePoint4d(m_Line.startPoint(), 1.0), EoGePoint4d(m_Line.endPoint(), 1.0)};
	view->ModelViewTransformPoints(2, &pt[0]);
	return EoGePoint4d::ClipLine(pt[0], pt[1]);
}

bool EoDbDimension::IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	for (unsigned ControlPointIndex = 0; ControlPointIndex < 2; ControlPointIndex++) {
		EoGePoint4d pt(ControlPointIndex == 0 ? m_Line.startPoint() : m_Line.endPoint(), 1.0);
		view->ModelViewTransformPoint(pt);
		if (point.DistanceToPointXY(pt) < ms_SelectApertureSize) { return true; }
	}
	return false;
}

void EoDbDimension::ModifyState() noexcept {
	if ((sm_wFlags & 1U) != 0) { EoDbPrimitive::ModifyState(); }
	if ((sm_wFlags & 2U) != 0) { m_FontDefinition = g_PrimitiveState.FontDefinition(); }
}

const EoDbFontDefinition& EoDbDimension::FontDefinition() const noexcept {
	return m_FontDefinition;
}

const EoGeLineSeg3d& EoDbDimension::Line() const noexcept {
	return m_Line;
}

void EoDbDimension::GetPts(OdGePoint3d& startPoint, OdGePoint3d& endPoint) const {
	startPoint = m_Line.startPoint();
	endPoint = m_Line.endPoint();
}

EoGeReferenceSystem EoDbDimension::ReferenceSystem() const {
	return m_ReferenceSystem;
}

double EoDbDimension::Length() const {
	return m_Line.length();
}

double EoDbDimension::ParametricRelationshipOf(const OdGePoint3d& point) const {
	double dRel;
	m_Line.ParametricRelationshipOf(point, dRel);
	return dRel;
}

OdGePoint3d EoDbDimension::SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	ms_ControlPointIndex = SIZE_T_MAX;
	OdGePoint3d ControlPoint;
	auto Aperture {ms_SelectApertureSize};
	for (unsigned ControlPointIndex = 0; ControlPointIndex < 2; ControlPointIndex++) {
		EoGePoint4d pt(ControlPointIndex == 0 ? m_Line.startPoint() : m_Line.endPoint(), 1.0);
		view->ModelViewTransformPoint(pt);
		const auto Distance {point.DistanceToPointXY(pt)};
		if (Distance < Aperture) {
			ms_ControlPointIndex = ControlPointIndex;
			ControlPoint = ControlPointIndex == 0 ? m_Line.startPoint() : m_Line.endPoint();
			Aperture = Distance;
		}
	}
	return ControlPoint;
}

bool EoDbDimension::SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& projectedPoint) const {
	sm_wFlags &= ~3U;
	EoGePoint4d pt[4];
	pt[0] = EoGePoint4d(m_Line.startPoint(), 1.0);
	pt[1] = EoGePoint4d(m_Line.endPoint(), 1.0);
	view->ModelViewTransformPoints(2, &pt[0]);
	EoGeLineSeg3d ln;
	ln.set(pt[0].Convert3d(), pt[1].Convert3d());
	if (ln.IsSelectedBy_xy(point.Convert3d(), view->SelectApertureSize(), projectedPoint, ms_RelationshipOfPoint)) {
		projectedPoint.z = ln.startPoint().z + ms_RelationshipOfPoint * (ln.endPoint().z - ln.startPoint().z);
		sm_wFlags |= 1U;
		return true;
	}
	OdGePoint3dArray ptsExt;
	GetBoundingBox(ptsExt, 0.0);
	pt[0] = EoGePoint4d(ptsExt[0], 1.0);
	pt[1] = EoGePoint4d(ptsExt[1], 1.0);
	pt[2] = EoGePoint4d(ptsExt[2], 1.0);
	pt[3] = EoGePoint4d(ptsExt[3], 1.0);
	view->ModelViewTransformPoints(4, pt);
	for (auto n = 0; n < 4; n++) {
		if (EoGeLineSeg3d(pt[n].Convert3d(), pt[(n + 1) % 4].Convert3d()).DirectedRelationshipOf(point.Convert3d()) < 0) { return false; }
	}
	projectedPoint = point.Convert3d();
	sm_wFlags |= 2U;
	return true;
}

bool EoDbDimension::SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view, OdGePoint3dArray& intersections) {
	polyline::BeginLineStrip();
	polyline::SetVertex(m_Line.startPoint());
	polyline::SetVertex(m_Line.endPoint());
	return polyline::SelectUsingLineSeg(lineSeg, view, intersections);
}

bool EoDbDimension::SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const {
	polyline::BeginLineStrip();
	polyline::SetVertex(m_Line.startPoint());
	polyline::SetVertex(m_Line.endPoint());
	if (!polyline::SelectUsingRectangle(lowerLeftCorner, upperRightCorner, view)) {
		OdGePoint3dArray Points;
		GetBoundingBox(Points, 0.0);
		return polyline::SelectUsingRectangle(view, lowerLeftCorner, upperRightCorner, Points);
	}
	return true;
}

void EoDbDimension::SetStartPoint(const OdGePoint3d& startPoint) {
	m_Line.SetStartPoint(startPoint);
}

void EoDbDimension::SetText(const CString& text) {
	m_strText = text;
}

void EoDbDimension::SetTextColorIndex(const short colorIndex) noexcept {
	m_TextColorIndex = colorIndex;
}

void EoDbDimension::SetEndPoint(const OdGePoint3d& endPoint) {
	m_Line.SetEndPoint(endPoint);
}

const CString& EoDbDimension::Text() const noexcept {
	return m_strText;
}

const short& EoDbDimension::TextColorIndex() const noexcept {
	return m_TextColorIndex;
}

void EoDbDimension::SetDefaultNote() {
	const auto ActiveView {AeSysView::GetActiveView()};
	m_ReferenceSystem.SetOrigin(m_Line.midPoint());
	auto Angle {0.0};
	const auto FirstCharacter {m_strText[0]};
	if (FirstCharacter != 'R' && FirstCharacter != 'D') {
		Angle = m_Line.AngleFromXAxis_xy();
		auto Distance {.075};
		if (Angle > OdaPI2 + OdaPI / 180.0 && Angle < Oda2PI - OdaPI2 + OdaPI2) {
			Angle -= OdaPI;
			Distance = -Distance;
		}
		OdGePoint3d Origin;
		EoGeLineSeg3d(m_ReferenceSystem.Origin(), m_Line.endPoint()).ProjPtFrom_xy(0.0, Distance, Origin);
		m_ReferenceSystem.SetOrigin(Origin);
	}
	const auto ActiveViewPlaneNormal {ActiveView->CameraDirection()};
	auto YDirection {ActiveView->ViewUp()};
	YDirection.rotateBy(Angle, ActiveViewPlaneNormal);
	YDirection *= 0.1;
	auto XDirection {YDirection};
	XDirection.rotateBy(-OdaPI2, ActiveViewPlaneNormal);
	XDirection *= 0.6;
	m_ReferenceSystem.SetXDirection(XDirection);
	m_ReferenceSystem.SetYDirection(YDirection);
	auto Units {theApp.GetUnits()};
	if (Units == AeSys::kArchitectural) { Units = AeSys::kArchitecturalS; }
	m_strText = theApp.FormatLength(m_Line.length(), Units);
	if (FirstCharacter == 'R' || FirstCharacter == 'D') { m_strText = FirstCharacter + m_strText; }
}

void EoDbDimension::SetFontDefinition(const EoDbFontDefinition& fontDefinition) noexcept {
	m_FontDefinition = fontDefinition;
}

void EoDbDimension::SetReferenceSystem(const EoGeReferenceSystem& referenceSystem) noexcept {
	m_ReferenceSystem = referenceSystem;
}

void EoDbDimension::SetTextHorizontalAlignment(const EoDb::HorizontalAlignment horizontalAlignment) noexcept {
	m_FontDefinition.SetHorizontalAlignment(horizontalAlignment);
}

void EoDbDimension::SetTextVerticalAlignment(const EoDb::VerticalAlignment verticalAlignment) noexcept {
	m_FontDefinition.SetVerticalAlignment(verticalAlignment);
}

void EoDbDimension::TransformBy(const EoGeMatrix3d& transformMatrix) {
	if ((sm_wFlags & 1U) != 0) {
		m_Line.SetStartPoint(transformMatrix * m_Line.startPoint());
		m_Line.SetEndPoint(transformMatrix * m_Line.endPoint());
	}
	if ((sm_wFlags & 2U) != 0) {
		m_ReferenceSystem.TransformBy(transformMatrix);
	}
}

void EoDbDimension::TranslateUsingMask(const OdGeVector3d& translate, const unsigned mask) {
	if ((mask & 1U) == 1) { m_Line.SetStartPoint(m_Line.startPoint() + translate); }
	if ((mask & 2U) == 2) { m_Line.SetEndPoint(m_Line.endPoint() + translate); }
	SetDefaultNote();
}

bool EoDbDimension::Write(EoDbFile& file) const {
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

void EoDbDimension::Write(CFile& file, unsigned char* buffer) const {
	const auto NumberOfCharacters {static_cast<short>(m_strText.GetLength())};
	buffer[3] = static_cast<unsigned char>((118 + NumberOfCharacters) / 32);
	*reinterpret_cast<unsigned short*>(& buffer[4]) = static_cast<unsigned short>(EoDb::kDimensionPrimitive);
	buffer[6] = static_cast<unsigned char>(m_ColorIndex == mc_ColorindexBylayer ? ms_LayerColorIndex : m_ColorIndex);
	buffer[7] = static_cast<unsigned char>(m_LinetypeIndex == mc_LinetypeBylayer ? ms_LayerLinetypeIndex : m_LinetypeIndex);
	if (buffer[7] >= 16) { buffer[7] = 2; }
	reinterpret_cast<EoVaxPoint3d*>(& buffer[8])->Convert(m_Line.startPoint());
	reinterpret_cast<EoVaxPoint3d*>(& buffer[20])->Convert(m_Line.endPoint());
	buffer[32] = static_cast<unsigned char>(m_ColorIndex);
	buffer[33] = static_cast<signed char>(EoDb::kStrokeType);
	*reinterpret_cast<short*>(& buffer[34]) = 0;
	reinterpret_cast<EoVaxFloat*>(& buffer[36])->Convert(m_FontDefinition.CharacterSpacing());
	buffer[40] = static_cast<unsigned char>(m_FontDefinition.Path());
	buffer[41] = static_cast<unsigned char>(m_FontDefinition.HorizontalAlignment());
	buffer[42] = static_cast<unsigned char>(m_FontDefinition.VerticalAlignment());
	const auto ReferenceSystem {m_ReferenceSystem};
	reinterpret_cast<EoVaxPoint3d*>(& buffer[43])->Convert(ReferenceSystem.Origin());
	reinterpret_cast<EoVaxVector3d*>(& buffer[55])->Convert(ReferenceSystem.XDirection());
	reinterpret_cast<EoVaxVector3d*>(& buffer[67])->Convert(ReferenceSystem.YDirection());
	*reinterpret_cast<short*>(& buffer[79]) = NumberOfCharacters;
	unsigned BufferOffset = 81;
	for (short CharacterIndex = 0; CharacterIndex < NumberOfCharacters; CharacterIndex++) {
		buffer[BufferOffset++] = static_cast<unsigned char>(m_strText[CharacterIndex]);
	}
	file.Write(buffer, static_cast<unsigned>(buffer[3] * 32));
}

EoDbDimension* EoDbDimension::Create(OdDbAlignedDimensionPtr& alignedDimension) {
	if (alignedDimension->dimBlockId()) {
		OdDbBlockTableRecordPtr Block = alignedDimension->dimBlockId().safeOpenObject(OdDb::kForRead);
		auto DimensionBlockName {Block->getName()};
	}
	const auto Measurement {alignedDimension->getMeasurement()};
	const auto DimensionText {alignedDimension->dimensionText()};
	OdString FormattedMeasurement;
	if (Measurement >= 0.0) {
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
	Dimension->SetColorIndex(static_cast<short>(DimensionStyle->dimclrd().colorIndex()));
	//Dimension->SetLinetypeIndex(LinetypeIndex);
	Dimension->SetStartPoint(ExtensionLine1Point);
	Dimension->SetEndPoint(ExtensionLine2Point);
	Dimension->SetTextColorIndex(static_cast<short>(DimensionStyle->dimclrt().colorIndex()));
	EoDbFontDefinition FontDefinition;
	FontDefinition.SetHorizontalAlignment(EoDb::kAlignCenter);
	Dimension->SetFontDefinition(FontDefinition);
	auto XDirection {ExtensionLine2Point - ExtensionLine1Point};
	XDirection.normalize();
	auto YDirection {XDirection * TextHeight};
	YDirection.rotateBy(OdaPI2, Normal);
	XDirection *= TextHeight * 0.6;
	EoGeReferenceSystem ReferenceSystem;
	ReferenceSystem.Set(TextPosition, XDirection, YDirection);
	Dimension->SetReferenceSystem(ReferenceSystem);
	Dimension->SetText(static_cast<const wchar_t*>(FormattedMeasurement));
	return Dimension;
}

OdDbAlignedDimensionPtr EoDbDimension::Create(OdDbBlockTableRecordPtr blockTableRecord) {
	auto AlignedDimension {OdDbAlignedDimension::createObject()};
	AlignedDimension->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(AlignedDimension);
	AlignedDimension->setColorIndex(static_cast<unsigned short>(g_PrimitiveState.ColorIndex()));
	const auto Linetype {LinetypeObjectFromIndex(g_PrimitiveState.LinetypeIndex())};
	AlignedDimension->setLinetype(Linetype);
	return AlignedDimension;
}

OdDbAlignedDimensionPtr EoDbDimension::Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file) {
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
	const auto Database {blockTableRecord->database()};
	auto AlignedDimension {OdDbAlignedDimension::createObject()};
	AlignedDimension->setDatabaseDefaults(Database);
	OdDbDimStyleTablePtr DimStyleTable = Database->getDimStyleTableId().safeOpenObject(OdDb::kForRead);
	const auto DimStyleRecord {DimStyleTable->getAt(L"EoStandard")};
	AlignedDimension->setDimensionStyle(DimStyleRecord);
	blockTableRecord->appendOdDbEntity(AlignedDimension);
	AlignedDimension->setColorIndex(static_cast<unsigned short>(ColorIndex));
	const auto Linetype {LinetypeObjectFromIndex0(Database, LinetypeIndex)};
	AlignedDimension->setLinetype(Linetype);
	AlignedDimension->setXLine1Point(StartPoint);
	AlignedDimension->setXLine2Point(EndPoint);
	AlignedDimension->setDimLinePoint(EndPoint);
	AlignedDimension->measurement(); // initial compute of the measurement
	AlignedDimension->downgradeOpen();
	return AlignedDimension;
}

OdDbAlignedDimensionPtr EoDbDimension::Create(OdDbBlockTableRecordPtr blockTableRecord, unsigned char* primitiveBuffer, int /*versionNumber*/) {
	const auto ColorIndex {static_cast<short>(primitiveBuffer[6])};
	const auto LinetypeIndex {static_cast<short>(primitiveBuffer[7])};
	EoGeLineSeg3d Line;
	Line.set(reinterpret_cast<EoVaxPoint3d*>(& primitiveBuffer[8])->Convert(), reinterpret_cast<EoVaxPoint3d*>(& primitiveBuffer[20])->Convert());
	const auto TextColorIndex {static_cast<short>(primitiveBuffer[32])};
	EoDbFontDefinition FontDefinition;
	FontDefinition.SetFontName(L"Simplex.psf");
	FontDefinition.SetPrecision(EoDb::kStrokeType);
	FontDefinition.SetCharacterSpacing(reinterpret_cast<EoVaxFloat*>(& primitiveBuffer[36])->Convert());
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
	ReferenceSystem.SetOrigin(reinterpret_cast<EoVaxPoint3d*>(& primitiveBuffer[43])->Convert());
	ReferenceSystem.SetXDirection(reinterpret_cast<EoVaxVector3d*>(& primitiveBuffer[55])->Convert());
	ReferenceSystem.SetYDirection(reinterpret_cast<EoVaxVector3d*>(& primitiveBuffer[67])->Convert());
	const auto TextLength {*reinterpret_cast<short*>(&primitiveBuffer[79])};
	primitiveBuffer[81 + TextLength] = '\0';
	auto Text {CString(reinterpret_cast<LPCSTR>(&primitiveBuffer[81]))};
	const auto Database {blockTableRecord->database()};
	auto AlignedDimension {OdDbAlignedDimension::createObject()};
	AlignedDimension->setDatabaseDefaults(Database);
	OdDbDimStyleTablePtr DimStyleTable = Database->getDimStyleTableId().safeOpenObject(OdDb::kForRead);
	const auto DimStyleRecord {DimStyleTable->getAt(L"EoStandard")};
	AlignedDimension->setDimensionStyle(DimStyleRecord);
	blockTableRecord->appendOdDbEntity(AlignedDimension);
	AlignedDimension->setColorIndex(static_cast<unsigned short>(ColorIndex));
	const auto Linetype {LinetypeObjectFromIndex0(Database, LinetypeIndex)};
	AlignedDimension->setLinetype(Linetype);
	AlignedDimension->setXLine1Point(Line.startPoint());
	AlignedDimension->setXLine2Point(Line.endPoint());
	AlignedDimension->setDimLinePoint(Line.endPoint());
	AlignedDimension->measurement(); // initial compute of the measurement
	AlignedDimension->downgradeOpen();
	return AlignedDimension;
}
