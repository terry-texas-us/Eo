#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"

OdInt16	EoDbPrimitive::sm_LayerColorIndex = 1;
OdInt16	EoDbPrimitive::sm_LayerLinetypeIndex = 1;
OdInt16	EoDbPrimitive::sm_HighlightLinetypeIndex = 0;
OdInt16	EoDbPrimitive::sm_HighlightColorIndex = 0;

size_t EoDbPrimitive::sm_ControlPointIndex = SIZE_T_MAX;
double EoDbPrimitive::sm_RelationshipOfPoint = 0.;
double EoDbPrimitive::sm_SelectApertureSize = .02;

EoDbPrimitive::EoDbPrimitive() noexcept
	: m_LayerId(NULL)
    , m_ColorIndex(1)
    , m_LinetypeIndex(1) {
}

EoDbPrimitive::EoDbPrimitive(OdInt16 colorIndex, OdInt16 linetypeIndex)
	: m_LayerId(NULL)
    , m_ColorIndex(colorIndex)
    , m_LinetypeIndex(linetypeIndex) {
}

EoDbPrimitive::~EoDbPrimitive() {
}

void EoDbPrimitive::CutAt(const OdGePoint3d& point, EoDbGroup*, OdDbDatabasePtr& database) {
}

void EoDbPrimitive::CutAt2Points(OdGePoint3d* points, EoDbGroupList* group, EoDbGroupList* newGroup, OdDbDatabasePtr& database) {
}

int EoDbPrimitive::IsWithinArea(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, OdGePoint3d* intersections) noexcept {
	return 0;
}

bool EoDbPrimitive::PivotOnGripPoint(AeSysView* view, const EoGePoint4d& point) noexcept {
	return false;
}

bool EoDbPrimitive::SelectBy(const EoGeLineSeg3d& line, AeSysView* view, OdGePoint3dArray& intersections) {
	const CRuntimeClass* PrimitiveClass = GetRuntimeClass();
	theApp.AddStringToMessageList(L"Selection by line segment not implemented for <%s>\n", CString(PrimitiveClass->m_lpszClassName));
	return false;
}

CString EoDbPrimitive::FormatColorIndex() const {
	CString str;
	if (m_ColorIndex == COLORINDEX_BYLAYER) {
		str = L"ByLayer";
	}
	else if (m_ColorIndex == COLORINDEX_BYBLOCK) {
		str = L"ByBlock";
	}
	else {
		wchar_t szBuf[16];
		_itow_s(m_ColorIndex, szBuf, 16, 10);
		str = szBuf;
	}
	return str;
}

CString EoDbPrimitive::FormatLinetypeIndex() const {
	CString str;
	if (m_LinetypeIndex == LINETYPE_BYLAYER) {
		str = L"ByLayer";
	}
	else if (m_LinetypeIndex == LINETYPE_BYBLOCK) {
		str = L"ByBlock";
	}
	else {
		wchar_t szBuf[16];
		_itow_s(m_LinetypeIndex, szBuf, 16, 10);
		str = szBuf;
	}
	return str;
}

OdInt16 EoDbPrimitive::LogicalColorIndex() const noexcept {
	OdInt16 ColorIndex = sm_HighlightColorIndex == 0 ? m_ColorIndex : sm_HighlightColorIndex;
	if (ColorIndex == COLORINDEX_BYLAYER) {
		ColorIndex = sm_LayerColorIndex;
	}
	else if (ColorIndex == COLORINDEX_BYBLOCK) {
		ColorIndex = 7;
	}
	return (ColorIndex);
}

OdInt16 EoDbPrimitive::LogicalLinetypeIndex() const noexcept {
	OdInt16 LinetypeIndex = sm_HighlightLinetypeIndex == 0 ? m_LinetypeIndex : sm_HighlightLinetypeIndex;
	if (LinetypeIndex == LINETYPE_BYLAYER) {
		LinetypeIndex = sm_LayerLinetypeIndex;
	}
	else if (LinetypeIndex == LINETYPE_BYBLOCK) {
		LinetypeIndex = 1;
	}
	return (LinetypeIndex);
}

void EoDbPrimitive::ModifyState() noexcept {
	m_ColorIndex = pstate.ColorIndex();
	m_LinetypeIndex = pstate.LinetypeIndex();
}

size_t EoDbPrimitive::ControlPointIndex() noexcept {
	return sm_ControlPointIndex;
}

bool EoDbPrimitive::IsSupportedLinetype(int linetype) noexcept {
	return (linetype <= 7 && linetype != 4 && linetype != 5);
}

OdInt16 EoDbPrimitive::LayerColorIndex() noexcept {
	return sm_LayerColorIndex;
}

void EoDbPrimitive::SetLayerColorIndex(OdInt16 colorIndex) noexcept {
	sm_LayerColorIndex = colorIndex;
}

OdInt16 EoDbPrimitive::LayerLinetypeIndex() noexcept {
	return sm_LayerLinetypeIndex;
}

void EoDbPrimitive::SetLayerLinetypeIndex(OdInt16 linetypeIndex) noexcept {
	sm_LayerLinetypeIndex = linetypeIndex;
}

double EoDbPrimitive::RelationshipOfPoint() noexcept {
	return sm_RelationshipOfPoint;
}

void EoDbPrimitive::SetColorIndex(OdInt16 colorIndex) {
	m_ColorIndex = colorIndex;
	if (!m_EntityObjectId.isNull()) {
		OdDbEntityPtr Entity = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		Entity->setColorIndex(colorIndex);
	}
}

void EoDbPrimitive::SetLinetypeIndex(OdInt16 linetypeIndex) {
	m_LinetypeIndex = linetypeIndex;

	if (!m_EntityObjectId.isNull()) {
		const OdDbObjectId Linetype = LinetypeObjectFromIndex(LinetypeIndex());

		OdDbEntityPtr Entity = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		Entity->setLinetype(Linetype);
	}
}

OdInt16 EoDbPrimitive::HighlightColorIndex() noexcept {
	return sm_HighlightColorIndex;
}

OdInt16 EoDbPrimitive::HighlightLinetypeIndex() noexcept {
	return sm_HighlightLinetypeIndex;
}

void EoDbPrimitive::SetHighlightColorIndex(OdInt16 colorIndex) noexcept {
	sm_HighlightColorIndex = colorIndex;
}

void EoDbPrimitive::SetHighlightLinetypeIndex(OdInt16 linetypeIndex) noexcept {
	sm_HighlightLinetypeIndex = linetypeIndex;
}

OdGeVector3d ComputeArbitraryAxis(const OdGeVector3d& normal) {
	const double Epsilon = 1. / 64.;

	OdGeVector3d ArbitraryAxis;
	if ((fabs(normal.x) < Epsilon) && (fabs(normal.y) < Epsilon)) {
		ArbitraryAxis = OdGeVector3d::kYAxis.crossProduct(normal);
	}
	else {
		ArbitraryAxis = OdGeVector3d::kZAxis.crossProduct(normal);
	}
	return ArbitraryAxis;
}

double ComputeElevation(const OdGePoint3d& point, const OdGeVector3d& normal) {
    OdGePlane Plane(point, normal);

    OdGeMatrix3d WorldToPlaneTransform;
    WorldToPlaneTransform.setToWorldToPlane(Plane);

    auto OriginOnPlane {OdGePoint3d::kOrigin.orthoProject(Plane)};
    auto OriginToPlaneVector {OriginOnPlane.asVector()};
    OriginToPlaneVector.transformBy(WorldToPlaneTransform);

    return OriginToPlaneVector.z;
}

OdDbObjectId EoDbPrimitive::LinetypeObjectFromIndex(OdInt16 linetypeIndex) {
	OdDbDatabasePtr Database = AeSysDoc::GetDoc()->m_DatabasePtr;
	OdDbLinetypeTablePtr Linetypes = Database->getLinetypeTableId().safeOpenObject(OdDb::kForRead);

	OdDbObjectId Linetype = 0;

	if (linetypeIndex == EoDbPrimitive::LINETYPE_BYLAYER) {
		Linetype = Linetypes->getLinetypeByLayerId();
	}
	else if (linetypeIndex == EoDbPrimitive::LINETYPE_BYBLOCK) {
		Linetype = Linetypes->getLinetypeByBlockId();
	}
	else {
		OdString Name = EoDbLinetypeTable::LegacyLinetypeName(linetypeIndex);
		Linetype = Linetypes->getAt(Name); // <tas="Assumes the linetype created already"</tas>
	}
	return Linetype;
}