#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

EoDbPrimitive* EoDbGroup::sm_PrimitiveToIgnore = static_cast<EoDbPrimitive*>(NULL);

EoDbGroup::EoDbGroup() 
	: m_Document(AeSysDoc::GetDoc()) {
}
EoDbGroup::EoDbGroup(const EoDbBlock& block) {
	OdDbDatabasePtr Database = m_Document->m_DatabasePtr;

	POSITION Position = block.GetHeadPosition();
	while (Position != 0) {
		AddTail((block.GetNext(Position))->Clone(Database));
	}
}
EoDbGroup::EoDbGroup(const EoDbGroup& group)
	: m_Document(AeSysDoc::GetDoc()) {
	OdDbDatabasePtr Database = m_Document->m_DatabasePtr;
	
	POSITION Position = group.GetHeadPosition();
	while (Position != 0) {
		AddTail((group.GetNext(Position))->Clone(Database));
	}
}
EoDbGroup::~EoDbGroup() {
}
void EoDbGroup::AddPrimsToTreeViewControl(HWND tree, HTREEITEM parent) {
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);
		Primitive->AddToTreeViewControl(tree, parent);
	}
}
HTREEITEM EoDbGroup::AddToTreeViewControl(HWND tree, HTREEITEM parent) {
	HTREEITEM TreeItem = CMainFrame::InsertTreeViewControlItem(tree, parent, L"<Group>", this);
	AddPrimsToTreeViewControl(tree, TreeItem);
	return TreeItem;
}
void EoDbGroup::BreakPolylines() {
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		POSITION PrimitivePosition = Position;
		EoDbPrimitive* Primitive = GetNext(Position);
		if (Primitive->Is(EoDb::kPolylinePrimitive)) {
			EoDbPolyline* PolylinePrimitive = static_cast<EoDbPolyline*>(Primitive);
			OdGePoint3dArray Points;
			PolylinePrimitive->GetAllPoints(Points);
			EoDbLine* Line;
			for (EoUInt16 w = 0; w < Points.size() - 1; w++) {
				Line = EoDbLine::Create(Points[w], Points[w + 1]);
				Line->SetColorIndex(Primitive->ColorIndex());
				Line->SetLinetypeIndex(Primitive->LinetypeIndex());
				CObList::InsertBefore(PrimitivePosition, Line);
			}
			if (PolylinePrimitive->IsClosed()) {
				Line = EoDbLine::Create(Points[Points.size() - 1], Points[0]);
				Line->SetColorIndex(Primitive->ColorIndex());
				Line->SetLinetypeIndex(Primitive->LinetypeIndex());
				CObList::InsertBefore(PrimitivePosition, Line);
			}
			this->RemoveAt(PrimitivePosition);
			delete Primitive;
		}
		else if (Primitive->Is(EoDb::kGroupReferencePrimitive)) {
			EoDbBlock* Block;
			if (AeSysDoc::GetDoc()->LookupBlock(static_cast<EoDbBlockReference*>(Primitive)->Name(), Block) != 0) {
				Block->BreakPolylines();
			}
		}
	}
}
void EoDbGroup::BreakSegRefs() {
	int iSegRefs;
	do {
		iSegRefs = 0;
		POSITION Position = GetHeadPosition();
		while (Position != 0) {
			POSITION PrimitivePosition = Position;
			EoDbPrimitive* Primitive = GetNext(Position);
			if (Primitive->Is(EoDb::kGroupReferencePrimitive)) {
				iSegRefs++;
				EoDbBlock* Block;
				if (AeSysDoc::GetDoc()->LookupBlock(static_cast<EoDbBlockReference*>(Primitive)->Name(), Block) != 0) {
					EoDbGroup* pSegT = new EoDbGroup(*Block);
					EoGeMatrix3d tm = static_cast<EoDbBlockReference*>(Primitive)->BlockTransformMatrix(Block->BasePoint());
					pSegT->TransformBy(tm);

					this->InsertBefore(PrimitivePosition, pSegT);
					this->RemoveAt(PrimitivePosition);
					delete Primitive;
					pSegT->RemoveAll();
					delete pSegT;
				}
			}
		}
	}
	while (iSegRefs != 0);
}
// <tas="Deletion of trap is irreversible. Should undo be used?"</tas>
void EoDbGroup::DeletePrimitivesAndRemoveAll() {
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);
		OdDbObjectId EntityObjectId = Primitive->EntityObjectId();
		if (!EntityObjectId.isNull()) {
			OdDbEntityPtr Entity = EntityObjectId.safeOpenObject(OdDb::kForWrite);
			Entity->erase();
		}
		delete Primitive;
	}
	RemoveAll();
}

void EoDbGroup::Display(AeSysView* view, CDC* deviceContext) {
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);
		Primitive->Display(view, deviceContext);
	}
}
void EoDbGroup::Erase() {
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);
		OdDbObjectId EntityObjectId = Primitive->EntityObjectId();
		if (!EntityObjectId.isNull()) {
			OdDbEntityPtr Entity = EntityObjectId.safeOpenObject(OdDb::kForWrite);
			Entity->erase();
		}
	}
}
void EoDbGroup::UndoErase() {
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);
		OdDbObjectId EntityObjectId = Primitive->EntityObjectId();
		if (!EntityObjectId.isNull()) {
			OdDbEntityPtr Entity = EntityObjectId.safeOpenObject(OdDb::kForWrite, true);
			Entity->erase(false);
		}
	}
}
POSITION EoDbGroup::FindAndRemovePrimitive(EoDbPrimitive* primitive) {
	POSITION Position = Find(primitive);
	if (Position != 0) {
		RemoveAt(Position);
	}
	return Position;
}
EoDbPrimitive* EoDbGroup::GetAt(POSITION position) {
	return (EoDbPrimitive*)CObList::GetAt(position);
}
int EoDbGroup::GetBlockReferenceCount(const CString& name) const {
	int Count = 0;

	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);
		if (Primitive->Is(EoDb::kGroupReferencePrimitive)) {
			if (static_cast<EoDbBlockReference*>(Primitive)->Name() == name)
				Count++;
		}
	}
	return Count;
}
void EoDbGroup::GetExtents_(AeSysView* view, OdGeExtents3d& extents) {
	OdGeExtents3d GroupExtents;
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);
		Primitive->GetExtents(view, GroupExtents);
	}
	if (GroupExtents.isValidExtents()) {
		GroupExtents.transformBy(view->ModelToWorldTransform());
		extents.addExt(GroupExtents);
	}
}
EoDbPoint* EoDbGroup::GetFirstDifferentPoint(EoDbPoint* pointPrimitive) {
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);
		if (Primitive != pointPrimitive && Primitive->Is(EoDb::kPointPrimitive)) {
			return (static_cast<EoDbPoint*>(Primitive));
		}
	}
	return 0;
}
int EoDbGroup::GetLinetypeIndexRefCount(EoInt16 linetypeIndex) {
	int Count = 0;

	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);

		if (Primitive->LinetypeIndex() == linetypeIndex) {
			Count++;
		}
	}
	return (Count);
}
EoDbPrimitive* EoDbGroup::GetNext(POSITION& position) const {
	return ((EoDbPrimitive*) CObList::GetNext(position));
}
void EoDbGroup::InsertBefore(POSITION insertPosition, EoDbGroup* group) {
	POSITION Position = group->GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = group->GetNext(Position);
		CObList::InsertBefore(insertPosition, (CObject*) Primitive);
	}
}
bool EoDbGroup::IsInView(AeSysView* view) const {
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);
		if (Primitive->IsInView(view)) {
			return true;
		}
	}
	return false;
}
bool EoDbGroup::IsOn(const EoGePoint4d& point, AeSysView* view) const {
	OdGePoint3d Point;
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);
		if (Primitive->SelectBy(point, view, Point)) {
			return true;
		}
	}
	return false;
}
bool EoDbGroup::IsPersistent() const {
	return m_Document != 0;
}
void EoDbGroup::ModifyColorIndex(EoInt16 colorIndex) {
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);
		Primitive->SetColorIndex(colorIndex);
	}
}
void EoDbGroup::ModifyLinetypeIndex(EoInt16 linetypeIndex) {
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);
		Primitive->SetLinetypeIndex(linetypeIndex);
	}
}
void EoDbGroup::ModifyNotes(EoDbFontDefinition& fontDefinition, EoDbCharacterCellDefinition& characterCellDefinition, int iAtt) {
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);
		if (Primitive->Is(EoDb::kTextPrimitive)) {
			static_cast<EoDbText*>(Primitive)->ModifyNotes(fontDefinition, characterCellDefinition, iAtt);
		}
	}
}
void EoDbGroup::PenTranslation(EoUInt16 wCols, EoInt16* pColNew, EoInt16* pCol) {
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);

		for (EoUInt16 w = 0; w < wCols; w++) {
			if (Primitive->ColorIndex() == pCol[w]) {
				Primitive->SetColorIndex(pColNew[w]);
				break;
			}
		}
	}
}
// <tas="Is no longer working!"</tas>
void EoDbGroup::RemoveDuplicatePrimitives() {
	POSITION BasePosition = GetHeadPosition();
	while (BasePosition != 0) {
		EoDbPrimitive* BasePrimitive = GetNext(BasePosition);

		POSITION TestPosition = BasePosition;
		while (TestPosition != 0) {
			POSITION TestPositionSave = TestPosition;
			EoDbPrimitive* TestPrimitive = GetNext(TestPosition);

			if (BasePrimitive->IsEqualTo(TestPrimitive)) {
				RemoveAt(TestPositionSave);
				delete TestPrimitive;
			}
		}
	}
}
int EoDbGroup::RemoveEmptyNotesAndDelete() {
	int iCount = 0;

	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		POSITION posPrev = Position;
		EoDbPrimitive* Primitive = GetNext(Position);
		if (Primitive->Is(EoDb::kTextPrimitive)) {
			if (static_cast<EoDbText*>(Primitive)->Text().GetLength() == 0) {
				RemoveAt(posPrev);
				delete Primitive;
				iCount++;
			}
		}
	}
	return (iCount);
}
bool EoDbGroup::SelectBy(const EoGeLineSeg3d& line, AeSysView* view) const {
	OdGePoint3dArray Intersections;

	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);
		if (Primitive->SelectBy(line, view, Intersections)) {
			return true;
		}
	}
	return false;
}
bool EoDbGroup::SelectBy(const OdGePoint3d& pt1, const OdGePoint3d& pt2, AeSysView* view) const {
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);
		if (Primitive->SelectBy(pt1, pt2, view)) {
			return true;
		}
	}
	return false;
}
EoDbPrimitive* EoDbGroup::SelectControlPointBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d* ptCtrl) {
	EoDbPrimitive* EngagedPrimitive = 0;

	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);

		if (Primitive == sm_PrimitiveToIgnore) {
			continue;
		}
		OdGePoint3d pt = Primitive->SelectAtControlPoint(view, point);

		if (EoDbPrimitive::ControlPointIndex() != SIZE_T_MAX) {
			EngagedPrimitive = Primitive;

			EoGePoint4d ptView4(pt, 1.);
			view->ModelViewTransformPoint(ptView4);
			*ptCtrl = ptView4.Convert3d();
		}
	}
	return (EngagedPrimitive);
}
EoDbPrimitive* EoDbGroup::SelPrimUsingPoint(const EoGePoint4d& point, AeSysView* view, double& dPicApert, OdGePoint3d& pDetPt) {
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);

		if (Primitive->SelectBy(point, view, pDetPt)) {
			dPicApert = point.DistanceToPointXY(EoGePoint4d(pDetPt, 1.));
			return (Primitive);
		}
	}
	return 0;
}
void EoDbGroup::SetPrimitiveToIgnore(EoDbPrimitive* primitive) {
	sm_PrimitiveToIgnore = primitive;
}
void EoDbGroup::SortTextOnY() {
	int iT;
	int iCount = (int) GetCount();

	do {
		iT = 0;

		POSITION Position = GetHeadPosition();
		for (int i = 1; i < iCount; i++) {
			POSITION pos1 = Position;
			EoDbPrimitive* pPrim1 = GetNext(pos1);

			POSITION pos2 = pos1;
			EoDbPrimitive* pPrim2 = GetNext(pos2);

			if (pPrim1->Is(EoDb::kTextPrimitive) && pPrim2->Is(EoDb::kTextPrimitive)) {
				double dY1 = static_cast<EoDbText*>(pPrim1)->Position().y;
				double dY2 = static_cast<EoDbText*>(pPrim2)->Position().y;
				if (dY1 < dY2) {
					SetAt(Position, pPrim2);
					SetAt(pos1, pPrim1);
					iT = i;
				}
			}
			else if (pPrim1->Is(EoDb::kTextPrimitive) || pPrim2->Is(EoDb::kTextPrimitive)) {
				SetAt(Position, pPrim2);
				SetAt(pos1, pPrim1);
				iT = i;
			}

			Position = pos1;
		}
		iCount = iT;
	}
	while (iT != 0);
}
void EoDbGroup::Square(AeSysView* view) {
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);
		if (Primitive->Is(EoDb::kLinePrimitive)) {
			static_cast<EoDbLine*>(Primitive)->Square(view);
		}
	}
}
void EoDbGroup::TransformBy(const EoGeMatrix3d& transformMatrix) {
	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = GetNext(Position);
		OdDbObjectId EntityObjectId = Primitive->EntityObjectId();
		if (!EntityObjectId.isNull()) {
			OdDbEntityPtr Entity = EntityObjectId.safeOpenObject(OdDb::kForWrite);
			Entity->transformBy(transformMatrix);
		}
		Primitive->TransformBy(transformMatrix);
	}
}
void EoDbGroup::Write(EoDbFile& file) {
	file.WriteUInt16(EoUInt16(GetCount()));

	for (POSITION Position = GetHeadPosition(); Position != 0;) {
		EoDbPrimitive* Primitive = GetNext(Position);
		Primitive->Write(file);
	}
}
void EoDbGroup::Write(CFile& file, EoByte* buffer) {
	// group flags
	buffer[0] = 0;
	// number of primitives in group
	*((EoInt16*) &buffer[1]) = EoInt16(GetCount());

	POSITION Position = GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive =	GetNext(Position);
		Primitive->Write(file, buffer);
	}
}
