#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "PrimState.h"
#include "EoGePolyline.h"
#include "EoDbFile.h"
#include "EoDbPolyline.h"
IMPLEMENT_DYNAMIC(EoDbPolyline, EoDbPrimitive)

unsigned EoDbPolyline::ms_EdgeToEvaluate = 0;
unsigned EoDbPolyline::ms_Edge = 0;
unsigned EoDbPolyline::ms_PivotVertex = 0;

EoDbPolyline::EoDbPolyline() {
	m_Vertices.clear();
	m_StartWidths.clear();
	m_EndWidths.clear();
	m_Bulges.clear();
}

EoDbPolyline::EoDbPolyline(const EoDbPolyline& other) {
	m_Flags = other.m_Flags;
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;
	m_ColorIndex = other.m_ColorIndex;
	m_LinetypeIndex = other.m_LinetypeIndex;
	m_ConstantWidth = other.m_ConstantWidth;
	m_Elevation = other.m_Elevation;
	m_Thickness = other.m_Thickness;
	m_Vertices.append(other.m_Vertices);
	m_StartWidths.append(other.m_StartWidths);
	m_EndWidths.append(other.m_EndWidths);
	m_Bulges.append(other.m_Bulges);
	m_Normal = other.m_Normal;
}

EoDbPolyline& EoDbPolyline::operator=(const EoDbPolyline& other) {
	m_Flags = other.m_Flags;
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;
	m_ColorIndex = other.m_ColorIndex;
	m_LinetypeIndex = other.m_LinetypeIndex;
	m_ConstantWidth = other.m_ConstantWidth;
	m_Elevation = other.m_Elevation;
	m_Thickness = other.m_Thickness;
	m_Vertices.append(other.m_Vertices);
	m_StartWidths.append(other.m_StartWidths);
	m_EndWidths.append(other.m_EndWidths);
	m_Bulges.append(other.m_Bulges);
	m_Normal = other.m_Normal;
	return *this;
}

void EoDbPolyline::AddReportToMessageList(const OdGePoint3d& point) const {
	const auto NumberOfVertices = m_Vertices.size();
	if (ms_Edge > 0 && ms_Edge <= NumberOfVertices) {
		OdGePoint3d ptBeg;
		OdGePoint3d ptEnd;
		GetPointAt(ms_Edge - 1, ptBeg);
		GetPointAt(ms_Edge % NumberOfVertices, ptEnd);
		if (ms_PivotVertex < NumberOfVertices) {
			GetPointAt(ms_PivotVertex, ptBeg);
			GetPointAt(SwingVertex(), ptEnd);
		}
		double AngleInXyPlane;
		const auto EdgeLength {OdGeVector3d(ptEnd - ptBeg).length()};
		if (OdGeVector3d(ptBeg - point).length() > EdgeLength * 0.5) {
			AngleInXyPlane = EoGeLineSeg3d(ptEnd, ptBeg).AngleFromXAxis_xy();
		} else {
			AngleInXyPlane = EoGeLineSeg3d(ptBeg, ptEnd).AngleFromXAxis_xy();
		}
		CString Report(L"<Polyline-Edge>");
		Report += L" Color:" + FormatColorIndex();
		Report += L" Linetype:" + FormatLinetypeIndex();
		Report += L" [" + theApp.FormatLength(EdgeLength, theApp.GetUnits()) + L" @ " + AeSys::FormatAngle(AngleInXyPlane) + L"]";
		AeSys::AddStringToMessageList(Report);
		theApp.SetEngagedLength(EdgeLength);
		theApp.SetEngagedAngle(AngleInXyPlane);
	}
}

void EoDbPolyline::AddToTreeViewControl(const HWND tree, const HTREEITEM parent) const noexcept {
	CMainFrame::InsertTreeViewControlItem(tree, parent, L"<Polyline>", this);
}

EoDbPrimitive* EoDbPolyline::Clone(OdDbBlockTableRecordPtr blockTableRecord) const {
	OdDbPolylinePtr Polyline = m_EntityObjectId.safeOpenObject()->clone();
	blockTableRecord->appendOdDbEntity(Polyline);
	return Create(Polyline);
}

void EoDbPolyline::Display(AeSysView* view, CDC* deviceContext) {
	const auto ColorIndex {LogicalColorIndex()};
	const auto LinetypeIndex {LogicalLinetypeIndex()};
	g_PrimitiveState.SetPen(view, deviceContext, ColorIndex, LinetypeIndex);
	if (IsClosed()) polyline::BeginLineLoop();
	else polyline::BeginLineStrip();
	const auto Origin {OdGePoint3d::kOrigin + m_Normal * m_Elevation};
	const auto XAxis {ComputeArbitraryAxis(m_Normal)};
	const auto YAxis {m_Normal.crossProduct(XAxis)};
	const OdGePlane Plane(Origin, XAxis, YAxis);
	OdGePoint3d Point;
	for (auto Vertex : m_Vertices) {
		Point.set(Plane, Vertex);
		polyline::SetVertex(Point);
	}
	polyline::__End(view, deviceContext, LinetypeIndex);
}

void EoDbPolyline::FormatExtra(CString& extra) const {
	extra.Empty();
	extra += L"Color;" + FormatColorIndex() + L"\t";
	extra += L"Linetype;" + FormatLinetypeIndex() + L"\t";
	CString NumberOfVertices;
	NumberOfVertices.Format(L"Number of Vertices;%d", m_Vertices.size());
	extra += NumberOfVertices;
}

void EoDbPolyline::FormatGeometry(CString& geometry) const {
	CString PointString;
	for (unsigned VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		OdGePoint3d Point;
		GetPointAt(VertexIndex, Point);
		PointString.Format(L"Point;%f;%f;%f\t", Point.x, Point.y, Point.z);
		geometry += PointString;
	}
}

void EoDbPolyline::GetAllPoints(OdGePoint3dArray& points) const {
	points.clear();
	for (unsigned VertexIndex = 0; VertexIndex < points.size(); VertexIndex++) {
		GetPointAt(VertexIndex, points[VertexIndex]);
	}
}

OdGePoint3d EoDbPolyline::GetCtrlPt() const {
	OdGePoint3d StartPoint;
	OdGePoint3d EndPoint;
	const auto NumberOfVertices = m_Vertices.size();
	GetPointAt(ms_Edge - 1, StartPoint);
	GetPointAt(ms_Edge % NumberOfVertices, EndPoint);
	return EoGeLineSeg3d(StartPoint, EndPoint).midPoint();
}

void EoDbPolyline::GetExtents(AeSysView* /*view*/, OdGeExtents3d& extents) const {
	OdGePoint3d Point;
	for (unsigned VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		GetPointAt(VertexIndex, Point);
		extents.addPoint(Point);
	}
}

/// <summary> This function sets point to the 3D location of the vertex index in World Coordinates.</summary>
void EoDbPolyline::GetPointAt(const unsigned vertexIndex, OdGePoint3d& point) const {
	const auto Origin {OdGePoint3d::kOrigin + m_Normal * m_Elevation};
	const auto XAxis {ComputeArbitraryAxis(m_Normal)};
	const auto YAxis {m_Normal.crossProduct(XAxis)};
	const OdGePlane Plane(Origin, XAxis, YAxis);
	point.set(Plane, m_Vertices[vertexIndex]);
}

OdGePoint3d EoDbPolyline::GoToNxtCtrlPt() const {
	const auto NumberOfVertices = m_Vertices.size();
	if (ms_PivotVertex >= NumberOfVertices) { // have not yet rocked to a vertex
		const auto StartVertexIndex {ms_Edge - 1};
		const auto EndVertexIndex {ms_Edge % NumberOfVertices};
		OdGePoint3d StartPoint;
		GetPointAt(StartVertexIndex, StartPoint);
		OdGePoint3d EndPoint;
		GetPointAt(EndVertexIndex, EndPoint);
		if (EndPoint.x > StartPoint.x) {
			ms_PivotVertex = StartVertexIndex;
		} else if (EndPoint.x < StartPoint.x) {
			ms_PivotVertex = EndVertexIndex;
		} else if (EndPoint.y > StartPoint.y) {
			ms_PivotVertex = StartVertexIndex;
		} else {
			ms_PivotVertex = EndVertexIndex;
		}
	} else if (ms_PivotVertex == 0) {
		if (ms_Edge == 1) {
			ms_PivotVertex = 1;
		} else {
			ms_PivotVertex = NumberOfVertices - 1;
		}
	} else if (ms_PivotVertex == NumberOfVertices - 1) {
		if (ms_Edge == NumberOfVertices) {
			ms_PivotVertex = 0;
		} else {
			ms_PivotVertex--;
		}
	} else {
		if (ms_Edge == ms_PivotVertex) {
			ms_PivotVertex--;
		} else {
			ms_PivotVertex++;
		}
	}
	OdGePoint3d PivotPoint;
	GetPointAt(ms_PivotVertex, PivotPoint);
	return PivotPoint;
}

bool EoDbPolyline::IsClosed() const noexcept {
	return m_Flags != 0;
}

bool EoDbPolyline::IsInView(AeSysView* view) const {
	OdGePoint3d Point;
	EoGePoint4d pt[2];
	GetPointAt(0, Point);
	pt[0] = EoGePoint4d(Point, 1.0);
	view->ModelViewTransformPoint(pt[0]);
	for (unsigned VertexIndex = 1; VertexIndex < m_Vertices.size(); VertexIndex++) {
		GetPointAt(VertexIndex, Point);
		pt[1] = EoGePoint4d(Point, 1.0);
		view->ModelViewTransformPoint(pt[1]);
		if (EoGePoint4d::ClipLine(pt[0], pt[1])) { return true; }
		pt[0] = pt[1];
	}
	return false;
}

bool EoDbPolyline::IsPointOnControlPoint(AeSysView* /*view*/, const EoGePoint4d& /*point*/) const noexcept {
	// <tas="Polyline: need to implement IsPointOnControlPoint"</tas>
	return false;
}

bool EoDbPolyline::PivotOnGripPoint(AeSysView* view, const EoGePoint4d& point) noexcept {
	const auto NumberOfVertices {m_Vertices.size()};
	if (ms_PivotVertex >= NumberOfVertices) { return false; } // Not engaged at a vertex
	OdGePoint3d Point;
	GetPointAt(ms_PivotVertex, Point);
	EoGePoint4d ptCtrl(Point, 1.0);
	view->ModelViewTransformPoint(ptCtrl);
	if (ptCtrl.DistanceToPointXY(point) >= sm_SelectApertureSize) { return false; } // Not on proper vertex
	if (ms_PivotVertex == 0) {
		ms_Edge = ms_Edge == 1 ? NumberOfVertices : 1;
	} else if (ms_PivotVertex == NumberOfVertices - 1) {
		ms_Edge = ms_Edge == NumberOfVertices ? ms_Edge - 1 : NumberOfVertices;
	} else if (ms_PivotVertex == ms_Edge) {
		ms_Edge++;
	} else {
		ms_Edge--;
	}
	return true;
}

OdGePoint3d EoDbPolyline::SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	sm_ControlPointIndex = SIZE_T_MAX;
	auto Aperture {sm_SelectApertureSize};
	ms_PivotVertex = m_Vertices.size();
	for (unsigned VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		OdGePoint3d Point;
		GetPointAt(VertexIndex, Point);
		EoGePoint4d pt(Point, 1.0);
		view->ModelViewTransformPoint(pt);
		const auto dDis {point.DistanceToPointXY(pt)};
		if (dDis < Aperture) {
			sm_ControlPointIndex = VertexIndex;
			Aperture = dDis;
			ms_Edge = VertexIndex + 1;
			ms_PivotVertex = VertexIndex;
		}
	}
	auto ControlPoint {OdGePoint3d::kOrigin};
	if (sm_ControlPointIndex != SIZE_T_MAX) {
		GetPointAt(sm_ControlPointIndex, ControlPoint);
	}
	return ControlPoint;
}

bool EoDbPolyline::SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& projectedPoint) const {
	const auto NumberOfVertices = m_Vertices.size();
	if (ms_EdgeToEvaluate > 0 && ms_EdgeToEvaluate <= NumberOfVertices) { // Evaluate specified edge of polyline
		OdGePoint3d StartPoint;
		GetPointAt(ms_EdgeToEvaluate - 1, StartPoint);
		OdGePoint3d EndPoint;
		GetPointAt(ms_EdgeToEvaluate % NumberOfVertices, EndPoint);
		EoGePoint4d ptBeg(StartPoint, 1.0);
		EoGePoint4d ptEnd(EndPoint, 1.0);
		view->ModelViewTransformPoint(ptBeg);
		view->ModelViewTransformPoint(ptEnd);
		const EoGeLineSeg3d LineSegment(ptBeg.Convert3d(), ptEnd.Convert3d());
		if (LineSegment.IsSelectedBy_xy(point.Convert3d(), view->SelectApertureSize(), projectedPoint, sm_RelationshipOfPoint)) {
			projectedPoint.z = ptBeg.z + sm_RelationshipOfPoint * (ptEnd.z - ptBeg.z);
			return true;
		}
	} else { // Evaluate entire polyline
		auto NumberOfEdges = NumberOfVertices;
		if (!IsClosed()) { NumberOfEdges--; }
		OdGePoint3d StartPoint;
		GetPointAt(0, StartPoint);
		EoGePoint4d ptBeg(StartPoint, 1.0);
		view->ModelViewTransformPoint(ptBeg);
		for (unsigned VertexIndex = 1; VertexIndex <= NumberOfEdges; VertexIndex++) {
			OdGePoint3d EndPoint;
			GetPointAt(VertexIndex % NumberOfVertices, EndPoint);
			EoGePoint4d ptEnd(EndPoint, 1.0);
			view->ModelViewTransformPoint(ptEnd);
			EoGeLineSeg3d LineSegment(ptBeg.Convert3d(), ptEnd.Convert3d());
			if (LineSegment.IsSelectedBy_xy(point.Convert3d(), view->SelectApertureSize(), projectedPoint, sm_RelationshipOfPoint)) {
				projectedPoint.z = ptBeg.z + sm_RelationshipOfPoint * (ptEnd.z - ptBeg.z);
				ms_Edge = VertexIndex;
				ms_PivotVertex = NumberOfVertices;
				return true;
			}
			ptBeg = ptEnd;
		}
	}
	return false;
}

bool EoDbPolyline::SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const {
	OdGePoint3dArray Points;
	for (unsigned VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		OdGePoint3d Point;
		GetPointAt(VertexIndex, Point);
		Points.append(Point);
	}
	return polyline::SelectUsingRectangle(view, lowerLeftCorner, upperRightCorner, Points);
}

void EoDbPolyline::SetClosed(const bool closed) noexcept {
	if (closed) {
		m_Flags |= ms_Closed;
	} else {
		m_Flags &= ~ms_Closed;
	}
}

void EoDbPolyline::AppendVertex(const OdGePoint2d& vertex, const double bulge, const double startWidth, const double endWidth) {
	const auto VertexIndex {m_Vertices.append(vertex)};
	m_Bulges.append(bulge);
	m_StartWidths.append(startWidth);
	m_EndWidths.append(endWidth);
}

bool EoDbPolyline::SelectUsingLineSeg(const EoGeLineSeg3d& /*lineSeg*/, AeSysView* /*view*/, OdGePoint3dArray& /*intersections*/) {
	const CRuntimeClass* PrimitiveClass = GetRuntimeClass();
	theApp.AddStringToMessageList(L"Selection by line segment not implemented for <%s>\n", CString(PrimitiveClass->m_lpszClassName));
	return false;
}

void EoDbPolyline::SetConstantWidth(const double constantWidth) noexcept {
	m_ConstantWidth = constantWidth;
}

void EoDbPolyline::SetNormal(const OdGeVector3d& normal) {
	m_Normal = normal.isZeroLength() ? OdGeVector3d::kZAxis : normal;
}

void EoDbPolyline::TransformBy(const EoGeMatrix3d& /*transformMatrix*/) {
	// <tas="TransformBy broken. Need to go to world and back?"</tas>
	for (unsigned VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		//m_Vertices[VertexIndex].transformBy(transformMatrix);
	}
}

void EoDbPolyline::TranslateUsingMask(const OdGeVector3d& /*translate*/, unsigned long /*mask*/) {
	// <tas="TranslateUsingMask broken. Need to go to world and back? This type of operation could get polyline vertex off plane"</tas>
	for (unsigned VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		//	if (((mask >> VertexIndex) & 1UL) == 1)
		//		m_Vertices[VertexIndex] += translate;
	}
}

bool EoDbPolyline::Write(EoDbFile& file) const {
	file.WriteUInt16(EoDb::kPolylinePrimitive);
	file.WriteInt16(m_ColorIndex);
	file.WriteInt16(m_LinetypeIndex);
	file.WriteUInt16(m_Flags);
	file.WriteDouble(m_ConstantWidth);
	file.WriteDouble(m_Elevation);
	file.WriteDouble(m_Thickness);
	file.WriteVector3d(m_Normal);
	file.WriteUInt16(static_cast<unsigned short>(m_Vertices.size()));
	for (unsigned VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		file.WritePoint2d(m_Vertices[VertexIndex]);
		file.WriteDouble(m_StartWidths[VertexIndex]);
		file.WriteDouble(m_EndWidths[VertexIndex]);
		file.WriteDouble(m_Bulges[VertexIndex]);
	}
	return true;
}

/// <remarks> Job (.jb1) files did not have a polyline primitive</remarks>
void EoDbPolyline::Write(CFile& /*file*/, unsigned char* /*buffer*/) const noexcept {
}

unsigned EoDbPolyline::SwingVertex() const {
	const auto NumberOfVertices = m_Vertices.size();
	unsigned SwingVertex;
	if (ms_PivotVertex == 0) {
		SwingVertex = ms_Edge == 1 ? 1 : NumberOfVertices - 1;
	} else if (ms_PivotVertex == NumberOfVertices - 1) {
		SwingVertex = ms_Edge == NumberOfVertices ? 0 : ms_PivotVertex - 1;
	} else {
		SwingVertex = ms_Edge == ms_PivotVertex ? ms_PivotVertex - 1 : ms_PivotVertex + 1;
	}
	return SwingVertex;
}

unsigned EoDbPolyline::Edge() noexcept {
	return ms_Edge;
}

void EoDbPolyline::SetEdgeToEvaluate(const unsigned edgeToEvaluate) noexcept {
	ms_EdgeToEvaluate = edgeToEvaluate;
}

OdDbPolylinePtr EoDbPolyline::Create(OdDbBlockTableRecordPtr blockTableRecord) {
	auto Polyline {OdDbPolyline::createObject()};
	Polyline->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(Polyline);
	Polyline->setColorIndex(static_cast<unsigned short>(g_PrimitiveState.ColorIndex()));
	const auto Linetype {LinetypeObjectFromIndex(g_PrimitiveState.LinetypeIndex())};
	Polyline->setLinetype(Linetype);
	return Polyline;
}

OdDbPolylinePtr EoDbPolyline::Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file) {
	auto Polyline {OdDbPolyline::createObject()};
	Polyline->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(Polyline);
	Polyline->setColorIndex(static_cast<unsigned short>(file.ReadInt16()));
	const auto Linetype {LinetypeObjectFromIndex(file.ReadInt16())};
	Polyline->setLinetype(Linetype);
	const auto Flags {file.ReadUInt16()};
	const auto Closed {(Flags & ms_Closed) == ms_Closed};
	Polyline->setClosed(Closed);
	Polyline->setConstantWidth(file.ReadDouble());
	Polyline->setElevation(file.ReadDouble());
	Polyline->setThickness(file.ReadDouble());
	Polyline->setNormal(file.ReadVector3d());
	const auto NumberOfVertices = file.ReadUInt16();
	for (unsigned VertexIndex = 0; VertexIndex < NumberOfVertices; VertexIndex++) {
		Polyline->addVertexAt(VertexIndex, file.ReadPoint2d());
		Polyline->setWidthsAt(VertexIndex, file.ReadDouble(), file.ReadDouble());
		Polyline->setBulgeAt(VertexIndex, file.ReadDouble());
	}
	return Polyline;
}

EoDbPolyline* EoDbPolyline::Create(OdDbPolylinePtr polyline) {
	auto Polyline {new EoDbPolyline()};
	Polyline->m_EntityObjectId = polyline->objectId();
	Polyline->m_ColorIndex = static_cast<short>(polyline->colorIndex());
	Polyline->m_LinetypeIndex = static_cast<short>(EoDbLinetypeTable::LegacyLinetypeIndex(polyline->linetype()));
	const auto NumberOfVertices {polyline->numVerts()};
	auto Vertex {OdGePoint2d::kOrigin};
	auto StartWidth {0.};
	auto EndWidth {0.};
	for (unsigned VertexIndex = 0; VertexIndex < NumberOfVertices; VertexIndex++) {
		polyline->getPointAt(VertexIndex, Vertex);
		polyline->getWidthsAt(VertexIndex, StartWidth, EndWidth);
		Polyline->AppendVertex(Vertex, polyline->getBulgeAt(VertexIndex), StartWidth, EndWidth);
	}
	Polyline->SetClosed(polyline->isClosed());
	Polyline->m_ConstantWidth = polyline->getConstantWidth();
	Polyline->m_Normal = polyline->normal();
	Polyline->m_Elevation = polyline->elevation();
	return Polyline;
}
