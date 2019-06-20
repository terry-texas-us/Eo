#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

#include "EoDbHatch.h"
#include "EoDbPolyline.h"

void AeSysView::OnModePrimitiveMend() {
	const auto CurrentPnt {GetCursorPosition()};
	EoGePoint4d ptView(CurrentPnt, 1.0);
	ModelViewTransformPoint(ptView);

	m_PrimitiveToMend = nullptr;

	if (GroupIsEngaged()) { // Group is currently engaged, see if cursor is on a control point
		OdGePoint3d ptDet;
		auto Primitive {EngagedPrimitive()};

		EoDbHatch::SetEdgeToEvaluate(EoDbHatch::Edge());
		EoDbPolyline::SetEdgeToEvaluate(EoDbPolyline::Edge());

		if (Primitive->SelectUsingPoint(ptView, this, ptDet)) { // Cursor is close enough to engaged primitive to use it first
			m_PrimitiveToMend = Primitive;
		}
	}
	if (m_PrimitiveToMend == nullptr) { // No engaged group, or engaged primitive to far from cursor
		if (SelectGroupAndPrimitive(m_MendPrimitiveBegin) != nullptr) { // Group successfully engaged
			m_PrimitiveToMend = EngagedPrimitive();
		}
	}
	m_MendPrimitiveBegin = CurrentPnt;

	if (m_PrimitiveToMend != nullptr) {
		OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
		m_PrimitiveToMendCopy = m_PrimitiveToMend->Clone(BlockTableRecord);
		m_MendPrimitiveBegin = m_PrimitiveToMend->SelectAtControlPoint(this, ptView);
		m_MendPrimitiveVertexIndex = static_cast<unsigned long>(1 << EoDbPrimitive::ControlPointIndex());

		theApp.LoadModeResources(ID_MODE_PRIMITIVE_MEND);
	}
}
void AeSysView::PreviewMendPrimitive() {
	const auto CurrentPnt {GetCursorPosition()};
	const auto Translate {CurrentPnt - m_MendPrimitiveBegin};
	GetDocument()->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, m_PrimitiveToMendCopy);
	m_PrimitiveToMendCopy->TranslateUsingMask(Translate, m_MendPrimitiveVertexIndex);
	GetDocument()->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, m_PrimitiveToMendCopy);
	m_MendPrimitiveBegin = CurrentPnt;
}
void AeSysView::MendPrimitiveReturn() {
	GetDocument()->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, m_PrimitiveToMendCopy);
	// <tas="May have been broken when Assign method replaced operator=()"</tas>
	m_PrimitiveToMend = m_PrimitiveToMendCopy;
	GetDocument()->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, m_PrimitiveToMend);

	delete m_PrimitiveToMendCopy;

	theApp.LoadModeResources(static_cast<unsigned>(theApp.PrimaryMode()));
}
void AeSysView::MendPrimitiveEscape() {
	GetDocument()->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, m_PrimitiveToMendCopy);
	GetDocument()->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, m_PrimitiveToMend);

	delete m_PrimitiveToMendCopy;

	theApp.LoadModeResources(static_cast<unsigned>(theApp.PrimaryMode()));
}
