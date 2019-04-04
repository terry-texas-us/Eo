#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

void AeSysView::OnModeGroupEdit() {
	InitializeGroupAndPrimitiveEdit();

	m_SubModeEditBeginPoint = GetCursorPosition();

	EoDbGroup* Group = SelectGroupAndPrimitive(m_SubModeEditBeginPoint);

	if (Group != 0) {
		m_SubModeEditGroup = Group;
		theApp.LoadModeResources(ID_MODE_GROUP_EDIT);
	}
}
void AeSysView::DoEditGroupCopy() {
	AeSysDoc* Document = GetDocument();
	if (m_SubModeEditGroup != 0) {
		EoDbGroup* Group = new EoDbGroup(*m_SubModeEditGroup);

		Document->AddWorkLayerGroup(Group);
		m_SubModeEditGroup = Group;

		Document->UpdateGroupInAllViews(kGroupEraseSafe, m_SubModeEditGroup);
		m_tmEditSeg.setToIdentity();
	}
}
void AeSysView::DoEditGroupEscape() {
	if (m_SubModeEditGroup != 0) {
		m_tmEditSeg.invert();

		GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, m_SubModeEditGroup);
 		m_SubModeEditGroup->TransformBy(m_tmEditSeg);
		GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, m_SubModeEditGroup);

		InitializeGroupAndPrimitiveEdit();

		theApp.LoadModeResources(theApp.PrimaryMode());
	}
}
void AeSysView::DoEditGroupTransform(OdUInt16 operation) {
	AeSysDoc* Document = GetDocument();
	if (m_SubModeEditGroup != 0) {
		EoGeMatrix3d TransformMatrix;
		TransformMatrix.setToTranslation(- m_SubModeEditBeginPoint.asVector());

		if (operation == ID_OP2) {
			TransformMatrix.preMultBy(EditModeRotationMatrix());
		}
		else if (operation == ID_OP3) {
			TransformMatrix.preMultBy(EditModeInvertedRotationMatrix());
		}
		else if (operation == ID_OP6) {
			EoGeMatrix3d ScaleMatrix;
			EditModeMirrorScaleFactors().getMatrix(ScaleMatrix);
			TransformMatrix.preMultBy(ScaleMatrix);
		}
		else if (operation == ID_OP7) {
			EoGeMatrix3d ScaleMatrix;
			EditModeScaleFactors().inverse().getMatrix(ScaleMatrix);
			TransformMatrix.preMultBy(ScaleMatrix);
		}
		else if (operation == ID_OP8) {
			EoGeMatrix3d ScaleMatrix;
			EditModeScaleFactors().getMatrix(ScaleMatrix);
			TransformMatrix.preMultBy(ScaleMatrix);
		}
		EoGeMatrix3d OriginToBeginPointMatrix;
		OriginToBeginPointMatrix.setToTranslation(m_SubModeEditBeginPoint.asVector());
		TransformMatrix.preMultBy(OriginToBeginPointMatrix);

		Document->UpdateGroupInAllViews(kGroupEraseSafe, m_SubModeEditGroup);
		m_SubModeEditGroup->TransformBy(TransformMatrix);
		Document->UpdateGroupInAllViews(kGroupEraseSafe, m_SubModeEditGroup);

		m_tmEditSeg.preMultBy(TransformMatrix);
	}
}
void AeSysView::PreviewGroupEdit() {
	AeSysDoc* Document = GetDocument();
	if (m_SubModeEditGroup != 0) {
		m_SubModeEditEndPoint = GetCursorPosition();
		EoGeMatrix3d tm;
		tm.setToTranslation(m_SubModeEditEndPoint - m_SubModeEditBeginPoint);

		if (theApp.IsTrapHighlighted() && Document->FindTrappedGroup(m_SubModeEditGroup) != 0) {
			EoDbPrimitive::SetHighlightColorIndex(theApp.TrapHighlightColor());
		}
		Document->UpdateGroupInAllViews(kGroupEraseSafe, m_SubModeEditGroup);
		m_SubModeEditGroup->TransformBy(tm);
		Document->UpdateGroupInAllViews(kGroupEraseSafe, m_SubModeEditGroup);

		EoDbPrimitive::SetHighlightColorIndex(0);

		m_tmEditSeg *= tm;

		m_SubModeEditBeginPoint = m_SubModeEditEndPoint;
	}
}
void AeSysDoc::InitializeGroupAndPrimitiveEdit() {
	POSITION Position = GetFirstViewPosition();
	while (Position != 0) {
		AeSysView* View = (AeSysView*) GetNextView(Position);
		View->InitializeGroupAndPrimitiveEdit();
	}
}
void AeSysView::InitializeGroupAndPrimitiveEdit() {
	m_SubModeEditBeginPoint = OdGePoint3d::kOrigin;
	m_SubModeEditEndPoint = m_SubModeEditBeginPoint;

	m_SubModeEditGroup = 0;
	m_SubModeEditPrimitive = 0;

	m_tmEditSeg.setToIdentity();
}
