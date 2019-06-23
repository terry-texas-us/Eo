#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDlgFixupOptions.h"
SelectionPair ReferenceSelection {nullptr, nullptr};
SelectionPair PreviousSelection {nullptr, nullptr};
SelectionPair CurrentSelection {nullptr, nullptr};
unsigned short PreviousFixupCommand = 0;

// <tas="FixupMode Axis Tolerance is not properly independent of the global Constraint influence angle"</tas>
void AeSysView::OnFixupModeOptions() {
	EoDlgFixupOptions Dialog;
	Dialog.m_AxisTolerance = m_AxisTolerance;
	Dialog.m_CornerSize = m_CornerSize;
	if (Dialog.DoModal() == IDOK) {
		m_CornerSize = EoMax(0.0, Dialog.m_CornerSize);
		m_AxisTolerance = EoMax(0.0, Dialog.m_AxisTolerance);
		SetAxisConstraintInfluenceAngle(m_AxisTolerance);
	}
}

void AeSysView::OnFixupModeReference() {
	CurrentSelection = {nullptr, nullptr};
	ReferenceSelection = SelectLineUsingPoint(GetCursorPosition());
	if (std::get<tGroup>(ReferenceSelection) == nullptr) { return; }
	if (PreviousFixupCommand == 0) {
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP1);
	} else if (PreviousFixupCommand == ID_OP1) {
	} else {
		OdGePoint3d IntersectionPoint;
		const auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(ReferenceSelection))->LineSeg()};
		const auto PreviousLine {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(PreviousSelection))};
		const auto PreviousLineSeg {PreviousLine->LineSeg()};
		if (!PreviousLineSeg.intersectWith(ReferenceLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_NO_INTERSECTION_WITH_REFERENCE);
			return;
		}
		if (PreviousFixupCommand == ID_OP2) {
			GenerateCorner(IntersectionPoint, PreviousSelection, ReferenceSelection, kTrimPreviousToIntersection);
		} else if (PreviousFixupCommand == ID_OP3) {
			GenerateCorner(IntersectionPoint, PreviousSelection, ReferenceSelection, kTrimPreviousToSize | kChamfer);
		} else if (PreviousFixupCommand == ID_OP4) {
			GenerateCorner(IntersectionPoint, PreviousSelection, ReferenceSelection, kTrimPreviousToSize | kFillet);
		}
		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeMend() {
	CurrentSelection = SelectLineUsingPoint(GetCursorPosition());
	if (std::get<tGroup>(CurrentSelection) == nullptr) {
		theApp.AddStringToMessageList(L"0 lines found");
		return;
	}
	const auto CurrentLine {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(CurrentSelection))};
	const auto CurrentLineSeg {CurrentLine->LineSeg()};
	OdGePoint3d IntersectionPoint;
	if (PreviousFixupCommand == 0) {
		PreviousSelection = CurrentSelection;
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP2);
	} else if (PreviousFixupCommand == ID_OP1) {
		const auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(ReferenceSelection))->LineSeg()};
		if (!ReferenceLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_NO_INTERSECTION_WITH_REFERENCE);
			return;
		}
		GenerateCorner(IntersectionPoint, ReferenceSelection, CurrentSelection, kTrimCurrentToIntersection);
		ModeLineUnhighlightOp(PreviousFixupCommand);
	} else {
		const auto PreviousLine {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(PreviousSelection))};
		const auto PreviousLineSeg {PreviousLine->LineSeg()};
		if (!PreviousLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_SELECTED_LINES_DO_NOT_INTERSECT);
			return;
		}
		if (PreviousFixupCommand == ID_OP2) {
			GenerateCorner(IntersectionPoint, PreviousSelection, CurrentSelection);
		} else if (PreviousFixupCommand == ID_OP3) {
			GenerateCorner(IntersectionPoint, PreviousSelection, CurrentSelection, kTrimPreviousToSize | kTrimCurrentToIntersection | kChamfer);
		} else if (PreviousFixupCommand == ID_OP4) {
			GenerateCorner(IntersectionPoint, PreviousSelection, CurrentSelection, kTrimPreviousToSize | kTrimCurrentToIntersection | kFillet);
		}
		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeChamfer() {
	CurrentSelection = SelectLineUsingPoint(GetCursorPosition());
	if (std::get<tGroup>(CurrentSelection) == nullptr) {
		theApp.AddStringToMessageList(L"0 lines found");
		return;
	}
	const auto CurrentLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(CurrentSelection))->LineSeg()};
	OdGePoint3d IntersectionPoint;
	if (PreviousFixupCommand == 0) {
		PreviousSelection = CurrentSelection;
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP3);
	} else if (PreviousFixupCommand == ID_OP1) {
		const auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(ReferenceSelection))->LineSeg()};
		if (!ReferenceLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_NO_INTERSECTION_WITH_REFERENCE);
			return;
		}
		GenerateCorner(IntersectionPoint, ReferenceSelection, CurrentSelection, kTrimCurrentToSize | kChamfer);
		ModeLineUnhighlightOp(PreviousFixupCommand);
	} else {
		const auto PreviousLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(PreviousSelection))->LineSeg()};
		if (!PreviousLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_SELECTED_LINES_DO_NOT_INTERSECT);
			return;
		}
		const auto CornerType {(PreviousFixupCommand == ID_OP2 ? kTrimPreviousToIntersection : kTrimPreviousToSize) | kTrimCurrentToSize | kChamfer};
		GenerateCorner(IntersectionPoint, PreviousSelection, CurrentSelection, CornerType);
		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeFillet() {
	CurrentSelection = SelectLineUsingPoint(GetCursorPosition());
	if (std::get<tGroup>(CurrentSelection) == nullptr) {
		theApp.AddStringToMessageList(L"0 lines found");
		return;
	}
	const auto CurrentLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(CurrentSelection))->LineSeg()};
	OdGePoint3d IntersectionPoint;
	if (PreviousFixupCommand == 0) {
		PreviousSelection = CurrentSelection;
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP4);
	} else if (PreviousFixupCommand == ID_OP1) {
		const auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(ReferenceSelection))->LineSeg()};
		if (!ReferenceLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_NO_INTERSECTION_WITH_REFERENCE);
			return;
		}
		GenerateCorner(IntersectionPoint, ReferenceSelection, CurrentSelection, kTrimCurrentToSize | kFillet);
		ModeLineUnhighlightOp(PreviousFixupCommand);
	} else {
		const auto PreviousLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(PreviousSelection))->LineSeg()};
		if (!PreviousLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_SELECTED_LINES_DO_NOT_INTERSECT);
			return;
		}
		const auto CornerType {(PreviousFixupCommand == ID_OP2 ? kTrimPreviousToIntersection : kTrimPreviousToSize) | kTrimCurrentToSize | kFillet};
		GenerateCorner(IntersectionPoint, PreviousSelection, CurrentSelection, CornerType);
		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeSquare() {
	auto Document {GetDocument()};
	auto CurrentPnt {GetCursorPosition()};
	CurrentSelection = SelectLineUsingPoint(CurrentPnt);
	if (std::get<tGroup>(CurrentSelection) == nullptr) { return; }
	auto CurrentLine {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(CurrentSelection))};
	const auto CurrentLineSeg {CurrentLine->LineSeg()};
	CurrentPnt = CurrentLineSeg.ProjPt(CurrentPnt);
	const auto CurrentLineSegLength {CurrentLineSeg.length()};
	Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, CurrentLine);
	const auto StartPoint {SnapPointToAxis(CurrentPnt, CurrentLineSeg.startPoint())};
	const auto EndPoint {ProjectToward(StartPoint, CurrentPnt, CurrentLineSegLength)};
	CurrentLine->SetStartPoint(SnapPointToGrid(StartPoint));
	CurrentLine->SetEndPoint(SnapPointToGrid(EndPoint));
	Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, CurrentLine);
}

void AeSysView::OnFixupModeParallel() {
	auto Document {GetDocument()};
	if (std::get<tGroup>(ReferenceSelection) == nullptr) {
		theApp.AddStringToMessageList(L"Reference line must be selected first.");
		return;
	}
	CurrentSelection = SelectLineUsingPoint(GetCursorPosition());
	if (std::get<tGroup>(CurrentSelection) == nullptr) { return; }
	auto CurrentLine {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(CurrentSelection))};
	auto CurrentLineSeg {CurrentLine->LineSeg()};
	const auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(ReferenceSelection))->LineSeg()};
	CurrentLineSeg.set(ReferenceLineSeg.ProjPt(CurrentLine->StartPoint()), ReferenceLineSeg.ProjPt(CurrentLine->EndPoint()));
	Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, CurrentLine);
	const auto StartPoint {CurrentLineSeg.startPoint()};
	const auto EndPoint {CurrentLineSeg.endPoint()};
	CurrentLine->SetStartPoint(ProjectToward(StartPoint, CurrentLine->StartPoint(), theApp.DimensionLength()));
	CurrentLine->SetEndPoint(ProjectToward(EndPoint, CurrentLine->EndPoint(), theApp.DimensionLength()));
	Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, CurrentLine);
}

void AeSysView::OnFixupModeReturn() {
}

void AeSysView::OnFixupModeEscape() {
	ReferenceSelection = {nullptr, nullptr};
	CurrentSelection = {nullptr, nullptr};
	PreviousSelection = {nullptr, nullptr};
	theApp.AddStringToMessageList(L"*cancel*");
	ModeLineUnhighlightOp(PreviousFixupCommand);
}

void AeSysView::GenerateCorner(const OdGePoint3d intersection, SelectionPair previousSelection, SelectionPair currentSelection, const int cornerType) {
	auto PreviousLine {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(previousSelection))};
	auto PreviousLineSeg {PreviousLine->LineSeg()};
	auto CurrentLine {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(currentSelection))};
	auto CurrentLineSeg {CurrentLine->LineSeg()};
	if ((intersection - PreviousLineSeg.startPoint()).length() < (intersection - PreviousLineSeg.endPoint()).length()) {
		PreviousLineSeg.SetStartPoint(PreviousLineSeg.endPoint());
	}
	PreviousLineSeg.SetEndPoint(intersection);
	if ((intersection - CurrentLineSeg.endPoint()).length() < (intersection - CurrentLineSeg.startPoint()).length()) {
		CurrentLineSeg.SetEndPoint(CurrentLineSeg.startPoint());
	}
	CurrentLineSeg.SetStartPoint(intersection);
	OdGePoint3d CenterPoint;
	if (FindCenterPointGivenRadiusAndTwoLineSegments(m_CornerSize, PreviousLineSeg, CurrentLineSeg, CenterPoint)) {
		auto Document {GetDocument()};
		PreviousLineSeg.SetEndPoint(PreviousLineSeg.ProjPt(CenterPoint));
		if (GETBIT(cornerType, kTrimPrevious)) {
			const auto StartPoint {PreviousLineSeg.startPoint()};
			const auto EndPoint {GETBIT(cornerType, kTrimPreviousToSize) ? PreviousLineSeg.endPoint() : intersection};
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, PreviousLine);
			PreviousLine->SetStartPoint(StartPoint);
			PreviousLine->SetEndPoint(EndPoint);
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, PreviousLine);
		}
		if (GETBIT(cornerType, kTrimCurrent)) {
			const auto StartPoint {GETBIT(cornerType, kTrimCurrentToSize) ? CurrentLineSeg.ProjPt(CenterPoint) : intersection};
			const auto EndPoint {CurrentLineSeg.endPoint()};
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, CurrentLine);
			CurrentLine->SetStartPoint(StartPoint);
			CurrentLine->SetEndPoint(EndPoint);
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, CurrentLine);
		}
		CurrentLineSeg.SetStartPoint(CurrentLineSeg.ProjPt(CenterPoint));
		if (!GETBIT(cornerType, kCorner)) {
			OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
			auto Group {new EoDbGroup};
			const auto StartPoint {PreviousLineSeg.endPoint()};
			const auto EndPoint {CurrentLineSeg.startPoint()};
			if (GETBIT(cornerType, kChamfer)) {
				auto Line {EoDbLine::Create(BlockTableRecord, StartPoint, EndPoint)};
				Line->setColorIndex(static_cast<unsigned short>(PreviousLine->ColorIndex()));
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(PreviousLine->LinetypeIndex()));
				Group->AddTail(EoDbLine::Create(Line));
			} else if (GETBIT(cornerType, kFillet)) {
				auto PlaneNormal {(intersection - StartPoint).crossProduct(EndPoint - StartPoint)};
				PlaneNormal.normalize();
				double SweepAngle;
				pFndSwpAngGivPlnAnd3Lns(PlaneNormal, StartPoint, intersection, EndPoint, CenterPoint, SweepAngle);
				const auto MajorAxis {StartPoint - CenterPoint};
				auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
				Ellipse->set(CenterPoint, PlaneNormal, MajorAxis, 1.0, 0.0, SweepAngle);
				Ellipse->setColorIndex(static_cast<unsigned short>(PreviousLine->ColorIndex()));
				Ellipse->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(PreviousLine->LinetypeIndex()));
				Group->AddTail(EoDbEllipse::Create(Ellipse));
			}
			Document->AddWorkLayerGroup(Group);
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
		}
	}
}

bool AeSysView::FindCenterPointGivenRadiusAndTwoLineSegments(const double radius, OdGeLineSeg3d firstLineSeg, OdGeLineSeg3d secondLineSeg, OdGePoint3d& centerPoint) {
	const auto FirstLineStartPoint = firstLineSeg.startPoint();
	auto FirstLineEndPoint = firstLineSeg.endPoint();
	const auto FirstLineVector {FirstLineEndPoint - FirstLineStartPoint};
	if (FirstLineVector.isZeroLength()) { return false; }
	const auto FirstLineVectorLength {FirstLineVector.length()};
	auto SecondLineStartPoint = secondLineSeg.startPoint();
	auto SecondLineEndPoint = secondLineSeg.endPoint();
	auto SecondLineVector {SecondLineEndPoint - SecondLineStartPoint};
	if (SecondLineVector.isZeroLength()) { return false; }
	const auto SecondLineVectorLength {SecondLineVector.length()};
	auto Normal {FirstLineVector.crossProduct(SecondLineVector)};
	if (Normal.isZeroLength()) { return false; }
	Normal.normalize();
	if (fabs(Normal.dotProduct(SecondLineStartPoint - FirstLineStartPoint)) > DBL_EPSILON) { // Four points are not coplanar
		return false;
	}
	EoGeMatrix3d WorldToPlaneTransform;
	WorldToPlaneTransform.setToWorldToPlane(OdGePlane(FirstLineStartPoint, Normal));
	FirstLineEndPoint.transformBy(WorldToPlaneTransform);
	SecondLineStartPoint.transformBy(WorldToPlaneTransform);
	SecondLineEndPoint.transformBy(WorldToPlaneTransform);
	const auto FirstLineA {-FirstLineEndPoint.y / FirstLineVectorLength};
	const auto FirstLineB {FirstLineEndPoint.x / FirstLineVectorLength};
	SecondLineVector.x = SecondLineEndPoint.x - SecondLineStartPoint.x;
	SecondLineVector.y = SecondLineEndPoint.y - SecondLineStartPoint.y;
	const auto SecondLineA {-SecondLineVector.y / SecondLineVectorLength};
	const auto SecondLineB {SecondLineVector.x / SecondLineVectorLength};
	const auto Determinant {SecondLineA * FirstLineB - FirstLineA * SecondLineB};
	const auto SignedRadius {FirstLineEndPoint.x * SecondLineEndPoint.y - SecondLineEndPoint.x * FirstLineEndPoint.y >= 0. ? -fabs(radius) : fabs(radius)};
	const auto dC1RAB1 {SignedRadius};
	const auto dC2RAB2 {(SecondLineStartPoint.x * SecondLineEndPoint.y - SecondLineEndPoint.x * SecondLineStartPoint.y) / SecondLineVectorLength + SignedRadius};
	centerPoint.x = (SecondLineB * dC1RAB1 - FirstLineB * dC2RAB2) / Determinant;
	centerPoint.y = (FirstLineA * dC2RAB2 - SecondLineA * dC1RAB1) / Determinant;
	centerPoint.z = 0.0;
	WorldToPlaneTransform.invert();
	centerPoint.transformBy(WorldToPlaneTransform);
	return true;
}
