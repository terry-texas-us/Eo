#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDlgBlockInsert.h"

#include "DbGroup.h"
#include "DbAudit.h"
#include "EoDbEntityToPrimitiveProtocolExtension.h"

#include "Ge/GeCircArc3d.h"

OdUInt16 PreviousDrawCommand = 0;

void AeSysView::OnDrawModeOptions() {
    AeSysDoc::GetDoc()->OnSetupOptionsDraw();
}

void AeSysView::OnDrawModePoint() {
    const auto CurrentPnt {GetCursorPosition()};
    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
    auto Point {EoDbPoint::Create(BlockTableRecord)};

    Point->setPosition(CurrentPnt);

    auto Group {new EoDbGroup};
    Group->AddTail(EoDbPoint::Create(Point));
    GetDocument()->AddWorkLayerGroup(Group);
    GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
}

void AeSysView::OnDrawModeLine() {
    auto CurrentPnt {GetCursorPosition()};

    if (PreviousDrawCommand != ID_OP2) {
        PreviousDrawCommand = ModeLineHighlightOp(ID_OP2);
        m_DrawModePoints.clear();
        m_DrawModePoints.append(CurrentPnt);
    } else {
        CurrentPnt = SnapPointToAxis(m_DrawModePoints[0], CurrentPnt);
        OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
        auto Line {EoDbLine::Create(BlockTableRecord)};
        Line->setStartPoint(m_DrawModePoints[0]);
        Line->setEndPoint(CurrentPnt);

        auto Group {new EoDbGroup};
        Group->AddTail(EoDbLine::Create(Line));
        GetDocument()->AddWorkLayerGroup(Group);

        m_DrawModePoints[0] = CurrentPnt;
        m_PreviewGroup.DeletePrimitivesAndRemoveAll();
    }
}

void AeSysView::OnDrawModePolygon() {
    auto CurrentPnt {GetCursorPosition()};

    if (PreviousDrawCommand != ID_OP3) {
        PreviousDrawCommand = ModeLineHighlightOp(ID_OP3);
        m_DrawModePoints.clear();
        m_DrawModePoints.append(CurrentPnt);
    } else {
        const int NumberOfPoints = m_DrawModePoints.size();

        if (m_DrawModePoints[NumberOfPoints - 1] != CurrentPnt) {
            CurrentPnt = SnapPointToAxis(m_DrawModePoints[NumberOfPoints - 1], CurrentPnt);
            m_DrawModePoints.append(CurrentPnt);
        }
    }
}

void AeSysView::OnDrawModeQuad() {
    const auto CurrentPnt {GetCursorPosition()};

    if (PreviousDrawCommand != ID_OP4) {
        PreviousDrawCommand = ModeLineHighlightOp(ID_OP4);
        m_DrawModePoints.clear();
        m_DrawModePoints.append(CurrentPnt);
    } else {
        OnDrawModeReturn();
    }
}

void AeSysView::OnDrawModeArc() {
    const auto CurrentPnt {GetCursorPosition()};

    if (PreviousDrawCommand != ID_OP5) {
        PreviousDrawCommand = ModeLineHighlightOp(ID_OP5);
        m_DrawModePoints.clear();
        m_DrawModePoints.append(CurrentPnt);
    } else {
        OnDrawModeReturn();
    }
}

void AeSysView::OnDrawModeBspline() {
    const auto CurrentPnt {GetCursorPosition()};

    if (PreviousDrawCommand != ID_OP6) {
        PreviousDrawCommand = ModeLineHighlightOp(ID_OP6);

        m_DrawModePoints.clear();
        m_DrawModePoints.append(CurrentPnt);
    } else {
        if (!m_DrawModePoints[m_DrawModePoints.size() - 1].isEqualTo(CurrentPnt)) {
            m_DrawModePoints.append(CurrentPnt);
        }
    }
}
void AeSysView::OnDrawModeCircle() {
    const auto CurrentPnt {GetCursorPosition()};

    if (PreviousDrawCommand != ID_OP7) {
        PreviousDrawCommand = ModeLineHighlightOp(ID_OP7);
        m_DrawModePoints.clear();
        m_DrawModePoints.append(CurrentPnt);
        m_PreviewGroup.DeletePrimitivesAndRemoveAll();
    } else {
        OnDrawModeReturn();
    }
}

void AeSysView::OnDrawModeEllipse() {
    const auto CurrentPnt {GetCursorPosition()};

    if (PreviousDrawCommand != ID_OP8) {
        PreviousDrawCommand = ModeLineHighlightOp(ID_OP8);
        m_DrawModePoints.clear();
        m_DrawModePoints.append(CurrentPnt);
    } else {
        OnDrawModeReturn();
    }
}

void AeSysView::OnDrawModeInsert() {
    auto Document {GetDocument()};

    if (Document->BlockTableSize() > 0) {

        EoDlgBlockInsert Dialog(Document);
        Dialog.DoModal();
    }
}

void AeSysView::OnDrawModeReturn() {
    auto CurrentPnt {GetCursorPosition()};

    const int NumberOfPoints = m_DrawModePoints.size();
    EoDbGroup* Group {nullptr};

    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    switch (PreviousDrawCommand) {
    case ID_OP2: {
        CurrentPnt = SnapPointToAxis(m_DrawModePoints[0], CurrentPnt);
        auto Line {EoDbLine::Create(BlockTableRecord)};
        Line->setStartPoint(m_DrawModePoints[0]);
        Line->setEndPoint(CurrentPnt);

        Group = new EoDbGroup;
        Group->AddTail(EoDbLine::Create(Line));
        break;
    }
    case ID_OP3: {
        if (NumberOfPoints == 1)
            return;

        if (m_DrawModePoints[NumberOfPoints - 1] == CurrentPnt) {
            theApp.AddStringToMessageList(IDS_MSG_PTS_COINCIDE);
            return;
        }
        CurrentPnt = SnapPointToAxis(m_DrawModePoints[NumberOfPoints - 1], CurrentPnt);
        m_DrawModePoints.append(CurrentPnt);
        Group = new EoDbGroup;
        auto Hatch {EoDbHatch::Create0(Database())};
        Hatch->SetVertices(m_DrawModePoints);
        Group->AddTail(Hatch);
        GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
        break;
    }
    case ID_OP4: {
        if (m_DrawModePoints[NumberOfPoints - 1] == CurrentPnt) {
            theApp.AddStringToMessageList(IDS_MSG_PTS_COINCIDE);
            return;
        }
        CurrentPnt = SnapPointToAxis(m_DrawModePoints[NumberOfPoints - 1], CurrentPnt);
        m_DrawModePoints.append(CurrentPnt);

        if (NumberOfPoints == 1)
            return;

        m_DrawModePoints.append(m_DrawModePoints[0] + OdGeVector3d(m_DrawModePoints[2] - m_DrawModePoints[1]));

        OdDbDictionaryPtr GroupDictionary = Database()->getGroupDictionaryId().safeOpenObject(OdDb::kForWrite);
        auto pGroup = OdDbGroup::createObject(); // do not attempt to add entries to the newly created group before adding the group to the group dictionary. 
        GroupDictionary->setAt(L"*", pGroup);

        pGroup->setSelectable(true);
        pGroup->setAnonymous();

        Group = new EoDbGroup;
        for (int i = 0; i < 4; i++) {
            auto Line {EoDbLine::Create(BlockTableRecord)};
            Line->setStartPoint(m_DrawModePoints[i]);
            Line->setEndPoint(m_DrawModePoints[(i + 1) % 4]);

            pGroup->append(Line->objectId());

            Group->AddTail(EoDbLine::Create(Line));
        }
        break;
    }
    case ID_OP5: {
        if (m_DrawModePoints[NumberOfPoints - 1] == CurrentPnt) {
            theApp.AddStringToMessageList(IDS_MSG_PTS_COINCIDE);
            return;
        }
        m_DrawModePoints.append(CurrentPnt);

        if (NumberOfPoints == 1)
            return;

        auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
        OdGeCircArc3d CircularArc(m_DrawModePoints[0], m_DrawModePoints[1], m_DrawModePoints[2]);
        Ellipse->set(CircularArc.center(), CircularArc.normal(), CircularArc.refVec() * CircularArc.radius(), 1., 0., CircularArc.endAng());

        Group = new EoDbGroup;
        Group->AddTail({EoDbEllipse::Create(Ellipse)});
        break;
    }
    case ID_OP6: {
        m_DrawModePoints.append(CurrentPnt);
        const int NumberOfControlPoints = m_DrawModePoints.size();
        Group = new EoDbGroup;
        auto Spline {EoDbSpline::Create(Database())};
        OdGePoint3dArray Points;
        for (int ControlPointIndex = 0; ControlPointIndex < NumberOfControlPoints; ControlPointIndex++) {
            Points.append(m_DrawModePoints[ControlPointIndex]);
        }
        Spline->SetControlPoints(Points);
        Group->AddTail(Spline);
        break;
    }
    case ID_OP7: {
        if (m_DrawModePoints[NumberOfPoints - 1] == CurrentPnt) {
            theApp.AddStringToMessageList(IDS_MSG_PTS_COINCIDE);
            return;
        }
        Group = new EoDbGroup;

        const auto ActiveViewPlaneNormal {GetActiveView()->CameraDirection()};

        auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
        auto MajorAxis {ComputeArbitraryAxis(ActiveViewPlaneNormal)};
        MajorAxis.normalize();
        MajorAxis *= OdGeVector3d(CurrentPnt - m_DrawModePoints[0]).length();

        Ellipse->set(m_DrawModePoints[0], ActiveViewPlaneNormal, MajorAxis, 1.);
        Group->AddTail(EoDbEllipse::Create(Ellipse));

        break;
    }
    case ID_OP8: {
        if (m_DrawModePoints[NumberOfPoints - 1] == CurrentPnt) {
            theApp.AddStringToMessageList(IDS_MSG_PTS_COINCIDE);
            return;
        }
        m_DrawModePoints.append(CurrentPnt);
        if (NumberOfPoints == 1) {
            SetCursorPosition(m_DrawModePoints[0]);
            return;
        }
        const auto MajorAxis {m_DrawModePoints[1] - m_DrawModePoints[0]};
        const auto MinorAxis {m_DrawModePoints[2] - m_DrawModePoints[0]};
        // <tas="Ellipse major and minor axis may not properly define a plane. Memory leaks?"</tas>
        // <tas="Ellipse major must always be longer than minor. Asserts otherwise!"</tas>
        Group = new EoDbGroup;
        auto Ellipse {EoDbEllipse::Create0(BlockTableRecord)};
        Ellipse->SetTo(m_DrawModePoints[0], MajorAxis, MinorAxis, TWOPI);
        Group->AddTail(Ellipse);
        break;
    }
    default:
        return;
    }
    m_PreviewGroup.DeletePrimitivesAndRemoveAll();
    GetDocument()->AddWorkLayerGroup(Group);
    GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);

    m_DrawModePoints.clear();
    ModeLineUnhighlightOp(PreviousDrawCommand);
}

void AeSysView::OnDrawModeEscape() {
    GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);

    m_PreviewGroup.DeletePrimitivesAndRemoveAll();
    m_DrawModePoints.clear();
    ModeLineUnhighlightOp(PreviousDrawCommand);
}

void AeSysView::DoDrawModeMouseMove() {
    auto CurrentPnt {GetCursorPosition()};
    const int NumberOfPoints = m_DrawModePoints.size();

    switch (PreviousDrawCommand) {
    case ID_OP2:
        if (m_DrawModePoints[0] != CurrentPnt) {
            CurrentPnt = SnapPointToAxis(m_DrawModePoints[0], CurrentPnt);
            m_DrawModePoints.append(CurrentPnt);

            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
            m_PreviewGroup.DeletePrimitivesAndRemoveAll();
            m_PreviewGroup.AddTail(new EoDbLine(m_DrawModePoints[0], CurrentPnt));
            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        }
        break;

    case ID_OP3: {
        CurrentPnt = SnapPointToAxis(m_DrawModePoints[NumberOfPoints - 1], CurrentPnt);
        m_DrawModePoints.append(CurrentPnt);

        GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        m_PreviewGroup.DeletePrimitivesAndRemoveAll();
     
        OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
        
        if (NumberOfPoints == 1) {
            auto Line {EoDbLine::Create(BlockTableRecord)};
            Line->setStartPoint(m_DrawModePoints[0]);
            Line->setEndPoint(CurrentPnt);
            m_PreviewGroup.AddTail(EoDbLine::Create(Line));
        } else {
            // <tas="This works for plane normal as long as input does not take current point off the view plane. Once input is off the view plane need to use three points for normal and ensure the inputs are planar."/>
            const auto PlaneNormal {CameraDirection()};

            auto Polyline {EoDbPolyline::Create(BlockTableRecord)};

            OdGeMatrix3d WorldToPlaneTransform;
            WorldToPlaneTransform.setToWorldToPlane(OdGePlane(OdGePoint3d::kOrigin, PlaneNormal));

            for (size_t VertexIndex = 0; VertexIndex < m_DrawModePoints.size(); VertexIndex++) {
                auto Vertex {m_DrawModePoints[VertexIndex]};
                Vertex.transformBy(WorldToPlaneTransform);
                Polyline->addVertexAt(VertexIndex, Vertex.convert2d());
            }
            Polyline->setClosed(true);

            Polyline->setNormal(PlaneNormal);
            Polyline->setElevation(ComputeElevation(m_DrawModePoints[0], PlaneNormal));

            m_PreviewGroup.AddTail(EoDbPolyline::Create(Polyline));
        }
        GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        break;
    }
    case ID_OP4: {
        if (m_DrawModePoints.last() != CurrentPnt) {
            CurrentPnt = SnapPointToAxis(m_DrawModePoints.last(), CurrentPnt);
            m_DrawModePoints.append(CurrentPnt);

            if (NumberOfPoints == 2) {
                m_DrawModePoints.append(m_DrawModePoints[0] + OdGeVector3d(CurrentPnt - m_DrawModePoints[1]));
                m_DrawModePoints.append(m_DrawModePoints[0]);
            }
            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
            m_PreviewGroup.DeletePrimitivesAndRemoveAll();

            for (size_t PointsIndex = 0; PointsIndex < m_DrawModePoints.size() - 1; PointsIndex++) {
                const auto StartPoint {m_DrawModePoints[PointsIndex]};
                const auto EndPoint {m_DrawModePoints[(PointsIndex + 1) % 4]};
                m_PreviewGroup.AddTail(EoDbLine::Create2(StartPoint, EndPoint));
            }
            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        }
        break;
    }
    case ID_OP5:
        m_DrawModePoints.append(CurrentPnt);

        GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        m_PreviewGroup.DeletePrimitivesAndRemoveAll();

        if (NumberOfPoints == 1) {
            m_PreviewGroup.AddTail(new EoDbLine(m_DrawModePoints[0], CurrentPnt));
        }
        if (NumberOfPoints == 2) {
            auto Arc {new EoDbEllipse()};
            Arc->SetTo3PointArc(m_DrawModePoints[0], m_DrawModePoints[1], CurrentPnt);
            Arc->SetColorIndex(pstate.ColorIndex());
            Arc->SetLinetypeIndex(pstate.LinetypeIndex());
            m_PreviewGroup.AddTail(Arc);
        }
        GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        break;

    case ID_OP6:
        if (!m_DrawModePoints[m_DrawModePoints.size() - 1].isEqualTo(CurrentPnt)) {
            m_DrawModePoints.append(CurrentPnt);

            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);

            m_PreviewGroup.DeletePrimitivesAndRemoveAll();
            const int NumberOfControlPoints = m_DrawModePoints.size();
            const int Degree = EoMin(3, NumberOfControlPoints - 1);
            OdGePoint3dArray Points;
            for (int ControlPointIndex = 0; ControlPointIndex < NumberOfControlPoints; ControlPointIndex++) {
                Points.append(m_DrawModePoints[ControlPointIndex]);
            }
            OdGeKnotVector Knots;
            EoGeNurbCurve3d::SetDefaultKnotVector(Degree, Points, Knots);
            OdGeDoubleArray Weights;
            Weights.setLogicalLength(NumberOfControlPoints);
            auto Spline {new EoDbSpline()};
            Spline->Set(Degree, Knots, Points, Weights);
            m_PreviewGroup.AddTail(Spline);
            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        }
        break;
    case ID_OP7:
        if (m_DrawModePoints[0] != CurrentPnt) {
            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
            const auto ActiveViewPlaneNormal {GetActiveView()->CameraDirection()};

            m_PreviewGroup.DeletePrimitivesAndRemoveAll();

            m_PreviewGroup.AddTail(new EoDbEllipse(m_DrawModePoints[0], ActiveViewPlaneNormal, OdGeVector3d(CurrentPnt - m_DrawModePoints[0]).length()));
            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        }
        break;

    case ID_OP8:
        if (m_DrawModePoints[0] != CurrentPnt) {
            m_DrawModePoints.append(CurrentPnt);

            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
            m_PreviewGroup.DeletePrimitivesAndRemoveAll();
            if (NumberOfPoints == 1) {
                m_PreviewGroup.AddTail(new EoDbLine(m_DrawModePoints[0], CurrentPnt));
            } else {
                const auto MajorAxis {m_DrawModePoints[1] - m_DrawModePoints[0]};
                const auto MinorAxis {CurrentPnt - m_DrawModePoints[0]};

                m_PreviewGroup.AddTail(new EoDbEllipse(m_DrawModePoints[0], MajorAxis, MinorAxis, TWOPI));
            }
            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        }
        break;

    }
    m_DrawModePoints.setLogicalLength(NumberOfPoints);
}
