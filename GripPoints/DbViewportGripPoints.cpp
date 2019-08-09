#include <OdaCommon.h>
#include <DbViewport.h>
#include "DbViewportGripPoints.h"
#include <Gs/GsDefs.h>
#include <AbstractViewPE.h>

OdResult OdDbViewportGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbViewportPtr pVpt(entity);
	if (!pVpt->isNonRectClipOn()) {
		const auto centerPt {pVpt->centerPoint()};
		const auto widthD {pVpt->width()};
		const auto heightD {pVpt->height()};
		const auto xAxis {OdGeVector3d::kXAxis * (widthD * 0.5)};
		const auto yAxis {OdGeVector3d::kYAxis * (heightD * 0.5)};
		gripPoints.reserve(4);
		gripPoints.append(centerPt - xAxis - yAxis);
		gripPoints.append(centerPt + xAxis - yAxis);
		gripPoints.append(centerPt + xAxis + yAxis);
		gripPoints.append(centerPt - xAxis + yAxis);
	}
	return eOk;
}

OdResult OdDbViewportGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	OdDbViewportPtr Viewport {entity};
	OdGsDCRectDouble rect;
	const auto CenterPoint {Viewport->centerPoint()};
	auto widthD {Viewport->width() * 0.5};
	auto heightD {Viewport->height() * 0.5};
	rect.m_min.x = CenterPoint.x - widthD;
	rect.m_min.y = CenterPoint.y - heightD;
	rect.m_max.x = CenterPoint.x + widthD;
	rect.m_max.y = CenterPoint.y + heightD;
	double* logX[4] = {
		&rect.m_min.x,
		&rect.m_max.x,
		&rect.m_max.x,
		&rect.m_min.x
	};
	double* logY[4] = {
		&rect.m_min.y,
		&rect.m_min.y,
		&rect.m_max.y,
		&rect.m_max.y
	};
	bool bMovement[4] = {false, false, false, false};
	bool* bMovX[4] = {bMovement + 0, bMovement + 2, bMovement + 2, bMovement + 0};
	bool* bMovY[4] = {bMovement + 1, bMovement + 1, bMovement + 3, bMovement + 3};
	for (auto Index : indices) {
		if (!(*bMovX[Index])) {
			*logX[Index] += offset.x;
			*bMovX[Index] = true;
		}
		if (!(*bMovY[Index])) {
			*logY[Index] += offset.y;
			*bMovY[Index] = true;
		}
	}
	if (rect.m_min.x > rect.m_max.x) {
		const auto tmp {rect.m_min.x};
		rect.m_min.x = rect.m_max.x;
		rect.m_max.x = tmp;
	}
	if (rect.m_min.y > rect.m_max.y) {
		const auto tmp {rect.m_min.y};
		rect.m_min.y = rect.m_max.y;
		rect.m_max.y = tmp;
	}
	const auto newWidth {rect.m_max.x - rect.m_min.x};
	const auto newHeight {rect.m_max.y - rect.m_min.y};
	const OdGePoint3d newCenter(rect.m_min.x + newWidth * 0.5, rect.m_min.y + newHeight * 0.5, CenterPoint.z);
	if (OdNonZero(newWidth) && OdNonZero(newHeight) && !Viewport->isPerspectiveOn()) {
		widthD *= 2;
		heightD *= 2;
		OdAbstractViewPEPtr pViewPE(Viewport);
		const auto zAxis {pViewPE->direction(Viewport)};
		const auto yAxis {pViewPE->upVector(Viewport)};
		const auto xAxis {yAxis.crossProduct(zAxis).normal()};
		const auto fieldHeight {pViewPE->fieldHeight(Viewport) / heightD * newHeight};
		const auto fieldWidth {fieldHeight / heightD * widthD};
		const auto diff {(newCenter - CenterPoint) / newHeight * fieldHeight};
		pViewPE->setView(Viewport, pViewPE->target(Viewport) + xAxis * diff.x + yAxis * diff.y, zAxis, yAxis, fieldWidth, fieldHeight, false);
	}
	Viewport->setCenterPoint(newCenter);
	Viewport->setWidth(newWidth);
	Viewport->setHeight(newHeight);
	return eOk;
}

OdResult OdDbViewportGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDbViewportGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDbViewportGripPointsPE::getOsnapPoints(const OdDbEntity* /*entity*/, OdDb::OsnapMode /*objectSnapMode*/, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& /*snapPoints*/) const {
	return eOk;
}
