#include "stdafx.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "PrimState.h"
unsigned short wPrvKeyDwn = 0;
OdGePoint3d rPrvPos;

void AeSysView::OnCutModeOptions() noexcept {}

void AeSysView::OnCutModeTorch() {
	auto Document {GetDocument()};
	const auto pt {GetCursorPosition()};
	auto Groups {new EoDbGroupList};
	OdGePoint3d ptCut;
	EoGePoint4d ptView(pt, 1.0);
	ModelViewTransformPoint(ptView);
	auto GroupPosition {GetFirstVisibleGroupPosition()};
	while (GroupPosition != nullptr) {
		const auto Group {GetNextVisibleGroup(GroupPosition)};
		auto PrimitivePosition {Group->GetHeadPosition()};
		while (PrimitivePosition != nullptr) {
			auto Primitive {Group->GetNext(PrimitivePosition)};
			if (Primitive->SelectUsingPoint(ptView, this, ptCut)) { // Pick point is within tolerance of primitive
				const auto NewGroup {new EoDbGroup};
				auto TransformMatrix {ModelViewMatrix()};
				ptCut.transformBy(TransformMatrix.invert());
				Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, Primitive);
				Primitive->CutAt(ptCut, NewGroup);
				Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, Primitive);
				Groups->AddTail(NewGroup);
				break;
			}
		}
	}
	Document->AddWorkLayerGroups(Groups);
	Document->UpdateGroupsInAllViews(EoDb::kGroupsSafe, Groups);
	delete Groups;
}

void AeSysView::OnCutModeSlice() {
	const auto ptCur {GetCursorPosition()};
	if (wPrvKeyDwn != ID_OP2) {
		rPrvPos = ptCur;
		RubberBandingStartAtEnable(ptCur, kLines);
		wPrvKeyDwn = ModeLineHighlightOp(ID_OP2);
	} else {
		const auto pt1 {rPrvPos};
		const auto pt2 {ptCur};
		auto Document {GetDocument()};
		auto Groups {new EoDbGroupList};
		OdGePoint3dArray Intersections;
		EoGePoint4d ptView[] = {EoGePoint4d(pt1, 1.0), EoGePoint4d(pt2, 1.0)};
		ModelViewTransformPoints(2, ptView);
		auto TransformMatrix {ModelViewMatrix()};
		TransformMatrix.invert();
		auto GroupPosition {GetFirstVisibleGroupPosition()};
		while (GroupPosition != nullptr) {
			const auto Group {GetNextVisibleGroup(GroupPosition)};
			if (Document->FindTrappedGroup(Group) != nullptr) {
				continue;
			}
			auto PrimitivePosition {Group->GetHeadPosition()};
			while (PrimitivePosition != nullptr) {
				auto Primitive {Group->GetNext(PrimitivePosition)};
				EoGeLineSeg3d LineSeg;
				LineSeg = EoGeLineSeg3d(ptView[0].Convert3d(), ptView[1].Convert3d());
				Primitive->SelectUsingLineSeg(LineSeg, this, Intersections);
				for (auto& Intersection : Intersections) {
					const auto NewGroup {new EoDbGroup};
					Intersection.transformBy(TransformMatrix);
					Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, Primitive);
					Primitive->CutAt(Intersection, NewGroup);
					Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, Primitive);
					Groups->AddTail(NewGroup);
				}
			}
		}
		Document->AddWorkLayerGroups(Groups);
		Document->UpdateGroupsInAllViews(EoDb::kGroupsSafe, Groups);
		delete Groups;
		RubberBandingDisable();
		ModeLineUnhighlightOp(wPrvKeyDwn);
	}
}

void AeSysView::OnCutModeField() {
	const auto DeviceContext {GetDC()};
	const auto CurrentPnt {GetCursorPosition()};
	if (wPrvKeyDwn != ID_OP4) {
		rPrvPos = CurrentPnt;
		RubberBandingStartAtEnable(CurrentPnt, kRectangles);
		wPrvKeyDwn = ModeLineHighlightOp(ID_OP4);
	} else {
		const OdGePoint3d LowerLeftCorner {EoMin(rPrvPos.x, CurrentPnt.x), EoMin(rPrvPos.y, CurrentPnt.y), 0.0};
		const OdGePoint3d UpperRightCorner {EoMax(rPrvPos.x, CurrentPnt.x), EoMax(rPrvPos.y, CurrentPnt.y), 0.0};
		int NumberOfIntersections;
		OdGePoint3d Intersections[10];
		auto Document {GetDocument()};
		const auto ColorIndex {g_PrimitiveState.ColorIndex()};
		const auto LinetypeIndex {g_PrimitiveState.LinetypeIndex()};
		auto GroupsOut {new EoDbGroupList};
		const auto GroupsIn {new EoDbGroupList};
		POSITION PreviousGroupPosition;
		for (auto GroupPosition = GetFirstVisibleGroupPosition(); (PreviousGroupPosition = GroupPosition) != nullptr;) {
			auto Group {GetNextVisibleGroup(GroupPosition)};
			if (Document->FindTrappedGroup(Group) != nullptr) {
				continue;
			}
			POSITION PrimitivePosition;
			POSITION PreviousPrimitivePosition;
			for (PrimitivePosition = Group->GetHeadPosition(); (PreviousPrimitivePosition = PrimitivePosition) != nullptr;) {
				auto Primitive {Group->GetNext(PrimitivePosition)};
				if ((NumberOfIntersections = Primitive->IsWithinArea(LowerLeftCorner, UpperRightCorner, Intersections)) == 0) {
					continue;
				}
				Group->RemoveAt(PreviousPrimitivePosition);
				for (auto i = 0; i < NumberOfIntersections; i += 2) {
					if (i != 0) {
						GroupsOut->RemoveTail();
					}
					Primitive->CutAt2Points(&Intersections[i], GroupsOut, GroupsIn, Database());
				}
			}
			if (Group->IsEmpty() != 0) { // seg was emptied remove from lists
				Document->AnyLayerRemove(Group);
				Document->RemoveGroupFromAllViews(Group);
				Group->DeletePrimitivesAndRemoveAll();
				delete Group;
			}
		}
		if (GroupsOut->GetCount() > 0) {
			Document->AddWorkLayerGroups(GroupsOut);
			Document->UpdateGroupsInAllViews(EoDb::kGroups, GroupsOut);
		}
		if (GroupsIn->GetCount() > 0) {
			Document->AddWorkLayerGroups(GroupsIn);
			Document->AddGroupsToTrap(GroupsIn);
		}
		delete GroupsIn;
		delete GroupsOut;
		g_PrimitiveState.SetPen(this, DeviceContext, ColorIndex, LinetypeIndex);
		UpdateStateInformation(kBothCounts);
		RubberBandingDisable();
		ModeLineUnhighlightOp(wPrvKeyDwn);
	}
}

void AeSysView::OnCutModeClip() {
	const auto DeviceContext {GetDC()};
	const auto ptCur {GetCursorPosition()};
	if (wPrvKeyDwn != ID_OP7) {
		rPrvPos = ptCur;
		wPrvKeyDwn = ModeLineHighlightOp(ID_OP7);
	} else {
		const auto pt1 {rPrvPos};
		const auto pt2 {ptCur};
		if (pt1 == pt2) {
			return;
		}
		double dRel[2];
		OdGePoint3d ptCut[2];
		const auto ColorIndex {g_PrimitiveState.ColorIndex()};
		const auto LinetypeIndex {g_PrimitiveState.LinetypeIndex()};
		auto Document {GetDocument()};
		auto TransformMatrix {ModelViewMatrix()};
		TransformMatrix.invert();
		EoGePoint4d ptView[] = {EoGePoint4d(pt1, 1.0), EoGePoint4d(pt2, 1.0)};
		ModelViewTransformPoints(2, ptView);
		const auto GroupsOut {new EoDbGroupList};
		const auto GroupsIn {new EoDbGroupList};
		POSITION PreviousGroupPosition;
		for (auto VisibleGroupPosition = GetFirstVisibleGroupPosition(); (PreviousGroupPosition = VisibleGroupPosition) != nullptr;) {
			auto Group {GetNextVisibleGroup(VisibleGroupPosition)};
			if (Document->FindTrappedGroup(Group) != nullptr) {
				continue;
			}
			POSITION FirstPrimitivePosition {nullptr};
			POSITION SecondPrimitivePosition;
			for (FirstPrimitivePosition = Group->GetHeadPosition(); (SecondPrimitivePosition = FirstPrimitivePosition) != nullptr;) {
				auto Primitive {Group->GetNext(FirstPrimitivePosition)};
				if (!Primitive->SelectUsingPoint(ptView[0], this, ptCut[0])) {
					continue;
				}
				dRel[0] = EoDbPrimitive::RelationshipOfPoint();
				if (!Primitive->SelectUsingPoint(ptView[1], this, ptCut[1])) {
					continue;
				}
				dRel[1] = EoDbPrimitive::RelationshipOfPoint();
				// Both pick points are within tolerance of primitive
				ptCut[0].transformBy(TransformMatrix);
				ptCut[1].transformBy(TransformMatrix);
				if (dRel[0] > dRel[1]) {
					const auto ptTmp {ptCut[0]};
					ptCut[0] = ptCut[1];
					ptCut[1] = ptTmp;
				}
				Group->RemoveAt(SecondPrimitivePosition);
				Primitive->CutAt2Points(ptCut, GroupsOut, GroupsIn, Database());
			}
			if (Group->IsEmpty() != 0) { // seg was emptied remove from lists
				Document->AnyLayerRemove(Group);
				Document->RemoveGroupFromAllViews(Group);
				Group->DeletePrimitivesAndRemoveAll();
				delete Group;
			}
		}
		if (GroupsOut->GetCount() > 0) {
			Document->AddWorkLayerGroups(GroupsOut);
			Document->UpdateGroupsInAllViews(EoDb::kGroups, GroupsOut);
		}
		if (GroupsIn->GetCount() > 0) {
			Document->AddWorkLayerGroups(GroupsIn);
			Document->AddGroupsToTrap(GroupsIn);
			Document->UpdateGroupsInAllViews(EoDb::kGroupsTrap, GroupsIn);
		}
		delete GroupsIn;
		delete GroupsOut;
		g_PrimitiveState.SetPen(this, DeviceContext, ColorIndex, LinetypeIndex);
		UpdateStateInformation(kBothCounts);
		ModeLineUnhighlightOp(wPrvKeyDwn);
	}
}

void AeSysView::OnCutModeDivide() noexcept {}

void AeSysView::OnCutModeReturn() {
	RubberBandingDisable();
	ModeLineUnhighlightOp(wPrvKeyDwn);
}

void AeSysView::OnCutModeEscape() {
	RubberBandingDisable();
	ModeLineUnhighlightOp(wPrvKeyDwn);
}
