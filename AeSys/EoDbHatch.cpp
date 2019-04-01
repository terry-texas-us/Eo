#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysView.h"

#include "HatchPatternManager.h"

size_t EoDbHatch::sm_EdgeToEvaluate = 0;
size_t EoDbHatch::sm_Edge = 0;
size_t EoDbHatch::sm_PivotVertex = 0;

double EoDbHatch::sm_PatternAngle = 0.;
double EoDbHatch::sm_PatternScaleX = .1;
double EoDbHatch::sm_PatternScaleY = .1;

OdStringArray EoDbHatch::sm_HatchNames;
int EoDbHatch::sm_HatchPatternOffsets[64];
double EoDbHatch::sm_HatchPatternTable[1536];

struct EoEdge {
	double dMinY; // minimum y extent of edge
	double dMaxY; // maximum y extent of edge
	double dX; // x intersection on edge
	union {
		double dInvSlope; // inverse slope of edge
		double dStepSiz; // change in x for each scanline
	};
};
EoDbHatch::EoDbHatch() 
    : m_InteriorStyle(EoDbHatch::kHatch)
    , m_InteriorStyleIndex(1)
    , m_Vertices(0)
    , m_NumberOfLoops(0)
    , m_HatchOrigin(OdGePoint3d::kOrigin)
    , m_HatchXAxis(OdGeVector3d::kXAxis)
    , m_HatchYAxis(OdGeVector3d::kYAxis) {
}

EoDbHatch::EoDbHatch(const EoDbHatch& other) 
    : m_NumberOfLoops(0) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;

	m_ColorIndex = other.m_ColorIndex;
	m_InteriorStyle = other.m_InteriorStyle;
	m_InteriorStyleIndex = other.m_InteriorStyleIndex;
	m_HatchOrigin = other.m_HatchOrigin;
	m_HatchXAxis = other.m_HatchXAxis;
	m_HatchYAxis = other.m_HatchYAxis;
	
	m_Vertices.clear();
	m_Vertices.append(other.m_Vertices);
}
EoDbHatch::~EoDbHatch() {
}
const EoDbHatch& EoDbHatch::operator=(const EoDbHatch& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;

	m_ColorIndex = other.m_ColorIndex;
	m_InteriorStyle = other.m_InteriorStyle;
	m_InteriorStyleIndex = other.m_InteriorStyleIndex;
	m_HatchOrigin = other.m_HatchOrigin;
	m_HatchXAxis = other.m_HatchXAxis;
	m_HatchYAxis = other.m_HatchYAxis;

	m_Vertices.clear();
	m_Vertices.append(other.m_Vertices);

	return (*this);
}
void EoDbHatch::AddReportToMessageList(const OdGePoint3d& point) const {
	const size_t NumberOfVertices = m_Vertices.size();

	if (sm_Edge > 0 && sm_Edge <= NumberOfVertices) {
		OdGePoint3d StartPoint = m_Vertices[sm_Edge - 1];
		OdGePoint3d EndPoint = m_Vertices[sm_Edge % NumberOfVertices];

		if (sm_PivotVertex < NumberOfVertices) {
			StartPoint = m_Vertices[sm_PivotVertex];
			EndPoint = m_Vertices[SwingVertex()];
		}
		double AngleInXYPlane;
		const double Length = OdGeVector3d(EndPoint - StartPoint).length();

		if (OdGeVector3d(StartPoint - point).length() > Length * .5) {
			AngleInXYPlane = EoGeLineSeg3d(EndPoint, StartPoint).AngleFromXAxis_xy();
		}
		else {
			AngleInXYPlane = EoGeLineSeg3d(StartPoint, EndPoint).AngleFromXAxis_xy();
		}
		CString Report(L"<Hatch-Edge> ");
		Report += L" Color:" + FormatColorIndex();
		Report += L" [" + theApp.FormatLength(Length, theApp.GetUnits()) + L" @ " + theApp.FormatAngle(AngleInXYPlane) + L"]";
		theApp.AddStringToMessageList(Report);

		theApp.SetEngagedLength(Length);
		theApp.SetEngagedAngle(AngleInXYPlane);
	}
}
void EoDbHatch::AddToTreeViewControl(HWND tree, HTREEITEM parent) const {
	CMainFrame::InsertTreeViewControlItem(tree, parent, L"<Hatch>", this);
}
void EoDbHatch::AssociateWith(OdDbBlockTableRecordPtr blockTableRecord) {
	OdDbHatchPtr HatchEntity = OdDbHatch::createObject();
	blockTableRecord->appendOdDbEntity(HatchEntity);
	HatchEntity->setDatabaseDefaults();

	SetEntityObjectId(HatchEntity->objectId());

	HatchEntity->setAssociative(false);
	
	HatchEntity->setColorIndex(m_ColorIndex);
	OdString HatchName(m_InteriorStyle == kSolid ? L"SOLID" : EoDbHatch::sm_HatchNames[m_InteriorStyleIndex]);
	HatchEntity->setPattern(OdDbHatch::kPreDefined, HatchName);
	
	const OdGeVector3d PlaneNormal = RecomputeReferenceSystem();
	HatchEntity->setNormal(PlaneNormal);
	
	OdGePlane Plane(m_Vertices[0], PlaneNormal);
	OdGeMatrix3d WorldToPlaneTransform;
	WorldToPlaneTransform.setToWorldToPlane(Plane);

	OdGePoint3d WorldOriginOnPlane = OdGePoint3d::kOrigin.orthoProject(Plane);
	OdGeVector3d PointToPlaneVector(WorldOriginOnPlane.asVector());
	PointToPlaneVector.transformBy(WorldToPlaneTransform);
	HatchEntity->setElevation(PointToPlaneVector.z);
	
	WorldToPlaneTransform.setToWorldToPlane(OdGePlane(OdGePoint3d::kOrigin, PlaneNormal));

	OdGePoint2dArray Vertices;
	Vertices.clear();
	OdGeDoubleArray Bulges;
	Bulges.clear();
	for (size_t VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		OdGePoint3d Vertex(m_Vertices[VertexIndex]);
		Vertex.transformBy(WorldToPlaneTransform);
		Vertices.append(Vertex.convert2d());
		Bulges.append(0.);
	}
	HatchEntity->appendLoop(OdDbHatch::kPolyline, Vertices, Bulges);
}
EoDbPrimitive* EoDbHatch::Clone(OdDbDatabasePtr database) const {
	return (EoDbHatch::Create(*this, database));
}
void EoDbHatch::Display(AeSysView* view, CDC* deviceContext) {
	const EoInt16 ColorIndex = LogicalColorIndex();

	pstate.SetColorIndex(deviceContext, ColorIndex);
	pstate.SetHatchInteriorStyle(m_InteriorStyle);
	pstate.SetHatchInteriorStyleIndex(m_InteriorStyleIndex);

	if (m_InteriorStyle == EoDbHatch::kHatch) {
		DisplayHatch(view, deviceContext);
	}
	else { // Fill area interior style is hollow, solid or pattern
	 	DisplaySolid(view, deviceContext);
	}
}
void EoDbHatch::FormatExtra(CString& extra) const {
	extra.Empty();
	extra += L"Color;" + FormatColorIndex() + L"\t";
	extra += L"Interior Style;" + FormatInteriorStyle() + L"\t";
	extra += L"Interior Style Name;" + CString((LPCWSTR) sm_HatchNames[m_InteriorStyleIndex]) + L"\t";
	CString NumberOfVertices;
	NumberOfVertices.Format(L"Number of Vertices;%d", m_Vertices.size());
	extra += NumberOfVertices;
}
void EoDbHatch::FormatGeometry(CString& geometry) const {
	CString HatchOriginString;
	HatchOriginString.Format(L"Hatch Origin;%f;%f;%f\t", m_HatchOrigin.x, m_HatchOrigin.y, m_HatchOrigin.z);
	geometry += HatchOriginString;
	CString XAxisString;
	XAxisString.Format(L"X Axis;%f;%f;%f\t", m_HatchXAxis.x, m_HatchXAxis.y, m_HatchXAxis.z);
	geometry += XAxisString;
	CString YAxisString;
	YAxisString.Format(L"Y Axis;%f;%f;%f\t", m_HatchYAxis.x, m_HatchYAxis.y, m_HatchYAxis.z);
	geometry += YAxisString;
	CString VertexString;
	for (size_t VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		VertexString.Format(L"Vertex;%f;%f;%f\t", m_Vertices[VertexIndex].x, m_Vertices[VertexIndex].y, m_Vertices[VertexIndex].z);
		geometry += VertexString;
	}
}
void EoDbHatch::GetAllPoints(OdGePoint3dArray& points) const {
	points.empty();
	for (size_t VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		points.append(m_Vertices[VertexIndex]);
	}
}
OdGePoint3d EoDbHatch::GetCtrlPt() const {
	const size_t StartPointIndex = sm_Edge - 1;
	const size_t EndPointIndex = sm_Edge % m_Vertices.size();
	return (EoGeLineSeg3d(m_Vertices[StartPointIndex], m_Vertices[EndPointIndex]).midPoint());
};
void EoDbHatch::GetExtents(AeSysView* view, OdGeExtents3d& extents) const {
	for (size_t VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		extents.addPoint(m_Vertices[VertexIndex]);
	}
}
OdGePoint3d EoDbHatch::GoToNxtCtrlPt() const {
	const size_t NumberOfVertices = m_Vertices.size();
	if (sm_PivotVertex >= NumberOfVertices) { // have not yet rocked to a vertex
		const size_t StartVertexIndex = sm_Edge - 1;
		const OdGePoint3d StartPoint(m_Vertices[StartVertexIndex]);
		const size_t EndVertexIndex = sm_Edge % NumberOfVertices;
		const OdGePoint3d EndPoint(m_Vertices[EndVertexIndex]);
		if (EndPoint.x > StartPoint.x) {
			sm_PivotVertex = StartVertexIndex;
		}
		else if (EndPoint.x < StartPoint.x) {
			sm_PivotVertex = EndVertexIndex;
		}
		else if (EndPoint.y > StartPoint.y) {
			sm_PivotVertex = StartVertexIndex;
		}
		else {
			sm_PivotVertex = EndVertexIndex;
		}
	}
	else if (sm_PivotVertex == 0) {
		if (sm_Edge == 1) {
			sm_PivotVertex = 1;
		}
		else {
			sm_PivotVertex = NumberOfVertices - 1;
		}
	}
	else if (sm_PivotVertex == NumberOfVertices - 1) {
		if (sm_Edge == NumberOfVertices) {
			sm_PivotVertex = 0;
		}
		else {
			sm_PivotVertex--;
		}
	}
	else {
		if (sm_Edge == sm_PivotVertex) {
			sm_PivotVertex--;
		}
		else {
			sm_PivotVertex++;
		}
	}
	return (m_Vertices[sm_PivotVertex]);
}
bool EoDbHatch::Is(EoUInt16 type) const {
	return type == EoDb::kHatchPrimitive;
}
bool EoDbHatch::IsInView(AeSysView* view) const {
	EoGePoint4d pt[2];

	pt[0] = EoGePoint4d(m_Vertices[0], 1.);
	view->ModelViewTransformPoint(pt[0]);

	for (int i = m_Vertices.size() - 1; i >= 0; i--) {
		pt[1] = EoGePoint4d(m_Vertices[i], 1.);
		view->ModelViewTransformPoint(pt[1]);

		if (EoGePoint4d::ClipLine(pt[0], pt[1]))
			return true;
		pt[0] = pt[1];
	}
	return false;
}
bool EoDbHatch::IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	for (size_t VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		EoGePoint4d pt(m_Vertices[VertexIndex], 1.);
		view->ModelViewTransformPoint(pt);

		if (point.DistanceToPointXY(pt) < sm_SelectApertureSize)
			return true;
	}
	return false;
}
OdGePoint3d EoDbHatch::SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	sm_ControlPointIndex = SIZE_T_MAX;
	double dApert = sm_SelectApertureSize;

	sm_PivotVertex = m_Vertices.size();

	for (size_t VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		EoGePoint4d pt(m_Vertices[VertexIndex], 1.);
		view->ModelViewTransformPoint(pt);

		const double dDis = point.DistanceToPointXY(pt);

		if (dDis < dApert) {
			sm_ControlPointIndex = VertexIndex;
			dApert = dDis;

			sm_Edge = VertexIndex + 1;
			sm_PivotVertex = VertexIndex;
		}
	}
	return (sm_ControlPointIndex == SIZE_T_MAX) ? OdGePoint3d::kOrigin : m_Vertices[sm_ControlPointIndex];
}
bool EoDbHatch::SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const {
	OdGePoint3dArray Points;
	for (size_t VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		Points.append(m_Vertices[VertexIndex]);
	}
	return polyline::SelectUsingRectangle(view, lowerLeftCorner, upperRightCorner, Points);
}
bool EoDbHatch::SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& ptProj) const {
	const size_t NumberOfVertices = m_Vertices.size();
	if (sm_EdgeToEvaluate > 0 && sm_EdgeToEvaluate <= NumberOfVertices) { // Evaluate specified edge of polygon
		EoGePoint4d ptBeg(m_Vertices[sm_EdgeToEvaluate - 1], 1.);
		EoGePoint4d ptEnd(m_Vertices[sm_EdgeToEvaluate % NumberOfVertices], 1.);

		view->ModelViewTransformPoint(ptBeg);
		view->ModelViewTransformPoint(ptEnd);

		EoGeLineSeg3d Edge(ptBeg.Convert3d(), ptEnd.Convert3d());
		if (Edge.IsSelectedBy_xy(point.Convert3d(), view->SelectApertureSize(), ptProj, sm_RelationshipOfPoint)) {
			ptProj.z = ptBeg.z + sm_RelationshipOfPoint * (ptEnd.z - ptBeg.z);
			return true;
		}
	}
	else { // Evaluate entire polygon
		EoGePoint4d ptBeg(m_Vertices[0], 1.);
		view->ModelViewTransformPoint(ptBeg);

		for (size_t VertexIndex = 1; VertexIndex <= NumberOfVertices; VertexIndex++) {
			EoGePoint4d ptEnd(m_Vertices[VertexIndex % NumberOfVertices], 1.);
			view->ModelViewTransformPoint(ptEnd);

			EoGeLineSeg3d Edge(ptBeg.Convert3d(), ptEnd.Convert3d());
			if (Edge.IsSelectedBy_xy(point.Convert3d(), view->SelectApertureSize(), ptProj, sm_RelationshipOfPoint)) {
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
void EoDbHatch::TransformBy(const EoGeMatrix3d& transformMatrix) {
	m_HatchOrigin.transformBy(transformMatrix);
	m_HatchXAxis.transformBy(transformMatrix);
	m_HatchYAxis.transformBy(transformMatrix);
	for (size_t VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++)
		m_Vertices[VertexIndex].transformBy(transformMatrix);
}
void EoDbHatch::TranslateUsingMask(const OdGeVector3d& translate, const DWORD mask) {
	// nothing done to hatch coordinate origin

	for (size_t VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		if (((mask >> VertexIndex) & 1UL) == 1) {
			m_Vertices[VertexIndex] += translate;
		}
	}
}
bool EoDbHatch::Write(EoDbFile& file) const {
	file.WriteUInt16(EoDb::kHatchPrimitive);
	file.WriteInt16(m_ColorIndex);
	file.WriteInt16(m_InteriorStyle);  // note polygon style stuffed up into unused line type on io
	file.WriteUInt16(EoUInt16(EoMax(1U, m_InteriorStyleIndex)));
	file.WriteUInt16(EoUInt16(m_Vertices.size()));
	file.WritePoint3d(m_HatchOrigin);
	
	file.WriteDouble(m_HatchXAxis.x);
	file.WriteDouble(m_HatchXAxis.y);
	file.WriteDouble(m_HatchXAxis.z);

	file.WriteDouble(m_HatchYAxis.x);
	file.WriteDouble(m_HatchYAxis.y);
	file.WriteDouble(m_HatchYAxis.z);


	for (size_t VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		file.WritePoint3d(m_Vertices[VertexIndex]);
	}
	return true;
}
void EoDbHatch::Write(CFile& file, EoByte* buffer) const {
	buffer[3] = EoSbyte((79 + m_Vertices.size() * 12) / 32);
	*((EoUInt16*) &buffer[4]) = EoUInt16(EoDb::kHatchPrimitive);
	buffer[6] = EoSbyte(m_ColorIndex == COLORINDEX_BYLAYER ? sm_LayerColorIndex : m_ColorIndex);
	buffer[7] = EoSbyte(m_InteriorStyle);
	*((EoInt16*) &buffer[8]) = EoInt16(m_InteriorStyleIndex);
	*((EoInt16*) &buffer[10]) = EoInt16(m_Vertices.size());

	((EoVaxPoint3d*) &buffer[12])->Convert(m_HatchOrigin);
	((EoVaxVector3d*) &buffer[24])->Convert(m_HatchXAxis);
	((EoVaxVector3d*) &buffer[36])->Convert(m_HatchYAxis);

	int i = 48;

	for (size_t VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		((EoVaxPoint3d*) &buffer[i])->Convert(m_Vertices[VertexIndex]);
		i += sizeof(EoVaxPoint3d);
	}
	file.Write(buffer, buffer[3] * 32);
}

int EoDbHatch::Append(const OdGePoint3d& vertex) {
	return (m_Vertices.append(vertex));
}
void EoDbHatch::DisplayHatch(AeSysView* view, CDC* deviceContext) const {
	EoGeMatrix3d tm;
	tm.setToWorldToPlane(OdGePlane(m_HatchOrigin, m_HatchXAxis, m_HatchYAxis));

	const int NumberOfLoops = 1;
	int LoopPointsOffsets[2];
	LoopPointsOffsets[0] = m_Vertices.size();

	EoEdge Edges[128];

	const EoInt16 ColorIndex = pstate.ColorIndex();
	const EoInt16 LinetypeIndex = pstate.LinetypeIndex();
	pstate.SetLinetypeIndex(deviceContext, 1);
	const int InteriorStyleIndex = pstate.HatchInteriorStyleIndex();

	OdHatchPattern HatchPattern;
	RetrieveHatchPattern(sm_HatchNames[InteriorStyleIndex], HatchPattern);
	const size_t NumberOfPatterns = HatchPattern.size();

	OdHatchPatternLine HatchPatternLine;

	for (size_t PatternIndex = 0; PatternIndex < NumberOfPatterns; PatternIndex++) {
		HatchPatternLine = HatchPattern.getAt(PatternIndex);
		const int NumberOfDashesInPattern = HatchPatternLine.m_dashes.size();
		double TotalPatternLength = 0;
		for (int DashIndex = 0; DashIndex < NumberOfDashesInPattern; DashIndex++) {
			TotalPatternLength += fabs(HatchPatternLine.m_dashes[DashIndex]);
		}
		OdGePoint2d RotatedBasePoint(HatchPatternLine.m_basePoint);
		RotatedBasePoint.rotateBy(- HatchPatternLine.m_dLineAngle);

		// Add rotation to matrix which gets current scan lines parallel to x-axis
		EoGeMatrix3d tmRotZ;
		tmRotZ.setToRotation(- HatchPatternLine.m_dLineAngle, OdGeVector3d::kZAxis);
		tm.preMultBy(tmRotZ);
		EoGeMatrix3d tmInv = tm;
		tmInv.invert();

		int ActiveEdges = 0;
		int FirstLoopPointIndex = 0;
		for (int LoopIndex = 0; LoopIndex < NumberOfLoops; LoopIndex++) {
			if (LoopIndex != 0) {
				FirstLoopPointIndex = LoopPointsOffsets[LoopIndex - 1];
			}
			OdGePoint3d StartPoint(m_Vertices[FirstLoopPointIndex]);
			StartPoint.transformBy(tm);		// Apply transform to get areas first point in z0 plane
			
			const int SizeOfCurrentLoop = LoopPointsOffsets[LoopIndex] - FirstLoopPointIndex;
			for (int LoopPointIndex = FirstLoopPointIndex; LoopPointIndex < LoopPointsOffsets[LoopIndex]; LoopPointIndex++) {
				OdGePoint3d EndPoint(m_Vertices[((LoopPointIndex - FirstLoopPointIndex + 1) % SizeOfCurrentLoop) + FirstLoopPointIndex]);
				EndPoint.transformBy(tm);
				const OdGeVector2d Edge(EndPoint.x - StartPoint.x, EndPoint.y - StartPoint.y);
				if (!Edge.isZeroLength() && !Edge.isParallelTo(OdGeVector2d::kXAxis)) {
					const double dMaxY = EoMax(StartPoint.y, EndPoint.y);
					int CurrentEdgeIndex = ActiveEdges + 1;
					// Find correct insertion point for edge in edge list using ymax as sort key
					while (CurrentEdgeIndex != 1 && Edges[CurrentEdgeIndex - 1].dMaxY < dMaxY) {
						Edges[CurrentEdgeIndex] = Edges[CurrentEdgeIndex - 1];		// Move entry down
						CurrentEdgeIndex--;
					}
					// Insert information about new edge
					Edges[CurrentEdgeIndex].dMaxY = dMaxY;
					Edges[CurrentEdgeIndex].dInvSlope = Edge.x / Edge.y;
					if (StartPoint.y > EndPoint.y) {
						Edges[CurrentEdgeIndex].dMinY = EndPoint.y;
						Edges[CurrentEdgeIndex].dX = StartPoint.x;
					}
					else {
						Edges[CurrentEdgeIndex].dMinY = StartPoint.y;
						Edges[CurrentEdgeIndex].dX = EndPoint.x;
					}
					ActiveEdges++;
				}
				StartPoint = EndPoint;
			}
		}
		OdGeVector2d PatternOffset(HatchPatternLine.m_patternOffset);
		if (PatternOffset.y < 0.) {
			PatternOffset.negate();
		}
		// Determine where first scan position is
		double dScan = Edges[1].dMaxY - fmod((Edges[1].dMaxY - RotatedBasePoint.y), PatternOffset.y);
		if (Edges[1].dMaxY < dScan) {
			dScan = dScan - PatternOffset.y;
		}
		double dSecBeg = RotatedBasePoint.x + PatternOffset.x * (dScan - RotatedBasePoint.y) / PatternOffset.y;
		// Edge list pointers
		int iBegEdg = 1;
		int iEndEdg = 1;
		// Determine relative epsilon to be used for extent tests
l1:		const double dEps1 = DBL_EPSILON + DBL_EPSILON * fabs(dScan);
		while (iEndEdg <= ActiveEdges && Edges[iEndEdg].dMaxY >= dScan - dEps1) {
			// Set x intersection back to last scanline
			Edges[iEndEdg].dX += Edges[iEndEdg].dInvSlope * (PatternOffset.y + dScan - Edges[iEndEdg].dMaxY);
			// Determine the change in x per scan
			Edges[iEndEdg].dStepSiz = - Edges[iEndEdg].dInvSlope * PatternOffset.y;
			iEndEdg++;
		}
		for (int i = iBegEdg; i < iEndEdg; i++) {
			int CurrentEdgeIndex = i;
			if (Edges[i].dMinY < dScan - dEps1) { // Edge y-extent overlaps current scan . determine intersections
				Edges[i].dX += Edges[i].dStepSiz;
				while (CurrentEdgeIndex > iBegEdg && Edges[CurrentEdgeIndex].dX < Edges[CurrentEdgeIndex - 1].dX) {
					Edges[0] = Edges[CurrentEdgeIndex];
					Edges[CurrentEdgeIndex] = Edges[CurrentEdgeIndex - 1];
					Edges[CurrentEdgeIndex - 1] = Edges[0];
					CurrentEdgeIndex--;
				}
			}
			else { // Edge y-extent does not overlap current scan. remove edge from active edge list
				iBegEdg++;
				while (CurrentEdgeIndex >= iBegEdg) {
					Edges[CurrentEdgeIndex] = Edges[CurrentEdgeIndex - 1];
					CurrentEdgeIndex--;
				}
			}
		}
		if (iEndEdg != iBegEdg) { // At least one pair of edge intersections .. generate pattern lines for each pair
			int CurrentEdgeIndex = iBegEdg;

			if (HatchPatternLine.m_dashes.isEmpty()) {
				for (int EdgePairIndex = 1; EdgePairIndex <= (iEndEdg - iBegEdg) / 2; EdgePairIndex++) {
					const OdGePoint3d StartPoint(Edges[CurrentEdgeIndex].dX, dScan, 0.);
					const OdGePoint3d EndPoint(Edges[CurrentEdgeIndex + 1].dX, dScan, 0.);
					if (!StartPoint.isEqualTo(EndPoint)) {
						EoGeLineSeg3d Line(StartPoint, EndPoint);
						Line.transformBy(tmInv);
						Line.Display(view, deviceContext);
					}
					CurrentEdgeIndex = CurrentEdgeIndex + 2;
				}
			}
			else {
				OdGePoint3d StartPoint;
				OdGePoint3d EndPoint;

				StartPoint.y = dScan;
				EndPoint.y = dScan;

				for (int EdgePairIndex = 1; EdgePairIndex <= (iEndEdg - iBegEdg) / 2; EdgePairIndex++) {
					StartPoint.x = Edges[CurrentEdgeIndex].dX - fmod((Edges[CurrentEdgeIndex].dX - dSecBeg), TotalPatternLength);
					if (StartPoint.x > Edges[CurrentEdgeIndex].dX) {
						StartPoint.x -= TotalPatternLength;
					}
					// Determine the index of the pattern item which intersects the left edge and how much of it is between the edges
					int DashIndex = 0;
					double DistanceToLeftEdge = Edges[CurrentEdgeIndex].dX - StartPoint.x;
					double CurrentDashLength = fabs(HatchPatternLine.m_dashes[DashIndex]);
					while (CurrentDashLength <= DistanceToLeftEdge + DBL_EPSILON) {
						StartPoint.x += CurrentDashLength;
						DistanceToLeftEdge -= CurrentDashLength;
						DashIndex = (DashIndex + 1) % NumberOfDashesInPattern;
						CurrentDashLength = fabs(HatchPatternLine.m_dashes[DashIndex]);
					}
					StartPoint.x = Edges[CurrentEdgeIndex].dX;
					CurrentDashLength -= DistanceToLeftEdge;

					double DistanceToRightEdge = Edges[CurrentEdgeIndex + 1].dX - Edges[CurrentEdgeIndex].dX;
					while (CurrentDashLength <= DistanceToRightEdge + DBL_EPSILON) {
						EndPoint.x = StartPoint.x + CurrentDashLength;
						if (HatchPatternLine.m_dashes[DashIndex] >= 0.) {
							if (HatchPatternLine.m_dashes[DashIndex] == 0.) {
								OdGePoint3d Dot(StartPoint);
								Dot.transformBy(tmInv);
								view->DisplayPixel(deviceContext, theApp.GetHotColor(ColorIndex), Dot);
							}
							else
							{
								EoGeLineSeg3d Line(StartPoint, EndPoint);
								Line.transformBy(tmInv);
								Line.Display(view, deviceContext);
							}
						}
						DistanceToRightEdge -= CurrentDashLength;
						DashIndex = (DashIndex + 1) % NumberOfDashesInPattern;
						CurrentDashLength = fabs(HatchPatternLine.m_dashes[DashIndex]);
						StartPoint.x = EndPoint.x;
					}
					if (HatchPatternLine.m_dashes[DashIndex] >= 0.) {
						EndPoint.x = Edges[CurrentEdgeIndex + 1].dX;
						if (!StartPoint.isEqualTo(EndPoint)) {
							EoGeLineSeg3d Line(StartPoint, EndPoint);
							Line.transformBy(tmInv);
							Line.Display(view, deviceContext);
						}
					}
					CurrentEdgeIndex = CurrentEdgeIndex + 2;
				}
			}
			// Update position of scan line
			dScan -= PatternOffset.y;
			dSecBeg -= PatternOffset.x;
			goto l1;
		}
		tmRotZ.setToRotation(HatchPatternLine.m_dLineAngle, OdGeVector3d::kZAxis);
		tm.preMultBy(tmRotZ);
	}
	pstate.SetPen(view, deviceContext, ColorIndex, LinetypeIndex);
}
void EoDbHatch::DisplaySolid(AeSysView* view, CDC* deviceContext) const {
	size_t NumberOfVertices = m_Vertices.size();
	if (NumberOfVertices >= 2) {
		EoGePoint4dArray Vertices;

		Vertices.SetSize(NumberOfVertices);

		for (size_t VertexIndex = 0; VertexIndex < NumberOfVertices; VertexIndex++) {
			Vertices[VertexIndex] = EoGePoint4d(m_Vertices[VertexIndex], 1.);
		}
		view->ModelViewTransformPoints(Vertices);
		EoGePoint4d::ClipPolygon(Vertices);

		NumberOfVertices = Vertices.GetSize();
		CPoint* pnt = new CPoint[NumberOfVertices];

		view->DoViewportProjection(pnt, Vertices);

		if (m_InteriorStyle == EoDbHatch::kSolid) {
			CBrush Brush(pColTbl[pstate.ColorIndex()]);
			CBrush* pBrushOld = deviceContext->SelectObject(&Brush);
			deviceContext->Polygon(pnt, NumberOfVertices);
			deviceContext->SelectObject(pBrushOld);
		}
		else if (m_InteriorStyle == EoDbHatch::kHollow) {
			CBrush* pBrushOld = (CBrush*) deviceContext->SelectStockObject(NULL_BRUSH);
			deviceContext->Polygon(pnt, NumberOfVertices);
			deviceContext->SelectObject(pBrushOld);
		}
		else {
			deviceContext->Polygon(pnt, NumberOfVertices);
		}
		delete [] pnt;
	}
}
CString EoDbHatch::FormatInteriorStyle() const {
	const CString strStyle[] = {L"Hollow", L"Solid", L"Pattern", L"Hatch"};

	CString str = (m_InteriorStyle >= 0 && m_InteriorStyle <= 3) ? strStyle[m_InteriorStyle] : L"Invalid!";

	return (str);
}
OdGePoint3d EoDbHatch::GetPointAt(int pointIndex) {
	return (m_Vertices[pointIndex]);
}
void EoDbHatch::ModifyState() {
	EoDbPrimitive::ModifyState();

	m_InteriorStyle = pstate.HatchInteriorStyle();
	m_InteriorStyleIndex = pstate.HatchInteriorStyleIndex();
}
int EoDbHatch::NumberOfVertices() const {
	return (m_Vertices.size());
}
bool EoDbHatch::PivotOnGripPoint(AeSysView* view, const EoGePoint4d& point) {
	const size_t NumberOfVertices = m_Vertices.size();
	if (sm_PivotVertex >= NumberOfVertices) { // Not engaged at a vertex
		return false;
	}
	EoGePoint4d ptCtrl(m_Vertices[sm_PivotVertex], 1.);
	view->ModelViewTransformPoint(ptCtrl);

	if (ptCtrl.DistanceToPointXY(point) >= sm_SelectApertureSize) { // Not on proper vertex
		return false;
	}
	if (sm_PivotVertex == 0) {
		sm_Edge = sm_Edge == 1 ? NumberOfVertices : 1;
	}
	else if (sm_PivotVertex == NumberOfVertices - 1) {
		sm_Edge = (sm_Edge == NumberOfVertices) ? sm_Edge - 1 : NumberOfVertices;
	}
	else if (sm_PivotVertex == sm_Edge) {
		sm_Edge++;
	}
	else {
		sm_Edge--;
	}
	return true;
}
OdGeVector3d EoDbHatch::RecomputeReferenceSystem() {
	const OdGeVector3d HatchXAxis(m_Vertices[1] - m_Vertices[0]);
	const OdGeVector3d HatchYAxis(m_Vertices[2] - m_Vertices[0]);
	
	OdGeVector3d PlaneNormal = HatchXAxis.crossProduct(HatchYAxis);
	if (!PlaneNormal.isZeroLength()) {
		PlaneNormal.normalize();
		if (m_InteriorStyle != kHatch) {
			m_HatchXAxis = ComputeArbitraryAxis(PlaneNormal);
			m_HatchYAxis = PlaneNormal.crossProduct(m_HatchXAxis);
		}
	}
	return (PlaneNormal);
}
void EoDbHatch::RetrieveHatchPattern(const OdString& hatchPatternName, OdHatchPattern& hatchPattern) const {
	const size_t InteriorStyleIndex = LegacyInteriorStyleIndex(hatchPatternName);
	if (InteriorStyleIndex == 0) {
		if (!m_EntityObjectId.isNull()) {
			OdDbHatchPtr Hatch = m_EntityObjectId.safeOpenObject();
			ATLTRACE2(atlTraceGeneral, 2, L"Hatch pattern <%s> loaded from entity\n", (LPCWSTR) Hatch->patternName());
			hatchPattern = Hatch->getPattern();
		}
	}
	else {
		int TableOffset = sm_HatchPatternOffsets[InteriorStyleIndex];
		const int NumberOfPatterns = int(sm_HatchPatternTable[TableOffset++]);

		OdHatchPatternLine HatchPatternLine;
		for (int PatternIndex = 0; PatternIndex < NumberOfPatterns; PatternIndex++) {
			const int NumberOfDashesInPattern = int(sm_HatchPatternTable[TableOffset++]);
		
			HatchPatternLine.m_dLineAngle = sm_HatchPatternTable[TableOffset++];
			HatchPatternLine.m_basePoint.x = sm_HatchPatternTable[TableOffset++];
			HatchPatternLine.m_basePoint.y = sm_HatchPatternTable[TableOffset++];
			HatchPatternLine.m_patternOffset.x = sm_HatchPatternTable[TableOffset++];
			HatchPatternLine.m_patternOffset.y = sm_HatchPatternTable[TableOffset++];
			HatchPatternLine.m_dashes.clear();
		
			for (int DashIndex = 0; DashIndex < NumberOfDashesInPattern; DashIndex++) {
				HatchPatternLine.m_dashes.append(sm_HatchPatternTable[TableOffset++]);
 				if ((DashIndex & 1) == 1) {
					HatchPatternLine.m_dashes[DashIndex] = - HatchPatternLine.m_dashes[DashIndex];
				}
			}
			// <tas="Until the hatch file is modified Legacy continuous pattern line marked by 1.E16 dash length</ToReview>
			if (HatchPatternLine.m_dashes.first() == 1.E16) {
				HatchPatternLine.m_dashes.clear();
			}
			hatchPattern.append(HatchPatternLine);
		}		
	}
}
void EoDbHatch::SetHatchOrigin(const OdGePoint3d& origin) {
	m_HatchOrigin = origin;
}
void EoDbHatch::SetHatchXAxis(const OdGeVector3d& xAxis) {
	m_HatchXAxis = xAxis;
}
void EoDbHatch::SetHatchYAxis(const OdGeVector3d& yAxis) {
	m_HatchYAxis = yAxis;
}
void EoDbHatch::SetHatRefVecs(double patternAngle, double patternScaleX, double patternScaleY) {
	m_HatchXAxis = OdGeVector3d(m_Vertices[1] - m_Vertices[0]);
	m_HatchYAxis = OdGeVector3d(m_Vertices[2] - m_Vertices[0]);

	OdGeVector3d PlaneNormal = m_HatchXAxis.crossProduct(m_HatchYAxis);
	PlaneNormal.normalize();

	if (PlaneNormal.z < 0) {
		PlaneNormal = - PlaneNormal;
	}
	m_HatchXAxis.normalize();
	m_HatchXAxis.rotateBy(patternAngle, PlaneNormal);
	m_HatchYAxis = m_HatchXAxis;
	m_HatchYAxis.rotateBy(HALF_PI, PlaneNormal);
	m_HatchXAxis *= patternScaleX;
	m_HatchYAxis *= patternScaleY;
}
void EoDbHatch::SetInteriorStyle(EoInt16 interiorStyle) {
	m_InteriorStyle = interiorStyle;
}
void EoDbHatch::SetInteriorStyleIndex(size_t styleIndex) {
	if (!m_EntityObjectId.isNull()) {
		OdDbHatchPtr Hatch = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		
		OdHatchPatternManager* Manager = theApp.patternManager();
		
		OdString HatchName = m_InteriorStyle == kSolid ? L"SOLID" : EoDbHatch::sm_HatchNames[styleIndex];
		OdHatchPattern HatchPattern;
				
		if (Manager->retrievePattern(Hatch->patternType(), HatchName, OdDb::kEnglish, HatchPattern) != OdResult::eOk) {
			OdString ReportItem;
			ReportItem.format(L"Hatch pattern not defined for %s (%s)\n", (LPCWSTR) HatchName, (LPCWSTR) Hatch->patternName());
			theApp.AddStringToReportList(ReportItem);
		}
		else {
			Hatch->setPattern(OdDbHatch::kPreDefined, HatchName);
		}
	}
	m_InteriorStyleIndex = styleIndex;
}
void EoDbHatch::SetLoopAt(int loopIndex, OdDbHatchPtr& hatchEntity) {
	hatchEntity->getLoopAt(loopIndex, m_Vertices2d, m_Bulges);
	for (size_t VertexIndex = 0; VertexIndex < m_Vertices2d.size(); VertexIndex++) {
		m_Vertices.append(OdGePoint3d(m_Vertices2d[VertexIndex].x, m_Vertices2d[VertexIndex].y, hatchEntity->elevation()));
	}
}
void EoDbHatch::SetPatternReferenceSystem(const OdGePoint3d& origin, const OdGeVector3d& normal, double patternAngle, double patternScale) {
	m_HatchOrigin = origin;
	m_HatchXAxis = ComputeArbitraryAxis(normal);
	m_HatchXAxis.rotateBy(patternAngle, normal);
	m_HatchYAxis = normal.crossProduct(m_HatchXAxis);
	m_HatchXAxis *= patternScale;
	m_HatchYAxis *= patternScale;
}
void EoDbHatch::SetVertices(const OdGePoint3dArray& vertices) {
	if (!m_EntityObjectId.isNull()) {
		OdDbHatchPtr Hatch = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		OdGePoint2dArray Vertices;
		Vertices.clear();
		OdGeDoubleArray Bulges;
		Bulges.clear();
		for (size_t VertexIndex = 0; VertexIndex < vertices.size(); VertexIndex++) {
			Vertices.append(vertices[VertexIndex].convert2d());
			Bulges.append(0.);
		}
		Hatch->appendLoop(OdDbHatch::kPolyline, Vertices, Bulges);
	}
	m_Vertices.clear();
	m_Vertices.append(vertices);
}
size_t EoDbHatch::SwingVertex() const {
	const size_t NumberOfVertices = m_Vertices.size();
	size_t SwingVertex;

	if (sm_PivotVertex == 0) {
		SwingVertex = (sm_Edge == 1) ? 1 : NumberOfVertices - 1;
	}
	else if (sm_PivotVertex == NumberOfVertices - 1) {
		SwingVertex = (sm_Edge == NumberOfVertices) ? 0 : sm_PivotVertex - 1;
	}
	else {
		SwingVertex = (sm_Edge == sm_PivotVertex) ? sm_PivotVertex - 1 : sm_PivotVertex + 1;
	}
	return (SwingVertex);
}
// Methods - static
EoDbHatch* EoDbHatch::ConstructFrom(EoDbFile& file) {
	EoDbHatch* HatchPrimitive = new EoDbHatch();
	HatchPrimitive->SetColorIndex(file.ReadInt16());
	HatchPrimitive->SetInteriorStyle(file.ReadInt16());
	HatchPrimitive->SetInteriorStyleIndex(file.ReadInt16());
	const EoUInt16 NumberOfVertices = file.ReadUInt16();
	HatchPrimitive->SetHatchOrigin(file.ReadPoint3d());
	HatchPrimitive->SetHatchXAxis(file.ReadVector3d());
	HatchPrimitive->SetHatchYAxis(file.ReadVector3d());
	
	OdGePoint3dArray Vertices;
	Vertices.setLogicalLength(NumberOfVertices);
	for (size_t VertexIndex = 0; VertexIndex < NumberOfVertices; VertexIndex++) {
		Vertices[VertexIndex] = file.ReadPoint3d();
	}
	HatchPrimitive->SetVertices(Vertices);

	return (HatchPrimitive);
}
EoDbHatch* EoDbHatch::ConstructFrom(EoByte* primitiveBuffer, int versionNumber) {
	EoInt16 ColorIndex;
	EoInt16 InteriorStyle;
	size_t InteriorStyleIndex = 0;
	OdGePoint3d HatchOrigin;
	OdGeVector3d HatchXAxis;
	OdGeVector3d HatchYAxis;
	OdGePoint3dArray Vertices;

	if (versionNumber == 1) {
		ColorIndex = EoInt16(primitiveBuffer[4] & 0x000f);

		const double StyleDefinition = ((EoVaxFloat*) &primitiveBuffer[12])->Convert();
		InteriorStyle = EoInt16(int(StyleDefinition) % 16);

		switch (InteriorStyle) {
		case EoDbHatch::kHatch: {
			const double ScaleFactorX = ((EoVaxFloat*) &primitiveBuffer[16])->Convert();
			const double ScaleFactorY = ((EoVaxFloat*) &primitiveBuffer[20])->Convert();
			double PatternAngle = ((EoVaxFloat*) &primitiveBuffer[24])->Convert();

			if (fabs(ScaleFactorX) > FLT_EPSILON && fabs(ScaleFactorY) > FLT_EPSILON) { // Have 2 hatch lines
				InteriorStyleIndex = 2;
				HatchXAxis = OdGeVector3d(cos(PatternAngle), sin(PatternAngle), 0.);
				HatchYAxis = OdGeVector3d(- sin(PatternAngle), cos(PatternAngle), 0.);
				HatchXAxis *= ScaleFactorX * 1.e-3;
				HatchYAxis *= ScaleFactorY * 1.e-3;
			}
			else if (fabs(ScaleFactorX) > FLT_EPSILON) { // Vertical hatch lines
				InteriorStyleIndex = 1;
				PatternAngle += HALF_PI;

				HatchXAxis = OdGeVector3d(cos(PatternAngle), sin(PatternAngle), 0.);
				HatchYAxis = OdGeVector3d(- sin(PatternAngle), cos(PatternAngle), 0.);
				HatchXAxis *= ScaleFactorX * 1.e-3;
			}
			else { // Horizontal hatch lines
				InteriorStyleIndex = 1;
				HatchXAxis = OdGeVector3d(cos(PatternAngle), sin(PatternAngle), 0.);
				HatchYAxis = OdGeVector3d(- sin(PatternAngle), cos(PatternAngle), 0.);
				HatchYAxis *= ScaleFactorY * 1.e-3;
			}
			break;
		}
		case EoDbHatch::kHollow:
		case EoDbHatch::kSolid:
		case EoDbHatch::kPattern:
			HatchXAxis = OdGeVector3d::kXAxis * 1.e-3;
			HatchYAxis = OdGeVector3d::kYAxis * 1.e-3;
			break;

		default:
			throw L"Exception.FileJob: Unknown hatch primitive interior style.";
		}
		const size_t NumberOfVertices = EoUInt16(((EoVaxFloat*) &primitiveBuffer[8])->Convert());
		
		int BufferOffset = 36;
		Vertices.clear();
		for (size_t VertexIndex = 0; VertexIndex < NumberOfVertices; VertexIndex++) {
			Vertices.append(((EoVaxPoint3d*) &primitiveBuffer[BufferOffset])->Convert() * 1.e-3);
			BufferOffset += sizeof(EoVaxPoint3d);
		}
		HatchOrigin = Vertices[0];
	}
	else {
		ColorIndex = EoInt16(primitiveBuffer[6]);
		InteriorStyle = EoSbyte(primitiveBuffer[7]);
		InteriorStyleIndex = *((EoInt16*) &primitiveBuffer[8]);
		const size_t NumberOfVertices = *((EoInt16*) &primitiveBuffer[10]);
		HatchOrigin = ((EoVaxPoint3d*) &primitiveBuffer[12])->Convert();
		HatchXAxis = ((EoVaxVector3d*) &primitiveBuffer[24])->Convert();
		HatchYAxis = ((EoVaxVector3d*) &primitiveBuffer[36])->Convert();
		
		int BufferOffset = 48;
		Vertices.clear();
		for (size_t VertexIndex = 0; VertexIndex < NumberOfVertices; VertexIndex++) {
			Vertices.append(((EoVaxPoint3d*) &primitiveBuffer[BufferOffset])->Convert());
			BufferOffset += sizeof(EoVaxPoint3d);
		}
	}
	EoDbHatch* HatchPrimitive = new EoDbHatch();
	HatchPrimitive->SetColorIndex(ColorIndex);
	HatchPrimitive->SetInteriorStyle(InteriorStyle);
	HatchPrimitive->SetInteriorStyleIndex(InteriorStyleIndex);
	HatchPrimitive->SetHatchOrigin(HatchOrigin);
	HatchPrimitive->SetHatchXAxis(HatchXAxis);
	HatchPrimitive->SetHatchYAxis(HatchYAxis);

	HatchPrimitive->SetVertices(Vertices);

	return (HatchPrimitive);
}
EoDbHatch* EoDbHatch::Create(OdDbDatabasePtr database) {
	OdDbBlockTableRecordPtr BlockTableRecord = database->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

	OdDbHatchPtr HatchEntity = OdDbHatch::createObject();
	HatchEntity->setDatabaseDefaults(database);
	HatchEntity->setAssociative(false);
	OdString HatchName = EoDbHatch::sm_HatchNames[pstate.HatchInteriorStyleIndex()];
	HatchEntity->setPattern(OdDbHatch::kPreDefined, HatchName);
	
	BlockTableRecord->appendOdDbEntity(HatchEntity);
	
	EoDbHatch* HatchPrimitive = new EoDbHatch();
	HatchPrimitive->SetEntityObjectId(HatchEntity->objectId());
	
	HatchPrimitive->SetColorIndex(pstate.ColorIndex());
	HatchPrimitive->SetInteriorStyleIndex(pstate.HatchInteriorStyleIndex());

	return HatchPrimitive;
}
EoDbHatch* EoDbHatch::Create(const EoDbHatch& other, OdDbDatabasePtr database) {
	OdDbBlockTableRecordPtr BlockTableRecord = database->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
	OdDbHatchPtr HatchEntity = other.EntityObjectId().safeOpenObject()->clone();
	BlockTableRecord->appendOdDbEntity(HatchEntity);

	EoDbHatch* Hatch = new EoDbHatch(other);
	Hatch->SetEntityObjectId(HatchEntity->objectId());

	return Hatch;
}

size_t EoDbHatch::Edge() {
	return sm_Edge;
}
size_t EoDbHatch::LegacyInteriorStyleIndex(const OdString& name) {
	size_t Index = 0;
	const size_t NumberOfLegacyStyles = EoDbHatch::sm_HatchNames.size();

	while (Index < NumberOfLegacyStyles && name.iCompare(EoDbHatch::sm_HatchNames[Index]) != 0) {
		Index++;
	}
	Index = (Index < NumberOfLegacyStyles) ? Index : 0;
	return Index;
}
void EoDbHatch::SetEdgeToEvaluate(size_t edgeToEvaluate) {
	sm_EdgeToEvaluate = edgeToEvaluate;
}
