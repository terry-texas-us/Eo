#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

void AeSysView::OnModePrimitiveEdit() {
	InitializeGroupAndPrimitiveEdit();

	m_SubModeEditBeginPoint = GetCursorPosition();

	auto Group {SelectGroupAndPrimitive(m_SubModeEditBeginPoint)};

	if (Group != nullptr) {
		m_SubModeEditGroup = Group;
		m_SubModeEditPrimitive = EngagedPrimitive();
		theApp.LoadModeResources(ID_MODE_PRIMITIVE_EDIT);
	}
}
void AeSysView::DoEditPrimitiveCopy() {
	if (m_SubModeEditPrimitive != 0) {
		OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
		EoDbPrimitive* Primitive = m_SubModeEditPrimitive->Clone(BlockTableRecord);
		m_SubModeEditPrimitive = Primitive;
		m_SubModeEditGroup = new EoDbGroup;
		m_SubModeEditGroup->AddTail(m_SubModeEditPrimitive);
		GetDocument()->AddWorkLayerGroup(m_SubModeEditGroup);

		GetDocument()->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, m_SubModeEditPrimitive);
		m_tmEditSeg.setToIdentity();
	}
}
void AeSysView::DoEditPrimitiveEscape() {
	if (m_SubModeEditPrimitive != 0) {
		m_tmEditSeg.invert();

		GetDocument()->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, m_SubModeEditPrimitive);
		m_SubModeEditPrimitive->TransformBy(m_tmEditSeg);
		GetDocument()->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, m_SubModeEditPrimitive);

		InitializeGroupAndPrimitiveEdit();

		theApp.LoadModeResources(theApp.PrimaryMode());
	}
}
void AeSysView::DoEditPrimitiveTransform(unsigned short operation) {
	if (m_SubModeEditPrimitive != 0) {
		EoGeMatrix3d TransformMatrix;
		TransformMatrix.setToTranslation(-m_SubModeEditBeginPoint.asVector());

		if (operation == ID_OP2) {
			TransformMatrix.preMultBy(EditModeRotationMatrix());
		} else if (operation == ID_OP3) {
			TransformMatrix.preMultBy(EditModeInvertedRotationMatrix());
		} else if (operation == ID_OP6) {
			EoGeMatrix3d ScaleMatrix;
			EditModeMirrorScaleFactors().getMatrix(ScaleMatrix);
			TransformMatrix.preMultBy(ScaleMatrix);
		} else if (operation == ID_OP7) {
			EoGeMatrix3d ScaleMatrix;
			EditModeScaleFactors().inverse().getMatrix(ScaleMatrix);
			TransformMatrix.preMultBy(ScaleMatrix);
		} else if (operation == ID_OP8) {
			EoGeMatrix3d ScaleMatrix;
			EditModeScaleFactors().getMatrix(ScaleMatrix);
			TransformMatrix.preMultBy(ScaleMatrix);
		}
		EoGeMatrix3d OriginToBeginPointMatrix;
		OriginToBeginPointMatrix.setToTranslation(m_SubModeEditBeginPoint.asVector());
		TransformMatrix.preMultBy(OriginToBeginPointMatrix);

		GetDocument()->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, m_SubModeEditPrimitive);
		m_SubModeEditPrimitive->TransformBy(TransformMatrix);
		GetDocument()->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, m_SubModeEditPrimitive);

		m_tmEditSeg.preMultBy(TransformMatrix);
	}
}
void AeSysView::PreviewPrimitiveEdit() {
	if (m_SubModeEditPrimitive != 0) {
		m_SubModeEditEndPoint = GetCursorPosition();
		EoGeMatrix3d TransformMatrix;
		TransformMatrix.setToTranslation(m_SubModeEditEndPoint - m_SubModeEditBeginPoint);

		if (theApp.IsTrapHighlighted() && GetDocument()->FindTrappedGroup(m_SubModeEditGroup) != 0)
			EoDbPrimitive::SetHighlightColorIndex(theApp.TrapHighlightColor());

		GetDocument()->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, m_SubModeEditPrimitive);
		m_SubModeEditPrimitive->TransformBy(TransformMatrix);
		GetDocument()->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, m_SubModeEditPrimitive);

		EoDbPrimitive::SetHighlightColorIndex(0);

		m_tmEditSeg.preMultBy(TransformMatrix);

		m_SubModeEditBeginPoint = m_SubModeEditEndPoint;
	}
}
