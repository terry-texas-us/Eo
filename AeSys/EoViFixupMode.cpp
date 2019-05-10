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

	auto CurrentPnt {GetCursorPosition()};

	OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};

	OdGePoint3d IntersectionPoint;

	if (ReferenceGroup != nullptr) {
		Document->UpdatePrimitiveInAllViews(EoDb::kPrimitive, ReferencePrimitive);
	}
	auto Selection {SelectLineUsingPoint(CurrentPnt)};
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
		
		if (!m_FirstLineSeg.intersectWith(m_ReferenceLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Unable to determine intersection with reference line");
			theApp.AddModeInformationToMessageList();
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
			
			if ((IntersectionPoint - m_FirstLineSeg.startPoint()).length() < (IntersectionPoint - m_FirstLineSeg.endPoint()).length()) {
				m_FirstLineSeg.SetStartPoint(m_FirstLineSeg.endPoint());
			}
			m_FirstLineSeg.SetEndPoint(IntersectionPoint);
			
			if ((IntersectionPoint - m_ReferenceLineSeg.endPoint()).length() < (IntersectionPoint - m_ReferenceLineSeg.startPoint()).length()) {
				m_ReferenceLineSeg.SetEndPoint(m_ReferenceLineSeg.startPoint());
			}
			m_ReferenceLineSeg.SetStartPoint(IntersectionPoint);
			OdGePoint3d	ptCP;
			
			if (pFndCPGivRadAnd4Pts(m_CornerSize, m_FirstLineSeg.startPoint(), m_FirstLineSeg.endPoint(), m_ReferenceLineSeg.startPoint(), m_ReferenceLineSeg.endPoint(), &ptCP)) {
				m_FirstLineSeg.SetEndPoint(m_FirstLineSeg.ProjPt(ptCP));
				m_ReferenceLineSeg.SetStartPoint(m_ReferenceLineSeg.ProjPt(ptCP));
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				PreviousLine->SetStartPoint2(m_FirstLineSeg.startPoint());
				PreviousLine->SetEndPoint2(m_FirstLineSeg.endPoint());

				auto Line {EoDbLine::Create(BlockTableRecord, m_FirstLineSeg.endPoint(), m_ReferenceLineSeg.startPoint())};
				Line->setColorIndex(PreviousLine->ColorIndex());
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(PreviousLine->LinetypeIndex()));
				PreviousGroup->AddTail(EoDbLine::Create(Line));

				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			}
		} else if (PreviousFixupCommand == ID_OP4) {
			
			if ((IntersectionPoint - m_FirstLineSeg.startPoint()).length() < (IntersectionPoint - m_FirstLineSeg.endPoint()).length()) {
				m_FirstLineSeg.SetStartPoint(m_FirstLineSeg.endPoint());
			}
			m_FirstLineSeg.SetEndPoint(IntersectionPoint);
			
			if ((IntersectionPoint - m_ReferenceLineSeg.endPoint()).length() < (IntersectionPoint - m_ReferenceLineSeg.startPoint()).length()) {
				m_ReferenceLineSeg.SetEndPoint(m_ReferenceLineSeg.startPoint());
			}
			m_ReferenceLineSeg.SetStartPoint(IntersectionPoint);
			OdGePoint3d	CenterPoint;
			
			if (pFndCPGivRadAnd4Pts(m_CornerSize, m_FirstLineSeg.startPoint(), m_FirstLineSeg.endPoint(), m_ReferenceLineSeg.startPoint(), m_ReferenceLineSeg.endPoint(), &CenterPoint)) {
				m_FirstLineSeg.SetEndPoint(m_FirstLineSeg.ProjPt(CenterPoint));
				m_ReferenceLineSeg.SetStartPoint(m_ReferenceLineSeg.ProjPt(CenterPoint));

				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				PreviousLine->SetStartPoint2(m_FirstLineSeg.startPoint());
				PreviousLine->SetEndPoint2(m_FirstLineSeg.endPoint());
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);

				const auto rPrvEndInter {IntersectionPoint - m_FirstLineSeg.endPoint()};
				const auto rPrvEndRefBeg {m_ReferenceLineSeg.startPoint() - m_FirstLineSeg.endPoint()};
				auto PlaneNormal {rPrvEndInter.crossProduct(rPrvEndRefBeg)};
				PlaneNormal.normalize();
				double SweepAngle;
				pFndSwpAngGivPlnAnd3Lns(PlaneNormal, m_FirstLineSeg.endPoint(), IntersectionPoint, m_ReferenceLineSeg.startPoint(), CenterPoint, SweepAngle);
				const auto MajorAxis {m_FirstLineSeg.endPoint() - CenterPoint};

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

void AeSysView::OnFixupModeMend() {
	auto Document {GetDocument()};

	OdGePoint3d IntersectionPoint;
	OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};

	auto OtherGroup {SelectGroupAndPrimitive(GetCursorPosition())};
	if (OtherGroup == nullptr) {
		return;
	}
	auto OtherPrimitive {EngagedPrimitive()};

	if (!OtherPrimitive->Is(EoDb::kLinePrimitive)) {
		theApp.AddStringToMessageList(L"Mending only supported in line primitives");
		return;
	}
	auto pLine {static_cast<EoDbLine*>(OtherPrimitive)};

	m_SecondLineSeg = pLine->LineSeg();

	if (PreviousFixupCommand == 0) {
		PreviousGroup = OtherGroup;
		PreviousPrimitive = OtherPrimitive;
		m_FirstLineSeg.set(m_SecondLineSeg.startPoint(), m_SecondLineSeg.endPoint());
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP2);
	} else if (PreviousFixupCommand == ID_OP1) {
		if (!m_ReferenceLineSeg.intersectWith(m_SecondLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Unable to determine intersection with reference line");
			theApp.AddModeInformationToMessageList();
			return;
		}
		Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, OtherGroup);

		if ((IntersectionPoint - pLine->StartPoint()).length() < (IntersectionPoint - pLine->EndPoint()).length()) {
			pLine->SetStartPoint2(IntersectionPoint);
		} else {
			pLine->SetEndPoint2(IntersectionPoint);
		}
		Document->UpdateGroupInAllViews(EoDb::kGroupSafe, OtherGroup);
	} else {
		
		if (!m_FirstLineSeg.intersectWith(m_SecondLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Selected lines do not define an intersection");
			theApp.AddModeInformationToMessageList();
			return;
		}
		if (PreviousFixupCommand == ID_OP2) {
			pLine = static_cast<EoDbLine*>(PreviousPrimitive);
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
			if ((IntersectionPoint - pLine->StartPoint()).length() < (IntersectionPoint - pLine->EndPoint()).length()) {
				pLine->SetStartPoint2(IntersectionPoint);
			} else {
				pLine->SetEndPoint2(IntersectionPoint);
			}
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
		} else if (PreviousFixupCommand == ID_OP3) {
			
			if ((IntersectionPoint - m_FirstLineSeg.startPoint()).length() < (IntersectionPoint - m_FirstLineSeg.endPoint()).length()) {
				m_FirstLineSeg.SetStartPoint(m_FirstLineSeg.endPoint());
			}
			m_FirstLineSeg.SetEndPoint(IntersectionPoint);
			
			if ((IntersectionPoint - m_SecondLineSeg.endPoint()).length() < (IntersectionPoint - m_SecondLineSeg.startPoint()).length()) {
				m_SecondLineSeg.SetEndPoint(m_SecondLineSeg.startPoint());
			}
			m_SecondLineSeg.SetStartPoint(IntersectionPoint);
			OdGePoint3d	ptCP;
			
			if (pFndCPGivRadAnd4Pts(m_CornerSize, m_FirstLineSeg.startPoint(), m_FirstLineSeg.endPoint(), m_SecondLineSeg.startPoint(), m_SecondLineSeg.endPoint(), &ptCP)) {
				pLine = dynamic_cast<EoDbLine*>(PreviousPrimitive);
				m_FirstLineSeg.SetEndPoint(m_FirstLineSeg.ProjPt(ptCP));
				m_SecondLineSeg.SetStartPoint(m_SecondLineSeg.ProjPt(ptCP));
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				pLine->SetStartPoint2(m_FirstLineSeg.startPoint());
				pLine->SetEndPoint2(m_FirstLineSeg.endPoint());

				auto Line {EoDbLine::Create(BlockTableRecord, m_FirstLineSeg.endPoint(), m_SecondLineSeg.startPoint())};
				Line->setColorIndex(pLine->ColorIndex());
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pLine->LinetypeIndex()));
				PreviousGroup->AddTail(EoDbLine::Create(Line));

				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			}
		} else if (PreviousFixupCommand == ID_OP4) {
			
			if ((IntersectionPoint - m_FirstLineSeg.startPoint()).length() < (IntersectionPoint - m_FirstLineSeg.endPoint()).length()) {
				m_FirstLineSeg.SetStartPoint(m_FirstLineSeg.endPoint());
			}
			m_FirstLineSeg.SetEndPoint(IntersectionPoint);
			
			if ((IntersectionPoint - m_SecondLineSeg.endPoint()).length() < (IntersectionPoint - m_SecondLineSeg.startPoint()).length()) {
				m_SecondLineSeg.SetEndPoint(m_SecondLineSeg.startPoint());
			}
			m_SecondLineSeg.SetStartPoint(IntersectionPoint);
			OdGePoint3d	CenterPoint;
			
			if (pFndCPGivRadAnd4Pts(m_CornerSize, m_FirstLineSeg.startPoint(), m_FirstLineSeg.endPoint(), m_SecondLineSeg.startPoint(), m_SecondLineSeg.endPoint(), &CenterPoint)) {
				pLine = static_cast<EoDbLine*>(PreviousPrimitive);
				m_FirstLineSeg.SetEndPoint(m_FirstLineSeg.ProjPt(CenterPoint));
				m_SecondLineSeg.SetStartPoint(m_SecondLineSeg.ProjPt(CenterPoint));
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				pLine->SetStartPoint2(m_FirstLineSeg.startPoint());
				pLine->SetEndPoint2(m_FirstLineSeg.endPoint());
				const auto rPrvEndInter {IntersectionPoint - m_FirstLineSeg.endPoint()};
				const auto rPrvEndSecBeg {m_SecondLineSeg.startPoint() - m_FirstLineSeg.endPoint()};
				auto PlaneNormal {rPrvEndInter.crossProduct(rPrvEndSecBeg)};
				PlaneNormal.normalize();
				double SweepAngle;
				pFndSwpAngGivPlnAnd3Lns(PlaneNormal, m_FirstLineSeg.endPoint(), IntersectionPoint, m_SecondLineSeg.startPoint(), CenterPoint, SweepAngle);
				const auto MajorAxis {m_FirstLineSeg.endPoint() - CenterPoint};

				auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
				Ellipse->set(CenterPoint, PlaneNormal, MajorAxis, 1., 0., SweepAngle);

				PreviousGroup->AddTail(EoDbEllipse::Create(Ellipse));
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			}
		}
		Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, OtherGroup);
		pLine = static_cast<EoDbLine*>(OtherPrimitive);
		
		if ((IntersectionPoint - pLine->StartPoint()).length() < (IntersectionPoint - pLine->EndPoint()).length()) {
			pLine->SetStartPoint2(IntersectionPoint);
		} else {
			pLine->SetEndPoint2(IntersectionPoint);
		}
		Document->UpdateGroupInAllViews(EoDb::kGroupSafe, OtherGroup);
		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeChamfer() {
	auto Document {GetDocument()};

	const auto CurrentPnt {GetCursorPosition()};
	OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};

	OdGePoint3d IntersectionPoint;
	OdGePoint3d	ptCP;

	EoDbLine* pLine;

	auto OtherGroup {SelectGroupAndPrimitive(CurrentPnt)};
	if (OtherGroup == nullptr) {
		return;
	}
	EoDbPrimitive* OtherPrimitive = EngagedPrimitive();
	pLine = static_cast<EoDbLine*>(OtherPrimitive);
	m_SecondLineSeg = pLine->LineSeg();

	if (PreviousFixupCommand == 0) {
		PreviousGroup = OtherGroup;
		PreviousPrimitive = OtherPrimitive;
		m_FirstLineSeg = m_SecondLineSeg;
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP3);
	} else {
		
		if (PreviousFixupCommand == ID_OP1) {
			PreviousGroup = OtherGroup;
			PreviousPrimitive = OtherPrimitive;
			m_FirstLineSeg = m_ReferenceLineSeg;
		}
		if (!m_FirstLineSeg.intersectWith(m_SecondLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Selected lines do not define an intersection");
			theApp.AddModeInformationToMessageList();
			return;
		}
		if ((IntersectionPoint - m_FirstLineSeg.startPoint()).length() < (IntersectionPoint - m_FirstLineSeg.endPoint()).length()) {
			m_FirstLineSeg.SetStartPoint(m_FirstLineSeg.endPoint());
		}
		m_FirstLineSeg.SetEndPoint(IntersectionPoint);
		
		if ((IntersectionPoint - m_SecondLineSeg.endPoint()).length() < (IntersectionPoint - m_SecondLineSeg.startPoint()).length()) {
			m_SecondLineSeg.SetEndPoint(m_SecondLineSeg.startPoint());
		}
		m_SecondLineSeg.SetStartPoint(IntersectionPoint);
		
		if (pFndCPGivRadAnd4Pts(m_CornerSize, m_FirstLineSeg.startPoint(), m_FirstLineSeg.endPoint(), m_SecondLineSeg.startPoint(), m_SecondLineSeg.endPoint(), &ptCP)) { // Center point is defined .. determine arc endpoints
			m_FirstLineSeg.SetEndPoint(m_FirstLineSeg.ProjPt(ptCP));
			m_SecondLineSeg.SetStartPoint(m_SecondLineSeg.ProjPt(ptCP));
			
			if (PreviousFixupCommand == ID_OP1)
				;
			else if (PreviousFixupCommand == ID_OP2) {
				pLine = dynamic_cast<EoDbLine*>(PreviousPrimitive);
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				pLine->SetStartPoint2(m_FirstLineSeg.startPoint());
				pLine->SetEndPoint2(IntersectionPoint);
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			} else if (PreviousFixupCommand == ID_OP3 || PreviousFixupCommand == ID_OP4) {
				pLine = dynamic_cast<EoDbLine*>(PreviousPrimitive);
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				pLine->SetStartPoint2(m_FirstLineSeg.startPoint());
				pLine->SetEndPoint2(m_FirstLineSeg.endPoint());
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			}
			pLine = dynamic_cast<EoDbLine*>(OtherPrimitive);
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, OtherGroup);
			pLine->SetStartPoint2(m_SecondLineSeg.startPoint());
			pLine->SetEndPoint2(m_SecondLineSeg.endPoint());

			auto Line {EoDbLine::Create(BlockTableRecord, m_FirstLineSeg.endPoint(), m_SecondLineSeg.startPoint())};
			Line->setColorIndex(pLine->ColorIndex());
			Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pLine->LinetypeIndex()));
			OtherGroup->AddTail(EoDbLine::Create(Line));

			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, OtherGroup);
		}
		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeFillet() {
	auto Document {GetDocument()};

	const auto CurrentPnt {GetCursorPosition()};
	OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};

	auto Selection {SelectLineUsingPoint(CurrentPnt)};
	auto OtherGroup {std::get<0>(Selection)};
	
	if (OtherGroup == nullptr) { return; }

	auto OtherPrimitive {std::get<1>(Selection)};
	
	auto pLine {dynamic_cast<EoDbLine*>(OtherPrimitive)};
	m_SecondLineSeg = pLine->LineSeg();

	if (PreviousFixupCommand == 0) {
		PreviousGroup = OtherGroup;
		PreviousPrimitive = OtherPrimitive;
		m_FirstLineSeg = m_SecondLineSeg;
		PreviousFixupCommand = ModeLineHighlightOp(ID_OP4);
	} else {
		OdGePoint3d IntersectionPoint;

		if (PreviousFixupCommand == ID_OP1) {
			PreviousGroup = OtherGroup;
			PreviousPrimitive = OtherPrimitive;
			m_FirstLineSeg = m_ReferenceLineSeg;
		}
		if (!m_FirstLineSeg.intersectWith(m_SecondLineSeg, IntersectionPoint)) {
			theApp.AddStringToMessageList(L"Selected lines do not define an intersection");
			theApp.AddModeInformationToMessageList();
			return;
		}
		if ((IntersectionPoint - m_FirstLineSeg.startPoint()).length() < (IntersectionPoint - m_FirstLineSeg.endPoint()).length()) {
			m_FirstLineSeg.SetStartPoint(m_FirstLineSeg.endPoint());
		}
		m_FirstLineSeg.SetEndPoint(IntersectionPoint);
		
		if ((IntersectionPoint - m_SecondLineSeg.endPoint()).length() < (IntersectionPoint - m_SecondLineSeg.startPoint()).length()) {
			m_SecondLineSeg.SetEndPoint(m_SecondLineSeg.startPoint());
		}
		m_SecondLineSeg.SetStartPoint(IntersectionPoint);
		OdGePoint3d	CenterPoint;
		
		if (pFndCPGivRadAnd4Pts(m_CornerSize, m_FirstLineSeg.startPoint(), m_FirstLineSeg.endPoint(), m_SecondLineSeg.startPoint(), m_SecondLineSeg.endPoint(), &CenterPoint)) {
			m_FirstLineSeg.SetEndPoint(m_FirstLineSeg.ProjPt(CenterPoint));
			m_SecondLineSeg.SetStartPoint(m_SecondLineSeg.ProjPt(CenterPoint));
			
			if (PreviousFixupCommand == ID_OP1)
				;
			else if (PreviousFixupCommand == ID_OP2) {
				pLine = dynamic_cast<EoDbLine*>(PreviousPrimitive);
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				pLine->SetStartPoint2(m_FirstLineSeg.startPoint());
				pLine->SetEndPoint2(IntersectionPoint);
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			} else if (PreviousFixupCommand == ID_OP3 || PreviousFixupCommand == ID_OP4) {
				pLine = dynamic_cast<EoDbLine*>(PreviousPrimitive);
				Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, PreviousGroup);
				pLine->SetStartPoint2(m_FirstLineSeg.startPoint());
				pLine->SetEndPoint2(m_FirstLineSeg.endPoint());
				Document->UpdateGroupInAllViews(EoDb::kGroupSafe, PreviousGroup);
			}
			pLine = dynamic_cast<EoDbLine*>(OtherPrimitive);
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, OtherGroup);
			pLine->SetStartPoint2(m_SecondLineSeg.startPoint());
			pLine->SetEndPoint2(m_SecondLineSeg.endPoint());

			double SweepAngle;
			const auto rPrvEndInter {IntersectionPoint - m_FirstLineSeg.endPoint()};
			const auto rPrvEndSecBeg {m_SecondLineSeg.startPoint() - m_FirstLineSeg.endPoint()};
			auto PlaneNormal {rPrvEndInter.crossProduct(rPrvEndSecBeg)};
			PlaneNormal.normalize();
			pFndSwpAngGivPlnAnd3Lns(PlaneNormal, m_FirstLineSeg.endPoint(), IntersectionPoint, m_SecondLineSeg.startPoint(), CenterPoint, SweepAngle);
			const auto MajorAxis {m_FirstLineSeg.endPoint() - CenterPoint};

			auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
			Ellipse->set(CenterPoint, PlaneNormal, MajorAxis, 1., 0., SweepAngle);

			OtherGroup->AddTail(EoDbEllipse::Create(Ellipse));

			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, OtherGroup);
		}
		ModeLineUnhighlightOp(PreviousFixupCommand);
	}
}

void AeSysView::OnFixupModeSquare() {
	auto Document {GetDocument()};

	auto CurrentPnt {GetCursorPosition()};

	EoDbLine* pLine;
	auto OtherGroup {SelectGroupAndPrimitive(CurrentPnt)};
	
	if (OtherGroup != nullptr) {
		auto OtherPrimitive {EngagedPrimitive()};
		CurrentPnt = DetPt();
		
		if (OtherPrimitive->Is(EoDb::kLinePrimitive)) {
			pLine = static_cast<EoDbLine*>(OtherPrimitive);
			m_SecondLineSeg = pLine->LineSeg();
			const double dLen = m_SecondLineSeg.length();
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, OtherGroup);
			m_SecondLineSeg.SetStartPoint(SnapPointToAxis(CurrentPnt, m_SecondLineSeg.startPoint()));
			const OdGePoint3d StartPoint = m_SecondLineSeg.startPoint();
			m_SecondLineSeg.SetEndPoint(ProjectToward(StartPoint, CurrentPnt, dLen));
			pLine->SetStartPoint2(SnapPointToGrid(m_SecondLineSeg.startPoint()));
			pLine->SetEndPoint2(SnapPointToGrid(m_SecondLineSeg.endPoint()));
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, OtherGroup);
		}
	}
}

void AeSysView::OnFixupModeParallel() {
	auto Document {GetDocument()};

	const auto CurrentPnt {GetCursorPosition()};

	EoDbLine* pLine;
	auto OtherGroup {SelectGroupAndPrimitive(CurrentPnt)};
	
	if (ReferenceGroup != nullptr && OtherGroup != nullptr) {
		auto OtherPrimitive {EngagedPrimitive()};
		
		if (OtherPrimitive->Is(EoDb::kLinePrimitive)) {
			pLine = static_cast<EoDbLine*>(OtherPrimitive);

			m_SecondLineSeg.set(m_ReferenceLineSeg.ProjPt(pLine->StartPoint()), m_ReferenceLineSeg.ProjPt(pLine->EndPoint()));
			Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, OtherGroup);
			const OdGePoint3d StartPoint = m_SecondLineSeg.startPoint();
			pLine->SetStartPoint2(ProjectToward(StartPoint, pLine->StartPoint(), theApp.DimensionLength()));
			const OdGePoint3d EndPoint = m_SecondLineSeg.endPoint();
			pLine->SetEndPoint2(ProjectToward(EndPoint, pLine->EndPoint(), theApp.DimensionLength()));
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, OtherGroup);
		}
	}
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
