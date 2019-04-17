#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

double NodalModePickTolerance = .05;
OdUInt16 PreviousNodalCommand = 0;
OdGePoint3d PreviousNodalCursorPosition;

void AeSysView::OnNodalModeAddRemove(void) {
	theApp.m_NodalModeAddGroups = !theApp.m_NodalModeAddGroups;
	if (theApp.m_NodalModeAddGroups) {
		SetModeCursor(ID_MODE_NODAL);
	}
	else {
		SetModeCursor(ID_MODE_NODALR);
	}
}
void AeSysView::OnNodalModePoint(void) {
	const OdGePoint3d CurrentPnt = GetCursorPosition();
	OdGePoint3dArray Points;

	POSITION GroupPosition = GetFirstVisibleGroupPosition();
	while (GroupPosition != 0) {
		EoDbGroup* Group = GetNextVisibleGroup(GroupPosition);

		POSITION PrimitivePosition = Group->GetHeadPosition();
		while (PrimitivePosition != 0) {
			EoDbPrimitive* Primitive = Group->GetNext(PrimitivePosition);

			const DWORD Mask = GetDocument()->GetPrimitiveMask(Primitive);
			Primitive->GetAllPoints(Points);

			for (size_t i = 0; i < Points.size(); i++) {
				if (OdGeVector3d(CurrentPnt - Points[i]).length() <= NodalModePickTolerance) {
					GetDocument()->UpdateNodalList(Group, Primitive, Mask, i, Points[i]);
				}
			}
		}
	}
}
void AeSysView::OnNodalModeLine(void) {
	const OdGePoint3d CurrentPnt = GetCursorPosition();
	OdGePoint3dArray Points;
	EoDbGroup* Group = SelectGroupAndPrimitive(CurrentPnt);
	if (Group != 0) {
		EoDbPrimitive* Primitive = EngagedPrimitive();

		const DWORD Mask = GetDocument()->GetPrimitiveMask(Primitive);
		Primitive->GetAllPoints(Points);

		for (size_t i = 0; i < Points.size(); i++) {
			GetDocument()->UpdateNodalList(Group, Primitive, Mask, i, Points[i]);
		}
	}
}
void AeSysView::OnNodalModeArea(void) {
	const OdGePoint3d CurrentPnt = GetCursorPosition();
	if (PreviousNodalCommand != ID_OP3) {
		PreviousNodalCursorPosition = CurrentPnt;
		RubberBandingStartAtEnable(CurrentPnt, Rectangles);
		PreviousNodalCommand = ModeLineHighlightOp(ID_OP3);
	}
	else {
		if (PreviousNodalCursorPosition != CurrentPnt) {
			OdGePoint3dArray Points;
			OdGePoint3d	MinExtent;
			OdGePoint3d	MaxExtent;
			EoGeLineSeg3d(PreviousNodalCursorPosition, CurrentPnt).Extents(MinExtent, MaxExtent);

			POSITION GroupPosition = GetFirstVisibleGroupPosition();
			while (GroupPosition != 0) {
				EoDbGroup* Group = GetNextVisibleGroup(GroupPosition);

				POSITION PrimitivePosition = Group->GetHeadPosition();
				while (PrimitivePosition != 0) {
					EoDbPrimitive* Primitive = Group->GetNext(PrimitivePosition);
					const DWORD Mask = GetDocument()->GetPrimitiveMask(Primitive);
					Primitive->GetAllPoints(Points);

					for (size_t i = 0; i < Points.size(); i++) {
						if (ContainmentOf(Points[i], MinExtent, MaxExtent)) {
							GetDocument()->UpdateNodalList(Group, Primitive, Mask, i, Points[i]);
						}
					}
				}
			}
		}
		RubberBandingDisable();
		ModeLineUnhighlightOp(PreviousNodalCommand);
	}
}
void AeSysView::OnNodalModeMove(void) {
	const OdGePoint3d CurrentPnt = GetCursorPosition();
	if (PreviousNodalCommand != ID_OP4) {
		PreviousNodalCommand = ModeLineHighlightOp(ID_OP4);
		m_NodalModePoints.clear();
		m_NodalModePoints.append(CurrentPnt);
		RubberBandingStartAtEnable(CurrentPnt, Lines);
		ConstructPreviewGroup();
	}
	else {
		OnNodalModeReturn();
	}
}
void AeSysView::OnNodalModeCopy(void) {
	const OdGePoint3d CurrentPnt = GetCursorPosition();
	if (PreviousNodalCommand != ID_OP5) {
		PreviousNodalCommand = ModeLineHighlightOp(ID_OP5);
		m_NodalModePoints.clear();
		m_NodalModePoints.append(CurrentPnt);
		RubberBandingStartAtEnable(CurrentPnt, Lines);
		ConstructPreviewGroupForNodalGroups();
	}
	else {
		OnNodalModeReturn();
	}
}
void AeSysView::OnNodalModeToLine(void) {
	OdGePoint3d CurrentPnt = GetCursorPosition();
	if (PreviousNodalCommand != ID_OP6) {
		PreviousNodalCursorPosition = CurrentPnt;
		RubberBandingStartAtEnable(CurrentPnt, Lines);
		PreviousNodalCommand = ModeLineHighlightOp(ID_OP6);
	}
	else {
		if (PreviousNodalCursorPosition != CurrentPnt) {
			CurrentPnt = SnapPointToAxis(PreviousNodalCursorPosition, CurrentPnt);
			const OdGeVector3d Translate(CurrentPnt - PreviousNodalCursorPosition);

			EoDbGroup* Group = new EoDbGroup;

			POSITION PointPosition = GetDocument()->GetFirstUniquePointPosition();
			while (PointPosition != 0) {
				EoGeUniquePoint* UniquePoint = GetDocument()->GetNextUniquePoint(PointPosition);
                auto Primitive {EoDbLine::Create2(UniquePoint->m_Point, UniquePoint->m_Point + Translate)};
				Group->AddTail(Primitive);
			}
			GetDocument()->AddWorkLayerGroup(Group);
			GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);

			SetCursorPosition(CurrentPnt);
		}
		RubberBandingDisable();
		ModeLineUnhighlightOp(PreviousNodalCommand);
	}
}
/// <remarks>
/// The pen color used for any polygons added to drawing is the current pen color and not the pen color of the reference primitives.
/// </remarks>
void AeSysView::OnNodalModeToPolygon(void) {
	OdGePoint3d CurrentPnt = GetCursorPosition();
	if (PreviousNodalCommand != ID_OP7) {
		PreviousNodalCursorPosition = CurrentPnt;
		RubberBandingStartAtEnable(CurrentPnt, Lines);
		PreviousNodalCommand = ModeLineHighlightOp(ID_OP7);
	}
	else {
		if (PreviousNodalCursorPosition != CurrentPnt) {
			CurrentPnt = SnapPointToAxis(PreviousNodalCursorPosition, CurrentPnt);
			const OdGeVector3d Translate(CurrentPnt - PreviousNodalCursorPosition);

			OdGePoint3dArray Points;
			Points.setLogicalLength(4);

			CDC* DeviceContext = GetDC();

			const int PrimitiveState = pstate.Save();

			POSITION GroupPosition = GetDocument()->GetFirstNodalGroupPosition();
			while (GroupPosition != 0) {
				EoDbGroup* Group = GetDocument()->GetNextNodalGroup(GroupPosition);

				POSITION PrimitivePosition = Group->GetHeadPosition();
				while (PrimitivePosition != 0) {
					EoDbPrimitive* Primitive = Group->GetNext(PrimitivePosition);

					const DWORD Mask = GetDocument()->GetPrimitiveMask(Primitive);
					if (Mask != 0) {
						if (Primitive->Is(kLinePrimitive)) {
							if ((Mask & 3) == 3) {
								EoDbLine* LinePrimitive = static_cast<EoDbLine*>(Primitive);

								Points[0] = LinePrimitive->StartPoint();
								Points[1] = LinePrimitive->EndPoint();
								Points[2] = Points[1] + Translate;
								Points[3] = Points[0] + Translate;
								// <tas="Behavior changed. Line extruded into solid hatch primitive"</tas>
								EoDbGroup* NewGroup = new EoDbGroup;
								EoDbHatch* NewHatch = EoDbHatch::Create0(Database());
								NewHatch->SetInteriorStyle(EoDbHatch::kSolid);
								NewHatch->SetVertices(Points);
								NewGroup->AddTail(NewHatch);
								GetDocument()->AddWorkLayerGroup(NewGroup);
								GetDocument()->UpdateGroupInAllViews(kGroupSafe, NewGroup);
							}
						}
						else if (Primitive->Is(kHatchPrimitive)) {
							EoDbHatch* Hatch = static_cast<EoDbHatch*>(Primitive);
							const int iPts = Hatch->NumberOfVertices();

							for (int i = 0; i < iPts; i++) {
								if (btest(Mask, i) && btest(Mask, ((i + 1) % iPts))) {
									Points[0] = Hatch->GetPointAt(i);
									Points[1] = Hatch->GetPointAt((i + 1) % iPts);
									Points[2] = Points[1] + Translate;
									Points[3] = Points[0] + Translate;
									// <tas="Behavior changed. Edges of hatch extruded into solid hatch primitives"</tas>
									EoDbGroup* NewGroup = new EoDbGroup;
									EoDbHatch* NewHatch = EoDbHatch::Create0(Database());
									NewHatch->SetInteriorStyle(EoDbHatch::kSolid);
									NewHatch->SetVertices(Points);
									NewGroup->AddTail(NewHatch);
									GetDocument()->AddWorkLayerGroup(NewGroup);
									GetDocument()->UpdateGroupInAllViews(kGroupSafe, NewGroup);
								}
							}
						}
					}
				}
			}
			pstate.Restore(DeviceContext, PrimitiveState);

			SetCursorPosition(CurrentPnt);
			RubberBandingDisable();
			ModeLineUnhighlightOp(PreviousNodalCommand);
		}
	}
}
void AeSysView::OnNodalModeEmpty(void) {
	OnNodalModeEscape();
}
void AeSysView::OnNodalModeEngage(void) {
	if (GroupIsEngaged()) {
		const DWORD Mask = GetDocument()->GetPrimitiveMask(EngagedPrimitive());
		OdGePoint3dArray Points;

		EngagedPrimitive()->GetAllPoints(Points);

		for (size_t i = 0; i < Points.size(); i++) {
			GetDocument()->UpdateNodalList(EngagedGroup(), EngagedPrimitive(), Mask, i, Points[i]);
		}
	}
}
void AeSysView::OnNodalModeReturn(void) {
	OdGePoint3d CurrentPnt = GetCursorPosition();

	switch (PreviousNodalCommand) {
	case ID_OP4:
		if (m_NodalModePoints[0] != CurrentPnt) {
			CurrentPnt = SnapPointToAxis(m_NodalModePoints[0], CurrentPnt);
			const OdGeVector3d Translate(CurrentPnt - m_NodalModePoints[0]);

			POSITION MaskedPrimitivePosition = GetDocument()->GetFirstMaskedPrimitivePosition();
			while (MaskedPrimitivePosition != 0) {
				EoDbMaskedPrimitive* MaskedPrimitive = GetDocument()->GetNextMaskedPrimitive(MaskedPrimitivePosition);
				EoDbPrimitive* Primitive = MaskedPrimitive->GetPrimitive();
				const DWORD Mask = MaskedPrimitive->GetMask();
				Primitive->TranslateUsingMask(Translate, Mask);
			}
			EoGeUniquePoint* Point;

			POSITION UniquePointPosition = GetDocument()->GetFirstUniquePointPosition();
			while (UniquePointPosition != 0) {
				Point = GetDocument()->GetNextUniquePoint(UniquePointPosition);
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

			POSITION GroupPosition = GetDocument()->GetFirstNodalGroupPosition();
			while (GroupPosition != 0) {
				EoDbGroup* Group = GetDocument()->GetNextNodalGroup(GroupPosition);
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
void AeSysView::OnNodalModeEscape(void) {
	if (PreviousNodalCommand == 0) {
		GetDocument()->DisplayUniquePoints();
		GetDocument()->DeleteNodalResources();
	}
	else {
		RubberBandingDisable();
		GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);

		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
		ConstructPreviewGroup();
		GetDocument()->UpdateGroupInAllViews(kGroupSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
		m_NodalModePoints.clear();

		ModeLineUnhighlightOp(PreviousNodalCommand);
	}
}
void AeSysView::DoNodalModeMouseMove() {
	OdGePoint3d CurrentPnt = GetCursorPosition();
	const int NumberOfPoints = m_NodalModePoints.size();

	switch (PreviousNodalCommand) {
	case ID_OP4:
		VERIFY(m_NodalModePoints.size() > 0);

		if (m_NodalModePoints[0] != CurrentPnt) {
			CurrentPnt = SnapPointToAxis(m_NodalModePoints[0], CurrentPnt);
			m_NodalModePoints.append(CurrentPnt);

			const OdGeVector3d Translate(CurrentPnt - m_NodalModePoints[0]);

			GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
			m_PreviewGroup.DeletePrimitivesAndRemoveAll();

			POSITION MaskedPrimitivePosition = GetDocument()->GetFirstMaskedPrimitivePosition();
			while (MaskedPrimitivePosition != 0) {
				EoDbMaskedPrimitive* MaskedPrimitive = GetDocument()->GetNextMaskedPrimitive(MaskedPrimitivePosition);
				EoDbPrimitive* Primitive = MaskedPrimitive->GetPrimitive();
				const DWORD Mask = MaskedPrimitive->GetMask();
				m_PreviewGroup.AddTail(Primitive->Clone(Database()));
				((EoDbPrimitive*) m_PreviewGroup.GetTail())->TranslateUsingMask(Translate, Mask);
			}
			POSITION UniquePointPosition = GetDocument()->GetFirstUniquePointPosition();
			while (UniquePointPosition != 0) {
				EoGeUniquePoint* UniquePoint = GetDocument()->GetNextUniquePoint(UniquePointPosition);
				const OdGePoint3d Point = (UniquePoint->m_Point) + Translate;
				EoDbPoint* PointPrimitive = new EoDbPoint(Point);
				PointPrimitive->SetColorIndex(252);
				PointPrimitive->SetPointDisplayMode(8);
				m_PreviewGroup.AddTail(PointPrimitive);
			}
			GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
		}
		break;

	case ID_OP5:

		if (m_NodalModePoints[0] != CurrentPnt) {
			CurrentPnt = SnapPointToAxis(m_NodalModePoints[0], CurrentPnt);
			m_NodalModePoints.append(CurrentPnt);
			EoGeMatrix3d TranslationMatrix;
			TranslationMatrix.setToTranslation(CurrentPnt - m_NodalModePoints[0]);

			GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
			m_PreviewGroup.DeletePrimitivesAndRemoveAll();
			ConstructPreviewGroupForNodalGroups();
			m_PreviewGroup.TransformBy(TranslationMatrix);
			GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
		}
		break;
	}
	m_NodalModePoints.setLogicalLength(NumberOfPoints);
}

void AeSysView::ConstructPreviewGroup() {
	m_PreviewGroup.DeletePrimitivesAndRemoveAll();

	POSITION MaskedPrimitivePosition = GetDocument()->GetFirstMaskedPrimitivePosition();
	while (MaskedPrimitivePosition != 0) {
		EoDbMaskedPrimitive* MaskedPrimitive = GetDocument()->GetNextMaskedPrimitive(MaskedPrimitivePosition);
		EoDbPrimitive* Primitive = MaskedPrimitive->GetPrimitive();
		m_PreviewGroup.AddTail(Primitive->Clone(Database()));
	}
	POSITION UniquePointPosition = GetDocument()->GetFirstUniquePointPosition();
	while (UniquePointPosition != 0) {
		EoGeUniquePoint* UniquePoint = GetDocument()->GetNextUniquePoint(UniquePointPosition);
		EoDbPoint* PointPrimitive = new EoDbPoint(UniquePoint->m_Point);
		PointPrimitive->SetColorIndex(252);
		PointPrimitive->SetPointDisplayMode(8);
		m_PreviewGroup.AddTail(PointPrimitive);
	}
}
void AeSysView::ConstructPreviewGroupForNodalGroups() {
	POSITION GroupPosition = GetDocument()->GetFirstNodalGroupPosition();
	while (GroupPosition != 0) {
		EoDbGroup* Group = GetDocument()->GetNextNodalGroup(GroupPosition);

		POSITION PrimitivePosition = Group->GetHeadPosition();
		while (PrimitivePosition != 0) {
			EoDbPrimitive* Primitive = Group->GetNext(PrimitivePosition);
			m_PreviewGroup.AddTail(Primitive->Clone(Database()));
		}
	}
	POSITION UniquePointPosition = GetDocument()->GetFirstUniquePointPosition();
	while (UniquePointPosition != 0) {
		EoGeUniquePoint* UniquePoint = GetDocument()->GetNextUniquePoint(UniquePointPosition);
		EoDbPoint* PointPrimitive = new EoDbPoint(UniquePoint->m_Point);
		PointPrimitive->SetColorIndex(252);
		PointPrimitive->SetPointDisplayMode(8);
		m_PreviewGroup.AddTail(PointPrimitive);
	}
}
