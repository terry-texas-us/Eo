#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

void AeSysView::OnModePrimitiveMend() {
	const OdGePoint3d CurrentPnt = GetCursorPosition();
	EoGePoint4d ptView(CurrentPnt, 1.);
	ModelViewTransformPoint(ptView);

	m_PrimitiveToMend = 0;

	if (GroupIsEngaged()) { // Group is currently engaged, see if cursor is on a control point
		OdGePoint3d ptDet;

		EoDbPrimitive* Primitive = EngagedPrimitive();

		EoDbHatch::SetEdgeToEvaluate(EoDbHatch::Edge());
		EoDbPolyline::SetEdgeToEvaluate(EoDbPolyline::Edge());

		if (Primitive->SelectBy(ptView, this, ptDet)) { // Cursor is close enough to engaged primitive to use it first
			m_PrimitiveToMend = Primitive;
		}
	}
	if (m_PrimitiveToMend == 0) { // No engaged group, or engaged primitive to far from cursor
		if (SelectGroupAndPrimitive(m_MendPrimitiveBegin) != nullptr) { // Group successfully engaged
			m_PrimitiveToMend = EngagedPrimitive();
		}
	}
	m_MendPrimitiveBegin = CurrentPnt;

	if (m_PrimitiveToMend != 0) {
		m_PrimitiveToMendCopy = m_PrimitiveToMend->Clone(Database());
		m_MendPrimitiveBegin = m_PrimitiveToMend->SelectAtControlPoint(this, ptView);
		m_MendPrimitiveVertexIndex = 1 << EoDbPrimitive::ControlPointIndex();

		theApp.LoadModeResources(ID_MODE_PRIMITIVE_MEND);
	}
}
void AeSysView::PreviewMendPrimitive() {
	const OdGePoint3d CurrentPnt = GetCursorPosition();
	const OdGeVector3d Translate(CurrentPnt - m_MendPrimitiveBegin);
	GetDocument()->UpdatePrimitiveInAllViews(kPrimitiveEraseSafe, m_PrimitiveToMendCopy);
	m_PrimitiveToMendCopy->TranslateUsingMask(Translate, m_MendPrimitiveVertexIndex);
	GetDocument()->UpdatePrimitiveInAllViews(kPrimitiveSafe, m_PrimitiveToMendCopy);
	m_MendPrimitiveBegin = CurrentPnt;
}
void AeSysView::MendPrimitiveReturn() {
	GetDocument()->UpdatePrimitiveInAllViews(kPrimitiveEraseSafe, m_PrimitiveToMendCopy);
	// <tas="May have been broken when Assign method replaced operator=()"</tas>
	m_PrimitiveToMend = m_PrimitiveToMendCopy;
	GetDocument()->UpdatePrimitiveInAllViews(kPrimitiveSafe, m_PrimitiveToMend);

	delete m_PrimitiveToMendCopy;

	theApp.LoadModeResources(theApp.PrimaryMode());
}
void AeSysView::MendPrimitiveEscape() {
	GetDocument()->UpdatePrimitiveInAllViews(kPrimitiveEraseSafe, m_PrimitiveToMendCopy);
	GetDocument()->UpdatePrimitiveInAllViews(kPrimitiveSafe, m_PrimitiveToMend);

	delete m_PrimitiveToMendCopy;

	theApp.LoadModeResources(theApp.PrimaryMode());
}
