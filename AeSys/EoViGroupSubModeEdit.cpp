#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

void AeSysView::OnModeGroupEdit() {
	InitializeGroupAndPrimitiveEdit();

	m_SubModeEditBeginPoint = GetCursorPosition();

	auto Group {SelectGroupAndPrimitive(m_SubModeEditBeginPoint)};

	if (Group != nullptr) {
		m_SubModeEditGroup = Group;
		theApp.LoadModeResources(ID_MODE_GROUP_EDIT);
	}
}

void AeSysView::DoEditGroupCopy() {

	if (m_SubModeEditGroup != 0) {
		auto Group {new EoDbGroup(*m_SubModeEditGroup)};

		GetDocument()->AddWorkLayerGroup(Group);
		m_SubModeEditGroup = Group;

		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, m_SubModeEditGroup);
		m_tmEditSeg.setToIdentity();
	}
}

void AeSysView::DoEditGroupEscape() {
	if (m_SubModeEditGroup != 0) {
		m_tmEditSeg.invert();

		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, m_SubModeEditGroup);
		m_SubModeEditGroup->TransformBy(m_tmEditSeg);
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, m_SubModeEditGroup);

		InitializeGroupAndPrimitiveEdit();

		theApp.LoadModeResources(theApp.PrimaryMode());
	}
}

void AeSysView::DoEditGroupTransform(unsigned short operation) {

	if (m_SubModeEditGroup != 0) {
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

		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, m_SubModeEditGroup);
		m_SubModeEditGroup->TransformBy(TransformMatrix);
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, m_SubModeEditGroup);

		m_tmEditSeg.preMultBy(TransformMatrix);
	}
}

void AeSysView::PreviewGroupEdit() {
	auto Document {GetDocument()};

	if (m_SubModeEditGroup != 0) {
		m_SubModeEditEndPoint = GetCursorPosition();
		EoGeMatrix3d tm;
		tm.setToTranslation(m_SubModeEditEndPoint - m_SubModeEditBeginPoint);

		if (theApp.IsTrapHighlighted() && Document->FindTrappedGroup(m_SubModeEditGroup) != 0) {
			EoDbPrimitive::SetHighlightColorIndex(theApp.TrapHighlightColor());
		}
		Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, m_SubModeEditGroup);
		m_SubModeEditGroup->TransformBy(tm);
		Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, m_SubModeEditGroup);

		EoDbPrimitive::SetHighlightColorIndex(0);

		m_tmEditSeg *= tm;

		m_SubModeEditBeginPoint = m_SubModeEditEndPoint;
	}
}

void AeSysDoc::InitializeGroupAndPrimitiveEdit() {
	auto ViewPosition {GetFirstViewPosition()};

	while (ViewPosition != nullptr) {
		auto View {dynamic_cast<AeSysView*>(GetNextView(ViewPosition))};
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
