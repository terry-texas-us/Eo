#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoGePolyline.h"
#include "EoDlgAnnotateOptions.h"
#include "EoDlgSetText.h"
#include "EoDbPolyline.h"
OdGePoint3dArray EoViAnn_points;

void AeSysView::OnAnnotateModeOptions() {
	EoDlgAnnotateOptions Dialog(this);
	if (Dialog.DoModal() == IDOK) {
	}
}

void AeSysView::OnAnnotateModeLine() {
	auto CurrentPnt {GetCursorPosition()};
	if (m_PreviousOp == 0) {
		EoViAnn_points.clear();
		EoViAnn_points.append(CurrentPnt);
	} else {
		if (CorrectLeaderEndpoints(m_PreviousOp, ID_OP2, EoViAnn_points[0], CurrentPnt)) {
			auto Group {new EoDbGroup};
			GetDocument()->AddWorkLayerGroup(Group);
			if (m_PreviousOp == ID_OP3) {
				GenerateLineEndItem(EndItemType(), EndItemSize(), CurrentPnt, EoViAnn_points[0], Group);
			}
			const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
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
	auto CurrentPnt {GetCursorPosition()};
	if (m_PreviousOp == 0) {
		EoViAnn_points.clear();
		EoViAnn_points.append(CurrentPnt);
	} else {
		if (CorrectLeaderEndpoints(m_PreviousOp, ID_OP3, EoViAnn_points[0], CurrentPnt)) {
			auto Group {new EoDbGroup};
			if (m_PreviousOp == ID_OP3) {
				GenerateLineEndItem(EndItemType(), EndItemSize(), CurrentPnt, EoViAnn_points[0], Group);
			}
			const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
			auto Line {EoDbLine::Create(BlockTableRecord)};
			Line->setStartPoint(EoViAnn_points[0]);
			Line->setEndPoint(CurrentPnt);
			Line->setColorIndex(1);
			Line->setLinetype(L"Continuous");
			Group->AddTail(EoDbLine::Create(Line));
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
	auto CurrentPnt {GetCursorPosition()};
	OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
	EoDlgSetText dlg;
	dlg.m_strTitle = L"Set Bubble Text";
	dlg.m_sText = CurrentText;
	if (dlg.DoModal() == IDOK) {
		CurrentText = dlg.m_sText;
	}
	auto Group {new EoDbGroup};
	GetDocument()->AddWorkLayerGroup(Group);
	if (m_PreviousOp == 0) { // No operation pending
		EoViAnn_points.clear();
		EoViAnn_points.append(CurrentPnt);
	} else {
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
		auto pt {CurrentPnt};
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
	const auto ActiveViewPlaneNormal {GetActiveView()->CameraDirection()};
	auto MajorAxis = ComputeArbitraryAxis(ActiveViewPlaneNormal);
	MajorAxis.normalize();
	if (!CurrentText.IsEmpty()) {
		auto MinorAxis {MajorAxis};
		MinorAxis.rotateBy(OdaPI2, ActiveViewPlaneNormal);
		EoGeReferenceSystem ReferenceSystem(CurrentPnt, MajorAxis * .06, MinorAxis * .1);
		auto Text {EoDbText::Create(BlockTableRecord, ReferenceSystem.Origin(), static_cast<const wchar_t*>(CurrentText))};
		Text->setNormal(ActiveViewPlaneNormal);
		Text->setRotation(ReferenceSystem.Rotation());
		Text->setHeight(ReferenceSystem.YDirection().length());
		Text->setAlignmentPoint(ReferenceSystem.Origin());
		Text->setColorIndex(2);
		Text->setHorizontalMode(EoDbText::ConvertHorizontalMode(EoDb::kAlignCenter));
		Text->setVerticalMode(EoDbText::ConvertVerticalMode(EoDb::kAlignMiddle));
		Group->AddTail(EoDbText::Create(Text));
	}
	if (NumberOfSides() == 0) {
		auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
		Ellipse->setColorIndex(1);
		Ellipse->setLinetype(L"Continuous");
		Ellipse->set(CurrentPnt, ActiveViewPlaneNormal, MajorAxis * BubbleRadius(), 1.0);
		Group->AddTail(EoDbEllipse::Create(Ellipse));
	} else {
		auto Polyline {EoDbPolyline::Create(BlockTableRecord)};
		Polyline->setColorIndex(1);
		Polyline->setLinetype(L"Continuous");
		Polyline->setClosed(true);
		OdGePoint3dArray Points;
		polyline::GeneratePointsForNPoly(CurrentPnt, ActiveViewPlaneNormal, BubbleRadius(), static_cast<unsigned>(NumberOfSides()), Points);
		OdGeMatrix3d WorldToPlaneTransform;
		WorldToPlaneTransform.setToWorldToPlane(OdGePlane(OdGePoint3d::kOrigin, ActiveViewPlaneNormal));
		for (unsigned VertexIndex = 0; VertexIndex < Points.size(); VertexIndex++) {
			auto Vertex = Points[VertexIndex];
			Vertex.transformBy(WorldToPlaneTransform);
			Polyline->addVertexAt(VertexIndex, Vertex.convert2d());
		}
		Polyline->setNormal(ActiveViewPlaneNormal);
		Polyline->setElevation(ComputeElevation(CurrentPnt, ActiveViewPlaneNormal));
		Group->AddTail(EoDbPolyline::Create(Polyline));
	}
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
	EoViAnn_points[0] = CurrentPnt;
}

void AeSysView::OnAnnotateModeHook() {
	const auto CurrentPnt {GetCursorPosition()};
	auto Group {new EoDbGroup};
	OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
	if (m_PreviousOp == 0) {
		EoViAnn_points.clear();
		EoViAnn_points.append(CurrentPnt);
	} else {
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
		auto pt {CurrentPnt};
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
	const auto ActiveViewPlaneNormal {GetActiveView()->CameraDirection()};
	auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
	Ellipse->setColorIndex(1);
	Ellipse->setLinetype(L"Continuous");
	auto MajorAxis = ComputeArbitraryAxis(ActiveViewPlaneNormal);
	MajorAxis.normalize();
	MajorAxis *= CircleRadius();
	Ellipse->set(CurrentPnt, ActiveViewPlaneNormal, MajorAxis, 1.0);
	Group->AddTail(EoDbEllipse::Create(Ellipse));
	GetDocument()->AddWorkLayerGroup(Group);
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
	EoViAnn_points[0] = CurrentPnt;
}

void AeSysView::OnAnnotateModeUnderline() {
	const auto CurrentPnt {GetCursorPosition()};
	if (m_PreviousOp != 0) {
		ModeLineUnhighlightOp(m_PreviousOp);
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	}
	const auto pText {SelectTextUsingPoint(CurrentPnt)};
	if (pText != nullptr) {
		OdGePoint3dArray Underline;
		pText->GetBoundingBox(Underline, GapSpaceFactor());
		auto Group {new EoDbGroup};
		const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
		auto Line {EoDbLine::Create(BlockTableRecord)};
		Line->setStartPoint(Underline[0]);
		Line->setEndPoint(Underline[1]);
		Line->setLinetype(L"Continuous");
		Group->AddTail(EoDbLine::Create(Line));
		GetDocument()->AddWorkLayerGroup(Group);
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
	}
}

void AeSysView::OnAnnotateModeBox() {
	const auto CurrentPnt {GetCursorPosition()};
	if (m_PreviousOp != ID_OP7) {
		if (m_PreviousOp != 0) {
			RubberBandingDisable();
			ModeLineUnhighlightOp(m_PreviousOp);
		}
		m_PreviousOp = ModeLineHighlightOp(ID_OP7);
		EoViAnn_points.clear();
		EoViAnn_points.append(CurrentPnt);
	} else {
		OdGePoint3dArray ptsBox1;
		OdGePoint3dArray ptsBox2;
		auto bG1Flg {false};
		auto bG2Flg {false};
		auto pText {SelectTextUsingPoint(EoViAnn_points[0])};
		if (pText != nullptr) {
			pText->GetBoundingBox(ptsBox1, GapSpaceFactor());
			bG1Flg = true;
		}
		pText = SelectTextUsingPoint(CurrentPnt);
		if (pText != nullptr) {
			pText->GetBoundingBox(ptsBox2, GapSpaceFactor());
			bG2Flg = true;
		}
		if (bG1Flg && bG2Flg) {
			OdGePoint3dArray JoinedBoxes;
			JoinedBoxes.setLogicalLength(4);
			JoinedBoxes.setAll(ptsBox1[0]);
			for (unsigned i = 1; i < 4; i++) {
				JoinedBoxes[0].x = EoMin(JoinedBoxes[0].x, ptsBox1[i].x);
				JoinedBoxes[2].x = EoMax(JoinedBoxes[2].x, ptsBox1[i].x);
				JoinedBoxes[0].y = EoMin(JoinedBoxes[0].y, ptsBox1[i].y);
				JoinedBoxes[2].y = EoMax(JoinedBoxes[2].y, ptsBox1[i].y);
			}
			for (unsigned i = 0; i < 4; i++) {
				JoinedBoxes[0].x = EoMin(JoinedBoxes[0].x, ptsBox2[i].x);
				JoinedBoxes[2].x = EoMax(JoinedBoxes[2].x, ptsBox2[i].x);
				JoinedBoxes[0].y = EoMin(JoinedBoxes[0].y, ptsBox2[i].y);
				JoinedBoxes[2].y = EoMax(JoinedBoxes[2].y, ptsBox2[i].y);
			}
			JoinedBoxes[1].x = JoinedBoxes[2].x;
			JoinedBoxes[1].y = JoinedBoxes[0].y;
			JoinedBoxes[3].x = JoinedBoxes[0].x;
			JoinedBoxes[3].y = JoinedBoxes[2].y;
			auto Group {new EoDbGroup};
			const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
			for (unsigned i = 0; i < 4; i++) {
				auto Line {EoDbLine::Create(BlockTableRecord)};
				Line->setStartPoint(JoinedBoxes[i]);
				Line->setEndPoint(JoinedBoxes[(i + 1) % 4]);
				Line->setColorIndex(1);
				Line->setLinetype(L"Continuous");
				Group->AddTail(EoDbLine::Create(Line));
			}
			GetDocument()->AddWorkLayerGroup(Group);
			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
		}
		ModeLineUnhighlightOp(m_PreviousOp);
	}
}

void AeSysView::OnAnnotateModeCutIn() {
	auto DeviceContext {GetDC()};
	if (DeviceContext == nullptr) { return; }
	auto CurrentPnt {GetCursorPosition()};
	auto Selection {SelectLineUsingPoint(CurrentPnt)};
	auto Group {std::get<tGroup>(Selection)};
	if (Group != nullptr) {
		auto EngagedLine {std::get<1>(Selection)};
		CurrentPnt = EngagedLine->ProjPt_(CurrentPnt);
		CString CurrentText;
		EoDlgSetText dlg;
		dlg.m_strTitle = L"Set Cut-in Text";
		dlg.m_sText = CurrentText;
		if (dlg.DoModal() == IDOK) {
			CurrentText = dlg.m_sText;
		}
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, Group);
		const auto PrimitiveState {g_PrimitiveState.Save()};
		if (!CurrentText.IsEmpty()) {
			auto LineSeg {EngagedLine->LineSeg()};
			auto dAng {LineSeg.AngleFromXAxis_xy()};
			if (dAng > .25 * Oda2PI && dAng < .75 * Oda2PI) dAng += OdaPI;
			const auto PlaneNormal {CameraDirection()};
			auto MinorAxis {ViewUp()};
			MinorAxis.rotateBy(dAng, PlaneNormal);
			auto MajorAxis {MinorAxis};
			MajorAxis.rotateBy(-OdaPI2, PlaneNormal);
			MajorAxis *= .06;
			MinorAxis *= .1;
			EoGeReferenceSystem ReferenceSystem(CurrentPnt, MajorAxis, MinorAxis);
			const auto ColorIndex {g_PrimitiveState.ColorIndex()};
			g_PrimitiveState.SetColorIndex(DeviceContext, 2);
			auto FontDefinition {g_PrimitiveState.FontDefinition()};
			FontDefinition.SetHorizontalAlignment(EoDb::kAlignCenter);
			FontDefinition.SetVerticalAlignment(EoDb::kAlignMiddle);
			auto CharacterCellDefinition {g_PrimitiveState.CharacterCellDefinition()};
			CharacterCellDefinition.SetRotationAngle(0.0);
			g_PrimitiveState.SetCharacterCellDefinition(CharacterCellDefinition);
			OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
			auto Text {EoDbText::Create(BlockTableRecord, ReferenceSystem.Origin(), static_cast<const wchar_t*>(CurrentText))};
			Text->setHeight(ReferenceSystem.YDirection().length());
			Text->setRotation(ReferenceSystem.Rotation());
			Text->setAlignmentPoint(ReferenceSystem.Origin());
			Text->setHorizontalMode(EoDbText::ConvertHorizontalMode(EoDb::kAlignCenter));
			Text->setVerticalMode(EoDbText::ConvertVerticalMode(EoDb::kAlignMiddle));

			//            OdGePoint3dArray BoundingBox;
			//            Text->getBoundingPoints(BoundingBox);
			auto TextPrimitive {EoDbText::Create(Text)};
			Group->AddTail(TextPrimitive);
			g_PrimitiveState.SetColorIndex(DeviceContext, ColorIndex);
			OdGePoint3dArray BoundingBox;
			TextPrimitive->GetBoundingBox(BoundingBox, GapSpaceFactor());
			const auto dGap {OdGeVector3d(BoundingBox[1] - BoundingBox[0]).length()};
			BoundingBox[0] = ProjectToward(CurrentPnt, EngagedLine->StartPoint(), dGap / 2.);
			BoundingBox[1] = ProjectToward(CurrentPnt, EngagedLine->EndPoint(), dGap / 2.);
			double dRel[2];
			dRel[0] = EngagedLine->ParametricRelationshipOf(BoundingBox[0]);
			dRel[1] = EngagedLine->ParametricRelationshipOf(BoundingBox[1]);
			OdDbLinePtr Line {EngagedLine->EntityObjectId().safeOpenObject(OdDb::kForWrite)};
			if (dRel[0] > DBL_EPSILON && dRel[1] < 1. - DBL_EPSILON) {
				OdDbLinePtr NewLine {Line->clone()};
				BlockTableRecord->appendOdDbEntity(NewLine);
				Line->setEndPoint(BoundingBox[0]);
				NewLine->setStartPoint(BoundingBox[1]);
				EngagedLine->SetEndPoint(BoundingBox[0]);
				Group->AddTail(EoDbLine::Create(NewLine));
			} else if (dRel[0] <= DBL_EPSILON) {
				Line->setStartPoint(BoundingBox[1]);
				EngagedLine->SetStartPoint(BoundingBox[1]);
			} else if (dRel[1] >= 1. - DBL_EPSILON) {
				Line->setEndPoint(BoundingBox[0]);
				EngagedLine->SetEndPoint(BoundingBox[0]);
			}
		}
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroup, Group);
		g_PrimitiveState.Restore(*DeviceContext, PrimitiveState);
	}
	ReleaseDC(DeviceContext);
}

void AeSysView::OnAnnotateModeConstructionLine() {
	auto CurrentPnt {GetCursorPosition()};
	const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	if (m_PreviousOp != ID_OP9) {
		m_PreviousOp = ModeLineHighlightOp(ID_OP9);
		EoViAnn_points.clear();
		EoViAnn_points.append(CurrentPnt);
	} else {
		CurrentPnt = SnapPointToAxis(EoViAnn_points[0], CurrentPnt);
		EoViAnn_points.append(ProjectToward(EoViAnn_points[0], CurrentPnt, 48.));
		EoViAnn_points.append(ProjectToward(EoViAnn_points[1], EoViAnn_points[0], 96.));
		auto Group {new EoDbGroup};
		auto Line {EoDbLine::Create(BlockTableRecord, EoViAnn_points[1], EoViAnn_points[2])};
		Line->setColorIndex(15);
		Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(2));
		Group->AddTail(EoDbLine::Create(Line));
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
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
	m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	EoViAnn_points.clear();
	ModeLineUnhighlightOp(m_PreviousOp);
}

bool AeSysView::CorrectLeaderEndpoints(int beginType, int endType, OdGePoint3d& startPoint, OdGePoint3d& endPoint) const {
	const auto LineSegmentLength {OdGeVector3d(endPoint - startPoint).length()};
	auto BeginDistance {0.0};
	if (beginType == ID_OP4) {
		BeginDistance = BubbleRadius();
	} else if (beginType == ID_OP5) {
		BeginDistance = CircleRadius();
	}
	auto EndDistance {0.0};
	if (endType == ID_OP4) {
		EndDistance = BubbleRadius();
	} else if (endType == ID_OP5) EndDistance = CircleRadius();
	if (LineSegmentLength > BeginDistance + EndDistance + DBL_EPSILON) {
		if (BeginDistance != 0.0) startPoint = ProjectToward(startPoint, endPoint, BeginDistance);
		if (EndDistance != 0.0) endPoint = ProjectToward(endPoint, startPoint, EndDistance);
		return true;
	}
	theApp.AddModeInformationToMessageList();
	return false;
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
	auto CurrentPnt {GetCursorPosition()};
	const auto NumberOfPoints {EoViAnn_points.size()};
	EoViAnn_points.append(CurrentPnt);
	const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
	m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	switch (m_PreviousOp) {
		case ID_OP2: case ID_OP3:
			if (EoViAnn_points[0] != CurrentPnt) {
				if (m_PreviousOp == ID_OP3) {
					GenerateLineEndItem(EndItemType(), EndItemSize(), CurrentPnt, EoViAnn_points[0], &m_PreviewGroup);
				}
				auto Line {EoDbLine::Create(BlockTableRecord, EoViAnn_points[0], CurrentPnt)};
				Line->setColorIndex(1);
				Line->setLinetype(L"Continuous");
				m_PreviewGroup.AddTail(EoDbLine::Create(Line));
				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
			}
			break;
		case ID_OP4: case ID_OP5: {
			m_PreviousPnt = EoViAnn_points[0];
			if (CorrectLeaderEndpoints(m_PreviousOp, 0, EoViAnn_points[0], EoViAnn_points[1])) {
				auto Line {EoDbLine::Create(BlockTableRecord, EoViAnn_points[0], EoViAnn_points[1])};
				Line->setColorIndex(1);
				Line->setLinetype(L"Continuous");
				m_PreviewGroup.AddTail(EoDbLine::Create(Line));
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
				auto Line {EoDbLine::Create(BlockTableRecord, EoViAnn_points[2], EoViAnn_points[3])};
				Line->setColorIndex(15);
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(2));
				m_PreviewGroup.AddTail(EoDbLine::Create(Line));
				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
			}
			break;
	}
	EoViAnn_points.setLogicalLength(NumberOfPoints);
}

void AeSysView::GenerateLineEndItem(int type, double size, const OdGePoint3d& startPoint, const OdGePoint3d& endPoint, EoDbGroup* group) {
	const auto PlaneNormal {CameraDirection()};
	const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
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
		ItemPoints.append(BasePoint.rotateBy(-2. * Angle, PlaneNormal, endPoint));
		if (type == 2) {
			Polyline->setClosed(true);
		}
	} else if (type == 3) { // half arrow
		const auto Angle {9.96686524912e-2};
		const auto Size {size / .99503719021};
		auto BasePoint {ProjectToward(endPoint, startPoint, Size)};
		ItemPoints.append(BasePoint.rotateBy(Angle, PlaneNormal, endPoint));
		ItemPoints.append(endPoint);
	} else { // hash
		const auto Angle {.785398163397};
		const auto Size {.5 * size / .707106781187};
		auto BasePoint {ProjectToward(endPoint, startPoint, Size)};
		ItemPoints.append(BasePoint.rotateBy(Angle, PlaneNormal, endPoint));
		ItemPoints.append(BasePoint.rotateBy(OdaPI, PlaneNormal, endPoint));
	}
	OdGeMatrix3d WorldToPlaneTransform;
	WorldToPlaneTransform.setToWorldToPlane(OdGePlane(OdGePoint3d::kOrigin, PlaneNormal));
	for (unsigned VertexIndex = 0; VertexIndex < ItemPoints.size(); VertexIndex++) {
		auto Vertex = ItemPoints[VertexIndex];
		Vertex.transformBy(WorldToPlaneTransform);
		Polyline->addVertexAt(VertexIndex, Vertex.convert2d());
	}
	Polyline->setNormal(PlaneNormal);
	Polyline->setElevation(ComputeElevation(endPoint, PlaneNormal));
	group->AddTail(EoDbPolyline::Create(Polyline));
}
