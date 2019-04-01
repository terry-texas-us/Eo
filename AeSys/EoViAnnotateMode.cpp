#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDlgAnnotateOptions.h"
#include "EoDlgSetText.h"

OdGePoint3dArray EoViAnn_points;

void AeSysView::OnAnnotateModeOptions() {
	EoDlgAnnotateOptions Dialog(this);

	if (Dialog.DoModal() == IDOK) {
	}
}

void AeSysView::OnAnnotateModeLine() {
	OdGePoint3d CurrentPnt = GetCursorPosition();
	if (m_PreviousOp == 0) {
		EoViAnn_points.clear();
		EoViAnn_points.append(CurrentPnt);
	}
	else {
		if (CorrectLeaderEndpoints(m_PreviousOp, ID_OP2, EoViAnn_points[0], CurrentPnt)) {
			EoDbGroup* Group = new EoDbGroup;
			GetDocument()->AddWorkLayerGroup(Group);

			if (m_PreviousOp == ID_OP3) {
				GenerateLineEndItem(EndItemType(), EndItemSize(), CurrentPnt, EoViAnn_points[0], Group);
			}
			EoDbLine* Line = EoDbLine::Create(Database());
			Line->SetTo(EoViAnn_points[0], CurrentPnt);
			Line->SetColorIndex(1);
			Line->SetLinetypeIndex(1);
			Group->AddTail(Line);
			EoViAnn_points[0] = CurrentPnt;
			m_PreviewGroup.DeletePrimitivesAndRemoveAll();
		}
	}
	m_PreviousOp = ModeLineHighlightOp(ID_OP2);
}

void AeSysView::OnAnnotateModeArrow() {
	OdGePoint3d CurrentPnt = GetCursorPosition();
	if (m_PreviousOp == 0) {
		EoViAnn_points.clear();
		EoViAnn_points.append(CurrentPnt);
	}
	else {
		if (CorrectLeaderEndpoints(m_PreviousOp, ID_OP3, EoViAnn_points[0], CurrentPnt)) {
			EoDbGroup* Group = new EoDbGroup;

			if (m_PreviousOp == ID_OP3) {
				GenerateLineEndItem(EndItemType(), EndItemSize(), CurrentPnt, EoViAnn_points[0], Group);
			}
			EoDbLine* Line = EoDbLine::Create(Database());
			Line->SetTo(EoViAnn_points[0], CurrentPnt);
			Line->SetColorIndex(1);
			Line->SetLinetypeIndex(1);
			Group->AddTail(Line);
			GenerateLineEndItem(EndItemType(), EndItemSize(), EoViAnn_points[0], CurrentPnt, Group);
			GetDocument()->AddWorkLayerGroup(Group);
			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
			EoViAnn_points[0] = CurrentPnt;
			m_PreviewGroup.DeletePrimitivesAndRemoveAll();
		}
	}
	m_PreviousOp = ModeLineHighlightOp(ID_OP3);
}

void AeSysView::OnAnnotateModeBubble() {
	static CString CurrentText;
	OdGePoint3d CurrentPnt = GetCursorPosition();

	EoDlgSetText dlg;
	dlg.m_strTitle = L"Set Bubble Text";
	dlg.m_sText = CurrentText;
	if (dlg.DoModal() == IDOK) {
		CurrentText = dlg.m_sText;
	}
	EoDbGroup* Group = new EoDbGroup;
	GetDocument()->AddWorkLayerGroup(Group);
	if (m_PreviousOp == 0) { // No operation pending
		EoViAnn_points.clear();
		EoViAnn_points.append(CurrentPnt);
	}
	else {
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();

		OdGePoint3d pt(CurrentPnt);

		if (CorrectLeaderEndpoints(m_PreviousOp, ID_OP4, EoViAnn_points[0], pt)) {
			if (m_PreviousOp == ID_OP3) {
				GenerateLineEndItem(EndItemType(), EndItemSize(), CurrentPnt, EoViAnn_points[0], Group);
			}
			EoDbLine* Line = EoDbLine::Create(Database());
			Line->SetTo(EoViAnn_points[0], pt);
			Line->SetColorIndex(1);
			Line->SetLinetypeIndex(1);
			Group->AddTail(Line);
		}
	}
	m_PreviousOp = ModeLineHighlightOp(ID_OP4);

	if (!CurrentText.IsEmpty()) {
		CDC* DeviceContext = GetDC();

		const OdGeVector3d PlaneNormal = CameraDirection();
		OdGeVector3d MinorAxis = ViewUp();
		OdGeVector3d MajorAxis = MinorAxis;
		MajorAxis.rotateBy(- HALF_PI, PlaneNormal);

		MajorAxis *= .06;
		MinorAxis *= .1;
		EoGeReferenceSystem ReferenceSystem(CurrentPnt, MajorAxis, MinorAxis);

		const int PrimitiveState = pstate.Save();
		pstate.SetColorIndex(DeviceContext, 2);

		EoDbFontDefinition FontDefinition = pstate.FontDefinition();
		FontDefinition.SetHorizontalAlignment(EoDb::kAlignCenter);
		FontDefinition.SetVerticalAlignment(EoDb::kAlignMiddle);

		EoDbCharacterCellDefinition CharacterCellDefinition = pstate.CharacterCellDefinition();
		CharacterCellDefinition.SetRotationAngle(0.);
		pstate.SetCharacterCellDefinition(CharacterCellDefinition);

		EoDbText* TextPrimitive = EoDbText::Create(Database());
		TextPrimitive->SetTo(FontDefinition, ReferenceSystem, CurrentText);
		Group->AddTail(TextPrimitive);
		pstate.Restore(DeviceContext, PrimitiveState);
		ReleaseDC(DeviceContext);
	}
	const OdGeVector3d ActiveViewPlaneNormal = GetActiveView()->CameraDirection();
	if (NumberOfSides() == 0) {
		EoDbEllipse* Circle = EoDbEllipse::Create(Database());
		Circle->SetToCircle(CurrentPnt, ActiveViewPlaneNormal, BubbleRadius());
		Circle->SetColorIndex(1);
		Circle->SetLinetypeIndex(1);
		Group->AddTail(Circle);
	}
	else {
		EoDbPolyline* Polyline = EoDbPolyline::Create(Database());
		Polyline->SetColorIndex(1);
		Polyline->SetLinetypeIndex(1);
		Polyline->SetClosed(true);

		OdGePoint3dArray Points;
		polyline::GeneratePointsForNPoly(CurrentPnt, ActiveViewPlaneNormal, BubbleRadius(), NumberOfSides(), Points);

		const OdGeVector3d ActiveViewPlaneNormal = GetActiveView()->CameraDirection();

		OdGeMatrix3d WorldToPlaneTransform;
		OdGePlane Plane(CurrentPnt, ActiveViewPlaneNormal);
			
		WorldToPlaneTransform.setToWorldToPlane(Plane);

		OdGePoint3d WorldOriginOnPlane = OdGePoint3d::kOrigin.orthoProject(Plane);
		OdGeVector3d PointToPlaneVector(WorldOriginOnPlane.asVector());
		PointToPlaneVector.transformBy(WorldToPlaneTransform);
	
		const double Elevation = PointToPlaneVector.z;

		WorldToPlaneTransform.setToWorldToPlane(OdGePlane(OdGePoint3d::kOrigin, ActiveViewPlaneNormal));

		for (size_t VertexIndex = 0; VertexIndex < Points.size(); VertexIndex++) {
			OdGePoint3d Vertex = Points[VertexIndex];
			Vertex.transformBy(WorldToPlaneTransform);
			Polyline->AppendVertex(Vertex.convert2d());
		}
		Polyline->SetNormal(ActiveViewPlaneNormal);
		Polyline->SetElevation(Elevation);
		Group->AddTail(Polyline);
	}
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
	EoViAnn_points[0] = CurrentPnt;
}

void AeSysView::OnAnnotateModeHook() {
	OdGePoint3d CurrentPnt = GetCursorPosition();
	EoDbGroup* Group = new EoDbGroup;
	if (m_PreviousOp == 0) {
		EoViAnn_points.clear();
		EoViAnn_points.append(CurrentPnt);
	}
	else {
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();

		OdGePoint3d pt(CurrentPnt);

		if (CorrectLeaderEndpoints(m_PreviousOp, ID_OP5, EoViAnn_points[0], pt)) {
			if (m_PreviousOp == ID_OP3)
				GenerateLineEndItem(EndItemType(), EndItemSize(), CurrentPnt, EoViAnn_points[0], Group);

			EoDbLine* Line = EoDbLine::Create(Database());
			Line->SetTo(EoViAnn_points[0], pt);
			Line->SetColorIndex(1);
			Line->SetLinetypeIndex(1);
			Group->AddTail(Line);
		}
	}
	m_PreviousOp = ModeLineHighlightOp(ID_OP5);
	const OdGeVector3d ActiveViewPlaneNormal = GetActiveView()->CameraDirection();
	EoDbEllipse* Circle = EoDbEllipse::Create(Database());
	Circle->SetToCircle(CurrentPnt, ActiveViewPlaneNormal, CircleRadius());
	Circle->SetColorIndex(1);
	Circle->SetLinetypeIndex(1);
	Group->AddTail(Circle);
	GetDocument()->AddWorkLayerGroup(Group);
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
	EoViAnn_points[0] = CurrentPnt;
}

void AeSysView::OnAnnotateModeUnderline() {
	const OdGePoint3d CurrentPnt = GetCursorPosition();

	if (m_PreviousOp != 0) {
		ModeLineUnhighlightOp(m_PreviousOp);
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	}
	EoDbText* pText = SelectTextUsingPoint(CurrentPnt);
	if (pText != 0) {
		OdGePoint3dArray Underline;
		pText->GetBoundingBox(Underline, GapSpaceFactor());

		EoDbGroup* Group = new EoDbGroup;
		EoDbLine* Line = EoDbLine::Create(Database());
		Line->SetTo(Underline[0], Underline[1]);
		Line->SetColorIndex(pstate.ColorIndex());
		Line->SetLinetypeIndex(1);
		Group->AddTail(Line);
		GetDocument()->AddWorkLayerGroup(Group);
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
	}
}

void AeSysView::OnAnnotateModeBox() {
	const OdGePoint3d CurrentPnt = GetCursorPosition();
	if (m_PreviousOp != ID_OP7) {
		if (m_PreviousOp != 0) {
			RubberBandingDisable();
			ModeLineUnhighlightOp(m_PreviousOp);
		}
		m_PreviousOp = ModeLineHighlightOp(ID_OP7);
		EoViAnn_points.clear();
		EoViAnn_points.append(CurrentPnt);
	}
	else {
		OdGePoint3dArray ptsBox1;
		OdGePoint3dArray ptsBox2;
		bool bG1Flg = false;
		bool bG2Flg = false;
		EoDbText* pText = SelectTextUsingPoint(EoViAnn_points[0]);
		if (pText != 0) {
			pText->GetBoundingBox(ptsBox1, GapSpaceFactor());
			bG1Flg = true;
		}
		pText = SelectTextUsingPoint(CurrentPnt);
		if (pText != 0) {
			pText->GetBoundingBox(ptsBox2, GapSpaceFactor());
			bG2Flg = true;
		}
		if (bG1Flg && bG2Flg) {
			OdGePoint3dArray JoinedBoxes;
			JoinedBoxes.setLogicalLength(4);
			JoinedBoxes.setAll(ptsBox1[0]);
			for (int i = 1; i < 4; i++) {
				JoinedBoxes[0].x = EoMin(JoinedBoxes[0].x, ptsBox1[i].x);
				JoinedBoxes[2].x = EoMax(JoinedBoxes[2].x, ptsBox1[i].x);
				JoinedBoxes[0].y = EoMin(JoinedBoxes[0].y, ptsBox1[i].y);
				JoinedBoxes[2].y = EoMax(JoinedBoxes[2].y, ptsBox1[i].y);
			}
			for (int i = 0; i < 4; i++) {
				JoinedBoxes[0].x = EoMin(JoinedBoxes[0].x, ptsBox2[i].x);
				JoinedBoxes[2].x = EoMax(JoinedBoxes[2].x, ptsBox2[i].x);
				JoinedBoxes[0].y = EoMin(JoinedBoxes[0].y, ptsBox2[i].y);
				JoinedBoxes[2].y = EoMax(JoinedBoxes[2].y, ptsBox2[i].y);
			}
			JoinedBoxes[1].x = JoinedBoxes[2].x;
			JoinedBoxes[1].y = JoinedBoxes[0].y;
			JoinedBoxes[3].x = JoinedBoxes[0].x;
			JoinedBoxes[3].y = JoinedBoxes[2].y;

			EoDbGroup* Group = new EoDbGroup;
			EoDbLine* Line;
			for (int i = 0; i < 4; i++) {
				Line = EoDbLine::Create(Database());
				Line->SetTo(JoinedBoxes[i], JoinedBoxes[(i + 1) % 4]);
				Line->SetColorIndex(1);
				Line->SetLinetypeIndex(1);
				Group->AddTail(Line);
			}
			GetDocument()->AddWorkLayerGroup(Group);
			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
		}
		ModeLineUnhighlightOp(m_PreviousOp);
	}
}

void AeSysView::OnAnnotateModeCutIn() {
	CDC* DeviceContext = GetDC();

	OdGePoint3d CurrentPnt = GetCursorPosition();

	EoDbGroup* Group = SelectLineBy(CurrentPnt);
	if (Group != 0) {
		EoDbLine* pLine = static_cast<EoDbLine*>(EngagedPrimitive());

		CurrentPnt = DetPt();

		CString CurrentText;

		EoDlgSetText dlg;
		dlg.m_strTitle = L"Set Cut-in Text";
		dlg.m_sText = CurrentText;
		if (dlg.DoModal() == IDOK) {
			CurrentText = dlg.m_sText;
		}
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, Group);

		const int PrimitiveState = pstate.Save();

		if (!CurrentText.IsEmpty()) {
			EoGeLineSeg3d Line = pLine->Line();
			double dAng = Line.AngleFromXAxis_xy();
			if (dAng > .25 * TWOPI && dAng <  .75 * TWOPI)
				dAng += PI;

			const OdGeVector3d PlaneNormal = CameraDirection();
			OdGeVector3d MinorAxis = ViewUp();
			MinorAxis.rotateBy(dAng, PlaneNormal);
			OdGeVector3d MajorAxis = MinorAxis;
			MajorAxis.rotateBy(- HALF_PI, PlaneNormal);
			MajorAxis *= .06;
			MinorAxis *= .1;
			EoGeReferenceSystem ReferenceSystem(CurrentPnt, MajorAxis, MinorAxis);

			const EoInt16 ColorIndex = pstate.ColorIndex();
			pstate.SetColorIndex(DeviceContext, 2);

			EoDbFontDefinition FontDefinition = pstate.FontDefinition();
			FontDefinition.SetHorizontalAlignment(EoDb::kAlignCenter);
			FontDefinition.SetVerticalAlignment(EoDb::kAlignMiddle);

			EoDbCharacterCellDefinition CharacterCellDefinition = pstate.CharacterCellDefinition();
			CharacterCellDefinition.SetRotationAngle(0.);
			pstate.SetCharacterCellDefinition(CharacterCellDefinition);

			EoDbText* TextPrimitive = EoDbText::Create(Database());
			TextPrimitive->SetTo(FontDefinition, ReferenceSystem, CurrentText);
			pstate.SetColorIndex(DeviceContext, ColorIndex);

			Group->AddTail(TextPrimitive);

			OdGePoint3dArray BoundingBox;
			TextPrimitive->GetBoundingBox(BoundingBox, GapSpaceFactor());

			const double dGap = OdGeVector3d(BoundingBox[1] - BoundingBox[0]).length();

			BoundingBox[0] = ProjectToward(CurrentPnt, pLine->StartPoint(), dGap / 2.);
			BoundingBox[1] = ProjectToward(CurrentPnt, pLine->EndPoint(), dGap / 2.);

			double dRel[2];

			dRel[0] = pLine->ParametricRelationshipOf(BoundingBox[0]);
			dRel[1] = pLine->ParametricRelationshipOf(BoundingBox[1]);

			if (dRel[0] > DBL_EPSILON && dRel[1] < 1. - DBL_EPSILON) {
				EoDbLine* NewLinePrimitive = EoDbLine::Create(*pLine, Database());
				pLine->SetEndPoint(BoundingBox[0]);
				NewLinePrimitive->SetStartPoint(BoundingBox[1]);
				Group->AddTail(NewLinePrimitive);
			}
			else if (dRel[0] <= DBL_EPSILON)
				pLine->SetStartPoint(BoundingBox[1]);
			else if (dRel[1] >= 1. - DBL_EPSILON)
				pLine->SetEndPoint(BoundingBox[0]);

		}
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroup, Group);
		pstate.Restore(DeviceContext, PrimitiveState);
	}
	ReleaseDC(DeviceContext);
}

void AeSysView::OnAnnotateModeConstructionLine() {
	OdGePoint3d CurrentPnt = GetCursorPosition();

	if (m_PreviousOp != ID_OP9) {
		m_PreviousOp = ModeLineHighlightOp(ID_OP9);
		EoViAnn_points.clear();
		EoViAnn_points.append(CurrentPnt);
	}
	else {
		CurrentPnt = SnapPointToAxis(EoViAnn_points[0], CurrentPnt);
		EoViAnn_points.append(ProjectToward(EoViAnn_points[0], CurrentPnt, 48.));
		EoViAnn_points.append(ProjectToward(EoViAnn_points[1], EoViAnn_points[0], 96.));
		EoDbLine* Line = EoDbLine::Create(Database());
		Line->SetTo(EoViAnn_points[1], EoViAnn_points[2]);
		Line->SetColorIndex(15);
		Line->SetLinetypeIndex(2);
		EoDbGroup* Group = new EoDbGroup;
		Group->AddTail(Line);
		GetDocument()->AddWorkLayerGroup(Group);
		ModeLineUnhighlightOp(m_PreviousOp);
		EoViAnn_points.clear();
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	}
}

void AeSysView::OnAnnotateModeReturn() {
	// TODO: Add your command handler code here
}

void AeSysView::OnAnnotateModeEscape() {
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);

	m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	EoViAnn_points.clear();
	ModeLineUnhighlightOp(m_PreviousOp);
}

bool AeSysView::CorrectLeaderEndpoints(int beginType, int endType, OdGePoint3d& startPoint, OdGePoint3d& endPoint) const {
	const double LineSegmentLength = OdGeVector3d(endPoint - startPoint).length();

	double BeginDistance = 0.;

	if (beginType == ID_OP4) {
		BeginDistance = BubbleRadius();
	}
	else if (beginType == ID_OP5) {
		BeginDistance = CircleRadius();
	}
	double EndDistance = 0.;

	if (endType == ID_OP4) {
		EndDistance = BubbleRadius();
	}
	else if (endType == ID_OP5)
		EndDistance = CircleRadius();

	if (LineSegmentLength > BeginDistance + EndDistance + DBL_EPSILON) {
		if (BeginDistance != 0.)
			startPoint = ProjectToward(startPoint, endPoint, BeginDistance);
		if (EndDistance != 0.)
			endPoint = ProjectToward(endPoint, startPoint, EndDistance);
		return true;
	}
	else {
		theApp.AddModeInformationToMessageList();
		return false;
	}
}
double AeSysView::BubbleRadius() const {
	return m_BubbleRadius;
}
void AeSysView::SetBubbleRadius(double radius) {
	m_BubbleRadius = radius;
}
double AeSysView::CircleRadius() const {
	return m_CircleRadius;
}
void AeSysView::SetCircleRadius(double radius) {
	m_CircleRadius = radius;
}
CString AeSysView::DefaultText() const {
	return m_DefaultText;
}
void AeSysView::SetDefaultText(const CString& text) {
	m_DefaultText = text;
}
double AeSysView::EndItemSize() const {
	return m_EndItemSize;
}
void AeSysView::SetEndItemSize(double size) {
	m_EndItemSize = size;
}
int AeSysView::EndItemType() {
	return m_EndItemType;
}
void AeSysView::SetEndItemType(int type) {
	m_EndItemType = type;
}
double AeSysView::GapSpaceFactor() const {
	return m_GapSpaceFactor;
}
void AeSysView::SetGapSpaceFactor(double factor) {
	m_GapSpaceFactor = factor;
}
int AeSysView::NumberOfSides() const {
	return m_NumberOfSides;
}
void AeSysView::SetNumberOfSides(int number) {
	m_NumberOfSides = number;
}

void AeSysView::DoAnnotateModeMouseMove() {
	OdGePoint3d CurrentPnt = GetCursorPosition();
	const int NumberOfPoints = EoViAnn_points.size();
	EoViAnn_points.append(CurrentPnt);

	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
	m_PreviewGroup.DeletePrimitivesAndRemoveAll();

	switch(m_PreviousOp) {
	case ID_OP2:
	case ID_OP3:
		if (EoViAnn_points[0] != CurrentPnt) {
			if (m_PreviousOp == ID_OP3) {
				GenerateLineEndItem(EndItemType(), EndItemSize(), CurrentPnt, EoViAnn_points[0], &m_PreviewGroup);
			}
			m_PreviewGroup.AddTail(new EoDbLine(EoViAnn_points[0], CurrentPnt));
			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		}
		break;

	case ID_OP4:
	case ID_OP5: {
		OdGePoint3d m_PreviousPnt(EoViAnn_points[0]);
		if (CorrectLeaderEndpoints(m_PreviousOp, 0, EoViAnn_points[0], EoViAnn_points[1])) {
			m_PreviewGroup.AddTail(new EoDbLine(EoViAnn_points[0], EoViAnn_points[1]));
			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		}
		EoViAnn_points[0] = m_PreviousPnt;
		break;
				 }
	case ID_OP9:
		if (EoViAnn_points[0] != CurrentPnt) {
			CurrentPnt = SnapPointToAxis(EoViAnn_points[0], CurrentPnt);

			EoViAnn_points.append(ProjectToward(EoViAnn_points[0], CurrentPnt, 48.));
			EoViAnn_points.append(ProjectToward(EoViAnn_points[2], EoViAnn_points[0], 96.));

			EoDbLine* Line = new EoDbLine(EoViAnn_points[2], EoViAnn_points[3]);
			Line->SetColorIndex(15);
			Line->SetLinetypeIndex(2);
			m_PreviewGroup.AddTail(Line);
			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		}
		break;

	}
	EoViAnn_points.setLogicalLength(NumberOfPoints);
}
void AeSysView::GenerateLineEndItem(int type, double size, const OdGePoint3d& startPoint, const OdGePoint3d& endPoint, EoDbGroup* group) {
	const OdGeVector3d PlaneNormal = CameraDirection();

	const OdGePoint3d EndPoint = endPoint;

	OdGePoint3dArray ItemPoints;
	ItemPoints.clear();

	if (type == 1 || type == 2) {
		const double Angle = .244978663127;
		const double Size = size / .970142500145;

		OdGePoint3d BasePoint(ProjectToward(EndPoint, startPoint, Size));
		ItemPoints.append(BasePoint.rotateBy(Angle, PlaneNormal, endPoint));
		ItemPoints.append(endPoint);
		ItemPoints.append(BasePoint.rotateBy(- 2. * Angle, PlaneNormal, endPoint));
		EoDbPolyline* Polyline = EoDbPolyline::Create(Database());
		Polyline->SetColorIndex(1);
		Polyline->SetLinetypeIndex(1);
		Polyline->SetPoints(ItemPoints);
		if (type == 2) {
			Polyline->SetClosed(true);
		}
		group->AddTail(Polyline);
	}
	else if (type == 3) {
		const double Angle = 9.96686524912e-2;
		const double Size = size / .99503719021;

		OdGePoint3d BasePoint(ProjectToward(EndPoint, startPoint, Size));
		ItemPoints.append(BasePoint.rotateBy(Angle, PlaneNormal, endPoint));
		ItemPoints.append(endPoint);
		EoDbPolyline* Polyline = EoDbPolyline::Create(Database());
		Polyline->SetColorIndex(1);
		Polyline->SetLinetypeIndex(1);
		Polyline->SetPoints(ItemPoints);

		group->AddTail(Polyline);
	}
	else if (type == 4) {
		const double Angle = .785398163397;
		const double Size = .5 * size / .707106781187;

		OdGePoint3d BasePoint(ProjectToward(EndPoint, startPoint, Size));
		ItemPoints.append(BasePoint.rotateBy(Angle, PlaneNormal, endPoint));
		ItemPoints.append(BasePoint.rotateBy(PI, PlaneNormal, endPoint));
		EoDbPolyline* Polyline = EoDbPolyline::Create(Database());
		Polyline->SetColorIndex(1);
		Polyline->SetLinetypeIndex(1);
		Polyline->SetPoints(ItemPoints);
		group->AddTail(Polyline);
	}
}
