#include "stdafx.h"
#include <DbEllipse.h>
#include <Ge/GeCircArc3d.h>
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "PrimState.h"
#include "EoVaxFloat.h"
#include "EoGePolyline.h"
#include "EoDbFile.h"
IMPLEMENT_DYNAMIC(EoDbEllipse, EoDbPrimitive)

EoDbEllipse::EoDbEllipse(const OdGePoint3d& center, const OdGeVector3d& majorAxis, const OdGeVector3d& minorAxis, const double sweepAngle) noexcept
	: m_Center(center)
	, m_MajorAxis(majorAxis)
	, m_MinorAxis(minorAxis)
	, m_SweepAngle(sweepAngle) {
	m_ColorIndex = g_PrimitiveState.ColorIndex();
	m_LinetypeIndex = g_PrimitiveState.LinetypeIndex();
}

EoDbEllipse::EoDbEllipse(const OdGePoint3d& center, const OdGeVector3d& planeNormal, const double radius)
	: m_Center(center) {
	m_ColorIndex = g_PrimitiveState.ColorIndex();
	m_LinetypeIndex = g_PrimitiveState.LinetypeIndex();
	auto PlaneNormal(planeNormal);
	PlaneNormal.normalize();
	m_MajorAxis = ComputeArbitraryAxis(PlaneNormal);
	m_MajorAxis.normalize();
	m_MajorAxis *= radius;
	m_MinorAxis = PlaneNormal.crossProduct(m_MajorAxis);
	m_SweepAngle = Oda2PI;
}

EoDbEllipse::EoDbEllipse(const EoDbEllipse& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;
	m_ColorIndex = other.m_ColorIndex;
	m_LinetypeIndex = other.m_LinetypeIndex;
	m_Center = other.m_Center;
	m_MajorAxis = other.m_MajorAxis;
	m_MinorAxis = other.m_MinorAxis;
	m_SweepAngle = other.m_SweepAngle;
}

EoDbEllipse& EoDbEllipse::operator=(const EoDbEllipse& other) noexcept {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;
	m_ColorIndex = other.m_ColorIndex;
	m_LinetypeIndex = other.m_LinetypeIndex;
	m_Center = other.m_Center;
	m_MajorAxis = other.m_MajorAxis;
	m_MinorAxis = other.m_MinorAxis;
	m_SweepAngle = other.m_SweepAngle;
	return *this;
}

void EoDbEllipse::AddReportToMessageList(const OdGePoint3d& /*point*/) const {
	CString Report;
	Report += L" Color:" + FormatColorIndex();
	Report += L" Linetype:" + FormatLinetypeIndex();
	const auto LengthAsString {theApp.FormatLength(m_MajorAxis.length(), theApp.GetUnits())};
	if (fabs(m_MajorAxis.lengthSqrd() - m_MinorAxis.lengthSqrd()) <= FLT_EPSILON) {
		if (fabs(m_SweepAngle - Oda2PI) <= FLT_EPSILON) {
			Report = L"<Circle>" + Report;
			Report += L" Radius:" + LengthAsString;
		} else {
			Report = L"<Arc>" + Report;
			Report += L" Radius:" + LengthAsString;
			Report += L" SweepAngle:" + AeSys::FormatAngle(m_SweepAngle);
		}
	} else {
		Report = L"<Ellipse>" + Report;
		Report += L" MajorAxisLength:" + LengthAsString;
	}
	AeSys::AddStringToMessageList(Report);
}

void EoDbEllipse::AddToTreeViewControl(const HWND tree, const HTREEITEM parent) const noexcept {
	CMainFrame::InsertTreeViewControlItem(tree, parent, L"<Arc>", this);
}

EoDbPrimitive* EoDbEllipse::Clone(OdDbBlockTableRecordPtr blockTableRecord) const {
	OdDbEllipsePtr Ellipse = m_EntityObjectId.safeOpenObject()->clone();
	blockTableRecord->appendOdDbEntity(Ellipse);
	return Create(Ellipse);
}

void EoDbEllipse::CutAt(const OdGePoint3d& point, EoDbGroup* newGroup) {
	if (fabs(m_SweepAngle - Oda2PI) <= DBL_EPSILON) {
		// <tas="Never allowing a point cut on closed ellipse"</tas>
	}
	const auto Relationship {SwpAngToPt(point) / m_SweepAngle};
	if (Relationship <= DBL_EPSILON || Relationship >= 1. - DBL_EPSILON) { return; }
	const auto SweepAngle {m_SweepAngle * Relationship};
	OdDbDatabasePtr Database {this->m_EntityObjectId.database()};
	OdDbBlockTableRecordPtr BlockTableRecord {Database->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	auto Arc {Create3(*this, BlockTableRecord)};
	Arc->SetTo2(m_Center, m_MajorAxis, m_MinorAxis, SweepAngle);
	newGroup->AddTail(Arc);
	auto PlaneNormal {m_MajorAxis.crossProduct(m_MinorAxis)};
	PlaneNormal.normalize();
	m_MajorAxis.rotateBy(SweepAngle, PlaneNormal);
	m_MinorAxis.rotateBy(SweepAngle, PlaneNormal);
	m_SweepAngle -= SweepAngle;
	SetTo2(m_Center, m_MajorAxis, m_MinorAxis, m_SweepAngle);
}

void EoDbEllipse::CutAt2Points(OdGePoint3d* points, EoDbGroupList* groups, EoDbGroupList* newGroups, OdDbDatabasePtr database) {
	EoDbEllipse* pArc;
	double dRel[2];
	dRel[0] = SwpAngToPt(points[0]) / m_SweepAngle;
	dRel[1] = SwpAngToPt(points[1]) / m_SweepAngle;
	if (dRel[0] <= DBL_EPSILON && dRel[1] >= 1. - DBL_EPSILON) { // Put entire arc in trap
		pArc = this;
	} else { // Something gets cut
		OdDbBlockTableRecordPtr BlockTableRecord {database->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
		auto PlaneNormal {m_MajorAxis.crossProduct(m_MinorAxis)};
		PlaneNormal.normalize();
		if (fabs(m_SweepAngle - Oda2PI) <= DBL_EPSILON) { // Closed arc
			m_SweepAngle = (dRel[1] - dRel[0]) * Oda2PI;
			m_MajorAxis.rotateBy(dRel[0] * Oda2PI, PlaneNormal);
			m_MinorAxis.rotateBy(dRel[0] * Oda2PI, PlaneNormal);
			pArc = Create3(*this, BlockTableRecord);
			m_MajorAxis.rotateBy(m_SweepAngle, PlaneNormal);
			m_MinorAxis.rotateBy(m_SweepAngle, PlaneNormal);
			m_SweepAngle = Oda2PI - m_SweepAngle;
		} else { // Arc section with a cut
			pArc = Create3(*this, BlockTableRecord);
			const auto dSwpAng {m_SweepAngle};
			const auto dAng1 {dRel[0] * m_SweepAngle};
			const auto dAng2 {dRel[1] * m_SweepAngle};
			if (isgreater(dRel[0], 0.0)) {
			}
			if (dRel[0] > DBL_EPSILON && dRel[1] < 1. - DBL_EPSILON) { // Cut section out of middle
				pArc->SetSweepAngle(dAng1);
				auto Group {new EoDbGroup};
				Group->AddTail(pArc);
				groups->AddTail(Group);
				m_MajorAxis.rotateBy(dAng1, PlaneNormal);
				m_MinorAxis.rotateBy(dAng1, PlaneNormal);
				m_SweepAngle = dAng2 - dAng1;
				pArc = Create3(*this, BlockTableRecord);
				m_MajorAxis.rotateBy(m_SweepAngle, PlaneNormal);
				m_MinorAxis.rotateBy(m_SweepAngle, PlaneNormal);
				m_SweepAngle = dSwpAng - dAng2;
			} else if (dRel[1] < 1. - DBL_EPSILON) { // Cut section in two and place begin section in trap
				pArc->SetSweepAngle(dAng2);
				m_MajorAxis.rotateBy(dAng2, PlaneNormal);
				m_MinorAxis.rotateBy(dAng2, PlaneNormal);
				m_SweepAngle = dSwpAng - dAng2;
			} else { // Cut section in two and place end section in trap
				m_SweepAngle = dAng1;
				auto v {m_MajorAxis};
				v.rotateBy(dAng1, PlaneNormal);
				pArc->SetMajorAxis(v);
				v = m_MinorAxis;
				v.rotateBy(dAng1, PlaneNormal);
				pArc->SetMinorAxis(v);
				pArc->SetSweepAngle(dSwpAng - dAng1);
			}
		}
		auto Group {new EoDbGroup};
		Group->AddTail(this);
		groups->AddTail(Group);
	}
	auto NewGroup {new EoDbGroup};
	NewGroup->AddTail(pArc);
	newGroups->AddTail(NewGroup);
}

void EoDbEllipse::Display(AeSysView* view, CDC* deviceContext) {
	if (fabs(m_SweepAngle) <= DBL_EPSILON) { return; }
	const auto ColorIndex {LogicalColorIndex()};
	const auto LinetypeIndex {LogicalLinetypeIndex()};
	g_PrimitiveState.SetPen(view, deviceContext, ColorIndex, LinetypeIndex);
	polyline::BeginLineStrip();
	GenPts(OdGePlane(m_Center, m_MajorAxis, m_MinorAxis), m_SweepAngle);
	polyline::__End(view, deviceContext, LinetypeIndex);
}

void EoDbEllipse::FormatExtra(CString& extra) const {
	extra.Empty();
	extra += L"Color;" + FormatColorIndex() + L"\t";
	extra += L"Linetype;" + FormatLinetypeIndex() + L"\t";
	extra += L"Sweep Angle;" + AeSys::FormatAngle(m_SweepAngle) + L"\t";
	extra += L"Major Axis Length;" + theApp.FormatLength(m_MajorAxis.length(), theApp.GetUnits());
}

void EoDbEllipse::FormatGeometry(CString& geometry) const {
	const auto Normal {m_MajorAxis.crossProduct(m_MinorAxis)};
	CString CenterString;
	CenterString.Format(L"Center Point;%f;%f;%f\t", m_Center.x, m_Center.y, m_Center.z);
	geometry += CenterString;
	CString MajorAxisString;
	MajorAxisString.Format(L"Major Axis;%f;%f;%f\t", m_MajorAxis.x, m_MajorAxis.y, m_MajorAxis.z);
	geometry += MajorAxisString;
	CString MinorAxisString;
	MinorAxisString.Format(L"Minor Axis;%f;%f;%f\t", m_MinorAxis.x, m_MinorAxis.y, m_MinorAxis.z);
	geometry += MinorAxisString;
	CString NormalString;
	NormalString.Format(L"Plane Normal;%f;%f;%f\t", Normal.x, Normal.y, Normal.z);
	geometry += NormalString;
}

void EoDbEllipse::GenPts(const OdGePlane& plane, const double sweepAngle) const {
	OdGeMatrix3d ScaleMatrix;
	ScaleMatrix.setToScaling(OdGeScale3d(m_MajorAxis.length(), m_MinorAxis.length(), 1.0));
	OdGeMatrix3d PlaneToWorldTransform;
	PlaneToWorldTransform.setToPlaneToWorld(plane); // <tas=Builds a matrix which performs rotation and translation, but no scaling.</tas>
	// Number of points based on angle and a smoothness coefficient
	const auto Length {EoMax(m_MajorAxis.length(), m_MinorAxis.length())};
	auto NumberOfPoints {EoMax(2L, labs(lround(sweepAngle / Oda2PI * 32.0)))};
	NumberOfPoints = EoMin(128L, EoMax(NumberOfPoints, labs(lround(sweepAngle * Length / 0.25))));
	const auto Angle {m_SweepAngle / (static_cast<double>(NumberOfPoints) - 1.0)};
	const auto CosineAngle {cos(Angle)};
	const auto SineAngle {sin(Angle)};

	// Generate an origin-centered unit radial curve, then scale before transforming back the world
	OdGePoint3d Point(1.0, 0.0, 0.0);
	for (auto PointIndex = 0; PointIndex < NumberOfPoints; PointIndex++) {
		polyline::SetVertex(PlaneToWorldTransform * ScaleMatrix * Point);
		const auto X {Point.x};
		Point.x = X * CosineAngle - Point.y * SineAngle;
		Point.y = Point.y * CosineAngle + X * SineAngle;
		Point.z = 0.0;
	}
}

void EoDbEllipse::GetAllPoints(OdGePoint3dArray& points) const {
	points.clear();
	points.append(m_Center);
}

OdGePoint3d EoDbEllipse::GetCtrlPt() const noexcept {
	return m_Center;
}

OdGePoint3d EoDbEllipse::StartPoint() const {
	return m_Center + m_MajorAxis;
}

void EoDbEllipse::GetBoundingBox(OdGePoint3dArray& ptsBox) const {
	ptsBox.setLogicalLength(4);
	ptsBox[0] = OdGePoint3d(-1.0, -1.0, 0.0);
	ptsBox[1] = OdGePoint3d(1.0, -1.0, 0.0);
	ptsBox[2] = OdGePoint3d(1.0, 1.0, 0.0);
	ptsBox[3] = OdGePoint3d(-1.0, 1.0, 0.0);
	if (m_SweepAngle < 3. * Oda2PI / 4.) {
		const auto dEndX {cos(m_SweepAngle)};
		const auto dEndY {sin(m_SweepAngle)};
		if (dEndX >= 0.0) {
			if (dEndY >= 0.0) { // Arc ends in quadrant one
				ptsBox[0].x = dEndX;
				ptsBox[0].y = 0.0;
				ptsBox[1].y = 0.0;
				ptsBox[2].y = dEndY;
				ptsBox[3].x = dEndX;
				ptsBox[3].y = dEndY;
			}
		} else {
			if (dEndY >= 0.0) { // Arc ends in quadrant two
				ptsBox[0].x = dEndX;
				ptsBox[0].y = 0.0;
				ptsBox[1].y = 0.0;
				ptsBox[3].x = dEndX;
			} else { // Arc ends in quadrant three
				ptsBox[0].y = dEndY;
				ptsBox[1].y = dEndY;
			}
		}
	}
	OdGeMatrix3d ScaleMatrix;
	ScaleMatrix.setToScaling(OdGeScale3d(m_MajorAxis.length(), m_MinorAxis.length(), 1.0));
	OdGeMatrix3d PlaneToWorldTransform;
	PlaneToWorldTransform.setToPlaneToWorld(OdGePlane(m_Center, m_MajorAxis, m_MinorAxis));
	PlaneToWorldTransform.postMultBy(ScaleMatrix);
	for (unsigned w = 0; w < 4; w++) {
		ptsBox[w].transformBy(PlaneToWorldTransform);
	}
}

OdGePoint3d EoDbEllipse::EndPoint() const {
	OdGeMatrix3d ScaleMatrix;
	ScaleMatrix.setToScaling(OdGeScale3d(m_MajorAxis.length(), m_MinorAxis.length(), 1.0));
	EoGeMatrix3d PlaneToWorldTransform;
	PlaneToWorldTransform.setToPlaneToWorld(OdGePlane(m_Center, m_MajorAxis, m_MinorAxis));
	PlaneToWorldTransform.postMultBy(ScaleMatrix);
	OdGePoint3d pt(cos(m_SweepAngle), sin(m_SweepAngle), 0.0);
	pt.transformBy(PlaneToWorldTransform);
	return pt;
}

OdGeVector3d EoDbEllipse::MajorAxis() const noexcept {
	return m_MajorAxis;
}

OdGeVector3d EoDbEllipse::MinorAxis() const noexcept {
	return m_MinorAxis;
}

OdGePoint3d EoDbEllipse::Center() const noexcept {
	return m_Center;
}

double EoDbEllipse::SweepAngle() const noexcept {
	return m_SweepAngle;
}

void EoDbEllipse::GetXYExtents(const OdGePoint3d arBeg, const OdGePoint3d arEnd, OdGePoint3d* arMin, OdGePoint3d* arMax) noexcept {
	const auto dx {m_Center.x - arBeg.x};
	const auto dy {m_Center.y - arBeg.y};
	const auto dRad {sqrt(dx * dx + dy * dy)};
	(*arMin).x = m_Center.x - dRad;
	(*arMin).y = m_Center.y - dRad;
	(*arMax).x = m_Center.x + dRad;
	(*arMax).y = m_Center.y + dRad;
	if (arBeg.x >= m_Center.x) {
		if (arBeg.y >= m_Center.y) { // Arc begins in quadrant one
			if (arEnd.x >= m_Center.x) {
				if (arEnd.y >= m_Center.y) { // Arc ends in quadrant one
					if (arBeg.x > arEnd.x) { // Arc in quadrant one only
						(*arMin).x = arEnd.x;
						(*arMin).y = arBeg.y;
						(*arMax).x = arBeg.x;
						(*arMax).y = arEnd.y;
					}
				} else { // Arc ends in quadrant four
					(*arMax).x = EoMax(arBeg.x, arEnd.x);
				}
			} else {
				if (arEnd.y >= m_Center.y) { // Arc ends in quadrant two
					(*arMin).x = arEnd.x;
					(*arMin).y = EoMin(arBeg.y, arEnd.y);
				} else { // Arc ends in quadrant three
					(*arMin).y = arEnd.y;
				}
				(*arMax).x = arBeg.x;
			}
		} else { // Arc begins in quadrant four
			if (arEnd.x >= m_Center.x) {
				if (arEnd.y >= m_Center.y) { // Arc ends in quadrant one
					(*arMin).x = EoMin(arBeg.x, arEnd.x);
					(*arMin).y = arBeg.y;
					(*arMax).y = arEnd.y;
				} else { // Arc ends in quadrant four
					if (arBeg.x < arEnd.x) { // Arc in quadrant one only
						(*arMin).x = arBeg.x;
						(*arMin).y = arBeg.y;
						(*arMax).x = arEnd.x;
						(*arMax).y = arEnd.y;
					}
				}
			} else {
				if (arEnd.y >= m_Center.y) { // Arc ends in quadrant two
					(*arMin).x = arEnd.x;
					(*arMin).y = arBeg.y;
				} else { // Arc ends in quadrant three
					(*arMin).y = EoMin(arBeg.y, arEnd.y);
				}
			}
		}
	} else {
		if (arBeg.y >= m_Center.y) { // Arc begins in quadrant two
			if (arEnd.x >= m_Center.x) {
				if (arEnd.y >= m_Center.y) { // Arc ends in quadrant one
					(*arMax).y = EoMax(arBeg.y, arEnd.y);
				} else { // Arc ends in quadrant four
					(*arMax).x = arEnd.x;
					(*arMax).y = arBeg.y;
				}
			} else {
				if (arEnd.y >= m_Center.y) { // Arc ends in quadrant two
					if (arBeg.x > arEnd.x) { // Arc in quadrant two only
						(*arMin).x = arEnd.x;
						(*arMin).y = arEnd.y;
						(*arMax).x = arBeg.x;
						(*arMax).y = arBeg.y;
					}
				} else { // Arc ends in quadrant three
					(*arMin).y = arEnd.y;
					(*arMax).x = EoMax(arBeg.x, arEnd.x);
					(*arMax).y = arBeg.y;
				}
			}
		} else { // Arc begins in quadrant three
			if (arEnd.x >= m_Center.x) {
				if (arEnd.y >= m_Center.y) { // Arc ends in quadrant one
					(*arMax).y = arEnd.y;
				} else { // Arc ends in quadrant four
					(*arMax).x = arEnd.x;
					(*arMax).y = EoMax(arBeg.y, arEnd.y);
				}
				(*arMin).x = arBeg.x;
			} else {
				if (arEnd.y >= m_Center.y) { // Arc ends in quadrant two
					(*arMin).x = EoMin(arBeg.x, arEnd.x);
				} else { // Arc ends in quadrant three
					if (arBeg.x < arEnd.x) { // Arc in quadrant three only
						(*arMin).x = arBeg.x;
						(*arMin).y = arEnd.y;
						(*arMax).x = arEnd.x;
						(*arMax).y = arBeg.y;
					}
				}
			}
		}
	}
}

void EoDbEllipse::GetExtents(AeSysView* /*view*/, OdGeExtents3d& extents) const {
	if (!m_EntityObjectId.isNull()) {
		auto Entity {m_EntityObjectId.safeOpenObject()};
		OdGeExtents3d Extents;
		Entity->getGeomExtents(Extents);
		extents.addExt(Extents);
	} else {
		OdGePoint3dArray BoundingBox;
		GetBoundingBox(BoundingBox);
		for (unsigned w = 0; w < 4; w++) {
			extents.addPoint(BoundingBox[w]);
		}
	}
}

bool EoDbEllipse::IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	EoGePoint4d Points[] {EoGePoint4d(StartPoint(), 1.0), EoGePoint4d(EndPoint(), 1.0)};
	for (auto& Point : Points) {
		view->ModelViewTransformPoint(Point);
		if (point.DistanceToPointXY(Point) < ms_SelectApertureSize) { return true; }
	}
	return false;
}

int EoDbEllipse::IsWithinArea(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, OdGePoint3d* intersections) {
	auto PlaneNormal {m_MajorAxis.crossProduct(m_MinorAxis)};
	PlaneNormal.normalize();
	if (!OdGeVector3d::kZAxis.crossProduct(PlaneNormal).isZeroLength()) { return 0; } // not on plane normal to z-axis
	if (fabs(m_MajorAxis.length() - m_MinorAxis.length()) > FLT_EPSILON) { return 0; } // not radial
	OdGePoint3d ptMin;
	OdGePoint3d ptMax;
	auto ptBeg {StartPoint()};
	auto ptEnd {EndPoint()};
	if (PlaneNormal.z < 0.0) {
		const auto pt {ptBeg};
		ptBeg = ptEnd;
		ptEnd = pt;
		PlaneNormal = -PlaneNormal;
		m_MajorAxis = ptBeg - m_Center;
		m_MinorAxis = PlaneNormal.crossProduct(m_MajorAxis);
	}
	GetXYExtents(ptBeg, ptEnd, &ptMin, &ptMax);
	if (ptMin.x >= lowerLeftCorner.x && ptMax.x <= upperRightCorner.x && ptMin.y >= lowerLeftCorner.y && ptMax.y <= upperRightCorner.y) { // Totally within window boundaries
		intersections[0] = ptBeg;
		intersections[1] = ptEnd;
		return 2;
	}
	if (ptMin.x >= upperRightCorner.x || ptMax.x <= lowerLeftCorner.x || ptMin.y >= upperRightCorner.y || ptMax.y <= lowerLeftCorner.y) { return 0; }
	OdGePoint3d ptWrk[8];
	double dDis;
	double dOff;
	auto iSecs {0};
	const auto dRad {OdGeVector3d(ptBeg - m_Center).length()};
	if (ptMax.x > upperRightCorner.x) { // Arc may intersect with right window boundary
		dDis = upperRightCorner.x - m_Center.x;
		dOff = sqrt(dRad * dRad - dDis * dDis);
		if (m_Center.y - dOff >= lowerLeftCorner.y && m_Center.y - dOff <= upperRightCorner.y) {
			ptWrk[iSecs].x = upperRightCorner.x;
			ptWrk[iSecs++].y = m_Center.y - dOff;
		}
		if (m_Center.y + dOff <= upperRightCorner.y && m_Center.y + dOff >= lowerLeftCorner.y) {
			ptWrk[iSecs].x = upperRightCorner.x;
			ptWrk[iSecs++].y = m_Center.y + dOff;
		}
	}
	if (ptMax.y > upperRightCorner.y) { // Arc may intersect with top window boundary
		dDis = upperRightCorner.y - m_Center.y;
		dOff = sqrt(dRad * dRad - dDis * dDis);
		if (m_Center.x + dOff <= upperRightCorner.x && m_Center.x + dOff >= lowerLeftCorner.x) {
			ptWrk[iSecs].x = m_Center.x + dOff;
			ptWrk[iSecs++].y = upperRightCorner.y;
		}
		if (m_Center.x - dOff >= lowerLeftCorner.x && m_Center.x - dOff <= upperRightCorner.x) {
			ptWrk[iSecs].x = m_Center.x - dOff;
			ptWrk[iSecs++].y = upperRightCorner.y;
		}
	}
	if (ptMin.x < lowerLeftCorner.x) { // Arc may intersect with left window boundary
		dDis = m_Center.x - lowerLeftCorner.x;
		dOff = sqrt(dRad * dRad - dDis * dDis);
		if (m_Center.y + dOff <= upperRightCorner.y && m_Center.y + dOff >= lowerLeftCorner.y) {
			ptWrk[iSecs].x = lowerLeftCorner.x;
			ptWrk[iSecs++].y = m_Center.y + dOff;
		}
		if (m_Center.y - dOff >= lowerLeftCorner.y && m_Center.y - dOff <= upperRightCorner.y) {
			ptWrk[iSecs].x = lowerLeftCorner.x;
			ptWrk[iSecs++].y = m_Center.y - dOff;
		}
	}
	if (ptMin.y < lowerLeftCorner.y) { // Arc may intersect with bottom window boundary
		dDis = m_Center.y - lowerLeftCorner.y;
		dOff = sqrt(dRad * dRad - dDis * dDis);
		if (m_Center.x - dOff >= lowerLeftCorner.x && m_Center.x - dOff <= upperRightCorner.x) {
			ptWrk[iSecs].x = m_Center.x - dOff;
			ptWrk[iSecs++].y = lowerLeftCorner.y;
		}
		if (m_Center.x + dOff <= upperRightCorner.x && m_Center.x + dOff >= lowerLeftCorner.x) {
			ptWrk[iSecs].x = m_Center.x + dOff;
			ptWrk[iSecs++].y = lowerLeftCorner.y;
		}
	}
	if (iSecs == 0) { return 0; }
	const auto BeginAngle {atan2(ptBeg.y - m_Center.y, ptBeg.x - m_Center.x)}; // Arc begin angle (- pi to pi)
	double AngleIntersections[8] {0.0};
	auto IntersectionIndex {0};
	for (auto i2 = 0; i2 < iSecs; i2++) { // Loop thru possible intersections
		const auto CurrentIntersectionAngle {atan2(ptWrk[i2].y - m_Center.y, ptWrk[i2].x - m_Center.x)};
		AngleIntersections[IntersectionIndex] = CurrentIntersectionAngle - BeginAngle;
		if (AngleIntersections[IntersectionIndex] < 0.0) { AngleIntersections[IntersectionIndex] += Oda2PI; }
		if (fabs(AngleIntersections[IntersectionIndex]) - m_SweepAngle < 0.0) { // Intersection lies on arc
			int i;
			for (i = 0; i < IntersectionIndex && ptWrk[i2] != intersections[i]; i++) {
			}
			if (i == IntersectionIndex) { // Unique intersection
				intersections[IntersectionIndex++] = ptWrk[i2];
			}
		}
	}
	if (IntersectionIndex == 0) { return 0; } // None of the intersections are on sweep of arc
	for (auto i1 = 0; i1 < IntersectionIndex; i1++) { // Sort intersections from begin to end of sweep
		for (auto i2 = 1; i2 < IntersectionIndex - i1; i2++) {
			if (fabs(AngleIntersections[i2]) < fabs(AngleIntersections[i2 - 1])) {
				const auto AngleIntersection {AngleIntersections[i2 - 1]};
				AngleIntersections[i2 - 1] = AngleIntersections[i2];
				AngleIntersections[i2] = AngleIntersection;
				const auto pt {intersections[i2 - 1]};
				intersections[i2 - 1] = intersections[i2];
				intersections[i2] = pt;
			}
		}
	}
	if (fabs(m_SweepAngle - Oda2PI) <= DBL_EPSILON) { // Arc is a circle in disguise
	} else {
		if (ptBeg.x >= lowerLeftCorner.x && ptBeg.x <= upperRightCorner.x && ptBeg.y >= lowerLeftCorner.y && ptBeg.y <= upperRightCorner.y) { // Add beg point to int set
			for (auto i = IntersectionIndex; i > 0; i--) {
				intersections[i] = intersections[i - 1];
			}
			intersections[0] = ptBeg;
			IntersectionIndex++;
		}
		if (ptEnd.x >= lowerLeftCorner.x && ptEnd.x <= upperRightCorner.x && ptEnd.y >= lowerLeftCorner.y && ptEnd.y <= upperRightCorner.y) { // Add end point to int set
			intersections[IntersectionIndex] = ptEnd;
			IntersectionIndex++;
		}
	}
	return IntersectionIndex;
}

OdGePoint3d EoDbEllipse::GoToNxtCtrlPt() const {
	const auto dAng {ms_RelationshipOfPoint <= DBL_EPSILON ? m_SweepAngle : 0.0};
	return FindPointOnArc(m_Center, m_MajorAxis, m_MinorAxis, dAng);
}

bool EoDbEllipse::IsEqualTo(EoDbPrimitive* /*primitive*/) const noexcept {
	return false;
}

bool EoDbEllipse::IsInView(AeSysView* view) const {
	OdGePoint3dArray BoundingBox;
	GetBoundingBox(BoundingBox);
	EoGePoint4d ptBeg(BoundingBox[0], 1.0);
	view->ModelViewTransformPoint(ptBeg);
	for (unsigned w = 1; w < 4; w++) {
		EoGePoint4d ptEnd(BoundingBox[w], 1.0);
		view->ModelViewTransformPoint(ptEnd);
		if (EoGePoint4d::ClipLine(ptBeg, ptEnd)) { return true; }
		ptBeg = ptEnd;
	}
	return false;
}

OdGePoint3d EoDbEllipse::SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	ms_ControlPointIndex = SIZE_T_MAX;
	auto Aperture {ms_SelectApertureSize};
	OdGePoint3d ptCtrl[] = {StartPoint(), EndPoint()};
	for (unsigned w = 0; w < 2; w++) {
		EoGePoint4d pt(ptCtrl[w], 1.0);
		view->ModelViewTransformPoint(pt);
		const auto dDis {point.DistanceToPointXY(pt)};
		if (dDis < Aperture) {
			ms_ControlPointIndex = w;
			Aperture = dDis;
		}
	}
	return ms_ControlPointIndex == SIZE_T_MAX ? OdGePoint3d::kOrigin : ptCtrl[ms_ControlPointIndex];
}

bool EoDbEllipse::SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view, OdGePoint3dArray& intersections) {
	polyline::BeginLineStrip();
	GenPts(OdGePlane(m_Center, m_MajorAxis, m_MinorAxis), m_SweepAngle);
	return polyline::SelectUsingLineSeg(lineSeg, view, intersections);
}

bool EoDbEllipse::SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& projectedPoint) const {
	polyline::BeginLineStrip();
	GenPts(OdGePlane(m_Center, m_MajorAxis, m_MinorAxis), m_SweepAngle);
	return polyline::SelectUsingPoint(point, view, ms_RelationshipOfPoint, projectedPoint);
}

bool EoDbEllipse::SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const {
	polyline::BeginLineStrip();
	GenPts(OdGePlane(m_Center, m_MajorAxis, m_MinorAxis), m_SweepAngle);
	return polyline::SelectUsingRectangle(lowerLeftCorner, upperRightCorner, view);
}

void EoDbEllipse::SetCenter(const OdGePoint3d& center) noexcept {
	m_Center = center;
}

void EoDbEllipse::SetMajorAxis(const OdGeVector3d& majorAxis) noexcept {
	m_MajorAxis = majorAxis;
}

void EoDbEllipse::SetMinorAxis(const OdGeVector3d& minorAxis) noexcept {
	m_MinorAxis = minorAxis;
}

void EoDbEllipse::SetSweepAngle(const double angle) noexcept {
	m_SweepAngle = angle;
}

EoDbEllipse& EoDbEllipse::SetTo2(const OdGePoint3d& center, const OdGeVector3d& majorAxis, const OdGeVector3d& minorAxis, const double sweepAngle) {
	auto PlaneNormal {majorAxis.crossProduct(minorAxis)};
	if (!PlaneNormal.isZeroLength()) {
		m_Center = center;
		m_MajorAxis = majorAxis;
		m_MinorAxis = minorAxis;
		m_SweepAngle = sweepAngle;
		if (!m_EntityObjectId.isNull()) {
			OdDbEllipsePtr Ellipse = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
			const auto RadiusRatio {minorAxis.length() / majorAxis.length()};
			PlaneNormal.normalize();
			Ellipse->set(center, PlaneNormal, majorAxis, RadiusRatio, 0.0, sweepAngle);
		}
	}
	return *this;
}

EoDbEllipse& EoDbEllipse::SetTo3PointArc(const OdGePoint3d& startPoint, const OdGePoint3d& intermediatePoint, const OdGePoint3d& endPoint) {
	m_SweepAngle = 0.0;
	auto PlaneNormal {OdGeVector3d(intermediatePoint - startPoint).crossProduct(OdGeVector3d(endPoint - startPoint))};
	if (PlaneNormal.isZeroLength()) {
		return *this;
	}
	PlaneNormal.normalize();

	// Build transformation matrix which will get intermediate and end points to z=0 plane with start point as origin
	EoGeMatrix3d WorldToPlaneAtStartPoint;
	WorldToPlaneAtStartPoint.setToWorldToPlane(OdGePlane(startPoint, PlaneNormal));
	OdGePoint3d pt[3];
	pt[0] = startPoint;
	pt[1] = intermediatePoint;
	pt[2] = endPoint;
	pt[1].transformBy(WorldToPlaneAtStartPoint);
	pt[2].transformBy(WorldToPlaneAtStartPoint);
	const auto dDet {pt[1].x * pt[2].y - pt[2].x * pt[1].y};
	if (fabs(dDet) > DBL_EPSILON) { // Three points are not colinear
		const auto dT {((pt[2].x - pt[1].x) * pt[2].x + pt[2].y * (pt[2].y - pt[1].y)) / dDet};
		m_Center.x = (pt[1].x - pt[1].y * dT) * 0.5;
		m_Center.y = (pt[1].y + pt[1].x * dT) * 0.5;
		m_Center.z = 0.0;
		WorldToPlaneAtStartPoint.invert();

		// Transform back to original plane
		m_Center.transformBy(WorldToPlaneAtStartPoint);

		// None of the points coincide with center point
		EoGeMatrix3d WorldToPlaneAtCenterPointTransform;
		WorldToPlaneAtCenterPointTransform.setToWorldToPlane(OdGePlane(m_Center, PlaneNormal));
		double dAng[3];
		pt[1] = intermediatePoint;
		pt[2] = endPoint;
		for (auto i = 0; i < 3; i++) { // Translate points into z=0 plane with center point at origin
			pt[i].transformBy(WorldToPlaneAtCenterPointTransform);
			dAng[i] = atan2(pt[i].y, pt[i].x);
			if (dAng[i] < 0.0) { dAng[i] += Oda2PI; }
		}
		const auto dMin {EoMin(dAng[0], dAng[2])};
		const auto dMax {EoMax(dAng[0], dAng[2])};
		if (fabs(dAng[1] - dMax) > DBL_EPSILON && fabs(dAng[1] - dMin) > DBL_EPSILON) { // Inside line is not colinear with outside lines
			m_SweepAngle = dMax - dMin;
			if (dAng[1] > dMin && dAng[1] < dMax) {
				if (dAng[0] == dMax) { m_SweepAngle = -m_SweepAngle; }
			} else {
				m_SweepAngle = Oda2PI - m_SweepAngle;
				if (dAng[2] == dMax) { m_SweepAngle = -m_SweepAngle; }
			}
			auto ptRot {startPoint};
			ptRot.rotateBy(OdaPI2, PlaneNormal, m_Center);
			m_MajorAxis = OdGeVector3d(startPoint - m_Center);
			m_MinorAxis = OdGeVector3d(ptRot - m_Center);
			SetTo2(m_Center, m_MajorAxis, m_MinorAxis, m_SweepAngle);
		}
	}
	return *this;
}

EoDbEllipse& EoDbEllipse::SetToCircle(const OdGePoint3d& center, const OdGeVector3d& planeNormal, const double radius) {
	if (!planeNormal.isZeroLength()) {
		auto PlaneNormal(planeNormal);
		PlaneNormal.normalize();
		m_Center = center;
		m_MajorAxis = ComputeArbitraryAxis(PlaneNormal);
		m_MajorAxis.normalize();
		m_MajorAxis *= radius;
		m_MinorAxis = PlaneNormal.crossProduct(m_MajorAxis);
		m_SweepAngle = Oda2PI;
		SetTo2(m_Center, m_MajorAxis, m_MinorAxis, m_SweepAngle);
	}
	return *this;
}

double EoDbEllipse::SwpAngToPt(const OdGePoint3d& point) {
	auto PlaneNormal {m_MajorAxis.crossProduct(m_MinorAxis)};
	PlaneNormal.normalize();
	EoGeMatrix3d tm;
	tm.setToWorldToPlane(OdGePlane(m_Center, PlaneNormal));
	auto StartPoint {m_Center + m_MajorAxis};
	auto Point {point};

	// Translate points into z=0 plane
	StartPoint.transformBy(tm);
	Point.transformBy(tm);
	return EoGeLineSeg3d(OdGePoint3d::kOrigin, StartPoint).AngleBetween_xy(EoGeLineSeg3d(OdGePoint3d::kOrigin, Point));
}

void EoDbEllipse::TransformBy(const EoGeMatrix3d& transformMatrix) {
	m_Center.transformBy(transformMatrix);
	m_MajorAxis.transformBy(transformMatrix);
	m_MinorAxis.transformBy(transformMatrix);
}

void EoDbEllipse::TranslateUsingMask(const OdGeVector3d& translate, const unsigned mask) {
	if (mask != 0) { m_Center += translate; }
}

bool EoDbEllipse::Write(EoDbFile& file) const {
	file.WriteUInt16(EoDb::kEllipsePrimitive);
	file.WriteInt16(m_ColorIndex);
	file.WriteInt16(m_LinetypeIndex);
	file.WriteDouble(m_Center.x);
	file.WriteDouble(m_Center.y);
	file.WriteDouble(m_Center.z);
	file.WriteDouble(m_MajorAxis.x);
	file.WriteDouble(m_MajorAxis.y);
	file.WriteDouble(m_MajorAxis.z);
	file.WriteDouble(m_MinorAxis.x);
	file.WriteDouble(m_MinorAxis.y);
	file.WriteDouble(m_MinorAxis.z);
	file.WriteDouble(m_SweepAngle);
	return true;
}

void EoDbEllipse::Write(CFile& file, unsigned char* buffer) const {
	buffer[3] = 2;
	*reinterpret_cast<unsigned short*>(& buffer[4]) = static_cast<unsigned short>(EoDb::kEllipsePrimitive);
	buffer[6] = static_cast<unsigned char>(m_ColorIndex == mc_ColorindexBylayer ? ms_LayerColorIndex : m_ColorIndex);
	buffer[7] = static_cast<unsigned char>(m_LinetypeIndex == mc_LinetypeBylayer ? ms_LayerLinetypeIndex : m_LinetypeIndex);
	if (buffer[7] >= 16) { buffer[7] = 2; }
	reinterpret_cast<EoVaxPoint3d*>(& buffer[8])->Convert(m_Center);
	reinterpret_cast<EoVaxVector3d*>(& buffer[20])->Convert(m_MajorAxis);
	reinterpret_cast<EoVaxVector3d*>(& buffer[32])->Convert(m_MinorAxis);
	reinterpret_cast<EoVaxFloat*>(& buffer[44])->Convert(m_SweepAngle);
	file.Write(buffer, 64);
}

EoDbEllipse* EoDbEllipse::Create3(const EoDbEllipse& ellipse, OdDbBlockTableRecordPtr& blockTableRecord) {
	// <tas="Possibly need additional typing of the ObjectId produced by cloning"></tas>
	OdDbEllipsePtr EllipseEntity {ellipse.EntityObjectId().safeOpenObject()->clone()};
	blockTableRecord->appendOdDbEntity(EllipseEntity);
	auto Ellipse {new EoDbEllipse(ellipse)};
	Ellipse->SetEntityObjectId(EllipseEntity->objectId());
	return Ellipse;
}

EoDbEllipse* EoDbEllipse::Create(OdDbEllipsePtr& ellipse) {
	auto Ellipse {new EoDbEllipse()};
	Ellipse->SetEntityObjectId(ellipse->objectId());
	Ellipse->m_ColorIndex = static_cast<short>(ellipse->colorIndex());
	Ellipse->m_LinetypeIndex = static_cast<short>(EoDbLinetypeTable::LegacyLinetypeIndex(ellipse->linetype()));
	auto MajorAxis(ellipse->majorAxis());
	auto MinorAxis(ellipse->minorAxis());
	auto StartAngle {ellipse->startAngle()};
	auto EndAngle {ellipse->endAngle()};
	if (StartAngle >= Oda2PI) { // need to rationalize angles to first period angles in range on (0 to twopi)
		StartAngle -= Oda2PI;
		EndAngle -= Oda2PI;
	}
	auto SweepAngle {EndAngle - StartAngle};
	if (SweepAngle <= FLT_EPSILON) { SweepAngle += Oda2PI; }
	if (StartAngle != 0.0) {
		MajorAxis.rotateBy(StartAngle, ellipse->normal());
		MinorAxis.rotateBy(StartAngle, ellipse->normal());
		if (ellipse->radiusRatio() != 1.0) {
			TRACE0("Ellipse: Non radial with start parameter not 0.\n");
		}
	}
	Ellipse->SetCenter(ellipse->center());
	Ellipse->SetMajorAxis(MajorAxis);
	Ellipse->SetMinorAxis(MinorAxis);
	Ellipse->SetSweepAngle(SweepAngle);
	return Ellipse;
}

OdDbEllipsePtr EoDbEllipse::Create(OdDbBlockTableRecordPtr& blockTableRecord) {
	auto Ellipse {OdDbEllipse::createObject()};
	Ellipse->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(Ellipse);
	Ellipse->setColorIndex(static_cast<unsigned short>(g_PrimitiveState.ColorIndex()));
	const auto Linetype {LinetypeObjectFromIndex(g_PrimitiveState.LinetypeIndex())};
	Ellipse->setLinetype(Linetype);
	return Ellipse;
}

OdDbEllipsePtr EoDbEllipse::CreateCircle(OdDbBlockTableRecordPtr& blockTableRecord, const OdGePoint3d& center, const OdGeVector3d& normal, const double radius) {
	auto Ellipse {OdDbEllipse::createObject()};
	Ellipse->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(Ellipse);
	const OdGeCircArc3d Circle {center, normal, radius};
	Ellipse->set(center, Circle.normal(), Circle.refVec() * radius, 1.0);
	return Ellipse;
}

OdDbEllipsePtr EoDbEllipse::Create(OdDbBlockTableRecordPtr& blockTableRecord, EoDbFile& file) {
	const auto Database {blockTableRecord->database()};
	auto Ellipse {OdDbEllipse::createObject()};
	Ellipse->setDatabaseDefaults(Database);
	blockTableRecord->appendOdDbEntity(Ellipse);
	Ellipse->setColorIndex(static_cast<unsigned short>(file.ReadInt16()));
	const auto Linetype {LinetypeObjectFromIndex0(Database, file.ReadInt16())};
	Ellipse->setLinetype(Linetype);
	const auto CenterPoint {file.ReadPoint3d()};
	const auto MajorAxis {file.ReadVector3d()};
	const auto MinorAxis {file.ReadVector3d()};
	const auto SweepAngle {file.ReadDouble()};
	auto PlaneNormal {MajorAxis.crossProduct(MinorAxis)};
	if (!PlaneNormal.isZeroLength()) {
		PlaneNormal.normalize();
		// <tas="Apparently some ellipse primitives have a RadiusRatio > 1."></tas>
		const auto RadiusRatio {MinorAxis.length() / MajorAxis.length()};
		Ellipse->set(CenterPoint, PlaneNormal, MajorAxis, EoMin(1.0, RadiusRatio), 0.0, SweepAngle);
	}
	return Ellipse;
}

OdDbEllipsePtr EoDbEllipse::Create(OdDbBlockTableRecordPtr blockTableRecord, unsigned char* primitiveBuffer, const int versionNumber) {
	short ColorIndex;
	short LinetypeIndex;
	OdGePoint3d CenterPoint;
	OdGeVector3d MajorAxis;
	OdGeVector3d MinorAxis;
	double SweepAngle;
	if (versionNumber == 1) {
		ColorIndex = static_cast<short>(primitiveBuffer[4] & 0x000fU);
		LinetypeIndex = static_cast<short>((primitiveBuffer[4] & 0x00ffU) >> 4U);
		const auto BeginPoint {OdGePoint3d(reinterpret_cast<EoVaxFloat*>(&primitiveBuffer[8])->Convert(), reinterpret_cast<EoVaxFloat*>(&primitiveBuffer[12])->Convert(), 0.0) * 1.e-3};
		CenterPoint = OdGePoint3d(reinterpret_cast<EoVaxFloat*>(& primitiveBuffer[20])->Convert(), reinterpret_cast<EoVaxFloat*>(& primitiveBuffer[24])->Convert(), 0.0) * 1.e-3;
		SweepAngle = reinterpret_cast<EoVaxFloat*>(& primitiveBuffer[28])->Convert();
		if (SweepAngle < 0.0) {
			OdGePoint3d pt;
			pt.x = CenterPoint.x + ((BeginPoint.x - CenterPoint.x) * cos(SweepAngle) - (BeginPoint.y - CenterPoint.y) * sin(SweepAngle));
			pt.y = CenterPoint.y + ((BeginPoint.x - CenterPoint.x) * sin(SweepAngle) + (BeginPoint.y - CenterPoint.y) * cos(SweepAngle));
			MajorAxis = pt - CenterPoint;
		} else {
			MajorAxis = BeginPoint - CenterPoint;
		}
		MinorAxis = OdGeVector3d::kZAxis.crossProduct(MajorAxis);
		SweepAngle = fabs(SweepAngle);
	} else {
		ColorIndex = static_cast<short>(primitiveBuffer[6]);
		LinetypeIndex = static_cast<short>(primitiveBuffer[7]);
		CenterPoint = reinterpret_cast<EoVaxPoint3d*>(& primitiveBuffer[8])->Convert();
		MajorAxis = reinterpret_cast<EoVaxVector3d*>(& primitiveBuffer[20])->Convert();
		MinorAxis = reinterpret_cast<EoVaxVector3d*>(& primitiveBuffer[32])->Convert();
		SweepAngle = reinterpret_cast<EoVaxFloat*>(& primitiveBuffer[44])->Convert();
		if (SweepAngle > Oda2PI || SweepAngle < -Oda2PI) { SweepAngle = Oda2PI; }
	}
	const auto Database {blockTableRecord->database()};
	auto Ellipse {OdDbEllipse::createObject()};
	Ellipse->setDatabaseDefaults(Database);
	blockTableRecord->appendOdDbEntity(Ellipse);
	Ellipse->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Ellipse->setLinetype(LinetypeObjectFromIndex0(Database, LinetypeIndex));
	auto PlaneNormal {MajorAxis.crossProduct(MinorAxis)};
	if (!PlaneNormal.isZeroLength()) {
		PlaneNormal.normalize();
		// <tas="Apparently some ellipse primitives have a RadiusRatio > 1."></tas>
		const auto RadiusRatio {MinorAxis.length() / MajorAxis.length()};
		Ellipse->set(CenterPoint, PlaneNormal, MajorAxis, EoMin(1.0, RadiusRatio), 0.0, SweepAngle);
	}
	return Ellipse;
}

OdGePoint3d FindPointOnArc(const OdGePoint3d& center, const OdGeVector3d& majorAxis, const OdGeVector3d& minorAxis, const double angle) {
	OdGeMatrix3d ScaleMatrix;
	ScaleMatrix.setToScaling(OdGeScale3d(majorAxis.length(), minorAxis.length(), 1.0));
	EoGeMatrix3d PlaneToWorldTransform;
	PlaneToWorldTransform.setToPlaneToWorld(OdGePlane(center, majorAxis, minorAxis));
	PlaneToWorldTransform.postMultBy(ScaleMatrix);
	OdGePoint3d pt(cos(angle), sin(angle), 0.0);
	pt.transformBy(PlaneToWorldTransform);
	return pt;
}

int FindSweepAngleGivenPlaneAnd3Lines(const OdGeVector3d& planeNormal, const OdGePoint3d& firstPoint, const OdGePoint3d& secondPoint, const OdGePoint3d& thirdPoint, const OdGePoint3d& center, double& sweepAngle) {
	double dT[3];
	OdGePoint3d rR[3];
	if (firstPoint == center || secondPoint == center || thirdPoint == center) { return FALSE; }

	// None of the points coincide with center point
	EoGeMatrix3d WorldToPlaneTransform;
	WorldToPlaneTransform.setToWorldToPlane(OdGePlane(center, planeNormal));
	rR[0] = firstPoint;
	rR[1] = secondPoint;
	rR[2] = thirdPoint;
	for (auto i = 0; i < 3; i++) { // Translate points into z=0 plane with center point at origin
		rR[i].transformBy(WorldToPlaneTransform);
		dT[i] = atan2(rR[i].y, rR[i].x);
		if (dT[i] < 0.0) { dT[i] += Oda2PI; }
	}
	const auto dTMin {EoMin(dT[0], dT[2])};
	const auto dTMax {EoMax(dT[0], dT[2])};
	if (fabs(dT[1] - dTMax) > DBL_EPSILON && fabs(dT[1] - dTMin) > DBL_EPSILON) { // Inside line is not colinear with outside lines
		auto dTheta {dTMax - dTMin};
		if (dT[1] > dTMin && dT[1] < dTMax) {
			if (dT[0] == dTMax) { dTheta = -dTheta; }
		} else {
			dTheta = Oda2PI - dTheta;
			if (dT[2] == dTMax) { dTheta = -dTheta; }
		}
		sweepAngle = dTheta;
		return TRUE;
	}
	return FALSE;
}
