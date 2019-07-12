#include "stdafx.h"
#include <Gi/GiDrawableImpl.h>
#include <Gs/Gs.h>
#include <Gs/GsBaseVectorizer.h>
#include <GiContextForDbDatabase.h>
#include <DbCommandContext.h>
#include <DbAbstractViewportData.h>
#include <DbBlockTableRecord.h>
#include <DbViewportTable.h>
#include <DbHostAppServices.h>
#include <ExTrackers.h>
#include <Gs/GsModel.h>
#include "EoRtOrbitTracker.h"

void RtOrbitTracker::ViewportDcCorners(OdGePoint2d& lowerLeft, OdGePoint2d& upperRight) const {
	const auto Target {m_View->viewingMatrix() * m_View->target()};
	const auto HalfFieldWidth {m_View->fieldWidth() / 2.0};
	const auto HalfFieldHeight {m_View->fieldHeight() / 2.0};
	lowerLeft.x = Target.x - HalfFieldWidth;
	lowerLeft.y = Target.y - HalfFieldHeight;
	upperRight.x = Target.x + HalfFieldWidth;
	upperRight.y = Target.y + HalfFieldHeight;
}

void RtOrbitTracker::Initialize(OdGsView* view, const OdGePoint3d& pt) {
	m_View = view;
	m_Position = view->position();
	m_Target = view->target();
	m_UpVector = view->upVector();
	m_X = m_UpVector.crossProduct(view->target() - m_Position).normal();
	m_InitialViewingMatrixInverted = m_View->viewingMatrix();
	m_Point = m_InitialViewingMatrixInverted * pt;
	m_Point.z = 0.0;
	m_InitialViewingMatrixInverted.invert();
	OdGePoint3d LowerLeftPoint;
	OdGePoint2d UpperRightPoint;
	ViewportDcCorners(reinterpret_cast<OdGePoint2d&>(LowerLeftPoint), UpperRightPoint);
	UpperRightPoint.x -= LowerLeftPoint.x;
	UpperRightPoint.y -= LowerLeftPoint.y;
	const auto Radius {odmin(UpperRightPoint.x, UpperRightPoint.y) / 9.0 * 7.0 / 2.0};
	m_D = 2.0 * Radius;
	reinterpret_cast<OdGePoint2d&>(LowerLeftPoint) += UpperRightPoint.asVector() / 2.0;
	const auto r2Squared {Radius * Radius / 400.0};
	LowerLeftPoint.y += Radius;
	if ((LowerLeftPoint - m_Point).lengthSqrd() <= r2Squared) {
		m_Axis = kHorizontal;
	} else {
		LowerLeftPoint.y -= Radius;
		LowerLeftPoint.y -= Radius;
		if ((LowerLeftPoint - m_Point).lengthSqrd() <= r2Squared) {
			m_Axis = kHorizontal;
		} else {
			LowerLeftPoint.y += Radius;
			LowerLeftPoint.x += Radius;
			if ((LowerLeftPoint - m_Point).lengthSqrd() <= r2Squared) {
				m_Axis = kVertical;
			} else {
				LowerLeftPoint.x -= Radius;
				LowerLeftPoint.x -= Radius;
				if ((LowerLeftPoint - m_Point).lengthSqrd() <= r2Squared) {
					m_Axis = kVertical;
				} else {
					LowerLeftPoint.x += Radius;
					if ((LowerLeftPoint - m_Point).lengthSqrd() <= Radius * Radius) {
						m_Axis = kPerpDir;
					} else {
						m_Axis = kEye;
					}
				}
			}
		}
	}
	auto ComputeExtents {true};
	{ // Try to extract cached extents
		OdGsClientViewInfo ClientViewInfo;
		view->clientViewInfo(ClientViewInfo);
		OdDbObjectId SpaceId;
		if (!((ClientViewInfo.viewportFlags & OdGsClientViewInfo::kDependentGeometry) != 0)) {
			SpaceId = OdDbDatabasePtr(view->userGiContext()->database())->getModelSpaceId();
		} else {
			SpaceId = OdDbDatabasePtr(view->userGiContext()->database())->getPaperSpaceId();
		}
		auto BlockTableRecord {SpaceId.openObject()};
		OdGeExtents3d ExtentsWcs;
		if (BlockTableRecord->gsNode() != nullptr && BlockTableRecord->gsNode()->extents(ExtentsWcs)) {
			m_ViewCenter = ExtentsWcs.center(), ComputeExtents = false;
		}
	}
	if (ComputeExtents) { // Compute extents if no extents cached
		OdAbstractViewPEPtr AbstractView {view};
		OdGeBoundBlock3d BoundBox;
		AbstractView->viewExtents(view, BoundBox);
		m_ViewCenter = BoundBox.center();
		m_ViewCenter.transformBy(m_InitialViewingMatrixInverted);
	}
}

double RtOrbitTracker::Angle(const OdGePoint3d& value) const {
	const auto Point {m_View->viewingMatrix() * value};
	auto Distance {0.0};
	if (m_Axis == kHorizontal) {
		Distance = Point.y - m_Point.y;
	} else if (m_Axis == kVertical) {
		Distance = Point.x - m_Point.x;
	}
	return Distance * OdaPI / m_D;
}

double RtOrbitTracker::AngleZ(const OdGePoint3d& value) const {
	auto Point {m_View->viewingMatrix() * value};
	auto Target {m_View->viewingMatrix() * m_ViewCenter};
	Point.z = Target.z = m_Point.z;
	return (Point - Target).angleTo(m_Point - Target, OdGeVector3d::kZAxis);
}

double RtOrbitTracker::AnglePerpendicular(const OdGePoint3d& value) const {
	auto Point {m_View->viewingMatrix() * value};
	Point.z = 0.0;
	return Point.distanceTo(m_Point) * OdaPI / m_D;
}

void RtOrbitTracker::setValue(const OdGePoint3d& value) {
	if (m_View != nullptr) {
		OdGeMatrix3d Transform;
		switch (m_Axis) {
			case kHorizontal:
				Transform.setToRotation(-Angle(value), m_X, m_ViewCenter);
				break;
			case kVertical:
				Transform.setToRotation(-Angle(value), m_UpVector, m_ViewCenter);
				break;
			case kEye:
				Transform.setToRotation(-AngleZ(value), m_Target - m_Position, m_ViewCenter);
				break;
			case kPerpDir: {
				auto TransformedValue {value};
				TransformedValue.transformBy(m_View->viewingMatrix());
				TransformedValue.z = 0.0;
				const auto Direction {(TransformedValue - m_Point).convert2d()};
				const auto Perpendicular {Direction.perpVector()};
				OdGeVector3d Perp3d(Perpendicular.x, Perpendicular.y, 0.0);
				Perp3d.normalizeGetLength();
				Perp3d.transformBy(m_InitialViewingMatrixInverted);
				Transform.setToRotation(-AnglePerpendicular(value), Perp3d, m_ViewCenter);
				break;
			}
		}
		auto NewPosition {Transform * m_Position};
		const auto NewTarget {Transform * m_Target};
		auto NewPositionDirection {NewPosition - NewTarget};
		NewPositionDirection.normalizeGetLength();
		NewPositionDirection *= m_Position.distanceTo(m_Target);
		NewPosition = NewTarget + NewPositionDirection;
		m_View->setView(NewPosition, NewTarget, Transform * m_UpVector, m_View->fieldWidth(), m_View->fieldHeight(), m_View->isPerspective() ? OdGsView::kPerspective : OdGsView::kParallel);
	}
}

int RtOrbitTracker::addDrawables(OdGsView* pView) {
	m_Drawable = OdRxObjectImpl<OrbitCtrl>::createObject();
	if (m_Model.isNull()) {
		m_Model = pView->device()->createModel();
		if (!m_Model.isNull()) {
			m_Model->setRenderType(OdGsModel::kDirect); // Skip Z-buffer for 2d drawables.
			m_Model->setEnableViewExtentsCalculation(false); // Skip extents calculation.
			m_Model->setRenderModeOverride(OdGsView::k2DOptimized); // Setup 2dWireframe mode for all underlying geometry.
			const auto VisualStyleId {GraphTrackerBase::getVisualStyleOverride(pView->userGiContext()->database())};
			if (VisualStyleId != nullptr) {
				m_Model->setVisualStyle(VisualStyleId);
			} // 2dWireframe visual style.
		}
	}
	pView->add(m_Drawable, m_Model.get());
	return 1;
}

void RtOrbitTracker::removeDrawables(OdGsView* pView) {
	pView->erase(m_Drawable);
}
