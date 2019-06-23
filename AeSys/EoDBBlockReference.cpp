#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "PrimState.h"
#include <DbBlockTable.h>
#include <DbBlockTableRecord.h>
#include <DbAttribute.h>
#include "EoDb.h"
#include "EoDbFile.h"
#include "EoDbBlockReference.h"
IMPLEMENT_DYNAMIC(EoDbBlockReference, EoDbPrimitive)

class EoDbPegFile;

EoDbBlockReference::EoDbBlockReference(const EoDbBlockReference& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;
	m_Name = other.m_Name;
	m_Position = other.m_Position;
	m_Normal = other.m_Normal;
	m_ScaleFactors = other.m_ScaleFactors;
	m_Rotation = other.m_Rotation;
	m_Columns = other.m_Columns;
	m_Rows = other.m_Rows;
	m_ColumnSpacing = other.m_ColumnSpacing;
	m_RowSpacing = other.m_RowSpacing;
}

const EoDbBlockReference& EoDbBlockReference::operator=(const EoDbBlockReference& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;
	m_Name = other.m_Name;
	m_Position = other.m_Position;
	m_Normal = other.m_Normal;
	m_ScaleFactors = other.m_ScaleFactors;
	m_Rotation = other.m_Rotation;
	m_Columns = other.m_Columns;
	m_Rows = other.m_Rows;
	m_ColumnSpacing = other.m_ColumnSpacing;
	m_RowSpacing = other.m_RowSpacing;
	return *this;
}

void EoDbBlockReference::AddReportToMessageList(const OdGePoint3d& point) const {
	CString Report(L"<BlockReference>");
	Report += L" Color:" + FormatColorIndex();
	Report += L" Linetype:" + FormatLinetypeIndex();
	Report += L" BlockName:" + m_Name;
	theApp.AddStringToMessageList(Report);
}

void EoDbBlockReference::AddToTreeViewControl(const HWND tree, const HTREEITEM parent) const {
	EoDbBlock* Block;
	if (AeSysDoc::GetDoc()->LookupBlock(m_Name, Block) == 0) { return; }
	const auto TreeItemHandle {CMainFrame::InsertTreeViewControlItem(tree, parent, L"<BlockReference>", this)};
	static_cast<EoDbGroup*>(Block)->AddPrimitivesToTreeViewControl(tree, TreeItemHandle);
}

EoGeMatrix3d EoDbBlockReference::BlockTransformMatrix(const OdGePoint3d& basePoint) const {
	// <tas="Some of the BlockReference primitives are a substitute for non BlockReference entities. (example RotatedDimension)"</tas>
	// <tas="Type checking for access the BlockReference methods required"</tas>
	if (!m_EntityObjectId.isNull()) {
		OdDbEntityPtr Entity = m_EntityObjectId.safeOpenObject();
		if (!Entity->isKindOf(OdDbBlockReference::desc())) {
		}
	}
	EoGeMatrix3d LeftMatrix;
	EoGeMatrix3d RightMatrix;
	LeftMatrix.setToTranslation(-basePoint.asVector());
	m_ScaleFactors.getMatrix(RightMatrix);
	LeftMatrix.preMultBy(RightMatrix);
	RightMatrix.setToRotation(m_Rotation, OdGeVector3d::kZAxis);
	LeftMatrix.preMultBy(RightMatrix);
	RightMatrix.
		setToPlaneToWorld(m_Normal); // <tas="setToPlaneToWorld method change vs2013 to vs2015. It likely will not matter since this call uses normal instead of major, minor axis like ellipse usage. Have not checked if this is broken."</tas>
	LeftMatrix.preMultBy(RightMatrix);
	RightMatrix.setToTranslation(m_Position.asVector());
	LeftMatrix.preMultBy(RightMatrix);
	return LeftMatrix;
}

EoDbPrimitive* EoDbBlockReference::Clone(OdDbBlockTableRecordPtr blockTableRecord) const {
	return Create(*this, blockTableRecord->database());
}

void EoDbBlockReference::Display(AeSysView* view, CDC* deviceContext) {
	EoDbBlock* Block;
	if (AeSysDoc::GetDoc()->LookupBlock(m_Name, Block) == 0) { return; }
	view->PushModelTransform(BlockTransformMatrix(Block->BasePoint()));
	Block->Display(view, deviceContext);
	view->PopModelTransform();
}

void EoDbBlockReference::GetAllPoints(OdGePoint3dArray& points) const {
	points.clear();
	points.append(m_Position);
}

void EoDbBlockReference::FormatExtra(CString& extra) const {
	extra.Empty();
	extra += L"Color;" + FormatColorIndex() + L"\t";
	extra += L"Linetype;" + FormatLinetypeIndex() + L"\t";
	extra += L"Block Name;" + m_Name + L"\t";
	CString Angle;
	Angle.Format(L"Angle;%f", m_Rotation);
	extra += Angle;
}

void EoDbBlockReference::FormatGeometry(CString& geometry) const {
	CString PositionString;
	PositionString.Format(L"Insertion Point;%f;%f;%f\t", m_Position.x, m_Position.y, m_Position.z);
	geometry += PositionString;
	CString NormalString;
	NormalString.Format(L"Normal;%f;%f;%f\t", m_Normal.x, m_Normal.y, m_Normal.z);
	geometry += NormalString;
	CString ScaleFactorsString;
	ScaleFactorsString.Format(L"Scale Factors;%f;%f;%f\t", m_ScaleFactors.sx, m_ScaleFactors.sy, m_ScaleFactors.sz);
	geometry += ScaleFactorsString;
}

OdGePoint3d EoDbBlockReference::GetCtrlPt() const noexcept {
	return m_Position;
}

void EoDbBlockReference::GetExtents(AeSysView* view, OdGeExtents3d& extents) const {
	EoDbBlock* Block;
	if (AeSysDoc::GetDoc()->LookupBlock(m_Name, Block) != 0) {
		view->PushModelTransform(BlockTransformMatrix(Block->BasePoint()));
		Block->GetExtents_(view, extents);
		view->PopModelTransform();
	}
}

OdGePoint3d EoDbBlockReference::GoToNxtCtrlPt() const noexcept {
	return m_Position;
}

bool EoDbBlockReference::IsEqualTo(EoDbPrimitive* primitive) const noexcept {
	return false;
}

bool EoDbBlockReference::IsInView(AeSysView* view) const {
	// Test whether an instance of a block is wholly or partially within the current view volume.
	EoDbBlock* Block;
	if (AeSysDoc::GetDoc()->LookupBlock(m_Name, Block) == 0) { return false; }
	view->PushModelTransform(BlockTransformMatrix(Block->BasePoint()));
	const auto bInView {Block->IsInView(view)};
	view->PopModelTransform();
	return bInView;
}

bool EoDbBlockReference::IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const noexcept {
	return false;
}

OdGePoint3d EoDbBlockReference::SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	sm_ControlPointIndex = SIZE_T_MAX;
	OdGePoint3d ptCtrl;
	EoDbBlock* Block;
	if (AeSysDoc::GetDoc()->LookupBlock(m_Name, Block) == 0) { return ptCtrl; }
	view->PushModelTransform(BlockTransformMatrix(Block->BasePoint()));
	auto Position {Block->GetHeadPosition()};
	while (Position != nullptr) {
		const auto Primitive {Block->GetNext(Position)};
		ptCtrl = Primitive->SelectAtControlPoint(view, point);
		if (sm_ControlPointIndex != SIZE_T_MAX) {
			view->ModelTransformPoint(ptCtrl);
			break;
		}
	}
	view->PopModelTransform();
	return ptCtrl;
}

bool EoDbBlockReference::SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const {
	EoDbBlock* Block;
	if (AeSysDoc::GetDoc()->LookupBlock(m_Name, Block) == 0) { return false; }
	view->PushModelTransform(BlockTransformMatrix(Block->BasePoint()));
	const auto bResult {Block->SelectUsingRectangle(lowerLeftCorner, upperRightCorner, view)};
	view->PopModelTransform();
	return bResult;
}

bool EoDbBlockReference::SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& projectedPoint) const {
	auto Result {false};
	EoDbBlock* Block;
	if (AeSysDoc::GetDoc()->LookupBlock(m_Name, Block) == 0) { return Result; }
	view->PushModelTransform(BlockTransformMatrix(Block->BasePoint()));
	auto Position {Block->GetHeadPosition()};
	while (Position != nullptr) {
		if (Block->GetNext(Position)->SelectUsingPoint(point, view, projectedPoint)) {
			Result = true;
			break;
		}
	}
	view->PopModelTransform();
	return Result;
}

void EoDbBlockReference::TransformBy(const EoGeMatrix3d& transformMatrix) {
	if (m_EntityObjectId.isNull()) {
		theApp.AddStringToMessageList(L"Expected valid entity object to exist.");
	} else {
		OdDbEntityPtr Entity = m_EntityObjectId.safeOpenObject();
		if (Entity->isKindOf(OdDbBlockReference::desc())) {
			OdDbBlockReferencePtr BlockReference = Entity;
			m_Position = BlockReference->position();
			m_Normal = BlockReference->normal();
			SetScaleFactors(BlockReference->scaleFactors());
			m_Rotation = BlockReference->rotation();
		} else {
			theApp.AddStringToMessageList(L"Block used for unsupported entity type. Rotation is incorrect.");
			m_Position.transformBy(transformMatrix);
			m_Normal.transformBy(transformMatrix);
			OdGeMatrix3d ScaleMatrix;
			m_ScaleFactors.getMatrix(ScaleMatrix);
			ScaleMatrix.preMultBy(transformMatrix);
			m_ScaleFactors.extractScale(ScaleMatrix);
			const auto XAxis {transformMatrix.getCsXAxis()};
			const auto Rotation {XAxis.convert2d().angle()};
			m_Rotation += Rotation;
		}
	}
}

void EoDbBlockReference::TranslateUsingMask(const OdGeVector3d& translate, const unsigned long mask) {

	if (mask != 0) { m_Position += translate; }
}

bool EoDbBlockReference::Write(EoDbFile& file) const {
	file.WriteUInt16(EoDb::kGroupReferencePrimitive);
	file.WriteInt16(m_ColorIndex);
	file.WriteInt16(m_LinetypeIndex);
	file.WriteString(m_Name);
	file.WriteDouble(m_Position.x);
	file.WriteDouble(m_Position.y);
	file.WriteDouble(m_Position.z);
	file.WriteDouble(m_Normal.x);
	file.WriteDouble(m_Normal.y);
	file.WriteDouble(m_Normal.z);
	file.WriteDouble(m_ScaleFactors.sx);
	file.WriteDouble(m_ScaleFactors.sy);
	file.WriteDouble(m_ScaleFactors.sz);
	file.WriteDouble(m_Rotation);
	file.WriteUInt16(m_Columns);
	file.WriteUInt16(m_Rows);
	file.WriteDouble(m_ColumnSpacing);
	file.WriteDouble(m_RowSpacing);
	return true;
}

void EoDbBlockReference::Write(CFile& file, unsigned char* buffer) const noexcept {
}

unsigned short EoDbBlockReference::Columns() const noexcept {
	return m_Columns;
}

double EoDbBlockReference::ColumnSpacing() const noexcept {
	return m_ColumnSpacing;
}

CString EoDbBlockReference::Name() const {
	return m_Name;
}

double EoDbBlockReference::Rotation() const noexcept {
	return m_Rotation;
}

OdGeScale3d EoDbBlockReference::ScaleFactors() const noexcept {
	return m_ScaleFactors;
}

OdGePoint3d EoDbBlockReference::Position() const noexcept {
	return m_Position;
}

OdGeVector3d EoDbBlockReference::Normal() const noexcept {
	return m_Normal;
}

unsigned short EoDbBlockReference::Rows() const noexcept {
	return m_Rows;
}

double EoDbBlockReference::RowSpacing() const noexcept {
	return m_RowSpacing;
}

bool EoDbBlockReference::SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view, OdGePoint3dArray& intersections) {
	const CRuntimeClass* PrimitiveClass = GetRuntimeClass();
	theApp.AddStringToMessageList(L"Selection by line segment not implemented for <%s>\n", CString(PrimitiveClass->m_lpszClassName));
	return false;
}

void EoDbBlockReference::SetName(const wchar_t* name) {
	m_Name = name;
}

void EoDbBlockReference::SetPosition2(const OdGePoint3d& position) {
	m_Position = position;
	if (!m_EntityObjectId.isNull()) {
		OdDbBlockReferencePtr BlockReference = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		BlockReference->setPosition(position);
	}
}

void EoDbBlockReference::SetScaleFactors(const OdGeScale3d& scaleFactors) noexcept {
	m_ScaleFactors = scaleFactors;
}

void EoDbBlockReference::SetRotation(const double rotation) noexcept {
	m_Rotation = rotation;
}

void EoDbBlockReference::SetRows(const unsigned short rows) noexcept {
	m_Rows = rows;
}

void EoDbBlockReference::SetRowSpacing(const double rowSpacing) noexcept {
	m_RowSpacing = rowSpacing;
}

void EoDbBlockReference::SetColumns(const unsigned short columns) noexcept {
	m_Columns = columns;
}

void EoDbBlockReference::SetColumnSpacing(const double columnSpacing) noexcept {
	m_ColumnSpacing = columnSpacing;
}

EoDbBlockReference* EoDbBlockReference::Create(OdDbDatabasePtr& database) {
	OdDbBlockTableRecordPtr BlockTableRecord {database->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	auto BlockReferenceEntity = OdDbBlockReference::createObject();
	BlockReferenceEntity->setDatabaseDefaults(database);
	BlockTableRecord->appendOdDbEntity(BlockReferenceEntity);
	auto BlockReference {new EoDbBlockReference()};
	BlockReference->SetEntityObjectId(BlockReferenceEntity->objectId());
	BlockReference->SetColorIndex2(g_PrimitiveState.ColorIndex());
	BlockReference->SetLinetypeIndex2(g_PrimitiveState.LinetypeIndex());
	return BlockReference;
}

// <tas="Broken. Not doing a deep clone of block"</tas>
EoDbBlockReference* EoDbBlockReference::Create(const EoDbBlockReference& other, OdDbDatabasePtr database) {
	OdDbBlockTableRecordPtr BlockTableRecord {database->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	OdDbBlockReferencePtr BlockReferenceEntity {other.EntityObjectId().safeOpenObject()->clone()};
	BlockTableRecord->appendOdDbEntity(BlockReferenceEntity);
	auto BlockReference {new EoDbBlockReference(other)};
	BlockReference->SetEntityObjectId(BlockReferenceEntity->objectId());
	return BlockReference;
}

OdDbBlockReferencePtr EoDbBlockReference::Create(OdDbBlockTableRecordPtr blockTableRecord) {
	auto BlockReference {OdDbBlockReference::createObject()};
	BlockReference->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(BlockReference);
	BlockReference->setColorIndex(static_cast<unsigned short>(g_PrimitiveState.ColorIndex()));
	const auto Linetype {LinetypeObjectFromIndex(g_PrimitiveState.LinetypeIndex())};
	BlockReference->setLinetype(Linetype);
	return BlockReference;
}

OdDbBlockReferencePtr EoDbBlockReference::Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file) {
	auto BlockReference {OdDbBlockReference::createObject()};
	BlockReference->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(BlockReference);
	BlockReference->setColorIndex(static_cast<unsigned short>(file.ReadInt16()));
	const auto Linetype {LinetypeObjectFromIndex(file.ReadInt16())};
	BlockReference->setLinetype(Linetype);

	// <tas="BlockReference Association not trueview. Known issue postion when z normal is (0,0,-1)"</tas>
	OdDbDatabasePtr Database {BlockReference->database()};
	OdDbBlockTablePtr BlockTable {Database->getBlockTableId().safeOpenObject(OdDb::kForRead)};
	CString Name;
	file.ReadString(Name);
	const auto Block {BlockTable->getAt(static_cast<const wchar_t*>(Name))};
	BlockReference->setBlockTableRecord(Block);
	BlockReference->setPosition(file.ReadPoint3d());
	BlockReference->setNormal(file.ReadVector3d());
	OdGeScale3d ScaleFactors;
	ScaleFactors.sx = file.ReadDouble();
	ScaleFactors.sy = file.ReadDouble();
	ScaleFactors.sz = file.ReadDouble();
	BlockReference->setScaleFactors(ScaleFactors);
	BlockReference->setRotation(file.ReadDouble());

	// <tas="These four properties are required for OdDbMInsertBlock. Unused here.">
	/* auto Columns = */
	file.ReadUInt16(); /* auto Rows = */
	file.ReadUInt16(); /* auto ColumnSpacing = */
	file.ReadDouble(); /* auto RowSpacing = */
	file.ReadDouble();
	// </tas>
	return BlockReference;
}

EoDbBlockReference* EoDbBlockReference::Create(OdDbBlockReferencePtr blockReference) {
	auto BlockReference {new EoDbBlockReference()};
	BlockReference->SetEntityObjectId(blockReference->objectId());
	BlockReference->m_ColorIndex = static_cast<short>(blockReference->colorIndex());
	BlockReference->m_LinetypeIndex = static_cast<short>(EoDbLinetypeTable::LegacyLinetypeIndex(blockReference->linetype()));
	OdDbBlockTableRecordPtr BlockTableRecordPtr {blockReference->blockTableRecord().safeOpenObject(OdDb::kForRead)};
	BlockReference->SetName(BlockTableRecordPtr->getName());
	BlockReference->SetPosition(blockReference->position());
	BlockReference->SetNormal(blockReference->normal());
	BlockReference->SetScaleFactors(blockReference->scaleFactors());
	BlockReference->SetRotation(blockReference->rotation());

	// <tas="Block reference - attributes">
	auto ObjectIterator {blockReference->attributeIterator()};
	for (auto i = 0; !ObjectIterator->done(); i++, ObjectIterator->step()) {
		OdDbAttributePtr AttributePtr {ObjectIterator->entity()};
		if (!AttributePtr.isNull()) {

			if (!AttributePtr->isConstant() && !AttributePtr->isInvisible()) {
				/* <tas="Attribute pointer is to OdDbText entity. The atribute data is retained by the BlockReference entity, so it can be retrieved when needed.">
				auto AttributeText {static_cast<OdDbText*>(AttributePtr)};
				</tas> */
			}
		}
	}
	// </tas>
	return BlockReference;
}
