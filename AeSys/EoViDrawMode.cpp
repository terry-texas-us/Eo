#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDlgBlockInsert.h"

EoUInt16 PreviousDrawCommand = 0;

void AeSysView::OnDrawModeOptions() {
	AeSysDoc::GetDoc()->OnSetupOptionsDraw();
}

void AeSysView::OnDrawModePoint() {
	OdGePoint3d CurrentPnt = GetCursorPosition();

	OdDbPointPtr Point = EoDbPoint::Create(Database(), Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite));
	Point->setPosition(CurrentPnt);

	EoDbGroup* Group = new EoDbGroup;
	EoDbPoint* PointPrimitive = EoDbPoint::Create(Point);
	
	Group->AddTail(PointPrimitive);
	GetDocument()->AddWorkLayerGroup(Group);
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
}

void AeSysView::OnDrawModeLine() {
	OdGePoint3d CurrentPnt = GetCursorPosition();
	if (PreviousDrawCommand != ID_OP2) {
		m_DrawModePoints.clear();
		m_DrawModePoints.append(CurrentPnt);
		PreviousDrawCommand = ModeLineHighlightOp(ID_OP2);
	}
	else {
		CurrentPnt = SnapPointToAxis(m_DrawModePoints[0], CurrentPnt);

		OdDbLinePtr Line = EoDbLine::Create(Database(), Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite));
		Line->setStartPoint(m_DrawModePoints[0]);
		Line->setEndPoint(CurrentPnt);

		EoDbGroup* Group = new EoDbGroup;
		EoDbLine* LinePrimitive = EoDbLine::Create(Line);

		Group->AddTail(LinePrimitive);
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
	}
	else {
		int NumberOfPoints = m_DrawModePoints.size();

		if (m_DrawModePoints[NumberOfPoints - 1] != CurrentPnt) {
			CurrentPnt = SnapPointToAxis(m_DrawModePoints[NumberOfPoints - 1], CurrentPnt);
			m_DrawModePoints.append(CurrentPnt);
		}
	}
}

void AeSysView::OnDrawModeQuad() {
	OdGePoint3d CurrentPnt = GetCursorPosition();

	if (PreviousDrawCommand != ID_OP4) {
		PreviousDrawCommand = ModeLineHighlightOp(ID_OP4);
		m_DrawModePoints.clear();
		m_DrawModePoints.append(CurrentPnt);
	}
	else {
		OnDrawModeReturn();
	}
}

void AeSysView::OnDrawModeArc() {
	OdGePoint3d CurrentPnt = GetCursorPosition();

	if (PreviousDrawCommand != ID_OP5) {
		PreviousDrawCommand = ModeLineHighlightOp(ID_OP5);
		m_DrawModePoints.clear();
		m_DrawModePoints.append(CurrentPnt);
	}
	else {
		OnDrawModeReturn();
	}
}

void AeSysView::OnDrawModeBspline() {
	OdGePoint3d CurrentPnt = GetCursorPosition();

	if (PreviousDrawCommand != ID_OP6) {
		PreviousDrawCommand = ModeLineHighlightOp(ID_OP6);

		m_DrawModePoints.clear();
		m_DrawModePoints.append(CurrentPnt);
	}
	else {
		if (!m_DrawModePoints[m_DrawModePoints.size() - 1].isEqualTo(CurrentPnt)) {
		m_DrawModePoints.append(CurrentPnt);
	}
}
}
void AeSysView::OnDrawModeCircle() {
	OdGePoint3d CurrentPnt = GetCursorPosition();

	if (PreviousDrawCommand != ID_OP7) {
		PreviousDrawCommand = ModeLineHighlightOp(ID_OP7);
		m_DrawModePoints.clear();
		m_DrawModePoints.append(CurrentPnt);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	}
	else {
		OnDrawModeReturn();
	}
}

void AeSysView::OnDrawModeEllipse() {
	OdGePoint3d CurrentPnt = GetCursorPosition();

	if (PreviousDrawCommand != ID_OP8) {
		PreviousDrawCommand = ModeLineHighlightOp(ID_OP8);
		m_DrawModePoints.clear();
		m_DrawModePoints.append(CurrentPnt);
	}
	else {
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

	int NumberOfPoints = m_DrawModePoints.size();
	EoDbGroup* Group = 0;

	switch (PreviousDrawCommand) {
	case ID_OP2: {
		CurrentPnt = SnapPointToAxis(m_DrawModePoints[0], CurrentPnt);
		Group = new EoDbGroup;
		EoDbLine* Line = EoDbLine::Create(Database());
		Line->SetTo(m_DrawModePoints[0], CurrentPnt);
		Group->AddTail(Line);
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
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
		break;
	}
	case ID_OP4:
		if (m_DrawModePoints[NumberOfPoints - 1] == CurrentPnt) {
			theApp.AddStringToMessageList(IDS_MSG_PTS_COINCIDE);
			return;
		}
		CurrentPnt = SnapPointToAxis(m_DrawModePoints[NumberOfPoints - 1], CurrentPnt);
		m_DrawModePoints.append(CurrentPnt);

		if (NumberOfPoints == 1)
			return;

		m_DrawModePoints.append(m_DrawModePoints[0] + OdGeVector3d(m_DrawModePoints[2] - m_DrawModePoints[1]));

		Group = new EoDbGroup;
		EoDbLine* Line;
		for (int i = 0; i < 4; i++) {
			Line = EoDbLine::Create(Database());
			Line->SetTo(m_DrawModePoints[i], m_DrawModePoints[(i + 1) % 4]);
			Group->AddTail(Line);
		}
		break;

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
		int NumberOfControlPoints = m_DrawModePoints.size();
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
		OdGeVector3d ActiveViewPlaneNormal = GetActiveView()->CameraDirection();

		Group = new EoDbGroup;
		EoDbEllipse* Circle = EoDbEllipse::Create(Database());
		Circle->SetToCircle(m_DrawModePoints[0], ActiveViewPlaneNormal, OdGeVector3d(CurrentPnt - m_DrawModePoints[0]).length()); 
		Group->AddTail(Circle);
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
		OdGeVector3d MajorAxis(m_DrawModePoints[1] - m_DrawModePoints[0]);
		OdGeVector3d MinorAxis(m_DrawModePoints[2] - m_DrawModePoints[0]);
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
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);

	m_DrawModePoints.clear();
	ModeLineUnhighlightOp(PreviousDrawCommand);
}

void AeSysView::OnDrawModeEscape() {
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);

	m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	m_DrawModePoints.clear();
	ModeLineUnhighlightOp(PreviousDrawCommand);
}

void AeSysView::DoDrawModeMouseMove() {
	OdGePoint3d CurrentPnt = GetCursorPosition();
	int NumberOfPoints = m_DrawModePoints.size();

	switch (PreviousDrawCommand) {
	case ID_OP2:
		VERIFY(m_DrawModePoints.size() > 0);

		if (m_DrawModePoints[0] != CurrentPnt) {
			CurrentPnt = SnapPointToAxis(m_DrawModePoints[0], CurrentPnt);
			m_DrawModePoints.append(CurrentPnt);

			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
			m_PreviewGroup.DeletePrimitivesAndRemoveAll();
			m_PreviewGroup.AddTail(new EoDbLine(m_DrawModePoints[0], CurrentPnt));
			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		}
		break;

	case ID_OP3:
		CurrentPnt = SnapPointToAxis(m_DrawModePoints[NumberOfPoints - 1], CurrentPnt);
		m_DrawModePoints.append(CurrentPnt);

		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
		if (NumberOfPoints == 1) {
			m_PreviewGroup.AddTail(new EoDbLine(m_DrawModePoints[0], CurrentPnt));
		}
		else {
			OdGeVector3d ActiveViewPlaneNormal = GetActiveView()->CameraDirection();

			OdGeMatrix3d WorldToPlaneTransform;
			OdGePlane Plane(m_DrawModePoints[0], ActiveViewPlaneNormal);
			
			WorldToPlaneTransform.setToWorldToPlane(Plane);

			OdGePoint3d WorldOriginOnPlane = OdGePoint3d::kOrigin.orthoProject(Plane);
			OdGeVector3d PointToPlaneVector(WorldOriginOnPlane.asVector());
			PointToPlaneVector.transformBy(WorldToPlaneTransform);
	
			double Elevation = PointToPlaneVector.z;
		
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
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		break;

	case ID_OP4: {
		CurrentPnt = SnapPointToAxis(m_DrawModePoints[NumberOfPoints - 1], CurrentPnt);
		m_DrawModePoints.append(CurrentPnt);

		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		if (NumberOfPoints == 2) {
			m_DrawModePoints.append(m_DrawModePoints[0] + OdGeVector3d(CurrentPnt - m_DrawModePoints[1]));
			m_DrawModePoints.append(m_DrawModePoints[0]);
		}
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();

		OdGePoint3dArray Points;
		Points.setLogicalLength(m_DrawModePoints.size());
		for (size_t PointsIndex = 0; PointsIndex < m_DrawModePoints.size(); PointsIndex++) {
			Points[PointsIndex] = m_DrawModePoints[PointsIndex];
		}
		EoDbPolyline* Polyline = new EoDbPolyline();
		Polyline->SetPoints(Points);
		m_PreviewGroup.AddTail(Polyline);

		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		break;
	}
	case ID_OP5:
		m_DrawModePoints.append(CurrentPnt);

		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
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
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		break;

	case ID_OP6:
		if (!m_DrawModePoints[m_DrawModePoints.size() - 1].isEqualTo(CurrentPnt)) {
			m_DrawModePoints.append(CurrentPnt);

			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);

			m_PreviewGroup.DeletePrimitivesAndRemoveAll();
			int NumberOfControlPoints = m_DrawModePoints.size();
			int Degree = EoMin(3, NumberOfControlPoints - 1);
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
			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		}
		break;
	case ID_OP7:
		if (m_DrawModePoints[0] != CurrentPnt) {
			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
			OdGeVector3d ActiveViewPlaneNormal = GetActiveView()->CameraDirection();

			m_PreviewGroup.DeletePrimitivesAndRemoveAll();

			m_PreviewGroup.AddTail(new EoDbEllipse(m_DrawModePoints[0], ActiveViewPlaneNormal, OdGeVector3d(CurrentPnt - m_DrawModePoints[0]).length()));
			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		}
		break;

	case ID_OP8:
		if (m_DrawModePoints[0] != CurrentPnt) {
			m_DrawModePoints.append(CurrentPnt);

			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
			m_PreviewGroup.DeletePrimitivesAndRemoveAll();
			if (NumberOfPoints == 1) {
				m_PreviewGroup.AddTail(new EoDbLine(m_DrawModePoints[0], CurrentPnt));
			}
			else {
				OdGeVector3d MajorAxis(m_DrawModePoints[1] - m_DrawModePoints[0]);
				OdGeVector3d MinorAxis(CurrentPnt - m_DrawModePoints[0]);

				m_PreviewGroup.AddTail(new EoDbEllipse(m_DrawModePoints[0], MajorAxis, MinorAxis, TWOPI));
			}
			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		}
		break;

	}
	m_DrawModePoints.setLogicalLength(NumberOfPoints);
}
