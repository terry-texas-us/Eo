#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDlgTrapModify.h"

#include "EoDbHatch.h"
#include "EoDbPolyline.h"

void AeSysView::OnTrapModeRemoveAdd() {
	theApp.OnTrapCommandsAddGroups();
}

void AeSysView::OnTrapModePoint() {
	AeSysDoc* Document = GetDocument();

	EoGePoint4d ptView(GetCursorPosition(), 1.);
	ModelViewTransformPoint(ptView);

	EoDbHatch::SetEdgeToEvaluate(0);
	EoDbPolyline::SetEdgeToEvaluate(0);

	POSITION Position = GetFirstVisibleGroupPosition();
	while (Position != 0) {
		EoDbGroup* Group = GetNextVisibleGroup(Position);

		if (Document->FindTrappedGroup(Group) != 0) continue;

		if (Group->IsOn(ptView, this)) {
			Document->AddGroupToTrap(Group);
		}
	}
	UpdateStateInformation(TrapCount);
}

void AeSysView::OnTrapModeStitch() {
	if (m_PreviousOp != ID_OP2) {
		m_PreviousPnt = GetCursorPosition();
		RubberBandingStartAtEnable(m_PreviousPnt, Lines);
		m_PreviousOp = ModeLineHighlightOp(ID_OP2);
	}
	else {
		const OdGePoint3d pt = GetCursorPosition();

		if (m_PreviousPnt == pt) return;

		AeSysDoc* Document = GetDocument();

		EoGePoint4d ptView[] = {EoGePoint4d(m_PreviousPnt, 1.), EoGePoint4d(pt, 1.)};

		ModelViewTransformPoints(2, ptView);

		POSITION Position = GetFirstVisibleGroupPosition();
		while (Position != 0) {
			EoDbGroup* Group = GetNextVisibleGroup(Position);

			if (Document->FindTrappedGroup(Group) != 0) continue;

			if (Group->SelectBy(EoGeLineSeg3d(ptView[0].Convert3d(), ptView[1].Convert3d()), this)) {
				Document->AddGroupToTrap(Group);
			}
		}
		RubberBandingDisable();
		ModeLineUnhighlightOp(m_PreviousOp);
		UpdateStateInformation(TrapCount);
	}
}

void AeSysView::OnTrapModeField() {
	if (m_PreviousOp != ID_OP4) {
		m_PreviousPnt = GetCursorPosition();
		RubberBandingStartAtEnable(m_PreviousPnt, Rectangles);
		m_PreviousOp = ModeLineHighlightOp(ID_OP4);
	}
	else {
		const OdGePoint3d pt = GetCursorPosition();
		if (m_PreviousPnt == pt) return;

		AeSysDoc* Document = GetDocument();

		EoGePoint4d ptView[] = {EoGePoint4d(m_PreviousPnt, 1.), EoGePoint4d(pt, 1.)};

		ModelViewTransformPoints(2, ptView);

		const OdGePoint3d ptMin = EoGePoint4d::Min(ptView[0], ptView[1]).Convert3d();
		const OdGePoint3d ptMax = EoGePoint4d::Max(ptView[0], ptView[1]).Convert3d();

		POSITION Position = GetFirstVisibleGroupPosition();
		while (Position != 0) {
			EoDbGroup* Group = GetNextVisibleGroup(Position);

			if (Document->FindTrappedGroup(Group) != 0) continue;

			if (Group->SelectBy(ptMin, ptMax, this)) {
				Document->AddGroupToTrap(Group);
			}
		}
		RubberBandingDisable();
		ModeLineUnhighlightOp(m_PreviousOp);
		UpdateStateInformation(TrapCount);
	}
}

void AeSysView::OnTrapModeLast() {
	AeSysDoc* Document = GetDocument();

	POSITION Position = Document->GetLastWorkLayerGroupPosition();
	while (Position != 0) {
		EoDbGroup* Group = Document->GetPreviousWorkLayerGroup(Position);

		if (!Document->FindTrappedGroup(Group)) {
			Document->AddGroupToTrap(Group);
			UpdateStateInformation(TrapCount);
			break;
		}
	}
}

void AeSysView::OnTrapModeEngage() {
	if (GroupIsEngaged()) {
		AeSysDoc* Document = GetDocument();

		POSITION Position = Document->FindWorkLayerGroup(EngagedGroup());

		EoDbGroup* Group = Document->GetNextWorkLayerGroup(Position);

		if (Document->FindTrappedGroup(Group) == 0) {
			Document->AddGroupToTrap(Group);
			UpdateStateInformation(TrapCount);
		}
	}
	else {
		theApp.AddModeInformationToMessageList();
	}
}
void AeSysView::OnTrapModeMenu() {
	CPoint CurrentPosition;
	::GetCursorPos(&CurrentPosition);
	HMENU TrapMenu = ::LoadMenu(theApp.GetInstance(), MAKEINTRESOURCE(IDR_TRAP));
	CMenu* SubMenu = CMenu::FromHandle(::GetSubMenu(TrapMenu, 0));
	SubMenu->TrackPopupMenuEx(0, CurrentPosition.x, CurrentPosition.y, AfxGetMainWnd(), 0);
	::DestroyMenu(TrapMenu);
}
void AeSysView::OnTrapModeModify() {
	if (!GetDocument()->IsTrapEmpty()) {
		EoDlgTrapModify Dialog(GetDocument());
		if (Dialog.DoModal() == IDOK) {
			GetDocument()->UpdateAllViews(nullptr);
		}
	}
	else {
		theApp.AddModeInformationToMessageList();
	}
}

void AeSysView::OnTrapModeEscape() {
	RubberBandingDisable();
	ModeLineUnhighlightOp(m_PreviousOp);
}

void AeSysView::OnTraprModeRemoveAdd() {
	theApp.OnTrapCommandsAddGroups();
}

void AeSysView::OnTraprModePoint() {
	AeSysDoc* Document = GetDocument();

	EoGePoint4d ptView(GetCursorPosition(), 1.);
	ModelViewTransformPoint(ptView);

	EoDbHatch::SetEdgeToEvaluate(0);
	EoDbPolyline::SetEdgeToEvaluate(0);

	POSITION Position = Document->GetFirstTrappedGroupPosition();
	while (Position != 0) {
		EoDbGroup* Group = Document->GetNextTrappedGroup(Position);

		if (Group->IsOn(ptView, this)) {
			Document->RemoveTrappedGroupAt(Document->FindTrappedGroup(Group));
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
		}
	}
	UpdateStateInformation(TrapCount);
}

void AeSysView::OnTraprModeStitch() {
	if (m_PreviousOp != ID_OP2) {
		m_PreviousPnt = GetCursorPosition();
		RubberBandingStartAtEnable(m_PreviousPnt, Lines);
		m_PreviousOp = ModeLineHighlightOp(ID_OP2);
	}
	else {
		const OdGePoint3d pt = GetCursorPosition();

		if (m_PreviousPnt == pt) return;
		AeSysDoc* Document = GetDocument();

		EoGePoint4d ptView[] = {EoGePoint4d(m_PreviousPnt, 1.), EoGePoint4d(pt, 1.)};

		ModelViewTransformPoints(2, ptView);

		POSITION Position = Document->GetFirstTrappedGroupPosition();
		while (Position != 0) {
			EoDbGroup* Group = Document->GetNextTrappedGroup(Position);

			if (Group->SelectBy(EoGeLineSeg3d(ptView[0].Convert3d(), ptView[1].Convert3d()), this)) {
				Document->RemoveTrappedGroupAt(Document->FindTrappedGroup(Group));
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
			}
		}
		RubberBandingDisable();
		ModeLineUnhighlightOp(m_PreviousOp);
		UpdateStateInformation(TrapCount);
	}
}

void AeSysView::OnTraprModeField() {
	if (m_PreviousOp != ID_OP4) {
		m_PreviousPnt = GetCursorPosition();
		RubberBandingStartAtEnable(m_PreviousPnt, Rectangles);
		m_PreviousOp = ModeLineHighlightOp(ID_OP4);
	}
	else {
		const OdGePoint3d pt = GetCursorPosition();
		if (m_PreviousPnt == pt) return;

		AeSysDoc* Document = GetDocument();

		EoGePoint4d ptView[] = {EoGePoint4d(m_PreviousPnt, 1.), EoGePoint4d(pt, 1.)};

		ModelViewTransformPoints(2, ptView);

		const OdGePoint3d ptMin = EoGePoint4d::Min(ptView[0], ptView[1]).Convert3d();
		const OdGePoint3d ptMax = EoGePoint4d::Max(ptView[0], ptView[1]).Convert3d();

		POSITION Position = Document->GetFirstTrappedGroupPosition();
		while (Position != 0) {
			EoDbGroup* Group = Document->GetNextTrappedGroup(Position);

			if (Group->SelectBy(ptMin, ptMax, this)) {
				Document->RemoveTrappedGroupAt(Document->FindTrappedGroup(Group));
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
			}
		}
		RubberBandingDisable();
		ModeLineUnhighlightOp(m_PreviousOp);
		UpdateStateInformation(TrapCount);
	}
}
void AeSysView::OnTraprModeLast() {
	AeSysDoc* Document = GetDocument();

	if (!Document->IsTrapEmpty()) {
		EoDbGroup* Group = Document->RemoveLastTrappedGroup();
		Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
		UpdateStateInformation(TrapCount);
	}
}
void AeSysView::OnTraprModeEngage() noexcept {
	// TODO: Add your command handler code here
}
void AeSysView::OnTraprModeMenu() {
	CPoint CurrentPosition;
	::GetCursorPos(&CurrentPosition);
	HMENU TrapMenu = ::LoadMenu(theApp.GetInstance(), MAKEINTRESOURCE(IDR_TRAP));
	CMenu* SubMenu = CMenu::FromHandle(::GetSubMenu(TrapMenu, 0));
	SubMenu->TrackPopupMenuEx(0, CurrentPosition.x, CurrentPosition.y, AfxGetMainWnd(), 0);
	::DestroyMenu(TrapMenu);
}
void AeSysView::OnTraprModeModify() {
	if (!GetDocument()->IsTrapEmpty()) {
		EoDlgTrapModify Dialog(GetDocument());
		if (Dialog.DoModal() == IDOK) {
			GetDocument()->UpdateAllViews(nullptr);
		}
	}
	else {
		theApp.AddModeInformationToMessageList();
	}
}
void AeSysView::OnTraprModeEscape() {
	RubberBandingDisable();
	ModeLineUnhighlightOp(m_PreviousOp);
}
