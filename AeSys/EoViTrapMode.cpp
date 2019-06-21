#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDlgTrapModify.h"
#include "EoDbHatch.h"
#include "EoDbPolyline.h"

void AeSysView::OnTrapModeRemoveAdd() {
	theApp.OnTrapCommandsAddGroups();
}

void AeSysView::OnTrapModePoint() {
	EoGePoint4d ptView(GetCursorPosition(), 1.0);
	ModelViewTransformPoint(ptView);
	EoDbHatch::SetEdgeToEvaluate(0);
	EoDbPolyline::SetEdgeToEvaluate(0);
	auto Position {GetFirstVisibleGroupPosition()};
	while (Position != nullptr) {
		const auto Group {GetNextVisibleGroup(Position)};
		if (GetDocument()->FindTrappedGroup(Group) != nullptr) { continue; }
		if (Group->IsOn(ptView, this)) { GetDocument()->AddGroupToTrap(Group); }
	}
	UpdateStateInformation(TrapCount);
}

void AeSysView::OnTrapModeStitch() {
	if (m_PreviousOp != ID_OP2) {
		m_PreviousPnt = GetCursorPosition();
		RubberBandingStartAtEnable(m_PreviousPnt, Lines);
		m_PreviousOp = ModeLineHighlightOp(ID_OP2);
	} else {
		const auto CurrentPnt {GetCursorPosition()};
		if (m_PreviousPnt == CurrentPnt) { return; }
		EoGePoint4d ptView[] = {EoGePoint4d(m_PreviousPnt, 1.0), EoGePoint4d(CurrentPnt, 1.0)};
		ModelViewTransformPoints(2, ptView);
		auto Position {GetFirstVisibleGroupPosition()};
		while (Position != nullptr) {
			const auto Group {GetNextVisibleGroup(Position)};
			if (GetDocument()->FindTrappedGroup(Group) != nullptr) { continue; }
			if (Group->SelectUsingLineSeg(EoGeLineSeg3d(ptView[0].Convert3d(), ptView[1].Convert3d()), this)) { GetDocument()->AddGroupToTrap(Group); }
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
	} else {
		const auto CurrentPnt {GetCursorPosition()};
		if (m_PreviousPnt == CurrentPnt) { return; }
		EoGePoint4d ptView[] = {EoGePoint4d(m_PreviousPnt, 1.0), EoGePoint4d(CurrentPnt, 1.0)};
		ModelViewTransformPoints(2, ptView);
		const auto LowerLeftCorner {EoGePoint4d::Min(ptView[0], ptView[1]).Convert3d()};
		const auto UpperRightCorner {EoGePoint4d::Max(ptView[0], ptView[1]).Convert3d()};
		auto Position {GetFirstVisibleGroupPosition()};
		while (Position != nullptr) {
			const auto Group {GetNextVisibleGroup(Position)};
			if (GetDocument()->FindTrappedGroup(Group) != nullptr) { continue; }
			if (Group->SelectUsingRectangle(LowerLeftCorner, UpperRightCorner, this)) { GetDocument()->AddGroupToTrap(Group); }
		}
		RubberBandingDisable();
		ModeLineUnhighlightOp(m_PreviousOp);
		UpdateStateInformation(TrapCount);
	}
}

void AeSysView::OnTrapModeLast() {
	auto Document {GetDocument()};
	auto Position {Document->GetLastWorkLayerGroupPosition()};
	while (Position != nullptr) {
		const auto Group {Document->GetPreviousWorkLayerGroup(Position)};
		if (!Document->FindTrappedGroup(Group)) {
			Document->AddGroupToTrap(Group);
			UpdateStateInformation(TrapCount);
			break;
		}
	}
}

void AeSysView::OnTrapModeEngage() {
	if (GroupIsEngaged()) {
		auto Document {GetDocument()};
		auto Position {Document->FindWorkLayerGroup(EngagedGroup())};
		const auto Group {Document->GetNextWorkLayerGroup(Position)};
		if (Document->FindTrappedGroup(Group) == nullptr) {
			Document->AddGroupToTrap(Group);
			UpdateStateInformation(TrapCount);
		}
	} else {
		theApp.AddModeInformationToMessageList();
	}
}

void AeSysView::OnTrapModeMenu() {
	CPoint CurrentPosition;
	GetCursorPos(&CurrentPosition);
	const auto TrapMenu {LoadMenuW(theApp.GetInstance(), MAKEINTRESOURCEW(IDR_TRAP))};
	auto SubMenu {CMenu::FromHandle(GetSubMenu(TrapMenu, 0))};
	SubMenu->TrackPopupMenuEx(0, CurrentPosition.x, CurrentPosition.y, AfxGetMainWnd(), nullptr);
	DestroyMenu(TrapMenu);
}

void AeSysView::OnTrapModeModify() {

	if (!GetDocument()->IsTrapEmpty()) {
		EoDlgTrapModify Dialog(GetDocument());
		if (Dialog.DoModal() == IDOK) {
			GetDocument()->UpdateAllViews(nullptr);
		}
	} else {
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
	auto Document {GetDocument()};
	EoGePoint4d ptView(GetCursorPosition(), 1.0);
	ModelViewTransformPoint(ptView);
	EoDbHatch::SetEdgeToEvaluate(0);
	EoDbPolyline::SetEdgeToEvaluate(0);
	auto Position {Document->GetFirstTrappedGroupPosition()};
	while (Position != nullptr) {
		const auto Group {Document->GetNextTrappedGroup(Position)};
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
	} else {
		const auto CurrentPnt {GetCursorPosition()};
		if (m_PreviousPnt == CurrentPnt) { return; }
		auto Document {GetDocument()};
		EoGePoint4d ptView[] = {EoGePoint4d(m_PreviousPnt, 1.0), EoGePoint4d(CurrentPnt, 1.0)};
		ModelViewTransformPoints(2, ptView);
		auto Position {Document->GetFirstTrappedGroupPosition()};
		while (Position != nullptr) {
			const auto Group {Document->GetNextTrappedGroup(Position)};
			if (Group->SelectUsingLineSeg(EoGeLineSeg3d(ptView[0].Convert3d(), ptView[1].Convert3d()), this)) {
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
	} else {
		const auto CurrentPnt {GetCursorPosition()};
		if (m_PreviousPnt == CurrentPnt) { return; }
		auto Document {GetDocument()};
		EoGePoint4d ptView[] = {EoGePoint4d(m_PreviousPnt, 1.0), EoGePoint4d(CurrentPnt, 1.0)};
		ModelViewTransformPoints(2, ptView);
		const auto LowerLeftCorner {EoGePoint4d::Min(ptView[0], ptView[1]).Convert3d()};
		const auto UpperRightCorner {EoGePoint4d::Max(ptView[0], ptView[1]).Convert3d()};
		auto Position {Document->GetFirstTrappedGroupPosition()};
		while (Position != nullptr) {
			const auto Group {Document->GetNextTrappedGroup(Position)};
			if (Group->SelectUsingRectangle(LowerLeftCorner, UpperRightCorner, this)) {
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

	if (!GetDocument()->IsTrapEmpty()) {
		const auto Group {GetDocument()->RemoveLastTrappedGroup()};
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
		UpdateStateInformation(TrapCount);
	}
}

void AeSysView::OnTraprModeEngage() noexcept {
	// TODO: Add your command handler code here
}

void AeSysView::OnTraprModeMenu() {
	CPoint CurrentPosition;
	GetCursorPos(&CurrentPosition);
	const auto TrapMenu {LoadMenuW(theApp.GetInstance(), MAKEINTRESOURCEW(IDR_TRAP))};
	auto SubMenu {CMenu::FromHandle(GetSubMenu(TrapMenu, 0))};
	SubMenu->TrackPopupMenuEx(0, CurrentPosition.x, CurrentPosition.y, AfxGetMainWnd(), nullptr);
	DestroyMenu(TrapMenu);
}

void AeSysView::OnTraprModeModify() {

	if (!GetDocument()->IsTrapEmpty()) {
		EoDlgTrapModify Dialog(GetDocument());
		if (Dialog.DoModal() == IDOK) {
			GetDocument()->UpdateAllViews(nullptr);
		}
	} else {
		theApp.AddModeInformationToMessageList();
	}
}

void AeSysView::OnTraprModeEscape() {
	RubberBandingDisable();
	ModeLineUnhighlightOp(m_PreviousOp);
}
