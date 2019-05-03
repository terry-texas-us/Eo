#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

#include "EoVaxFloat.h"

#include "EoGePolyline.h"

#include "EoDbFile.h"

#include "DbEllipse.h"
#include "Ge/GeCircArc3d.h"

EoDbEllipse::EoDbEllipse() noexcept
	: m_Center(OdGePoint3d::kOrigin)
	, m_MajorAxis(OdGeVector3d::kXAxis)
	, m_MinorAxis(OdGeVector3d::kYAxis)
	, m_SweepAngle(TWOPI) {
}

EoDbEllipse::EoDbEllipse(const OdGePoint3d& center, const OdGeVector3d& majorAxis, const OdGeVector3d& minorAxis, double sweepAngle) noexcept
	: m_Center(center)
	, m_MajorAxis(majorAxis)
	, m_MinorAxis(minorAxis)
	, m_SweepAngle(sweepAngle) {
	m_ColorIndex = pstate.ColorIndex();
	m_LinetypeIndex = pstate.LinetypeIndex();
}

EoDbEllipse::EoDbEllipse(const OdGePoint3d& center, const OdGeVector3d& planeNormal, double radius)
	: m_Center(center) {
	m_ColorIndex = pstate.ColorIndex();
	m_LinetypeIndex = pstate.LinetypeIndex();

	OdGeVector3d PlaneNormal(planeNormal);
	PlaneNormal.normalize();
	m_MajorAxis = ComputeArbitraryAxis(PlaneNormal);
	m_MajorAxis.normalize();
	m_MajorAxis *= radius;
	m_MinorAxis = PlaneNormal.crossProduct(m_MajorAxis);
	m_SweepAngle = TWOPI;
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

EoDbEllipse::~EoDbEllipse() {
}

const EoDbEllipse& EoDbEllipse::operator=(const EoDbEllipse& other) noexcept {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;

	m_ColorIndex = other.m_ColorIndex;
	m_LinetypeIndex = other.m_LinetypeIndex;
	m_Center = other.m_Center;
	m_MajorAxis = other.m_MajorAxis;
	m_MinorAxis = other.m_MinorAxis;
	m_SweepAngle = other.m_SweepAngle;
	return (*this);
}

void EoDbEllipse::AddReportToMessageList(const OdGePoint3d& point) const {
	CString Report;
	Report += L" Color:" + FormatColorIndex();
	Report += L" Linetype:" + FormatLinetypeIndex();
	CString LengthAsString = theApp.FormatLength(m_MajorAxis.length(), theApp.GetUnits());
	if (fabs(m_MajorAxis.lengthSqrd() - m_MinorAxis.lengthSqrd()) <= FLT_EPSILON) {
		if (fabs(m_SweepAngle - TWOPI) <= FLT_EPSILON) {
			Report = L"<Circle>" + Report;
			Report += L" Radius:" + LengthAsString;
		} else {
			Report = L"<Arc>" + Report;
			Report += L" Radius:" + LengthAsString;
			Report += L" SweepAngle:" + theApp.FormatAngle(m_SweepAngle);
		}
	} else {
		Report = L"<Ellipse>" + Report;
		Report += L" MajorAxisLength:" + LengthAsString;
	}
	theApp.AddStringToMessageList(Report);
}

void EoDbEllipse::AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept {
	CMainFrame::InsertTreeViewControlItem(tree, parent, L"<Arc>", this);
}

EoDbPrimitive* EoDbEllipse::Clone(OdDbDatabasePtr& database) const {
	OdDbBlockTableRecordPtr BlockTableRecord = database->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

	OdDbEllipsePtr Ellipse = m_EntityObjectId.safeOpenObject()->clone();
	BlockTableRecord->appendOdDbEntity(Ellipse);

	return EoDbEllipse::Create(Ellipse);
}

void EoDbEllipse::CutAt(const OdGePoint3d& point, EoDbGroup* newGroup) noexcept {
	if (fabs(m_SweepAngle - TWOPI) <= DBL_EPSILON) {
		// <tas="Never allowing a point cut on closed ellipse"</tas>
	}
	const double Relationship = SwpAngToPt(point) / m_SweepAngle;

	if (Relationship <= DBL_EPSILON || Relationship >= 1. - DBL_EPSILON) {
		return;
	}
	const double SweepAngle = m_SweepAngle * Relationship;

	OdDbDatabasePtr Database = this->m_EntityObjectId.database();
	OdDbBlockTableRecordPtr BlockTableRecord = Database->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

	EoDbEllipse* Arc = EoDbEllipse::Create3(*this, BlockTableRecord);
	Arc->SetTo(m_Center, m_MajorAxis, m_MinorAxis, SweepAngle);
	newGroup->AddTail(Arc);

	OdGeVector3d PlaneNormal = m_MajorAxis.crossProduct(m_MinorAxis);
	PlaneNormal.normalize();

	m_MajorAxis.rotateBy(SweepAngle, PlaneNormal);
	m_MinorAxis.rotateBy(SweepAngle, PlaneNormal);
	m_SweepAngle -= SweepAngle;
	SetTo(m_Center, m_MajorAxis, m_MinorAxis, m_SweepAngle);
}

void EoDbEllipse::CutAt2Points(OdGePoint3d* points, EoDbGroupList* groups, EoDbGroupList* newGroups, OdDbDatabasePtr& database) noexcept {
	EoDbEllipse* pArc;

	double dRel[2];

	dRel[0] = SwpAngToPt(points[0]) / m_SweepAngle;
	dRel[1] = SwpAngToPt(points[1]) / m_SweepAngle;

	if (dRel[0] <= DBL_EPSILON && dRel[1] >= 1. - DBL_EPSILON) { // Put entire arc in trap
		pArc = this;
	} else { // Something gets cut
		OdDbBlockTableRecordPtr BlockTableRecord = database->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

		OdGeVector3d vPlnNorm = m_MajorAxis.crossProduct(m_MinorAxis);
		vPlnNorm.normalize();

		if (fabs(m_SweepAngle - TWOPI) <= DBL_EPSILON) { // Closed arc
			m_SweepAngle = (dRel[1] - dRel[0]) * TWOPI;

			m_MajorAxis.rotateBy(dRel[0] * TWOPI, vPlnNorm);
			m_MinorAxis.rotateBy(dRel[0] * TWOPI, vPlnNorm);

			pArc = EoDbEllipse::Create3(*this, BlockTableRecord);

			m_MajorAxis.rotateBy(m_SweepAngle, vPlnNorm);
			m_MinorAxis.rotateBy(m_SweepAngle, vPlnNorm);

			m_SweepAngle = TWOPI - m_SweepAngle;
		} else { // Arc section with a cut
			pArc = EoDbEllipse::Create3(*this, BlockTableRecord);
			const double dSwpAng = m_SweepAngle;

			const double dAng1 = dRel[0] * m_SweepAngle;
			const double dAng2 = dRel[1] * m_SweepAngle;

			if (dRel[0] > DBL_EPSILON && dRel[1] < 1. - DBL_EPSILON) { // Cut section out of middle
				pArc->SetSweepAngle(dAng1);
				EoDbGroup* Group = new EoDbGroup;
				Group->AddTail(pArc);
				groups->AddTail(Group);

				m_MajorAxis.rotateBy(dAng1, vPlnNorm);
				m_MinorAxis.rotateBy(dAng1, vPlnNorm);
				m_SweepAngle = dAng2 - dAng1;

				pArc = EoDbEllipse::Create3(*this, BlockTableRecord);

				m_MajorAxis.rotateBy(m_SweepAngle, vPlnNorm);
				m_MinorAxis.rotateBy(m_SweepAngle, vPlnNorm);
				m_SweepAngle = dSwpAng - dAng2;
			} else if (dRel[1] < 1. - DBL_EPSILON) { // Cut section in two and place begin section in trap
				pArc->SetSweepAngle(dAng2);

				m_MajorAxis.rotateBy(dAng2, vPlnNorm);
				m_MinorAxis.rotateBy(dAng2, vPlnNorm);
				m_SweepAngle = dSwpAng - dAng2;
			} else { // Cut section in two and place end section in trap
				m_SweepAngle = dAng1;

				OdGeVector3d v = m_MajorAxis;
				v.rotateBy(dAng1, vPlnNorm);
				pArc->SetMajorAxis(v);
				v = m_MinorAxis;
				v.rotateBy(dAng1, vPlnNorm);
				pArc->SetMinorAxis(v);
				pArc->SetSweepAngle(dSwpAng - dAng1);
			}
		}
		EoDbGroup* Group = new EoDbGroup;
		Group->AddTail(this);
		groups->AddTail(Group);
	}
	EoDbGroup* NewGroup = new EoDbGroup;
	NewGroup->AddTail(pArc);
	newGroups->AddTail(NewGroup);
}

void EoDbEllipse::Display(AeSysView * view, CDC * deviceContext) {
	if (fabs(m_SweepAngle) <= DBL_EPSILON) return;

	const OdInt16 ColorIndex = LogicalColorIndex();
	const OdInt16 LinetypeIndex = LogicalLinetypeIndex();

	pstate.SetPen(view, deviceContext, ColorIndex, LinetypeIndex);

	polyline::BeginLineStrip();
	GenPts(OdGePlane(m_Center, m_MajorAxis, m_MinorAxis), m_SweepAngle);
	polyline::__End(view, deviceContext, LinetypeIndex);
}

void EoDbEllipse::FormatExtra(CString & extra) const {
	extra.Empty();
	extra += L"Color;" + FormatColorIndex() + L"\t";
	extra += L"Linetype;" + FormatLinetypeIndex() + L"\t";
	extra += L"Sweep Angle;" + theApp.FormatAngle(m_SweepAngle) + L"\t";
	extra += L"Major Axis Length;" + theApp.FormatLength(m_MajorAxis.length(), theApp.GetUnits());
}

void EoDbEllipse::FormatGeometry(CString & geometry) const {
	const OdGeVector3d Normal = m_MajorAxis.crossProduct(m_MinorAxis);
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

void EoDbEllipse::GenPts(const OdGePlane & plane, double sweepAngle) const {
	OdGeMatrix3d ScaleMatrix;
	ScaleMatrix.setToScaling(OdGeScale3d(m_MajorAxis.length(), m_MinorAxis.length(), 1.));

	OdGeMatrix3d PlaneToWorldTransform;
	PlaneToWorldTransform.setToPlaneToWorld(plane); // <tas=Builds a matrix which performs rotation and translation, but no scaling.</tas>

	// Number of points based on angle and a smothness coefficient
	const double dLen = EoMax(m_MajorAxis.length(), m_MinorAxis.length());
	int iPts = EoMax(2, abs(EoRound(sweepAngle / TWOPI * 32.)));
	iPts = EoMin(128, EoMax(iPts, abs(EoRound(sweepAngle * dLen / .250))));

	const double dAng = m_SweepAngle / (double(iPts) - 1.);
	const double dCos = cos(dAng);
	const double dSin = sin(dAng);

	// Generate an origin-centered unit radial curve, then scale before transforming back the world

	OdGePoint3d pt(1., 0., 0.);

	for (int i = 0; i < iPts; i++) {
		polyline::SetVertex(PlaneToWorldTransform * ScaleMatrix * pt);

		const double X = pt.x;
		pt.x = X * dCos - pt.y * dSin;
		pt.y = pt.y * dCos + X * dSin;
		pt.z = 0.;
	}
}

void EoDbEllipse::GetAllPoints(OdGePoint3dArray & points) const {
	points.clear();
	points.append(m_Center);
}

OdGePoint3d EoDbEllipse::GetCtrlPt() const noexcept {
	return (m_Center);
}

OdGePoint3d EoDbEllipse::StartPoint() const {
	return (m_Center + m_MajorAxis);
}

void EoDbEllipse::GetBoundingBox(OdGePoint3dArray & ptsBox) const {
	ptsBox.setLogicalLength(4);
	ptsBox[0] = OdGePoint3d(-1., -1., 0.);
	ptsBox[1] = OdGePoint3d(1., -1., 0.);
	ptsBox[2] = OdGePoint3d(1., 1., 0.);
	ptsBox[3] = OdGePoint3d(-1., 1., 0.);

	if (m_SweepAngle < 3. * TWOPI / 4.) {
		const double dEndX = cos(m_SweepAngle);
		const double dEndY = sin(m_SweepAngle);

		if (dEndX >= 0.) {
			if (dEndY >= 0.) { // Arc ends in quadrant one
				ptsBox[0].x = dEndX;
				ptsBox[0].y = 0.;
				ptsBox[1].y = 0.;
				ptsBox[2].y = dEndY;
				ptsBox[3].x = dEndX;
				ptsBox[3].y = dEndY;
			}
		} else {
			if (dEndY >= 0.) { // Arc ends in quadrant two
				ptsBox[0].x = dEndX;
				ptsBox[0].y = 0.;
				ptsBox[1].y = 0.;
				ptsBox[3].x = dEndX;
			} else { // Arc ends in quadrant three
				ptsBox[0].y = dEndY;
				ptsBox[1].y = dEndY;
			}
		}
	}
	OdGeMatrix3d ScaleMatrix;
	ScaleMatrix.setToScaling(OdGeScale3d(m_MajorAxis.length(), m_MinorAxis.length(), 1.));

	OdGeMatrix3d PlaneToWorldTransform;
	PlaneToWorldTransform.setToPlaneToWorld(OdGePlane(m_Center, m_MajorAxis, m_MinorAxis));
	PlaneToWorldTransform.postMultBy(ScaleMatrix);

	for (OdUInt16 w = 0; w < 4; w++) {
		ptsBox[w].transformBy(PlaneToWorldTransform);
	}
}

OdGePoint3d EoDbEllipse::EndPoint() const {
	OdGeMatrix3d ScaleMatrix;
	ScaleMatrix.setToScaling(OdGeScale3d(m_MajorAxis.length(), m_MinorAxis.length(), 1.));

	EoGeMatrix3d PlaneToWorldTransform;
	PlaneToWorldTransform.setToPlaneToWorld(OdGePlane(m_Center, m_MajorAxis, m_MinorAxis));
	PlaneToWorldTransform.postMultBy(ScaleMatrix);

	OdGePoint3d pt(cos(m_SweepAngle), sin(m_SweepAngle), 0.);

	pt.transformBy(PlaneToWorldTransform);
	return (pt);
}

OdGeVector3d EoDbEllipse::MajorAxis() const noexcept {
	return m_MajorAxis;
}

OdGeVector3d EoDbEllipse::MinorAxis() const noexcept {
	return m_MinorAxis;
}

OdGePoint3d EoDbEllipse::Center() const noexcept {
	return (m_Center);
}

double EoDbEllipse::SweepAngle() const noexcept {
	return m_SweepAngle;
}

void EoDbEllipse::GetXYExtents(OdGePoint3d arBeg, OdGePoint3d arEnd, OdGePoint3d * arMin, OdGePoint3d * arMax) noexcept {

	const double dx = double(m_Center.x - arBeg.x);
	const double dy = double(m_Center.y - arBeg.y);

	const double dRad = sqrt(dx * dx + dy * dy);

	(*arMin).x = m_Center.x - dRad;
	(*arMin).y = m_Center.y - dRad;
	(*arMax).x = m_Center.x + dRad;
	(*arMax).y = m_Center.y + dRad;

	if (arBeg.x >= m_Center.x) {
		if (arBeg.y >= m_Center.y) { // Arc begins in quadrant one
			if (arEnd.x >= m_Center.x) {
				if (arEnd.y >= m_Center.y) { // Arc ends in quadrant one
					if (arBeg.x > arEnd.x) { // Arc in qraudrant one only
						(*arMin).x = arEnd.x;
						(*arMin).y = arBeg.y;
						(*arMax).x = arBeg.x;
						(*arMax).y = arEnd.y;
					}
				} else											// Arc ends in quadrant four
					(*arMax).x = EoMax(arBeg.x, arEnd.x);
			} else {
				if (arEnd.y >= m_Center.y) { // Arc ends in quadrant two
					(*arMin).x = arEnd.x;
					(*arMin).y = EoMin(arBeg.y, arEnd.y);
				} else // Arc ends in quadrant three
					(*arMin).y = arEnd.y;
				(*arMax).x = arBeg.x;
			}
		} else { // Arc begins in quadrant four
			if (arEnd.x >= m_Center.x) {
				if (arEnd.y >= m_Center.y) { // Arc ends in quadrant one
					(*arMin).x = EoMin(arBeg.x, arEnd.x);
					(*arMin).y = arBeg.y;
					(*arMax).y = arEnd.y;
				} else { // Arc ends in quadrant four
					if (arBeg.x < arEnd.x) { // Arc in qraudrant one only
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
				} else											// Arc ends in quadrant three
					(*arMin).y = EoMin(arBeg.y, arEnd.y);
			}
		}
	} else {
		if (arBeg.y >= m_Center.y) { // Arc begins in quadrant two
			if (arEnd.x >= m_Center.x) {
				if (arEnd.y >= m_Center.y) // Arc ends in quadrant one
					(*arMax).y = EoMax(arBeg.y, arEnd.y);
				else { // Arc ends in quadrant four
					(*arMax).x = arEnd.x;
					(*arMax).y = arBeg.y;
				}
			} else {
				if (arEnd.y >= m_Center.y) { // Arc ends in quadrant two
					if (arBeg.x > arEnd.x) { // Arc in qraudrant two only
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
				if (arEnd.y >= m_Center.y) // Arc ends in quadrant one
					(*arMax).y = arEnd.y;
				else { // Arc ends in quadrant four
					(*arMax).x = arEnd.x;
					(*arMax).y = EoMax(arBeg.y, arEnd.y);
				}
				(*arMin).x = arBeg.x;
			} else {
				if (arEnd.y >= m_Center.y)							// Arc ends in quadrant two
					(*arMin).x = EoMin(arBeg.x, arEnd.x);
				else { // Arc ends in quadrant three
					if (arBeg.x < arEnd.x) { // Arc in qraudrant three only
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

void EoDbEllipse::GetExtents(AeSysView * view, OdGeExtents3d & extents) const {
	if (!m_EntityObjectId.isNull()) {
		OdDbObjectPtr Entity = m_EntityObjectId.safeOpenObject();
		OdGeExtents3d Extents;
		Entity->getGeomExtents(Extents);
		extents.addExt(Extents);
	} else {
		OdGePoint3dArray BoundingBox;
		GetBoundingBox(BoundingBox);

		for (OdUInt16 w = 0; w < 4; w++) {
			extents.addPoint(BoundingBox[w]);
		}
	}
}

bool EoDbEllipse::IsPointOnControlPoint(AeSysView * view, const EoGePoint4d & point) const {
	// Determines if a point is on a control point of the arc.

	EoGePoint4d pt[] = {EoGePoint4d(StartPoint(), 1.), EoGePoint4d(EndPoint(), 1.)};

	for (OdUInt16 w = 0; w < 2; w++) {
		view->ModelViewTransformPoint(pt[w]);

		if (point.DistanceToPointXY(pt[w]) < sm_SelectApertureSize)
			return true;
	}
	return false;
}

int EoDbEllipse::IsWithinArea(const OdGePoint3d & lowerLeftCorner, const OdGePoint3d & upperRightCorner, OdGePoint3d * intersections) noexcept {
	OdGeVector3d vPlnNorm = m_MajorAxis.crossProduct(m_MinorAxis);
	vPlnNorm.normalize();

	if (!(OdGeVector3d::kZAxis.crossProduct(vPlnNorm)).isZeroLength())
		// not on plane normal to z-axis
		return 0;

	if (fabs(m_MajorAxis.length() - m_MinorAxis.length()) > FLT_EPSILON)
		// not radial
		return 0;

	OdGePoint3d ptMin;
	OdGePoint3d ptMax;

	OdGePoint3d ptBeg = StartPoint();
	OdGePoint3d ptEnd = EndPoint();

	if (vPlnNorm.z < 0.) {
		const OdGePoint3d pt = ptBeg;
		ptBeg = ptEnd;
		ptEnd = pt;

		vPlnNorm = -vPlnNorm;
		m_MajorAxis = ptBeg - m_Center;
		m_MinorAxis = vPlnNorm.crossProduct(m_MajorAxis);
	}

	GetXYExtents(ptBeg, ptEnd, &ptMin, &ptMax);

	if (ptMin.x >= lowerLeftCorner.x && ptMax.x <= upperRightCorner.x && ptMin.y >= lowerLeftCorner.y && ptMax.y <= upperRightCorner.y) { // Totally within window boundaries
		intersections[0] = ptBeg;
		intersections[1] = ptEnd;
		return (2);
	}
	if (ptMin.x >= upperRightCorner.x || ptMax.x <= lowerLeftCorner.x || ptMin.y >= upperRightCorner.y || ptMax.y <= lowerLeftCorner.y)
		// No extent overlap
		return 0;

	OdGePoint3d ptWrk[8];

	double dDis;
	double dOff;
	int iSecs = 0;

	const double dRad = OdGeVector3d(ptBeg - m_Center).length();
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
	if (iSecs == 0)
		return 0;

	const double dBegAng = atan2(ptBeg.y - m_Center.y, ptBeg.x - m_Center.x); // Arc begin angle (- pi to pi)

	double dIntAng[8];
	double dWrkAng;
	int iInts = 0;
	for (int i2 = 0; i2 < iSecs; i2++) { // Loop thru possible intersections
		dWrkAng = atan2(ptWrk[i2].y - m_Center.y, ptWrk[i2].x - m_Center.x); // Current intersection angle (- pi to
		dIntAng[iInts] = dWrkAng - dBegAng; 					// Sweep from begin to intersection
		if (dIntAng[iInts] < 0.)
			dIntAng[iInts] += TWOPI;
		if (fabs(dIntAng[iInts]) - m_SweepAngle < 0.) { // Intersection lies on arc
			int i;
			for (i = 0; i < iInts && ptWrk[i2] != intersections[i]; i++);
			if (i == iInts) 								// Unique intersection
				intersections[iInts++] = ptWrk[i2];
		}
	}
	if (iInts == 0)
		// None of the intersections are on sweep of arc
		return 0;

	for (int i1 = 0; i1 < iInts; i1++) { // Sort intersections from begin to end of sweep
		for (int i2 = 1; i2 < iInts - i1; i2++) {
			if (fabs(dIntAng[i2]) < fabs(dIntAng[i2 - 1])) {
				const double dAng = dIntAng[i2 - 1];
				dIntAng[i2 - 1] = dIntAng[i2];
				dIntAng[i2] = dAng;
				const OdGePoint3d pt = intersections[i2 - 1];
				intersections[i2 - 1] = intersections[i2];
				intersections[i2] = pt;
			}
		}
	}
	if (fabs(m_SweepAngle - TWOPI) <= DBL_EPSILON) { // Arc is a circle in disuise

	} else {
		if (ptBeg.x >= lowerLeftCorner.x && ptBeg.x <= upperRightCorner.x && ptBeg.y >= lowerLeftCorner.y && ptBeg.y <= upperRightCorner.y) { // Add beg point to int set
			for (int i = iInts; i > 0; i--)
				intersections[i] = intersections[i - 1];
			intersections[0] = ptBeg;
			iInts++;
		}
		if (ptEnd.x >= lowerLeftCorner.x && ptEnd.x <= upperRightCorner.x && ptEnd.y >= lowerLeftCorner.y && ptEnd.y <= upperRightCorner.y) { // Add end point to int set
			intersections[iInts] = ptEnd;
			iInts++;
		}
	}
	return (iInts);
}

OdGePoint3d EoDbEllipse::GoToNxtCtrlPt() const {
	const double dAng = (sm_RelationshipOfPoint <= DBL_EPSILON) ? m_SweepAngle : 0.;
	return (pFndPtOnArc(m_Center, m_MajorAxis, m_MinorAxis, dAng));
}

bool EoDbEllipse::IsEqualTo(EoDbPrimitive * primitive) const noexcept {
	return false;
}

bool EoDbEllipse::IsInView(AeSysView * view) const {
	OdGePoint3dArray BoundingBox;

	GetBoundingBox(BoundingBox);

	EoGePoint4d ptBeg(BoundingBox[0], 1.);
	view->ModelViewTransformPoint(ptBeg);

	for (OdUInt16 w = 1; w < 4; w++) {
		EoGePoint4d ptEnd(BoundingBox[w], 1.);
		view->ModelViewTransformPoint(ptEnd);

		if (EoGePoint4d::ClipLine(ptBeg, ptEnd))
			return true;

		ptBeg = ptEnd;
	}
	return false;
}

OdGePoint3d EoDbEllipse::SelectAtControlPoint(AeSysView * view, const EoGePoint4d & point) const {
	sm_ControlPointIndex = SIZE_T_MAX;

	double dAPert = sm_SelectApertureSize;

	OdGePoint3d ptCtrl[] = {StartPoint(), EndPoint()};

	for (OdUInt16 w = 0; w < 2; w++) {
		EoGePoint4d pt(ptCtrl[w], 1.);

		view->ModelViewTransformPoint(pt);

		const double dDis = point.DistanceToPointXY(pt);

		if (dDis < dAPert) {
			sm_ControlPointIndex = w;
			dAPert = dDis;
		}
	}
	return (sm_ControlPointIndex == SIZE_T_MAX) ? OdGePoint3d::kOrigin : ptCtrl[sm_ControlPointIndex];
}

bool EoDbEllipse::SelectBy(const EoGeLineSeg3d & line, AeSysView * view, OdGePoint3dArray & intersections) {
	polyline::BeginLineStrip();
	GenPts(OdGePlane(m_Center, m_MajorAxis, m_MinorAxis), m_SweepAngle);
	return polyline::SelectBy(line, view, intersections);
}

bool EoDbEllipse::SelectBy(const EoGePoint4d & point, AeSysView * view, OdGePoint3d & ptProj) const {
	polyline::BeginLineStrip();
	GenPts(OdGePlane(m_Center, m_MajorAxis, m_MinorAxis), m_SweepAngle);
	return (polyline::SelectBy(point, view, sm_RelationshipOfPoint, ptProj));
}

bool EoDbEllipse::SelectBy(const OdGePoint3d & lowerLeftCorner, const OdGePoint3d & upperRightCorner, AeSysView * view) const {
	polyline::BeginLineStrip();
	GenPts(OdGePlane(m_Center, m_MajorAxis, m_MinorAxis), m_SweepAngle);
	return polyline::SelectBy(lowerLeftCorner, upperRightCorner, view);
}

void EoDbEllipse::SetCenter(const OdGePoint3d & center) noexcept {
	m_Center = center;
}

void EoDbEllipse::SetMajorAxis(const OdGeVector3d & majorAxis) noexcept {
	m_MajorAxis = majorAxis;
}

void EoDbEllipse::SetMinorAxis(const OdGeVector3d & minorAxis) noexcept {
	m_MinorAxis = minorAxis;
}

void EoDbEllipse::SetSweepAngle(double angle) noexcept {
	m_SweepAngle = angle;
}

EoDbEllipse& EoDbEllipse::SetTo(const OdGePoint3d & center, const OdGeVector3d & majorAxis, const OdGeVector3d & minorAxis, double sweepAngle) {
	OdGeVector3d PlaneNormal = majorAxis.crossProduct(minorAxis);
	if (!PlaneNormal.isZeroLength()) {
		m_Center = center;
		m_MajorAxis = majorAxis;
		m_MinorAxis = minorAxis;
		m_SweepAngle = sweepAngle;

		if (!m_EntityObjectId.isNull()) {
			OdDbEllipsePtr Ellipse = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
			const double RadiusRatio = minorAxis.length() / majorAxis.length();
			PlaneNormal.normalize();
			Ellipse->set(center, PlaneNormal, majorAxis, RadiusRatio, 0., sweepAngle);
		}
	}
	return *this;
}

EoDbEllipse& EoDbEllipse::SetTo3PointArc(const OdGePoint3d & startPoint, const OdGePoint3d & intermediatePoint, const OdGePoint3d & endPoint) {
	m_SweepAngle = 0.;

	OdGeVector3d PlaneNormal = OdGeVector3d(intermediatePoint - startPoint).crossProduct(OdGeVector3d(endPoint - startPoint));
	if (PlaneNormal.isZeroLength()) {
		return *this;
	}
	PlaneNormal.normalize();

	// Build transformation matrix which will get int and end points to z=0 plane with beg point as origin

	EoGeMatrix3d tm;
	tm.setToWorldToPlane(OdGePlane(startPoint, PlaneNormal));

	OdGePoint3d pt[3];

	pt[0] = startPoint;
	pt[1] = intermediatePoint;
	pt[2] = endPoint;

	pt[1].transformBy(tm);
	pt[2].transformBy(tm);

	const double dDet = (pt[1].x * pt[2].y - pt[2].x * pt[1].y);

	if (fabs(dDet) > DBL_EPSILON) { // Three points are not colinear
		const double dT = ((pt[2].x - pt[1].x) * pt[2].x + pt[2].y * (pt[2].y - pt[1].y)) / dDet;

		m_Center.x = (pt[1].x - pt[1].y * dT) * .5;
		m_Center.y = (pt[1].y + pt[1].x * dT) * .5;
		m_Center.z = 0.;
		tm.invert();

		// Transform back to original plane
		m_Center.transformBy(tm);

		// None of the points coincide with center point

		EoGeMatrix3d tm;
		tm.setToWorldToPlane(OdGePlane(m_Center, PlaneNormal));
		double dAng[3];

		pt[1] = intermediatePoint;
		pt[2] = endPoint;

		for (int i = 0; i < 3; i++) { // Translate points into z=0 plane with center point at origin
			pt[i].transformBy(tm);
			dAng[i] = atan2(pt[i].y, pt[i].x);
			if (dAng[i] < 0.)
				dAng[i] += TWOPI;
		}
		const double dMin = EoMin(dAng[0], dAng[2]);
		const double dMax = EoMax(dAng[0], dAng[2]);

		if (fabs(dAng[1] - dMax) > DBL_EPSILON && fabs(dAng[1] - dMin) > DBL_EPSILON) { // Inside line is not colinear with outside lines
			m_SweepAngle = dMax - dMin;
			if (dAng[1] > dMin && dAng[1] < dMax) {
				if (dAng[0] == dMax)
					m_SweepAngle = -m_SweepAngle;
			} else {
				m_SweepAngle = TWOPI - m_SweepAngle;
				if (dAng[2] == dMax)
					m_SweepAngle = -m_SweepAngle;
			}
			OdGePoint3d ptRot = startPoint;
			ptRot.rotateBy(HALF_PI, PlaneNormal, m_Center);

			m_MajorAxis = OdGeVector3d(startPoint - m_Center);
			m_MinorAxis = OdGeVector3d(ptRot - m_Center);

			SetTo(m_Center, m_MajorAxis, m_MinorAxis, m_SweepAngle);
		}
	}
	return *this;
}

EoDbEllipse & EoDbEllipse::SetToCircle(const OdGePoint3d & center, const OdGeVector3d & planeNormal, double radius) {
	if (!planeNormal.isZeroLength()) {
		OdGeVector3d PlaneNormal(planeNormal);
		PlaneNormal.normalize();
		m_Center = center;
		m_MajorAxis = ComputeArbitraryAxis(PlaneNormal);
		m_MajorAxis.normalize();
		m_MajorAxis *= radius;
		m_MinorAxis = PlaneNormal.crossProduct(m_MajorAxis);
		m_SweepAngle = TWOPI;

		SetTo(m_Center, m_MajorAxis, m_MinorAxis, m_SweepAngle);
	}
	return *this;
}

double EoDbEllipse::SwpAngToPt(const OdGePoint3d & point) {
	OdGeVector3d PlaneNormal = m_MajorAxis.crossProduct(m_MinorAxis);
	PlaneNormal.normalize();

	EoGeMatrix3d tm;
	tm.setToWorldToPlane(OdGePlane(m_Center, PlaneNormal));

	OdGePoint3d StartPoint = m_Center + m_MajorAxis;
	OdGePoint3d Point = point;

	// Translate points into z=0 plane
	StartPoint.transformBy(tm);
	Point.transformBy(tm);

	return (EoGeLineSeg3d(OdGePoint3d::kOrigin, StartPoint).AngleBetween_xy(EoGeLineSeg3d(OdGePoint3d::kOrigin, Point)));
}

void EoDbEllipse::TransformBy(const EoGeMatrix3d & transformMatrix) {
	m_Center.transformBy(transformMatrix);
	m_MajorAxis.transformBy(transformMatrix);
	m_MinorAxis.transformBy(transformMatrix);
}

void EoDbEllipse::TranslateUsingMask(const OdGeVector3d & translate, const DWORD mask) {
	if (mask != 0)
		m_Center += translate;
}

bool EoDbEllipse::Write(EoDbFile & file) const {
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

void EoDbEllipse::Write(CFile & file, OdUInt8 * buffer) const {
	buffer[3] = 2;
	*((OdUInt16*) & buffer[4]) = OdUInt16(EoDb::kEllipsePrimitive);
	buffer[6] = OdInt8(m_ColorIndex == COLORINDEX_BYLAYER ? sm_LayerColorIndex : m_ColorIndex);
	buffer[7] = OdInt8(m_LinetypeIndex == LINETYPE_BYLAYER ? sm_LayerLinetypeIndex : m_LinetypeIndex);
	if (buffer[7] >= 16) buffer[7] = 2;

	((EoVaxPoint3d*) & buffer[8])->Convert(m_Center);
	((EoVaxVector3d*) & buffer[20])->Convert(m_MajorAxis);
	((EoVaxVector3d*) & buffer[32])->Convert(m_MinorAxis);
	((EoVaxFloat*) & buffer[44])->Convert(m_SweepAngle);

	file.Write(buffer, 64);
}

EoDbEllipse * EoDbEllipse::ConstructFrom(OdUInt8 * primitiveBufer, int versionNumber) {
	OdInt16 ColorIndex;
	OdInt16 LinetypeIndex;
	OdGePoint3d CenterPoint;
	OdGeVector3d MajorAxis;
	OdGeVector3d MinorAxis;
	double SweepAngle;

	if (versionNumber == 1) {
		ColorIndex = OdInt16(primitiveBufer[4] & 0x000f);
		LinetypeIndex = OdInt16((primitiveBufer[4] & 0x00ff) >> 4);

		OdGePoint3d BeginPoint;
		BeginPoint = OdGePoint3d(((EoVaxFloat*) & primitiveBufer[8])->Convert(), ((EoVaxFloat*) & primitiveBufer[12])->Convert(), 0.) * 1.e-3;
		CenterPoint = OdGePoint3d(((EoVaxFloat*) & primitiveBufer[20])->Convert(), ((EoVaxFloat*) & primitiveBufer[24])->Convert(), 0.) * 1.e-3;
		SweepAngle = ((EoVaxFloat*) & primitiveBufer[28])->Convert();

		if (SweepAngle < 0.) {
			OdGePoint3d pt;
			pt.x = (CenterPoint.x + ((BeginPoint.x - CenterPoint.x) * cos(SweepAngle) - (BeginPoint.y - CenterPoint.y) * sin(SweepAngle)));
			pt.y = (CenterPoint.y + ((BeginPoint.x - CenterPoint.x) * sin(SweepAngle) + (BeginPoint.y - CenterPoint.y) * cos(SweepAngle)));
			MajorAxis = pt - CenterPoint;
		} else {
			MajorAxis = BeginPoint - CenterPoint;
		}
		MinorAxis = OdGeVector3d::kZAxis.crossProduct(MajorAxis);
		SweepAngle = fabs(SweepAngle);
	} else {
		ColorIndex = OdInt16(primitiveBufer[6]);
		LinetypeIndex = OdInt16(primitiveBufer[7]);

		CenterPoint = ((EoVaxPoint3d*) & primitiveBufer[8])->Convert();
		MajorAxis = ((EoVaxVector3d*) & primitiveBufer[20])->Convert();
		MinorAxis = ((EoVaxVector3d*) & primitiveBufer[32])->Convert();

		SweepAngle = ((EoVaxFloat*) & primitiveBufer[44])->Convert();

		if (SweepAngle > TWOPI || SweepAngle < -TWOPI) {
			SweepAngle = TWOPI;
		}
	}
	auto Ellipse {new EoDbEllipse(CenterPoint, MajorAxis, MinorAxis, SweepAngle)};
	Ellipse->SetColorIndex_(ColorIndex);
	Ellipse->SetLinetypeIndex_(LinetypeIndex);

	return (Ellipse);
}

EoDbEllipse* EoDbEllipse::Create0(OdDbBlockTableRecordPtr & blockTableRecord) {
	auto Ellipse {OdDbEllipse::createObject()};
	Ellipse->setDatabaseDefaults(blockTableRecord->database());

	blockTableRecord->appendOdDbEntity(Ellipse);
	Ellipse->setColorIndex(pstate.ColorIndex());

	const auto Linetype {EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex())};

	Ellipse->setLinetype(Linetype);

	return EoDbEllipse::Create(Ellipse);
}

EoDbEllipse* EoDbEllipse::Create3(const EoDbEllipse & other, OdDbBlockTableRecordPtr & blockTableRecord) {
	// <tas="Possibly need additional typing of the ObjecId producted by cloning"></tas>
	OdDbEllipsePtr EllipseEntity = other.EntityObjectId().safeOpenObject()->clone();
	blockTableRecord->appendOdDbEntity(EllipseEntity);

	EoDbEllipse* Ellipse = new EoDbEllipse(other);
	Ellipse->SetEntityObjectId(EllipseEntity->objectId());

	return Ellipse;
}

EoDbEllipse* EoDbEllipse::Create(OdDbEllipsePtr & ellipse) {
	auto Ellipse {new EoDbEllipse()};
	Ellipse->SetEntityObjectId(ellipse->objectId());

	Ellipse->m_ColorIndex = ellipse->colorIndex();
	Ellipse->m_LinetypeIndex = EoDbLinetypeTable::LegacyLinetypeIndex(ellipse->linetype());

	OdGeVector3d MajorAxis(ellipse->majorAxis());
	OdGeVector3d MinorAxis(ellipse->minorAxis());

	double StartAngle = ellipse->startAngle();
	double EndAngle = ellipse->endAngle();

	if (StartAngle >= TWOPI) { // need to rationalize angs to first period angles in range on (0 to twopi)
		StartAngle -= TWOPI;
		EndAngle -= TWOPI;
	}
	double SweepAngle = EndAngle - StartAngle;
	if (SweepAngle <= FLT_EPSILON)
		SweepAngle += TWOPI;

	if (StartAngle != 0.) {
		MajorAxis.rotateBy(StartAngle, ellipse->normal());
		MinorAxis.rotateBy(StartAngle, ellipse->normal());
		if (ellipse->radiusRatio() != 1.) {
			ATLTRACE2(atlTraceGeneral, 0, L"Ellipse: Non radial with start parameter not 0.\n");
		}
	}
	Ellipse->SetCenter(ellipse->center());
	Ellipse->SetMajorAxis(MajorAxis);
	Ellipse->SetMinorAxis(MinorAxis);
	Ellipse->SetSweepAngle(SweepAngle);

	return Ellipse;
}

OdDbEllipsePtr EoDbEllipse::Create(OdDbBlockTableRecordPtr & blockTableRecord) {
	auto Ellipse {OdDbEllipse::createObject()};
	Ellipse->setDatabaseDefaults(blockTableRecord->database());

	blockTableRecord->appendOdDbEntity(Ellipse);
	Ellipse->setColorIndex(pstate.ColorIndex());

	const auto Linetype {EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex())};

	Ellipse->setLinetype(Linetype);

	return Ellipse;
}

OdDbEllipsePtr EoDbEllipse::CreateCircle(OdDbBlockTableRecordPtr & blockTableRecord, const OdGePoint3d & center, const OdGeVector3d & normal, double radius) {
	auto Ellipse {OdDbEllipse::createObject()};
	Ellipse->setDatabaseDefaults(blockTableRecord->database());

	blockTableRecord->appendOdDbEntity(Ellipse);
	OdGeCircArc3d Circle(center, normal, radius);

	Ellipse->set(center, Circle.normal(), Circle.refVec() * radius, 1.);
	return Ellipse;
}

OdDbEllipsePtr EoDbEllipse::Create(OdDbBlockTableRecordPtr & blockTableRecord, EoDbFile & file) {
	auto Database {blockTableRecord->database()};

	auto Ellipse {OdDbEllipse::createObject()};
	Ellipse->setDatabaseDefaults(Database);

	blockTableRecord->appendOdDbEntity(Ellipse);

	Ellipse->setColorIndex(file.ReadInt16());

	const auto Linetype {EoDbPrimitive::LinetypeObjectFromIndex0(Database, file.ReadInt16())};

	Ellipse->setLinetype(Linetype);

	const auto CenterPoint(file.ReadPoint3d());
	const auto MajorAxis(file.ReadVector3d());
	const auto MinorAxis(file.ReadVector3d());

	const auto SweepAngle = file.ReadDouble();

	auto PlaneNormal {MajorAxis.crossProduct(MinorAxis)};
	if (!PlaneNormal.isZeroLength()) {
		PlaneNormal.normalize();
		// <tas="Apparently some ellipse primitives have a RadiusRatio > 1."></tas>
		const double RadiusRatio = MinorAxis.length() / MajorAxis.length();
		Ellipse->set(CenterPoint, PlaneNormal, MajorAxis, EoMin(1., RadiusRatio), 0., SweepAngle);
	}
	return Ellipse;
}

OdGePoint3d pFndPtOnArc(const OdGePoint3d & center, const OdGeVector3d & majorAxis, const OdGeVector3d & minorAxis, const double dAng) {
	OdGeMatrix3d ScaleMatrix;
	ScaleMatrix.setToScaling(OdGeScale3d(majorAxis.length(), minorAxis.length(), 1.));

	EoGeMatrix3d PlaneToWorldTransform;
	PlaneToWorldTransform.setToPlaneToWorld(OdGePlane(center, majorAxis, minorAxis));
	PlaneToWorldTransform.postMultBy(ScaleMatrix);

	OdGePoint3d pt(cos(dAng), sin(dAng), 0.);

	pt.transformBy(PlaneToWorldTransform);
	return (pt);
}

bool pFndCPGivRadAnd4Pts(double radius, OdGePoint3d arLn1Beg, OdGePoint3d arLn1End, OdGePoint3d arLn2Beg, OdGePoint3d arLn2End, OdGePoint3d * center) {
	const OdGeVector3d v1 = OdGeVector3d(arLn1End - arLn1Beg); // Determine vector defined by endpoints of first line
	auto dV1Mag {v1.length()};
	if (dV1Mag <= DBL_EPSILON)
		return false;

	OdGeVector3d v2 = OdGeVector3d(arLn2End - arLn2Beg);
	auto dV2Mag {v2.length()};
	if (dV2Mag <= DBL_EPSILON)
		return false;

	auto vPlnNorm {v1.crossProduct(v2)}; // Determine vector normal to tangent vectors
	if (vPlnNorm.isZeroLength()) {
		return false;
	}
	vPlnNorm.normalize();

	if (fabs((vPlnNorm.dotProduct(OdGeVector3d(arLn2Beg - arLn1Beg)))) > DBL_EPSILON) // Four points are not coplanar
		return false;

	EoGeMatrix3d WorldToPlaneTransform;
	WorldToPlaneTransform.setToWorldToPlane(OdGePlane(arLn1Beg, vPlnNorm));

	arLn1End.transformBy(WorldToPlaneTransform);
	arLn2Beg.transformBy(WorldToPlaneTransform);
	arLn2End.transformBy(WorldToPlaneTransform);
	double dA1 {-arLn1End.y / dV1Mag};
	double dB1 {arLn1End.x / dV1Mag};
	v2.x = arLn2End.x - arLn2Beg.x;
	v2.y = arLn2End.y - arLn2Beg.y;
	double dA2 {-v2.y / dV2Mag};
	double dB2 {v2.x / dV2Mag};
	double dDet {dA2 * dB1 - dA1 * dB2};

	double dSgnRad {(arLn1End.x * arLn2End.y - arLn2End.x * arLn1End.y) >= 0. ? -fabs(radius) : fabs(radius)};

	double dC1RAB1 {dSgnRad};
	double dC2RAB2 {(arLn2Beg.x * arLn2End.y - arLn2End.x * arLn2Beg.y) / dV2Mag + dSgnRad};
	(*center).x = (dB2 * dC1RAB1 - dB1 * dC2RAB2) / dDet;
	(*center).y = (dA1 * dC2RAB2 - dA2 * dC1RAB1) / dDet;
	(*center).z = 0.;
	WorldToPlaneTransform.invert();
	*center = WorldToPlaneTransform * (*center);
	return true;
}

int pFndSwpAngGivPlnAnd3Lns(const OdGeVector3d & planeNormal, const OdGePoint3d & arP1, const OdGePoint3d & arP2, const OdGePoint3d & arP3, const OdGePoint3d & center, double& sweepAngle) {
	double dT[3];
	OdGePoint3d rR[3];

	if (arP1 == center || arP2 == center || arP3 == center)
		return (FALSE);

	// None of the points coincide with center point
	EoGeMatrix3d WorldToPlaneTransform;
	WorldToPlaneTransform.setToWorldToPlane(OdGePlane(center, planeNormal));

	rR[0] = arP1;
	rR[1] = arP2;
	rR[2] = arP3;
	for (int i = 0; i < 3; i++) { // Translate points into z=0 plane with center point at origin
		rR[i].transformBy(WorldToPlaneTransform);
		dT[i] = atan2(rR[i].y, rR[i].x);
		if (dT[i] < 0.)
			dT[i] += TWOPI;
	}
	const double dTMin = EoMin(dT[0], dT[2]);
	const double dTMax = EoMax(dT[0], dT[2]);
	if (fabs(dT[1] - dTMax) > DBL_EPSILON && fabs(dT[1] - dTMin) > DBL_EPSILON) { // Inside line is not colinear with outside lines
		double dTheta = dTMax - dTMin;
		if (dT[1] > dTMin && dT[1] < dTMax) {
			if (dT[0] == dTMax)
				dTheta = -dTheta;
		} else {
			dTheta = TWOPI - dTheta;
			if (dT[2] == dTMax)
				dTheta = -dTheta;
		}
		sweepAngle = dTheta;

		return (TRUE);
	}
	return (FALSE);
}