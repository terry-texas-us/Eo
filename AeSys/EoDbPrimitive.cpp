#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "PrimState.h"
#include <DbLinetypeTable.h>
IMPLEMENT_DYNAMIC(EoDbPrimitive, CObject)

short EoDbPrimitive::ms_LayerColorIndex = 1;
short EoDbPrimitive::ms_LayerLinetypeIndex = 1;
short EoDbPrimitive::ms_HighlightLinetypeIndex = 0;
short EoDbPrimitive::ms_HighlightColorIndex = 0;
unsigned EoDbPrimitive::ms_ControlPointIndex = SIZE_T_MAX;
double EoDbPrimitive::ms_RelationshipOfPoint = 0.0;
double EoDbPrimitive::ms_SelectApertureSize = 0.02;

void EoDbPrimitive::CutAt(const OdGePoint3d& /*point*/, EoDbGroup* /*newGroup*/) {}

void EoDbPrimitive::CutAt2Points(OdGePoint3d* /*points*/, EoDbGroupList* /*groups*/, EoDbGroupList* /*newGroups*/, OdDbDatabasePtr /*database*/) {}

int EoDbPrimitive::IsWithinArea(const OdGePoint3d& /*lowerLeftCorner*/, const OdGePoint3d& /*upperRightCorner*/, OdGePoint3d* /*intersections*/) {
	return 0;
}

bool EoDbPrimitive::PivotOnGripPoint(AeSysView* /*view*/, const EoGePoint4d& /*point*/) noexcept {
	return false;
}

CString EoDbPrimitive::FormatColorIndex() const {
	if (m_ColorIndex == mc_ColorindexBylayer) {
		return L"ByLayer";
	}
	if (m_ColorIndex == mc_ColorindexByblock) {
		return L"ByBlock";
	}
	wchar_t Buffer[16];
	_itow_s(m_ColorIndex, Buffer, 16, 10);
	return Buffer;
}

CString EoDbPrimitive::FormatLinetypeIndex() const {
	if (m_LinetypeIndex == mc_LinetypeBylayer) {
		return L"ByLayer";
	}
	if (m_LinetypeIndex == mc_LinetypeByblock) {
		return L"ByBlock";
	}
	wchar_t Buffer[16];
	_itow_s(m_LinetypeIndex, Buffer, 16, 10);
	return Buffer;
}

short EoDbPrimitive::LogicalColorIndex() const noexcept {
	auto ColorIndex {ms_HighlightColorIndex == 0 ? m_ColorIndex : ms_HighlightColorIndex};
	if (ColorIndex == mc_ColorindexBylayer) {
		ColorIndex = ms_LayerColorIndex;
	} else if (ColorIndex == mc_ColorindexByblock) {
		ColorIndex = 7;
	}
	return ColorIndex;
}

short EoDbPrimitive::LogicalLinetypeIndex() const noexcept {
	auto LinetypeIndex {ms_HighlightLinetypeIndex == 0 ? m_LinetypeIndex : ms_HighlightLinetypeIndex};
	if (LinetypeIndex == mc_LinetypeBylayer) {
		LinetypeIndex = ms_LayerLinetypeIndex;
	} else if (LinetypeIndex == mc_LinetypeByblock) {
		LinetypeIndex = 1;
	}
	return LinetypeIndex;
}

void EoDbPrimitive::ModifyState() noexcept {
	m_ColorIndex = g_PrimitiveState.ColorIndex();
	m_LinetypeIndex = g_PrimitiveState.LinetypeIndex();
}

unsigned EoDbPrimitive::ControlPointIndex() noexcept {
	return ms_ControlPointIndex;
}

bool EoDbPrimitive::IsSupportedLinetype(const int linetype) noexcept {
	return linetype <= 7 && linetype != 4 && linetype != 5;
}

short EoDbPrimitive::LayerColorIndex() noexcept {
	return ms_LayerColorIndex;
}

void EoDbPrimitive::SetLayerColorIndex(const short colorIndex) noexcept {
	ms_LayerColorIndex = colorIndex;
}

short EoDbPrimitive::LayerLinetypeIndex() noexcept {
	return ms_LayerLinetypeIndex;
}

void EoDbPrimitive::SetLayerLinetypeIndex(const short linetypeIndex) noexcept {
	ms_LayerLinetypeIndex = linetypeIndex;
}

double EoDbPrimitive::RelationshipOfPoint() noexcept {
	return ms_RelationshipOfPoint;
}

void EoDbPrimitive::SetColorIndex2(const short colorIndex) {
	m_ColorIndex = colorIndex;
	if (!m_EntityObjectId.isNull()) {
		OdDbEntityPtr Entity = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		Entity->setColorIndex(static_cast<unsigned short>(colorIndex));
	}
}

void EoDbPrimitive::SetLinetypeIndex2(const short linetypeIndex) {
	m_LinetypeIndex = linetypeIndex;
	if (!m_EntityObjectId.isNull()) {
		const auto Linetype {LinetypeObjectFromIndex(LinetypeIndex())};
		OdDbEntityPtr Entity = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		Entity->setLinetype(Linetype);
	}
}

short EoDbPrimitive::HighlightColorIndex() noexcept {
	return ms_HighlightColorIndex;
}

short EoDbPrimitive::HighlightLinetypeIndex() noexcept {
	return ms_HighlightLinetypeIndex;
}

void EoDbPrimitive::SetHighlightColorIndex(const short colorIndex) noexcept {
	ms_HighlightColorIndex = colorIndex;
}

void EoDbPrimitive::SetHighlightLinetypeIndex(const short linetypeIndex) noexcept {
	ms_HighlightLinetypeIndex = linetypeIndex;
}

OdGeVector3d ComputeArbitraryAxis(const OdGeVector3d& normal) {
	const auto Epsilon {1.0 / 64.0};
	OdGeVector3d ArbitraryAxis;
	if (fabs(normal.x) < Epsilon && fabs(normal.y) < Epsilon) {
		ArbitraryAxis = OdGeVector3d::kYAxis.crossProduct(normal);
	} else {
		ArbitraryAxis = OdGeVector3d::kZAxis.crossProduct(normal);
	}
	return ArbitraryAxis;
}

double ComputeElevation(const OdGePoint3d& point, const OdGeVector3d& normal) {
	const OdGePlane Plane(point, normal);
	OdGeMatrix3d WorldToPlaneTransform;
	WorldToPlaneTransform.setToWorldToPlane(Plane);
	auto OriginOnPlane {OdGePoint3d::kOrigin.orthoProject(Plane)};
	auto OriginToPlaneVector {OriginOnPlane.asVector()};
	OriginToPlaneVector.transformBy(WorldToPlaneTransform);
	return OriginToPlaneVector.z;
}
// <summary>Computes the plane normal. Expects uAxis = pointU - origin and vAxis = pointV - origin to be non-collinear.</summary>
OdGeVector3d ComputeNormal(const OdGePoint3d& pointU, const OdGePoint3d& origin, const OdGePoint3d& pointV) {
	auto Normal = OdGeVector3d(pointU - origin).crossProduct(OdGeVector3d(pointV - origin));
	if (Normal.isZeroLength()) {
		return OdGeVector3d::kZAxis;
	}
	return Normal.normalize();
}

OdDbObjectId EoDbPrimitive::LinetypeObjectFromIndex(const short linetypeIndex) {
	const auto Document {AeSysDoc::GetDoc()};
	if (Document != nullptr) {
		return LinetypeObjectFromIndex0(Document->m_DatabasePtr, linetypeIndex);
	}
	TRACE0("Document not associated with ChildFrame yet\n");
	return nullptr;
}

OdDbObjectId EoDbPrimitive::LinetypeObjectFromIndex0(OdDbDatabasePtr database, const short linetypeIndex) {
	OdDbObjectId Linetype;
	OdDbLinetypeTablePtr Linetypes {database->getLinetypeTableId().safeOpenObject(OdDb::kForRead)};
	if (linetypeIndex == mc_LinetypeBylayer) {
		Linetype = Linetypes->getLinetypeByLayerId();
	} else if (linetypeIndex == mc_LinetypeByblock) {
		Linetype = Linetypes->getLinetypeByBlockId();
	} else {
		const auto Name {EoDbLinetypeTable::LegacyLinetypeName(linetypeIndex)};
		Linetype = Linetypes->getAt(Name); // <tas="Assumes the linetype created already"/>
	}
	return Linetype;
}
