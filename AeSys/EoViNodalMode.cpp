#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "PrimState.h"
#include "EoDbHatch.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
double NodalModePickTolerance = .05;
unsigned short PreviousNodalCommand = 0;
OdGePoint3d PreviousNodalCursorPosition;

void AeSysView::OnNodalModeAddRemove() {
	theApp.nodalModeAddGroups = !theApp.nodalModeAddGroups;
	if (theApp.nodalModeAddGroups) {
		SetModeCursor(ID_MODE_NODAL);
	} else {
		SetModeCursor(ID_MODE_NODALR);
	}
}

void AeSysView::OnNodalModePoint() {
	const auto CurrentPnt {GetCursorPosition()};
	OdGePoint3dArray Points;
	auto GroupPosition {GetFirstVisibleGroupPosition()};
	while (GroupPosition != nullptr) {
		const auto Group {GetNextVisibleGroup(GroupPosition)};
		auto PrimitivePosition {Group->GetHeadPosition()};
		while (PrimitivePosition != nullptr) {
			const auto Primitive {Group->GetNext(PrimitivePosition)};
			const auto Mask {GetDocument()->GetPrimitiveMask(Primitive)};
			Primitive->GetAllPoints(Points);
			for (unsigned i = 0; i < Points.size(); i++) {
				if (OdGeVector3d(CurrentPnt - Points[i]).length() <= NodalModePickTolerance) {
					GetDocument()->UpdateNodalList(Group, Primitive, Mask, static_cast<int>(i), Points[i]);
				}
			}
		}
	}
}

void AeSysView::OnNodalModeLine() {
	const auto CurrentPnt {GetCursorPosition()};
	OdGePoint3dArray Points;
	const auto Group {SelectGroupAndPrimitive(CurrentPnt)};
	if (Group != nullptr) {
		const auto Primitive {EngagedPrimitive()};
		const auto Mask {GetDocument()->GetPrimitiveMask(Primitive)};
		Primitive->GetAllPoints(Points);
		for (unsigned i = 0; i < Points.size(); i++) {
			GetDocument()->UpdateNodalList(Group, Primitive, Mask, static_cast<int>(i), Points[i]);
		}
	}
}

void AeSysView::OnNodalModeArea() {
	const auto CurrentPnt {GetCursorPosition()};
	if (PreviousNodalCommand != ID_OP3) {
		PreviousNodalCursorPosition = CurrentPnt;
		RubberBandingStartAtEnable(CurrentPnt, kRectangles);
		PreviousNodalCommand = ModeLineHighlightOp(ID_OP3);
	} else {
		if (PreviousNodalCursorPosition != CurrentPnt) {
			OdGePoint3dArray Points;
			OdGePoint3d MinExtent;
			OdGePoint3d MaxExtent;
			EoGeLineSeg3d(PreviousNodalCursorPosition, CurrentPnt).Extents(MinExtent, MaxExtent);
			auto GroupPosition = GetFirstVisibleGroupPosition();
			while (GroupPosition != nullptr) {
				const auto Group {GetNextVisibleGroup(GroupPosition)};
				auto PrimitivePosition {Group->GetHeadPosition()};
				while (PrimitivePosition != nullptr) {
					const auto Primitive = Group->GetNext(PrimitivePosition);
					const auto Mask {GetDocument()->GetPrimitiveMask(Primitive)};
					Primitive->GetAllPoints(Points);
					for (unsigned i = 0; i < Points.size(); i++) {
						if (ContainmentOf(Points[i], MinExtent, MaxExtent)) {
							GetDocument()->UpdateNodalList(Group, Primitive, Mask, static_cast<int>(i), Points[i]);
						}
					}
				}
			}
		}
		RubberBandingDisable();
		ModeLineUnhighlightOp(PreviousNodalCommand);
	}
}

void AeSysView::OnNodalModeMove() {
	const auto CurrentPnt {GetCursorPosition()};
	if (PreviousNodalCommand != ID_OP4) {
		PreviousNodalCommand = ModeLineHighlightOp(ID_OP4);
		m_NodalModePoints.clear();
		m_NodalModePoints.append(CurrentPnt);
		RubberBandingStartAtEnable(CurrentPnt, kLines);
		ConstructPreviewGroup();
	} else {
		OnNodalModeReturn();
	}
}

void AeSysView::OnNodalModeCopy() {
	const auto CurrentPnt {GetCursorPosition()};
	if (PreviousNodalCommand != ID_OP5) {
		PreviousNodalCommand = ModeLineHighlightOp(ID_OP5);
		m_NodalModePoints.clear();
		m_NodalModePoints.append(CurrentPnt);
		RubberBandingStartAtEnable(CurrentPnt, kLines);
		ConstructPreviewGroupForNodalGroups();
	} else {
		OnNodalModeReturn();
	}
}

void AeSysView::OnNodalModeToLine() {
	auto CurrentPnt {GetCursorPosition()};
	if (PreviousNodalCommand != ID_OP6) {
		PreviousNodalCursorPosition = CurrentPnt;
		RubberBandingStartAtEnable(CurrentPnt, kLines);
		PreviousNodalCommand = ModeLineHighlightOp(ID_OP6);
	} else {
		if (PreviousNodalCursorPosition != CurrentPnt) {
			const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
			CurrentPnt = SnapPointToAxis(PreviousNodalCursorPosition, CurrentPnt);
			const auto Translate {CurrentPnt - PreviousNodalCursorPosition};
			auto Group {new EoDbGroup};
			auto PointPosition {GetDocument()->GetFirstUniquePointPosition()};
			while (PointPosition != nullptr) {
				const auto UniquePoint {GetDocument()->GetNextUniquePoint(PointPosition)};
				auto Line {EoDbLine::Create(BlockTableRecord, UniquePoint->m_Point, UniquePoint->m_Point + Translate)};
				Line->setColorIndex(static_cast<unsigned short>(g_PrimitiveState.ColorIndex()));
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(g_PrimitiveState.LinetypeIndex()));
				Group->AddTail(EoDbLine::Create(Line));
			}
			GetDocument()->AddWorkLayerGroup(Group);
			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
			SetCursorPosition(CurrentPnt);
		}
		RubberBandingDisable();
		ModeLineUnhighlightOp(PreviousNodalCommand);
	}
}

/// <remarks>
/// The pen color used for any polygons added to drawing is the current pen color and not the pen color of the reference primitives.
/// </remarks>
void AeSysView::OnNodalModeToPolygon() {
	auto CurrentPnt {GetCursorPosition()};
	if (PreviousNodalCommand != ID_OP7) {
		PreviousNodalCursorPosition = CurrentPnt;
		RubberBandingStartAtEnable(CurrentPnt, kLines);
		PreviousNodalCommand = ModeLineHighlightOp(ID_OP7);
	} else {
		const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
		if (PreviousNodalCursorPosition != CurrentPnt) {
			CurrentPnt = SnapPointToAxis(PreviousNodalCursorPosition, CurrentPnt);
			const auto Translate {CurrentPnt - PreviousNodalCursorPosition};
			OdGePoint3dArray Points;
			Points.setLogicalLength(4);
			const auto DeviceContext {GetDC()};
			const auto PrimitiveState {g_PrimitiveState.Save()};
			auto GroupPosition {GetDocument()->GetFirstNodalGroupPosition()};
			while (GroupPosition != nullptr) {
				const auto Group {GetDocument()->GetNextNodalGroup(GroupPosition)};
				auto PrimitivePosition {Group->GetHeadPosition()};
				while (PrimitivePosition != nullptr) {
					const auto Primitive {Group->GetNext(PrimitivePosition)};
					const auto Mask {GetDocument()->GetPrimitiveMask(Primitive)};
					if (Mask != 0) {
						if (Primitive->IsKindOf(RUNTIME_CLASS(EoDbLine))) {
							if ((Mask & 3) == 3) {
								const auto Line {dynamic_cast<EoDbLine*>(Primitive)};
								Points[0] = Line->StartPoint();
								Points[1] = Line->EndPoint();
								Points[2] = Points[1] + Translate;
								Points[3] = Points[0] + Translate;

								// <tas="Behavior changed. Line extruded into solid hatch primitive"/>
								auto NewHatch {EoDbHatch::Create(BlockTableRecord)};
								NewHatch->setPattern(OdDbHatch::kPreDefined, L"SOLID");
								const auto PlaneNormal {ComputeNormal(Points[1], Points[0], Points[2])};
								NewHatch->setNormal(PlaneNormal);
								NewHatch->setElevation(ComputeElevation(Points[0], PlaneNormal));
								EoDbHatch::AppendLoop(Points, NewHatch);
								auto NewGroup {new EoDbGroup};
								NewGroup->AddTail(EoDbHatch::Create(NewHatch));
								GetDocument()->AddWorkLayerGroup(NewGroup);
								GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, NewGroup);
							}
						} else if (Primitive->IsKindOf(RUNTIME_CLASS(EoDbHatch))) {
							auto Hatch {dynamic_cast<EoDbHatch*>(Primitive)};
							const auto iPts {static_cast<unsigned>(Hatch->NumberOfVertices())};
							for (unsigned i = 0; i < iPts; i++) {
								if (btest(Mask, i) && btest(Mask, ((i + 1) % iPts))) {
									Points[0] = Hatch->GetPointAt(i);
									Points[1] = Hatch->GetPointAt((i + 1) % iPts);
									Points[2] = Points[1] + Translate;
									Points[3] = Points[0] + Translate;

									// <tas="Behavior changed. Edges of hatch extruded into solid hatch primitives"/>
									auto NewHatch {EoDbHatch::Create(BlockTableRecord)};
									NewHatch->setPattern(OdDbHatch::kPreDefined, L"SOLID");
									const auto PlaneNormal {ComputeNormal(Points[1], Points[0], Points[2])};
									NewHatch->setNormal(PlaneNormal);
									NewHatch->setElevation(ComputeElevation(Points[0], PlaneNormal));
									EoDbHatch::AppendLoop(Points, NewHatch);
									auto NewGroup {new EoDbGroup};
									NewGroup->AddTail(EoDbHatch::Create(NewHatch));
									GetDocument()->AddWorkLayerGroup(NewGroup);
									GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, NewGroup);
								}
							}
						}
					}
				}
			}
			g_PrimitiveState.Restore(*DeviceContext, PrimitiveState);
			SetCursorPosition(CurrentPnt);
			RubberBandingDisable();
			ModeLineUnhighlightOp(PreviousNodalCommand);
		}
	}
}

void AeSysView::OnNodalModeEmpty() {
	OnNodalModeEscape();
}

void AeSysView::OnNodalModeEngage() {
	if (GroupIsEngaged()) {
		const auto Mask {GetDocument()->GetPrimitiveMask(EngagedPrimitive())};
		OdGePoint3dArray Points;
		EngagedPrimitive()->GetAllPoints(Points);
		for (unsigned i = 0; i < Points.size(); i++) {
			GetDocument()->UpdateNodalList(EngagedGroup(), EngagedPrimitive(), Mask, static_cast<int>(i), Points[i]);
		}
	}
}

void AeSysView::OnNodalModeReturn() {
	auto CurrentPnt {GetCursorPosition()};
	switch (PreviousNodalCommand) {
		case ID_OP4:
			if (m_NodalModePoints[0] != CurrentPnt) {
				CurrentPnt = SnapPointToAxis(m_NodalModePoints[0], CurrentPnt);
				const auto Translate {CurrentPnt - m_NodalModePoints[0]};
				auto MaskedPrimitivePosition {GetDocument()->GetFirstMaskedPrimitivePosition()};
				while (MaskedPrimitivePosition != nullptr) {
					auto MaskedPrimitive = GetDocument()->GetNextMaskedPrimitive(MaskedPrimitivePosition);
					auto Primitive = MaskedPrimitive->GetPrimitive();
					const auto Mask {MaskedPrimitive->GetMask()};
					Primitive->TranslateUsingMask(Translate, Mask);
				}
				auto UniquePointPosition {GetDocument()->GetFirstUniquePointPosition()};
				while (UniquePointPosition != nullptr) {
					auto Point {GetDocument()->GetNextUniquePoint(UniquePointPosition)};
					Point->m_Point += Translate;
				}
				SetCursorPosition(CurrentPnt);
			}
			break;
		case ID_OP5:
			if (m_NodalModePoints[0] != CurrentPnt) {
				CurrentPnt = SnapPointToAxis(m_NodalModePoints[0], CurrentPnt);
				EoGeMatrix3d TranslationMatrix;
				TranslationMatrix.setToTranslation(CurrentPnt - m_NodalModePoints[0]);
				auto GroupPosition {GetDocument()->GetFirstNodalGroupPosition()};
				while (GroupPosition != nullptr) {
					const auto Group {GetDocument()->GetNextNodalGroup(GroupPosition)};
					GetDocument()->AddWorkLayerGroup(new EoDbGroup(*Group));
					GetDocument()->GetLastWorkLayerGroup()->TransformBy(TranslationMatrix);
				}
				SetCursorPosition(CurrentPnt);
			}
			break;
		default:
			return;
	}
	m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	m_NodalModePoints.clear();
	RubberBandingDisable();
	ModeLineUnhighlightOp(PreviousNodalCommand);
}

void AeSysView::OnNodalModeEscape() {
	if (PreviousNodalCommand == 0) {
		GetDocument()->DisplayUniquePoints();
		GetDocument()->DeleteNodalResources();
	} else {
		RubberBandingDisable();
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
		ConstructPreviewGroup();
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
		m_NodalModePoints.clear();
		ModeLineUnhighlightOp(PreviousNodalCommand);
	}
}

void AeSysView::DoNodalModeMouseMove() {
	auto CurrentPnt {GetCursorPosition()};
	const auto NumberOfPoints {m_NodalModePoints.size()};
	switch (PreviousNodalCommand) {
		case ID_OP4: VERIFY(m_NodalModePoints.size() > 0);
			if (m_NodalModePoints[0] != CurrentPnt) {
				CurrentPnt = SnapPointToAxis(m_NodalModePoints[0], CurrentPnt);
				m_NodalModePoints.append(CurrentPnt);
				const auto Translate {CurrentPnt - m_NodalModePoints[0]};
				const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
				m_PreviewGroup.DeletePrimitivesAndRemoveAll();
				auto MaskedPrimitivePosition {GetDocument()->GetFirstMaskedPrimitivePosition()};
				while (MaskedPrimitivePosition != nullptr) {
					auto MaskedPrimitive {GetDocument()->GetNextMaskedPrimitive(MaskedPrimitivePosition)};
					const auto Primitive {MaskedPrimitive->GetPrimitive()};
					const auto Mask {MaskedPrimitive->GetMask()};
					m_PreviewGroup.AddTail(Primitive->Clone(BlockTableRecord));
					static_cast<EoDbPrimitive*>(m_PreviewGroup.GetTail())->TranslateUsingMask(Translate, Mask);
				}
				auto UniquePointPosition {GetDocument()->GetFirstUniquePointPosition()};
				while (UniquePointPosition != nullptr) {
					const auto UniquePoint {GetDocument()->GetNextUniquePoint(UniquePointPosition)};
					const auto Point {UniquePoint->m_Point + Translate};
					auto PointPrimitive {new EoDbPoint(Point)};
					PointPrimitive->SetColorIndex2(252);
					PointPrimitive->SetPointDisplayMode(8);
					m_PreviewGroup.AddTail(PointPrimitive);
				}
				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
			}
			break;
		case ID_OP5:
			if (m_NodalModePoints[0] != CurrentPnt) {
				CurrentPnt = SnapPointToAxis(m_NodalModePoints[0], CurrentPnt);
				m_NodalModePoints.append(CurrentPnt);
				EoGeMatrix3d TranslationMatrix;
				TranslationMatrix.setToTranslation(CurrentPnt - m_NodalModePoints[0]);
				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
				m_PreviewGroup.DeletePrimitivesAndRemoveAll();
				ConstructPreviewGroupForNodalGroups();
				m_PreviewGroup.TransformBy(TranslationMatrix);
				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
			}
			break;
	}
	m_NodalModePoints.setLogicalLength(static_cast<unsigned>(NumberOfPoints));
}

void AeSysView::ConstructPreviewGroup() {
	m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	auto MaskedPrimitivePosition {GetDocument()->GetFirstMaskedPrimitivePosition()};
	while (MaskedPrimitivePosition != nullptr) {
		auto MaskedPrimitive {GetDocument()->GetNextMaskedPrimitive(MaskedPrimitivePosition)};
		const auto Primitive {MaskedPrimitive->GetPrimitive()};
		m_PreviewGroup.AddTail(Primitive->Clone(BlockTableRecord));
	}
	auto UniquePointPosition {GetDocument()->GetFirstUniquePointPosition()};
	while (UniquePointPosition != nullptr) {
		const auto UniquePoint {GetDocument()->GetNextUniquePoint(UniquePointPosition)};
		auto PointPrimitive {new EoDbPoint(UniquePoint->m_Point)};
		PointPrimitive->SetColorIndex2(252);
		PointPrimitive->SetPointDisplayMode(8);
		m_PreviewGroup.AddTail(PointPrimitive);
	}
}

void AeSysView::ConstructPreviewGroupForNodalGroups() {
	const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	auto GroupPosition {GetDocument()->GetFirstNodalGroupPosition()};
	while (GroupPosition != nullptr) {
		const auto Group {GetDocument()->GetNextNodalGroup(GroupPosition)};
		auto PrimitivePosition {Group->GetHeadPosition()};
		while (PrimitivePosition != nullptr) {
			const auto Primitive {Group->GetNext(PrimitivePosition)};
			m_PreviewGroup.AddTail(Primitive->Clone(BlockTableRecord));
		}
	}
	auto UniquePointPosition {GetDocument()->GetFirstUniquePointPosition()};
	while (UniquePointPosition != nullptr) {
		const auto UniquePoint {GetDocument()->GetNextUniquePoint(UniquePointPosition)};
		auto PointPrimitive {new EoDbPoint(UniquePoint->m_Point)};
		PointPrimitive->SetColorIndex2(252);
		PointPrimitive->SetPointDisplayMode(8);
		m_PreviewGroup.AddTail(PointPrimitive);
	}
}
