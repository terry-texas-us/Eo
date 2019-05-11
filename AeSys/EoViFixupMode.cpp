#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDlgFixupOptions.h"

using namespace std;

OdUInt16 PreviousFixupCommand = 0;

pair<EoDbGroup*, EoDbPrimitive*> ReferenceSelection {nullptr, nullptr};
pair<EoDbGroup*, EoDbPrimitive*> PreviousSelection {nullptr, nullptr};
pair<EoDbGroup*, EoDbPrimitive*> CurrentSelection {nullptr, nullptr};

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
	
	if (get<0>(ReferenceSelection) != nullptr) {
		Document->UpdatePrimitiveInAllViews(EoDb::kPrimitive, get<1>(ReferenceSelection));
	}
	ReferenceSelection = SelectLineUsingPoint(GetCursorPosition());

	if (get<0>(ReferenceSelection) == nullptr) { return; }

	auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(get<1>(ReferenceSelection))->LineSeg()};

	if (PreviousFixupCommand == 0) {
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP1);
	} else if (PreviousFixupCommand == ID_OP1) {
		;
	} else {
		OdGePoint3d IntersectionPoint;
		auto PreviousLineSeg {dynamic_cast<EoDbLine*>(get<1>(PreviousSelection))->LineSeg()};

		if (!PreviousLineSeg.intersectWith(ReferenceLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Unable to determine intersection with reference line");
			return;
		}
		if (PreviousFixupCommand == ID_OP2) {
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, get<0>(PreviousSelection));
			
			auto PreviousLine {dynamic_cast<EoDbLine*>(get<1>(PreviousSelection))};

			if ((IntersectionPoint - PreviousLine->StartPoint()).length() < (IntersectionPoint - PreviousLine->EndPoint()).length()) {
				PreviousLine->SetStartPoint2(IntersectionPoint);
			} else {
				PreviousLine->SetEndPoint2(IntersectionPoint);
			}
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, get<0>(PreviousSelection));
		} else if (PreviousFixupCommand == ID_OP3) {
			GenerateCorner(IntersectionPoint, PreviousSelection, ReferenceLineSeg, 3);
		} else if (PreviousFixupCommand == ID_OP4) {
			GenerateCorner(IntersectionPoint, PreviousSelection, ReferenceLineSeg, 4);
		}
		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeMend() {
	auto Document {GetDocument()};

	CurrentSelection = SelectLineUsingPoint(GetCursorPosition());
	auto CurrentGroup {get<0>(CurrentSelection)};
	
	if (CurrentGroup == nullptr) {
		theApp.AddStringToMessageList(L"0 lines found");
		return;
	}
	auto CurrentPrimitive {get<1>(CurrentSelection)};
	auto CurrentLine {dynamic_cast<EoDbLine*>(CurrentPrimitive)};
	auto CurrentLineSeg {CurrentLine->LineSeg()};
	
	OdGePoint3d IntersectionPoint;

	if (PreviousFixupCommand == 0) {
		PreviousSelection = CurrentSelection;
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP2);
	} else if (PreviousFixupCommand == ID_OP1) {
		auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(get<1>(ReferenceSelection))->LineSeg()};
		if (!ReferenceLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Unable to determine intersection with reference line");
			return;
		}
		Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, CurrentGroup);

		if ((IntersectionPoint - CurrentLine->StartPoint()).length() < (IntersectionPoint - CurrentLine->EndPoint()).length()) {
			CurrentLine->SetStartPoint2(IntersectionPoint);
		} else {
			CurrentLine->SetEndPoint2(IntersectionPoint);
		}
		Document->UpdateGroupInAllViews(EoDb::kGroupSafe, CurrentGroup);
	} else {
		auto PreviousLineSeg {dynamic_cast<EoDbLine*>(get<1>(PreviousSelection))->LineSeg()};

		if (!PreviousLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Selected lines do not define an intersection");
			return;
		}
		if (PreviousFixupCommand == ID_OP2) {
			auto PreviousLine {dynamic_cast<EoDbLine*>(get<1>(PreviousSelection))};
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, get<0>(PreviousSelection));
			
			if ((IntersectionPoint - PreviousLine->StartPoint()).length() < (IntersectionPoint - PreviousLine->EndPoint()).length()) {
				PreviousLine->SetStartPoint2(IntersectionPoint);
			} else {
				PreviousLine->SetEndPoint2(IntersectionPoint);
			}
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, get<0>(PreviousSelection));
		} else if (PreviousFixupCommand == ID_OP3) {
			GenerateCorner(IntersectionPoint, PreviousSelection, CurrentLineSeg, 3);
		} else if (PreviousFixupCommand == ID_OP4) {
			GenerateCorner(IntersectionPoint, PreviousSelection, CurrentLineSeg, 4);
		}
		Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, CurrentGroup);
		auto CurrentLine {dynamic_cast<EoDbLine*>(CurrentPrimitive)};
		
		if ((IntersectionPoint - CurrentLine->StartPoint()).length() < (IntersectionPoint - CurrentLine->EndPoint()).length()) {
			CurrentLine->SetStartPoint2(IntersectionPoint);
		} else {
			CurrentLine->SetEndPoint2(IntersectionPoint);
		}
		Document->UpdateGroupInAllViews(EoDb::kGroupSafe, CurrentGroup);
		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeChamfer() {
	CurrentSelection = SelectLineUsingPoint(GetCursorPosition());

	if (get<0>(CurrentSelection) == nullptr) {
		theApp.AddStringToMessageList(L"0 lines found");
		return;
	}
	auto CurrentLineSeg {dynamic_cast<EoDbLine*>(get<1>(CurrentSelection))->LineSeg()};

	OdGePoint3d IntersectionPoint;

	if (PreviousFixupCommand == 0) {
		PreviousSelection = CurrentSelection;
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP3);
	} else if (PreviousFixupCommand == ID_OP1) {
		auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(get<1>(ReferenceSelection))->LineSeg()};

		if (!ReferenceLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Selected lines do not define an intersection");
			return;
		}
		GenerateCorner(IntersectionPoint, ReferenceSelection, CurrentLineSeg, 3);

		ModeLineUnhighlightOp(PreviousFixupCommand);
	} else {
		auto PreviousLineSeg {dynamic_cast<EoDbLine*>(get<1>(PreviousSelection))->LineSeg()};

		if (!PreviousLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Selected lines do not define an intersection");
			return;
		}
		GenerateCorner(IntersectionPoint, PreviousSelection, CurrentLineSeg, 3);

		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeFillet() {
	CurrentSelection = SelectLineUsingPoint(GetCursorPosition());
	
	if (get<0>(CurrentSelection) == nullptr) {
		theApp.AddStringToMessageList(L"0 lines found");
		return;
	}
	auto CurrentLineSeg {dynamic_cast<EoDbLine*>(get<1>(CurrentSelection))->LineSeg()};

	OdGePoint3d IntersectionPoint;

	if (PreviousFixupCommand == 0) {
		PreviousSelection = CurrentSelection;
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP4);
	} else if (PreviousFixupCommand == ID_OP1) {
		auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(get<1>(ReferenceSelection))->LineSeg()};

		if (!ReferenceLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Selected lines do not define an intersection");
			return;
		}
		GenerateCorner(IntersectionPoint, ReferenceSelection, CurrentLineSeg, 4);

		ModeLineUnhighlightOp(PreviousFixupCommand);
	} else {
		auto PreviousLineSeg {dynamic_cast<EoDbLine*>(get<1>(PreviousSelection))->LineSeg()};

		if (!PreviousLineSeg.intersectWith(CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Selected lines do not define an intersection");
			return;
		}
		GenerateCorner(IntersectionPoint, PreviousSelection, CurrentLineSeg, 4);

		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeSquare() {
	auto Document {GetDocument()};

	auto CurrentPnt {GetCursorPosition()};

	CurrentSelection = SelectLineUsingPoint(CurrentPnt);

	if (get<0>(CurrentSelection) == nullptr) { return; }

	auto CurrentPrimitive {get<1>(CurrentSelection)};
	auto CurrentLine {dynamic_cast<EoDbLine*>(CurrentPrimitive)};
	auto CurrentLineSeg {dynamic_cast<EoDbLine*>(get<1>(CurrentSelection))->LineSeg()};

	CurrentPnt = CurrentLineSeg.ProjPt(CurrentPnt);

	const double CurrentLineSegLength = CurrentLineSeg.length();
	Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, get<0>(CurrentSelection));
	
	auto StartPoint {SnapPointToAxis(CurrentPnt, CurrentLineSeg.startPoint())};
	auto EndPoint {ProjectToward(StartPoint, CurrentPnt, CurrentLineSegLength)};

	CurrentLineSeg.set(StartPoint, EndPoint);

	CurrentLine->SetStartPoint2(SnapPointToGrid(CurrentLineSeg.startPoint()));
	CurrentLine->SetEndPoint2(SnapPointToGrid(CurrentLineSeg.endPoint()));

	Document->UpdateGroupInAllViews(EoDb::kGroupSafe, get<0>(CurrentSelection));
}

void AeSysView::OnFixupModeParallel() {
	auto Document {GetDocument()};

	if (get<0>(ReferenceSelection) == nullptr) {
		theApp.AddStringToMessageList(L"Reference line must be selected first.");
		return;
	}
	CurrentSelection = SelectLineUsingPoint(GetCursorPosition());

	if (get<0>(CurrentSelection) == nullptr) { return; }

	auto CurrentLine {dynamic_cast<EoDbLine*>(get<1>(CurrentSelection))};
	auto CurrentLineSeg {CurrentLine->LineSeg()};

	auto ReferenceLineSeg {dynamic_cast<EoDbLine*>(get<1>(ReferenceSelection))->LineSeg()};

	CurrentLineSeg.set(ReferenceLineSeg.ProjPt(CurrentLine->StartPoint()), ReferenceLineSeg.ProjPt(CurrentLine->EndPoint()));
	Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, get<0>(CurrentSelection));
	const auto StartPoint {CurrentLineSeg.startPoint()};
	CurrentLine->SetStartPoint2(ProjectToward(StartPoint, CurrentLine->StartPoint(), theApp.DimensionLength()));
	const auto EndPoint {CurrentLineSeg.endPoint()};
	CurrentLine->SetEndPoint2(ProjectToward(EndPoint, CurrentLine->EndPoint(), theApp.DimensionLength()));
	Document->UpdateGroupInAllViews(EoDb::kGroupSafe, get<0>(CurrentSelection));
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

void AeSysView::GenerateCorner(OdGePoint3d intersection, pair<EoDbGroup*, EoDbPrimitive*> previousSelection, EoGeLineSeg3d currentLineSeg, int cornerType) {
	
	auto PreviousLine {dynamic_cast<EoDbLine*>(get<1>(previousSelection))};
	auto PreviousLineSeg {PreviousLine->LineSeg()};

	if ((intersection - PreviousLineSeg.startPoint()).length() < (intersection - PreviousLineSeg.endPoint()).length()) {
		PreviousLineSeg.SetStartPoint(PreviousLineSeg.endPoint());
	}
	PreviousLineSeg.SetEndPoint(intersection);

	if ((intersection - currentLineSeg.endPoint()).length() < (intersection - currentLineSeg.startPoint()).length()) {
		currentLineSeg.SetEndPoint(currentLineSeg.startPoint());
	}
	currentLineSeg.SetStartPoint(intersection);

	OdGePoint3d	CenterPoint;
	if (FindCenterPointGivenRadiusAndTwoLineSegments(m_CornerSize, PreviousLineSeg, currentLineSeg, CenterPoint)) {
		auto Document {GetDocument()};

		PreviousLineSeg.SetEndPoint(PreviousLineSeg.ProjPt(CenterPoint));
		currentLineSeg.SetStartPoint(currentLineSeg.ProjPt(CenterPoint));

		if (PreviousFixupCommand == ID_OP1)
			;
		else if (PreviousFixupCommand == ID_OP2) {
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, get<0>(previousSelection));
			PreviousLine->SetStartPoint2(PreviousLineSeg.startPoint());
			PreviousLine->SetEndPoint2(intersection);
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, get<0>(previousSelection));
		} else if (PreviousFixupCommand == ID_OP3 || PreviousFixupCommand == ID_OP4) {
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, get<0>(previousSelection));
			PreviousLine->SetStartPoint2(PreviousLineSeg.startPoint());
			PreviousLine->SetEndPoint2(PreviousLineSeg.endPoint());
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, get<0>(previousSelection));
		}
		if (get<1>(CurrentSelection) != nullptr) {
			auto CurrentLine {dynamic_cast<EoDbLine*>(get<1>(CurrentSelection))};
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, get<0>(CurrentSelection));
			CurrentLine->SetStartPoint2(currentLineSeg.startPoint());
			CurrentLine->SetEndPoint2(currentLineSeg.endPoint());
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, get<0>(CurrentSelection));
		}
		OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
		auto Group {new EoDbGroup};

		if (cornerType == 3) {
			auto Line {EoDbLine::Create(BlockTableRecord, PreviousLineSeg.endPoint(), currentLineSeg.startPoint())};
			Line->setColorIndex(PreviousLine->ColorIndex());
			Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(PreviousLine->LinetypeIndex()));
			Group->AddTail(EoDbLine::Create(Line));
		} else if (cornerType == 4) {
			auto PlaneNormal {(intersection - PreviousLineSeg.endPoint()).crossProduct(currentLineSeg.startPoint() - PreviousLineSeg.endPoint())};
			PlaneNormal.normalize();
			double SweepAngle;
			pFndSwpAngGivPlnAnd3Lns(PlaneNormal, PreviousLineSeg.endPoint(), intersection, currentLineSeg.startPoint(), CenterPoint, SweepAngle);
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