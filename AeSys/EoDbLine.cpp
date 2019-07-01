#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "PrimState.h"
#include "EoVaxFloat.h"
#include "EoGePolyline.h"
#include "EoDbFile.h"
IMPLEMENT_DYNAMIC(EoDbLine, EoDbPrimitive)

EoDbLine::EoDbLine(const EoDbLine& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;
	m_ColorIndex = other.m_ColorIndex;
	m_LinetypeIndex = other.m_LinetypeIndex;
	m_LineSeg = other.m_LineSeg;
}

EoDbLine& EoDbLine::operator=(const EoDbLine& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;
	m_ColorIndex = other.m_ColorIndex;
	m_LinetypeIndex = other.m_LinetypeIndex;
	m_LineSeg = other.m_LineSeg;
	return *this;
}

void EoDbLine::AddReportToMessageList(const OdGePoint3d& point) const {
	auto AngleInXYPlane {m_LineSeg.AngleFromXAxis_xy()};
	double Relationship;
	m_LineSeg.ParametricRelationshipOf(point, Relationship);
	if (Relationship > 0.5) {
		AngleInXYPlane += OdaPI;
	}
	AngleInXYPlane = fmod(AngleInXYPlane, Oda2PI);
	const auto Length {m_LineSeg.length()};
	CString Report(L"<Line>");
	Report += L" Color:" + FormatColorIndex();
	Report += L" Linetype:" + FormatLinetypeIndex();
	Report += L" [" + theApp.FormatLength(Length, theApp.GetUnits()) + L" @ " + AeSys::FormatAngle(AngleInXYPlane) + L"]";
	AeSys::AddStringToMessageList(Report);
	theApp.SetEngagedLength(Length);
	theApp.SetEngagedAngle(AngleInXYPlane);
}

void EoDbLine::AddToTreeViewControl(const HWND tree, const HTREEITEM parent) const noexcept {
	CMainFrame::InsertTreeViewControlItem(tree, parent, L"<Line>", this);
}

EoDbPrimitive* EoDbLine::Clone(OdDbBlockTableRecordPtr blockTableRecord) const {
	OdDbLinePtr Line = m_EntityObjectId.safeOpenObject()->clone();
	blockTableRecord->appendOdDbEntity(Line);
	return Create(Line);
}

void EoDbLine::CutAt(const OdGePoint3d& point, EoDbGroup* newGroup) {
	EoGeLineSeg3d LineSeg;
	if (m_LineSeg.CutAt(point, LineSeg) != 0) {
		OdDbLinePtr Line {m_EntityObjectId.safeOpenObject(OdDb::kForWrite)};
		Line->setStartPoint(m_LineSeg.startPoint());
		Line->setEndPoint(m_LineSeg.endPoint());
		OdDbBlockTableRecordPtr BlockTableRecord = m_EntityObjectId.database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
		OdDbLinePtr NewLine {m_EntityObjectId.safeOpenObject()->clone()};
		BlockTableRecord->appendOdDbEntity(NewLine);
		NewLine->setStartPoint(LineSeg.startPoint());
		NewLine->setEndPoint(LineSeg.endPoint());
		newGroup->AddTail(Create(NewLine));
	}
}

void EoDbLine::CutAt2Points(OdGePoint3d* points, EoDbGroupList* groupsOut, EoDbGroupList* groupsIn, OdDbDatabasePtr /*database*/) {
	EoDbLine* LineIn;
	double FirstPointParameter;
	double SecondPointParameter;
	m_LineSeg.ParametricRelationshipOf(points[0], FirstPointParameter);
	m_LineSeg.ParametricRelationshipOf(points[1], SecondPointParameter);
	if (FirstPointParameter <= DBL_EPSILON && SecondPointParameter >= 1. - DBL_EPSILON) { // Put entire line in trap
		LineIn = this;
	} else { // Something gets cut
		OdDbBlockTableRecordPtr BlockTableRecord {m_EntityObjectId.database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
		OdDbLinePtr Line {m_EntityObjectId.safeOpenObject()->clone()};
		BlockTableRecord->appendOdDbEntity(Line);
		if (FirstPointParameter > DBL_EPSILON && SecondPointParameter < 1. - DBL_EPSILON) { // Cut section out of middle
			Line->setStartPoint(points[1]);
			auto Group {new EoDbGroup};
			Group->AddTail(Create(Line));
			groupsOut->AddTail(Group);
			OdDbLinePtr Line {m_EntityObjectId.safeOpenObject()->clone()};
			Line->setStartPoint(points[0]);
			Line->setEndPoint(points[1]);
			BlockTableRecord->appendOdDbEntity(Line);
			LineIn = Create(Line);
			Line = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
			Line->setEndPoint(points[0]);
			m_LineSeg.SetEndPoint(points[0]);
		} else if (SecondPointParameter < 1. - DBL_EPSILON) { // Cut in two and place begin section in trap
			Line->setEndPoint(points[1]);
			LineIn = Create(Line);
			Line = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
			Line->setStartPoint(points[1]);
			m_LineSeg.SetStartPoint(points[1]);
		} else { // Cut in two and place end section in trap
			Line->setStartPoint(points[0]);
			LineIn = Create(Line);
			Line = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
			Line->setEndPoint(points[0]);
			m_LineSeg.SetEndPoint(points[0]);
		}
		auto Group {new EoDbGroup};
		Group->AddTail(this);
		groupsOut->AddTail(Group);
	}
	auto GroupIn {new EoDbGroup};
	GroupIn->AddTail(LineIn);
	groupsIn->AddTail(GroupIn);
}

void EoDbLine::Display(AeSysView* view, CDC* deviceContext) {
	const auto ColorIndex {LogicalColorIndex()};
	const auto LinetypeIndex {LogicalLinetypeIndex()};
	g_PrimitiveState.SetPen(view, deviceContext, ColorIndex, LinetypeIndex);
	polyline::BeginLineStrip();
	polyline::SetVertex(m_LineSeg.startPoint());
	polyline::SetVertex(m_LineSeg.endPoint());
	polyline::__End(view, deviceContext, LinetypeIndex);
}

void EoDbLine::FormatExtra(CString& extra) const {
	extra.Empty();
	extra += L"Color;" + FormatColorIndex() + L"\t";
	extra += L"Linetype;" + FormatLinetypeIndex() + L"\t";
	extra += L"Length;" + theApp.FormatLength(Length(), theApp.GetUnits()) + L"\t";
	extra += L"Z-Angle;" + AeSys::FormatAngle(m_LineSeg.AngleFromXAxis_xy());
}

void EoDbLine::FormatGeometry(CString& geometry) const {
	auto Point {m_LineSeg.startPoint()};
	CString PointString;
	PointString.Format(L"Start Point;%f;%f;%f\t", Point.x, Point.y, Point.z);
	geometry += PointString;
	Point = m_LineSeg.endPoint();
	PointString.Format(L"End Point;%f;%f;%f\t", Point.x, Point.y, Point.z);
	geometry += PointString;
}

void EoDbLine::GetAllPoints(OdGePoint3dArray& points) const {
	points.clear();
	points.append(m_LineSeg.startPoint());
	points.append(m_LineSeg.endPoint());
}

OdGePoint3d EoDbLine::GetCtrlPt() const {
	return m_LineSeg.midPoint();
}

void EoDbLine::GetExtents(AeSysView* /*view*/, OdGeExtents3d& extents) const {
	extents.addPoint(m_LineSeg.startPoint());
	extents.addPoint(m_LineSeg.endPoint());
}

OdGePoint3d EoDbLine::GoToNxtCtrlPt() const {
	if (ms_ControlPointIndex == 0) ms_ControlPointIndex = 1;
	else if (ms_ControlPointIndex == 1) {
		ms_ControlPointIndex = 0;
	} else { // Initial rock .. jump to point at lower left or down if vertical
		const auto StartPoint {m_LineSeg.startPoint()};
		const auto EndPoint {m_LineSeg.endPoint()};
		if (EndPoint.x > StartPoint.x) ms_ControlPointIndex = 0;
		else if (EndPoint.x < StartPoint.x) ms_ControlPointIndex = 1;
		else if (EndPoint.y > StartPoint.y) ms_ControlPointIndex = 0;
		else ms_ControlPointIndex = 1;
	}
	return ms_ControlPointIndex == 0 ? m_LineSeg.startPoint() : m_LineSeg.endPoint();
}

bool EoDbLine::IsEqualTo(EoDbPrimitive* primitive) const {
	auto Result {false};
	if (primitive->IsKindOf(RUNTIME_CLASS(EoDbLine))) {
		Result = m_LineSeg.isEqualTo(dynamic_cast<EoDbLine*>(primitive)->LineSeg());
	}
	return Result;
}

bool EoDbLine::IsInView(AeSysView* view) const {
	EoGePoint4d pt[] = {EoGePoint4d(m_LineSeg.startPoint(), 1.0), EoGePoint4d(m_LineSeg.endPoint(), 1.0)};
	view->ModelViewTransformPoints(2, &pt[0]);
	return EoGePoint4d::ClipLine(pt[0], pt[1]);
}

bool EoDbLine::IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	auto Point {EoGePoint4d(m_LineSeg.startPoint(), 1.0)};
	view->ModelViewTransformPoint(Point);
	if (point.DistanceToPointXY(Point) < ms_SelectApertureSize) { return true; }
	Point = EoGePoint4d(m_LineSeg.endPoint(), 1.0);
	view->ModelViewTransformPoint(Point);
	if (point.DistanceToPointXY(Point) < ms_SelectApertureSize) { return true; }
	return false;
}

int EoDbLine::IsWithinArea(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, OdGePoint3d* intersections) {
	int i;
	int iLoc[2];
	intersections[0] = m_LineSeg.startPoint();
	intersections[1] = m_LineSeg.endPoint();
	for (i = 0; i < 2; i++) {
		iLoc[i] = RelationshipToRectangleOf(intersections[i], lowerLeftCorner, upperRightCorner);
	}
	while (iLoc[0] != 0 || iLoc[1] != 0) {
		if ((iLoc[0] & iLoc[1]) != 0) return 0;
		i = iLoc[0] != 0 ? 0 : 1;
		if ((iLoc[i] & 1) != 0) { // Clip against top
			intersections[i].x = intersections[i].x + (intersections[1].x - intersections[0].x) * (upperRightCorner.y - intersections[i].y) / (intersections[1].y - intersections[0].y);
			intersections[i].y = upperRightCorner.y;
		} else if ((iLoc[i] & 2) != 0) { // Clip against bottom
			intersections[i].x = intersections[i].x + (intersections[1].x - intersections[0].x) * (lowerLeftCorner.y - intersections[i].y) / (intersections[1].y - intersections[0].y);
			intersections[i].y = lowerLeftCorner.y;
		} else if ((iLoc[i] & 4) != 0) { // Clip against right
			intersections[i].y = intersections[i].y + (intersections[1].y - intersections[0].y) * (upperRightCorner.x - intersections[i].x) / (intersections[1].x - intersections[0].x);
			intersections[i].x = upperRightCorner.x;
		} else if ((iLoc[i] & 8) != 0) { // Clip against left
			intersections[i].y = intersections[i].y + (intersections[1].y - intersections[0].y) * (lowerLeftCorner.x - intersections[i].x) / (intersections[1].x - intersections[0].x);
			intersections[i].x = lowerLeftCorner.x;
		}
		iLoc[i] = RelationshipToRectangleOf(intersections[i], lowerLeftCorner, upperRightCorner);
	}
	return 2;
}

OdGePoint3d EoDbLine::ProjPt_(const OdGePoint3d& point) const {
	return m_LineSeg.ProjPt(point);
}

double EoDbLine::ParametricRelationshipOf(const OdGePoint3d& point) const {
	double dRel;
	m_LineSeg.ParametricRelationshipOf(point, dRel);
	return dRel;
}

OdGePoint3d EoDbLine::SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	ms_ControlPointIndex = SIZE_T_MAX;
	OdGePoint3d ControlPoint;
	auto Aperture {ms_SelectApertureSize};
	for (unsigned ControlPointIndex = 0; ControlPointIndex < 2; ControlPointIndex++) {
		EoGePoint4d pt(ControlPointIndex == 0 ? m_LineSeg.startPoint() : m_LineSeg.endPoint(), 1.0);
		view->ModelViewTransformPoint(pt);
		const auto Distance {point.DistanceToPointXY(pt)};
		if (Distance < Aperture) {
			ms_ControlPointIndex = ControlPointIndex;
			ControlPoint = ControlPointIndex == 0 ? m_LineSeg.startPoint() : m_LineSeg.endPoint();
			Aperture = Distance;
		}
	}
	return ControlPoint;
}

bool EoDbLine::SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view, OdGePoint3dArray& intersections) {
	polyline::BeginLineStrip();
	polyline::SetVertex(m_LineSeg.startPoint());
	polyline::SetVertex(m_LineSeg.endPoint());
	return polyline::SelectUsingLineSeg(lineSeg, view, intersections);
}

bool EoDbLine::SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& projectedPoint) const {
	polyline::BeginLineStrip();
	polyline::SetVertex(m_LineSeg.startPoint());
	polyline::SetVertex(m_LineSeg.endPoint());
	return polyline::SelectUsingPoint(point, view, ms_RelationshipOfPoint, projectedPoint);
}

bool EoDbLine::SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const {
	polyline::BeginLineStrip();
	polyline::SetVertex(m_LineSeg.startPoint());
	polyline::SetVertex(m_LineSeg.endPoint());
	return polyline::SelectUsingRectangle(lowerLeftCorner, upperRightCorner, view);
}

void EoDbLine::SetEndPoint(const OdGePoint3d& endPoint) {
	OdDbLinePtr Line {m_EntityObjectId.safeOpenObject(OdDb::kForWrite)};
	Line->setEndPoint(endPoint);
	Line->downgradeOpen();
	m_LineSeg.SetEndPoint(endPoint);
}

void EoDbLine::SetStartPoint(const OdGePoint3d& startPoint) {
	OdDbLinePtr Line {m_EntityObjectId.safeOpenObject(OdDb::kForWrite)};
	Line->setStartPoint(startPoint);
	Line->downgradeOpen();
	m_LineSeg.SetStartPoint(startPoint);
}

void EoDbLine::Square(AeSysView* view) {
	const auto StartPoint {view->SnapPointToGrid(m_LineSeg.startPoint())};
	auto EndPoint {view->SnapPointToGrid(m_LineSeg.endPoint())};
	const auto MidPoint {EoGeLineSeg3d(StartPoint, EndPoint).midPoint()};
	const auto Length {OdGeVector3d(EndPoint - StartPoint).length()};
	EndPoint = view->SnapPointToAxis(MidPoint, EndPoint);
	SetStartPoint(ProjectToward(EndPoint, MidPoint, Length));
	SetEndPoint(EndPoint);
}

void EoDbLine::TransformBy(const EoGeMatrix3d& transformMatrix) {
	m_LineSeg.transformBy(transformMatrix);
}

void EoDbLine::TranslateUsingMask(const OdGeVector3d& translate, const unsigned long mask) {
	if ((mask & 1) == 1) { SetStartPoint(m_LineSeg.startPoint() + translate); }
	if ((mask & 2) == 2) { SetEndPoint(m_LineSeg.endPoint() + translate); }
}

bool EoDbLine::Write(EoDbFile& file) const {
	file.WriteUInt16(EoDb::kLinePrimitive);
	file.WriteInt16(m_ColorIndex);
	file.WriteInt16(m_LinetypeIndex);
	file.WritePoint3d(m_LineSeg.startPoint());
	file.WritePoint3d(m_LineSeg.endPoint());
	return true;
}

void EoDbLine::Write(CFile& file, unsigned char* buffer) const {
	buffer[3] = 1;
	*reinterpret_cast<unsigned short*>(& buffer[4]) = static_cast<unsigned short>(EoDb::kLinePrimitive);
	buffer[6] = static_cast<unsigned char>(m_ColorIndex == mc_ColorindexBylayer ? ms_LayerColorIndex : m_ColorIndex);
	buffer[7] = static_cast<unsigned char>(m_LinetypeIndex == mc_LinetypeBylayer ? ms_LayerLinetypeIndex : m_LinetypeIndex);
	if (buffer[7] >= 16) buffer[7] = 2;
	reinterpret_cast<EoVaxPoint3d*>(& buffer[8])->Convert(m_LineSeg.startPoint());
	reinterpret_cast<EoVaxPoint3d*>(& buffer[20])->Convert(m_LineSeg.endPoint());
	file.Write(buffer, 32);
}

// Static
EoDbLine* EoDbLine::Create(const OdDbLinePtr& line) {
	auto Line {new EoDbLine};
	Line->SetEntityObjectId(line->objectId());
	Line->m_ColorIndex = static_cast<short>(line->colorIndex());
	Line->m_LinetypeIndex = static_cast<short>(EoDbLinetypeTable::LegacyLinetypeIndex(line->linetype()));
	Line->m_LineSeg.SetStartPoint(line->startPoint());
	Line->m_LineSeg.SetEndPoint(line->endPoint());
	return Line;
}

OdDbLinePtr EoDbLine::Create(OdDbBlockTableRecordPtr blockTableRecord) {
	auto Line {OdDbLine::createObject()};
	Line->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(Line);
	Line->setColorIndex(static_cast<unsigned short>(g_PrimitiveState.ColorIndex()));
	const auto Linetype {LinetypeObjectFromIndex(g_PrimitiveState.LinetypeIndex())};
	Line->setLinetype(Linetype);
	return Line;
}

OdDbLinePtr EoDbLine::Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file) {
	const auto Database {blockTableRecord->database()};
	auto Line {OdDbLine::createObject()};
	Line->setDatabaseDefaults(Database);
	blockTableRecord->appendOdDbEntity(Line);
	Line->setColorIndex(static_cast<unsigned short>(file.ReadInt16()));
	const auto Linetype {LinetypeObjectFromIndex0(Database, file.ReadInt16())};
	Line->setLinetype(Linetype);
	Line->setStartPoint(file.ReadPoint3d());
	Line->setEndPoint(file.ReadPoint3d());
	return Line;
}

OdDbLinePtr EoDbLine::Create(OdDbBlockTableRecordPtr blockTableRecord, unsigned char* primitiveBuffer, const int versionNumber) {
	short ColorIndex;
	short LinetypeIndex;
	OdGePoint3d StartPoint;
	OdGePoint3d EndPoint;
	if (versionNumber == 1) {
		ColorIndex = static_cast<short>(primitiveBuffer[4] & 0x000f);
		LinetypeIndex = static_cast<short>((primitiveBuffer[4] & 0x00ff) >> 4);
		StartPoint = reinterpret_cast<EoVaxPoint3d*>(& primitiveBuffer[8])->Convert() * 1.e-3;
		EndPoint = reinterpret_cast<EoVaxPoint3d*>(& primitiveBuffer[20])->Convert() * 1.e-3;
	} else {
		ColorIndex = static_cast<short>(primitiveBuffer[6]);
		LinetypeIndex = static_cast<short>(primitiveBuffer[7]);
		StartPoint = reinterpret_cast<EoVaxPoint3d*>(& primitiveBuffer[8])->Convert();
		EndPoint = reinterpret_cast<EoVaxPoint3d*>(& primitiveBuffer[20])->Convert();
	}
	const auto Database {blockTableRecord->database()};
	auto Line {OdDbLine::createObject()};
	Line->setDatabaseDefaults(Database);
	blockTableRecord->appendOdDbEntity(Line);
	Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Line->setLinetype(LinetypeObjectFromIndex0(Database, LinetypeIndex));
	Line->setStartPoint(StartPoint);
	Line->setEndPoint(EndPoint);
	return Line;
}

OdDbLinePtr EoDbLine::Create(OdDbBlockTableRecordPtr blockTableRecord, const OdGePoint3d& startPoint, const OdGePoint3d& endPoint) {
	auto Line {OdDbLine::createObject()};
	Line->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(Line);
	Line->setStartPoint(startPoint);
	Line->setEndPoint(endPoint);
	return Line;
}
