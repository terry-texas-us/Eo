#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

EoDbLine::EoDbLine() noexcept {
}

EoDbLine::EoDbLine(const OdGePoint3d& startPoint, const OdGePoint3d& endPoint) {
	m_ColorIndex = pstate.ColorIndex();
	m_LinetypeIndex = pstate.LinetypeIndex();
	m_Line.set(startPoint, endPoint);
}

EoDbLine::EoDbLine(const EoDbLine& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;
	
	m_ColorIndex = other.m_ColorIndex;
	m_LinetypeIndex = other.m_LinetypeIndex;
	m_Line = other.m_Line;
}

EoDbLine::~EoDbLine() {
}

const EoDbLine& EoDbLine::operator=(const EoDbLine& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;
	
	m_ColorIndex = other.m_ColorIndex;
	m_LinetypeIndex = other.m_LinetypeIndex;
	m_Line = other.m_Line;
	return (*this);
}

void EoDbLine::AddReportToMessageList(const OdGePoint3d& point) const {
	double AngleInXYPlane = m_Line.AngleFromXAxis_xy();

	double Relationship;
	m_Line.ParametricRelationshipOf(point, Relationship);

	if (Relationship > .5) {
		AngleInXYPlane += PI;
	}
	AngleInXYPlane = fmod(AngleInXYPlane, TWOPI);

	const double Length = m_Line.length();

	CString Report(L"<Line>");
	Report += L" Color:" + FormatColorIndex();
	Report += L" Linetype:" + FormatLinetypeIndex();
	Report += L" [" + theApp.FormatLength(Length, theApp.GetUnits()) + L" @ " + theApp.FormatAngle(AngleInXYPlane) + L"]";
	theApp.AddStringToMessageList(Report);

	theApp.SetEngagedLength(Length);
	theApp.SetEngagedAngle(AngleInXYPlane);
}

void EoDbLine::AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept {
	CMainFrame::InsertTreeViewControlItem(tree, parent, L"<Line>", this);
}

void EoDbLine::AssociateWith(OdDbBlockTableRecordPtr& blockTableRecord) {
	OdDbLinePtr LineEntity = OdDbLine::createObject();
	blockTableRecord->appendOdDbEntity(LineEntity);
	LineEntity->setDatabaseDefaults();
	
	SetEntityObjectId(LineEntity->objectId());
	
	LineEntity->setColorIndex(m_ColorIndex);
	SetLinetypeIndex(m_LinetypeIndex);
	LineEntity->setStartPoint(m_Line.startPoint());
	LineEntity->setEndPoint(m_Line.endPoint());
}

EoDbPrimitive* EoDbLine::Clone(OdDbDatabasePtr& database) const {
    OdDbBlockTableRecordPtr BlockTableRecord = database->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    OdDbLinePtr Line = m_EntityObjectId.safeOpenObject()->clone();
    BlockTableRecord->appendOdDbEntity(Line);

    return EoDbLine::Create(Line);
}

void EoDbLine::CutAt(const OdGePoint3d& point, EoDbGroup* group, OdDbDatabasePtr& database) {
	EoGeLineSeg3d LineSeg;

	if (m_Line.CutAt(point, LineSeg) != 0) {
        OdDbLinePtr Line {m_EntityObjectId.safeOpenObject(OdDb::kForWrite)};
        Line->setStartPoint(m_Line.startPoint());
        Line->setEndPoint(m_Line.endPoint());

        OdDbBlockTableRecordPtr BlockTableRecord = m_EntityObjectId.database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

        OdDbLinePtr NewLine {m_EntityObjectId.safeOpenObject()->clone()};
        BlockTableRecord->appendOdDbEntity(NewLine);

        NewLine->setStartPoint(LineSeg.startPoint());
        NewLine->setEndPoint(LineSeg.endPoint());

        group->AddTail(EoDbLine::Create(NewLine));
	}
}

void EoDbLine::CutAt2Points(OdGePoint3d* points, EoDbGroupList* groupsOut, EoDbGroupList* groupsIn, OdDbDatabasePtr& database) {
	EoDbLine* LineIn;
	double FirstPointParameter;
	double SecondPointParameter;
	m_Line.ParametricRelationshipOf(points[0], FirstPointParameter);
	m_Line.ParametricRelationshipOf(points[1], SecondPointParameter);

	if (FirstPointParameter <= DBL_EPSILON && SecondPointParameter >= 1. - DBL_EPSILON) { // Put entire line in trap
        LineIn = this;
	}
	else { // Something gets cut
        OdDbBlockTableRecordPtr BlockTableRecord = m_EntityObjectId.database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

        OdDbLinePtr Line = m_EntityObjectId.safeOpenObject()->clone();
        BlockTableRecord->appendOdDbEntity(Line);

		if (FirstPointParameter > DBL_EPSILON && SecondPointParameter < 1. - DBL_EPSILON) { // Cut section out of middle
            Line->setStartPoint(points[1]);

            auto Group {new EoDbGroup};
			Group->AddTail(EoDbLine::Create(Line));
            groupsOut->AddTail(Group);

            OdDbLinePtr Line = m_EntityObjectId.safeOpenObject()->clone();
            Line->setStartPoint(points[0]);
            Line->setEndPoint(points[1]);
            BlockTableRecord->appendOdDbEntity(Line);
            LineIn = EoDbLine::Create(Line);
            
            Line = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
            Line->setEndPoint(points[0]);
            m_Line.SetEndPoint(points[0]);
        }
		else if (SecondPointParameter < 1. - DBL_EPSILON) { // Cut in two and place begin section in trap
            Line->setEndPoint(points[1]);
            LineIn = EoDbLine::Create(Line);
            Line = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
            Line->setStartPoint(points[1]);
            m_Line.SetStartPoint(points[1]);
		}
		else { // Cut in two and place end section in trap
            Line->setStartPoint(points[0]);
            LineIn = EoDbLine::Create(Line);
            Line = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
            Line->setEndPoint(points[0]);
            m_Line.SetEndPoint(points[0]);
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
	const OdInt16 ColorIndex = LogicalColorIndex();
	const OdInt16 LinetypeIndex = LogicalLinetypeIndex();

	pstate.SetPen(view, deviceContext, ColorIndex, LinetypeIndex);

	polyline::BeginLineStrip();
	polyline::SetVertex(m_Line.startPoint());
	polyline::SetVertex(m_Line.endPoint());
	polyline::__End(view, deviceContext, LinetypeIndex);
}

OdGePoint3d EoDbLine::EndPoint() const {
	return m_Line.endPoint();
}

void EoDbLine::FormatExtra(CString& extra) const {
	extra.Empty();
	extra += L"Color;" + FormatColorIndex() + L"\t";
	extra += L"Linetype;" + FormatLinetypeIndex() + L"\t";
	extra += L"Length;" + theApp.FormatLength(Length(), theApp.GetUnits()) + L"\t";
	extra += L"Z-Angle;" + theApp.FormatAngle(m_Line.AngleFromXAxis_xy());
}

void EoDbLine::FormatGeometry(CString& geometry) const {
	OdGePoint3d Point;
	Point = m_Line.startPoint();
	CString PointString;
	PointString.Format(L"Start Point;%f;%f;%f\t", Point.x, Point.y, Point.z);
	geometry += PointString;
	Point = m_Line.endPoint();
	PointString.Format(L"End Point;%f;%f;%f\t", Point.x, Point.y, Point.z);
	geometry += PointString;
}

void EoDbLine::GetAllPoints(OdGePoint3dArray& points) const {
	points.clear();
	points.append(m_Line.startPoint());
	points.append(m_Line.endPoint());
}

OdGePoint3d EoDbLine::GetCtrlPt() const {
	return m_Line.midPoint();
}

void EoDbLine::GetExtents(AeSysView* view, OdGeExtents3d& extents) const {
	extents.addPoint(m_Line.startPoint());
	extents.addPoint(m_Line.endPoint());
}

OdGePoint3d EoDbLine::GoToNxtCtrlPt() const {
	if (sm_ControlPointIndex == 0)
		sm_ControlPointIndex = 1;
	else if (sm_ControlPointIndex == 1)
		sm_ControlPointIndex = 0;
	else { // Initial rock .. jump to point at lower left or down if vertical
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

bool EoDbLine::IsEqualTo(EoDbPrimitive* primitive)  const {
	bool IsEqualTo = primitive->Is(kLinePrimitive);
	if (IsEqualTo) {
		IsEqualTo = m_Line.isEqualTo(static_cast<EoDbLine*>(primitive)->Line());
	}
	return IsEqualTo;
}

bool EoDbLine::IsInView(AeSysView* view) const {
	EoGePoint4d pt[] = {EoGePoint4d(m_Line.startPoint(), 1.), EoGePoint4d(m_Line.endPoint(), 1.)};
	view->ModelViewTransformPoints(2, &pt[0]);

	return (EoGePoint4d::ClipLine(pt[0], pt[1]));
}

bool EoDbLine::IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	EoGePoint4d pt;

	pt = EoGePoint4d(m_Line.startPoint(), 1.);
	view->ModelViewTransformPoint(pt);

	if (point.DistanceToPointXY(pt) < sm_SelectApertureSize)
		return true;

	pt = EoGePoint4d(m_Line.endPoint(), 1.);
	view->ModelViewTransformPoint(pt);

	if (point.DistanceToPointXY(pt) < sm_SelectApertureSize)
		return true;

	return false;
}

void EoDbLine::GetLine(EoGeLineSeg3d& line) const {
	line = m_Line;
}

void EoDbLine::GetPoints(OdGePoint3d& startPoint, OdGePoint3d& endPoint) {
	startPoint = m_Line.startPoint();
	endPoint = m_Line.endPoint();
}

int EoDbLine::IsWithinArea(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, OdGePoint3d* intersections) noexcept {
	int i;
	int iLoc[2];

	GetPoints(intersections[0], intersections[1]);

	for (i = 0; i < 2; i++) {
		iLoc[i] = RelationshipToRectangleOf(intersections[i], lowerLeftCorner, upperRightCorner);
	}
	while (iLoc[0] != 0 || iLoc[1] != 0) {
		if ((iLoc[0] & iLoc[1]) != 0)
			return 0;

		i = (iLoc[0] != 0) ? 0 : 1;
		if ((iLoc[i] & 1) != 0) { // Clip against top
			intersections[i].x = intersections[i].x + (intersections[1].x - intersections[0].x) * (upperRightCorner.y - intersections[i].y) / (intersections[1].y - intersections[0].y);
			intersections[i].y = upperRightCorner.y;
		}
		else if ((iLoc[i] & 2) != 0) { // Clip against bottom
			intersections[i].x = intersections[i].x + (intersections[1].x - intersections[0].x) * (lowerLeftCorner.y - intersections[i].y) / (intersections[1].y - intersections[0].y);
			intersections[i].y = lowerLeftCorner.y;
		}
		else if ((iLoc[i] & 4) != 0) { // Clip against right
			intersections[i].y = intersections[i].y + (intersections[1].y - intersections[0].y) * (upperRightCorner.x - intersections[i].x) / (intersections[1].x - intersections[0].x);
			intersections[i].x = upperRightCorner.x;
		}
		else if ((iLoc[i] & 8) != 0) { // Clip against left
			intersections[i].y = intersections[i].y + (intersections[1].y - intersections[0].y) * (lowerLeftCorner.x - intersections[i].x) / (intersections[1].x - intersections[0].x);
			intersections[i].x = lowerLeftCorner.x;
		}
		iLoc[i] = RelationshipToRectangleOf(intersections[i], lowerLeftCorner, upperRightCorner);
	}
	return (2);
}

double EoDbLine::Length() const {
	return (m_Line.length());
}

EoGeLineSeg3d EoDbLine::Line() const {
	return m_Line;
}

OdGePoint3d EoDbLine::ProjPt_(const OdGePoint3d& point) const {
	return (m_Line.ProjPt(point));
}

double EoDbLine::ParametricRelationshipOf(const OdGePoint3d& point) const {
	double dRel;
	m_Line.ParametricRelationshipOf(point, dRel);
	return dRel;
}

OdGePoint3d EoDbLine::SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	sm_ControlPointIndex = SIZE_T_MAX;
	OdGePoint3d ControlPoint;

	double Aperture = sm_SelectApertureSize;

	for (OdUInt16 ControlPointIndex = 0; ControlPointIndex < 2; ControlPointIndex++) {
		EoGePoint4d pt(ControlPointIndex == 0 ? m_Line.startPoint() : m_Line.endPoint(), 1.);

		view->ModelViewTransformPoint(pt);

		const double Distance = point.DistanceToPointXY(pt);

		if (Distance < Aperture) {
			sm_ControlPointIndex = ControlPointIndex;
			ControlPoint = ControlPointIndex == 0 ?  m_Line.startPoint() : m_Line.endPoint();
			Aperture = Distance;
		}
	}
	return ControlPoint;
}

bool EoDbLine::SelectBy(const EoGeLineSeg3d& line, AeSysView* view, OdGePoint3dArray& intersections) {
	polyline::BeginLineStrip();
	polyline::SetVertex(m_Line.startPoint());
	polyline::SetVertex(m_Line.endPoint());
	return polyline::SelectBy(line, view, intersections);
}

bool EoDbLine::SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& ptProj) const {
	polyline::BeginLineStrip();
	polyline::SetVertex(m_Line.startPoint());
	polyline::SetVertex(m_Line.endPoint());
	return polyline::SelectBy(point, view, sm_RelationshipOfPoint, ptProj);
}

bool EoDbLine::SelectBy(const OdGePoint3d& pt1, const OdGePoint3d& pt2, AeSysView* view) const {
	polyline::BeginLineStrip();
	polyline::SetVertex(m_Line.startPoint());
	polyline::SetVertex(m_Line.endPoint());
	return polyline::SelectBy(pt1, pt2, view);
}

EoDbLine& EoDbLine::SetTo(const OdGePoint3d& startPoint, const OdGePoint3d& endPoint) {
	m_Line.set(startPoint, endPoint);
	if (!m_EntityObjectId.isNull()) {
		OdDbLinePtr Line = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		Line->setStartPoint(startPoint);
		Line->setEndPoint(endPoint);
	}
	return (*this);
}

void EoDbLine::SetEndPoint(const OdGePoint3d& endPoint) {
	if (!m_EntityObjectId.isNull()) {
		OdDbLinePtr Line = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		Line->setEndPoint(endPoint);
	}
	m_Line.SetEndPoint(endPoint);
}

void EoDbLine::SetStartPoint(const OdGePoint3d& startPoint) {
	if (!m_EntityObjectId.isNull()) {
		OdDbLinePtr Line = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		Line->setStartPoint(startPoint);
	}
	m_Line.SetStartPoint(startPoint);
}

void EoDbLine::Square(AeSysView* view) {
	const OdGePoint3d StartPoint = view->SnapPointToGrid(m_Line.startPoint());
	OdGePoint3d EndPoint = view->SnapPointToGrid(m_Line.endPoint());

	const OdGePoint3d MidPoint = EoGeLineSeg3d(StartPoint, EndPoint).midPoint();
	const double Length = OdGeVector3d(EndPoint - StartPoint).length();
	EndPoint = view->SnapPointToAxis(MidPoint, EndPoint);
	SetStartPoint(ProjectToward(EndPoint, MidPoint, Length));
	SetEndPoint(EndPoint);
}

OdGePoint3d EoDbLine::StartPoint() const {
	return m_Line.startPoint();
}

void EoDbLine::TransformBy(const EoGeMatrix3d& transformMatrix) {
	m_Line.transformBy(transformMatrix);
}

void EoDbLine::TranslateUsingMask(const OdGeVector3d& translate, const DWORD mask) {
	if ((mask & 1) == 1) {
		SetStartPoint(m_Line.startPoint() + translate);
	}
	if ((mask & 2) == 2) {
		SetEndPoint(m_Line.endPoint() + translate);
	}
}

bool EoDbLine::Write(EoDbFile& file) const {
	file.WriteUInt16(kLinePrimitive);
	file.WriteInt16(m_ColorIndex);
	file.WriteInt16(m_LinetypeIndex);
	file.WritePoint3d(m_Line.startPoint());
	file.WritePoint3d(m_Line.endPoint());
	return true;
}

void EoDbLine::Write(CFile& file, OdUInt8* buffer) const {
	buffer[3] = 1;
	*((OdUInt16*) &buffer[4]) = OdUInt16(kLinePrimitive);
	buffer[6] = OdInt8(m_ColorIndex == COLORINDEX_BYLAYER ? sm_LayerColorIndex : m_ColorIndex);
	buffer[7] = OdInt8(m_LinetypeIndex == LINETYPE_BYLAYER ? sm_LayerLinetypeIndex : m_LinetypeIndex);
	if (buffer[7] >= 16) buffer[7] = 2;

	((EoVaxPoint3d*) &buffer[8])->Convert(m_Line.startPoint());
	((EoVaxPoint3d*) &buffer[20])->Convert(m_Line.endPoint());

	file.Write(buffer, 32);
}

// Static

EoDbLine* EoDbLine::ConstructFrom(OdUInt8* primitiveBuffer, int versionNumber) {
	EoDbLine* LinePrimitive = new EoDbLine();
	OdInt16 ColorIndex;
	OdInt16 LinetypeIndex;
	OdGePoint3d StartPoint;
	OdGePoint3d EndPoint;
	if (versionNumber == 1) {
		ColorIndex = OdInt16(primitiveBuffer[4] & 0x000f);
		LinetypeIndex = OdInt16((primitiveBuffer[4] & 0x00ff) >> 4);
		StartPoint = ((EoVaxPoint3d*) &primitiveBuffer[8])->Convert() * 1.e-3;
		EndPoint = ((EoVaxPoint3d*) &primitiveBuffer[20])->Convert() * 1.e-3;
	}
	else {
		ColorIndex = OdInt16(primitiveBuffer[6]);
		LinetypeIndex = OdInt16(primitiveBuffer[7]);
		StartPoint = ((EoVaxPoint3d*) &primitiveBuffer[8])->Convert();
		EndPoint = ((EoVaxPoint3d*) &primitiveBuffer[20])->Convert();
	}
	LinePrimitive->SetColorIndex(ColorIndex);
	LinePrimitive->SetLinetypeIndex(LinetypeIndex);
	LinePrimitive->SetStartPoint(StartPoint);
	LinePrimitive->SetEndPoint(EndPoint);
	return (LinePrimitive);
}

EoDbLine* EoDbLine::Create(OdDbLinePtr line) {
    auto Line {new EoDbLine()};
    Line->SetEntityObjectId(line->objectId());
    Line->SetColorIndex_(line->colorIndex());
    Line->SetLinetypeIndex_(EoDbLinetypeTable::LegacyLinetypeIndex(line->linetype()));

    Line->SetStartPoint_(line->startPoint());
    Line->SetEndPoint_(line->endPoint());

    return Line;
}

EoDbLine* EoDbLine::Create0(OdDbBlockTableRecordPtr blockTableRecord) {
    auto Line {new EoDbLine()};
    Line->SetColorIndex(pstate.ColorIndex());
    Line->SetLinetypeIndex(pstate.LinetypeIndex());
    Line->AssociateWith(blockTableRecord);

    return Line;
}

EoDbLine* EoDbLine::Create2(const OdGePoint3d& startPoint, const OdGePoint3d& endPoint) {
	OdDbDatabasePtr Database = AeSysDoc::GetDoc()->m_DatabasePtr;
    OdDbBlockTableRecordPtr BlockTableRecord = Database->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
    auto Line {Create0(BlockTableRecord)};
	Line->SetStartPoint(startPoint);
	Line->SetEndPoint(endPoint);

	return Line;
}

OdDbLinePtr EoDbLine::Create(OdDbBlockTableRecordPtr blockTableRecord) {
    auto Line {OdDbLine::createObject()};
    Line->setDatabaseDefaults(blockTableRecord->database());

    blockTableRecord->appendOdDbEntity(Line);
    Line->setColorIndex(pstate.ColorIndex());

    const auto Linetype = EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex());

    Line->setLinetype(Linetype);

    return Line;
}

OdDbLinePtr EoDbLine::Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file) {
    auto Line {OdDbLine::createObject()};
    Line->setDatabaseDefaults(blockTableRecord->database());

    blockTableRecord->appendOdDbEntity(Line);

    Line->setColorIndex(file.ReadInt16());
    
    const auto Linetype = EoDbPrimitive::LinetypeObjectFromIndex(file.ReadInt16());
    
    Line->setLinetype(Linetype);
    
    Line->setStartPoint(file.ReadPoint3d());
    Line->setEndPoint(file.ReadPoint3d());
    
    return (Line);
}
