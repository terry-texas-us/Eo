#include "stdafx.h"
#include <Ge/GeCircArc2d.h>
#include <Ge/GeEllipArc2d.h>
#include <Ge/GeNurbCurve2d.h>
#include <HatchPatternManager.h>
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "PrimState.h"
#include "EoVaxFloat.h"
#include "EoGePolyline.h"
#include "EoDbFile.h"
#include "EoDbHatch.h"
#include "EoDbHatchPatternTable.h"
IMPLEMENT_DYNAMIC(EoDbHatch, EoDbPrimitive)

unsigned EoDbHatch::ms_EdgeToEvaluate = 0;
unsigned EoDbHatch::ms_Edge = 0;
unsigned EoDbHatch::ms_PivotVertex = 0;
double EoDbHatch::patternAngle = 0.0;
double EoDbHatch::patternScaleX = 0.1;
double EoDbHatch::patternScaleY = 0.1;

struct EoEdge {
	double minimumExtentY {0.0}; // minimum y extent of edge
	double maximumExtentY {0.0}; // maximum y extent of edge
	double intersectionX {0.0}; // x intersection on edge
	union {
		double inverseSlope {0.0}; // inverse slope of edge
		double stepSize; // change in x for each scanline
	};
};

EoDbHatch::EoDbHatch() noexcept
	: m_Vertices(0) {}

EoDbHatch::EoDbHatch(const EoDbHatch& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;
	m_ColorIndex = other.m_ColorIndex;
	m_InteriorStyle = other.m_InteriorStyle;
	m_InteriorStyleIndex = other.m_InteriorStyleIndex;
	m_HatchOrigin = other.m_HatchOrigin;
	m_HatchXAxis = other.m_HatchXAxis;
	m_HatchYAxis = other.m_HatchYAxis;
	m_NumberOfLoops = 0;
	m_Vertices.clear();
	m_Vertices.append(other.m_Vertices);
}

EoDbHatch& EoDbHatch::operator=(const EoDbHatch& other) {
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
	return *this;
}

void EoDbHatch::AddReportToMessageList(const OdGePoint3d& point) const {
	const auto NumberOfVertices {m_Vertices.size()};
	if (ms_Edge > 0 && ms_Edge <= NumberOfVertices) {
		auto StartPoint {m_Vertices[ms_Edge - 1]};
		auto EndPoint {m_Vertices[ms_Edge % NumberOfVertices]};
		if (ms_PivotVertex < NumberOfVertices) {
			StartPoint = m_Vertices[ms_PivotVertex];
			EndPoint = m_Vertices[SwingVertex()];
		}
		double AngleInXYPlane;
		const auto Length {OdGeVector3d(EndPoint - StartPoint).length()};
		if (OdGeVector3d(StartPoint - point).length() > Length * 0.5) {
			AngleInXYPlane = EoGeLineSeg3d(EndPoint, StartPoint).AngleFromXAxis_xy();
		} else {
			AngleInXYPlane = EoGeLineSeg3d(StartPoint, EndPoint).AngleFromXAxis_xy();
		}
		CString Report(L"<Hatch-Edge> ");
		Report += L" Color:" + FormatColorIndex();
		Report += L" [" + theApp.FormatLength(Length, theApp.GetUnits()) + L" @ " + AeSys::FormatAngle(AngleInXYPlane) + L"]";
		AeSys::AddStringToMessageList(Report);
		theApp.SetEngagedLength(Length);
		theApp.SetEngagedAngle(AngleInXYPlane);
	}
}

void EoDbHatch::AddToTreeViewControl(const HWND tree, const HTREEITEM parent) const noexcept {
	CMainFrame::InsertTreeViewControlItem(tree, parent, L"<Hatch>", this);
}

EoDbPrimitive* EoDbHatch::Clone(OdDbBlockTableRecordPtr blockTableRecord) const {
	OdDbHatchPtr Hatch {m_EntityObjectId.safeOpenObject()->clone()};
	blockTableRecord->appendOdDbEntity(Hatch);
	return Create(Hatch);
}

void EoDbHatch::Display(AeSysView* view, CDC* deviceContext) {
	const auto ColorIndex {LogicalColorIndex()};
	g_PrimitiveState.SetColorIndex(deviceContext, ColorIndex);
	g_PrimitiveState.SetHatchInteriorStyle(m_InteriorStyle);
	g_PrimitiveState.SetHatchInteriorStyleIndex(m_InteriorStyleIndex);
	if (m_InteriorStyle == kHatch) {
		DisplayHatch(view, deviceContext);
	} else { // Fill area interior style is hollow, solid or pattern
		DisplaySolid(view, deviceContext);
	}
}

void EoDbHatch::FormatExtra(CString& extra) const {
	extra.Empty();
	extra += L"Color;" + FormatColorIndex() + L"\t";
	extra += L"Interior Style;" + FormatInteriorStyle() + L"\t";
	extra += L"Interior Style Name;" + CString(static_cast<const wchar_t*>(EoDbHatchPatternTable::LegacyHatchPatternName(m_InteriorStyleIndex))) + L"\t";
	CString NumberOfVertices;
	NumberOfVertices.Format(L"Number of Vertices;%d\t", m_Vertices.size());
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
	for (const auto& Vertex : m_Vertices) {
		VertexString.Format(L"Vertex;%f;%f;%f\t", Vertex.x, Vertex.y, Vertex.z);
		geometry += VertexString;
	}
}

void EoDbHatch::GetAllPoints(OdGePoint3dArray& points) const {
	points.clear();
	for (const auto& Vertex : m_Vertices) {
		points.append(Vertex);
	}
}

OdGePoint3d EoDbHatch::GetCtrlPt() const {
	const auto StartPointIndex = ms_Edge - 1;
	const auto EndPointIndex = ms_Edge % m_Vertices.size();
	return EoGeLineSeg3d(m_Vertices[StartPointIndex], m_Vertices[EndPointIndex]).midPoint();
}

void EoDbHatch::GetExtents(AeSysView* /*view*/, OdGeExtents3d& extents) const {
	for (const auto& Vertex : m_Vertices) {
		extents.addPoint(Vertex);
	}
}

OdGePoint3d EoDbHatch::GoToNxtCtrlPt() const {
	const auto NumberOfVertices {m_Vertices.size()};
	if (ms_PivotVertex >= NumberOfVertices) { // have not yet rocked to a vertex
		const auto StartVertexIndex {ms_Edge - 1};
		const auto StartPoint(m_Vertices[StartVertexIndex]);
		const auto EndVertexIndex {ms_Edge % NumberOfVertices};
		const auto EndPoint(m_Vertices[EndVertexIndex]);
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
	return m_Vertices[ms_PivotVertex];
}

bool EoDbHatch::IsInView(AeSysView* view) const {
	EoGePoint4d pt[2];
	pt[0] = EoGePoint4d(m_Vertices[0], 1.0);
	view->ModelViewTransformPoint(pt[0]);
	for (auto i = m_Vertices.size() - 1; i > 0; i--) {
		pt[1] = EoGePoint4d(m_Vertices[i], 1.0);
		view->ModelViewTransformPoint(pt[1]);
		if (EoGePoint4d::ClipLine(pt[0], pt[1])) {
			return true;
		}
		pt[0] = pt[1];
	}
	return false;
}

bool EoDbHatch::IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	for (const auto& Vertex : m_Vertices) {
		EoGePoint4d Point(Vertex, 1.0);
		view->ModelViewTransformPoint(Point);
		if (point.DistanceToPointXY(Point) < ms_SelectApertureSize) {
			return true;
		}
	}
	return false;
}

OdGePoint3d EoDbHatch::SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	ms_ControlPointIndex = SIZE_T_MAX;
	auto Aperture {ms_SelectApertureSize};
	ms_PivotVertex = m_Vertices.size();
	for (unsigned VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		EoGePoint4d pt(m_Vertices[VertexIndex], 1.0);
		view->ModelViewTransformPoint(pt);
		const auto dDis {point.DistanceToPointXY(pt)};
		if (dDis < Aperture) {
			ms_ControlPointIndex = VertexIndex;
			Aperture = dDis;
			ms_Edge = VertexIndex + 1;
			ms_PivotVertex = VertexIndex;
		}
	}
	return ms_ControlPointIndex == SIZE_T_MAX ? OdGePoint3d::kOrigin : m_Vertices[ms_ControlPointIndex];
}

bool EoDbHatch::SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const {
	OdGePoint3dArray Points;
	for (const auto& Vertex : m_Vertices) {
		Points.append(Vertex);
	}
	return polyline::SelectUsingRectangle(view, lowerLeftCorner, upperRightCorner, Points);
}

bool EoDbHatch::SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& projectedPoint) const {
	const auto NumberOfVertices {m_Vertices.size()};
	if (ms_EdgeToEvaluate > 0 && ms_EdgeToEvaluate <= NumberOfVertices) { // Evaluate specified edge of polygon
		EoGePoint4d ptBeg(m_Vertices[ms_EdgeToEvaluate - 1], 1.0);
		EoGePoint4d ptEnd(m_Vertices[ms_EdgeToEvaluate % NumberOfVertices], 1.0);
		view->ModelViewTransformPoint(ptBeg);
		view->ModelViewTransformPoint(ptEnd);
		const EoGeLineSeg3d Edge(ptBeg.Convert3d(), ptEnd.Convert3d());
		if (Edge.IsSelectedBy_xy(point.Convert3d(), view->SelectApertureSize(), projectedPoint, ms_RelationshipOfPoint)) {
			projectedPoint.z = ptBeg.z + ms_RelationshipOfPoint * (ptEnd.z - ptBeg.z);
			return true;
		}
	} else { // Evaluate entire polygon
		EoGePoint4d ptBeg(m_Vertices[0], 1.0);
		view->ModelViewTransformPoint(ptBeg);
		for (unsigned VertexIndex = 1; VertexIndex <= NumberOfVertices; VertexIndex++) {
			EoGePoint4d ptEnd(m_Vertices[VertexIndex % NumberOfVertices], 1.0);
			view->ModelViewTransformPoint(ptEnd);
			EoGeLineSeg3d Edge(ptBeg.Convert3d(), ptEnd.Convert3d());
			if (Edge.IsSelectedBy_xy(point.Convert3d(), view->SelectApertureSize(), projectedPoint, ms_RelationshipOfPoint)) {
				projectedPoint.z = ptBeg.z + ms_RelationshipOfPoint * (ptEnd.z - ptBeg.z);
				ms_Edge = VertexIndex;
				ms_PivotVertex = NumberOfVertices;
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
	for (auto& Vertex : m_Vertices) {
		Vertex.transformBy(transformMatrix);
	}
}

void EoDbHatch::TranslateUsingMask(const OdGeVector3d& translate, const unsigned mask) {
	// nothing done to hatch coordinate origin
	for (unsigned VertexIndex = 0; VertexIndex < m_Vertices.size(); VertexIndex++) {
		if ((mask >> VertexIndex & 1UL) == 1) {
			m_Vertices[VertexIndex] += translate;
		}
	}
}

bool EoDbHatch::Write(EoDbFile& file) const {
	file.WriteUInt16(EoDb::kHatchPrimitive);
	file.WriteInt16(m_ColorIndex);
	file.WriteInt16(m_InteriorStyle);  // note polygon style stuffed up into unused line type on io
	file.WriteUInt16(static_cast<unsigned short>(EoMax(1U, m_InteriorStyleIndex)));
	file.WriteUInt16(static_cast<unsigned short>(m_Vertices.size()));
	file.WritePoint3d(m_HatchOrigin);
	file.WriteDouble(m_HatchXAxis.x);
	file.WriteDouble(m_HatchXAxis.y);
	file.WriteDouble(m_HatchXAxis.z);
	file.WriteDouble(m_HatchYAxis.x);
	file.WriteDouble(m_HatchYAxis.y);
	file.WriteDouble(m_HatchYAxis.z);
	for (const auto& Vertex : m_Vertices) {
		file.WritePoint3d(Vertex);
	}
	return true;
}

void EoDbHatch::Write(CFile& file, unsigned char* buffer) const {
	buffer[3] = static_cast<unsigned char>((79 + m_Vertices.size() * 12) / 32);
	*reinterpret_cast<unsigned short*>(& buffer[4]) = static_cast<unsigned short>(EoDb::kHatchPrimitive);
	buffer[6] = static_cast<unsigned char>(m_ColorIndex == mc_ColorindexBylayer ? ms_LayerColorIndex : m_ColorIndex);
	buffer[7] = static_cast<unsigned char>(m_InteriorStyle);
	*reinterpret_cast<short*>(& buffer[8]) = static_cast<short>(m_InteriorStyleIndex);
	*reinterpret_cast<short*>(& buffer[10]) = static_cast<short>(m_Vertices.size());
	reinterpret_cast<EoVaxPoint3d*>(& buffer[12])->Convert(m_HatchOrigin);
	reinterpret_cast<EoVaxVector3d*>(& buffer[24])->Convert(m_HatchXAxis);
	reinterpret_cast<EoVaxVector3d*>(& buffer[36])->Convert(m_HatchYAxis);
	auto i {48};
	for (const auto& Vertex : m_Vertices) {
		reinterpret_cast<EoVaxPoint3d*>(& buffer[i])->Convert(Vertex);
		i += sizeof(EoVaxPoint3d);
	}
	file.Write(buffer, static_cast<unsigned>(buffer[3] * 32));
}

int EoDbHatch::Append(const OdGePoint3d& vertex) {
	return static_cast<int>(m_Vertices.append(vertex));
}

void EoDbHatch::DisplayHatch(AeSysView* view, CDC* deviceContext) const {
	EoGeMatrix3d tm;
	tm.setToWorldToPlane(OdGePlane(m_HatchOrigin, m_HatchXAxis, m_HatchYAxis));
	const auto NumberOfLoops {1};
	int LoopPointsOffsets[2];
	LoopPointsOffsets[0] = static_cast<int>(m_Vertices.size());
	EoEdge Edges[128];
	const auto ColorIndex {g_PrimitiveState.ColorIndex()};
	const auto LinetypeIndex {g_PrimitiveState.LinetypeIndex()};
	g_PrimitiveState.SetLinetypeIndexPs(deviceContext, 1);
	const auto InteriorStyleIndex {g_PrimitiveState.HatchInteriorStyleIndex()};
	OdHatchPattern HatchPattern;
	EoDbHatchPatternTable::RetrieveHatchPattern(EoDbHatchPatternTable::LegacyHatchPatternName(InteriorStyleIndex), HatchPattern);
	const auto NumberOfPatterns = HatchPattern.size();
	OdHatchPatternLine HatchPatternLine;
	for (unsigned PatternIndex = 0; PatternIndex < NumberOfPatterns; PatternIndex++) {
		HatchPatternLine = HatchPattern.getAt(PatternIndex);
		const auto NumberOfDashesInPattern {HatchPatternLine.m_dashes.size()};
		double TotalPatternLength {0};
		for (unsigned DashIndex = 0; DashIndex < NumberOfDashesInPattern; DashIndex++) {
			TotalPatternLength += fabs(HatchPatternLine.m_dashes[DashIndex]);
		}
		auto RotatedBasePoint {HatchPatternLine.m_basePoint};
		const auto LineAngleInRadians {HatchPatternLine.m_dLineAngle};
		RotatedBasePoint.rotateBy(-LineAngleInRadians);

		// Add rotation to matrix which gets current scan lines parallel to x-axis
		EoGeMatrix3d tmRotZ;
		tmRotZ.setToRotation(-LineAngleInRadians, OdGeVector3d::kZAxis);
		tm.preMultBy(tmRotZ);
		auto tmInv {tm};
		tmInv.invert();
		auto ActiveEdges {0};
		auto FirstLoopPointIndex {0};
		for (auto LoopIndex = 0; LoopIndex < NumberOfLoops; LoopIndex++) {
			if (LoopIndex != 0) {
				FirstLoopPointIndex = LoopPointsOffsets[LoopIndex - 1];
			}
			auto StartPoint(m_Vertices[static_cast<unsigned>(FirstLoopPointIndex)]);
			StartPoint.transformBy(tm);		// Apply transform to get areas first point in z0 plane
			const auto SizeOfCurrentLoop {LoopPointsOffsets[LoopIndex] - FirstLoopPointIndex};
			for (auto LoopPointIndex = FirstLoopPointIndex; LoopPointIndex < LoopPointsOffsets[LoopIndex]; LoopPointIndex++) {
				auto EndPoint(m_Vertices[static_cast<unsigned>((LoopPointIndex - FirstLoopPointIndex + 1) % SizeOfCurrentLoop + FirstLoopPointIndex)]);
				EndPoint.transformBy(tm);
				const OdGeVector2d Edge(EndPoint.x - StartPoint.x, EndPoint.y - StartPoint.y);
				if (!Edge.isZeroLength() && !Edge.isParallelTo(OdGeVector2d::kXAxis)) {
					const auto dMaxY {EoMax(StartPoint.y, EndPoint.y)};
					auto CurrentEdgeIndex {ActiveEdges + 1};
					// Find correct insertion point for edge in edge list using ymax as sort key
					while (CurrentEdgeIndex != 1 && Edges[CurrentEdgeIndex - 1].maximumExtentY < dMaxY) {
						Edges[CurrentEdgeIndex] = Edges[CurrentEdgeIndex - 1];		// Move entry down
						CurrentEdgeIndex--;
					}
					// Insert information about new edge
					Edges[CurrentEdgeIndex].maximumExtentY = dMaxY;
					Edges[CurrentEdgeIndex].inverseSlope = Edge.x / Edge.y;
					if (StartPoint.y > EndPoint.y) {
						Edges[CurrentEdgeIndex].minimumExtentY = EndPoint.y;
						Edges[CurrentEdgeIndex].intersectionX = StartPoint.x;
					} else {
						Edges[CurrentEdgeIndex].minimumExtentY = StartPoint.y;
						Edges[CurrentEdgeIndex].intersectionX = EndPoint.x;
					}
					ActiveEdges++;
				}
				StartPoint = EndPoint;
			}
		}
		auto PatternOffset(HatchPatternLine.m_patternOffset);
		if (PatternOffset.y < 0.0) {
			PatternOffset.negate();
		}
		// Determine where first scan position is
		auto dScan {Edges[1].maximumExtentY - fmod(Edges[1].maximumExtentY - RotatedBasePoint.y, PatternOffset.y)};
		if (Edges[1].maximumExtentY < dScan) {
			dScan = dScan - PatternOffset.y;
		}
		auto dSecBeg {RotatedBasePoint.x + PatternOffset.x * (dScan - RotatedBasePoint.y) / PatternOffset.y};
		// Edge list pointers
		auto iBegEdg {1};
		auto iEndEdg {1};
		// Determine relative epsilon to be used for extent tests
	l1: const auto dEps1 {DBL_EPSILON + DBL_EPSILON * fabs(dScan)};
		while (iEndEdg <= ActiveEdges && Edges[iEndEdg].maximumExtentY >= dScan - dEps1) {
			// Set x intersection back to last scanline
			Edges[iEndEdg].intersectionX += Edges[iEndEdg].inverseSlope * (PatternOffset.y + dScan - Edges[iEndEdg].maximumExtentY);
			// Determine the change in x per scan
			Edges[iEndEdg].stepSize = -Edges[iEndEdg].inverseSlope * PatternOffset.y;
			iEndEdg++;
		}
		for (auto i = iBegEdg; i < iEndEdg; i++) {
			auto CurrentEdgeIndex {i};
			if (Edges[i].minimumExtentY < dScan - dEps1) { // Edge y-extent overlaps current scan . determine intersections
				Edges[i].intersectionX += Edges[i].stepSize;
				while (CurrentEdgeIndex > iBegEdg && Edges[CurrentEdgeIndex].intersectionX < Edges[CurrentEdgeIndex - 1].intersectionX) {
					Edges[0] = Edges[CurrentEdgeIndex];
					Edges[CurrentEdgeIndex] = Edges[CurrentEdgeIndex - 1];
					Edges[CurrentEdgeIndex - 1] = Edges[0];
					CurrentEdgeIndex--;
				}
			} else { // Edge y-extent does not overlap current scan. remove edge from active edge list
				iBegEdg++;
				while (CurrentEdgeIndex >= iBegEdg) {
					Edges[CurrentEdgeIndex] = Edges[CurrentEdgeIndex - 1];
					CurrentEdgeIndex--;
				}
			}
		}
		if (iEndEdg != iBegEdg) { // At least one pair of edge intersections .. generate pattern lines for each pair
			auto CurrentEdgeIndex {iBegEdg};
			if (HatchPatternLine.m_dashes.isEmpty()) {
				for (auto EdgePairIndex = 1; EdgePairIndex <= (iEndEdg - iBegEdg) / 2; EdgePairIndex++) {
					const OdGePoint3d StartPoint(Edges[CurrentEdgeIndex].intersectionX, dScan, 0.0);
					const OdGePoint3d EndPoint(Edges[CurrentEdgeIndex + 1].intersectionX, dScan, 0.0);
					if (!StartPoint.isEqualTo(EndPoint)) {
						EoGeLineSeg3d Line(StartPoint, EndPoint);
						Line.transformBy(tmInv);
						Line.Display(view, deviceContext);
					}
					CurrentEdgeIndex = CurrentEdgeIndex + 2;
				}
			} else {
				OdGePoint3d StartPoint;
				OdGePoint3d EndPoint;
				StartPoint.y = dScan;
				EndPoint.y = dScan;
				for (auto EdgePairIndex = 1; EdgePairIndex <= (iEndEdg - iBegEdg) / 2; EdgePairIndex++) {
					StartPoint.x = Edges[CurrentEdgeIndex].intersectionX - fmod(Edges[CurrentEdgeIndex].intersectionX - dSecBeg, TotalPatternLength);
					if (StartPoint.x > Edges[CurrentEdgeIndex].intersectionX) {
						StartPoint.x -= TotalPatternLength;
					}
					// Determine the index of the pattern item which intersects the left edge and how much of it is between the edges
					auto DashIndex = 0U;
					auto DistanceToLeftEdge {Edges[CurrentEdgeIndex].intersectionX - StartPoint.x};
					auto CurrentDashLength {fabs(HatchPatternLine.m_dashes[DashIndex])};
					while (CurrentDashLength <= DistanceToLeftEdge + DBL_EPSILON) {
						StartPoint.x += CurrentDashLength;
						DistanceToLeftEdge -= CurrentDashLength;
						DashIndex = (DashIndex + 1) % NumberOfDashesInPattern;
						CurrentDashLength = fabs(HatchPatternLine.m_dashes[DashIndex]);
					}
					StartPoint.x = Edges[CurrentEdgeIndex].intersectionX;
					CurrentDashLength -= DistanceToLeftEdge;
					auto DistanceToRightEdge {Edges[CurrentEdgeIndex + 1].intersectionX - Edges[CurrentEdgeIndex].intersectionX};
					while (CurrentDashLength <= DistanceToRightEdge + DBL_EPSILON) {
						EndPoint.x = StartPoint.x + CurrentDashLength;
						if (HatchPatternLine.m_dashes[DashIndex] >= 0.0) {
							if (HatchPatternLine.m_dashes[DashIndex] == 0.0) {
								auto Dot(StartPoint);
								Dot.transformBy(tmInv);
								view->DisplayPixel(deviceContext, AeSys::GetHotColor(ColorIndex), Dot);
							} else {
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
					if (HatchPatternLine.m_dashes[DashIndex] >= 0.0) {
						EndPoint.x = Edges[CurrentEdgeIndex + 1].intersectionX;
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
		tmRotZ.setToRotation(LineAngleInRadians, OdGeVector3d::kZAxis);
		tm.preMultBy(tmRotZ);
	}
	g_PrimitiveState.SetPen(view, deviceContext, ColorIndex, LinetypeIndex);
}

void EoDbHatch::DisplaySolid(AeSysView* view, CDC* deviceContext) const {
	const auto NumberOfVertices {static_cast<int>(m_Vertices.size())};
	if (NumberOfVertices >= 2) {
		EoGePoint4dArray Vertices;
		Vertices.SetSize(NumberOfVertices);
		for (auto VertexIndex = 0; VertexIndex < NumberOfVertices; VertexIndex++) {
			Vertices[VertexIndex] = EoGePoint4d(m_Vertices[static_cast<unsigned>(VertexIndex)], 1.0);
		}
		view->ModelViewTransformPoints(Vertices);
		EoGePoint4d::ClipPolygon(Vertices);
		const auto NumberOfPoints {Vertices.GetSize()};
		const auto Points {new CPoint[static_cast<unsigned>(NumberOfPoints)]};
		view->DoViewportProjection(Points, Vertices);
		if (m_InteriorStyle == kSolid) {
			CBrush Brush(g_CurrentPalette[g_PrimitiveState.ColorIndex()]);
			const auto OldBrush {deviceContext->SelectObject(&Brush)};
			deviceContext->Polygon(Points, NumberOfPoints);
			deviceContext->SelectObject(OldBrush);
		} else if (m_InteriorStyle == kHollow) {
			const auto OldBrush {dynamic_cast<CBrush*>(deviceContext->SelectStockObject(NULL_BRUSH))};
			deviceContext->Polygon(Points, NumberOfPoints);
			deviceContext->SelectObject(OldBrush);
		} else {
			deviceContext->Polygon(Points, NumberOfPoints);
		}
		delete[] Points;
	}
}

CString EoDbHatch::FormatInteriorStyle() const {
	const wchar_t* strStyle[] = {L"Hollow", L"Solid", L"Pattern", L"Hatch"};
	CString str = m_InteriorStyle >= 0 && m_InteriorStyle <= 3 ? strStyle[m_InteriorStyle] : L"Invalid!";
	return str;
}

OdGePoint3d EoDbHatch::GetPointAt(const unsigned pointIndex) {
	return m_Vertices[pointIndex];
}

void EoDbHatch::ModifyState() noexcept {
	EoDbPrimitive::ModifyState();
	m_InteriorStyle = g_PrimitiveState.HatchInteriorStyle();
	m_InteriorStyleIndex = g_PrimitiveState.HatchInteriorStyleIndex();
}

int EoDbHatch::NumberOfVertices() const {
	return static_cast<int>(m_Vertices.size());
}

bool EoDbHatch::PivotOnGripPoint(AeSysView* view, const EoGePoint4d& point) noexcept {
	const auto NumberOfVertices = m_Vertices.size();
	if (ms_PivotVertex >= NumberOfVertices) { // Not engaged at a vertex
		return false;
	}
	EoGePoint4d ptCtrl(m_Vertices[ms_PivotVertex], 1.0);
	view->ModelViewTransformPoint(ptCtrl);
	if (ptCtrl.DistanceToPointXY(point) >= ms_SelectApertureSize) { // Not on proper vertex
		return false;
	}
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

OdGeVector3d EoDbHatch::RecomputeReferenceSystem() {
	const auto HatchXAxis(m_Vertices[1] - m_Vertices[0]);
	const auto HatchYAxis(m_Vertices[2] - m_Vertices[0]);
	auto PlaneNormal {HatchXAxis.crossProduct(HatchYAxis)};
	if (!PlaneNormal.isZeroLength()) {
		PlaneNormal.normalize();
		if (m_InteriorStyle != kHatch) {
			m_HatchXAxis = ComputeArbitraryAxis(PlaneNormal);
			m_HatchYAxis = PlaneNormal.crossProduct(m_HatchXAxis);
		}
	}
	return PlaneNormal;
}

bool EoDbHatch::SelectUsingLineSeg(const EoGeLineSeg3d& /*lineSeg*/, AeSysView* /*view*/, OdGePoint3dArray& /*intersections*/) {
	const CRuntimeClass* PrimitiveClass = GetRuntimeClass();
	theApp.AddStringToMessageList(L"Selection by line segment not implemented for <%s>\n", CString(PrimitiveClass->m_lpszClassName));
	return false;
}

void EoDbHatch::SetHatchOrigin(const OdGePoint3d& origin) noexcept {
	m_HatchOrigin = origin;
}

void EoDbHatch::SetHatchXAxis(const OdGeVector3d& xAxis) noexcept {
	m_HatchXAxis = xAxis;
}

void EoDbHatch::SetHatchYAxis(const OdGeVector3d& yAxis) noexcept {
	m_HatchYAxis = yAxis;
}

void EoDbHatch::SetHatchReferenceAxes(const double patternAngle, const double patternScaleX, const double patternScaleY) {
	m_HatchXAxis = OdGeVector3d(m_Vertices[1] - m_Vertices[0]);
	m_HatchYAxis = OdGeVector3d(m_Vertices[2] - m_Vertices[0]);
	auto PlaneNormal {m_HatchXAxis.crossProduct(m_HatchYAxis)};
	PlaneNormal.normalize();
	if (PlaneNormal.z < 0) {
		PlaneNormal = -PlaneNormal;
	}
	m_HatchXAxis.normalize();
	m_HatchXAxis.rotateBy(patternAngle, PlaneNormal);
	m_HatchYAxis = m_HatchXAxis;
	m_HatchYAxis.rotateBy(OdaPI2, PlaneNormal);
	m_HatchXAxis *= patternScaleX;
	m_HatchYAxis *= patternScaleY;
}

void EoDbHatch::SetInteriorStyle(const short interiorStyle) noexcept {
	m_InteriorStyle = interiorStyle;
}

void EoDbHatch::SetInteriorStyleIndex2(const unsigned styleIndex) {
	if (!m_EntityObjectId.isNull()) {
		OdDbHatchPtr Hatch {m_EntityObjectId.safeOpenObject(OdDb::kForWrite)};
		auto HatchPatternManager {theApp.patternManager()};
		const auto HatchName {m_InteriorStyle == kSolid ? OdString(L"SOLID") : EoDbHatchPatternTable::LegacyHatchPatternName(styleIndex)};
		OdHatchPattern HatchPattern;
		if (HatchPatternManager->retrievePattern(Hatch->patternType(), HatchName, OdDb::kEnglish, HatchPattern) != eOk) {
			OdString ReportItem;
			ReportItem.format(L"Hatch pattern not defined for %s (%s)\n", static_cast<const wchar_t*>(HatchName), static_cast<const wchar_t*>(Hatch->patternName()));
			AeSys::AddStringToReportList(ReportItem);
		} else {
			Hatch->setPattern(OdDbHatch::kPreDefined, HatchName);
		}
	}
	m_InteriorStyleIndex = styleIndex;
}

void EoDbHatch::SetLoopAt(const int loopIndex, const OdDbHatchPtr& hatchEntity) {
	hatchEntity->getLoopAt(loopIndex, m_Vertices2d, m_Bulges);
	OdGePlane Plane;
	OdDb::Planarity ResultPlanarity;
	const auto Result {hatchEntity->getPlane(Plane, ResultPlanarity)};
	auto PlaneToWorld {OdGeMatrix3d::kIdentity};
	if (Result == eOk && ResultPlanarity == OdDb::kPlanar) {
		PlaneToWorld = PlaneToWorld.setToPlaneToWorld(Plane);
	}
	for (auto& Vertex2d : m_Vertices2d) {
		auto Vertex {OdGePoint3d(Vertex2d.x, Vertex2d.y, hatchEntity->elevation())};
		Vertex.transformBy(PlaneToWorld);
		m_Vertices.append(Vertex);
	}
}

void EoDbHatch::SetPatternReferenceSystem(const OdGePoint3d& origin, const OdGeVector3d& normal, const double patternAngle, const double patternScale) {
	m_HatchOrigin = origin;
	m_HatchXAxis = ComputeArbitraryAxis(normal);
	m_HatchXAxis.rotateBy(patternAngle, normal);
	m_HatchYAxis = normal.crossProduct(m_HatchXAxis);
	m_HatchXAxis *= patternScale;
	m_HatchYAxis *= patternScale;
}

unsigned EoDbHatch::SwingVertex() const {
	const auto NumberOfVertices {m_Vertices.size()};
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

unsigned EoDbHatch::Edge() noexcept {
	return ms_Edge;
}

void EoDbHatch::SetEdgeToEvaluate(const unsigned edgeToEvaluate) noexcept {
	ms_EdgeToEvaluate = edgeToEvaluate;
}

void EoDbHatch::ConvertPolylineType(const int loopIndex, const OdDbHatchPtr& hatchEntity, EoDbHatch* hatchPrimitive) {
	hatchPrimitive->SetLoopAt(loopIndex, hatchEntity);
}

void EoDbHatch::ConvertCircularArcEdge(OdGeCurve2d* /*edge*/) noexcept {
	/* OdGeCircArc2d* CircularArcEdge = */
	
	// <tas="Properties: center, radius, startAng, endAng, isClockWise"></tas>
}

void EoDbHatch::ConvertEllipticalArcEdge(OdGeCurve2d* /*edge*/) noexcept {
	/* OdGeEllipArc2d* EllipticalArcEdge = */

	// <tas="Properties: center, majorRadius, minorRadius, majorAxis, minorAxis, startAng, endAng, isClockWise"></tas>
}

void EoDbHatch::ConvertNurbCurveEdge(OdGeCurve2d* /*edge*/) noexcept {
	/* OdGeNurbCurve2d* NurbCurveEdge = */

	// <tas="Properties: degree, isRational, isPeriodic, numKnots, numControlPoints, controlPointAt, weightAt"></tas>
}

void EoDbHatch::ConvertEdgesType(const int loopIndex, const OdDbHatchPtr& hatchEntity, EoDbHatch* hatchPrimitive) {
	EdgeArray Edges;
	hatchEntity->getLoopAt(loopIndex, Edges);
	auto Lower {0.0};
	auto Upper {1.0};
	const auto NumberOfEdges {Edges.size()};
	for (unsigned EdgeIndex = 0; EdgeIndex < NumberOfEdges; EdgeIndex++) {
		const auto Edge {Edges[EdgeIndex]};
		if (Edge->type() == OdGe::kCircArc2d) {
			ConvertCircularArcEdge(Edge);
		} else if (Edge->type() == OdGe::kEllipArc2d) {
			ConvertEllipticalArcEdge(Edge);
		} else if (Edge->type() == OdGe::kNurbCurve2d) {
			ConvertNurbCurveEdge(Edge);
		}
		// Common Edge Properties
		OdGeInterval Interval;
		Edge->getInterval(Interval);
		Interval.getBounds(Lower, Upper);
		const auto LowerPoint {Edge->evalPoint(Lower)};
		hatchPrimitive->Append(OdGePoint3d(LowerPoint.x, LowerPoint.y, hatchEntity->elevation()));
	}
	const auto UpperPoint {Edges[NumberOfEdges - 1]->evalPoint(Upper)};
	hatchPrimitive->Append(OdGePoint3d(UpperPoint.x, UpperPoint.y, hatchEntity->elevation()));

	// <tas="Hatch edge conversion - not considering the effect of "Closed" edge property"></tas>
}

void EoDbHatch::AppendLoop(const OdGePoint3dArray& vertices, OdDbHatchPtr& hatch) {
	OdGeMatrix3d WorldToPlaneTransform;
	WorldToPlaneTransform.setToWorldToPlane(OdGePlane(OdGePoint3d::kOrigin, hatch->normal()));
	OdGePoint2dArray Vertices2d;
	Vertices2d.clear();
	OdGeDoubleArray Bulges;
	Bulges.clear();
	for (auto Vertex : vertices) {
		Vertex.transformBy(WorldToPlaneTransform);
		Vertices2d.append(Vertex.convert2d());
		Bulges.append(0.0);
	}
	hatch->appendLoop(OdDbHatch::kPolyline, Vertices2d, Bulges);
}

EoDbHatch* EoDbHatch::Create(const OdDbHatchPtr& hatch) {
	auto Hatch {new EoDbHatch};
	Hatch->SetEntityObjectId(hatch->objectId());
	Hatch->m_ColorIndex = static_cast<short>(hatch->colorIndex());
	Hatch->m_LinetypeIndex = static_cast<short>(EoDbLinetypeTable::LegacyLinetypeIndex(hatch->linetype()));
	if (hatch->isHatch()) {
		switch (hatch->patternType()) {
			case OdDbHatch::kPreDefined: case OdDbHatch::kCustomDefined:
				if (hatch->isSolidFill()) {
					Hatch->SetInteriorStyle(kSolid);
				} else {
					Hatch->SetInteriorStyle(kHatch);
					Hatch->m_InteriorStyleIndex = EoDbHatchPatternTable::LegacyHatchPatternIndex(hatch->patternName());
					const auto Origin {OdGePoint3d::kOrigin + hatch->elevation() * hatch->normal()};
					// <tas="Pattern scaling model to world issues. Resulting hatch is very large without the world scale division"</tas>
					Hatch->SetPatternReferenceSystem(Origin, hatch->normal(), hatch->patternAngle(), hatch->patternScale());
				}
				break;
			case OdDbHatch::kUserDefined:
				Hatch->SetInteriorStyle(kHatch);
				Hatch->m_InteriorStyleIndex = EoDbHatchPatternTable::LegacyHatchPatternIndex(hatch->patternName());
				const auto Origin {OdGePoint3d::kOrigin + hatch->elevation() * hatch->normal()};
				// <tas="Pattern scaling model to world issues. Resulting hatch is very large without the world scale division"</tas>
				Hatch->SetPatternReferenceSystem(Origin, hatch->normal(), hatch->patternAngle(), hatch->patternScale());
				break;
		}
	}
	if (hatch->isGradient()) {
		if (hatch->getGradientOneColorMode()) { }
		OdCmColorArray colors;
		OdGeDoubleArray values;
		hatch->getGradientColors(colors, values);
	}
	// <tas="Not working with associated objects. The objects are still resident just not incorporated in peg/tracing"></tas>
	// <tas="Seed points not incorporated in peg/tracing"></tas>
	const auto NumberOfLoops {hatch->numLoops()};
	if (NumberOfLoops > 1) {
		AeSys::AddStringToReportList(L"Only used one loop in multiple loop Hatch.");
	}
	for (auto i = 0; i < hatch->numLoops(); i++) {
		if (hatch->loopTypeAt(i) & OdDbHatch::kPolyline) {
			if (i == 0) {
				// <tas="Only handling the first loop"</tas>
				ConvertPolylineType(i, hatch, Hatch);
			}
		} else {
			ConvertEdgesType(i, hatch, Hatch);
		}
	}
	return Hatch;
}

OdDbHatchPtr EoDbHatch::Create(OdDbBlockTableRecordPtr blockTableRecord) {
	auto Hatch {OdDbHatch::createObject()};
	Hatch->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(Hatch);
	Hatch->setAssociative(false);
	Hatch->setColorIndex(static_cast<unsigned short>(g_PrimitiveState.ColorIndex()));
	const auto Linetype {LinetypeObjectFromIndex(g_PrimitiveState.LinetypeIndex())};
	Hatch->setLinetype(Linetype);
	const auto HatchName {EoDbHatchPatternTable::LegacyHatchPatternName(g_PrimitiveState.HatchInteriorStyleIndex())};
	Hatch->setPattern(OdDbHatch::kPreDefined, HatchName);
	return Hatch;
}

OdDbHatchPtr EoDbHatch::Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file) {
	auto Hatch {OdDbHatch::createObject()};
	Hatch->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(Hatch);
	const auto ColorIndex {file.ReadInt16()};
	const auto InteriorStyle {file.ReadInt16()};
	const auto InteriorStyleIndex {static_cast<unsigned>(file.ReadInt16())};
	const auto NumberOfVertices {file.ReadUInt16()};
	const auto HatchOrigin {file.ReadPoint3d()};
	const auto HatchXAxis {file.ReadVector3d()};
	const auto HatchYAxis {file.ReadVector3d()};
	OdGePoint3dArray Vertices;
	Vertices.setLogicalLength(NumberOfVertices);
	for (unsigned VertexIndex = 0; VertexIndex < NumberOfVertices; VertexIndex++) {
		Vertices[VertexIndex] = file.ReadPoint3d();
	}
	Hatch->setAssociative(false);
	Hatch->setColorIndex(static_cast<unsigned short>(ColorIndex));
	const auto HatchName(InteriorStyle == kSolid ? OdString(L"SOLID") : EoDbHatchPatternTable::LegacyHatchPatternName(InteriorStyleIndex));
	Hatch->setPattern(OdDbHatch::kPreDefined, HatchName);
	const auto PlaneNormal {ComputeNormal(Vertices[1], Vertices[0], Vertices[2])};
	Hatch->setNormal(PlaneNormal);
	Hatch->setElevation(ComputeElevation(Vertices[0], PlaneNormal));
	AppendLoop(Vertices, Hatch);
	return Hatch;
}

OdDbHatchPtr EoDbHatch::Create(OdDbBlockTableRecordPtr blockTableRecord, unsigned char* primitiveBuffer, const int versionNumber) {
	short ColorIndex;
	short InteriorStyle;
	unsigned InteriorStyleIndex {0};
	OdGePoint3d HatchOrigin;
	OdGeVector3d HatchXAxis;
	OdGeVector3d HatchYAxis;
	OdGePoint3dArray Vertices;
	if (versionNumber == 1) {
		ColorIndex = short(primitiveBuffer[4] & 0x000fU);
		const auto StyleDefinition {reinterpret_cast<EoVaxFloat*>(&primitiveBuffer[12])->Convert()};
		InteriorStyle = short(int(StyleDefinition) % 16);
		switch (InteriorStyle) {
			case kHatch: {
				const auto ScaleFactorX {reinterpret_cast<EoVaxFloat*>(&primitiveBuffer[16])->Convert()};
				const auto ScaleFactorY {reinterpret_cast<EoVaxFloat*>(&primitiveBuffer[20])->Convert()};
				auto PatternAngle {reinterpret_cast<EoVaxFloat*>(&primitiveBuffer[24])->Convert()};
				if (fabs(ScaleFactorX) > FLT_EPSILON && fabs(ScaleFactorY) > FLT_EPSILON) { // Have 2 hatch lines
					InteriorStyleIndex = 2;
					HatchXAxis = OdGeVector3d(cos(PatternAngle), sin(PatternAngle), 0.0);
					HatchYAxis = OdGeVector3d(-sin(PatternAngle), cos(PatternAngle), 0.0);
					HatchXAxis *= ScaleFactorX * 1.e-3;
					HatchYAxis *= ScaleFactorY * 1.e-3;
				} else if (fabs(ScaleFactorX) > FLT_EPSILON) { // Vertical hatch lines
					InteriorStyleIndex = 1;
					PatternAngle += OdaPI2;
					HatchXAxis = OdGeVector3d(cos(PatternAngle), sin(PatternAngle), 0.0);
					HatchYAxis = OdGeVector3d(-sin(PatternAngle), cos(PatternAngle), 0.0);
					HatchXAxis *= ScaleFactorX * 1.e-3;
				} else { // Horizontal hatch lines
					InteriorStyleIndex = 1;
					HatchXAxis = OdGeVector3d(cos(PatternAngle), sin(PatternAngle), 0.0);
					HatchYAxis = OdGeVector3d(-sin(PatternAngle), cos(PatternAngle), 0.0);
					HatchYAxis *= ScaleFactorY * 1.e-3;
				}
				break;
			}
			case kHollow: case kSolid: case kPattern:
				HatchXAxis = OdGeVector3d::kXAxis * 1.e-3;
				HatchYAxis = OdGeVector3d::kYAxis * 1.e-3;
				break;
			default:
				throw L"Exception.FileJob: Unknown hatch primitive interior style.";
		}
		const auto NumberOfVertices {static_cast<unsigned short>(reinterpret_cast<EoVaxFloat*>(& primitiveBuffer[8])->Convert())};
		auto BufferOffset {36};
		Vertices.clear();
		for (unsigned VertexIndex = 0; VertexIndex < NumberOfVertices; VertexIndex++) {
			Vertices.append(reinterpret_cast<EoVaxPoint3d*>(& primitiveBuffer[BufferOffset])->Convert() * 1.e-3);
			BufferOffset += sizeof(EoVaxPoint3d);
		}
		HatchOrigin = Vertices[0];
	} else {
		ColorIndex = short(primitiveBuffer[6]);
		InteriorStyle = static_cast<signed char>(primitiveBuffer[7]);
		InteriorStyleIndex = static_cast<unsigned>(*reinterpret_cast<short*>(& primitiveBuffer[8]));
		const auto NumberOfVertices = *reinterpret_cast<short*>(& primitiveBuffer[10]);
		HatchOrigin = reinterpret_cast<EoVaxPoint3d*>(& primitiveBuffer[12])->Convert();
		HatchXAxis = reinterpret_cast<EoVaxVector3d*>(& primitiveBuffer[24])->Convert();
		HatchYAxis = reinterpret_cast<EoVaxVector3d*>(& primitiveBuffer[36])->Convert();
		auto BufferOffset {48};
		Vertices.clear();
		for (auto VertexIndex = 0; VertexIndex < NumberOfVertices; VertexIndex++) {
			Vertices.append(reinterpret_cast<EoVaxPoint3d*>(& primitiveBuffer[BufferOffset])->Convert());
			BufferOffset += sizeof(EoVaxPoint3d);
		}
	}
	const auto Database {blockTableRecord->database()};
	auto Hatch {OdDbHatch::createObject()};
	Hatch->setDatabaseDefaults(Database);
	blockTableRecord->appendOdDbEntity(Hatch);
	Hatch->setAssociative(false);
	Hatch->setColorIndex(static_cast<unsigned short>(ColorIndex));
	const auto HatchName(InteriorStyle == kSolid ? OdString(L"SOLID") : EoDbHatchPatternTable::LegacyHatchPatternName(InteriorStyleIndex));
	Hatch->setPattern(OdDbHatch::kPreDefined, HatchName);
	const auto PlaneNormal {ComputeNormal(Vertices[1], Vertices[0], Vertices[2])};
	Hatch->setNormal(PlaneNormal);
	Hatch->setElevation(ComputeElevation(Vertices[0], PlaneNormal));
	AppendLoop(Vertices, Hatch);
	return Hatch;
}
