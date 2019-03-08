#include "stdafx.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

EoUInt16 wPrvKeyDwn = 0;
OdGePoint3d rPrvPos;

void AeSysView::OnCutModeOptions(void) {
}
void AeSysView::OnCutModeTorch(void) {
	AeSysDoc* Document = GetDocument();

	OdGePoint3d pt = GetCursorPosition();
	EoDbGroupList* Groups = new EoDbGroupList;

	OdGePoint3d ptCut;

	EoGePoint4d ptView(pt, 1.);
	ModelViewTransformPoint(ptView);

	POSITION GroupPosition = GetFirstVisibleGroupPosition();
	while (GroupPosition != 0) {
		EoDbGroup* Group = GetNextVisibleGroup(GroupPosition);

		POSITION PrimitivePosition = Group->GetHeadPosition();
		while (PrimitivePosition != 0) {
			EoDbPrimitive* Primitive = Group->GetNext(PrimitivePosition);

			if (Primitive->SelectBy(ptView, this, ptCut)) { // Pick point is within tolerance of primative
				EoDbGroup* NewGroup = new EoDbGroup;

				EoGeMatrix3d TransformMatrix = ModelViewMatrix();
				ptCut.transformBy(TransformMatrix.invert());
				Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, Primitive);
				Primitive->CutAt(ptCut, NewGroup, Database());
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
void AeSysView::OnCutModeSlice(void) {
	OdGePoint3d ptCur = GetCursorPosition();
	if (wPrvKeyDwn != ID_OP2) {
		rPrvPos = ptCur;
		RubberBandingStartAtEnable(ptCur, Lines);
		wPrvKeyDwn = ModeLineHighlightOp(ID_OP2);
	}
	else {
		OdGePoint3d pt1 = rPrvPos;
		OdGePoint3d pt2 = ptCur;

		AeSysDoc* Document = GetDocument();

		EoDbGroupList* Groups = new EoDbGroupList;

		OdGePoint3dArray Intersections;

		EoGePoint4d ptView[] = {EoGePoint4d(pt1, 1.), EoGePoint4d(pt2, 1.)};
		ModelViewTransformPoints(2, ptView);

		EoGeMatrix3d TransformMatrix = ModelViewMatrix();
		TransformMatrix.invert();

		POSITION GroupPosition = GetFirstVisibleGroupPosition();
		while (GroupPosition != 0) {
			EoDbGroup* Group = GetNextVisibleGroup(GroupPosition);

			if (Document->FindTrappedGroup(Group) != 0)
				continue;

			POSITION PrimitivePosition = Group->GetHeadPosition();
			while (PrimitivePosition != 0) {
				EoDbPrimitive* Primitive = Group->GetNext(PrimitivePosition);

				EoGeLineSeg3d ln;
				ln = EoGeLineSeg3d(ptView[0].Convert3d(), ptView[1].Convert3d());
				Primitive->SelectBy(ln, this, Intersections);
				for (EoUInt16 w = 0; w < Intersections.size(); w++) {
					EoDbGroup* NewGroup = new EoDbGroup;

					Intersections[w].transformBy(TransformMatrix);

					Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, Primitive);
					Primitive->CutAt(Intersections[w], NewGroup, Database());
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
void AeSysView::OnCutModeField(void) {
	CDC* DeviceContext = GetDC();
	OdGePoint3d ptCur = GetCursorPosition();
	if (wPrvKeyDwn != ID_OP4) {
		rPrvPos = ptCur;
		RubberBandingStartAtEnable(ptCur, Rectangles);
		wPrvKeyDwn = ModeLineHighlightOp(ID_OP4);
	}
	else {
		OdGePoint3d rLL, rUR;

		rLL.x = EoMin(rPrvPos.x, ptCur.x);
		rLL.y = EoMin(rPrvPos.y, ptCur.y);
		rUR.x = EoMax(rPrvPos.x, ptCur.x);
		rUR.y = EoMax(rPrvPos.y, ptCur.y);

		OdGePoint3d ptLL = rLL;
		OdGePoint3d ptUR = rUR;

		EoDbGroup* Group;
		EoDbPrimitive* Primitive;

		int 	iInts;
		OdGePoint3d	Intersections[10];

		AeSysDoc* Document = GetDocument();

		EoInt16 ColorIndex = pstate.ColorIndex();
		EoInt16 LinetypeIndex = pstate.LinetypeIndex();

		EoDbGroupList* GroupsOut = new EoDbGroupList;
		EoDbGroupList* GroupsIn = new EoDbGroupList;

		POSITION posSeg, posSegPrv;
		for (posSeg = GetFirstVisibleGroupPosition(); (posSegPrv = posSeg) != 0;) {
			Group = GetNextVisibleGroup(posSeg);

			if (Document->FindTrappedGroup(Group) != 0)
				continue;

			POSITION PrimitivePosition, posPrimPrv;
			for (PrimitivePosition = Group->GetHeadPosition(); (posPrimPrv = PrimitivePosition) != 0;) {
				Primitive = Group->GetNext(PrimitivePosition);

				if ((iInts = Primitive->IsWithinArea(ptLL, ptUR, Intersections)) == 0)
					continue;

				Group->RemoveAt(posPrimPrv);

				for (int i = 0; i < iInts; i += 2) {
					if (i != 0)
						GroupsOut->RemoveTail();
					Primitive->CutAt2Points(&Intersections[i], GroupsOut, GroupsIn, Database());
				}
			}
			if (Group->IsEmpty()) { // seg was emptied remove from lists
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

		pstate.SetPen(this, DeviceContext, ColorIndex, LinetypeIndex);
		UpdateStateInformation(BothCounts);

		RubberBandingDisable();
		ModeLineUnhighlightOp(wPrvKeyDwn);
	}
}
void AeSysView::OnCutModeClip(void) {
	CDC* DeviceContext = GetDC();
	OdGePoint3d ptCur = GetCursorPosition();
	if (wPrvKeyDwn != ID_OP7) {
		rPrvPos = ptCur;
		wPrvKeyDwn = ModeLineHighlightOp(ID_OP7);
	}
	else {
		OdGePoint3d pt1 = rPrvPos;
		OdGePoint3d pt2 = ptCur;

		if (pt1 == pt2) {
			return;
		}

		double dRel[2];
		OdGePoint3d	ptCut[2];

		EoInt16 ColorIndex = pstate.ColorIndex();
		EoInt16 LinetypeIndex = pstate.LinetypeIndex();

		AeSysDoc* Document = GetDocument();

		EoGeMatrix3d TransformMatrix = ModelViewMatrix();
		TransformMatrix.invert();

		EoGePoint4d ptView[] = {EoGePoint4d(pt1, 1.), EoGePoint4d(pt2, 1.)};

		ModelViewTransformPoints(2, ptView);

		EoDbGroupList* GroupsOut = new EoDbGroupList;
		EoDbGroupList* GroupsIn = new EoDbGroupList;

		POSITION posSeg;
		POSITION posSegPrv;

		for (posSeg = GetFirstVisibleGroupPosition(); (posSegPrv = posSeg) != 0;) {
			EoDbGroup* Group = GetNextVisibleGroup(posSeg);

			if (Document->FindTrappedGroup(Group) != 0)
				continue;

			POSITION posPrim1;
			POSITION posPrim2;

			for (posPrim1 = Group->GetHeadPosition(); (posPrim2 = posPrim1) != 0;) {
				EoDbPrimitive* Primitive = Group->GetNext(posPrim1);

				if (!Primitive->SelectBy(ptView[0], this, ptCut[0]))
					continue;
				dRel[0] = EoDbPrimitive::RelationshipOfPoint();
				if (!Primitive->SelectBy(ptView[1], this, ptCut[1]))
					continue;
				dRel[1] = EoDbPrimitive::RelationshipOfPoint();
				// Both pick points are within tolerance of primative
				ptCut[0].transformBy(TransformMatrix);
				ptCut[1].transformBy(TransformMatrix);
				if (dRel[0] > dRel[1]) {
					OdGePoint3d ptTmp = ptCut[0];
					ptCut[0] = ptCut[1]; ptCut[1] = ptTmp;
				}
				Group->RemoveAt(posPrim2);

				Primitive->CutAt2Points(ptCut, GroupsOut, GroupsIn, Database());
			}
			if (Group->IsEmpty()) { // seg was emptied remove from lists
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

		pstate.SetPen(this, DeviceContext, ColorIndex, LinetypeIndex);
		UpdateStateInformation(BothCounts);

		ModeLineUnhighlightOp(wPrvKeyDwn);
	}
}
void AeSysView::OnCutModeDivide(void) {
}
void AeSysView::OnCutModeReturn(void) {
	RubberBandingDisable();
	ModeLineUnhighlightOp(wPrvKeyDwn);
}
void AeSysView::OnCutModeEscape(void) {
	RubberBandingDisable();
	ModeLineUnhighlightOp(wPrvKeyDwn);
}
