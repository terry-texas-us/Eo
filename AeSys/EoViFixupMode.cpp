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
void AeSysView::OnFixupModeOptions(void) {
	EoDlgFixupOptions Dialog;
	Dialog.m_FixupAxisTolerance = m_FixupModeAxisTolerance;
	Dialog.m_FixupModeCornerSize = m_FixupModeCornerSize;
	if (Dialog.DoModal() == IDOK) {
		m_FixupModeCornerSize = EoMax(0., Dialog.m_FixupModeCornerSize);
		m_FixupModeAxisTolerance = EoMax(0., Dialog.m_FixupAxisTolerance);
		SetAxisConstraintInfluenceAngle(m_FixupModeAxisTolerance);
	}
}
void AeSysView::OnFixupModeReference(void) {
	AeSysDoc* Document = GetDocument();

	auto CurrentPnt {GetCursorPosition()};

	OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};

	OdGePoint3d IntersectionPoint;

	if (ReferenceGroup != nullptr) {
		Document->UpdatePrimitiveInAllViews(EoDb::kPrimitive, ReferencePrimitive);
	}
	ReferenceGroup = SelectGroupAndPrimitive(CurrentPnt);
	if (ReferenceGroup == nullptr) {
		return;
	}
	ReferencePrimitive = EngagedPrimitive();
	if (!ReferencePrimitive->Is(EoDb::kLinePrimitive)) {
		return;
	}
	CurrentPnt = DetPt();
	dynamic_cast<EoDbLine*>(ReferencePrimitive)->GetLine(m_FixupModeReferenceLine);

	if (PreviousFixupCommand == 0)
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP1);
	else if (PreviousFixupCommand == ID_OP1)
		;
	else {
		auto PreviousLine {dynamic_cast<EoDbLine*>(PreviousPrimitive)};
		if (!m_FixupModeFirstLine.intersectWith(m_FixupModeReferenceLine, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Unable to determine intersection with reference line");
			theApp.AddModeInformationToMessageList();
			return;
		}
		if (PreviousFixupCommand == ID_OP2) {
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
			if ((IntersectionPoint - PreviousLine->EndPoint()).length() < (IntersectionPoint - PreviousLine->EndPoint()).length())
				PreviousLine->SetStartPoint2(IntersectionPoint);
			else
				PreviousLine->SetEndPoint2(IntersectionPoint);
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
		} else if (PreviousFixupCommand == ID_OP3) {
			if ((IntersectionPoint - m_FixupModeFirstLine.startPoint()).length() < (IntersectionPoint - m_FixupModeFirstLine.endPoint()).length())
				m_FixupModeFirstLine.SetStartPoint(m_FixupModeFirstLine.endPoint());
			m_FixupModeFirstLine.SetEndPoint(IntersectionPoint);
			if ((IntersectionPoint - m_FixupModeReferenceLine.endPoint()).length() < (IntersectionPoint - m_FixupModeReferenceLine.startPoint()).length())
				m_FixupModeReferenceLine.SetEndPoint(m_FixupModeReferenceLine.startPoint());
			m_FixupModeReferenceLine.SetStartPoint(IntersectionPoint);
			OdGePoint3d	ptCP;
			if (pFndCPGivRadAnd4Pts(m_FixupModeCornerSize, m_FixupModeFirstLine.startPoint(), m_FixupModeFirstLine.endPoint(), m_FixupModeReferenceLine.startPoint(), m_FixupModeReferenceLine.endPoint(), &ptCP)) {
				m_FixupModeFirstLine.SetEndPoint(m_FixupModeFirstLine.ProjPt(ptCP));
				m_FixupModeReferenceLine.SetStartPoint(m_FixupModeReferenceLine.ProjPt(ptCP));
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				PreviousLine->SetStartPoint2(m_FixupModeFirstLine.startPoint());
				PreviousLine->SetEndPoint2(m_FixupModeFirstLine.endPoint());

				auto Line {EoDbLine::Create(BlockTableRecord, m_FixupModeFirstLine.endPoint(), m_FixupModeReferenceLine.startPoint())};
				Line->setColorIndex(PreviousLine->ColorIndex());
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(PreviousLine->LinetypeIndex()));
				PreviousGroup->AddTail(EoDbLine::Create(Line));

				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			}
		} else if (PreviousFixupCommand == ID_OP4) {
			if ((IntersectionPoint - m_FixupModeFirstLine.startPoint()).length() < (IntersectionPoint - m_FixupModeFirstLine.endPoint()).length())
				m_FixupModeFirstLine.SetStartPoint(m_FixupModeFirstLine.endPoint());
			m_FixupModeFirstLine.SetEndPoint(IntersectionPoint);
			if ((IntersectionPoint - m_FixupModeReferenceLine.endPoint()).length() < (IntersectionPoint - m_FixupModeReferenceLine.startPoint()).length())
				m_FixupModeReferenceLine.SetEndPoint(m_FixupModeReferenceLine.startPoint());
			m_FixupModeReferenceLine.SetStartPoint(IntersectionPoint);
			OdGePoint3d	CenterPoint;
			if (pFndCPGivRadAnd4Pts(m_FixupModeCornerSize, m_FixupModeFirstLine.startPoint(), m_FixupModeFirstLine.endPoint(), m_FixupModeReferenceLine.startPoint(), m_FixupModeReferenceLine.endPoint(), &CenterPoint)) {
				m_FixupModeFirstLine.SetEndPoint(m_FixupModeFirstLine.ProjPt(CenterPoint));
				m_FixupModeReferenceLine.SetStartPoint(m_FixupModeReferenceLine.ProjPt(CenterPoint));

				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				PreviousLine->SetStartPoint2(m_FixupModeFirstLine.startPoint());
				PreviousLine->SetEndPoint2(m_FixupModeFirstLine.endPoint());
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);

				const auto rPrvEndInter {IntersectionPoint - m_FixupModeFirstLine.endPoint()};
				const auto rPrvEndRefBeg {m_FixupModeReferenceLine.startPoint() - m_FixupModeFirstLine.endPoint()};
				auto PlaneNormal {rPrvEndInter.crossProduct(rPrvEndRefBeg)};
				PlaneNormal.normalize();
				double SweepAngle;
				pFndSwpAngGivPlnAnd3Lns(PlaneNormal, m_FixupModeFirstLine.endPoint(), IntersectionPoint, m_FixupModeReferenceLine.startPoint(), CenterPoint, SweepAngle);
				const auto MajorAxis {m_FixupModeFirstLine.endPoint() - CenterPoint};

				auto Group {new EoDbGroup};
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

void AeSysView::OnFixupModeMend(void) {
	AeSysDoc* Document = GetDocument();

	OdGePoint3d IntersectionPoint;
	OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

	auto OtherGroup {SelectGroupAndPrimitive(GetCursorPosition())};
	if (OtherGroup == nullptr) {
		return;
	}
	EoDbPrimitive* OtherPrimitive = EngagedPrimitive();
	if (!OtherPrimitive->Is(EoDb::kLinePrimitive)) {
		theApp.AddStringToMessageList(L"Mending only supported in line primitives");
		return;
	}
	EoDbLine* pLine = static_cast<EoDbLine*>(OtherPrimitive);

	pLine->GetLine(m_FixupModeSecondLine);

	if (PreviousFixupCommand == 0) {
		PreviousGroup = OtherGroup;
		PreviousPrimitive = OtherPrimitive;
		m_FixupModeFirstLine.set(m_FixupModeSecondLine.startPoint(), m_FixupModeSecondLine.endPoint());
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP2);
	} else if (PreviousFixupCommand == ID_OP1) {
		if (!m_FixupModeReferenceLine.intersectWith(m_FixupModeSecondLine, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Unable to determine intersection with reference line");
			theApp.AddModeInformationToMessageList();
			return;
		}
		Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, OtherGroup);
		if ((IntersectionPoint - pLine->StartPoint()).length() < (IntersectionPoint - pLine->EndPoint()).length())
			pLine->SetStartPoint2(IntersectionPoint);
		else
			pLine->SetEndPoint2(IntersectionPoint);
		Document->UpdateGroupInAllViews(EoDb::kGroupSafe, OtherGroup);
	} else {
		if (!m_FixupModeFirstLine.intersectWith(m_FixupModeSecondLine, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Selected lines do not define an intersection");
			theApp.AddModeInformationToMessageList();
			return;
		}
		if (PreviousFixupCommand == ID_OP2) {
			pLine = static_cast<EoDbLine*>(PreviousPrimitive);
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
			if ((IntersectionPoint - pLine->StartPoint()).length() < (IntersectionPoint - pLine->EndPoint()).length())
				pLine->SetStartPoint2(IntersectionPoint);
			else
				pLine->SetEndPoint2(IntersectionPoint);
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
		} else if (PreviousFixupCommand == ID_OP3) {
			if ((IntersectionPoint - m_FixupModeFirstLine.startPoint()).length() < (IntersectionPoint - m_FixupModeFirstLine.endPoint()).length())
				m_FixupModeFirstLine.SetStartPoint(m_FixupModeFirstLine.endPoint());
			m_FixupModeFirstLine.SetEndPoint(IntersectionPoint);
			if ((IntersectionPoint - m_FixupModeSecondLine.endPoint()).length() < (IntersectionPoint - m_FixupModeSecondLine.startPoint()).length())
				m_FixupModeSecondLine.SetEndPoint(m_FixupModeSecondLine.startPoint());
			m_FixupModeSecondLine.SetStartPoint(IntersectionPoint);
			OdGePoint3d	ptCP;
			if (pFndCPGivRadAnd4Pts(m_FixupModeCornerSize, m_FixupModeFirstLine.startPoint(), m_FixupModeFirstLine.endPoint(), m_FixupModeSecondLine.startPoint(), m_FixupModeSecondLine.endPoint(), &ptCP)) {
				pLine = dynamic_cast<EoDbLine*>(PreviousPrimitive);
				m_FixupModeFirstLine.SetEndPoint(m_FixupModeFirstLine.ProjPt(ptCP));
				m_FixupModeSecondLine.SetStartPoint(m_FixupModeSecondLine.ProjPt(ptCP));
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				pLine->SetStartPoint2(m_FixupModeFirstLine.startPoint());
				pLine->SetEndPoint2(m_FixupModeFirstLine.endPoint());

				auto Line {EoDbLine::Create(BlockTableRecord, m_FixupModeFirstLine.endPoint(), m_FixupModeSecondLine.startPoint())};
				Line->setColorIndex(pLine->ColorIndex());
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pLine->LinetypeIndex()));
				PreviousGroup->AddTail(EoDbLine::Create(Line));

				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			}
		} else if (PreviousFixupCommand == ID_OP4) {
			if ((IntersectionPoint - m_FixupModeFirstLine.startPoint()).length() < (IntersectionPoint - m_FixupModeFirstLine.endPoint()).length())
				m_FixupModeFirstLine.SetStartPoint(m_FixupModeFirstLine.endPoint());
			m_FixupModeFirstLine.SetEndPoint(IntersectionPoint);
			if ((IntersectionPoint - m_FixupModeSecondLine.endPoint()).length() < (IntersectionPoint - m_FixupModeSecondLine.startPoint()).length())
				m_FixupModeSecondLine.SetEndPoint(m_FixupModeSecondLine.startPoint());
			m_FixupModeSecondLine.SetStartPoint(IntersectionPoint);
			OdGePoint3d	CenterPoint;
			if (pFndCPGivRadAnd4Pts(m_FixupModeCornerSize, m_FixupModeFirstLine.startPoint(), m_FixupModeFirstLine.endPoint(), m_FixupModeSecondLine.startPoint(), m_FixupModeSecondLine.endPoint(), &CenterPoint)) {
				pLine = static_cast<EoDbLine*>(PreviousPrimitive);
				m_FixupModeFirstLine.SetEndPoint(m_FixupModeFirstLine.ProjPt(CenterPoint));
				m_FixupModeSecondLine.SetStartPoint(m_FixupModeSecondLine.ProjPt(CenterPoint));
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				pLine->SetStartPoint2(m_FixupModeFirstLine.startPoint());
				pLine->SetEndPoint2(m_FixupModeFirstLine.endPoint());
				const auto rPrvEndInter {IntersectionPoint - m_FixupModeFirstLine.endPoint()};
				const auto rPrvEndSecBeg {m_FixupModeSecondLine.startPoint() - m_FixupModeFirstLine.endPoint()};
				auto PlaneNormal {rPrvEndInter.crossProduct(rPrvEndSecBeg)};
				PlaneNormal.normalize();
				double SweepAngle;
				pFndSwpAngGivPlnAnd3Lns(PlaneNormal, m_FixupModeFirstLine.endPoint(), IntersectionPoint, m_FixupModeSecondLine.startPoint(), CenterPoint, SweepAngle);
				const auto MajorAxis {m_FixupModeFirstLine.endPoint() - CenterPoint};

				auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
				Ellipse->set(CenterPoint, PlaneNormal, MajorAxis, 1., 0., SweepAngle);

				PreviousGroup->AddTail(EoDbEllipse::Create(Ellipse));
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			}
		}
		Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, OtherGroup);
		pLine = static_cast<EoDbLine*>(OtherPrimitive);
		if ((IntersectionPoint - pLine->StartPoint()).length() < (IntersectionPoint - pLine->EndPoint()).length())
			pLine->SetStartPoint2(IntersectionPoint);
		else
			pLine->SetEndPoint2(IntersectionPoint);
		Document->UpdateGroupInAllViews(EoDb::kGroupSafe, OtherGroup);
		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}
void AeSysView::OnFixupModeChamfer(void) {
	AeSysDoc* Document = GetDocument();

	const auto CurrentPnt {GetCursorPosition()};
	OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

	OdGePoint3d IntersectionPoint;
	OdGePoint3d	ptCP;

	EoDbLine* pLine;

	auto OtherGroup {SelectGroupAndPrimitive(CurrentPnt)};
	if (OtherGroup == nullptr) {
		return;
	}
	EoDbPrimitive* OtherPrimitive = EngagedPrimitive();
	pLine = static_cast<EoDbLine*>(OtherPrimitive);
	pLine->GetLine(m_FixupModeSecondLine);

	if (PreviousFixupCommand == 0) {
		PreviousGroup = OtherGroup;
		PreviousPrimitive = OtherPrimitive;
		m_FixupModeFirstLine = m_FixupModeSecondLine;
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP3);
	} else {
		if (PreviousFixupCommand == ID_OP1) {
			PreviousGroup = OtherGroup;
			PreviousPrimitive = OtherPrimitive;
			m_FixupModeFirstLine = m_FixupModeReferenceLine;
		}
		if (!m_FixupModeFirstLine.intersectWith(m_FixupModeSecondLine, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Selected lines do not define an intersection");
			theApp.AddModeInformationToMessageList();
			return;
		}
		if ((IntersectionPoint - m_FixupModeFirstLine.startPoint()).length() < (IntersectionPoint - m_FixupModeFirstLine.endPoint()).length())
			m_FixupModeFirstLine.SetStartPoint(m_FixupModeFirstLine.endPoint());
		m_FixupModeFirstLine.SetEndPoint(IntersectionPoint);
		if ((IntersectionPoint - m_FixupModeSecondLine.endPoint()).length() < (IntersectionPoint - m_FixupModeSecondLine.startPoint()).length())
			m_FixupModeSecondLine.SetEndPoint(m_FixupModeSecondLine.startPoint());
		m_FixupModeSecondLine.SetStartPoint(IntersectionPoint);
		if (pFndCPGivRadAnd4Pts(m_FixupModeCornerSize, m_FixupModeFirstLine.startPoint(), m_FixupModeFirstLine.endPoint(), m_FixupModeSecondLine.startPoint(), m_FixupModeSecondLine.endPoint(), &ptCP)) { // Center point is defined .. determine arc endpoints
			m_FixupModeFirstLine.SetEndPoint(m_FixupModeFirstLine.ProjPt(ptCP));
			m_FixupModeSecondLine.SetStartPoint(m_FixupModeSecondLine.ProjPt(ptCP));
			if (PreviousFixupCommand == ID_OP1)
				;
			else if (PreviousFixupCommand == ID_OP2) {
				pLine = dynamic_cast<EoDbLine*>(PreviousPrimitive);
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				pLine->SetStartPoint2(m_FixupModeFirstLine.startPoint());
				pLine->SetEndPoint2(IntersectionPoint);
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			} else if (PreviousFixupCommand == ID_OP3 || PreviousFixupCommand == ID_OP4) {
				pLine = dynamic_cast<EoDbLine*>(PreviousPrimitive);
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				pLine->SetStartPoint2(m_FixupModeFirstLine.startPoint());
				pLine->SetEndPoint2(m_FixupModeFirstLine.endPoint());
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			}
			pLine = dynamic_cast<EoDbLine*>(OtherPrimitive);
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, OtherGroup);
			pLine->SetStartPoint2(m_FixupModeSecondLine.startPoint());
			pLine->SetEndPoint2(m_FixupModeSecondLine.endPoint());

			auto Line {EoDbLine::Create(BlockTableRecord, m_FixupModeFirstLine.endPoint(), m_FixupModeSecondLine.startPoint())};
			Line->setColorIndex(pLine->ColorIndex());
			Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pLine->LinetypeIndex()));
			OtherGroup->AddTail(EoDbLine::Create(Line));

			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, OtherGroup);
		}
		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}
void AeSysView::OnFixupModeFillet(void) {
	AeSysDoc* Document = GetDocument();

	const auto CurrentPnt {GetCursorPosition()};
	OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

	OdGePoint3d IntersectionPoint;

	EoDbLine* pLine;

	auto OtherGroup {SelectGroupAndPrimitive(CurrentPnt)};
	EoDbPrimitive* OtherPrimitive = EngagedPrimitive();
	pLine = dynamic_cast<EoDbLine*>(OtherPrimitive);
	pLine->GetLine(m_FixupModeSecondLine);

	if (PreviousFixupCommand == 0) {
		PreviousGroup = OtherGroup;
		PreviousPrimitive = OtherPrimitive;
		m_FixupModeFirstLine = m_FixupModeSecondLine;
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP3);
	} else {
		if (PreviousFixupCommand == ID_OP1) {
			PreviousGroup = OtherGroup;
			PreviousPrimitive = OtherPrimitive;
			m_FixupModeFirstLine = m_FixupModeReferenceLine;
		}

		m_FixupModeFirstLine.IntersectWithInfinite(m_FixupModeSecondLine, IntersectionPoint);

		if (!m_FixupModeFirstLine.intersectWith(m_FixupModeSecondLine, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Selected lines do not define an intersection");
			theApp.AddModeInformationToMessageList();
			return;
		}
		if ((IntersectionPoint - m_FixupModeFirstLine.startPoint()).length() < (IntersectionPoint - m_FixupModeFirstLine.endPoint()).length())
			m_FixupModeFirstLine.SetStartPoint(m_FixupModeFirstLine.endPoint());
		m_FixupModeFirstLine.SetEndPoint(IntersectionPoint);
		if ((IntersectionPoint - m_FixupModeSecondLine.endPoint()).length() < (IntersectionPoint - m_FixupModeSecondLine.startPoint()).length())
			m_FixupModeSecondLine.SetEndPoint(m_FixupModeSecondLine.startPoint());
		m_FixupModeSecondLine.SetStartPoint(IntersectionPoint);
		OdGePoint3d	CenterPoint;
		if (pFndCPGivRadAnd4Pts(m_FixupModeCornerSize, m_FixupModeFirstLine.startPoint(), m_FixupModeFirstLine.endPoint(), m_FixupModeSecondLine.startPoint(), m_FixupModeSecondLine.endPoint(), &CenterPoint)) {
			m_FixupModeFirstLine.SetEndPoint(m_FixupModeFirstLine.ProjPt(CenterPoint));
			m_FixupModeSecondLine.SetStartPoint(m_FixupModeSecondLine.ProjPt(CenterPoint));
			if (PreviousFixupCommand == ID_OP1)
				;
			else if (PreviousFixupCommand == ID_OP2) {
				pLine = dynamic_cast<EoDbLine*>(PreviousPrimitive);
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				pLine->SetStartPoint2(m_FixupModeFirstLine.startPoint());
				pLine->SetEndPoint2(IntersectionPoint);
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			} else if (PreviousFixupCommand == ID_OP3 || PreviousFixupCommand == ID_OP4) {
				pLine = dynamic_cast<EoDbLine*>(PreviousPrimitive);
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				pLine->SetStartPoint2(m_FixupModeFirstLine.startPoint());
				pLine->SetEndPoint2(m_FixupModeFirstLine.endPoint());
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			}
			pLine = dynamic_cast<EoDbLine*>(OtherPrimitive);
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, OtherGroup);
			pLine->SetStartPoint2(m_FixupModeSecondLine.startPoint());
			pLine->SetEndPoint2(m_FixupModeSecondLine.endPoint());

			double SweepAngle;
			const auto rPrvEndInter {IntersectionPoint - m_FixupModeFirstLine.endPoint()};
			const auto rPrvEndSecBeg {m_FixupModeSecondLine.startPoint() - m_FixupModeFirstLine.endPoint()};
			auto PlaneNormal {rPrvEndInter.crossProduct(rPrvEndSecBeg)};
			PlaneNormal.normalize();
			pFndSwpAngGivPlnAnd3Lns(PlaneNormal, m_FixupModeFirstLine.endPoint(), IntersectionPoint, m_FixupModeSecondLine.startPoint(), CenterPoint, SweepAngle);
			const auto MajorAxis {m_FixupModeFirstLine.endPoint() - CenterPoint};

			auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
			Ellipse->set(CenterPoint, PlaneNormal, MajorAxis, 1., 0., SweepAngle);

			OtherGroup->AddTail(EoDbEllipse::Create(Ellipse));

			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, OtherGroup);
		}
		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}
void AeSysView::OnFixupModeSquare(void) {
	AeSysDoc* Document = GetDocument();

	OdGePoint3d CurrentPnt = GetCursorPosition();

	EoDbLine* pLine;
	auto OtherGroup {SelectGroupAndPrimitive(CurrentPnt)};
	if (OtherGroup != nullptr) {
		EoDbPrimitive* OtherPrimitive = EngagedPrimitive();
		CurrentPnt = DetPt();
		if (OtherPrimitive->Is(EoDb::kLinePrimitive)) {
			pLine = static_cast<EoDbLine*>(OtherPrimitive);
			pLine->GetLine(m_FixupModeSecondLine);
			const double dLen = m_FixupModeSecondLine.length();
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, OtherGroup);
			m_FixupModeSecondLine.SetStartPoint(SnapPointToAxis(CurrentPnt, m_FixupModeSecondLine.startPoint()));
			const OdGePoint3d StartPoint = m_FixupModeSecondLine.startPoint();
			m_FixupModeSecondLine.SetEndPoint(ProjectToward(StartPoint, CurrentPnt, dLen));
			pLine->SetStartPoint2(SnapPointToGrid(m_FixupModeSecondLine.startPoint()));
			pLine->SetEndPoint2(SnapPointToGrid(m_FixupModeSecondLine.endPoint()));
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, OtherGroup);
		}
	}
}
void AeSysView::OnFixupModeParallel(void) {
	AeSysDoc* Document = GetDocument();

	const auto CurrentPnt {GetCursorPosition()};

	EoDbLine* pLine;
	auto OtherGroup {SelectGroupAndPrimitive(CurrentPnt)};
	if (ReferenceGroup != nullptr && OtherGroup != nullptr) {
		EoDbPrimitive* OtherPrimitive = EngagedPrimitive();
		if (OtherPrimitive->Is(EoDb::kLinePrimitive)) {
			pLine = static_cast<EoDbLine*>(OtherPrimitive);

			m_FixupModeSecondLine.set(m_FixupModeReferenceLine.ProjPt(pLine->StartPoint()), m_FixupModeReferenceLine.ProjPt(pLine->EndPoint()));
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, OtherGroup);
			const OdGePoint3d StartPoint = m_FixupModeSecondLine.startPoint();
			pLine->SetStartPoint2(ProjectToward(StartPoint, pLine->StartPoint(), theApp.DimensionLength()));
			const OdGePoint3d EndPoint = m_FixupModeSecondLine.endPoint();
			pLine->SetEndPoint2(ProjectToward(EndPoint, pLine->EndPoint(), theApp.DimensionLength()));
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, OtherGroup);
		}
	}
}

void AeSysView::OnFixupModeReturn() {
	AeSysDoc* Document = GetDocument();

	if (ReferenceGroup != nullptr) {
		Document->UpdatePrimitiveInAllViews(EoDb::kPrimitive, ReferencePrimitive);
		ReferenceGroup = nullptr;
		ReferencePrimitive = nullptr;
	}
	ModeLineUnhighlightOp(PreviousFixupCommand);
}

void AeSysView::OnFixupModeEscape() {
	AeSysDoc* Document = GetDocument();

	if (ReferenceGroup != nullptr) {
		Document->UpdatePrimitiveInAllViews(EoDb::kPrimitive, ReferencePrimitive);
		ReferenceGroup = nullptr;
		ReferencePrimitive = nullptr;
	}
	ModeLineUnhighlightOp(PreviousFixupCommand);
}
