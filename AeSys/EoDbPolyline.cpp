#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

#include "EoGePolyline.h"

#include "EoDbFile.h"

#include "EoDbPolyline.h"

unsigned EoDbPolyline::sm_EdgeToEvaluate = 0;
unsigned EoDbPolyline::sm_Edge = 0;
unsigned EoDbPolyline::sm_PivotVertex = 0;

EoDbPolyline::EoDbPolyline()
	: m_Flags(0)
	, m_ConstantWidth(0.0)
	, m_Elevation(0.0)
	, m_Thickness(0.0)
	, m_Normal(OdGeVector3d::kZAxis) {
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

EoDbPolyline::~EoDbPolyline() {
}

const EoDbPolyline& EoDbPolyline::operator=(const EoDbPolyline& other) {
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

	return (*this);
}

void EoDbPolyline::AddReportToMessageList(const OdGePoint3d& point) const {
	const auto NumberOfVertices = m_Vertices.size();

	if (sm_Edge > 0 && sm_Edge <= NumberOfVertices) {
		OdGePoint3d ptBeg;
		OdGePoint3d ptEnd;
		GetPointAt(sm_Edge - 1, ptBeg);
		GetPointAt(sm_Edge % NumberOfVertices, ptEnd);

		if (sm_PivotVertex < NumberOfVertices) {
			GetPointAt(sm_PivotVertex, ptBeg);
			GetPointAt(SwingVertex(), ptEnd);
		}
		double AngleInXYPlane {0.};
		const double EdgeLength = OdGeVector3d(ptEnd - ptBeg).length();

		if (OdGeVector3d(ptBeg - point).length() > EdgeLength * 0.5) {
			AngleInXYPlane = EoGeLineSeg3d(ptEnd, ptBeg).AngleFromXAxis_xy();
		} else {
			AngleInXYPlane = EoGeLineSeg3d(ptBeg, ptEnd).AngleFromXAxis_xy();
		}
		CString Report(L"<Polyline-Edge>");
		Report += L" Color:" + FormatColorIndex();
		Report += L" Linetype:" + FormatLinetypeIndex();
		Report += L" [" + theApp.FormatLength(EdgeLength, theApp.GetUnits()) + L" @ " + theApp.FormatAngle(AngleInXYPlane) + L"]";
		theApp.AddStringToMessageList(Report);

		theApp.SetEngagedLength(EdgeLength);
		theApp.SetEngagedAngle(AngleInXYPlane);
	}
}

void EoDbPolyline::AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept {
	CMainFrame::InsertTreeViewControlItem(tree, parent, L"<Polyline>", this);
}

EoDbPrimitive* EoDbPolyline::Clone(OdDbBlockTableRecordPtr blockTableRecord) const {
	OdDbPolylinePtr Polyline = m_EntityObjectId.safeOpenObject()->clone();
	blockTableRecord->appendOdDbEntity(Polyline);

	return EoDbPolyline::Create(Polyline);
}

void EoDbPolyline::Display(AeSysView* view, CDC* deviceContext) {
	const short ColorIndex = LogicalColorIndex();
	const short LinetypeIndex = LogicalLinetypeIndex();

	pstate.SetPen(view, deviceContext, ColorIndex, LinetypeIndex);

	if (IsClosed())
		polyline::BeginLineLoop();
	else
		polyline::BeginLineStrip();

	const OdGePoint3d Origin = OdGePoint3d::kOrigin + m_Normal * m_Elevation;
	const OdGeVector3d XAxis = ComputeArbitraryAxis(m_Normal);
	const OdGeVector3d YAxis = m_Normal.crossProduct(XAxis);

	OdGePlane Plane(Origin, XAxis, YAxis);
	OdGePoint3d Vertex;

	for (unsigned VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		Vertex.set(Plane, m_Vertices[VertexIndex]);
		polyline::SetVertex(Vertex);
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
	GetPointAt(sm_Edge - 1, StartPoint);
	GetPointAt(sm_Edge % NumberOfVertices, EndPoint);

	return (EoGeLineSeg3d(StartPoint, EndPoint).midPoint());
}

void EoDbPolyline::GetExtents(AeSysView* view, OdGeExtents3d& extents) const {
	OdGePoint3d Point;
	for (unsigned VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		GetPointAt(VertexIndex, Point);
		extents.addPoint(Point);
	}
}

/// <summary> This function sets point to the 3D location of the vertex index in World Coordinates.</summary>
void EoDbPolyline::GetPointAt(int vertexIndex, OdGePoint3d& point) const {
	const OdGePoint3d Origin = OdGePoint3d::kOrigin + m_Normal * m_Elevation;
	const OdGeVector3d XAxis = ComputeArbitraryAxis(m_Normal);
	const OdGeVector3d YAxis = m_Normal.crossProduct(XAxis);
	OdGePlane Plane(Origin, XAxis, YAxis);
	point.set(Plane, m_Vertices[vertexIndex]);
}

OdGePoint3d EoDbPolyline::GoToNxtCtrlPt() const {
	const auto NumberOfVertices = m_Vertices.size();
	if (sm_PivotVertex >= NumberOfVertices) { // have not yet rocked to a vertex
		const auto StartVertexIndex {sm_Edge - 1};
		const auto EndVertexIndex {sm_Edge % NumberOfVertices};

		OdGePoint3d StartPoint;
		GetPointAt(StartVertexIndex, StartPoint);
		OdGePoint3d EndPoint;
		GetPointAt(EndVertexIndex, EndPoint);

		if (EndPoint.x > StartPoint.x) {
			sm_PivotVertex = StartVertexIndex;
		} else if (EndPoint.x < StartPoint.x) {
			sm_PivotVertex = EndVertexIndex;
		} else if (EndPoint.y > StartPoint.y) {
			sm_PivotVertex = StartVertexIndex;
		} else {
			sm_PivotVertex = EndVertexIndex;
		}
	} else if (sm_PivotVertex == 0) {
		if (sm_Edge == 1) {
			sm_PivotVertex = 1;
		} else {
			sm_PivotVertex = NumberOfVertices - 1;
		}
	} else if (sm_PivotVertex == NumberOfVertices - 1) {
		if (sm_Edge == NumberOfVertices) {
			sm_PivotVertex = 0;
		} else {
			sm_PivotVertex--;
		}
	} else {
		if (sm_Edge == sm_PivotVertex) {
			sm_PivotVertex--;
		} else {
			sm_PivotVertex++;
		}
	}
	OdGePoint3d PivotPoint;
	GetPointAt(sm_PivotVertex, PivotPoint);
	return (PivotPoint);
}

bool EoDbPolyline::IsClosed() const noexcept {
	return (m_Flags != 0);
}

bool EoDbPolyline::IsInView(AeSysView* view) const {
	OdGePoint3d Point;
	EoGePoint4d	pt[2];
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

bool EoDbPolyline::IsPointOnControlPoint(AeSysView * view, const EoGePoint4d & point) const noexcept {
	// <tas="Polyline: need to implement IsPointOnControlPoint"</tas>
	return false;
}

bool EoDbPolyline::PivotOnGripPoint(AeSysView * view, const EoGePoint4d & point) noexcept {
	const auto NumberOfVertices {m_Vertices.size()};

	if (sm_PivotVertex >= NumberOfVertices) { return false; } // Not engaged at a vertex

	OdGePoint3d Point;
	GetPointAt(sm_PivotVertex, Point);
	EoGePoint4d ptCtrl(Point, 1.0);
	view->ModelViewTransformPoint(ptCtrl);

	if (ptCtrl.DistanceToPointXY(point) >= sm_SelectApertureSize) { return false; } // Not on proper vertex

	if (sm_PivotVertex == 0) {
		sm_Edge = sm_Edge == 1 ? NumberOfVertices : 1;
	} else if (sm_PivotVertex == NumberOfVertices - 1) {
		sm_Edge = (sm_Edge == NumberOfVertices) ? sm_Edge - 1 : NumberOfVertices;
	} else if (sm_PivotVertex == sm_Edge) {
		sm_Edge++;
	} else {
		sm_Edge--;
	}
	return true;
}

OdGePoint3d EoDbPolyline::SelectAtControlPoint(AeSysView * view, const EoGePoint4d & point) const {
	sm_ControlPointIndex = SIZE_T_MAX;
	double dApert = sm_SelectApertureSize;

	sm_PivotVertex = m_Vertices.size();

	for (unsigned VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		OdGePoint3d Point;
		GetPointAt(VertexIndex, Point);
		EoGePoint4d pt(Point, 1.0);
		view->ModelViewTransformPoint(pt);

		const double dDis = point.DistanceToPointXY(pt);

		if (dDis < dApert) {
			sm_ControlPointIndex = VertexIndex;
			dApert = dDis;

			sm_Edge = VertexIndex + 1;
			sm_PivotVertex = VertexIndex;
		}
	}
	OdGePoint3d ControlPoint(OdGePoint3d::kOrigin);
	if (sm_ControlPointIndex != SIZE_T_MAX) {
		GetPointAt(sm_ControlPointIndex, ControlPoint);
	}
	return (ControlPoint);
}

bool EoDbPolyline::SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& ptProj) const {
	const auto NumberOfVertices = m_Vertices.size();
	if (sm_EdgeToEvaluate > 0 && sm_EdgeToEvaluate <= NumberOfVertices) { // Evaluate specified edge of polyline
		OdGePoint3d StartPoint;
		GetPointAt(sm_EdgeToEvaluate - 1, StartPoint);
		OdGePoint3d EndPoint;
		GetPointAt(sm_EdgeToEvaluate % NumberOfVertices, EndPoint);
		EoGePoint4d ptBeg(StartPoint, 1.0);
		EoGePoint4d ptEnd(EndPoint, 1.0);

		view->ModelViewTransformPoint(ptBeg);
		view->ModelViewTransformPoint(ptEnd);

		EoGeLineSeg3d LineSegment(ptBeg.Convert3d(), ptEnd.Convert3d());
		if (LineSegment.IsSelectedBy_xy(point.Convert3d(), view->SelectApertureSize(), ptProj, sm_RelationshipOfPoint)) {
			ptProj.z = ptBeg.z + sm_RelationshipOfPoint * (ptEnd.z - ptBeg.z);
			return true;
		}
	} else { // Evaluate entire polyline
		auto NumberofEdges = NumberOfVertices;
		
		if (!IsClosed()) { NumberofEdges--; }

		OdGePoint3d StartPoint;
		GetPointAt(0, StartPoint);
		EoGePoint4d ptBeg(StartPoint, 1.0);
		view->ModelViewTransformPoint(ptBeg);

		for (unsigned VertexIndex = 1; VertexIndex <= NumberofEdges; VertexIndex++) {
			OdGePoint3d EndPoint;
			GetPointAt(VertexIndex % NumberOfVertices, EndPoint);
			EoGePoint4d ptEnd(EndPoint, 1.0);
			view->ModelViewTransformPoint(ptEnd);

			EoGeLineSeg3d LineSegment(ptBeg.Convert3d(), ptEnd.Convert3d());
			if (LineSegment.IsSelectedBy_xy(point.Convert3d(), view->SelectApertureSize(), ptProj, sm_RelationshipOfPoint)) {
				ptProj.z = ptBeg.z + sm_RelationshipOfPoint * (ptEnd.z - ptBeg.z);
				sm_Edge = VertexIndex;
				sm_PivotVertex = NumberOfVertices;
				return true;
			}
			ptBeg = ptEnd;
		}
	}
	return false;
}

bool EoDbPolyline::SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const {
	OdGePoint3dArray Points;

	for (unsigned VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		OdGePoint3d Point;
		GetPointAt(VertexIndex, Point);
		Points.append(Point);
	}
	return polyline::SelectUsingRectangle(view, lowerLeftCorner, upperRightCorner, Points);
}

void EoDbPolyline::SetClosed(bool closed) noexcept {
	if (closed) {
		m_Flags |= sm_Closed;
	} else {
		m_Flags &= ~sm_Closed;
	}
}

void EoDbPolyline::AppendVertex(const OdGePoint2d& vertex, double bulge, double startWidth, double endWidth) {
	const auto VertexIndex {m_Vertices.append(vertex)};
	m_Bulges.append(bulge);
	m_StartWidths.append(startWidth);
	m_EndWidths.append(endWidth);
}

void EoDbPolyline::SetConstantWidth(double constantWidth) noexcept {
	m_ConstantWidth = constantWidth;
}

void EoDbPolyline::SetNormal(const OdGeVector3d & normal) {
	m_Normal = normal.isZeroLength() ? OdGeVector3d::kZAxis : normal;
}

void EoDbPolyline::TransformBy(const EoGeMatrix3d & transformMatrix) {
	// <tas="TransformBy broken. Need to go to world and back?"</tas>
	for (unsigned VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		//m_Vertices[VertexIndex].transformBy(transformMatrix);
	}
}

void EoDbPolyline::TranslateUsingMask(const OdGeVector3d& translate, const unsigned long mask) {
	// <tas="TranslateUsingMask broken. Need to go to world and back? This type of operation could get polyline vertex off plane"</tas>
	for (unsigned VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
	//	if (((mask >> VertexIndex) & 1UL) == 1)
	//		m_Vertices[VertexIndex] += translate;
	}
}

bool EoDbPolyline::Write(EoDbFile & file) const {
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
void EoDbPolyline::Write(CFile& file, unsigned char* buffer) const noexcept {
};

unsigned EoDbPolyline::SwingVertex() const {
	const auto NumberOfVertices = m_Vertices.size();
	unsigned SwingVertex;

	if (sm_PivotVertex == 0) {
		SwingVertex = (sm_Edge == 1) ? 1 : NumberOfVertices - 1;
	} else if (sm_PivotVertex == NumberOfVertices - 1) {
		SwingVertex = (sm_Edge == NumberOfVertices) ? 0 : sm_PivotVertex - 1;
	} else {
		SwingVertex = (sm_Edge == sm_PivotVertex) ? sm_PivotVertex - 1 : sm_PivotVertex + 1;
	}
	return (SwingVertex);
}

unsigned EoDbPolyline::Edge() noexcept {
	return sm_Edge;
}

void EoDbPolyline::SetEdgeToEvaluate(unsigned edgeToEvaluate) noexcept {
	sm_EdgeToEvaluate = edgeToEvaluate;
}

OdDbPolylinePtr EoDbPolyline::Create(OdDbBlockTableRecordPtr blockTableRecord) {
	OdDbPolylinePtr Polyline = OdDbPolyline::createObject();
	Polyline->setDatabaseDefaults(blockTableRecord->database());

	blockTableRecord->appendOdDbEntity(Polyline);
	Polyline->setColorIndex(pstate.ColorIndex());

	const auto Linetype {EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex())};

	Polyline->setLinetype(Linetype);

	return Polyline;
}

OdDbPolylinePtr EoDbPolyline::Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile & file) {
	OdDbPolylinePtr Polyline = OdDbPolyline::createObject();
	Polyline->setDatabaseDefaults(blockTableRecord->database());

	blockTableRecord->appendOdDbEntity(Polyline);

	Polyline->setColorIndex(file.ReadInt16());

	const auto Linetype {EoDbPrimitive::LinetypeObjectFromIndex(file.ReadInt16())};

	Polyline->setLinetype(Linetype);

	unsigned short Flags = file.ReadUInt16();
	auto Closed {(Flags && sm_Closed) == sm_Closed};
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
	return (Polyline);
}

EoDbPolyline* EoDbPolyline::Create(OdDbPolylinePtr polyline) {
	auto Polyline {new EoDbPolyline()};

	Polyline->m_EntityObjectId = polyline->objectId();
	Polyline->m_ColorIndex = polyline->colorIndex();
	Polyline->m_LinetypeIndex = EoDbLinetypeTable::LegacyLinetypeIndex(polyline->linetype());

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
