#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDlgPipeOptions.h"
#include "EoDlgPipeSymbol.h"

#include "Ge/GeCircArc3d.h"

void AeSysView::OnPipeModeOptions() {
    EoDlgPipeOptions Dialog;
    Dialog.m_PipeTicSize = m_PipeTicSize;
    Dialog.m_PipeRiseDropRadius = m_PipeRiseDropRadius;
    if (Dialog.DoModal() == IDOK) {
        m_PipeTicSize = Dialog.m_PipeTicSize;
        m_PipeRiseDropRadius = Dialog.m_PipeRiseDropRadius;
    }
}

void AeSysView::OnPipeModeLine() {
    auto CurrentPnt {GetCursorPosition()};

    if (m_PipeModePoints.empty()) {
        m_PipeModePoints.append(CurrentPnt);
    } else {
        CurrentPnt = SnapPointToAxis(m_PipeModePoints[0], CurrentPnt);

        GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        m_PreviewGroup.DeletePrimitivesAndRemoveAll();

        EoDbGroup* Group = new EoDbGroup;
        GetDocument()->AddWorkLayerGroup(Group);
        GenerateLineWithFittings(m_PreviousOp, m_PipeModePoints[0], ID_OP2, CurrentPnt, Group);
        GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, Group);

        m_PipeModePoints[0] = CurrentPnt;
    }
    m_PreviousOp = ModeLineHighlightOp(ID_OP2);
}

void AeSysView::OnPipeModeFitting() {
    auto CurrentPnt {GetCursorPosition()};
    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    EoDbLine* HorizontalSection;
    auto Group {SelectLineBy(CurrentPnt, HorizontalSection)};
    if (Group != nullptr) {
        const auto BeginPoint {HorizontalSection->StartPoint()};
        const auto EndPoint {HorizontalSection->EndPoint()};

        if (!m_PipeModePoints.empty()) {
            CurrentPnt = SnapPointToAxis(m_PipeModePoints[0], CurrentPnt);
        }
        CurrentPnt = HorizontalSection->ProjPt_(CurrentPnt);
        HorizontalSection->SetEndPoint(CurrentPnt);

        auto Line {EoDbLine::Create(BlockTableRecord)};

        Line->setStartPoint(CurrentPnt);
        Line->setEndPoint(EndPoint);
        Line->setColorIndex(HorizontalSection->ColorIndex());
        Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(HorizontalSection->LinetypeIndex()));

        Group->AddTail(EoDbLine::Create(Line));

        Group = new EoDbGroup;
        GenerateTicMark(CurrentPnt, BeginPoint, m_PipeRiseDropRadius, Group);
        GenerateTicMark(CurrentPnt, EndPoint, m_PipeRiseDropRadius, Group);
        GetDocument()->AddWorkLayerGroup(Group);
        GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);

        if (m_PipeModePoints.empty()) {
            m_PipeModePoints.append(CurrentPnt);
            m_PreviousOp = ModeLineHighlightOp(ID_OP3);
        } else {
            GenerateTicMark(CurrentPnt, m_PipeModePoints[0], m_PipeRiseDropRadius, Group);

            Group = new EoDbGroup;
            GenerateLineWithFittings(m_PreviousOp, m_PipeModePoints[0], 0, CurrentPnt, Group);
            GetDocument()->AddWorkLayerGroup(Group);
            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, Group);
            OnPipeModeEscape();
        }
    } else {
        EoDbEllipse* VerticalSection;
        Group = SelectCircleUsingPoint(CurrentPnt, m_PipeRiseDropRadius, VerticalSection);
        if (Group != 0) {
            CurrentPnt = VerticalSection->Center();

            if (m_PipeModePoints.empty()) {
                m_PipeModePoints.append(CurrentPnt);
                m_PreviousOp = ModeLineHighlightOp(ID_OP4);
            } else {
                Group = new EoDbGroup;
                GenerateLineWithFittings(m_PreviousOp, m_PipeModePoints[0], ID_OP5, CurrentPnt, Group);
                GetDocument()->AddWorkLayerGroup(Group);
                GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, Group);
                OnPipeModeEscape();
            }
        } else {
            if (m_PipeModePoints.empty()) {
                m_PipeModePoints.append(CurrentPnt);
            } else {
                CurrentPnt = SnapPointToAxis(m_PipeModePoints[0], CurrentPnt);

                GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
                m_PreviewGroup.DeletePrimitivesAndRemoveAll();

                Group = new EoDbGroup;
                GenerateLineWithFittings(m_PreviousOp, m_PipeModePoints[0], ID_OP3, CurrentPnt, Group);
                GetDocument()->AddWorkLayerGroup(Group);
                GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, Group);

                m_PipeModePoints[0] = CurrentPnt;
            }
            m_PreviousOp = ModeLineHighlightOp(ID_OP3);
        }
    }
}

void AeSysView::OnPipeModeRise() {
    auto CurrentPnt {GetCursorPosition()};
    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    EoDbLine* HorizontalSection;
    EoDbGroup* Group = SelectLineBy(CurrentPnt, HorizontalSection);
    if (Group != 0) { // On an existing horizontal pipe section
        CurrentPnt = HorizontalSection->ProjPt_(CurrentPnt);

        if (m_PipeModePoints.empty()) { // Rising from an existing horizontal pipe section
            m_PipeModePoints.append(CurrentPnt);
            DropIntoOrRiseFromHorizontalSection(CurrentPnt, Group, HorizontalSection);
        } else { // Rising into an existing horizontal pipe section
            DropFromOrRiseIntoHorizontalSection(CurrentPnt, Group, HorizontalSection);
            Group = new EoDbGroup;
            GenerateLineWithFittings(m_PreviousOp, m_PipeModePoints[0], ID_OP5, CurrentPnt, Group);
            GetDocument()->AddWorkLayerGroup(Group);
            GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
        }
        m_PreviousOp = ModeLineHighlightOp(ID_OP5);
    } else {
        EoDbEllipse* VerticalSection;
        Group = SelectCircleUsingPoint(CurrentPnt, m_PipeRiseDropRadius, VerticalSection);
        if (Group != 0) { // On an existing vertical pipe section
            CurrentPnt = VerticalSection->Center();
            if (m_PipeModePoints.empty()) {
                m_PipeModePoints.append(CurrentPnt);
                m_PreviousOp = ModeLineHighlightOp(ID_OP4);
            } else { // Rising into an existing vertical pipe section
                GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
                m_PreviewGroup.DeletePrimitivesAndRemoveAll();
                Group = new EoDbGroup;
                GenerateLineWithFittings(m_PreviousOp, m_PipeModePoints[0], ID_OP5, CurrentPnt, Group);
                GetDocument()->AddWorkLayerGroup(Group);
                GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
                OnPipeModeEscape();
            }
        } else {
            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
            m_PreviewGroup.DeletePrimitivesAndRemoveAll();

            if (m_PipeModePoints.empty()) {
                m_PipeModePoints.append(CurrentPnt);
            } else {
                CurrentPnt = SnapPointToAxis(m_PipeModePoints[0], CurrentPnt);
                Group = new EoDbGroup;
                GenerateLineWithFittings(m_PreviousOp, m_PipeModePoints[0], ID_OP5, CurrentPnt, Group);
                GetDocument()->AddWorkLayerGroup(Group);
                GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
            }
            Group = new EoDbGroup;
            const auto ActiveViewPlaneNormal {GetActiveView()->CameraDirection()};

            auto Ellipse {EoDbEllipse::CreateCircle(BlockTableRecord, CurrentPnt, ActiveViewPlaneNormal, m_PipeRiseDropRadius)};
            Ellipse->setColorIndex(1);
            Ellipse->setLinetype(L"Continuous");
            Group->AddTail(EoDbEllipse::Create(Ellipse));

            GetDocument()->AddWorkLayerGroup(Group);
            GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
            m_PreviousOp = ModeLineHighlightOp(ID_OP5);
            m_PipeModePoints[0] = CurrentPnt;
        }
    }
}

void AeSysView::OnPipeModeDrop() {
    auto CurrentPnt {GetCursorPosition()};
    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    EoDbLine* HorizontalSection;
    auto Group {SelectLineBy(CurrentPnt, HorizontalSection)};
    if (Group != 0) { // On an existing horizontal pipe section
        CurrentPnt = HorizontalSection->ProjPt_(CurrentPnt);

        if (m_PipeModePoints.empty()) { // Dropping from an existing horizontal pipe section
            m_PipeModePoints.append(CurrentPnt);
            DropFromOrRiseIntoHorizontalSection(CurrentPnt, Group, HorizontalSection);
        } else { // Dropping into an existing horizontal pipe section
            DropIntoOrRiseFromHorizontalSection(CurrentPnt, Group, HorizontalSection);
            Group = new EoDbGroup;
            GenerateLineWithFittings(m_PreviousOp, m_PipeModePoints[0], ID_OP4, CurrentPnt, Group);
            GetDocument()->AddWorkLayerGroup(Group);
            GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
        }
        m_PreviousOp = ModeLineHighlightOp(ID_OP4);
    } else {
        EoDbEllipse* VerticalSection;
        Group = SelectCircleUsingPoint(CurrentPnt, m_PipeRiseDropRadius, VerticalSection);
        if (Group != 0) { // On an existing vertical pipe section
            CurrentPnt = VerticalSection->Center();
            if (m_PipeModePoints.empty()) {
                m_PipeModePoints.append(CurrentPnt);
                m_PreviousOp = ModeLineHighlightOp(ID_OP5);
            } else { // Dropping into an existing vertical pipe section
                GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
                m_PreviewGroup.DeletePrimitivesAndRemoveAll();
                Group = new EoDbGroup;
                GenerateLineWithFittings(m_PreviousOp, m_PipeModePoints[0], ID_OP4, CurrentPnt, Group);
                GetDocument()->AddWorkLayerGroup(Group);
                GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
                OnPipeModeEscape();
            }
        } else {
            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
            m_PreviewGroup.DeletePrimitivesAndRemoveAll();

            if (m_PipeModePoints.empty()) {
                m_PipeModePoints.append(CurrentPnt);
            } else {
                CurrentPnt = SnapPointToAxis(m_PipeModePoints[0], CurrentPnt);

                Group = new EoDbGroup;
                GenerateLineWithFittings(m_PreviousOp, m_PipeModePoints[0], ID_OP4, CurrentPnt, Group);
                GetDocument()->AddWorkLayerGroup(Group);
                GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
            }
            Group = new EoDbGroup;
            const auto ActiveViewPlaneNormal {GetActiveView()->CameraDirection()};

            auto Ellipse {EoDbEllipse::CreateCircle(BlockTableRecord, CurrentPnt, ActiveViewPlaneNormal, m_PipeRiseDropRadius)};
            Ellipse->setColorIndex(1);
            Ellipse->setLinetype(L"Continuous");
            Group->AddTail(EoDbEllipse::Create(Ellipse));

            GetDocument()->AddWorkLayerGroup(Group);
            GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);

            m_PreviousOp = ModeLineHighlightOp(ID_OP4);
            m_PipeModePoints[0] = CurrentPnt;
        }
    }
}

void AeSysView::OnPipeModeSymbol() {
    const double SymbolSize[] = {
        .09375, .09375, .09375, .09375, .125, .125, .125, .125, .125, .125,
        .125, .125, .125, .125, .125, 0., 0., .09375
    };
    const double TicDistance[] = {
        .125, .125, .125, .125, .15625, .15625, .15625, .15625, .15625, .15625,
        .15625, .15625, .15625, .15625, .15625, .03125, .03125, .125
    };

    const auto CurrentPnt {GetCursorPosition()};
    const auto ActiveViewPlaneNormal {GetActiveView()->CameraDirection()};
    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    OnPipeModeEscape();
    m_PipeModePoints.setLogicalLength(2);

    EoDbLine* HorizontalSection;
    auto Group {SelectLineBy(CurrentPnt, HorizontalSection)};
    if (Group == 0)
        return;

    EoDlgPipeSymbol Dialog;
    Dialog.m_CurrentPipeSymbolIndex = m_CurrentPipeSymbolIndex;
    if (Dialog.DoModal() == IDOK) {
        m_CurrentPipeSymbolIndex = Dialog.m_CurrentPipeSymbolIndex;
    }
    const auto BeginPoint {HorizontalSection->StartPoint()};
    const auto EndPoint {HorizontalSection->EndPoint()};
    const auto PointOnSection {HorizontalSection->ProjPt_(CurrentPnt)};

    EoGeLineSeg3d BeginSection(PointOnSection, BeginPoint);
    EoGeLineSeg3d EndSection(PointOnSection, EndPoint);

    GetDocument()->UpdatePrimitiveInAllViews(kPrimitiveEraseSafe, HorizontalSection);

    auto SymbolBeginPoint {ProjectToward(PointOnSection, BeginPoint, SymbolSize[m_CurrentPipeSymbolIndex])};
    auto SymbolEndPoint {ProjectToward(PointOnSection, EndPoint, SymbolSize[m_CurrentPipeSymbolIndex])};
    const double TicSize = m_PipeTicSize;

    HorizontalSection->SetEndPoint(SymbolBeginPoint);
    GetDocument()->UpdatePrimitiveInAllViews(kPrimitiveSafe, HorizontalSection);

    Group = new EoDbGroup;

    auto Line {EoDbLine::Create(BlockTableRecord, SymbolEndPoint, EndPoint)};
    Line->setColorIndex(HorizontalSection->ColorIndex());
    Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(HorizontalSection->LinetypeIndex()));
    Group->AddTail(EoDbLine::Create(Line));

    GetDocument()->AddWorkLayerGroup(Group);
    GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);

    Group = new EoDbGroup;
    GenerateTicMark(PointOnSection, BeginPoint, TicDistance[m_CurrentPipeSymbolIndex], Group);
    GenerateTicMark(PointOnSection, EndPoint, TicDistance[m_CurrentPipeSymbolIndex], Group);

    const auto ColorIndex {pstate.ColorIndex()};
    const auto Linetype {EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex())};

    switch (m_CurrentPipeSymbolIndex) {

    case 0:
    { // Generate flow switch
        auto Circle {EoDbEllipse::CreateCircle(BlockTableRecord, PointOnSection, ActiveViewPlaneNormal, SymbolSize[0])};
        Circle->setColorIndex(ColorIndex);
        Circle->setLinetype(Linetype);
        Group->AddTail(EoDbEllipse::Create(Circle));

        EndSection.ProjPtFrom_xy(SymbolSize[0], -SymbolSize[0] * 1.5, m_PipeModePoints[0]);
        EndSection.ProjPtFrom_xy(SymbolSize[0], -SymbolSize[0] * 2., m_PipeModePoints[1]);
        BeginSection.ProjPtFrom_xy(SymbolSize[0], SymbolSize[0] * 1.5, SymbolBeginPoint);
        BeginSection.ProjPtFrom_xy(SymbolSize[0], SymbolSize[0] * 2., SymbolEndPoint);

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolEndPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolEndPoint, SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));
        break;
    }
    case 1:
    { // Generate float and thermostatic trap
        auto Circle {EoDbEllipse::CreateCircle(BlockTableRecord, PointOnSection, ActiveViewPlaneNormal, SymbolSize[1])};
        Circle->setColorIndex(ColorIndex);
        Circle->setLinetype(Linetype);
        Group->AddTail(EoDbEllipse::Create(Circle));

        m_PipeModePoints[0] = SymbolBeginPoint;
        m_PipeModePoints[0].rotateBy(QUARTER_PI, OdGeVector3d::kZAxis, PointOnSection);
        m_PipeModePoints[1] = m_PipeModePoints[0];
        m_PipeModePoints[1].rotateBy(PI, OdGeVector3d::kZAxis, PointOnSection);

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        m_PipeModePoints[0] = SymbolBeginPoint;
        m_PipeModePoints[0].rotateBy(3. * QUARTER_PI, OdGeVector3d::kZAxis, PointOnSection);
        m_PipeModePoints[1] = m_PipeModePoints[0];
        m_PipeModePoints[1].rotateBy(PI, OdGeVector3d::kZAxis, PointOnSection);

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));
        break;
    }
    case 2:
    {
        auto Circle {EoDbEllipse::CreateCircle(BlockTableRecord, PointOnSection, ActiveViewPlaneNormal, SymbolSize[2])};
        Circle->setColorIndex(pstate.ColorIndex());
        Circle->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex()));
        Group->AddTail(EoDbEllipse::Create(Circle));

        EndSection.ProjPtFrom_xy(SymbolSize[2], SymbolSize[2] * 1.5, m_PipeModePoints[0]);
        EndSection.ProjPtFrom_xy(0., SymbolSize[2] * 1.5, m_PipeModePoints[1]);

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        EndSection.ProjPtFrom_xy(0., SymbolSize[2], SymbolBeginPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));
        break;
    }
    case 3:
    {
        auto Circle {EoDbEllipse::CreateCircle(BlockTableRecord, PointOnSection, ActiveViewPlaneNormal, SymbolSize[3])};
        Circle->setColorIndex(pstate.ColorIndex());
        Circle->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex()));
        Group->AddTail(EoDbEllipse::Create(Circle));

        EndSection.ProjPtFrom_xy(SymbolSize[3], SymbolSize[3] * 1.5, m_PipeModePoints[0]);
        EndSection.ProjPtFrom_xy(0., SymbolSize[3] * 1.5, m_PipeModePoints[1]);

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        BeginSection.ProjPtFrom_xy(0., SymbolSize[3], SymbolBeginPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));
        break;
    }
    case 4:
    {
        EndSection.ProjPtFrom_xy(SymbolSize[4], SymbolSize[4] * .5, m_PipeModePoints[0]);
        EndSection.ProjPtFrom_xy(SymbolSize[4], -SymbolSize[4] * .5, m_PipeModePoints[1]);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        BeginSection.ProjPtFrom_xy(SymbolSize[4], -SymbolSize[4] * .5, SymbolBeginPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        BeginSection.ProjPtFrom_xy(SymbolSize[4], SymbolSize[4] * .5, SymbolEndPoint);
        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, SymbolEndPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        BeginSection.ProjPtFrom_xy(SymbolSize[4], -SymbolSize[4] * .3, m_PipeModePoints[0]);

        auto Circle {EoDbEllipse::CreateCircle(BlockTableRecord, SymbolBeginPoint, ActiveViewPlaneNormal, (m_PipeModePoints[0] - SymbolBeginPoint).length())};
        Circle->setColorIndex(pstate.ColorIndex());
        Circle->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex()));
        Group->AddTail(EoDbEllipse::Create(Circle));
        break;
    }
    case 5:
    {
        EndSection.ProjPtFrom_xy(SymbolSize[5], SymbolSize[5] * .5, m_PipeModePoints[0]);
        EndSection.ProjPtFrom_xy(SymbolSize[5], -SymbolSize[5] * .5, m_PipeModePoints[1]);

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        BeginSection.ProjPtFrom_xy(SymbolSize[5], -SymbolSize[5] * .5, SymbolBeginPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        BeginSection.ProjPtFrom_xy(SymbolSize[5], SymbolSize[5] * .5, SymbolEndPoint);
        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, SymbolEndPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        BeginSection.ProjPtFrom_xy(SymbolSize[5], -SymbolSize[5] * .3, m_PipeModePoints[0]);

        auto Circle {EoDbEllipse::CreateCircle(BlockTableRecord, SymbolBeginPoint, ActiveViewPlaneNormal, (m_PipeModePoints[0] - SymbolBeginPoint).length())};
        Circle->setColorIndex(pstate.ColorIndex());
        Circle->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex()));
        Group->AddTail(EoDbEllipse::Create(Circle));

        Line = EoDbLine::Create(BlockTableRecord, SymbolEndPoint, PointOnSection);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));
        break;
    }
    case 6:
    { // Generate gate valve
        EndSection.ProjPtFrom_xy(SymbolSize[6], SymbolSize[6] * .5, m_PipeModePoints[0]);
        EndSection.ProjPtFrom_xy(SymbolSize[6], -SymbolSize[6] * .5, m_PipeModePoints[1]);
        BeginSection.ProjPtFrom_xy(SymbolSize[6], -SymbolSize[6] * .5, SymbolBeginPoint);
        BeginSection.ProjPtFrom_xy(SymbolSize[6], SymbolSize[6] * .5, SymbolEndPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, SymbolEndPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolEndPoint, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));
        break;
    }
    case 7:
    { // Generate globe valve
        EndSection.ProjPtFrom_xy(SymbolSize[7], SymbolSize[7] * .5, m_PipeModePoints[0]);
        EndSection.ProjPtFrom_xy(SymbolSize[7], -SymbolSize[7] * .5, m_PipeModePoints[1]);
        BeginSection.ProjPtFrom_xy(SymbolSize[7], -SymbolSize[7] * .5, SymbolBeginPoint);
        BeginSection.ProjPtFrom_xy(SymbolSize[7], SymbolSize[7] * .5, SymbolEndPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, SymbolEndPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolEndPoint, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        m_PipeModePoints[0] = ProjectToward(PointOnSection, EndPoint, SymbolSize[7] * .25);

        auto Circle {EoDbEllipse::CreateCircle(BlockTableRecord, PointOnSection, ActiveViewPlaneNormal, (m_PipeModePoints[0] - PointOnSection).length())};
        Circle->setColorIndex(pstate.ColorIndex());
        Circle->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex()));
        Group->AddTail(EoDbEllipse::Create(Circle));
        break;
    }
    case 8:
    { // Generate stop check valve
        EndSection.ProjPtFrom_xy(SymbolSize[8], SymbolSize[8] * .5, m_PipeModePoints[0]);
        EndSection.ProjPtFrom_xy(SymbolSize[8], -SymbolSize[8] * .5, m_PipeModePoints[1]);
        BeginSection.ProjPtFrom_xy(SymbolSize[8], -SymbolSize[8] * .5, SymbolBeginPoint);
        BeginSection.ProjPtFrom_xy(SymbolSize[8], SymbolSize[8] * .5, SymbolEndPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, SymbolEndPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolEndPoint, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        m_PipeModePoints[0] = ProjectToward(PointOnSection, EndPoint, SymbolSize[8] * .25);

        auto Circle {EoDbEllipse::CreateCircle(BlockTableRecord, PointOnSection, ActiveViewPlaneNormal, (m_PipeModePoints[0] - PointOnSection).length())};
        Circle->setColorIndex(pstate.ColorIndex());
        Circle->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex()));
        Group->AddTail(EoDbEllipse::Create(Circle));

        EndSection.ProjPtFrom_xy(0., SymbolSize[8], m_PipeModePoints[0]);
        Line = EoDbLine::Create(BlockTableRecord, PointOnSection, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        m_PipeTicSize = SymbolSize[8] * .25;
        GenerateTicMark(PointOnSection, m_PipeModePoints[0], SymbolSize[8] * .75, Group);
        break;
    }
    case 9:
    { // Generate pressure reducing valve
        EndSection.ProjPtFrom_xy(SymbolSize[9], SymbolSize[9] * .5, m_PipeModePoints[0]);
        EndSection.ProjPtFrom_xy(SymbolSize[9], -SymbolSize[9] * .5, m_PipeModePoints[1]);
        BeginSection.ProjPtFrom_xy(SymbolSize[9], -SymbolSize[9] * .5, SymbolBeginPoint);
        BeginSection.ProjPtFrom_xy(SymbolSize[9], SymbolSize[9] * .5, SymbolEndPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, SymbolEndPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolEndPoint, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        m_PipeModePoints[0] = ProjectToward(PointOnSection, EndPoint, SymbolSize[9] * .25);

        auto Circle {EoDbEllipse::CreateCircle(BlockTableRecord, PointOnSection, ActiveViewPlaneNormal, (m_PipeModePoints[0] - PointOnSection).length())};
        Circle->setColorIndex(pstate.ColorIndex());
        Circle->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex()));
        Group->AddTail(EoDbEllipse::Create(Circle));

        EndSection.ProjPtFrom_xy(0., SymbolSize[9], m_PipeModePoints[0]);
        Line = EoDbLine::Create(BlockTableRecord, PointOnSection, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        EndSection.ProjPtFrom_xy(SymbolSize[9] * .5, SymbolSize[9] * .75, m_PipeModePoints[1]);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        EndSection.ProjPtFrom_xy(0., SymbolSize[9] * .5, SymbolBeginPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));
        break;
    }
    case 10:
    {
        EndSection.ProjPtFrom_xy(SymbolSize[10], SymbolSize[10] * .5, m_PipeModePoints[0]);
        EndSection.ProjPtFrom_xy(SymbolSize[10], -SymbolSize[10] * .5, m_PipeModePoints[1]);
        BeginSection.ProjPtFrom_xy(SymbolSize[10], -SymbolSize[10] * .5, SymbolBeginPoint);
        BeginSection.ProjPtFrom_xy(SymbolSize[10], SymbolSize[10] * .5, SymbolEndPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, SymbolEndPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolEndPoint, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        EndSection.ProjPtFrom_xy(0., SymbolSize[10] * .5, m_PipeModePoints[0]);
        Line = EoDbLine::Create(BlockTableRecord, PointOnSection, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        EndSection.ProjPtFrom_xy(SymbolSize[10] * .25, SymbolSize[10] * .5, m_PipeModePoints[0]);
        EndSection.ProjPtFrom_xy(SymbolSize[10] * .25, SymbolSize[10] * .75, m_PipeModePoints[1]);
        BeginSection.ProjPtFrom_xy(SymbolSize[10] * .25, -SymbolSize[10] * .75, SymbolBeginPoint);
        BeginSection.ProjPtFrom_xy(SymbolSize[10] * .25, -SymbolSize[10] * .5, SymbolEndPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, SymbolEndPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolEndPoint, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));
        break;
    }
    case 11:
    { // Generate automatic 3-way valve
        EndSection.ProjPtFrom_xy(SymbolSize[11], SymbolSize[11] * .5, m_PipeModePoints[0]);
        EndSection.ProjPtFrom_xy(SymbolSize[11], -SymbolSize[11] * .5, m_PipeModePoints[1]);
        BeginSection.ProjPtFrom_xy(SymbolSize[11], -SymbolSize[11] * .5, SymbolBeginPoint);
        BeginSection.ProjPtFrom_xy(SymbolSize[11], SymbolSize[11] * .5, SymbolEndPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, SymbolEndPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolEndPoint, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        EndSection.ProjPtFrom_xy(0., SymbolSize[11] * .5, m_PipeModePoints[0]);
        Line = EoDbLine::Create(BlockTableRecord, PointOnSection, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        EndSection.ProjPtFrom_xy(SymbolSize[11] * .25, SymbolSize[11] * .5, m_PipeModePoints[0]);
        EndSection.ProjPtFrom_xy(SymbolSize[11] * .25, SymbolSize[11] * .75, m_PipeModePoints[1]);
        BeginSection.ProjPtFrom_xy(SymbolSize[11] * .25, -SymbolSize[11] * .75, SymbolBeginPoint);
        BeginSection.ProjPtFrom_xy(SymbolSize[11] * .25, -SymbolSize[11] * .5, SymbolEndPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, SymbolEndPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolEndPoint, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        EndSection.ProjPtFrom_xy(SymbolSize[11] * .5, -SymbolSize[11], m_PipeModePoints[0]);
        BeginSection.ProjPtFrom_xy(SymbolSize[11] * .5, SymbolSize[11], m_PipeModePoints[1]);
        Line = EoDbLine::Create(BlockTableRecord, PointOnSection, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], PointOnSection);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));
        break;
    }
    case 12:
    { // Generate self operated valve
        EndSection.ProjPtFrom_xy(SymbolSize[12], SymbolSize[12] * .5, m_PipeModePoints[0]);
        EndSection.ProjPtFrom_xy(SymbolSize[12], -SymbolSize[12] * .5, m_PipeModePoints[1]);
        BeginSection.ProjPtFrom_xy(SymbolSize[12], -SymbolSize[12] * .5, SymbolBeginPoint);
        BeginSection.ProjPtFrom_xy(SymbolSize[12], SymbolSize[12] * .5, SymbolEndPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, SymbolEndPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolEndPoint, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        EndSection.ProjPtFrom_xy(0., SymbolSize[12] * .5, m_PipeModePoints[0]);
        Line = EoDbLine::Create(BlockTableRecord, PointOnSection, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        EndSection.ProjPtFrom_xy(SymbolSize[12] * .25, SymbolSize[12] * .5, m_PipeModePoints[1]);
        BeginSection.ProjPtFrom_xy(SymbolSize[12] * .25, -SymbolSize[12] * .5, SymbolBeginPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));
        // add a half circle here i think
        BeginSection.ProjPtFrom_xy(SymbolSize[12] * 1.25, -SymbolSize[12] * .5, m_PipeModePoints[0]);
        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(2));
        Group->AddTail(EoDbLine::Create(Line));

        BeginSection.ProjPtFrom_xy(SymbolSize[12] * 1.25, -SymbolSize[12] * .75, m_PipeModePoints[1]);
        BeginSection.ProjPtFrom_xy(SymbolSize[12] * 2., -SymbolSize[12] * .75, SymbolBeginPoint);
        BeginSection.ProjPtFrom_xy(SymbolSize[12] * 2., -SymbolSize[12] * .5, SymbolEndPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, SymbolEndPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolEndPoint, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));
        break;
    }
    case 13:
    {
        EndSection.ProjPtFrom_xy(0., -SymbolSize[13], m_PipeModePoints[0]);
        EndSection.ProjPtFrom_xy(0., SymbolSize[13], m_PipeModePoints[1]);
        Line = EoDbLine::Create(BlockTableRecord, SymbolEndPoint, m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], SymbolEndPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));
        break;
    }
    case 14:
    {
        EndSection.ProjPtFrom_xy(0., -SymbolSize[14], m_PipeModePoints[0]);
        EndSection.ProjPtFrom_xy(0., SymbolSize[14], m_PipeModePoints[1]);
        Line = EoDbLine::Create(BlockTableRecord, SymbolEndPoint, m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], SymbolEndPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[0], m_PipeModePoints[1]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));
        break;
    }
    case 15:
    {
        EndSection.ProjPtFrom_xy(0., -.250, m_PipeModePoints[0]);
        Line = EoDbLine::Create(BlockTableRecord, PointOnSection, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        GenerateTicMark(PointOnSection, m_PipeModePoints[0], TicDistance[15], Group);
        BeginSection.ProjPtFrom_xy(.0625, .1875, m_PipeModePoints[1]);
        EndSection.ProjPtFrom_xy(.0625, -.1875, SymbolBeginPoint);
        EndSection.ProjPtFrom_xy(.0625, -.125, SymbolEndPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, SymbolEndPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));
        break;
    }
    case 16:
    {
        EndSection.ProjPtFrom_xy(0., -.250, m_PipeModePoints[0]);
        Line = EoDbLine::Create(BlockTableRecord, PointOnSection, m_PipeModePoints[0]);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        GenerateTicMark(PointOnSection, m_PipeModePoints[0], TicDistance[16], Group);
        BeginSection.ProjPtFrom_xy(.0625, .1875, m_PipeModePoints[1]);
        EndSection.ProjPtFrom_xy(.0625, -.1875, SymbolBeginPoint);
        EndSection.ProjPtFrom_xy(.0625, -.125, SymbolEndPoint);
        Line = EoDbLine::Create(BlockTableRecord, m_PipeModePoints[1], SymbolBeginPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, SymbolBeginPoint, SymbolEndPoint);
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        Group->AddTail(EoDbLine::Create(Line));

        m_PipeModePoints[1] = ProjectToward(PointOnSection, m_PipeModePoints[0], .28125);

        auto Circle {EoDbEllipse::CreateCircle(BlockTableRecord, m_PipeModePoints[1], ActiveViewPlaneNormal, (m_PipeModePoints[0] - m_PipeModePoints[1]).length())};
        Circle->setColorIndex(ColorIndex);
        Circle->setLinetype(Linetype);
        Group->AddTail(EoDbEllipse::Create(Circle));
        break;
    }
    case 17:	// Generate union
        m_PipeTicSize = SymbolSize[17];
        GenerateTicMark(PointOnSection, BeginPoint, SymbolSize[17], Group);
        GenerateTicMark(PointOnSection, EndPoint, SymbolSize[17], Group);
        m_PipeTicSize = m_PipeTicSize * 2.;
        GenerateTicMark(PointOnSection, BeginPoint, 0., Group);
        break;
    }
    m_PipeTicSize = TicSize;
    GetDocument()->AddWorkLayerGroup(Group);
    GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
}

void AeSysView::OnPipeModeWye() {
    const auto CurrentPnt {GetCursorPosition()};
    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    if (m_PipeModePoints.empty()) {
        m_PipeModePoints.append(CurrentPnt);
        m_PreviousOp = ModeLineHighlightOp(ID_OP9);
        return;
    }
    EoDbLine* HorizontalSection;
    auto Group = SelectLineBy(CurrentPnt, HorizontalSection);
    if (Group != 0) {
        auto PointOnSection {HorizontalSection->ProjPt_(CurrentPnt)};
        const auto BeginPointProjectedToSection {HorizontalSection->ProjPt_(m_PipeModePoints[0])};
        const auto DistanceToSection {(BeginPointProjectedToSection - m_PipeModePoints[0]).length()};

        if (DistanceToSection >= .25) {
            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
            m_PreviewGroup.DeletePrimitivesAndRemoveAll();
            const auto BeginPoint {HorizontalSection->StartPoint()};
            const auto EndPoint {HorizontalSection->EndPoint()};

            const auto DistanceBetweenSectionPoints {(PointOnSection - BeginPointProjectedToSection).length()};

            if (fabs(DistanceBetweenSectionPoints - DistanceToSection) <= .25) { // Just need to shift point on section and do a single 45 degree line
                PointOnSection = ProjectToward(BeginPointProjectedToSection, PointOnSection, DistanceToSection);
                HorizontalSection->SetEndPoint(PointOnSection);

                Group = new EoDbGroup;

                auto Line {EoDbLine::Create(BlockTableRecord, PointOnSection, EndPoint)};
                Line->setColorIndex(HorizontalSection->ColorIndex());
                Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(HorizontalSection->LinetypeIndex()));
                Group->AddTail(EoDbLine::Create(Line));

                GetDocument()->AddWorkLayerGroup(Group);

                Group = new EoDbGroup;
                GenerateTicMark(PointOnSection, BeginPoint, m_PipeRiseDropRadius, Group);
                GenerateTicMark(PointOnSection, EndPoint, m_PipeRiseDropRadius, Group);
                GetDocument()->AddWorkLayerGroup(Group);
                GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);

                Group = new EoDbGroup;
                GenerateLineWithFittings(m_PreviousOp, m_PipeModePoints[0], ID_OP3, PointOnSection, Group);
                GetDocument()->AddWorkLayerGroup(Group);
                GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
            } else {
                OdGePoint3d PointAtBend;

                if (DistanceBetweenSectionPoints - .25 <= DistanceToSection) {
                    const double d3 = (DistanceBetweenSectionPoints > .25) ? DistanceBetweenSectionPoints : .125;
                    PointAtBend = ProjectToward(BeginPointProjectedToSection, m_PipeModePoints[0], d3);
                    PointOnSection = ProjectToward(BeginPointProjectedToSection, PointOnSection, d3);
                } else {
                    PointAtBend = ProjectToward(BeginPointProjectedToSection, PointOnSection, DistanceBetweenSectionPoints - DistanceToSection);
                    PointAtBend = m_PipeModePoints[0] + OdGeVector3d(PointAtBend - BeginPointProjectedToSection);
                }
                HorizontalSection->SetEndPoint(PointOnSection);

                Group = new EoDbGroup;
                GenerateTicMark(PointOnSection, BeginPoint, m_PipeRiseDropRadius, Group);
                GenerateTicMark(PointOnSection, EndPoint, m_PipeRiseDropRadius, Group);
                GetDocument()->AddWorkLayerGroup(Group);
                GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);

                Group = new EoDbGroup;

                auto Line {EoDbLine::Create(BlockTableRecord, PointOnSection, EndPoint)};
                Line->setColorIndex(HorizontalSection->ColorIndex());
                Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(HorizontalSection->LinetypeIndex()));
                Group->AddTail(EoDbLine::Create(Line));

                GetDocument()->AddWorkLayerGroup(Group);
                Group = new EoDbGroup;
                GenerateLineWithFittings(m_PreviousOp, m_PipeModePoints[0], ID_OP3, PointAtBend, Group);
                GetDocument()->AddWorkLayerGroup(Group);
                GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
                Group = new EoDbGroup;
                GenerateLineWithFittings(ID_OP3, PointAtBend, ID_OP3, PointOnSection, Group);
                GetDocument()->AddWorkLayerGroup(Group);
                GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
            }
        }
        OnPipeModeEscape();
    }
}

void AeSysView::OnPipeModeReturn() {
    OnPipeModeEscape();
}

void AeSysView::OnPipeModeEscape() {
    GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
    m_PreviewGroup.DeletePrimitivesAndRemoveAll();
    m_PipeModePoints.clear();

    ModeLineUnhighlightOp(m_PreviousOp);
    m_PreviousOp = 0;
}

void AeSysView::DoPipeModeMouseMove() {
    auto CurrentPnt {GetCursorPosition()};
    const int NumberOfPoints = m_PipeModePoints.size();

    switch (m_PreviousOp) {
    case ID_OP2:
        if (m_PipeModePoints[0] != CurrentPnt) {
            CurrentPnt = SnapPointToAxis(m_PipeModePoints[0], CurrentPnt);
            m_PipeModePoints.append(CurrentPnt);

            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
            m_PreviewGroup.DeletePrimitivesAndRemoveAll();

            GenerateLineWithFittings(m_PreviousOp, m_PipeModePoints[0], ID_OP2, CurrentPnt, &m_PreviewGroup);
            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        }
        break;

    case ID_OP3:
        if (m_PipeModePoints[0] != CurrentPnt) {
            CurrentPnt = SnapPointToAxis(m_PipeModePoints[0], CurrentPnt);
            m_PipeModePoints.append(CurrentPnt);

            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
            m_PreviewGroup.DeletePrimitivesAndRemoveAll();

            GenerateLineWithFittings(m_PreviousOp, m_PipeModePoints[0], ID_OP3, CurrentPnt, &m_PreviewGroup);
            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        }
        break;

    case ID_OP4:
    case ID_OP5:
    case ID_OP9:
    {
        CurrentPnt = SnapPointToAxis(m_PipeModePoints[0], CurrentPnt);
        m_PipeModePoints.append(CurrentPnt);

        GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        m_PreviewGroup.DeletePrimitivesAndRemoveAll();

        GenerateLineWithFittings(m_PreviousOp, m_PipeModePoints[0], ID_OP3, CurrentPnt, &m_PreviewGroup);
        GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        break;
    }
    }
    m_PipeModePoints.setLogicalLength(NumberOfPoints);
}

void AeSysView::GenerateLineWithFittings(int beginType, OdGePoint3d & startPoint, int endType, OdGePoint3d & endPoint, EoDbGroup * group) {
    auto pt1 {startPoint};
    auto pt2 {endPoint};

    if (beginType == ID_OP3)
        // Previous fitting is an elbow or side tee
        GenerateTicMark(startPoint, endPoint, m_PipeRiseDropRadius, group);
    else if (beginType == ID_OP4) { // Previous fitting is an elbow down, riser down or bottom tee
        pt1 = ProjectToward(startPoint, endPoint, m_PipeRiseDropRadius);
        GenerateTicMark(pt1, endPoint, m_PipeRiseDropRadius, group);
    } else if (beginType == ID_OP5)
        // Previous fitting is an elbow up, riser up or top tee
        GenerateTicMark(startPoint, endPoint, 2. * m_PipeRiseDropRadius, group);

    if (endType == ID_OP3)
        // Current fitting is an elbow or side tee
        GenerateTicMark(endPoint, startPoint, m_PipeRiseDropRadius, group);
    else if (endType == ID_OP4)
        // Current fitting is an elbow down, riser down or bottom tee
        GenerateTicMark(endPoint, startPoint, 2. * m_PipeRiseDropRadius, group);
    else if (endType == ID_OP5) { // Current fitting is an elbow up, riser up or top tee
        pt2 = ProjectToward(endPoint, startPoint, m_PipeRiseDropRadius);
        GenerateTicMark(endPoint, startPoint, 2. * m_PipeRiseDropRadius, group);
    }
    auto Line = group->m_Document == 0 ? new EoDbLine(pt1, pt2) : EoDbLine::Create2(pt1, pt2);
    group->AddTail(Line);
}

void AeSysView::DropIntoOrRiseFromHorizontalSection(const OdGePoint3d & point, EoDbGroup * group, EoDbLine * section) {
    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    GetDocument()->UpdatePrimitiveInAllViews(kPrimitiveEraseSafe, section);

    const auto BeginPoint {section->StartPoint()};
    const auto EndPoint {section->EndPoint()};
    auto CutPoint {ProjectToward(point, BeginPoint, m_PipeRiseDropRadius)};

    section->SetEndPoint(CutPoint);
    CutPoint = ProjectToward(point, EndPoint, m_PipeRiseDropRadius);
    auto Line {EoDbLine::Create2(CutPoint, EndPoint)};
    Line->SetColorIndex(section->ColorIndex());
    Line->SetLinetypeIndex(section->LinetypeIndex());
    group->AddTail(Line);
    GetDocument()->UpdateGroupInAllViews(kGroupSafe, group);

    group = new EoDbGroup;
    GenerateTicMark(point, BeginPoint, 2. * m_PipeRiseDropRadius, group);

    const auto ActiveViewPlaneNormal {GetActiveView()->CameraDirection()};

    auto Circle {EoDbEllipse::CreateCircle(BlockTableRecord, point, ActiveViewPlaneNormal, m_PipeRiseDropRadius)};
    Circle->setColorIndex(1);
    Circle->setLinetype(L"Continuous");
    group->AddTail(EoDbEllipse::Create(Circle));

    GenerateTicMark(point, EndPoint, 2. * m_PipeRiseDropRadius, group);
    GetDocument()->AddWorkLayerGroup(group);
    GetDocument()->UpdateGroupInAllViews(kGroupSafe, group);
}

void AeSysView::DropFromOrRiseIntoHorizontalSection(const OdGePoint3d & point, EoDbGroup * group, EoDbLine * section) {
    const auto BeginPoint {section->StartPoint()};
    const auto EndPoint {section->EndPoint()};

    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    section->SetEndPoint(point);
    auto Line {EoDbLine::Create(BlockTableRecord, point, EndPoint)};
    Line->setColorIndex(section->ColorIndex());
    Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(section->LinetypeIndex()));
    group->AddTail(EoDbLine::Create(Line));

    group = new EoDbGroup;
    GenerateTicMark(point, BeginPoint, 2. * m_PipeRiseDropRadius, group);

    const auto ActiveViewPlaneNormal {GetActiveView()->CameraDirection()};

    auto Circle {EoDbEllipse::CreateCircle(BlockTableRecord, point, ActiveViewPlaneNormal, m_PipeRiseDropRadius)};
    Circle->setColorIndex(1);
    Circle->setLinetype(L"Continuous");
    group->AddTail(EoDbEllipse::Create(Circle));

    GenerateTicMark(point, EndPoint, 2. * m_PipeRiseDropRadius, group);
    GetDocument()->AddWorkLayerGroup(group);
    GetDocument()->UpdateGroupInAllViews(kGroupSafe, group);
}

bool AeSysView::GenerateTicMark(const OdGePoint3d & startPoint, const OdGePoint3d & endPoint, double distance, EoDbGroup * group) {
    const auto PointOnLine {ProjectToward(startPoint, endPoint, distance)};

    OdGeVector3d Projection(endPoint - PointOnLine);

    const double DistanceToEndPoint = Projection.length();

    const bool MarkGenerated = DistanceToEndPoint > DBL_EPSILON;
    if (MarkGenerated) {
        Projection *= m_PipeTicSize / DistanceToEndPoint;

        OdGePoint3d TicStartPoint(PointOnLine);
        TicStartPoint += OdGeVector3d(Projection.y, -Projection.x, 0.);

        OdGePoint3d TicEndPoint(PointOnLine);
        TicEndPoint += OdGeVector3d(-Projection.y, Projection.x, 0.);
        auto Line = group->m_Document == 0 ? new EoDbLine(TicStartPoint, TicEndPoint) : EoDbLine::Create2(TicStartPoint, TicEndPoint);
        Line->SetColorIndex(1);
        Line->SetLinetypeIndex(1);
        group->AddTail(Line);
    }
    return MarkGenerated;
}
