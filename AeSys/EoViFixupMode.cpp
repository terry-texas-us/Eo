#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDlgFixupOptions.h"

OdUInt16	PreviousFixupCommand = 0;

EoDbGroup* ReferenceGroup;
EoDbPrimitive* ReferencePrimitive;

EoDbGroup* pSegPrv;
EoDbPrimitive* pPrimPrv;

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

    auto ptCurPos {GetCursorPosition()};

    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    OdGePoint3d ptInt;

    if (ReferenceGroup != nullptr) {
        Document->UpdatePrimitiveInAllViews(EoDb::kPrimitive, ReferencePrimitive);
    }
    ReferenceGroup = SelectGroupAndPrimitive(ptCurPos);
    if (ReferenceGroup == nullptr) {
        return;
    }
    ReferencePrimitive = EngagedPrimitive();
    if (!ReferencePrimitive->Is(EoDb::kLinePrimitive)) {
        return;
    }
    ptCurPos = DetPt();
    static_cast<EoDbLine*>(ReferencePrimitive)->GetLine(m_FixupModeReferenceLine);

    if (PreviousFixupCommand == 0)
        PreviousFixupCommand = ModeLineHighlightOp(ID_OP1);
    else if (PreviousFixupCommand == ID_OP1)
        ;
    else {
        EoDbLine* pLinePrv = static_cast<EoDbLine*>(pPrimPrv);
        if (!m_FixupModeFirstLine.intersectWith(m_FixupModeReferenceLine, ptInt)) {
            theApp.AddStringToMessageList(L"Unable to determine intersection with reference line");
            theApp.AddModeInformationToMessageList();
            return;
        }
        if (PreviousFixupCommand == ID_OP2) {
            Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, pSegPrv);
            if (OdGeVector3d(ptInt - pLinePrv->EndPoint()).length() < OdGeVector3d(ptInt - pLinePrv->EndPoint()).length())
                pLinePrv->SetStartPoint2(ptInt);
            else
                pLinePrv->SetEndPoint2(ptInt);
            Document->UpdateGroupInAllViews(EoDb::kGroupSafe, pSegPrv);
        } else if (PreviousFixupCommand == ID_OP3) {
            if (OdGeVector3d(ptInt - m_FixupModeFirstLine.startPoint()).length() < OdGeVector3d(ptInt - m_FixupModeFirstLine.endPoint()).length())
                m_FixupModeFirstLine.SetStartPoint(m_FixupModeFirstLine.endPoint());
            m_FixupModeFirstLine.SetEndPoint(ptInt);
            if (OdGeVector3d(ptInt - m_FixupModeReferenceLine.endPoint()).length() < OdGeVector3d(ptInt - m_FixupModeReferenceLine.startPoint()).length())
                m_FixupModeReferenceLine.SetEndPoint(m_FixupModeReferenceLine.startPoint());
            m_FixupModeReferenceLine.SetStartPoint(ptInt);
            OdGePoint3d	ptCP;
            if (pFndCPGivRadAnd4Pts(m_FixupModeCornerSize, m_FixupModeFirstLine.startPoint(), m_FixupModeFirstLine.endPoint(), m_FixupModeReferenceLine.startPoint(), m_FixupModeReferenceLine.endPoint(), &ptCP)) {
                m_FixupModeFirstLine.SetEndPoint(m_FixupModeFirstLine.ProjPt(ptCP));
                m_FixupModeReferenceLine.SetStartPoint(m_FixupModeReferenceLine.ProjPt(ptCP));
                Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, pSegPrv);
                pLinePrv->SetStartPoint2(m_FixupModeFirstLine.startPoint());
                pLinePrv->SetEndPoint2(m_FixupModeFirstLine.endPoint());

                auto Line {EoDbLine::Create(BlockTableRecord, m_FixupModeFirstLine.endPoint(), m_FixupModeReferenceLine.startPoint())};
                Line->setColorIndex(pLinePrv->ColorIndex());
                Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pLinePrv->LinetypeIndex()));
                pSegPrv->AddTail(EoDbLine::Create(Line));

                Document->UpdateGroupInAllViews(EoDb::kGroupSafe, pSegPrv);
            }
        } else if (PreviousFixupCommand == ID_OP4) {
            if (OdGeVector3d(ptInt - m_FixupModeFirstLine.startPoint()).length() < OdGeVector3d(ptInt - m_FixupModeFirstLine.endPoint()).length())
                m_FixupModeFirstLine.SetStartPoint(m_FixupModeFirstLine.endPoint());
            m_FixupModeFirstLine.SetEndPoint(ptInt);
            if (OdGeVector3d(ptInt - m_FixupModeReferenceLine.endPoint()).length() < OdGeVector3d(ptInt - m_FixupModeReferenceLine.startPoint()).length())
                m_FixupModeReferenceLine.SetEndPoint(m_FixupModeReferenceLine.startPoint());
            m_FixupModeReferenceLine.SetStartPoint(ptInt);
            OdGePoint3d	CenterPoint;
            if (pFndCPGivRadAnd4Pts(m_FixupModeCornerSize, m_FixupModeFirstLine.startPoint(), m_FixupModeFirstLine.endPoint(), m_FixupModeReferenceLine.startPoint(), m_FixupModeReferenceLine.endPoint(), &CenterPoint)) {
                m_FixupModeFirstLine.SetEndPoint(m_FixupModeFirstLine.ProjPt(CenterPoint));
                m_FixupModeReferenceLine.SetStartPoint(m_FixupModeReferenceLine.ProjPt(CenterPoint));

                Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, pSegPrv);
                pLinePrv->SetStartPoint2(m_FixupModeFirstLine.startPoint());
                pLinePrv->SetEndPoint2(m_FixupModeFirstLine.endPoint());
                Document->UpdateGroupInAllViews(EoDb::kGroupSafe, pSegPrv);

                const OdGeVector3d rPrvEndInter(ptInt - m_FixupModeFirstLine.endPoint());
                const OdGeVector3d rPrvEndRefBeg(m_FixupModeReferenceLine.startPoint() - m_FixupModeFirstLine.endPoint());
                OdGeVector3d PlaneNormal = rPrvEndInter.crossProduct(rPrvEndRefBeg);
                PlaneNormal.normalize();
                double SweepAngle;
                pFndSwpAngGivPlnAnd3Lns(PlaneNormal, m_FixupModeFirstLine.endPoint(), ptInt, m_FixupModeReferenceLine.startPoint(), CenterPoint, SweepAngle);
                const OdGeVector3d MajorAxis(m_FixupModeFirstLine.endPoint() - CenterPoint);
                OdGePoint3d rTmp = m_FixupModeFirstLine.endPoint();
                rTmp.rotateBy(HALF_PI, PlaneNormal, CenterPoint);
                const OdGeVector3d MinorAxis(rTmp - CenterPoint);

                EoDbGroup * Group = new EoDbGroup;
                EoDbEllipse * Arc = EoDbEllipse::Create0(BlockTableRecord);
                Arc->SetTo2(CenterPoint, MajorAxis, MinorAxis, SweepAngle);
                Group->AddTail(Arc);
                Document->AddWorkLayerGroup(Group);
                Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
            }
        }
        ModeLineUnhighlightOp(PreviousFixupCommand);
    }
}
void AeSysView::OnFixupModeMend(void) {
    AeSysDoc* Document = GetDocument();

    OdGePoint3d ptInt;
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
        pSegPrv = OtherGroup;
        pPrimPrv = OtherPrimitive;
        m_FixupModeFirstLine.set(m_FixupModeSecondLine.startPoint(), m_FixupModeSecondLine.endPoint());
        PreviousFixupCommand = ModeLineHighlightOp(ID_OP2);
    } else if (PreviousFixupCommand == ID_OP1) {
        if (!m_FixupModeReferenceLine.intersectWith(m_FixupModeSecondLine, ptInt)) {
            theApp.AddStringToMessageList(L"Unable to determine intersection with reference line");
            theApp.AddModeInformationToMessageList();
            return;
        }
        Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, OtherGroup);
        if (OdGeVector3d(ptInt - pLine->StartPoint()).length() < OdGeVector3d(ptInt - pLine->EndPoint()).length())
            pLine->SetStartPoint2(ptInt);
        else
            pLine->SetEndPoint2(ptInt);
        Document->UpdateGroupInAllViews(EoDb::kGroupSafe, OtherGroup);
    } else {
        if (!m_FixupModeFirstLine.intersectWith(m_FixupModeSecondLine, ptInt)) {
            theApp.AddStringToMessageList(L"Selected lines do not define an intersection");
            theApp.AddModeInformationToMessageList();
            return;
        }
        if (PreviousFixupCommand == ID_OP2) {
            pLine = static_cast<EoDbLine*>(pPrimPrv);
            Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, pSegPrv);
            if (OdGeVector3d(ptInt - pLine->StartPoint()).length() < OdGeVector3d(ptInt - pLine->EndPoint()).length())
                pLine->SetStartPoint2(ptInt);
            else
                pLine->SetEndPoint2(ptInt);
            Document->UpdateGroupInAllViews(EoDb::kGroupSafe, pSegPrv);
        } else if (PreviousFixupCommand == ID_OP3) {
            if (OdGeVector3d(ptInt - m_FixupModeFirstLine.startPoint()).length() < OdGeVector3d(ptInt - m_FixupModeFirstLine.endPoint()).length())
                m_FixupModeFirstLine.SetStartPoint(m_FixupModeFirstLine.endPoint());
            m_FixupModeFirstLine.SetEndPoint(ptInt);
            if (OdGeVector3d(ptInt - m_FixupModeSecondLine.endPoint()).length() < OdGeVector3d(ptInt - m_FixupModeSecondLine.startPoint()).length())
                m_FixupModeSecondLine.SetEndPoint(m_FixupModeSecondLine.startPoint());
            m_FixupModeSecondLine.SetStartPoint(ptInt);
            OdGePoint3d	ptCP;
            if (pFndCPGivRadAnd4Pts(m_FixupModeCornerSize, m_FixupModeFirstLine.startPoint(), m_FixupModeFirstLine.endPoint(), m_FixupModeSecondLine.startPoint(), m_FixupModeSecondLine.endPoint(), &ptCP)) {
                pLine = static_cast<EoDbLine*>(pPrimPrv);
                m_FixupModeFirstLine.SetEndPoint(m_FixupModeFirstLine.ProjPt(ptCP));
                m_FixupModeSecondLine.SetStartPoint(m_FixupModeSecondLine.ProjPt(ptCP));
                Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, pSegPrv);
                pLine->SetStartPoint2(m_FixupModeFirstLine.startPoint());
                pLine->SetEndPoint2(m_FixupModeFirstLine.endPoint());

                auto Line {EoDbLine::Create(BlockTableRecord, m_FixupModeFirstLine.endPoint(), m_FixupModeSecondLine.startPoint())};
                Line->setColorIndex(pLine->ColorIndex());
                Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pLine->LinetypeIndex()));
                pSegPrv->AddTail(EoDbLine::Create(Line));

                Document->UpdateGroupInAllViews(EoDb::kGroupSafe, pSegPrv);
            }
        } else if (PreviousFixupCommand == ID_OP4) {
            if (OdGeVector3d(ptInt - m_FixupModeFirstLine.startPoint()).length() < OdGeVector3d(ptInt - m_FixupModeFirstLine.endPoint()).length())
                m_FixupModeFirstLine.SetStartPoint(m_FixupModeFirstLine.endPoint());
            m_FixupModeFirstLine.SetEndPoint(ptInt);
            if (OdGeVector3d(ptInt - m_FixupModeSecondLine.endPoint()).length() < OdGeVector3d(ptInt - m_FixupModeSecondLine.startPoint()).length())
                m_FixupModeSecondLine.SetEndPoint(m_FixupModeSecondLine.startPoint());
            m_FixupModeSecondLine.SetStartPoint(ptInt);
            OdGePoint3d	CenterPoint;
            if (pFndCPGivRadAnd4Pts(m_FixupModeCornerSize, m_FixupModeFirstLine.startPoint(), m_FixupModeFirstLine.endPoint(), m_FixupModeSecondLine.startPoint(), m_FixupModeSecondLine.endPoint(), &CenterPoint)) {
                pLine = static_cast<EoDbLine*>(pPrimPrv);
                m_FixupModeFirstLine.SetEndPoint(m_FixupModeFirstLine.ProjPt(CenterPoint));
                m_FixupModeSecondLine.SetStartPoint(m_FixupModeSecondLine.ProjPt(CenterPoint));
                Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, pSegPrv);
                pLine->SetStartPoint2(m_FixupModeFirstLine.startPoint());
                pLine->SetEndPoint2(m_FixupModeFirstLine.endPoint());
                const OdGeVector3d rPrvEndInter(ptInt - m_FixupModeFirstLine.endPoint());
                const OdGeVector3d rPrvEndSecBeg(m_FixupModeSecondLine.startPoint() - m_FixupModeFirstLine.endPoint());
                OdGeVector3d PlaneNormal = rPrvEndInter.crossProduct(rPrvEndSecBeg);
                PlaneNormal.normalize();
                double SweepAngle;
                pFndSwpAngGivPlnAnd3Lns(PlaneNormal, m_FixupModeFirstLine.endPoint(), ptInt, m_FixupModeSecondLine.startPoint(), CenterPoint, SweepAngle);
                const OdGeVector3d MajorAxis(m_FixupModeFirstLine.endPoint() - CenterPoint);
                OdGePoint3d rTmp = m_FixupModeFirstLine.endPoint();
                rTmp.rotateBy(HALF_PI, PlaneNormal, CenterPoint);
                const OdGeVector3d MinorAxis(rTmp - CenterPoint);
                EoDbEllipse * Arc = EoDbEllipse::Create0(BlockTableRecord);
                Arc->SetTo2(CenterPoint, MajorAxis, MinorAxis, SweepAngle);
                pSegPrv->AddTail(Arc);
                Document->UpdateGroupInAllViews(EoDb::kGroupSafe, pSegPrv);
            }
        }
        Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, OtherGroup);
        pLine = static_cast<EoDbLine*>(OtherPrimitive);
        if (OdGeVector3d(ptInt - pLine->StartPoint()).length() < OdGeVector3d(ptInt - pLine->EndPoint()).length())
            pLine->SetStartPoint2(ptInt);
        else
            pLine->SetEndPoint2(ptInt);
        Document->UpdateGroupInAllViews(EoDb::kGroupSafe, OtherGroup);
        ModeLineUnhighlightOp(PreviousFixupCommand);
    }
}
void AeSysView::OnFixupModeChamfer(void) {
    AeSysDoc* Document = GetDocument();

    const auto ptCurPos {GetCursorPosition()};
    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    OdGePoint3d ptInt;
    OdGePoint3d	ptCP;

    EoDbLine* pLine;

    auto OtherGroup {SelectGroupAndPrimitive(ptCurPos)};
    if (OtherGroup == nullptr) {
        return;
    }
    EoDbPrimitive* OtherPrimitive = EngagedPrimitive();
    pLine = static_cast<EoDbLine*>(OtherPrimitive);
    pLine->GetLine(m_FixupModeSecondLine);

    if (PreviousFixupCommand == 0) {
        pSegPrv = OtherGroup;
        pPrimPrv = OtherPrimitive;
        m_FixupModeFirstLine = m_FixupModeSecondLine;
        PreviousFixupCommand = ModeLineHighlightOp(ID_OP3);
    } else {
        if (PreviousFixupCommand == ID_OP1) {
            pSegPrv = OtherGroup;
            pPrimPrv = OtherPrimitive;
            m_FixupModeFirstLine = m_FixupModeReferenceLine;
        }
        if (!m_FixupModeFirstLine.intersectWith(m_FixupModeSecondLine, ptInt)) {
            theApp.AddStringToMessageList(L"Selected lines do not define an intersection");
            theApp.AddModeInformationToMessageList();
            return;
        }
        if (OdGeVector3d(ptInt - m_FixupModeFirstLine.startPoint()).length() < OdGeVector3d(ptInt - m_FixupModeFirstLine.endPoint()).length())
            m_FixupModeFirstLine.SetStartPoint(m_FixupModeFirstLine.endPoint());
        m_FixupModeFirstLine.SetEndPoint(ptInt);
        if (OdGeVector3d(ptInt - m_FixupModeSecondLine.endPoint()).length() < OdGeVector3d(ptInt - m_FixupModeSecondLine.startPoint()).length())
            m_FixupModeSecondLine.SetEndPoint(m_FixupModeSecondLine.startPoint());
        m_FixupModeSecondLine.SetStartPoint(ptInt);
        if (pFndCPGivRadAnd4Pts(m_FixupModeCornerSize, m_FixupModeFirstLine.startPoint(), m_FixupModeFirstLine.endPoint(), m_FixupModeSecondLine.startPoint(), m_FixupModeSecondLine.endPoint(), &ptCP)) { // Center point is defined .. determine arc endpoints
            m_FixupModeFirstLine.SetEndPoint(m_FixupModeFirstLine.ProjPt(ptCP));
            m_FixupModeSecondLine.SetStartPoint(m_FixupModeSecondLine.ProjPt(ptCP));
            if (PreviousFixupCommand == ID_OP1)
                ;
            else if (PreviousFixupCommand == ID_OP2) {
                pLine = dynamic_cast<EoDbLine*>(pPrimPrv);
                Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, pSegPrv);
                pLine->SetStartPoint2(m_FixupModeFirstLine.startPoint());
                pLine->SetEndPoint2(ptInt);
                Document->UpdateGroupInAllViews(EoDb::kGroupSafe, pSegPrv);
            } else if (PreviousFixupCommand == ID_OP3 || PreviousFixupCommand == ID_OP4) {
                pLine = dynamic_cast<EoDbLine*>(pPrimPrv);
                Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, pSegPrv);
                pLine->SetStartPoint2(m_FixupModeFirstLine.startPoint());
                pLine->SetEndPoint2(m_FixupModeFirstLine.endPoint());
                Document->UpdateGroupInAllViews(EoDb::kGroupSafe, pSegPrv);
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

    const auto ptCurPos {GetCursorPosition()};
    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    OdGePoint3d ptInt;

    EoDbLine* pLine;

    auto OtherGroup {SelectGroupAndPrimitive(ptCurPos)};
    EoDbPrimitive* OtherPrimitive = EngagedPrimitive();
    pLine = dynamic_cast<EoDbLine*>(OtherPrimitive);
    pLine->GetLine(m_FixupModeSecondLine);

    if (PreviousFixupCommand == 0) {
        pSegPrv = OtherGroup;
        pPrimPrv = OtherPrimitive;
        m_FixupModeFirstLine = m_FixupModeSecondLine;
        PreviousFixupCommand = ModeLineHighlightOp(ID_OP3);
    } else {
        if (PreviousFixupCommand == ID_OP1) {
            pSegPrv = OtherGroup;
            pPrimPrv = OtherPrimitive;
            m_FixupModeFirstLine = m_FixupModeReferenceLine;
        }
        if (!m_FixupModeFirstLine.intersectWith(m_FixupModeSecondLine, ptInt)) {
            theApp.AddStringToMessageList(L"Selected lines do not define an intersection");
            theApp.AddModeInformationToMessageList();
            return;
        }
        if (OdGeVector3d(ptInt - m_FixupModeFirstLine.startPoint()).length() < OdGeVector3d(ptInt - m_FixupModeFirstLine.endPoint()).length())
            m_FixupModeFirstLine.SetStartPoint(m_FixupModeFirstLine.endPoint());
        m_FixupModeFirstLine.SetEndPoint(ptInt);
        if (OdGeVector3d(ptInt - m_FixupModeSecondLine.endPoint()).length() < OdGeVector3d(ptInt - m_FixupModeSecondLine.startPoint()).length())
            m_FixupModeSecondLine.SetEndPoint(m_FixupModeSecondLine.startPoint());
        m_FixupModeSecondLine.SetStartPoint(ptInt);
        OdGePoint3d	CenterPoint;
        if (pFndCPGivRadAnd4Pts(m_FixupModeCornerSize, m_FixupModeFirstLine.startPoint(), m_FixupModeFirstLine.endPoint(), m_FixupModeSecondLine.startPoint(), m_FixupModeSecondLine.endPoint(), &CenterPoint)) {
            m_FixupModeFirstLine.SetEndPoint(m_FixupModeFirstLine.ProjPt(CenterPoint));
            m_FixupModeSecondLine.SetStartPoint(m_FixupModeSecondLine.ProjPt(CenterPoint));
            if (PreviousFixupCommand == ID_OP1)
                ;
            else if (PreviousFixupCommand == ID_OP2) {
                pLine = dynamic_cast<EoDbLine*>(pPrimPrv);
                Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, pSegPrv);
                pLine->SetStartPoint2(m_FixupModeFirstLine.startPoint());
                pLine->SetEndPoint2(ptInt);
                Document->UpdateGroupInAllViews(EoDb::kGroupSafe, pSegPrv);
            } else if (PreviousFixupCommand == ID_OP3 || PreviousFixupCommand == ID_OP4) {
                pLine = dynamic_cast<EoDbLine*>(pPrimPrv);
                Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, pSegPrv);
                pLine->SetStartPoint2(m_FixupModeFirstLine.startPoint());
                pLine->SetEndPoint2(m_FixupModeFirstLine.endPoint());
                Document->UpdateGroupInAllViews(EoDb::kGroupSafe, pSegPrv);
            }
            pLine = dynamic_cast<EoDbLine*>(OtherPrimitive);
            Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, OtherGroup);
            pLine->SetStartPoint2(m_FixupModeSecondLine.startPoint());
            pLine->SetEndPoint2(m_FixupModeSecondLine.endPoint());

            double SweepAngle;
            const OdGeVector3d rPrvEndInter(ptInt - m_FixupModeFirstLine.endPoint());
            const OdGeVector3d rPrvEndSecBeg(m_FixupModeSecondLine.startPoint() - m_FixupModeFirstLine.endPoint());
            OdGeVector3d PlaneNormal = rPrvEndInter.crossProduct(rPrvEndSecBeg);
            PlaneNormal.normalize();
            pFndSwpAngGivPlnAnd3Lns(PlaneNormal, m_FixupModeFirstLine.endPoint(), ptInt, m_FixupModeSecondLine.startPoint(), CenterPoint, SweepAngle);
            const OdGeVector3d MajorAxis(m_FixupModeFirstLine.endPoint() - CenterPoint);
            OdGePoint3d rTmp = m_FixupModeFirstLine.endPoint();
            rTmp.rotateBy(HALF_PI, PlaneNormal, CenterPoint);
            const OdGeVector3d MinorAxis(rTmp - CenterPoint);
            auto Fillet {EoDbEllipse::Create0(BlockTableRecord)};
            Fillet->SetTo2(CenterPoint, MajorAxis, MinorAxis, SweepAngle);
            OtherGroup->AddTail(Fillet);

            Document->UpdateGroupInAllViews(EoDb::kGroupSafe, OtherGroup);
        }
        ModeLineUnhighlightOp(PreviousFixupCommand);
    }
}
void AeSysView::OnFixupModeSquare(void) {
    AeSysDoc* Document = GetDocument();

    OdGePoint3d ptCurPos = GetCursorPosition();

    EoDbLine* pLine;
    auto OtherGroup {SelectGroupAndPrimitive(ptCurPos)};
    if (OtherGroup != nullptr) {
        EoDbPrimitive* OtherPrimitive = EngagedPrimitive();
        ptCurPos = DetPt();
        if (OtherPrimitive->Is(EoDb::kLinePrimitive)) {
            pLine = static_cast<EoDbLine*>(OtherPrimitive);
            pLine->GetLine(m_FixupModeSecondLine);
            const double dLen = m_FixupModeSecondLine.length();
            Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, OtherGroup);
            m_FixupModeSecondLine.SetStartPoint(SnapPointToAxis(ptCurPos, m_FixupModeSecondLine.startPoint()));
            const OdGePoint3d StartPoint = m_FixupModeSecondLine.startPoint();
            m_FixupModeSecondLine.SetEndPoint(ProjectToward(StartPoint, ptCurPos, dLen));
            pLine->SetStartPoint2(SnapPointToGrid(m_FixupModeSecondLine.startPoint()));
            pLine->SetEndPoint2(SnapPointToGrid(m_FixupModeSecondLine.endPoint()));
            Document->UpdateGroupInAllViews(EoDb::kGroupSafe, OtherGroup);
        }
    }
}
void AeSysView::OnFixupModeParallel(void) {
    AeSysDoc* Document = GetDocument();

    const auto ptCurPos {GetCursorPosition()};

    EoDbLine* pLine;
    auto OtherGroup {SelectGroupAndPrimitive(ptCurPos)};
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
void AeSysView::OnFixupModeReturn(void) {
    AeSysDoc* Document = GetDocument();

    if (ReferenceGroup != nullptr) {
        Document->UpdatePrimitiveInAllViews(EoDb::kPrimitive, ReferencePrimitive);
        ReferenceGroup = nullptr;
        ReferencePrimitive = 0;
    }
    ModeLineUnhighlightOp(PreviousFixupCommand);
}
void AeSysView::OnFixupModeEscape(void) {
    AeSysDoc* Document = GetDocument();

    if (ReferenceGroup != nullptr) {
        Document->UpdatePrimitiveInAllViews(EoDb::kPrimitive, ReferencePrimitive);
        ReferenceGroup = nullptr;
        ReferencePrimitive = 0;
    }
    ModeLineUnhighlightOp(PreviousFixupCommand);
}
