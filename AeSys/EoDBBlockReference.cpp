#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

class EoDbPegFile;

EoDbBlockReference::EoDbBlockReference() 
	: m_Position(OdGePoint3d::kOrigin), m_Normal(OdGeVector3d::kZAxis), m_ScaleFactors(OdGeScale3d(1., 1., 1.)), m_Rotation(0.) {
	
	m_Columns = 1;
	m_Rows = 1;
	m_ColumnSpacing = 0.;
	m_RowSpacing = 0.;
}
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
EoDbBlockReference::~EoDbBlockReference() {
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

	return (*this);
}
void EoDbBlockReference::AddToTreeViewControl(HWND tree, HTREEITEM parent) const {
	EoDbBlock* Block;
	if (AeSysDoc::GetDoc()->LookupBlock(m_Name, Block) == 0) {return;}

	HTREEITEM hti = CMainFrame::InsertTreeViewControlItem(tree, parent, L"<BlockReference>", this);

	((EoDbGroup*) Block)->AddPrimsToTreeViewControl(tree, hti);
}
void EoDbBlockReference::AssociateWith(OdDbBlockTableRecordPtr blockTableRecord) {
	OdDbBlockReferencePtr BlockReferenceEntity = OdDbBlockReference::createObject();
	blockTableRecord->appendOdDbEntity(BlockReferenceEntity);
	BlockReferenceEntity->setDatabaseDefaults();
	
	SetEntityObjectId(BlockReferenceEntity->objectId());

	BlockReferenceEntity->setColorIndex(m_ColorIndex);
	SetLinetypeIndex(m_LinetypeIndex);

	// <tas="BlockReferenceEntity Association not trueview. Known issue postion when z normal is (0,0,-1)"</tas>
	OdDbDatabasePtr Database = BlockReferenceEntity->database();

	OdDbBlockTablePtr BlockTable = Database->getBlockTableId().safeOpenObject(OdDb::kForRead);
	OdDbObjectId Block = BlockTable->getAt((LPCWSTR) m_Name);
	
	BlockReferenceEntity->setBlockTableRecord(Block);

	BlockReferenceEntity->setPosition(m_Position);
	BlockReferenceEntity->setNormal(m_Normal);
	BlockReferenceEntity->setScaleFactors(m_ScaleFactors);
	BlockReferenceEntity->setRotation(m_Rotation);
	
	//BlockReferenceEntity->SetColumns(m_Columns);
	//BlockReferenceEntity->SetRows(m_Rows);
	//BlockReferenceEntity->SetColumnSpacing(m_ColumnSpacing);
	//BlockReferenceEntity->SetRowSpacing(m_RowSpacing);
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
	LeftMatrix.setToTranslation(- basePoint.asVector());
	m_ScaleFactors.getMatrix(RightMatrix);
	LeftMatrix.preMultBy(RightMatrix);
	RightMatrix.setToRotation(m_Rotation, OdGeVector3d::kZAxis);
	LeftMatrix.preMultBy(RightMatrix);
	RightMatrix.setToPlaneToWorld(m_Normal); // <tas="setToPlaneToWorld method change vs2013 to vs2015. It likely will not matter since this call uses normal instead of major, minor axis like ellipse usage. Have not checked if this is broken."</tas>
	LeftMatrix.preMultBy(RightMatrix);
	RightMatrix.setToTranslation(m_Position.asVector());
	LeftMatrix.preMultBy(RightMatrix);

	return LeftMatrix;
}
EoDbPrimitive* EoDbBlockReference::Clone(OdDbDatabasePtr database) const {
	return (EoDbBlockReference::Create(*this, database));
}
void EoDbBlockReference::Display(AeSysView* view, CDC* deviceContext) {
	EoDbBlock* Block;
	if (AeSysDoc::GetDoc()->LookupBlock(m_Name, Block) == 0)
		return;

	view->PushModelTransform(BlockTransformMatrix(Block->BasePoint()));
	Block->Display(view, deviceContext);
	view->PopModelTransform();
}
void EoDbBlockReference::AddReportToMessageList(const OdGePoint3d& point) const {
	CString Report(L"<BlockReference>");
	Report += L" Color:" + FormatColorIndex();
	Report += L" Linetype:" +  FormatLinetypeIndex();
	Report += L" BlockName:" + m_Name;
	theApp.AddStringToMessageList(Report);
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
	geometry +=  ScaleFactorsString;
}
OdGePoint3d EoDbBlockReference::GetCtrlPt() const {
	return (m_Position);
}
void EoDbBlockReference::GetExtents(AeSysView* view, OdGeExtents3d& extents) const {

	EoDbBlock* Block;

	if (AeSysDoc::GetDoc()->LookupBlock(m_Name, Block) != 0) {
		view->PushModelTransform(BlockTransformMatrix(Block->BasePoint()));
		Block->GetExtents_(view, extents);
		view->PopModelTransform();
	}
}
OdGePoint3d	EoDbBlockReference::GoToNxtCtrlPt() const {
	return m_Position;
}
bool EoDbBlockReference::Is(EoUInt16 type) const {
	return type == EoDb::kGroupReferencePrimitive;
}
bool EoDbBlockReference::IsEqualTo(EoDbPrimitive* primitive) const {
	return false;
}
bool EoDbBlockReference::IsInView(AeSysView* view) const {
	// Test whether an instance of a block is wholly or partially within the current view volume.
	EoDbBlock* Block;

	if (AeSysDoc::GetDoc()->LookupBlock(m_Name, Block) == 0) {return false;}

	view->PushModelTransform(BlockTransformMatrix(Block->BasePoint()));
	bool bInView = Block->IsInView(view);
	view->PopModelTransform();

	return (bInView);
}
bool EoDbBlockReference::IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	return false;
}
OdGePoint3d EoDbBlockReference::SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	sm_ControlPointIndex = SIZE_T_MAX;
	OdGePoint3d ptCtrl;

	EoDbBlock* Block;

	if (AeSysDoc::GetDoc()->LookupBlock(m_Name, Block) == 0) {
		return ptCtrl;
	}
	view->PushModelTransform(BlockTransformMatrix(Block->BasePoint()));

	POSITION Position = Block->GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = Block->GetNext(Position);
		ptCtrl = Primitive->SelectAtControlPoint(view, point);
		if (sm_ControlPointIndex != SIZE_T_MAX) {
			view->ModelTransformPoint(ptCtrl);
			break;
		}
	}
	view->PopModelTransform();
	return ptCtrl;
}
bool EoDbBlockReference::SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const {
	EoDbBlock* Block;

	if (AeSysDoc::GetDoc()->LookupBlock(m_Name, Block) == 0) {
		return false;
	}
	view->PushModelTransform(BlockTransformMatrix(Block->BasePoint()));
	bool bResult = Block->SelectBy(lowerLeftCorner, upperRightCorner, view);
	view->PopModelTransform();

	return (bResult);
}
bool EoDbBlockReference::SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& ptProj) const {
	bool bResult = false;

	EoDbBlock* Block;

	if (AeSysDoc::GetDoc()->LookupBlock(m_Name, Block) == 0) {
		return (bResult);
	}
	view->PushModelTransform(BlockTransformMatrix(Block->BasePoint()));

	POSITION Position = Block->GetHeadPosition();
	while (Position != 0) {
		if ((Block->GetNext(Position))->SelectBy(point, view, ptProj)) {
			bResult = true; 
			break;
		}
	}
	view->PopModelTransform();

	return (bResult);
}
void EoDbBlockReference::TransformBy(const EoGeMatrix3d& transformMatrix) {
	if (m_EntityObjectId.isNull()) {
		theApp.AddStringToMessageList(L"Expected valid entity object to exist.");
	}
	else {
		OdDbEntityPtr Entity = m_EntityObjectId.safeOpenObject();
		if (Entity->isKindOf(OdDbBlockReference::desc())) {
			OdDbBlockReferencePtr BlockReference = Entity;
			m_Position = BlockReference->position();
			m_Normal = BlockReference->normal();
			SetScaleFactors(BlockReference->scaleFactors());
			m_Rotation = BlockReference->rotation();
		}
		else {
			theApp.AddStringToMessageList(L"Block used for unsupported entity type. Rotation is incorrect.");
			m_Position.transformBy(transformMatrix);
			m_Normal.transformBy(transformMatrix);
			OdGeMatrix3d ScaleMatrix;
			m_ScaleFactors.getMatrix(ScaleMatrix);
			ScaleMatrix.preMultBy(transformMatrix);
			m_ScaleFactors.extractScale(ScaleMatrix);

			OdGeVector3d XAxis = transformMatrix.getCsXAxis();
			double Rotation = XAxis.convert2d().angle();
			m_Rotation += Rotation;
		}
	}
}
void EoDbBlockReference::TranslateUsingMask(const OdGeVector3d& translate, DWORD mask) {
	if (mask != 0) {
		m_Position += translate;
	}
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
void EoDbBlockReference::Write(CFile& file, EoByte* buffer) const {
}

EoUInt16 EoDbBlockReference::Columns() const {
	return m_Columns;
}
double EoDbBlockReference::ColumnSpacing() const {
	return m_ColumnSpacing;
}
CString EoDbBlockReference::Name() const {
	return m_Name;
}
double EoDbBlockReference::Rotation() const {
	return m_Rotation;
}
OdGeScale3d EoDbBlockReference::ScaleFactors() const {
	return m_ScaleFactors;
}
OdGePoint3d EoDbBlockReference::Position() const {
	return m_Position;
}
OdGeVector3d EoDbBlockReference::Normal() const {
	return m_Normal;
}
EoUInt16 EoDbBlockReference::Rows() const {
	return m_Rows;
}
double EoDbBlockReference::RowSpacing() const {
	return m_RowSpacing;
}
void EoDbBlockReference::SetName(const CString& name) {
	m_Name = name;
}
void EoDbBlockReference::SetNormal(const OdGeVector3d& normal) {
	m_Normal = normal;
	if (!m_EntityObjectId.isNull()) {
		OdDbBlockReferencePtr BlockReference = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		BlockReference->setNormal(normal);
	}
}
void EoDbBlockReference::SetPosition(const OdGePoint3d& position) {
	m_Position = position;
	if (!m_EntityObjectId.isNull()) {
		OdDbBlockReferencePtr BlockReference = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		BlockReference->setPosition(position);
	}
}
void EoDbBlockReference::SetScaleFactors(const OdGeScale3d& scaleFactors) {
	m_ScaleFactors = scaleFactors;
}
void EoDbBlockReference::SetRotation(double rotation) {
	m_Rotation = rotation;
}
void EoDbBlockReference::SetRows(EoUInt16 rows) {
	m_Rows = rows;
}
void EoDbBlockReference::SetRowSpacing(double rowSpacing) {
	m_RowSpacing = rowSpacing;
}
void EoDbBlockReference::SetColumns(EoUInt16 columns) {
	m_Columns = columns;
}
void EoDbBlockReference::SetColumnSpacing(double columnSpacing) {
	m_ColumnSpacing = columnSpacing;
}
EoDbBlockReference* EoDbBlockReference::ConstructFrom(EoDbFile& file) {
	EoDbBlockReference* BlockReference = new EoDbBlockReference();
	BlockReference->SetColorIndex(file.ReadInt16());
	BlockReference->SetLinetypeIndex(file.ReadInt16());
	CString Name;
	file.ReadString(Name);
	BlockReference->SetName(Name);
	BlockReference->SetPosition(file.ReadPoint3d());
	BlockReference->SetNormal(file.ReadVector3d());
	OdGeScale3d ScaleFactors;
	ScaleFactors.sx = file.ReadDouble();
	ScaleFactors.sy = file.ReadDouble();
	ScaleFactors.sz = file.ReadDouble();
	BlockReference->SetScaleFactors(ScaleFactors);
	BlockReference->SetRotation(file.ReadDouble());
	BlockReference->SetColumns(file.ReadUInt16());
	BlockReference->SetRows(file.ReadUInt16());
	BlockReference->SetColumnSpacing(file.ReadDouble());
	BlockReference->SetRowSpacing(file.ReadDouble());
	
	return (BlockReference);
}
EoDbBlockReference* EoDbBlockReference::Create(OdDbDatabasePtr database) {
	OdDbBlockTableRecordPtr BlockTableRecord = database->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

	OdDbBlockReferencePtr BlockReferenceEntity = OdDbBlockReference::createObject();
	BlockReferenceEntity->setDatabaseDefaults(database);
	BlockTableRecord->appendOdDbEntity(BlockReferenceEntity);
	
	EoDbBlockReference* BlockReference = new EoDbBlockReference();
	BlockReference->SetEntityObjectId(BlockReferenceEntity->objectId());
	
	BlockReference->SetColorIndex(pstate.ColorIndex());
	BlockReference->SetLinetypeIndex(pstate.LinetypeIndex());
	
	return BlockReference;
}
// <tas="Broken. Not doing a deep clone of block"</tas>
EoDbBlockReference* EoDbBlockReference::Create(const EoDbBlockReference& other, OdDbDatabasePtr database) {
	OdDbBlockTableRecordPtr BlockTableRecord = database->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
	OdDbBlockReferencePtr BlockReferenceEntity = other.EntityObjectId().safeOpenObject()->clone();
	BlockTableRecord->appendOdDbEntity(BlockReferenceEntity);

	EoDbBlockReference* BlockReference = new EoDbBlockReference(other);
	BlockReference->SetEntityObjectId(BlockReferenceEntity->objectId());

	return BlockReference;
}

