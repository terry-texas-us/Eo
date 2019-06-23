#include "stdafx.h"
#include "AeSys.h"
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

OdGeVector3d AeSysView::EditModeRotationAngles() const noexcept {
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

OdGeScale3d AeSysView::EditModeScaleFactors() const noexcept {
	return m_ScaleFactors;
}

void AeSysView::SetEditModeScaleFactors(const double sx, const double sy, const double sz) noexcept {
	// <tas="Verify scale factors are always not zero"</tas>
	m_ScaleFactors.sx = sx;
	m_ScaleFactors.sy = sy;
	m_ScaleFactors.sz = sz;
}

void AeSysView::SetEditModeRotationAngles(const double x, const double y, const double z) noexcept {
	m_EditModeRotationAngles.x = x;
	m_EditModeRotationAngles.y = y;
	m_EditModeRotationAngles.z = z;
}

OdGeScale3d AeSysView::EditModeMirrorScaleFactors() const noexcept {
	return m_MirrorScaleFactors;
}

void AeSysView::SetEditModeMirrorScaleFactors(const double sx, const double sy, const double sz) noexcept {
	m_MirrorScaleFactors.set(sx, sy, sz);
}

void AeSysView::OnEditModePivot() {
	const auto CurrentPnt {GetCursorPosition()};
	GetDocument()->SetTrapPivotPoint(CurrentPnt);
}

void AeSysView::OnEditModeRotccw() {
	auto PivotPoint {GetDocument()->TrapPivotPoint()};
	EoGeMatrix3d TransformMatrix;
	TransformMatrix.setToTranslation(- PivotPoint.asVector());
	EoGeMatrix3d RotationMatrix;
	RotationMatrix.SetTo3AxisRotation(EditModeRotationAngles());
	TransformMatrix.preMultBy(RotationMatrix);
	EoGeMatrix3d OriginToPivotPointMatrix;
	OriginToPivotPointMatrix.setToTranslation(PivotPoint.asVector());
	TransformMatrix.preMultBy(OriginToPivotPointMatrix);
	GetDocument()->TransformTrappedGroups(TransformMatrix);
}

void AeSysView::OnEditModeRotcw() {
	auto PivotPoint {GetDocument()->TrapPivotPoint()};
	EoGeMatrix3d TransformMatrix;
	TransformMatrix.setToTranslation(- PivotPoint.asVector());
	EoGeMatrix3d RotationMatrix;
	RotationMatrix.SetTo3AxisRotation(EditModeRotationAngles());
	RotationMatrix.invert();
	TransformMatrix.preMultBy(RotationMatrix);
	EoGeMatrix3d OriginToPivotPointMatrix;
	OriginToPivotPointMatrix.setToTranslation(PivotPoint.asVector());
	TransformMatrix.preMultBy(OriginToPivotPointMatrix);
	GetDocument()->TransformTrappedGroups(TransformMatrix);
}

void AeSysView::OnEditModeMove() {
	auto Document {GetDocument()};
	const auto CurrentPnt {GetCursorPosition()};
	if (m_PreviousOp != ID_OP4) {
		m_PreviousOp = ModeLineHighlightOp(ID_OP4);
		RubberBandingStartAtEnable(CurrentPnt, Lines);
	} else {
		EoGeMatrix3d tm;
		tm.setToTranslation(CurrentPnt - Document->TrapPivotPoint());
		ModeLineUnhighlightOp(m_PreviousOp);
		RubberBandingDisable();
		Document->TransformTrappedGroups(tm);
	}
	Document->SetTrapPivotPoint(CurrentPnt);
}

void AeSysView::OnEditModeCopy() {
	auto Document {GetDocument()};
	const auto CurrentPnt {GetCursorPosition()};
	if (m_PreviousOp != ID_OP5) {
		m_PreviousOp = ModeLineHighlightOp(ID_OP5);
		RubberBandingStartAtEnable(CurrentPnt, Lines);
	} else {
		ModeLineUnhighlightOp(m_PreviousOp);
		RubberBandingDisable();
		Document->CopyTrappedGroups(CurrentPnt - Document->TrapPivotPoint());
	}
	Document->SetTrapPivotPoint(CurrentPnt);
}

void AeSysView::OnEditModeFlip() {
	auto PivotPoint {GetDocument()->TrapPivotPoint()};
	EoGeMatrix3d TransformMatrix;
	TransformMatrix.setToTranslation(- PivotPoint.asVector());
	EoGeMatrix3d ScaleMatrix;
	EditModeMirrorScaleFactors().getMatrix(ScaleMatrix);
	TransformMatrix.preMultBy(ScaleMatrix);
	EoGeMatrix3d OriginToPivotPointMatrix;
	OriginToPivotPointMatrix.setToTranslation(PivotPoint.asVector());
	TransformMatrix.preMultBy(OriginToPivotPointMatrix);
	GetDocument()->TransformTrappedGroups(TransformMatrix);
}

void AeSysView::OnEditModeReduce() {
	auto PivotPoint {GetDocument()->TrapPivotPoint()};
	EoGeMatrix3d TransformMatrix;
	TransformMatrix.setToTranslation(- PivotPoint.asVector());
	EoGeMatrix3d ScaleMatrix;
	EditModeScaleFactors().inverse().getMatrix(ScaleMatrix);
	TransformMatrix.preMultBy(ScaleMatrix);
	EoGeMatrix3d OriginToPivotPointMatrix;
	OriginToPivotPointMatrix.setToTranslation(PivotPoint.asVector());
	TransformMatrix.preMultBy(OriginToPivotPointMatrix);
	GetDocument()->TransformTrappedGroups(TransformMatrix);
}

void AeSysView::OnEditModeEnlarge() {
	auto PivotPoint {GetDocument()->TrapPivotPoint()};
	EoGeMatrix3d TransformMatrix;
	TransformMatrix.setToTranslation(- PivotPoint.asVector());
	EoGeMatrix3d ScaleMatrix;
	EditModeScaleFactors().getMatrix(ScaleMatrix);
	TransformMatrix.preMultBy(ScaleMatrix);
	EoGeMatrix3d OriginToPivotPointMatrix;
	OriginToPivotPointMatrix.setToTranslation(PivotPoint.asVector());
	TransformMatrix.preMultBy(OriginToPivotPointMatrix);
	GetDocument()->TransformTrappedGroups(TransformMatrix);
}

void AeSysView::OnEditModeReturn() noexcept {
	// TODO: Add your command handler code here
}

void AeSysView::OnEditModeEscape() {
	if (m_PreviousOp == ID_OP4 || m_PreviousOp == ID_OP5) {
		ModeLineUnhighlightOp(m_PreviousOp);
		RubberBandingDisable();
	}
}
