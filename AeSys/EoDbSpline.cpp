#include "stdafx.h"
#include "DbSpline.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoVaxFloat.h"
#include "EoGePolyline.h"
#include "EoDbFile.h"
#include "EoDbSpline.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
EoDbSpline::EoDbSpline(const EoDbSpline& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;
	m_ColorIndex = other.m_ColorIndex;
	m_LinetypeIndex = other.m_LinetypeIndex;
	m_Spline = other.m_Spline;
}

const EoDbSpline& EoDbSpline::operator=(const EoDbSpline& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;
	m_ColorIndex = other.m_ColorIndex;
	m_LinetypeIndex = other.m_LinetypeIndex;
	m_Spline = other.m_Spline;
	return *this;
}

void EoDbSpline::AddReportToMessageList(const OdGePoint3d& point) const {
	CString Report {L"<BSpline>"};
	Report += L" Color:" + FormatColorIndex();
	Report += L" Linetype:" + FormatLinetypeIndex();
	theApp.AddStringToMessageList(Report);
}

void EoDbSpline::AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept {
	CMainFrame::InsertTreeViewControlItem(tree, parent, L"<BSpline>", this);
}

EoDbPrimitive* EoDbSpline::Clone(OdDbBlockTableRecordPtr blockTableRecord) const {
	OdDbSplinePtr Spline = m_EntityObjectId.safeOpenObject()->clone();
	blockTableRecord->appendOdDbEntity(Spline);
	return Create(Spline);
}

void EoDbSpline::Display(AeSysView* view, CDC* deviceContext) {
	const auto ColorIndex {LogicalColorIndex()};
	const auto LinetypeIndex {LogicalLinetypeIndex()};
	g_PrimitiveState.SetPen(view, deviceContext, ColorIndex, LinetypeIndex);
	polyline::BeginLineStrip();
	EoGeNurbCurve3d::GeneratePoints(m_Spline);
	polyline::__End(view, deviceContext, LinetypeIndex);
}

void EoDbSpline::FormatExtra(CString& extra) const {
	extra.Empty();
	extra += L"Color;" + FormatColorIndex() + L"\t";
	extra += L"Linetype;" + FormatLinetypeIndex() + L"\t";
	CString NumberOfPoints;
	NumberOfPoints.Format(L"Number of Control Points;%d", m_Spline.numControlPoints());
	extra += NumberOfPoints;
}

void EoDbSpline::FormatGeometry(CString& geometry) const {
	CString ControlPointString;
	for (auto ControlPointIndex = 0; ControlPointIndex < m_Spline.numControlPoints(); ControlPointIndex++) {
		const auto ControlPoint {m_Spline.controlPointAt(ControlPointIndex)};
		ControlPointString.Format(L"Control Point;%f;%f;%f\t", ControlPoint.x, ControlPoint.y, ControlPoint.z);
		geometry += ControlPointString;
	}
}

void EoDbSpline::GetAllPoints(OdGePoint3dArray& points) const {
	points.setLogicalLength(static_cast<unsigned>(m_Spline.numControlPoints()));
	for (unsigned ControlPointIndex = 0; ControlPointIndex < static_cast<unsigned>(m_Spline.numControlPoints()); ControlPointIndex++) {
		points[ControlPointIndex] = m_Spline.controlPointAt(static_cast<int>(ControlPointIndex));
	}
}

OdGePoint3d EoDbSpline::GetCtrlPt() const {
	OdGePoint3d Point;
	if (!m_EntityObjectId.isNull()) {
		OdDbSplinePtr Spline = m_EntityObjectId.safeOpenObject();
		double EndParameter;
		Spline->getEndParam(EndParameter);
		Spline->getPointAtParam(EndParameter / 2., Point);
	} else {
		Point = m_Spline.controlPointAt(m_Spline.numControlPoints() / 2);
	}
	return Point;
}

void EoDbSpline::GetExtents(AeSysView* view, OdGeExtents3d& extents) const {
	if (!m_EntityObjectId.isNull()) {
		OdDbSplinePtr Spline = m_EntityObjectId.safeOpenObject();
		OdGeExtents3d Extents;
		Spline->getGeomExtents(Extents);
		extents.addExt(Extents);

	} else {
		// <tas="Extents should use the points on the curve and not the control points"</tas>
		for (unsigned short w = 0; w < m_Spline.numControlPoints(); w++) {
			extents.addPoint(m_Spline.controlPointAt(w));
		}
	}
}

OdGePoint3d EoDbSpline::GoToNxtCtrlPt() const {
	OdGePoint3d pt;
	if (sm_RelationshipOfPoint <= DBL_EPSILON) pt = m_Spline.endPoint();
	else if (sm_RelationshipOfPoint >= 1. - DBL_EPSILON) pt = m_Spline.startPoint();
	else if (m_Spline.endPoint().x > m_Spline.startPoint().x) pt = m_Spline.startPoint();
	else if (m_Spline.endPoint().x < m_Spline.startPoint().x) pt = m_Spline.endPoint();
	else if (m_Spline.endPoint().y > m_Spline.startPoint().y) pt = m_Spline.startPoint();
	else pt = m_Spline.endPoint();
	return pt;
}

bool EoDbSpline::IsEqualTo(EoDbPrimitive* other) const {
	auto IsEqual {false};
	const auto OtherObjectId {other->EntityObjectId()};
	if (!m_EntityObjectId.isNull() && !OtherObjectId.isNull()) {
		OdDbSplinePtr Spline = m_EntityObjectId.safeOpenObject();
		OdDbSplinePtr OtherSpline = OtherObjectId.safeOpenObject();
		IsEqual = Spline->isEqualTo(OtherSpline);
	}
	return IsEqual;
}

bool EoDbSpline::IsInView(AeSysView* view) const {
	EoGePoint4d pt[2];
	pt[0] = EoGePoint4d(m_Spline.controlPointAt(0), 1.0);
	view->ModelViewTransformPoint(pt[0]);
	for (unsigned short w = 1; w < m_Spline.numControlPoints(); w++) {
		pt[1] = EoGePoint4d(m_Spline.controlPointAt(w), 1.0);
		view->ModelViewTransformPoint(pt[1]);
		if (EoGePoint4d::ClipLine(pt[0], pt[1])) return true;
		pt[0] = pt[1];
	}
	return false;
}

bool EoDbSpline::IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const noexcept {
	return false;
}

OdGePoint3d EoDbSpline::SelectAtControlPoint(AeSysView*, const EoGePoint4d& point) const {
	sm_ControlPointIndex = SIZE_T_MAX;
	return point.Convert3d();
}

bool EoDbSpline::SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& ptProj) const {
	polyline::BeginLineStrip();
	EoGeNurbCurve3d::GeneratePoints(m_Spline);
	return polyline::SelectUsingPoint(point, view, sm_RelationshipOfPoint, ptProj);
}

bool EoDbSpline::SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const {
	OdGePoint3dArray ControlPoints;
	for (unsigned short w = 0; w < m_Spline.numControlPoints(); w++) {
		ControlPoints.append(m_Spline.controlPointAt(w));
	}
	return polyline::SelectUsingRectangle(view, lowerLeftCorner, upperRightCorner, ControlPoints);
}

bool EoDbSpline::SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view, OdGePoint3dArray& intersections) {
	const CRuntimeClass* PrimitiveClass = GetRuntimeClass();
	theApp.AddStringToMessageList(L"Selection by line segment not implemented for <%s>\n", CString(PrimitiveClass->m_lpszClassName));
	return false;
}

void EoDbSpline::Set(int degree, const OdGeKnotVector& knots, const OdGePoint3dArray& controlPoints, const OdGeDoubleArray& weights, bool isPeriodic) {
	m_Spline.set(degree, knots, controlPoints, weights, isPeriodic);
}

void EoDbSpline::TransformBy(const EoGeMatrix3d& transformMatrix) {
	m_Spline.transformBy(transformMatrix);
}

void EoDbSpline::TranslateUsingMask(const OdGeVector3d& translate, unsigned long mask) {
	for (auto ControlPointIndex = 0; ControlPointIndex < m_Spline.numControlPoints(); ControlPointIndex++)
		if ((mask >> ControlPointIndex & 1UL) == 1) {
			m_Spline.setControlPointAt(ControlPointIndex, m_Spline.controlPointAt(ControlPointIndex) + translate);
		}
}

// <tas="Currently allowing 1st degree (only 2 control points) splines to be saved. This likely will not load in legacy apps"</tas>
bool EoDbSpline::Write(EoDbFile& file) const {
	file.WriteUInt16(EoDb::kSplinePrimitive);
	file.WriteInt16(m_ColorIndex);
	file.WriteInt16(m_LinetypeIndex);
	file.WriteUInt16(static_cast<unsigned short>(m_Spline.numControlPoints()));
	for (unsigned short ControlPointIndex = 0; ControlPointIndex < m_Spline.numControlPoints(); ControlPointIndex++) {
		file.WritePoint3d(m_Spline.controlPointAt(ControlPointIndex));
	}
	return true;
}

void EoDbSpline::Write(CFile& file, unsigned char* buffer) const {
	buffer[3] = static_cast<unsigned char>((2 + m_Spline.numControlPoints() * 3) / 8 + 1);
	*reinterpret_cast<unsigned short*>(& buffer[4]) = static_cast<unsigned short>(EoDb::kSplinePrimitive);
	buffer[6] = static_cast<unsigned char>(m_ColorIndex == COLORINDEX_BYLAYER ? sm_LayerColorIndex : m_ColorIndex);
	buffer[7] = static_cast<unsigned char>(m_LinetypeIndex == LINETYPE_BYLAYER ? sm_LayerLinetypeIndex : m_LinetypeIndex);
	*reinterpret_cast<short*>(& buffer[8]) = static_cast<short>(m_Spline.numControlPoints());
	auto i {10};
	for (unsigned short w = 0; w < m_Spline.numControlPoints(); w++) {
		reinterpret_cast<EoVaxPoint3d*>(& buffer[i])->Convert(m_Spline.controlPointAt(w));
		i += sizeof(EoVaxPoint3d);
	}
	file.Write(buffer, static_cast<unsigned>(buffer[3] * 32));
}

// Static
OdDbSplinePtr EoDbSpline::Create(OdDbBlockTableRecordPtr& blockTableRecord) {
	auto Spline {OdDbSpline::createObject()};
	Spline->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(Spline);
	Spline->setColorIndex(static_cast<unsigned short>(g_PrimitiveState.ColorIndex()));
	const auto Linetype {LinetypeObjectFromIndex(g_PrimitiveState.LinetypeIndex())};
	Spline->setLinetype(Linetype);
	return Spline;
}

OdDbSplinePtr EoDbSpline::Create(OdDbBlockTableRecordPtr& blockTableRecord, EoDbFile& file) {
	auto Database {blockTableRecord->database()};
	auto Spline {OdDbSpline::createObject()};
	Spline->setDatabaseDefaults(Database);
	blockTableRecord->appendOdDbEntity(Spline);
	Spline->setColorIndex(static_cast<unsigned short>(file.ReadInt16()));
	const auto Linetype {LinetypeObjectFromIndex0(Database, file.ReadInt16())};
	Spline->setLinetype(Linetype);
	const auto NumberOfControlPoints = file.ReadUInt16();
	const auto Degree {EoMin(3, NumberOfControlPoints - 1)};
	OdGePoint3dArray ControlPoints;
	for (auto ControlPointIndex = 0; ControlPointIndex < NumberOfControlPoints; ControlPointIndex++) {
		ControlPoints.append(file.ReadPoint3d());
	}
	OdGeKnotVector Knots;
	EoGeNurbCurve3d::SetDefaultKnotVector(Degree, ControlPoints, Knots);
	OdGeDoubleArray Weights;
	Weights.setLogicalLength(NumberOfControlPoints);
	Spline->setNurbsData(Degree, false, false, false, ControlPoints, Knots, Weights, OdGeContext::gTol.equalPoint());
	return Spline;
}

OdDbSplinePtr EoDbSpline::Create(OdDbBlockTableRecordPtr blockTableRecord, unsigned char* primitiveBuffer, int versionNumber) {
	short ColorIndex;
	short LinetypeIndex;
	unsigned short NumberOfControlPoints {0};
	OdGePoint3dArray ControlPoints;
	if (versionNumber == 1) {
		ColorIndex = short(primitiveBuffer[4] & 0x000f);
		LinetypeIndex = short((primitiveBuffer[4] & 0x00ff) >> 4);
		NumberOfControlPoints = static_cast<unsigned short>(reinterpret_cast<EoVaxFloat*>(&primitiveBuffer[8])->Convert());
		ControlPoints.setLogicalLength(NumberOfControlPoints);
		auto BufferIndex {12};
		for (unsigned w = 0; w < NumberOfControlPoints; w++) {
			ControlPoints[w] = reinterpret_cast<EoVaxPoint3d*>(&primitiveBuffer[BufferIndex])->Convert() * 1.e-3;
			BufferIndex += sizeof(EoVaxPoint3d);
		}
	} else {
		ColorIndex = short(primitiveBuffer[6]);
		LinetypeIndex = short(primitiveBuffer[7]);
		NumberOfControlPoints = static_cast<unsigned short>(*reinterpret_cast<short*>(&primitiveBuffer[8]));
		ControlPoints.setLogicalLength(NumberOfControlPoints);
		auto BufferIndex {10};
		for (unsigned w = 0; w < NumberOfControlPoints; w++) {
			ControlPoints[w] = reinterpret_cast<EoVaxPoint3d*>(&primitiveBuffer[BufferIndex])->Convert();
			BufferIndex += sizeof(EoVaxPoint3d);
		}
	}
	auto Database {blockTableRecord->database()};
	auto Spline {OdDbSpline::createObject()};
	Spline->setDatabaseDefaults(Database);
	blockTableRecord->appendOdDbEntity(Spline);
	Spline->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Spline->setLinetype(LinetypeObjectFromIndex0(Database, LinetypeIndex));
	const auto Degree {EoMin(3, NumberOfControlPoints - 1)};
	OdGeKnotVector Knots;
	EoGeNurbCurve3d::SetDefaultKnotVector(Degree, ControlPoints, Knots);
	OdGeDoubleArray Weights;
	Weights.setLogicalLength(NumberOfControlPoints);
	Spline->setNurbsData(Degree, false, false, false, ControlPoints, Knots, Weights, OdGeContext::gTol.equalPoint());
	return Spline;
}

EoDbSpline* EoDbSpline::Create(OdDbSplinePtr& spline) {
	auto Spline {new EoDbSpline()};
	Spline->SetEntityObjectId(spline->objectId());
	Spline->m_ColorIndex = static_cast<short>(spline->colorIndex());
	Spline->m_LinetypeIndex = static_cast<short>(EoDbLinetypeTable::LegacyLinetypeIndex(spline->linetype()));
	int Degree;
	bool Rational;
	bool Closed;
	bool Periodic;
	OdGePoint3dArray ControlPoints;
	OdGeDoubleArray Weights;
	OdGeKnotVector Knots;
	double Tolerance;
	spline->getNurbsData(Degree, Rational, Closed, Periodic, ControlPoints, Knots, Weights, Tolerance);
	if (Periodic) {
		// <tas="Only creating non-periodic splines."></tas>
	} else {
		Spline->Set(Degree, Knots, ControlPoints, Weights, Periodic);
		// ConvertCurveData(entity, Spline);
	}
	return Spline;
}
