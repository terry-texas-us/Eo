#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDlgFixupOptions.h"
SelectionPair g_ReferenceSelection {nullptr, nullptr};
SelectionPair g_PreviousSelection {nullptr, nullptr};
SelectionPair g_CurrentSelection {nullptr, nullptr};
unsigned short g_PreviousFixupCommand = 0;

// <tas="FixupMode Axis Tolerance is not properly independent of the global Constraint influence angle"</tas>
void AeSysView::OnFixupModeOptions() {
	EoDlgFixupOptions Dialog;
	Dialog.axisTolerance = m_AxisTolerance;
	Dialog.cornerSize = m_CornerSize;
	if (Dialog.DoModal() == IDOK) {
		m_CornerSize = EoMax(0.0, Dialog.cornerSize);
		m_AxisTolerance = EoMax(0.0, Dialog.axisTolerance);
		SetAxisConstraintInfluenceAngle(m_AxisTolerance);
	}
}

void AeSysView::OnFixupModeReference() {
	g_CurrentSelection = {nullptr, nullptr};
	g_ReferenceSelection = SelectLineUsingPoint(GetCursorPosition());
	if (std::get<tGroup>(g_ReferenceSelection) == nullptr) {
		return;
	}
	if (g_PreviousFixupCommand == 0) {
		g_PreviousFixupCommand = ModeLineHighlightOp(ID_OP1);
	} else if (g_PreviousFixupCommand == ID_OP1) { } else {
		OdGePoint3d IntersectionPoint;
		const auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(g_ReferenceSelection))->LineSeg()};
		const auto PreviousLine {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(g_PreviousSelection))};
		const auto PreviousLineSeg {PreviousLine->LineSeg()};
		if (!PreviousLineSeg.intersectWith(ReferenceLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_NO_INTERSECTION_WITH_REFERENCE);
			return;
		}
		if (g_PreviousFixupCommand == ID_OP2) {
			GenerateCorner(IntersectionPoint, g_PreviousSelection, g_ReferenceSelection, kTrimPreviousToIntersection);
		} else if (g_PreviousFixupCommand == ID_OP3) {
			GenerateCorner(IntersectionPoint, g_PreviousSelection, g_ReferenceSelection, kTrimPreviousToSize | kChamfer);
		} else if (g_PreviousFixupCommand == ID_OP4) {
			GenerateCorner(IntersectionPoint, g_PreviousSelection, g_ReferenceSelection, kTrimPreviousToSize | kFillet);
		}
		ModeLineUnhighlightOp(g_PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeMend() {
	g_CurrentSelection = SelectLineUsingPoint(GetCursorPosition());
	if (std::get<tGroup>(g_CurrentSelection) == nullptr) {
		AeSys::AddStringToMessageList(L"0 lines found");
		return;
	}
	const auto CurrentLine {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(g_CurrentSelection))};
	const auto CurrentLineSeg {CurrentLine->LineSeg()};
	OdGePoint3d IntersectionPoint;
	if (g_PreviousFixupCommand == 0) {
		g_PreviousSelection = g_CurrentSelection;
		g_PreviousFixupCommand = ModeLineHighlightOp(ID_OP2);
	} else if (g_PreviousFixupCommand == ID_OP1) {
		const auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(g_ReferenceSelection))->LineSeg()};
		if (!ReferenceLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_NO_INTERSECTION_WITH_REFERENCE);
			return;
		}
		GenerateCorner(IntersectionPoint, g_ReferenceSelection, g_CurrentSelection, kTrimCurrentToIntersection);
		ModeLineUnhighlightOp(g_PreviousFixupCommand);
	} else {
		const auto PreviousLine {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(g_PreviousSelection))};
		const auto PreviousLineSeg {PreviousLine->LineSeg()};
		if (!PreviousLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_SELECTED_LINES_DO_NOT_INTERSECT);
			return;
		}
		if (g_PreviousFixupCommand == ID_OP2) {
			GenerateCorner(IntersectionPoint, g_PreviousSelection, g_CurrentSelection);
		} else if (g_PreviousFixupCommand == ID_OP3) {
			GenerateCorner(IntersectionPoint, g_PreviousSelection, g_CurrentSelection, kTrimPreviousToSize | kTrimCurrentToIntersection | kChamfer);
		} else if (g_PreviousFixupCommand == ID_OP4) {
			GenerateCorner(IntersectionPoint, g_PreviousSelection, g_CurrentSelection, kTrimPreviousToSize | kTrimCurrentToIntersection | kFillet);
		}
		ModeLineUnhighlightOp(g_PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeChamfer() {
	g_CurrentSelection = SelectLineUsingPoint(GetCursorPosition());
	if (std::get<tGroup>(g_CurrentSelection) == nullptr) {
		AeSys::AddStringToMessageList(L"0 lines found");
		return;
	}
	const auto CurrentLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(g_CurrentSelection))->LineSeg()};
	OdGePoint3d IntersectionPoint;
	if (g_PreviousFixupCommand == 0) {
		g_PreviousSelection = g_CurrentSelection;
		g_PreviousFixupCommand = ModeLineHighlightOp(ID_OP3);
	} else if (g_PreviousFixupCommand == ID_OP1) {
		const auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(g_ReferenceSelection))->LineSeg()};
		if (!ReferenceLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_NO_INTERSECTION_WITH_REFERENCE);
			return;
		}
		GenerateCorner(IntersectionPoint, g_ReferenceSelection, g_CurrentSelection, kTrimCurrentToSize | kChamfer);
		ModeLineUnhighlightOp(g_PreviousFixupCommand);
	} else {
		const auto PreviousLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(g_PreviousSelection))->LineSeg()};
		if (!PreviousLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_SELECTED_LINES_DO_NOT_INTERSECT);
			return;
		}
		const auto CornerType {(g_PreviousFixupCommand == ID_OP2 ? kTrimPreviousToIntersection : kTrimPreviousToSize) | kTrimCurrentToSize | kChamfer};
		GenerateCorner(IntersectionPoint, g_PreviousSelection, g_CurrentSelection, CornerType);
		ModeLineUnhighlightOp(g_PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeFillet() {
	g_CurrentSelection = SelectLineUsingPoint(GetCursorPosition());
	if (std::get<tGroup>(g_CurrentSelection) == nullptr) {
		AeSys::AddStringToMessageList(L"0 lines found");
		return;
	}
	const auto CurrentLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(g_CurrentSelection))->LineSeg()};
	OdGePoint3d IntersectionPoint;
	if (g_PreviousFixupCommand == 0) {
		g_PreviousSelection = g_CurrentSelection;
		g_PreviousFixupCommand = ModeLineHighlightOp(ID_OP4);
	} else if (g_PreviousFixupCommand == ID_OP1) {
		const auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(g_ReferenceSelection))->LineSeg()};
		if (!ReferenceLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_NO_INTERSECTION_WITH_REFERENCE);
			return;
		}
		GenerateCorner(IntersectionPoint, g_ReferenceSelection, g_CurrentSelection, kTrimCurrentToSize | kFillet);
		ModeLineUnhighlightOp(g_PreviousFixupCommand);
	} else {
		const auto PreviousLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(g_PreviousSelection))->LineSeg()};
		if (!PreviousLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_SELECTED_LINES_DO_NOT_INTERSECT);
			return;
		}
		const auto CornerType {(g_PreviousFixupCommand == ID_OP2 ? kTrimPreviousToIntersection : kTrimPreviousToSize) | kTrimCurrentToSize | kFillet};
		GenerateCorner(IntersectionPoint, g_PreviousSelection, g_CurrentSelection, CornerType);
		ModeLineUnhighlightOp(g_PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeSquare() {
	auto Document {GetDocument()};
	auto CurrentPnt {GetCursorPosition()};
	g_CurrentSelection = SelectLineUsingPoint(CurrentPnt);
	if (std::get<tGroup>(g_CurrentSelection) == nullptr) {
		return;
	}
	auto CurrentLine {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(g_CurrentSelection))};
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
	if (std::get<tGroup>(g_ReferenceSelection) == nullptr) {
		AeSys::AddStringToMessageList(L"Reference line must be selected first.");
		return;
	}
	g_CurrentSelection = SelectLineUsingPoint(GetCursorPosition());
	if (std::get<tGroup>(g_CurrentSelection) == nullptr) {
		return;
	}
	auto CurrentLine {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(g_CurrentSelection))};
	auto CurrentLineSeg {CurrentLine->LineSeg()};
	const auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(std::get<tPrimitive>(g_ReferenceSelection))->LineSeg()};
	CurrentLineSeg.set(ReferenceLineSeg.ProjPt(CurrentLine->StartPoint()), ReferenceLineSeg.ProjPt(CurrentLine->EndPoint()));
	Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, CurrentLine);
	const auto StartPoint {CurrentLineSeg.startPoint()};
	const auto EndPoint {CurrentLineSeg.endPoint()};
	CurrentLine->SetStartPoint(ProjectToward(StartPoint, CurrentLine->StartPoint(), theApp.DimensionLength()));
	CurrentLine->SetEndPoint(ProjectToward(EndPoint, CurrentLine->EndPoint(), theApp.DimensionLength()));
	Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, CurrentLine);
}

void AeSysView::OnFixupModeReturn() {}

void AeSysView::OnFixupModeEscape() {
	g_ReferenceSelection = {nullptr, nullptr};
	g_CurrentSelection = {nullptr, nullptr};
	g_PreviousSelection = {nullptr, nullptr};
	AeSys::AddStringToMessageList(L"*cancel*");
	ModeLineUnhighlightOp(g_PreviousFixupCommand);
}

void AeSysView::GenerateCorner(const OdGePoint3d intersection, SelectionPair previousSelection, SelectionPair currentSelection, const unsigned cornerType) const {
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
		if ((cornerType & kTrimPrevious) != 0) {
			const auto StartPoint {PreviousLineSeg.startPoint()};
			const auto EndPoint {(cornerType & kTrimPreviousToSize) != 0 ? PreviousLineSeg.endPoint() : intersection};
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, PreviousLine);
			PreviousLine->SetStartPoint(StartPoint);
			PreviousLine->SetEndPoint(EndPoint);
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, PreviousLine);
		}
		if ((cornerType & kTrimCurrent) != 0) {
			const auto StartPoint {(cornerType & kTrimCurrentToSize) != 0 ? CurrentLineSeg.ProjPt(CenterPoint) : intersection};
			const auto EndPoint {CurrentLineSeg.endPoint()};
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, CurrentLine);
			CurrentLine->SetStartPoint(StartPoint);
			CurrentLine->SetEndPoint(EndPoint);
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, CurrentLine);
		}
		CurrentLineSeg.SetStartPoint(CurrentLineSeg.ProjPt(CenterPoint));
		if (!((cornerType & kCorner) != 0)) {
			OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
			auto Group {new EoDbGroup};
			const auto StartPoint {PreviousLineSeg.endPoint()};
			const auto EndPoint {CurrentLineSeg.startPoint()};
			if ((cornerType & kChamfer) != 0) {
				auto Line {EoDbLine::Create(BlockTableRecord, StartPoint, EndPoint)};
				Line->setColorIndex(static_cast<unsigned short>(PreviousLine->ColorIndex()));
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(PreviousLine->LinetypeIndex()));
				Group->AddTail(EoDbLine::Create(Line));
			} else if ((cornerType & kFillet) != 0) {
				auto PlaneNormal {(intersection - StartPoint).crossProduct(EndPoint - StartPoint)};
				PlaneNormal.normalize();
				double SweepAngle;
				FindSweepAngleGivenPlaneAnd3Lines(PlaneNormal, StartPoint, intersection, EndPoint, CenterPoint, SweepAngle);
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

bool AeSysView::FindCenterPointGivenRadiusAndTwoLineSegments(const double radius, const OdGeLineSeg3d firstLineSeg, const OdGeLineSeg3d secondLineSeg, OdGePoint3d& centerPoint) const {
	const auto FirstLineStartPoint = firstLineSeg.startPoint();
	auto FirstLineEndPoint = firstLineSeg.endPoint();
	const auto FirstLineVector {FirstLineEndPoint - FirstLineStartPoint};
	if (FirstLineVector.isZeroLength()) {
		return false;
	}
	const auto FirstLineVectorLength {FirstLineVector.length()};
	auto SecondLineStartPoint = secondLineSeg.startPoint();
	auto SecondLineEndPoint = secondLineSeg.endPoint();
	auto SecondLineVector {SecondLineEndPoint - SecondLineStartPoint};
	if (SecondLineVector.isZeroLength()) {
		return false;
	}
	const auto SecondLineVectorLength {SecondLineVector.length()};
	auto Normal {FirstLineVector.crossProduct(SecondLineVector)};
	if (Normal.isZeroLength()) {
		return false;
	}
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
	const auto SignedRadius {FirstLineEndPoint.x * SecondLineEndPoint.y - SecondLineEndPoint.x * FirstLineEndPoint.y >= 0.0 ? -fabs(radius) : fabs(radius)};
	const auto dC1RAB1 {SignedRadius};
	const auto dC2RAB2 {(SecondLineStartPoint.x * SecondLineEndPoint.y - SecondLineEndPoint.x * SecondLineStartPoint.y) / SecondLineVectorLength + SignedRadius};
	centerPoint.x = (SecondLineB * dC1RAB1 - FirstLineB * dC2RAB2) / Determinant;
	centerPoint.y = (FirstLineA * dC2RAB2 - SecondLineA * dC1RAB1) / Determinant;
	centerPoint.z = 0.0;
	WorldToPlaneTransform.invert();
	centerPoint.transformBy(WorldToPlaneTransform);
	return true;
}
