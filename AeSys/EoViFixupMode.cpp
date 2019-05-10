#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDlgFixupOptions.h"

OdUInt16 PreviousFixupCommand = 0;

EoDbGroup* ReferenceGroup;
EoDbPrimitive* ReferencePrimitive;

EoDbGroup* PreviousGroup;
EoDbPrimitive* PreviousPrimitive;

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

	if (ReferenceGroup != nullptr) {
		Document->UpdatePrimitiveInAllViews(EoDb::kPrimitive, ReferencePrimitive);
	}
	auto Selection {SelectLineUsingPoint(GetCursorPosition())};
	ReferenceGroup = std::get<0>(Selection);
	
	if (ReferenceGroup == nullptr) { return; }

	ReferencePrimitive = std::get<1>(Selection);

	m_ReferenceLineSeg = dynamic_cast<EoDbLine*>(ReferencePrimitive)->LineSeg();

	if (PreviousFixupCommand == 0) {
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP1);
	} else if (PreviousFixupCommand == ID_OP1) {
		;
	} else {
		auto PreviousLine {dynamic_cast<EoDbLine*>(PreviousPrimitive)};

		OdGePoint3d IntersectionPoint;

		if (!m_PreviousLineSeg.intersectWith(m_ReferenceLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Unable to determine intersection with reference line");
			return;
		}
		if (PreviousFixupCommand == ID_OP2) {
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
			
			if ((IntersectionPoint - PreviousLine->EndPoint()).length() < (IntersectionPoint - PreviousLine->EndPoint()).length()) {
				PreviousLine->SetStartPoint2(IntersectionPoint);
			} else {
				PreviousLine->SetEndPoint2(IntersectionPoint);
			}
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
		} else if (PreviousFixupCommand == ID_OP3) {
			
			if ((IntersectionPoint - m_PreviousLineSeg.startPoint()).length() < (IntersectionPoint - m_PreviousLineSeg.endPoint()).length()) {
				m_PreviousLineSeg.SetStartPoint(m_PreviousLineSeg.endPoint());
			}
			m_PreviousLineSeg.SetEndPoint(IntersectionPoint);
			
			if ((IntersectionPoint - m_ReferenceLineSeg.endPoint()).length() < (IntersectionPoint - m_ReferenceLineSeg.startPoint()).length()) {
				m_ReferenceLineSeg.SetEndPoint(m_ReferenceLineSeg.startPoint());
			}
			m_ReferenceLineSeg.SetStartPoint(IntersectionPoint);
			OdGePoint3d	CenterPoint;
			
			if (FindCenterPointGivenRadiusAndTwoLineSegments(m_CornerSize, m_PreviousLineSeg, m_ReferenceLineSeg, CenterPoint)) {
				m_PreviousLineSeg.SetEndPoint(m_PreviousLineSeg.ProjPt(CenterPoint));
				m_ReferenceLineSeg.SetStartPoint(m_ReferenceLineSeg.ProjPt(CenterPoint));
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				PreviousLine->SetStartPoint2(m_PreviousLineSeg.startPoint());
				PreviousLine->SetEndPoint2(m_PreviousLineSeg.endPoint());

				OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
				auto Line {EoDbLine::Create(BlockTableRecord, m_PreviousLineSeg.endPoint(), m_ReferenceLineSeg.startPoint())};
				Line->setColorIndex(PreviousLine->ColorIndex());
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(PreviousLine->LinetypeIndex()));
				PreviousGroup->AddTail(EoDbLine::Create(Line));

				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			}
		} else if (PreviousFixupCommand == ID_OP4) {
			
			if ((IntersectionPoint - m_PreviousLineSeg.startPoint()).length() < (IntersectionPoint - m_PreviousLineSeg.endPoint()).length()) {
				m_PreviousLineSeg.SetStartPoint(m_PreviousLineSeg.endPoint());
			}
			m_PreviousLineSeg.SetEndPoint(IntersectionPoint);
			
			if ((IntersectionPoint - m_ReferenceLineSeg.endPoint()).length() < (IntersectionPoint - m_ReferenceLineSeg.startPoint()).length()) {
				m_ReferenceLineSeg.SetEndPoint(m_ReferenceLineSeg.startPoint());
			}
			m_ReferenceLineSeg.SetStartPoint(IntersectionPoint);
			OdGePoint3d	CenterPoint;
			
			if (FindCenterPointGivenRadiusAndTwoLineSegments(m_CornerSize, m_PreviousLineSeg, m_ReferenceLineSeg, CenterPoint)) {
				m_PreviousLineSeg.SetEndPoint(m_PreviousLineSeg.ProjPt(CenterPoint));
				m_ReferenceLineSeg.SetStartPoint(m_ReferenceLineSeg.ProjPt(CenterPoint));

				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				PreviousLine->SetStartPoint2(m_PreviousLineSeg.startPoint());
				PreviousLine->SetEndPoint2(m_PreviousLineSeg.endPoint());
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);

				auto PlaneNormal {(IntersectionPoint - m_PreviousLineSeg.endPoint()).crossProduct(m_ReferenceLineSeg.startPoint() - m_PreviousLineSeg.endPoint())};
				PlaneNormal.normalize();
				double SweepAngle;
				pFndSwpAngGivPlnAnd3Lns(PlaneNormal, m_PreviousLineSeg.endPoint(), IntersectionPoint, m_ReferenceLineSeg.startPoint(), CenterPoint, SweepAngle);
				const auto MajorAxis {m_PreviousLineSeg.endPoint() - CenterPoint};

				auto Group {new EoDbGroup};
				OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
				auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};

				Ellipse->set(CenterPoint, PlaneNormal, MajorAxis, 1., 0., SweepAngle);
				Group->AddTail(EoDbEllipse::Create(Ellipse));
				Document->AddWorkLayerGroup(Group);
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
			}
		}
		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeMend() {
	auto Document {GetDocument()};

	auto Selection {SelectLineUsingPoint(GetCursorPosition())};
	auto CurrentGroup = std::get<0>(Selection);
	
	if (CurrentGroup == nullptr) {
		theApp.AddStringToMessageList(L"0 lines found");
		return;
	}
	auto CurrentPrimitive = std::get<1>(Selection);
	auto CurrentLine {dynamic_cast<EoDbLine*>(CurrentPrimitive)};

	m_CurrentLineSeg = CurrentLine->LineSeg();

	OdGePoint3d IntersectionPoint;

	if (PreviousFixupCommand == 0) {
		PreviousGroup = CurrentGroup;
		PreviousPrimitive = CurrentPrimitive;
		m_PreviousLineSeg.set(m_CurrentLineSeg.startPoint(), m_CurrentLineSeg.endPoint());
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP2);
	} else if (PreviousFixupCommand == ID_OP1) {
		if (!m_ReferenceLineSeg.intersectWith(m_CurrentLineSeg, IntersectionPoint)) {
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
		
		if (!m_PreviousLineSeg.intersectWith(m_CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Selected lines do not define an intersection");
			return;
		}
		if (PreviousFixupCommand == ID_OP2) {
			auto PreviousLine {dynamic_cast<EoDbLine*>(PreviousPrimitive)};
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
			
			if ((IntersectionPoint - PreviousLine->StartPoint()).length() < (IntersectionPoint - PreviousLine->EndPoint()).length()) {
				PreviousLine->SetStartPoint2(IntersectionPoint);
			} else {
				PreviousLine->SetEndPoint2(IntersectionPoint);
			}
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
		} else if (PreviousFixupCommand == ID_OP3) {
			
			if ((IntersectionPoint - m_PreviousLineSeg.startPoint()).length() < (IntersectionPoint - m_PreviousLineSeg.endPoint()).length()) {
				m_PreviousLineSeg.SetStartPoint(m_PreviousLineSeg.endPoint());
			}
			m_PreviousLineSeg.SetEndPoint(IntersectionPoint);
			
			if ((IntersectionPoint - m_CurrentLineSeg.endPoint()).length() < (IntersectionPoint - m_CurrentLineSeg.startPoint()).length()) {
				m_CurrentLineSeg.SetEndPoint(m_CurrentLineSeg.startPoint());
			}
			m_CurrentLineSeg.SetStartPoint(IntersectionPoint);
			OdGePoint3d	CenterPoint;
			
			if (FindCenterPointGivenRadiusAndTwoLineSegments(m_CornerSize, m_PreviousLineSeg, m_CurrentLineSeg, CenterPoint)) {
				auto PreviousLine {dynamic_cast<EoDbLine*>(PreviousPrimitive)};
				m_PreviousLineSeg.SetEndPoint(m_PreviousLineSeg.ProjPt(CenterPoint));
				m_CurrentLineSeg.SetStartPoint(m_CurrentLineSeg.ProjPt(CenterPoint));
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				PreviousLine->SetStartPoint2(m_PreviousLineSeg.startPoint());
				PreviousLine->SetEndPoint2(m_PreviousLineSeg.endPoint());

				OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};

				auto Line {EoDbLine::Create(BlockTableRecord, m_PreviousLineSeg.endPoint(), m_CurrentLineSeg.startPoint())};
				Line->setColorIndex(PreviousLine->ColorIndex());
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(PreviousLine->LinetypeIndex()));
				PreviousGroup->AddTail(EoDbLine::Create(Line));

				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			}
		} else if (PreviousFixupCommand == ID_OP4) {
			
			if ((IntersectionPoint - m_PreviousLineSeg.startPoint()).length() < (IntersectionPoint - m_PreviousLineSeg.endPoint()).length()) {
				m_PreviousLineSeg.SetStartPoint(m_PreviousLineSeg.endPoint());
			}
			m_PreviousLineSeg.SetEndPoint(IntersectionPoint);
			
			if ((IntersectionPoint - m_CurrentLineSeg.endPoint()).length() < (IntersectionPoint - m_CurrentLineSeg.startPoint()).length()) {
				m_CurrentLineSeg.SetEndPoint(m_CurrentLineSeg.startPoint());
			}
			m_CurrentLineSeg.SetStartPoint(IntersectionPoint);
			OdGePoint3d	CenterPoint;
			
			if (FindCenterPointGivenRadiusAndTwoLineSegments(m_CornerSize, m_PreviousLineSeg, m_CurrentLineSeg, CenterPoint)) {
				auto PreviousLine {dynamic_cast<EoDbLine*>(PreviousPrimitive)};
				m_PreviousLineSeg.SetEndPoint(m_PreviousLineSeg.ProjPt(CenterPoint));
				m_CurrentLineSeg.SetStartPoint(m_CurrentLineSeg.ProjPt(CenterPoint));
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				PreviousLine->SetStartPoint2(m_PreviousLineSeg.startPoint());
				PreviousLine->SetEndPoint2(m_PreviousLineSeg.endPoint());
				auto PlaneNormal {(IntersectionPoint - m_PreviousLineSeg.endPoint()).crossProduct(m_CurrentLineSeg.startPoint() - m_PreviousLineSeg.endPoint())};
				PlaneNormal.normalize();
				double SweepAngle;
				pFndSwpAngGivPlnAnd3Lns(PlaneNormal, m_PreviousLineSeg.endPoint(), IntersectionPoint, m_CurrentLineSeg.startPoint(), CenterPoint, SweepAngle);
				const auto MajorAxis {m_PreviousLineSeg.endPoint() - CenterPoint};

				OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};

				auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
				Ellipse->set(CenterPoint, PlaneNormal, MajorAxis, 1., 0., SweepAngle);

				PreviousGroup->AddTail(EoDbEllipse::Create(Ellipse));
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			}
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
	auto Document {GetDocument()};

	const auto CurrentPnt {GetCursorPosition()};
	OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};

	OdGePoint3d IntersectionPoint;

	auto Selection {SelectLineUsingPoint(GetCursorPosition())};
	auto CurrentGroup = std::get<0>(Selection);

	if (CurrentGroup == nullptr) {
		theApp.AddStringToMessageList(L"0 lines found");
		return;
	}
	auto CurrentPrimitive = std::get<1>(Selection);
	auto CurrentLine {dynamic_cast<EoDbLine*>(CurrentPrimitive)};

	m_CurrentLineSeg = CurrentLine->LineSeg();

	if (PreviousFixupCommand == 0) {
		PreviousGroup = CurrentGroup;
		PreviousPrimitive = CurrentPrimitive;
		m_PreviousLineSeg = m_CurrentLineSeg;
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP3);
	} else {
		
		if (PreviousFixupCommand == ID_OP1) {
			PreviousGroup = CurrentGroup;
			PreviousPrimitive = CurrentPrimitive;
			m_PreviousLineSeg = m_ReferenceLineSeg;
		}
		if (!m_PreviousLineSeg.intersectWith(m_CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Selected lines do not define an intersection");
			return;
		}
		if ((IntersectionPoint - m_PreviousLineSeg.startPoint()).length() < (IntersectionPoint - m_PreviousLineSeg.endPoint()).length()) {
			m_PreviousLineSeg.SetStartPoint(m_PreviousLineSeg.endPoint());
		}
		m_PreviousLineSeg.SetEndPoint(IntersectionPoint);
		
		if ((IntersectionPoint - m_CurrentLineSeg.endPoint()).length() < (IntersectionPoint - m_CurrentLineSeg.startPoint()).length()) {
			m_CurrentLineSeg.SetEndPoint(m_CurrentLineSeg.startPoint());
		}
		m_CurrentLineSeg.SetStartPoint(IntersectionPoint);
		
		OdGePoint3d	CenterPoint;
		if (FindCenterPointGivenRadiusAndTwoLineSegments(m_CornerSize, m_PreviousLineSeg, m_CurrentLineSeg, CenterPoint)) {
			m_PreviousLineSeg.SetEndPoint(m_PreviousLineSeg.ProjPt(CenterPoint));
			m_CurrentLineSeg.SetStartPoint(m_CurrentLineSeg.ProjPt(CenterPoint));
			
			if (PreviousFixupCommand == ID_OP1)
				;
			else if (PreviousFixupCommand == ID_OP2) {
				auto PreviousLine {dynamic_cast<EoDbLine*>(PreviousPrimitive)};
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				PreviousLine->SetStartPoint2(m_PreviousLineSeg.startPoint());
				PreviousLine->SetEndPoint2(IntersectionPoint);
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			} else if (PreviousFixupCommand == ID_OP3 || PreviousFixupCommand == ID_OP4) {
				auto PreviousLine {dynamic_cast<EoDbLine*>(PreviousPrimitive)};
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				PreviousLine->SetStartPoint2(m_PreviousLineSeg.startPoint());
				PreviousLine->SetEndPoint2(m_PreviousLineSeg.endPoint());
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			}
			auto CurrentLine {dynamic_cast<EoDbLine*>(CurrentPrimitive)};
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, CurrentGroup);
			CurrentLine->SetStartPoint2(m_CurrentLineSeg.startPoint());
			CurrentLine->SetEndPoint2(m_CurrentLineSeg.endPoint());

			auto Line {EoDbLine::Create(BlockTableRecord, m_PreviousLineSeg.endPoint(), m_CurrentLineSeg.startPoint())};
			Line->setColorIndex(CurrentLine->ColorIndex());
			Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(CurrentLine->LinetypeIndex()));
			CurrentGroup->AddTail(EoDbLine::Create(Line));

			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, CurrentGroup);
		}
		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeFillet() {
	auto Document {GetDocument()};

	const auto CurrentPnt {GetCursorPosition()};
	OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};

	auto Selection {SelectLineUsingPoint(CurrentPnt)};
	auto CurrentGroup {std::get<0>(Selection)};
	
	if (CurrentGroup == nullptr) { return; }

	auto CurrentPrimitive {std::get<1>(Selection)};
	auto CurrentLine {dynamic_cast<EoDbLine*>(CurrentPrimitive)};

	m_CurrentLineSeg = CurrentLine->LineSeg();

	if (PreviousFixupCommand == 0) {
		PreviousGroup = CurrentGroup;
		PreviousPrimitive = CurrentPrimitive;
		m_PreviousLineSeg = m_CurrentLineSeg;
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP4);
	} else {
		OdGePoint3d IntersectionPoint;

		if (PreviousFixupCommand == ID_OP1) {
			PreviousGroup = CurrentGroup;
			PreviousPrimitive = CurrentPrimitive;
			m_PreviousLineSeg = m_ReferenceLineSeg;
		}
		if (!m_PreviousLineSeg.intersectWith(m_CurrentLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Selected lines do not define an intersection");
			return;
		}
		if ((IntersectionPoint - m_PreviousLineSeg.startPoint()).length() < (IntersectionPoint - m_PreviousLineSeg.endPoint()).length()) {
			m_PreviousLineSeg.SetStartPoint(m_PreviousLineSeg.endPoint());
		}
		m_PreviousLineSeg.SetEndPoint(IntersectionPoint);
		
		if ((IntersectionPoint - m_CurrentLineSeg.endPoint()).length() < (IntersectionPoint - m_CurrentLineSeg.startPoint()).length()) {
			m_CurrentLineSeg.SetEndPoint(m_CurrentLineSeg.startPoint());
		}
		m_CurrentLineSeg.SetStartPoint(IntersectionPoint);
		OdGePoint3d	CenterPoint;
		
		if (FindCenterPointGivenRadiusAndTwoLineSegments(m_CornerSize, m_PreviousLineSeg, m_CurrentLineSeg, CenterPoint)) {
			m_PreviousLineSeg.SetEndPoint(m_PreviousLineSeg.ProjPt(CenterPoint));
			m_CurrentLineSeg.SetStartPoint(m_CurrentLineSeg.ProjPt(CenterPoint));
			
			if (PreviousFixupCommand == ID_OP1)
				;
			else if (PreviousFixupCommand == ID_OP2) {
				auto PreviousLine {dynamic_cast<EoDbLine*>(PreviousPrimitive)};
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				PreviousLine->SetStartPoint2(m_PreviousLineSeg.startPoint());
				PreviousLine->SetEndPoint2(IntersectionPoint);
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			} else if (PreviousFixupCommand == ID_OP3 || PreviousFixupCommand == ID_OP4) {
				auto PreviousLine {dynamic_cast<EoDbLine*>(PreviousPrimitive)};
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				PreviousLine->SetStartPoint2(m_PreviousLineSeg.startPoint());
				PreviousLine->SetEndPoint2(m_PreviousLineSeg.endPoint());
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			}
			auto CurrentLine {dynamic_cast<EoDbLine*>(CurrentPrimitive)};
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, CurrentGroup);
			CurrentLine->SetStartPoint2(m_CurrentLineSeg.startPoint());
			CurrentLine->SetEndPoint2(m_CurrentLineSeg.endPoint());

			double SweepAngle;
			auto PlaneNormal {(IntersectionPoint - m_PreviousLineSeg.endPoint()).crossProduct(m_CurrentLineSeg.startPoint() - m_PreviousLineSeg.endPoint())};
			PlaneNormal.normalize();
			pFndSwpAngGivPlnAnd3Lns(PlaneNormal, m_PreviousLineSeg.endPoint(), IntersectionPoint, m_CurrentLineSeg.startPoint(), CenterPoint, SweepAngle);
			const auto MajorAxis {m_PreviousLineSeg.endPoint() - CenterPoint};

			auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
			Ellipse->set(CenterPoint, PlaneNormal, MajorAxis, 1., 0., SweepAngle);

			CurrentGroup->AddTail(EoDbEllipse::Create(Ellipse));

			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, CurrentGroup);
		}
		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeSquare() {
	auto Document {GetDocument()};

	auto CurrentPnt {GetCursorPosition()};

	auto Selection {SelectLineUsingPoint(CurrentPnt)};
	auto CurrentGroup {std::get<0>(Selection)};

	if (CurrentGroup == nullptr) { return; }

	auto CurrentPrimitive {std::get<1>(Selection)};
	auto CurrentLine {dynamic_cast<EoDbLine*>(CurrentPrimitive)};

	m_CurrentLineSeg = CurrentLine->LineSeg();

	CurrentPnt = m_CurrentLineSeg.ProjPt(CurrentPnt);

	const double CurrentLineSegLength = m_CurrentLineSeg.length();
	Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, CurrentGroup);
	
	auto StartPoint {SnapPointToAxis(CurrentPnt, m_CurrentLineSeg.startPoint())};
	auto EndPoint {ProjectToward(StartPoint, CurrentPnt, CurrentLineSegLength)};

	m_CurrentLineSeg.set(StartPoint, EndPoint);

	CurrentLine->SetStartPoint2(SnapPointToGrid(m_CurrentLineSeg.startPoint()));
	CurrentLine->SetEndPoint2(SnapPointToGrid(m_CurrentLineSeg.endPoint()));

	Document->UpdateGroupInAllViews(EoDb::kGroupSafe, CurrentGroup);
}

void AeSysView::OnFixupModeParallel() {
	auto Document {GetDocument()};

	if (ReferenceGroup == nullptr) {
		theApp.AddStringToMessageList(L"Reference line must be selected first.");
		return;
	}
	auto Selection {SelectLineUsingPoint(GetCursorPosition())};
	auto CurrentGroup {std::get<0>(Selection)};

	if (CurrentGroup == nullptr) { return; }

	auto CurrentPrimitive {std::get<1>(Selection)};
	auto CurrentLine {dynamic_cast<EoDbLine*>(CurrentPrimitive)};

	m_CurrentLineSeg.set(m_ReferenceLineSeg.ProjPt(CurrentLine->StartPoint()), m_ReferenceLineSeg.ProjPt(CurrentLine->EndPoint()));
	Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, CurrentGroup);
	const auto StartPoint {m_CurrentLineSeg.startPoint()};
	CurrentLine->SetStartPoint2(ProjectToward(StartPoint, CurrentLine->StartPoint(), theApp.DimensionLength()));
	const auto EndPoint {m_CurrentLineSeg.endPoint()};
	CurrentLine->SetEndPoint2(ProjectToward(EndPoint, CurrentLine->EndPoint(), theApp.DimensionLength()));
	Document->UpdateGroupInAllViews(EoDb::kGroupSafe, CurrentGroup);
}

void AeSysView::OnFixupModeReturn() {
	auto Document {GetDocument()};

	if (ReferenceGroup != nullptr) {
		Document->UpdatePrimitiveInAllViews(EoDb::kPrimitive, ReferencePrimitive);
		ReferenceGroup = nullptr;
		ReferencePrimitive = nullptr;
	}
	ModeLineUnhighlightOp(PreviousFixupCommand);
}

void AeSysView::OnFixupModeEscape() {
	auto Document {GetDocument()};

	if (ReferenceGroup != nullptr) {
		Document->UpdatePrimitiveInAllViews(EoDb::kPrimitive, ReferencePrimitive);
		ReferenceGroup = nullptr;
		ReferencePrimitive = nullptr;
	}
	ModeLineUnhighlightOp(PreviousFixupCommand);
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