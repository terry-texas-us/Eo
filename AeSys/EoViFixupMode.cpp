#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDlgFixupOptions.h"

SelectionPair ReferenceSelection {nullptr, nullptr};
SelectionPair PreviousSelection {nullptr, nullptr};
SelectionPair CurrentSelection {nullptr, nullptr};

OdUInt16 PreviousFixupCommand = 0;

// <tas="FixupMode Axis Tolerance is not properly independent of the global Constraint influence angle"</tas>
void AeSysView::OnFixupModeOptions() {
	EoDlgFixupOptions Dialog;
	Dialog.m_AxisTolerance = m_AxisTolerance;
	Dialog.m_CornerSize = m_CornerSize;
	if (Dialog.DoModal() == IDOK) {
		m_CornerSize = EoMax(0., Dialog.m_CornerSize);
		m_AxisTolerance = EoMax(0., Dialog.m_AxisTolerance);
		SetAxisConstraintInfluenceAngle(m_AxisTolerance);
	}
}

void AeSysView::OnFixupModeReference() {
	auto Document {GetDocument()};

	CurrentSelection = {nullptr, nullptr};
	
	ReferenceSelection = SelectLineUsingPoint(GetCursorPosition());

	if (get<tGroup>(ReferenceSelection) == nullptr) { return; }

	if (PreviousFixupCommand == 0) {
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP1);
	} else if (PreviousFixupCommand == ID_OP1) {
		;
	} else {
		OdGePoint3d IntersectionPoint;
		auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(get<tPrimitive>(ReferenceSelection))->LineSeg()};
		auto PreviousLine {dynamic_cast<EoDbLine*>(get<tPrimitive>(PreviousSelection))};
		auto PreviousLineSeg {PreviousLine->LineSeg()};

		if (!PreviousLineSeg.intersectWith(ReferenceLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_NO_INTERSECTION_WITH_REFERENCE);
			return;
		}
		if (PreviousFixupCommand == ID_OP2) {
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, PreviousLine);
			
			if ((IntersectionPoint - PreviousLineSeg.startPoint()).length() < (IntersectionPoint - PreviousLineSeg.endPoint()).length()) {
				PreviousLine->SetStartPoint2(IntersectionPoint);
			} else {
				PreviousLine->SetEndPoint2(IntersectionPoint);
			}
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, PreviousLine);
		} else if (PreviousFixupCommand == ID_OP3) {
			GenerateCorner(IntersectionPoint, PreviousSelection, ReferenceSelection, 0x104);
		} else if (PreviousFixupCommand == ID_OP4) {
			GenerateCorner(IntersectionPoint, PreviousSelection, ReferenceSelection, 0x204);
		}
		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeMend() {
	auto Document {GetDocument()};

	CurrentSelection = SelectLineUsingPoint(GetCursorPosition());
	
	if (get<tGroup>(CurrentSelection) == nullptr) {
		theApp.AddStringToMessageList(L"0 lines found");
		return;
	}
	auto CurrentLine {dynamic_cast<EoDbLine*>(get<tPrimitive>(CurrentSelection))};
	auto CurrentLineSeg {CurrentLine->LineSeg()};
	
	OdGePoint3d IntersectionPoint;

	if (PreviousFixupCommand == 0) {
		PreviousSelection = CurrentSelection;
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP2);
	} else if (PreviousFixupCommand == ID_OP1) {
		auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(get<tPrimitive>(ReferenceSelection))->LineSeg()};
		if (!ReferenceLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_NO_INTERSECTION_WITH_REFERENCE);
			return;
		}
		Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, CurrentLine);

		if ((IntersectionPoint - CurrentLine->StartPoint()).length() < (IntersectionPoint - CurrentLine->EndPoint()).length()) {
			CurrentLine->SetStartPoint2(IntersectionPoint);
		} else {
			CurrentLine->SetEndPoint2(IntersectionPoint);
		}
		Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, CurrentLine);
	} else {
		auto PreviousLine {dynamic_cast<EoDbLine*>(get<tPrimitive>(PreviousSelection))};
		auto PreviousLineSeg {PreviousLine->LineSeg()};

		if (!PreviousLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_SELECTED_LINES_DO_NOT_INTERSECT);
			return;
		}
		if (PreviousFixupCommand == ID_OP2) {
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, PreviousLine);
			
			if ((IntersectionPoint - PreviousLineSeg.startPoint()).length() < (IntersectionPoint - PreviousLineSeg.endPoint()).length()) {
				PreviousLine->SetStartPoint2(IntersectionPoint);
			} else {
				PreviousLine->SetEndPoint2(IntersectionPoint);
			}
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, PreviousLine);
		} else if (PreviousFixupCommand == ID_OP3) {
			GenerateCorner(IntersectionPoint, PreviousSelection, CurrentSelection, 0x10c);
		} else if (PreviousFixupCommand == ID_OP4) {
			GenerateCorner(IntersectionPoint, PreviousSelection, CurrentSelection, 0x20c);
		}
		Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, CurrentLine);
		
		if ((IntersectionPoint - CurrentLineSeg.startPoint()).length() < (IntersectionPoint - CurrentLineSeg.endPoint()).length()) {
			CurrentLine->SetStartPoint2(IntersectionPoint);
		} else {
			CurrentLine->SetEndPoint2(IntersectionPoint);
		}
		Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, CurrentLine);
		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeChamfer() {
	CurrentSelection = SelectLineUsingPoint(GetCursorPosition());

	if (get<tGroup>(CurrentSelection) == nullptr) {
		theApp.AddStringToMessageList(L"0 lines found");
		return;
	}
	auto CurrentLineSeg {dynamic_cast<EoDbLine*>(get<tPrimitive>(CurrentSelection))->LineSeg()};

	OdGePoint3d IntersectionPoint;

	if (PreviousFixupCommand == 0) {
		PreviousSelection = CurrentSelection;
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP3);
	} else if (PreviousFixupCommand == ID_OP1) {
		auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(get<tPrimitive>(ReferenceSelection))->LineSeg()};

		if (!ReferenceLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_NO_INTERSECTION_WITH_REFERENCE);
			return;
		}
		GenerateCorner(IntersectionPoint, ReferenceSelection, CurrentSelection, 0x108);

		ModeLineUnhighlightOp(PreviousFixupCommand);
	} else {
		auto PreviousLineSeg {dynamic_cast<EoDbLine*>(get<tPrimitive>(PreviousSelection))->LineSeg()};

		if (!PreviousLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_SELECTED_LINES_DO_NOT_INTERSECT);
			return;
		}
		GenerateCorner(IntersectionPoint, PreviousSelection, CurrentSelection, 0x10c);

		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeFillet() {
	CurrentSelection = SelectLineUsingPoint(GetCursorPosition());
	
	if (get<tGroup>(CurrentSelection) == nullptr) {
		theApp.AddStringToMessageList(L"0 lines found");
		return;
	}
	auto CurrentLineSeg {dynamic_cast<EoDbLine*>(get<tPrimitive>(CurrentSelection))->LineSeg()};

	OdGePoint3d IntersectionPoint;

	if (PreviousFixupCommand == 0) {
		PreviousSelection = CurrentSelection;
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP4);
	} else if (PreviousFixupCommand == ID_OP1) {
		auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(get<tPrimitive>(ReferenceSelection))->LineSeg()};

		if (!ReferenceLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_NO_INTERSECTION_WITH_REFERENCE);
			return;
		}
		GenerateCorner(IntersectionPoint, ReferenceSelection, CurrentSelection, 0x208);

		ModeLineUnhighlightOp(PreviousFixupCommand);
	} else {
		auto PreviousLineSeg {dynamic_cast<EoDbLine*>(get<tPrimitive>(PreviousSelection))->LineSeg()};

		if (!PreviousLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(IDS_SELECTED_LINES_DO_NOT_INTERSECT);
			return;
		}
		GenerateCorner(IntersectionPoint, PreviousSelection, CurrentSelection, 0x20c);

		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeSquare() {
	auto Document {GetDocument()};

	auto CurrentPnt {GetCursorPosition()};

	CurrentSelection = SelectLineUsingPoint(CurrentPnt);

	if (get<tGroup>(CurrentSelection) == nullptr) { return; }

	auto CurrentLine {dynamic_cast<EoDbLine*>(get<tPrimitive>(CurrentSelection))};
	auto CurrentLineSeg {CurrentLine->LineSeg()};

	CurrentPnt = CurrentLineSeg.ProjPt(CurrentPnt);

	const double CurrentLineSegLength = CurrentLineSeg.length();
	Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, CurrentLine);
	
	const auto StartPoint {SnapPointToAxis(CurrentPnt, CurrentLineSeg.startPoint())};
	const auto EndPoint {ProjectToward(StartPoint, CurrentPnt, CurrentLineSegLength)};

	CurrentLine->SetStartPoint2(SnapPointToGrid(StartPoint));
	CurrentLine->SetEndPoint2(SnapPointToGrid(EndPoint));

	Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, CurrentLine);
}

void AeSysView::OnFixupModeParallel() {
	auto Document {GetDocument()};

	if (get<tGroup>(ReferenceSelection) == nullptr) {
		theApp.AddStringToMessageList(L"Reference line must be selected first.");
		return;
	}
	CurrentSelection = SelectLineUsingPoint(GetCursorPosition());

	if (get<tGroup>(CurrentSelection) == nullptr) { return; }

	auto CurrentLine {dynamic_cast<EoDbLine*>(get<tPrimitive>(CurrentSelection))};
	auto CurrentLineSeg {CurrentLine->LineSeg()};

	auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(get<tPrimitive>(ReferenceSelection))->LineSeg()};

	CurrentLineSeg.set(ReferenceLineSeg.ProjPt(CurrentLine->StartPoint()), ReferenceLineSeg.ProjPt(CurrentLine->EndPoint()));
	Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, CurrentLine);

	const auto StartPoint {CurrentLineSeg.startPoint()};
	const auto EndPoint {CurrentLineSeg.endPoint()};

	CurrentLine->SetStartPoint2(ProjectToward(StartPoint, CurrentLine->StartPoint(), theApp.DimensionLength()));
	CurrentLine->SetEndPoint2(ProjectToward(EndPoint, CurrentLine->EndPoint(), theApp.DimensionLength()));

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

void AeSysView::GenerateCorner(OdGePoint3d intersection, SelectionPair previousSelection, SelectionPair currentSelection, int cornerType) {
	
	auto PreviousLine {dynamic_cast<EoDbLine*>(get<tPrimitive>(previousSelection))};
	auto PreviousLineSeg {PreviousLine->LineSeg()};

	auto CurrentLine {dynamic_cast<EoDbLine*>(get<tPrimitive>(currentSelection))};
	auto CurrentLineSeg {CurrentLine->LineSeg()};

	if ((intersection - PreviousLineSeg.startPoint()).length() < (intersection - PreviousLineSeg.endPoint()).length()) {
		PreviousLineSeg.SetStartPoint(PreviousLineSeg.endPoint());
	}
	PreviousLineSeg.SetEndPoint(intersection);

	if ((intersection - CurrentLineSeg.endPoint()).length() < (intersection - CurrentLineSeg.startPoint()).length()) {
		CurrentLineSeg.SetEndPoint(CurrentLineSeg.startPoint());
	}
	CurrentLineSeg.SetStartPoint(intersection);

	OdGePoint3d	CenterPoint;
	if (FindCenterPointGivenRadiusAndTwoLineSegments(m_CornerSize, PreviousLineSeg, CurrentLineSeg, CenterPoint)) {
		auto Document {GetDocument()};

		PreviousLineSeg.SetEndPoint(PreviousLineSeg.ProjPt(CenterPoint));
		CurrentLineSeg.SetStartPoint(CurrentLineSeg.ProjPt(CenterPoint));

		if (PreviousFixupCommand == ID_OP1)
			;
		else if (PreviousFixupCommand == ID_OP2) {
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, PreviousLine);
			PreviousLine->SetStartPoint2(PreviousLineSeg.startPoint());
			PreviousLine->SetEndPoint2(intersection);
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, PreviousLine);
		} else if (PreviousFixupCommand == ID_OP3 || PreviousFixupCommand == ID_OP4) {
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, PreviousLine);
			PreviousLine->SetStartPoint2(PreviousLineSeg.startPoint());
			PreviousLine->SetEndPoint2(PreviousLineSeg.endPoint());
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, PreviousLine);
		}
		if ((cornerType & kTrimCurrentToSize) == kTrimCurrentToSize) {
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, CurrentLine);
			CurrentLine->SetStartPoint2(CurrentLineSeg.startPoint());
			CurrentLine->SetEndPoint2(CurrentLineSeg.endPoint());
			Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, CurrentLine);
		}
		OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
		auto Group {new EoDbGroup};

		if ((cornerType & kChamfer) == kChamfer) {
			auto Line {EoDbLine::Create(BlockTableRecord, PreviousLineSeg.endPoint(), CurrentLineSeg.startPoint())};
			Line->setColorIndex(PreviousLine->ColorIndex());
			Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(PreviousLine->LinetypeIndex()));
			Group->AddTail(EoDbLine::Create(Line));
		} else if ((cornerType & kFillet) == kFillet) {
			auto PlaneNormal {(intersection - PreviousLineSeg.endPoint()).crossProduct(CurrentLineSeg.startPoint() - PreviousLineSeg.endPoint())};
			PlaneNormal.normalize();
			double SweepAngle;
			pFndSwpAngGivPlnAnd3Lns(PlaneNormal, PreviousLineSeg.endPoint(), intersection, CurrentLineSeg.startPoint(), CenterPoint, SweepAngle);
			const auto MajorAxis {PreviousLineSeg.endPoint() - CenterPoint};

			auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
			Ellipse->set(CenterPoint, PlaneNormal, MajorAxis, 1., 0., SweepAngle);
			Ellipse->setColorIndex(PreviousLine->ColorIndex());
			Ellipse->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(PreviousLine->LinetypeIndex()));
			Group->AddTail(EoDbEllipse::Create(Ellipse));
		}
		Document->AddWorkLayerGroup(Group);
		Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
	}
}

bool AeSysView::FindCenterPointGivenRadiusAndTwoLineSegments(double radius, OdGeLineSeg3d firstLineSeg, OdGeLineSeg3d secondLineSeg, OdGePoint3d& center) {
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

	if (fabs((Normal.dotProduct((SecondLineStartPoint - FirstLineStartPoint)))) > DBL_EPSILON) { // Four points are not coplanar
		return false;
	}
	EoGeMatrix3d WorldToPlaneTransform;
	WorldToPlaneTransform.setToWorldToPlane(OdGePlane(FirstLineStartPoint, Normal));

	FirstLineEndPoint.transformBy(WorldToPlaneTransform);
	SecondLineStartPoint.transformBy(WorldToPlaneTransform);
	SecondLineEndPoint.transformBy(WorldToPlaneTransform);
	
	const double FirstLineA {-FirstLineEndPoint.y / FirstLineVectorLength};
	const double FirstLineB {FirstLineEndPoint.x / FirstLineVectorLength};
	SecondLineVector.x = SecondLineEndPoint.x - SecondLineStartPoint.x;
	SecondLineVector.y = SecondLineEndPoint.y - SecondLineStartPoint.y;
	const double SecondLineA {-SecondLineVector.y / SecondLineVectorLength};
	const double SecondLineB {SecondLineVector.x / SecondLineVectorLength};
	const double Determinant {SecondLineA * FirstLineB - FirstLineA * SecondLineB};

	const double SignedRadius {(FirstLineEndPoint.x * SecondLineEndPoint.y - SecondLineEndPoint.x * FirstLineEndPoint.y) >= 0. ? -fabs(radius) : fabs(radius)};

	const double dC1RAB1 {SignedRadius};
	const double dC2RAB2 {(SecondLineStartPoint.x * SecondLineEndPoint.y - SecondLineEndPoint.x * SecondLineStartPoint.y) / SecondLineVectorLength + SignedRadius};
	center.x = (SecondLineB * dC1RAB1 - FirstLineB * dC2RAB2) / Determinant;
	center.y = (FirstLineA * dC2RAB2 - SecondLineA * dC1RAB1) / Determinant;
	center.z = 0.;
	WorldToPlaneTransform.invert();
	center.transformBy(WorldToPlaneTransform);
	return true;
}