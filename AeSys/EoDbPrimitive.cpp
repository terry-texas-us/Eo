#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "EoDbPrimitive.h"

EoInt16	EoDbPrimitive::sm_LayerColorIndex = 1;
EoInt16	EoDbPrimitive::sm_LayerLinetypeIndex = 1;
EoInt16	EoDbPrimitive::sm_HighlightLinetypeIndex = 0;
EoInt16	EoDbPrimitive::sm_HighlightColorIndex = 0;

size_t EoDbPrimitive::sm_ControlPointIndex = SIZE_T_MAX;
double EoDbPrimitive::sm_RelationshipOfPoint = 0.;
double EoDbPrimitive::sm_SelectApertureSize = .02;

EoDbPrimitive::EoDbPrimitive()
	: m_LayerId(NULL), m_ColorIndex(1), m_LinetypeIndex(1) {
}
EoDbPrimitive::EoDbPrimitive(EoInt16 colorIndex, EoInt16 linetypeIndex)
	: m_LayerId(NULL), m_ColorIndex(colorIndex), m_LinetypeIndex(linetypeIndex) {
}
EoDbPrimitive::~EoDbPrimitive() {
}
EoInt16 EoDbPrimitive::ColorIndex() const {
	return m_ColorIndex;
}
void EoDbPrimitive::CutAt(const OdGePoint3d& point, EoDbGroup*, OdDbDatabasePtr database) {
}
void EoDbPrimitive::CutAt2Points(OdGePoint3d* points, EoDbGroupList* group, EoDbGroupList* newGroup, OdDbDatabasePtr database) {
}
int EoDbPrimitive::IsWithinArea(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, OdGePoint3d* intersections) {
	return 0;
}
bool EoDbPrimitive::PivotOnGripPoint(AeSysView* view, const EoGePoint4d& point) {
	return false;
}
bool EoDbPrimitive::SelectBy(const EoGeLineSeg3d& line, AeSysView* view, OdGePoint3dArray& intersections) {
	CRuntimeClass* PrimitiveClass = GetRuntimeClass();
	theApp.AddStringToMessageList(L"Selection by line segment not implemented for <%s>\n", CString(PrimitiveClass->m_lpszClassName));
	return false;
}
OdDbObjectId EoDbPrimitive::EntityObjectId() const {
	return m_EntityObjectId;
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
EoInt16 EoDbPrimitive::LogicalColorIndex() const {
	EoInt16 ColorIndex = sm_HighlightColorIndex == 0 ? m_ColorIndex : sm_HighlightColorIndex;
	if (ColorIndex == COLORINDEX_BYLAYER) {
		ColorIndex = sm_LayerColorIndex;
	}
	else if (ColorIndex == COLORINDEX_BYBLOCK) {
		ColorIndex = 7;
	}
	return (ColorIndex);
}
EoInt16 EoDbPrimitive::LogicalLinetypeIndex() const {
	EoInt16 LinetypeIndex = sm_HighlightLinetypeIndex == 0 ? m_LinetypeIndex : sm_HighlightLinetypeIndex;
	if (LinetypeIndex == LINETYPE_BYLAYER) {
		LinetypeIndex = sm_LayerLinetypeIndex;
	}
	else if (LinetypeIndex == LINETYPE_BYBLOCK) {
		LinetypeIndex = 1;
	}
	return (LinetypeIndex);
}
void EoDbPrimitive::ModifyState() {
	m_ColorIndex = pstate.ColorIndex();
	m_LinetypeIndex = pstate.LinetypeIndex();
}
EoInt16 EoDbPrimitive::LinetypeIndex() const {
	return m_LinetypeIndex;
}
size_t EoDbPrimitive::ControlPointIndex() {
	return sm_ControlPointIndex;
}
bool EoDbPrimitive::IsSupportedLinetype(int linetype) {
	return (linetype <= 7 && linetype != 4 && linetype != 5);
}
EoInt16 EoDbPrimitive::LayerColorIndex() {
	return sm_LayerColorIndex;
}
void EoDbPrimitive::SetLayerColorIndex(EoInt16 colorIndex) {
	sm_LayerColorIndex = colorIndex;
}
EoInt16 EoDbPrimitive::LayerLinetypeIndex() {
	return sm_LayerLinetypeIndex;
}
void EoDbPrimitive::SetLayerLinetypeIndex(EoInt16 linetypeIndex) {
	sm_LayerLinetypeIndex = linetypeIndex;
}
double EoDbPrimitive::RelationshipOfPoint() {
	return sm_RelationshipOfPoint;
}
void EoDbPrimitive::SetEntityObjectId(OdDbObjectId entityObjectId) {
	m_EntityObjectId = entityObjectId;
}
void EoDbPrimitive::SetColorIndex(EoInt16 colorIndex) {
	m_ColorIndex = colorIndex;
	if (!m_EntityObjectId.isNull()) {
		OdDbEntityPtr Entity = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		Entity->setColorIndex(colorIndex);
	}
}
void EoDbPrimitive::SetLinetypeIndex(EoInt16 linetypeIndex) {
	m_LinetypeIndex = linetypeIndex;

	if (!m_EntityObjectId.isNull()) {
		OdDbObjectId Linetype = LinetypeObjectFromIndex(LinetypeIndex());

		OdDbEntityPtr Entity = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		Entity->setLinetype(Linetype);
	}
}
EoInt16 EoDbPrimitive::HighlightColorIndex() {
	return sm_HighlightColorIndex;
}
EoInt16 EoDbPrimitive::HighlightLinetypeIndex() {
	return sm_HighlightLinetypeIndex;
}
void EoDbPrimitive::SetHighlightColorIndex(EoInt16 colorIndex) {
	sm_HighlightColorIndex = colorIndex;
}
void EoDbPrimitive::SetHighlightLinetypeIndex(EoInt16 linetypeIndex) {
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

OdDbObjectId EoDbPrimitive::LinetypeObjectFromIndex(EoInt16 linetypeIndex) {
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