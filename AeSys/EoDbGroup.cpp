#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

#include "EoDbFile.h"
#include "EoDbBlockReference.h"
#include "EoDbPolyline.h"

EoDbPrimitive* EoDbGroup::sm_PrimitiveToIgnore = static_cast<EoDbPrimitive*>(NULL);

EoDbGroup::EoDbGroup() noexcept {}

EoDbGroup::EoDbGroup(const EoDbBlock& block) {
    auto Database {AeSysDoc::GetDoc()->m_DatabasePtr};

    auto Position {block.GetHeadPosition()};
    while (Position != 0) {
        AddTail((block.GetNext(Position))->Clone(Database));
    }
}

EoDbGroup::EoDbGroup(const EoDbGroup& group) {
    auto Database {AeSysDoc::GetDoc()->m_DatabasePtr};

    auto Position {group.GetHeadPosition()};
    while (Position != 0) {
        AddTail((group.GetNext(Position))->Clone(Database));
    }
}

EoDbGroup::~EoDbGroup() {}

void EoDbGroup::AddPrimsToTreeViewControl(HWND tree, HTREEITEM parent) {
    auto Position {GetHeadPosition()};
	while (Position != 0) {
        const auto Primitive {GetNext(Position)};
		Primitive->AddToTreeViewControl(tree, parent);
	}
}
HTREEITEM EoDbGroup::AddToTreeViewControl(HWND tree, HTREEITEM parent) {
    auto TreeItem {CMainFrame::InsertTreeViewControlItem(tree, parent, L"<Group>", this)};
	AddPrimsToTreeViewControl(tree, TreeItem);
	return TreeItem;
}
void EoDbGroup::BreakPolylines() {
    auto Database {AeSysDoc::GetDoc()->m_DatabasePtr};
    OdDbBlockTableRecordPtr BlockTableRecord = Database->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    auto Position {GetHeadPosition()};
	while (Position != 0) {
		POSITION PrimitivePosition = Position;
        auto Primitive {GetNext(Position)};
		if (Primitive->Is(EoDb::kPolylinePrimitive)) {
			const auto Polyline = dynamic_cast<EoDbPolyline*>(Primitive);
			OdGePoint3dArray Points;
			Polyline->GetAllPoints(Points);

            for (OdUInt16 w = 0; w < Points.size() - 1; w++) {
                auto Line = EoDbLine::Create(BlockTableRecord, Points[w], Points[w + 1]);
                Line->setColorIndex(Primitive->ColorIndex());
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(Primitive->LinetypeIndex()));
				CObList::InsertBefore(PrimitivePosition, EoDbLine::Create(Line));
			}
			
            if (Polyline->IsClosed()) {
                auto Line = EoDbLine::Create(BlockTableRecord, Points[Points.size() - 1], Points[0]);
				Line->setColorIndex(Primitive->ColorIndex());
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(Primitive->LinetypeIndex()));
				CObList::InsertBefore(PrimitivePosition, EoDbLine::Create(Line));
			}
			this->RemoveAt(PrimitivePosition);
			delete Primitive;
		}
		else if (Primitive->Is(EoDb::kGroupReferencePrimitive)) {
			EoDbBlock* Block;
			if (AeSysDoc::GetDoc()->LookupBlock(dynamic_cast<EoDbBlockReference*>(Primitive)->Name(), Block) != 0) {
				Block->BreakPolylines();
			}
		}
	}
}
void EoDbGroup::BreakSegRefs() {
	int iSegRefs;
	do {
		iSegRefs = 0;
        auto Position {GetHeadPosition()};
		while (Position != 0) {
            auto PrimitivePosition {Position};
            auto Primitive {GetNext(Position)};
			if (Primitive->Is(EoDb::kGroupReferencePrimitive)) {
				iSegRefs++;
				EoDbBlock* Block;
				if (AeSysDoc::GetDoc()->LookupBlock(dynamic_cast<EoDbBlockReference*>(Primitive)->Name(), Block) != 0) {
                    auto pSegT {new EoDbGroup(*Block)};
					EoGeMatrix3d tm = dynamic_cast<EoDbBlockReference*>(Primitive)->BlockTransformMatrix(Block->BasePoint());
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
    auto Position {GetHeadPosition()};
	while (Position != 0) {
        auto Primitive {GetNext(Position)};
		const OdDbObjectId EntityObjectId = Primitive->EntityObjectId();
		if (!EntityObjectId.isNull()) {
			OdDbEntityPtr Entity = EntityObjectId.safeOpenObject(OdDb::kForWrite);
			Entity->erase();
		}
		delete Primitive;
	}
	RemoveAll();
}

void EoDbGroup::Display(AeSysView* view, CDC* deviceContext) {
    auto Position {GetHeadPosition()};
	while (Position != 0) {
        auto Primitive {GetNext(Position)};
		Primitive->Display(view, deviceContext);
	}
}
void EoDbGroup::Erase() {
    auto Position {GetHeadPosition()};
	while (Position != 0) {
        const auto Primitive {GetNext(Position)};
		const OdDbObjectId EntityObjectId = Primitive->EntityObjectId();
		if (!EntityObjectId.isNull()) {
			OdDbEntityPtr Entity = EntityObjectId.safeOpenObject(OdDb::kForWrite);
			Entity->erase();
		}
	}
}
void EoDbGroup::UndoErase() {
    auto Position {GetHeadPosition()};
	while (Position != 0) {
        const auto Primitive {GetNext(Position)};
		const OdDbObjectId EntityObjectId = Primitive->EntityObjectId();
		if (!EntityObjectId.isNull()) {
			OdDbEntityPtr Entity = EntityObjectId.safeOpenObject(OdDb::kForWrite, true);
			Entity->erase(false);
		}
	}
}
POSITION EoDbGroup::FindAndRemovePrimitive(EoDbPrimitive* primitive) {
    auto Position {Find(primitive)};
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

    auto Position {GetHeadPosition()};
	while (Position != 0) {
        auto Primitive {GetNext(Position)};
		if (Primitive->Is(EoDb::kGroupReferencePrimitive)) {
			if (dynamic_cast<EoDbBlockReference*>(Primitive)->Name() == name)
				Count++;
		}
	}
	return Count;
}
void EoDbGroup::GetExtents_(AeSysView* view, OdGeExtents3d& extents) {
	OdGeExtents3d GroupExtents;
    auto Position {GetHeadPosition()};
	while (Position != 0) {
        const auto Primitive {GetNext(Position)};
		Primitive->GetExtents(view, GroupExtents);
	}
	if (GroupExtents.isValidExtents()) {
		GroupExtents.transformBy(view->ModelToWorldTransform());
		extents.addExt(GroupExtents);
	}
}
EoDbPoint* EoDbGroup::GetFirstDifferentPoint(EoDbPoint* pointPrimitive) {
    auto Position {GetHeadPosition()};
	while (Position != 0) {
        auto Primitive {GetNext(Position)};
		if (Primitive != pointPrimitive && Primitive->Is(EoDb::kPointPrimitive)) {
			return (dynamic_cast<EoDbPoint*>(Primitive));
		}
	}
	return 0;
}
int EoDbGroup::GetLinetypeIndexRefCount(OdInt16 linetypeIndex) {
	int Count = 0;

    auto Position {GetHeadPosition()};
	while (Position != 0) {
        const auto Primitive {GetNext(Position)};

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
    auto Position {group->GetHeadPosition()};
	while (Position != 0) {
        auto Primitive {group->GetNext(Position)};
		CObList::InsertBefore(insertPosition, (CObject*) Primitive);
	}
}
bool EoDbGroup::IsInView(AeSysView* view) const {
    auto Position {GetHeadPosition()};
	while (Position != 0) {
        const auto Primitive {GetNext(Position)};
		if (Primitive && Primitive->IsInView(view)) {
			return true;
		}
	}
	return false;
}
bool EoDbGroup::IsOn(const EoGePoint4d& point, AeSysView* view) const {
	OdGePoint3d Point;
    auto Position {GetHeadPosition()};
	while (Position != 0) {
        const auto Primitive {GetNext(Position)};
		if (Primitive->SelectBy(point, view, Point)) {
			return true;
		}
	}
	return false;
}

void EoDbGroup::ModifyColorIndex(OdInt16 colorIndex) {
    auto Position {GetHeadPosition()};
	while (Position != 0) {
        auto Primitive {GetNext(Position)};
		Primitive->SetColorIndex2(colorIndex);
	}
}
void EoDbGroup::ModifyLinetypeIndex(OdInt16 linetypeIndex) {
    auto Position {GetHeadPosition()};
	while (Position != 0) {
        auto Primitive {GetNext(Position)};
		Primitive->SetLinetypeIndex2(linetypeIndex);
	}
}
void EoDbGroup::ModifyNotes(EoDbFontDefinition& fontDefinition, EoDbCharacterCellDefinition& characterCellDefinition, int iAtt) {
    auto Position {GetHeadPosition()};
	while (Position != 0) {
        auto Primitive {GetNext(Position)};
		if (Primitive->Is(EoDb::kTextPrimitive)) {
			dynamic_cast<EoDbText*>(Primitive)->ModifyNotes(fontDefinition, characterCellDefinition, iAtt);
		}
	}
}
void EoDbGroup::PenTranslation(OdUInt16 wCols, OdInt16* pColNew, OdInt16* pCol) {
    auto Position {GetHeadPosition()};
	while (Position != 0) {
        auto Primitive {GetNext(Position)};

		for (OdUInt16 w = 0; w < wCols; w++) {
			if (Primitive->ColorIndex() == pCol[w]) {
				Primitive->SetColorIndex2(pColNew[w]);
				break;
			}
		}
	}
}
// <tas="Is no longer working!"</tas>
void EoDbGroup::RemoveDuplicatePrimitives() {
    auto BasePosition {GetHeadPosition()};
	while (BasePosition != 0) {
		const EoDbPrimitive* BasePrimitive = GetNext(BasePosition);

        auto TestPosition {BasePosition};
		while (TestPosition != 0) {
            auto TestPositionSave {TestPosition};
            auto TestPrimitive {GetNext(TestPosition)};

			if (BasePrimitive->IsEqualTo(TestPrimitive)) {
				RemoveAt(TestPositionSave);
				delete TestPrimitive;
			}
		}
	}
}
int EoDbGroup::RemoveEmptyNotesAndDelete() {
	int iCount = 0;

    auto Position {GetHeadPosition()};
	while (Position != 0) {
        auto posPrev {Position};
        auto Primitive {GetNext(Position)};
		if (Primitive->Is(EoDb::kTextPrimitive)) {
			if (dynamic_cast<EoDbText*>(Primitive)->Text().GetLength() == 0) {
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

    auto Position {GetHeadPosition()};
	while (Position != 0) {
        auto Primitive {GetNext(Position)};
		if (Primitive->SelectBy(line, view, Intersections)) {
			return true;
		}
	}
	return false;
}
bool EoDbGroup::SelectBy(const OdGePoint3d& pt1, const OdGePoint3d& pt2, AeSysView* view) const {
    auto Position {GetHeadPosition()};
	while (Position != 0) {
        const auto Primitive {GetNext(Position)};
		if (Primitive->SelectBy(pt1, pt2, view)) {
			return true;
		}
	}
	return false;
}
EoDbPrimitive* EoDbGroup::SelectControlPointBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d* ptCtrl) {
	EoDbPrimitive* EngagedPrimitive = 0;

    auto Position {GetHeadPosition()};
	while (Position != 0) {
        auto Primitive {GetNext(Position)};

		if (Primitive == sm_PrimitiveToIgnore) {
			continue;
		}
        const auto pt {Primitive->SelectAtControlPoint(view, point)};

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
    auto Position {GetHeadPosition()};
	while (Position != 0) {
        auto Primitive {GetNext(Position)};

		if (Primitive->SelectBy(point, view, pDetPt)) {
			dPicApert = point.DistanceToPointXY(EoGePoint4d(pDetPt, 1.));
			return (Primitive);
		}
	}
	return 0;
}
void EoDbGroup::SetPrimitiveToIgnore(EoDbPrimitive* primitive) noexcept {
	sm_PrimitiveToIgnore = primitive;
}
void EoDbGroup::SortTextOnY() {
	int iT;
	int iCount = (int) GetCount();

	do {
		iT = 0;

        auto Position {GetHeadPosition()};
		for (int i = 1; i < iCount; i++) {
            auto pos1 {Position};
            auto pPrim1 {GetNext(pos1)};

            auto pos2 {pos1};
            auto pPrim2 {GetNext(pos2)};

			if (pPrim1->Is(EoDb::kTextPrimitive) && pPrim2->Is(EoDb::kTextPrimitive)) {
                const auto dY1 {dynamic_cast<EoDbText*>(pPrim1)->Position().y};
                const auto dY2 {dynamic_cast<EoDbText*>(pPrim2)->Position().y};
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
    auto Position {GetHeadPosition()};
	while (Position != 0) {
        auto Primitive {GetNext(Position)};
		if (Primitive->Is(EoDb::kLinePrimitive)) {
			dynamic_cast<EoDbLine*>(Primitive)->Square(view);
		}
	}
}
void EoDbGroup::TransformBy(const EoGeMatrix3d& transformMatrix) {
    auto Position {GetHeadPosition()};
	while (Position != 0) {
        auto Primitive {GetNext(Position)};
		const OdDbObjectId EntityObjectId = Primitive->EntityObjectId();
		if (!EntityObjectId.isNull()) {
			OdDbEntityPtr Entity = EntityObjectId.safeOpenObject(OdDb::kForWrite);
			Entity->transformBy(transformMatrix);
		}
		Primitive->TransformBy(transformMatrix);
	}
}
void EoDbGroup::Write(EoDbFile& file) {
	file.WriteUInt16(OdUInt16(GetCount()));

	for (auto Position = GetHeadPosition(); Position != 0;) {
        const auto Primitive {GetNext(Position)};
		Primitive->Write(file);
	}
}
void EoDbGroup::Write(CFile& file, OdUInt8* buffer) {
	// group flags
	buffer[0] = 0;
	// number of primitives in group
	*((OdInt16*) &buffer[1]) = OdInt16(GetCount());

    auto Position {GetHeadPosition()};
	while (Position != 0) {
        const auto Primitive {GetNext(Position)};
		Primitive->Write(file, buffer);
	}
}

std::pair<EoDbGroup*, OdDbGroupPtr> EoDbGroup::Create(OdDbDatabasePtr database) {
    OdDbDictionaryPtr GroupDictionary {database->getGroupDictionaryId().safeOpenObject(OdDb::kForWrite)};
    auto Group {OdDbGroup::createObject()}; // do not attempt to add entries to the newly created group before adding the group to the group dictionary.
    GroupDictionary->setAt(L"*", Group);

    Group->setSelectable(true);
    Group->setAnonymous();

    return {new EoDbGroup, Group};
}
