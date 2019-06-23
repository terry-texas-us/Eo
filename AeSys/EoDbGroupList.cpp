#include "stdafx.h"
#include "AeSysView.h"
#include "EoDbHatch.h"
#include "EoDbPolyline.h"

void EoDbGroupList::AddToTreeViewControl(const HWND tree, const HTREEITEM htiParent) {
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		auto Group {GetNext(Position)};
		const auto TreeItem {Group->AddToTreeViewControl(tree, htiParent)};
		if (Group->GetCount() == 1) { TreeView_Expand(tree, TreeItem, TVE_EXPAND); }
	}
}

void EoDbGroupList::BreakPolylines() {
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		GetNext(Position)->BreakPolylines();
	}
}

void EoDbGroupList::BreakSegRefs() {
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		GetNext(Position)->BreakSegRefs();
	}
}

void EoDbGroupList::Display(AeSysView* view, CDC* deviceContext) {
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		GetNext(Position)->Display(view, deviceContext);
	}
}

POSITION EoDbGroupList::Remove(EoDbGroup* group) {
	const auto Position {Find(group)};
	if (Position != nullptr) { RemoveAt(Position); }
	return Position;
}

int EoDbGroupList::GetBlockReferenceCount(const CString& name) {
	auto Count {0};
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		Count += GetNext(Position)->GetBlockReferenceCount(name);
	}
	return Count;
}

void EoDbGroupList::GetExtents__(AeSysView* view, OdGeExtents3d& extents) {
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		GetNext(Position)->GetExtents_(view, extents);
	}
}

int EoDbGroupList::GetLinetypeIndexRefCount(const short linetypeIndex) {
	auto Count {0};
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		Count += GetNext(Position)->GetLinetypeIndexRefCount(linetypeIndex);
	}
	return Count;
}

void EoDbGroupList::ModifyColorIndex(const short colorIndex) {
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		GetNext(Position)->ModifyColorIndex(colorIndex);
	}
}

void EoDbGroupList::ModifyLinetypeIndex(const short linetypeIndex) {
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		GetNext(Position)->ModifyLinetypeIndex(linetypeIndex);
	}
}

void EoDbGroupList::ModifyNotes(EoDbFontDefinition& fontDefinition, EoDbCharacterCellDefinition& characterCellDefinition, const int iAtt) {
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		GetNext(Position)->ModifyNotes(fontDefinition, characterCellDefinition, iAtt);
	}
}

void EoDbGroupList::PenTranslation(const unsigned numberOfColors, std::vector<int>& newColors, std::vector<int>& pCol) {
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		GetNext(Position)->PenTranslation(numberOfColors, newColors, pCol);
	}
}

void EoDbGroupList::RemoveDuplicatePrimitives() {
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		GetNext(Position)->RemoveDuplicatePrimitives();
	}
}

int EoDbGroupList::RemoveEmptyNotesAndDelete() {
	auto Count {0};
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		Count += GetNext(Position)->RemoveEmptyNotesAndDelete();
	}
	return Count;
}

int EoDbGroupList::RemoveEmptyGroups() {
	auto Count {0};
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		const auto posPrev {Position};
		const auto Group {GetNext(Position)};
		if (Group->GetCount() == 0) {
			RemoveAt(posPrev);
			delete Group;
			Count++;
		}
	}
	return Count;
}

void EoDbGroupList::DeleteGroupsAndRemoveAll() {
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		auto Group {GetNext(Position)};
		Group->DeletePrimitivesAndRemoveAll();
		delete Group;
	}
	RemoveAll();
}

EoDbGroup* EoDbGroupList::SelectGroupBy(const OdGePoint3d& point) {
	auto ActiveView {AeSysView::GetActiveView()};
	OdGePoint3d PointAtSelection;
	EoDbGroup* SelectedGroup {nullptr};
	EoGePoint4d ptView(point, 1.0);
	ActiveView->ModelViewTransformPoint(ptView);
	auto ApertureSize {ActiveView->SelectApertureSize()};
	EoDbHatch::SetEdgeToEvaluate(0);
	EoDbPolyline::SetEdgeToEvaluate(0);
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		auto Group {GetNext(Position)};
		if (Group->SelectPrimitiveUsingPoint(ptView, ActiveView, ApertureSize, PointAtSelection) != nullptr) { SelectedGroup = Group; }
	}
	return SelectedGroup;
}

void EoDbGroupList::TransformBy(const EoGeMatrix3d& transformMatrix) {
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		GetNext(Position)->TransformBy(transformMatrix);
	}
}

void EoDbGroupList::Write(CFile& file, unsigned char* buffer) {
	auto Position {GetHeadPosition()};
	while (Position != nullptr) {
		GetNext(Position)->Write(file, buffer);
	}
}
