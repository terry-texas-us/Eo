#include "stdafx.h"

#include "AeSys.h"
#include "AeSysDoc.h"

#include "PrimState.h"

IMPLEMENT_DYNAMIC(EoDbPrimitive, CObject)

short	EoDbPrimitive::sm_LayerColorIndex = 1;
short	EoDbPrimitive::sm_LayerLinetypeIndex = 1;
short	EoDbPrimitive::sm_HighlightLinetypeIndex = 0;
short	EoDbPrimitive::sm_HighlightColorIndex = 0;

unsigned EoDbPrimitive::sm_ControlPointIndex = SIZE_T_MAX;
double EoDbPrimitive::sm_RelationshipOfPoint = 0.0;
double EoDbPrimitive::sm_SelectApertureSize = .02;

EoDbPrimitive::EoDbPrimitive(short colorIndex, short linetypeIndex)
	: m_ColorIndex(colorIndex)
	, m_LinetypeIndex(linetypeIndex) {
}

void EoDbPrimitive::CutAt(const OdGePoint3d& point, EoDbGroup* newGroup) {
}

void EoDbPrimitive::CutAt2Points(OdGePoint3d* points, EoDbGroupList* group, EoDbGroupList* newGroup, OdDbDatabasePtr database) {
}

int EoDbPrimitive::IsWithinArea(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, OdGePoint3d* intersections) {
	return 0;
}

bool EoDbPrimitive::PivotOnGripPoint(AeSysView* view, const EoGePoint4d& point) noexcept {
	return false;
}

CString EoDbPrimitive::FormatColorIndex() const {
	CString str;
	if (m_ColorIndex == COLORINDEX_BYLAYER) {
		str = L"ByLayer";
	} else if (m_ColorIndex == COLORINDEX_BYBLOCK) {
		str = L"ByBlock";
	} else {
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
	} else if (m_LinetypeIndex == LINETYPE_BYBLOCK) {
		str = L"ByBlock";
	} else {
		wchar_t szBuf[16];
		_itow_s(m_LinetypeIndex, szBuf, 16, 10);
		str = szBuf;
	}
	return str;
}

short EoDbPrimitive::LogicalColorIndex() const noexcept {
	short ColorIndex = sm_HighlightColorIndex == 0 ? m_ColorIndex : sm_HighlightColorIndex;
	if (ColorIndex == COLORINDEX_BYLAYER) {
		ColorIndex = sm_LayerColorIndex;
	} else if (ColorIndex == COLORINDEX_BYBLOCK) {
		ColorIndex = 7;
	}
	return ColorIndex;
}

short EoDbPrimitive::LogicalLinetypeIndex() const noexcept {
	short LinetypeIndex = sm_HighlightLinetypeIndex == 0 ? m_LinetypeIndex : sm_HighlightLinetypeIndex;
	if (LinetypeIndex == LINETYPE_BYLAYER) {
		LinetypeIndex = sm_LayerLinetypeIndex;
	} else if (LinetypeIndex == LINETYPE_BYBLOCK) {
		LinetypeIndex = 1;
	}
	return LinetypeIndex;
}

void EoDbPrimitive::ModifyState() noexcept {
	m_ColorIndex = pstate.ColorIndex();
	m_LinetypeIndex = pstate.LinetypeIndex();
}

unsigned EoDbPrimitive::ControlPointIndex() noexcept {
	return sm_ControlPointIndex;
}

bool EoDbPrimitive::IsSupportedLinetype(int linetype) noexcept {
	return linetype <= 7 && linetype != 4 && linetype != 5;
}

short EoDbPrimitive::LayerColorIndex() noexcept {
	return sm_LayerColorIndex;
}

void EoDbPrimitive::SetLayerColorIndex(short colorIndex) noexcept {
	sm_LayerColorIndex = colorIndex;
}

short EoDbPrimitive::LayerLinetypeIndex() noexcept {
	return sm_LayerLinetypeIndex;
}

void EoDbPrimitive::SetLayerLinetypeIndex(short linetypeIndex) noexcept {
	sm_LayerLinetypeIndex = linetypeIndex;
}

double EoDbPrimitive::RelationshipOfPoint() noexcept {
	return sm_RelationshipOfPoint;
}

void EoDbPrimitive::SetColorIndex2(short colorIndex) {
	m_ColorIndex = colorIndex;
	if (!m_EntityObjectId.isNull()) {
		OdDbEntityPtr Entity = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		Entity->setColorIndex(static_cast<unsigned short>(colorIndex));
	}
}

void EoDbPrimitive::SetLinetypeIndex2(short linetypeIndex) {
	m_LinetypeIndex = linetypeIndex;

	if (!m_EntityObjectId.isNull()) {
		const OdDbObjectId Linetype = LinetypeObjectFromIndex(LinetypeIndex());

		OdDbEntityPtr Entity = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		Entity->setLinetype(Linetype);
	}
}

short EoDbPrimitive::HighlightColorIndex() noexcept {
	return sm_HighlightColorIndex;
}

short EoDbPrimitive::HighlightLinetypeIndex() noexcept {
	return sm_HighlightLinetypeIndex;
}

void EoDbPrimitive::SetHighlightColorIndex(short colorIndex) noexcept {
	sm_HighlightColorIndex = colorIndex;
}

void EoDbPrimitive::SetHighlightLinetypeIndex(short linetypeIndex) noexcept {
	sm_HighlightLinetypeIndex = linetypeIndex;
}

OdGeVector3d ComputeArbitraryAxis(const OdGeVector3d & normal) {
	const double Epsilon = 1. / 64.;

	OdGeVector3d ArbitraryAxis;
	if (fabs(normal.x) < Epsilon && fabs(normal.y) < Epsilon) {
		ArbitraryAxis = OdGeVector3d::kYAxis.crossProduct(normal);
	} else {
		ArbitraryAxis = OdGeVector3d::kZAxis.crossProduct(normal);
	}
	return ArbitraryAxis;
}

double ComputeElevation(const OdGePoint3d & point, const OdGeVector3d & normal) {
	OdGePlane Plane(point, normal);

	OdGeMatrix3d WorldToPlaneTransform;
	WorldToPlaneTransform.setToWorldToPlane(Plane);

	auto OriginOnPlane {OdGePoint3d::kOrigin.orthoProject(Plane)};
	auto OriginToPlaneVector {OriginOnPlane.asVector()};
	OriginToPlaneVector.transformBy(WorldToPlaneTransform);

	return OriginToPlaneVector.z;
}
// <summary>Computes the plane normal. Expects uAxis = pointU - origin and vAxis = pointV - origin to be non-collinear.</summary>
OdGeVector3d ComputeNormal(const OdGePoint3d & pointU, const OdGePoint3d & origin, const OdGePoint3d & pointV) {
	auto Normal = OdGeVector3d(pointU - origin).crossProduct(OdGeVector3d(pointV - origin));
	if (Normal.isZeroLength()) {
		return OdGeVector3d::kZAxis;
	}
	return Normal.normalize();
}

OdDbObjectId EoDbPrimitive::LinetypeObjectFromIndex(short linetypeIndex) {
	const auto Document {AeSysDoc::GetDoc()};
	
	if (Document != nullptr) {
		return LinetypeObjectFromIndex0(Document->m_DatabasePtr, linetypeIndex);
	}
	TRACE0("Document not associated with ChildFrame yet\n");
	return nullptr;
}

OdDbObjectId EoDbPrimitive::LinetypeObjectFromIndex0(OdDbDatabasePtr database, short linetypeIndex) {
	OdDbObjectId Linetype {nullptr};

	OdDbLinetypeTablePtr Linetypes {database->getLinetypeTableId().safeOpenObject(OdDb::kForRead)};

	if (linetypeIndex == LINETYPE_BYLAYER) {
		Linetype = Linetypes->getLinetypeByLayerId();
	} else if (linetypeIndex == LINETYPE_BYBLOCK) {
		Linetype = Linetypes->getLinetypeByBlockId();
	} else {
		OdString Name {EoDbLinetypeTable::LegacyLinetypeName(linetypeIndex)};
		Linetype = Linetypes->getAt(Name); // <tas="Assumes the linetype created already"/>
	}
	return Linetype;
}