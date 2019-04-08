#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDlgBlockInsert.h"

#include "DbGroup.h"
#include "DbAudit.h"
#include "EoDbEntityToPrimitiveProtocolExtension.h"

OdUInt16 PreviousDrawCommand = 0;

void AeSysView::OnDrawModeOptions() {
    AeSysDoc::GetDoc()->OnSetupOptionsDraw();
}

void AeSysView::OnDrawModePoint() {
    const OdGePoint3d CurrentPnt = GetCursorPosition();
    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
    OdDbPointPtr Point = EoDbPoint::Create(BlockTableRecord);

    Point->setPosition(CurrentPnt);

    EoDbGroup* Group = new EoDbGroup;
    Group->AddTail(EoDbPoint::Create(Point));
    GetDocument()->AddWorkLayerGroup(Group);
    GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
}

void AeSysView::OnDrawModeLine() {
    OdGePoint3d CurrentPnt = GetCursorPosition();

    if (PreviousDrawCommand != ID_OP2) {
        PreviousDrawCommand = ModeLineHighlightOp(ID_OP2);
        m_DrawModePoints.clear();
        m_DrawModePoints.append(CurrentPnt);
    } else {
        CurrentPnt = SnapPointToAxis(m_DrawModePoints[0], CurrentPnt);
        OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
        OdDbLinePtr Line = EoDbLine::Create(BlockTableRecord);
        Line->setStartPoint(m_DrawModePoints[0]);
        Line->setEndPoint(CurrentPnt);

        EoDbGroup* Group = new EoDbGroup;
        Group->AddTail(EoDbLine::Create(Line));
        GetDocument()->AddWorkLayerGroup(Group);

        m_DrawModePoints[0] = CurrentPnt;
        m_PreviewGroup.DeletePrimitivesAndRemoveAll();
    }
}

void AeSysView::OnDrawModePolygon() {
    OdGePoint3d CurrentPnt = GetCursorPosition();

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
    const OdGePoint3d CurrentPnt = GetCursorPosition();

    if (PreviousDrawCommand != ID_OP4) {
        PreviousDrawCommand = ModeLineHighlightOp(ID_OP4);
        m_DrawModePoints.clear();
        m_DrawModePoints.append(CurrentPnt);
    } else {
        OnDrawModeReturn();
    }
}

void AeSysView::OnDrawModeArc() {
    const OdGePoint3d CurrentPnt = GetCursorPosition();

    if (PreviousDrawCommand != ID_OP5) {
        PreviousDrawCommand = ModeLineHighlightOp(ID_OP5);
        m_DrawModePoints.clear();
        m_DrawModePoints.append(CurrentPnt);
    } else {
        OnDrawModeReturn();
    }
}

void AeSysView::OnDrawModeBspline() {
    const OdGePoint3d CurrentPnt = GetCursorPosition();

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
    const OdGePoint3d CurrentPnt = GetCursorPosition();

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
    const OdGePoint3d CurrentPnt = GetCursorPosition();

    if (PreviousDrawCommand != ID_OP8) {
        PreviousDrawCommand = ModeLineHighlightOp(ID_OP8);
        m_DrawModePoints.clear();
        m_DrawModePoints.append(CurrentPnt);
    } else {
        OnDrawModeReturn();
    }
}

void AeSysView::OnDrawModeInsert() {
    AeSysDoc* Document = GetDocument();

    if (Document->BlockTableSize() > 0) {

        EoDlgBlockInsert Dialog(Document);
        Dialog.DoModal();
    }
}

void AeSysView::OnDrawModeReturn() {
    OdGePoint3d CurrentPnt = GetCursorPosition();

    const int NumberOfPoints = m_DrawModePoints.size();
    EoDbGroup* Group = 0;

    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    switch (PreviousDrawCommand) {
    case ID_OP2: {
        CurrentPnt = SnapPointToAxis(m_DrawModePoints[0], CurrentPnt);
        OdDbLinePtr Line = EoDbLine::Create(BlockTableRecord);
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
        EoDbHatch* Hatch = EoDbHatch::Create(Database());
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
        OdDbGroupPtr pGroup = OdDbGroup::createObject(); // do not attempt to add entries to the newly created group before adding the group to the group dictionary. 
        GroupDictionary->setAt(L"*", pGroup);

        pGroup->setSelectable(true);
        pGroup->setAnonymous();

        Group = new EoDbGroup;
        for (int i = 0; i < 4; i++) {
            OdDbLinePtr Line = EoDbLine::Create(BlockTableRecord);
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

        EoDbEllipse* Arc = EoDbEllipse::Create(Database());
        Arc->SetTo3PointArc(m_DrawModePoints[0], m_DrawModePoints[1], m_DrawModePoints[2]);
        Arc->SetColorIndex(pstate.ColorIndex());
        Arc->SetLinetypeIndex(pstate.LinetypeIndex());

        if (Arc->SweepAngle() == 0.) {
            delete Arc;
            theApp.AddStringToMessageList(IDS_MSG_PTS_COLINEAR);
            return;
        }
        Group = new EoDbGroup;
        Group->AddTail(Arc);
        break;
    }
    case ID_OP6: {
        m_DrawModePoints.append(CurrentPnt);
        const int NumberOfControlPoints = m_DrawModePoints.size();
        Group = new EoDbGroup;
        EoDbSpline* Spline = EoDbSpline::Create(Database());
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

        const OdGeVector3d ActiveViewPlaneNormal = GetActiveView()->CameraDirection();

        auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
        auto MajorAxis = ComputeArbitraryAxis(ActiveViewPlaneNormal);
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
        const OdGeVector3d MajorAxis(m_DrawModePoints[1] - m_DrawModePoints[0]);
        const OdGeVector3d MinorAxis(m_DrawModePoints[2] - m_DrawModePoints[0]);
        // <tas="Ellipse major and minor axis may not properly define a plane. Memory leaks?"</tas>
        // <tas="Ellipse major must always be longer than minor. Asserts otherwise!"</tas>
        Group = new EoDbGroup;
        EoDbEllipse* Ellipse = EoDbEllipse::Create(Database());
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
    OdGePoint3d CurrentPnt = GetCursorPosition();
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

    case ID_OP3:
        CurrentPnt = SnapPointToAxis(m_DrawModePoints[NumberOfPoints - 1], CurrentPnt);
        m_DrawModePoints.append(CurrentPnt);

        GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        m_PreviewGroup.DeletePrimitivesAndRemoveAll();
        if (NumberOfPoints == 1) {
            m_PreviewGroup.AddTail(new EoDbLine(m_DrawModePoints[0], CurrentPnt));
        } else {
            const OdGeVector3d ActiveViewPlaneNormal = GetActiveView()->CameraDirection();

            OdGeMatrix3d WorldToPlaneTransform;
            OdGePlane Plane(m_DrawModePoints[0], ActiveViewPlaneNormal);

            WorldToPlaneTransform.setToWorldToPlane(Plane);

            OdGePoint3d WorldOriginOnPlane = OdGePoint3d::kOrigin.orthoProject(Plane);
            OdGeVector3d PointToPlaneVector(WorldOriginOnPlane.asVector());
            PointToPlaneVector.transformBy(WorldToPlaneTransform);

            const double Elevation = PointToPlaneVector.z;

            WorldToPlaneTransform.setToWorldToPlane(OdGePlane(OdGePoint3d::kOrigin, ActiveViewPlaneNormal));

            EoDbPolyline* Polyline = new EoDbPolyline();
            Polyline->SetNormal(ActiveViewPlaneNormal);
            Polyline->SetElevation(Elevation);

            for (size_t VertexIndex = 0; VertexIndex < m_DrawModePoints.size(); VertexIndex++) {
                OdGePoint3d Vertex = m_DrawModePoints[VertexIndex];
                Vertex.transformBy(WorldToPlaneTransform);
                Polyline->AppendVertex(Vertex.convert2d());
            }
            Polyline->SetClosed(true);
            m_PreviewGroup.AddTail(Polyline);
        }
        GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        break;

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
                const OdGePoint3d StartPoint = m_DrawModePoints[PointsIndex];
                const OdGePoint3d EndPoint = m_DrawModePoints[(PointsIndex + 1) % 4];
                m_PreviewGroup.AddTail(EoDbLine::Create(StartPoint, EndPoint));
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
            EoDbEllipse* Arc = new EoDbEllipse();
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
            EoDbSpline* Spline = new EoDbSpline();
            Spline->Set(Degree, Knots, Points, Weights);
            m_PreviewGroup.AddTail(Spline);
            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        }
        break;
    case ID_OP7:
        if (m_DrawModePoints[0] != CurrentPnt) {
            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
            const OdGeVector3d ActiveViewPlaneNormal = GetActiveView()->CameraDirection();

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
                const OdGeVector3d MajorAxis(m_DrawModePoints[1] - m_DrawModePoints[0]);
                const OdGeVector3d MinorAxis(CurrentPnt - m_DrawModePoints[0]);

                m_PreviewGroup.AddTail(new EoDbEllipse(m_DrawModePoints[0], MajorAxis, MinorAxis, TWOPI));
            }
            GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
        }
        break;

    }
    m_DrawModePoints.setLogicalLength(NumberOfPoints);
}
