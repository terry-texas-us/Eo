#include "stdafx.h"
#include "AeSysView.h"

#include "EoDbHatch.h"
#include "EoDbPolyline.h"

POSITION EoDbGroupList::AddHead(EoDbGroup* group) {
	return (CObList::AddHead((CObject*) group));
}
POSITION EoDbGroupList::AddTail(EoDbGroup* group) {
	return (CObList::AddTail((CObject*) group));
}
void EoDbGroupList::AddTail(EoDbGroupList* groupList) {
	CObList::AddTail((CObList*) groupList);
}
EoDbGroup* EoDbGroupList::GetNext(POSITION& position) {
	return (EoDbGroup*) CObList::GetNext(position);
}
EoDbGroup* EoDbGroupList::GetPrev(POSITION& position) {
	return (EoDbGroup*) CObList::GetPrev(position);
}
EoDbGroup* EoDbGroupList::RemoveHead() {
	return (EoDbGroup*) CObList::RemoveHead();
}
EoDbGroup* EoDbGroupList::RemoveTail() {
	return (EoDbGroup*) CObList::RemoveTail();
}
void EoDbGroupList::AddToTreeViewControl(HWND tree, HTREEITEM htiParent) {
	auto Position {GetHeadPosition()};

	while (Position != 0) {
		auto Group {GetNext(Position)};
		const auto htiSeg {Group->AddToTreeViewControl(tree, htiParent)};
		if (Group->GetCount() == 1) {
			TreeView_Expand(tree, htiSeg, TVE_EXPAND);
		}
	}
}
void EoDbGroupList::BreakPolylines() {
	auto Position {GetHeadPosition()};
	while (Position != 0) {
		auto Group {GetNext(Position)};
		Group->BreakPolylines();
	}
}
void EoDbGroupList::BreakSegRefs() {
	auto Position {GetHeadPosition()};
	while (Position != 0) {
		auto Group {GetNext(Position)};
		Group->BreakSegRefs();
	}
}
void EoDbGroupList::Display(AeSysView* view, CDC* deviceContext) {
	auto Position {GetHeadPosition()};
	while (Position != 0) {
		auto Group {GetNext(Position)};
		Group->Display(view, deviceContext);
	}
}
POSITION EoDbGroupList::Remove(EoDbGroup* group) {
	auto Position {Find(group)};
	if (Position != 0)
		RemoveAt(Position);

	return (Position);
}
int EoDbGroupList::GetBlockReferenceCount(const CString & name) {
	int Count = 0;

	auto Position {GetHeadPosition()};
	while (Position != 0) {
		const auto Group {GetNext(Position)};
		Count += Group->GetBlockReferenceCount(name);
	}
	return Count;
}
void EoDbGroupList::GetExtents__(AeSysView * view, OdGeExtents3d & extents) {
	auto Position {GetHeadPosition()};
	while (Position != 0) {
		auto Group {GetNext(Position)};
		Group->GetExtents_(view, extents);
	}
}
int EoDbGroupList::GetLinetypeIndexRefCount(short linetypeIndex) {
	int iCount = 0;

	auto Position {GetHeadPosition()};
	while (Position != 0) {
		auto Group {GetNext(Position)};
		iCount += Group->GetLinetypeIndexRefCount(linetypeIndex);
	}
	return (iCount);
}
void EoDbGroupList::ModifyColorIndex(short colorIndex) {
	auto Position {GetHeadPosition()};
	while (Position != 0) {
		auto Group {GetNext(Position)};
		Group->ModifyColorIndex(colorIndex);
	}
}
void EoDbGroupList::ModifyLinetypeIndex(short linetypeIndex) {
	auto Position {GetHeadPosition()};
	while (Position != 0)
		(GetNext(Position))->ModifyLinetypeIndex(linetypeIndex);
}
void EoDbGroupList::ModifyNotes(EoDbFontDefinition & fontDefinition, EoDbCharacterCellDefinition & characterCellDefinition, int iAtt) {
	auto Position {GetHeadPosition()};
	while (Position != 0) {
		auto Group {GetNext(Position)};
		Group->ModifyNotes(fontDefinition, characterCellDefinition, iAtt);
	}
}
void EoDbGroupList::PenTranslation(unsigned short wCols, short * pColNew, short * pCol) {
	auto Position {GetHeadPosition()};
	while (Position != 0)
		(GetNext(Position))->PenTranslation(wCols, pColNew, pCol);
}
void EoDbGroupList::RemoveDuplicatePrimitives() {
	auto Position {GetHeadPosition()};
	while (Position != 0) {
		(GetNext(Position))->RemoveDuplicatePrimitives();
	}
}
int EoDbGroupList::RemoveEmptyNotesAndDelete() {
	int iCount = 0;

	auto Position {GetHeadPosition()};
	while (Position != 0) {
		auto Group {GetNext(Position)};
		iCount += Group->RemoveEmptyNotesAndDelete();
	}
	return (iCount);
}
int EoDbGroupList::RemoveEmptyGroups() {
	int iCount = 0;

	auto Position {GetHeadPosition()};
	while (Position != 0) {
		auto posPrev {Position};
		auto Group {GetNext(Position)};
		if (Group->GetCount() == 0) {
			RemoveAt(posPrev);
			delete Group;
			iCount++;
		}
	}
	return (iCount);
}
void EoDbGroupList::DeleteGroupsAndRemoveAll() {
	auto Position {GetHeadPosition()};
	while (Position != 0) {
		auto Group {GetNext(Position)};
		Group->DeletePrimitivesAndRemoveAll();
		delete Group;
	}
	RemoveAll();
}
EoDbGroup* EoDbGroupList::SelectGroupBy(const OdGePoint3d & point) {
	auto ActiveView {AeSysView::GetActiveView()};

	OdGePoint3d ptEng;

	EoDbGroup* pPicSeg = 0;

	EoGePoint4d ptView(point, 1.0);
	ActiveView->ModelViewTransformPoint(ptView);

	double dPicApert = ActiveView->SelectApertureSize();

	EoDbHatch::SetEdgeToEvaluate(0);
	EoDbPolyline::SetEdgeToEvaluate(0);

	auto Position {GetHeadPosition()};
	while (Position != 0) {
		auto Group {GetNext(Position)};
		if (Group->SelPrimUsingPoint(ptView, ActiveView, dPicApert, ptEng) != 0) {
			pPicSeg = Group;
		}
	}
	return (pPicSeg);
}
void EoDbGroupList::TransformBy(const EoGeMatrix3d & transformMatrix) {
	auto Position {GetHeadPosition()};
	while (Position != 0) {
		auto Group {GetNext(Position)};
		Group->TransformBy(transformMatrix);
	}
}
void EoDbGroupList::Write(CFile& file, unsigned char* buffer) {
	auto Position {GetHeadPosition()};
	while (Position != 0)
		GetNext(Position)->Write(file, buffer);
}
