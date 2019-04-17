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
            OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
            auto Line {EoDbLine::Create(BlockTableRecord)};
            Line->setStartPoint(EoViAnn_points[0]);
            Line->setEndPoint(CurrentPnt);
            Line->setColorIndex(1);
            Line->setLinetype(L"Continuous");
           
            Group->AddTail(EoDbLine::Create(Line));
            
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
            OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
            auto Line {EoDbLine::Create(BlockTableRecord)};
            Line->setStartPoint(EoViAnn_points[0]);
            Line->setEndPoint(CurrentPnt);
            Line->setColorIndex(1);
            Line->setLinetype(L"Continuous");

            Group->AddTail(EoDbLine::Create(Line));

            GenerateLineEndItem(EndItemType(), EndItemSize(), EoViAnn_points[0], CurrentPnt, Group);
			GetDocument()->AddWorkLayerGroup(Group);
			GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
			EoViAnn_points[0] = CurrentPnt;
			m_PreviewGroup.DeletePrimitivesAndRemoveAll();
		}
	}
	m_PreviousOp = ModeLineHighlightOp(ID_OP3);
}

void AeSysView::OnAnnotateModeBubble() {
	static CString CurrentText;
	OdGePoint3d CurrentPnt = GetCursorPosition();

    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

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
		GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();

		OdGePoint3d pt(CurrentPnt);

		if (CorrectLeaderEndpoints(m_PreviousOp, ID_OP4, EoViAnn_points[0], pt)) {
			if (m_PreviousOp == ID_OP3) {
				GenerateLineEndItem(EndItemType(), EndItemSize(), CurrentPnt, EoViAnn_points[0], Group);
			}
            auto Line {EoDbLine::Create(BlockTableRecord)};
            Line->setStartPoint(EoViAnn_points[0]);
            Line->setEndPoint(pt);
            Line->setColorIndex(1);
            Line->setLinetype(L"Continuous");

            Group->AddTail(EoDbLine::Create(Line));
		}
	}
	m_PreviousOp = ModeLineHighlightOp(ID_OP4);

    const auto ActiveViewPlaneNormal = GetActiveView()->CameraDirection();
    auto MajorAxis = ComputeArbitraryAxis(ActiveViewPlaneNormal);
    MajorAxis.normalize();

    if (!CurrentText.IsEmpty()) {
        OdGeVector3d MinorAxis {MajorAxis};
		MinorAxis.rotateBy(HALF_PI, ActiveViewPlaneNormal);

		EoGeReferenceSystem ReferenceSystem(CurrentPnt, MajorAxis *.06, MinorAxis * .1);

        OdDbTextPtr Text = EoDbText::Create(BlockTableRecord, ReferenceSystem.Origin(), (LPCWSTR) CurrentText);

        Text->setNormal(ActiveViewPlaneNormal);
        Text->setRotation(ReferenceSystem.Rotation());
        Text->setHeight(ReferenceSystem.YDirection().length());
        Text->setAlignmentPoint(ReferenceSystem.Origin());
        Text->setColorIndex(2);
        Text->setHorizontalMode(EoDbText::ConvertHorizontalMode(kAlignCenter));
        Text->setVerticalMode(EoDbText::ConvertVerticalMode(kAlignMiddle));

        Group->AddTail(EoDbText::Create(Text));
	}
    if (NumberOfSides() == 0) {
        auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
        Ellipse->setColorIndex(1);
        Ellipse->setLinetype(L"Continuous");

        Ellipse->set(CurrentPnt, ActiveViewPlaneNormal, MajorAxis * BubbleRadius(), 1.);
        Group->AddTail(EoDbEllipse::Create(Ellipse));
	}
    else {
        auto Polyline {EoDbPolyline::Create(BlockTableRecord)};

        Polyline->setColorIndex(1);
        Polyline->setLinetype(L"Continuous");
        Polyline->setClosed(true);

        OdGePoint3dArray Points;
        polyline::GeneratePointsForNPoly(CurrentPnt, ActiveViewPlaneNormal, BubbleRadius(), NumberOfSides(), Points);

        OdGeMatrix3d WorldToPlaneTransform;
        WorldToPlaneTransform.setToWorldToPlane(OdGePlane(OdGePoint3d::kOrigin, ActiveViewPlaneNormal));

		for (size_t VertexIndex = 0; VertexIndex < Points.size(); VertexIndex++) {
			auto Vertex = Points[VertexIndex];
			Vertex.transformBy(WorldToPlaneTransform);
            Polyline->addVertexAt(VertexIndex, Vertex.convert2d());
		}
		Polyline->setNormal(ActiveViewPlaneNormal);
        Polyline->setElevation(ComputeElevation(CurrentPnt, ActiveViewPlaneNormal));

        Group->AddTail(EoDbPolyline::Create(Polyline));
	}
	GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
	EoViAnn_points[0] = CurrentPnt;
}

void AeSysView::OnAnnotateModeHook() {
	OdGePoint3d CurrentPnt = GetCursorPosition();
	EoDbGroup* Group = new EoDbGroup;

    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    if (m_PreviousOp == 0) {
		EoViAnn_points.clear();
		EoViAnn_points.append(CurrentPnt);
	}
	else {
		GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();

		OdGePoint3d pt(CurrentPnt);

		if (CorrectLeaderEndpoints(m_PreviousOp, ID_OP5, EoViAnn_points[0], pt)) {
            if (m_PreviousOp == ID_OP3) {
                GenerateLineEndItem(EndItemType(), EndItemSize(), CurrentPnt, EoViAnn_points[0], Group);
            }
            auto Line {EoDbLine::Create(BlockTableRecord)};
            Line->setStartPoint(EoViAnn_points[0]);
            Line->setEndPoint(pt);
            Line->setColorIndex(1);
            Line->setLinetype(L"Continuous");

            Group->AddTail(EoDbLine::Create(Line));
        }
	}
	m_PreviousOp = ModeLineHighlightOp(ID_OP5);
	const OdGeVector3d ActiveViewPlaneNormal = GetActiveView()->CameraDirection();

    auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
    Ellipse->setColorIndex(1);
    Ellipse->setLinetype(L"Continuous");

    auto MajorAxis = ComputeArbitraryAxis(ActiveViewPlaneNormal);
    MajorAxis.normalize();
    MajorAxis *= CircleRadius();

    Ellipse->set(CurrentPnt, ActiveViewPlaneNormal, MajorAxis, 1.);
    Group->AddTail(EoDbEllipse::Create(Ellipse));
    
	GetDocument()->AddWorkLayerGroup(Group);
	GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
	EoViAnn_points[0] = CurrentPnt;
}

void AeSysView::OnAnnotateModeUnderline() {
	const OdGePoint3d CurrentPnt = GetCursorPosition();

	if (m_PreviousOp != 0) {
		ModeLineUnhighlightOp(m_PreviousOp);
		GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	}
	EoDbText* pText = SelectTextUsingPoint(CurrentPnt);
	if (pText != 0) {
		OdGePoint3dArray Underline;
		pText->GetBoundingBox(Underline, GapSpaceFactor());

		EoDbGroup* Group = new EoDbGroup;

        OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

        auto Line {EoDbLine::Create(BlockTableRecord)};
        Line->setStartPoint(Underline[0]);
        Line->setEndPoint(Underline[1]);
        
        Line->setLinetype(L"Continuous");

        Group->AddTail(EoDbLine::Create(Line));
        
		GetDocument()->AddWorkLayerGroup(Group);
		GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
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

            OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

			for (int i = 0; i < 4; i++) {
                auto Line {EoDbLine::Create(BlockTableRecord)};
                Line->setStartPoint(JoinedBoxes[i]);
                Line->setEndPoint(JoinedBoxes[(i + 1) % 4]);
                Line->setColorIndex(1);
                Line->setLinetype(L"Continuous");

                Group->AddTail(EoDbLine::Create(Line));
			}
			GetDocument()->AddWorkLayerGroup(Group);
			GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
		}
		ModeLineUnhighlightOp(m_PreviousOp);
	}
}

void AeSysView::OnAnnotateModeCutIn() {
	CDC* DeviceContext = GetDC();

	OdGePoint3d CurrentPnt = GetCursorPosition();

	EoDbGroup* Group = SelectLineBy(CurrentPnt);
	if (Group != 0) {
		EoDbLine* EngagedLine = static_cast<EoDbLine*>(EngagedPrimitive());

		CurrentPnt = DetPt();

		CString CurrentText;

		EoDlgSetText dlg;
		dlg.m_strTitle = L"Set Cut-in Text";
		dlg.m_sText = CurrentText;
		if (dlg.DoModal() == IDOK) {
			CurrentText = dlg.m_sText;
		}
		GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, Group);

		const int PrimitiveState = pstate.Save();

		if (!CurrentText.IsEmpty()) {
			EoGeLineSeg3d LineSeg = EngagedLine->Line();
			double dAng = LineSeg.AngleFromXAxis_xy();
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

			const OdInt16 ColorIndex = pstate.ColorIndex();
			pstate.SetColorIndex(DeviceContext, 2);

			EoDbFontDefinition FontDefinition = pstate.FontDefinition();
			FontDefinition.SetHorizontalAlignment(kAlignCenter);
			FontDefinition.SetVerticalAlignment(kAlignMiddle);

			EoDbCharacterCellDefinition CharacterCellDefinition = pstate.CharacterCellDefinition();
			CharacterCellDefinition.SetRotationAngle(0.);
			pstate.SetCharacterCellDefinition(CharacterCellDefinition);
            OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
            OdDbTextPtr Text = EoDbText::Create(BlockTableRecord, ReferenceSystem.Origin(), (LPCWSTR) CurrentText);
            
            Text->setHeight(ReferenceSystem.YDirection().length());
            Text->setRotation(ReferenceSystem.Rotation());
            Text->setAlignmentPoint(ReferenceSystem.Origin());
            Text->setHorizontalMode(EoDbText::ConvertHorizontalMode(kAlignCenter));
            Text->setVerticalMode(EoDbText::ConvertVerticalMode(kAlignMiddle));

//            OdGePoint3dArray BoundingBox;
//            Text->getBoundingPoints(BoundingBox);

            auto TextPrimitive {EoDbText::Create(Text)};
            Group->AddTail(TextPrimitive);

			pstate.SetColorIndex(DeviceContext, ColorIndex);

			OdGePoint3dArray BoundingBox;
			TextPrimitive->GetBoundingBox(BoundingBox, GapSpaceFactor());

			const double dGap = OdGeVector3d(BoundingBox[1] - BoundingBox[0]).length();

			BoundingBox[0] = ProjectToward(CurrentPnt, EngagedLine->StartPoint(), dGap / 2.);
			BoundingBox[1] = ProjectToward(CurrentPnt, EngagedLine->EndPoint(), dGap / 2.);

			double dRel[2];

			dRel[0] = EngagedLine->ParametricRelationshipOf(BoundingBox[0]);
			dRel[1] = EngagedLine->ParametricRelationshipOf(BoundingBox[1]);

            OdDbLinePtr Line {EngagedLine->EntityObjectId().safeOpenObject(OdDb::kForWrite)};
            if (dRel[0] > DBL_EPSILON && dRel[1] < 1. - DBL_EPSILON) {
                OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
                OdDbLinePtr NewLine {Line->clone()};
                BlockTableRecord->appendOdDbEntity(NewLine);
                Line->setEndPoint(BoundingBox[0]);
                NewLine->setStartPoint(BoundingBox[1]);

                EngagedLine->SetEndPoint(BoundingBox[0]);
				Group->AddTail(EoDbLine::Create(NewLine));
			}
            else if (dRel[0] <= DBL_EPSILON) {
                Line->setStartPoint(BoundingBox[1]);
                EngagedLine->SetStartPoint(BoundingBox[1]);
            }
            else if (dRel[1] >= 1. - DBL_EPSILON) {
                Line->setEndPoint(BoundingBox[0]);
                EngagedLine->SetEndPoint(BoundingBox[0]);
            }
		}
		GetDocument()->UpdateGroupInAllViews(kGroup, Group);
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
        auto Line {EoDbLine::Create0(Database())};
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

void AeSysView::OnAnnotateModeReturn() noexcept {
	// TODO: Add your command handler code here
}

void AeSysView::OnAnnotateModeEscape() {
	GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);

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
double AeSysView::BubbleRadius() const noexcept {
	return m_BubbleRadius;
}
void AeSysView::SetBubbleRadius(double radius) noexcept {
	m_BubbleRadius = radius;
}
double AeSysView::CircleRadius() const noexcept {
	return m_CircleRadius;
}
void AeSysView::SetCircleRadius(double radius) noexcept {
	m_CircleRadius = radius;
}
CString AeSysView::DefaultText() const {
	return m_DefaultText;
}
void AeSysView::SetDefaultText(const CString& text) {
	m_DefaultText = text;
}
double AeSysView::EndItemSize() const noexcept {
	return m_EndItemSize;
}
void AeSysView::SetEndItemSize(double size) noexcept {
	m_EndItemSize = size;
}
int AeSysView::EndItemType() noexcept {
	return m_EndItemType;
}
void AeSysView::SetEndItemType(int type) noexcept {
	m_EndItemType = type;
}
double AeSysView::GapSpaceFactor() const noexcept {
	return m_GapSpaceFactor;
}
void AeSysView::SetGapSpaceFactor(double factor) noexcept {
	m_GapSpaceFactor = factor;
}
int AeSysView::NumberOfSides() const noexcept {
	return m_NumberOfSides;
}
void AeSysView::SetNumberOfSides(int number) noexcept {
	m_NumberOfSides = number;
}

void AeSysView::DoAnnotateModeMouseMove() {
	OdGePoint3d CurrentPnt = GetCursorPosition();
	const int NumberOfPoints = EoViAnn_points.size();
	EoViAnn_points.append(CurrentPnt);

	GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
	m_PreviewGroup.DeletePrimitivesAndRemoveAll();

	switch(m_PreviousOp) {
	case ID_OP2:
	case ID_OP3:
		if (EoViAnn_points[0] != CurrentPnt) {
			if (m_PreviousOp == ID_OP3) {
				GenerateLineEndItem(EndItemType(), EndItemSize(), CurrentPnt, EoViAnn_points[0], &m_PreviewGroup);
			}
			m_PreviewGroup.AddTail(new EoDbLine(EoViAnn_points[0], CurrentPnt));
			GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
		}
		break;

	case ID_OP4:
	case ID_OP5: {
		OdGePoint3d m_PreviousPnt(EoViAnn_points[0]);
		if (CorrectLeaderEndpoints(m_PreviousOp, 0, EoViAnn_points[0], EoViAnn_points[1])) {
			m_PreviewGroup.AddTail(new EoDbLine(EoViAnn_points[0], EoViAnn_points[1]));
			GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
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
			GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
		}
		break;

	}
	EoViAnn_points.setLogicalLength(NumberOfPoints);
}
void AeSysView::GenerateLineEndItem(int type, double size, const OdGePoint3d& startPoint, const OdGePoint3d& endPoint, EoDbGroup* group) {
    const auto PlaneNormal {CameraDirection()};

    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

	OdGePoint3dArray ItemPoints;
	ItemPoints.clear();

    auto Polyline {EoDbPolyline::Create(BlockTableRecord)};

    Polyline->setColorIndex(1);
    Polyline->setLinetype(L"Continuous");

    if (type == 1 || type == 2) { // open arrow or closed arrow
        const auto Angle {.244978663127};
        const auto Size {size / .970142500145};

        auto BasePoint {ProjectToward(endPoint, startPoint, Size)};

        ItemPoints.append(BasePoint.rotateBy(Angle, PlaneNormal, endPoint));
		ItemPoints.append(endPoint);
		ItemPoints.append(BasePoint.rotateBy(- 2. * Angle, PlaneNormal, endPoint));

        if (type == 2) {
            Polyline->setClosed(true);
        }
	}
	else if (type == 3) { // half arrow
        const auto Angle {9.96686524912e-2};
        const auto Size {size / .99503719021};

        auto BasePoint {ProjectToward(endPoint, startPoint, Size)};
		ItemPoints.append(BasePoint.rotateBy(Angle, PlaneNormal, endPoint));
		ItemPoints.append(endPoint);
    }
	else { // hash
        const auto Angle {.785398163397};
        const auto Size {.5 * size / .707106781187};

        auto BasePoint {ProjectToward(endPoint, startPoint, Size)};
		ItemPoints.append(BasePoint.rotateBy(Angle, PlaneNormal, endPoint));
		ItemPoints.append(BasePoint.rotateBy(PI, PlaneNormal, endPoint));
    }
    OdGeMatrix3d WorldToPlaneTransform;
    WorldToPlaneTransform.setToWorldToPlane(OdGePlane(OdGePoint3d::kOrigin, PlaneNormal));

    for (size_t VertexIndex = 0; VertexIndex < ItemPoints.size(); VertexIndex++) {
        auto Vertex = ItemPoints[VertexIndex];
        Vertex.transformBy(WorldToPlaneTransform);
        Polyline->addVertexAt(VertexIndex, Vertex.convert2d());
    }
    Polyline->setNormal(PlaneNormal);
    Polyline->setElevation(ComputeElevation(endPoint, PlaneNormal));
    group->AddTail(EoDbPolyline::Create(Polyline));
}
