#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDlgEditOptions.h"

void AeSysView::OnEditModeOptions() {
	EoDlgEditOptions Dialog(this);
	Dialog.m_ScaleFactorX = m_ScaleFactors.sx;
	Dialog.m_ScaleFactorY = m_ScaleFactors.sy;
	Dialog.m_ScaleFactorZ = m_ScaleFactors.sz;
	Dialog.m_EditModeRotationAngleX = m_EditModeRotationAngles.x;
	Dialog.m_EditModeRotationAngleY = m_EditModeRotationAngles.y;
	Dialog.m_EditModeRotationAngleZ = m_EditModeRotationAngles.z;

	if (Dialog.DoModal() == IDOK) {
		m_EditModeRotationAngles.x = Dialog.m_EditModeRotationAngleX;
		m_EditModeRotationAngles.y = Dialog.m_EditModeRotationAngleY;
		m_EditModeRotationAngles.z = Dialog.m_EditModeRotationAngleZ;
		m_ScaleFactors.set(Dialog.m_ScaleFactorX, Dialog.m_ScaleFactorY, Dialog.m_ScaleFactorZ);
	}
}
OdGeVector3d AeSysView::EditModeRotationAngles() const {
	return m_EditModeRotationAngles;
}
EoGeMatrix3d AeSysView::EditModeInvertedRotationMatrix() const {
	EoGeMatrix3d Matrix;
	Matrix.SetTo3AxisRotation(EditModeRotationAngles());
	Matrix.invert();
	return Matrix;
}
EoGeMatrix3d AeSysView::EditModeRotationMatrix() const {
	EoGeMatrix3d Matrix;
	Matrix.SetTo3AxisRotation(EditModeRotationAngles());
	return Matrix;
}
OdGeScale3d AeSysView::EditModeScaleFactors() const {
	return m_ScaleFactors;
}
void AeSysView::SetEditModeScaleFactors(const double x, const double y, const double z) {
	// <tas="Verify scale factors are always not zero"</tas>
	m_ScaleFactors.sx = x;
	m_ScaleFactors.sy = y;
	m_ScaleFactors.sz = z;
}
void AeSysView::SetEditModeRotationAngles(double x, double y, double z) {
	m_EditModeRotationAngles.x = x;
	m_EditModeRotationAngles.y = y;
	m_EditModeRotationAngles.z = z;
}
OdGeScale3d AeSysView::EditModeMirrorScaleFactors() const {
	return m_MirrorScaleFactors;
}
void AeSysView::SetEditModeMirrorScaleFactors(double sx, double sy, double sz) {
	m_MirrorScaleFactors.set(sx, sy, sz);
}

void AeSysView::OnEditModePivot() {
	AeSysDoc* Document = GetDocument();

	const OdGePoint3d pt = GetCursorPosition();
	Document->SetTrapPivotPoint(pt);
	// pSetSegPos(pTRAP_PVT_MRK_ID, pt);
}
void AeSysView::OnEditModeRotccw() {
	AeSysDoc* Document = GetDocument();
	OdGePoint3d PivotPoint(Document->TrapPivotPoint());

	EoGeMatrix3d TransformMatrix;
	TransformMatrix.setToTranslation(- PivotPoint.asVector());
	EoGeMatrix3d RotationMatrix;
	RotationMatrix.SetTo3AxisRotation(EditModeRotationAngles());
	TransformMatrix.preMultBy(RotationMatrix);
	EoGeMatrix3d OriginToPivotPointMatrix;
	OriginToPivotPointMatrix.setToTranslation(PivotPoint.asVector());
	TransformMatrix.preMultBy(OriginToPivotPointMatrix);
	Document->TransformTrappedGroups(TransformMatrix);
}

void AeSysView::OnEditModeRotcw() {
	AeSysDoc* Document = GetDocument();
	OdGePoint3d PivotPoint(Document->TrapPivotPoint());

	EoGeMatrix3d TransformMatrix;
	TransformMatrix.setToTranslation(- PivotPoint.asVector());
	EoGeMatrix3d RotationMatrix;
	RotationMatrix.SetTo3AxisRotation(EditModeRotationAngles());
	RotationMatrix.invert();
	TransformMatrix.preMultBy(RotationMatrix);
	EoGeMatrix3d OriginToPivotPointMatrix;
	OriginToPivotPointMatrix.setToTranslation(PivotPoint.asVector());
	TransformMatrix.preMultBy(OriginToPivotPointMatrix);
	Document->TransformTrappedGroups(TransformMatrix);
}
void AeSysView::OnEditModeMove() {
	AeSysDoc* Document = GetDocument();

	const OdGePoint3d pt = GetCursorPosition();
	if (m_PreviousOp != ID_OP4) {
		m_PreviousOp = ModeLineHighlightOp(ID_OP4);
		RubberBandingStartAtEnable(pt, Lines);
	}
	else {
		EoGeMatrix3d tm;
		tm.setToTranslation(pt - Document->TrapPivotPoint());

		ModeLineUnhighlightOp(m_PreviousOp);
		RubberBandingDisable();
		Document->TransformTrappedGroups(tm);
	}
	Document->SetTrapPivotPoint(pt);
	// pSetSegPos(pTRAP_PVT_MRK_ID, pt);
}

void AeSysView::OnEditModeCopy() {
	AeSysDoc* Document = GetDocument();

	const OdGePoint3d pt = GetCursorPosition();
	if (m_PreviousOp != ID_OP5) {
		m_PreviousOp = ModeLineHighlightOp(ID_OP5);
		RubberBandingStartAtEnable(pt, Lines);
	}
	else {
		ModeLineUnhighlightOp(m_PreviousOp);
		RubberBandingDisable();
		Document->CopyTrappedGroups(pt - Document->TrapPivotPoint());
	}
	Document->SetTrapPivotPoint(pt);
	// pSetSegPos(pTRAP_PVT_MRK_ID, pt);
}

void AeSysView::OnEditModeFlip() {
	AeSysDoc* Document = GetDocument();
	OdGePoint3d PivotPoint(Document->TrapPivotPoint());

	EoGeMatrix3d TransformMatrix;
	TransformMatrix.setToTranslation(- PivotPoint.asVector());
	EoGeMatrix3d ScaleMatrix;
	EditModeMirrorScaleFactors().getMatrix(ScaleMatrix);
	TransformMatrix.preMultBy(ScaleMatrix);
	EoGeMatrix3d OriginToPivotPointMatrix;
	OriginToPivotPointMatrix.setToTranslation(PivotPoint.asVector());
	TransformMatrix.preMultBy(OriginToPivotPointMatrix);
	Document->TransformTrappedGroups(TransformMatrix);
}

void AeSysView::OnEditModeReduce() {
	AeSysDoc* Document = GetDocument();
	OdGePoint3d PivotPoint(Document->TrapPivotPoint());

	EoGeMatrix3d TransformMatrix;
	TransformMatrix.setToTranslation(- PivotPoint.asVector());
	EoGeMatrix3d ScaleMatrix;
	EditModeScaleFactors().inverse().getMatrix(ScaleMatrix);
	TransformMatrix.preMultBy(ScaleMatrix);
	EoGeMatrix3d OriginToPivotPointMatrix;
	OriginToPivotPointMatrix.setToTranslation(PivotPoint.asVector());
	TransformMatrix.preMultBy(OriginToPivotPointMatrix);
	Document->TransformTrappedGroups(TransformMatrix);
}

void AeSysView::OnEditModeEnlarge() {
	AeSysDoc* Document = GetDocument();
	OdGePoint3d PivotPoint(Document->TrapPivotPoint());

	EoGeMatrix3d TransformMatrix;
	TransformMatrix.setToTranslation(- PivotPoint.asVector());
	EoGeMatrix3d ScaleMatrix;
	EditModeScaleFactors().getMatrix(ScaleMatrix);
	TransformMatrix.preMultBy(ScaleMatrix);
	EoGeMatrix3d OriginToPivotPointMatrix;
	OriginToPivotPointMatrix.setToTranslation(PivotPoint.asVector());
	TransformMatrix.preMultBy(OriginToPivotPointMatrix);
	Document->TransformTrappedGroups(TransformMatrix);
}

void AeSysView::OnEditModeReturn() {
	// TODO: Add your command handler code here
}

void AeSysView::OnEditModeEscape() {
	if (m_PreviousOp == ID_OP4 || m_PreviousOp == ID_OP5) {
		ModeLineUnhighlightOp(m_PreviousOp);
		RubberBandingDisable();
	}
}
