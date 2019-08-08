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
	OdDbViewportPtr pVpt(entity);
	OdGsDCRectDouble rect;
	const auto centerPt {pVpt->centerPoint()};
	auto widthD {pVpt->width() * 0.5};
	auto heightD {pVpt->height() * 0.5};
	rect.m_min.x = centerPt.x - widthD;
	rect.m_min.y = centerPt.y - heightD;
	rect.m_max.x = centerPt.x + widthD;
	rect.m_max.y = centerPt.y + heightD;
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
	for (OdUInt32 i = 0; i < indices.size(); i++) {
		const auto id {indices[i]};
		if (!(*bMovX[id])) {
			*logX[id] += offset.x;
			*bMovX[id] = true;
		}
		if (!(*bMovY[id])) {
			*logY[id] += offset.y;
			*bMovY[id] = true;
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
	const OdGePoint3d newCenter(rect.m_min.x + newWidth * 0.5, rect.m_min.y + newHeight * 0.5, centerPt.z);
	if (OdNonZero(newWidth) && OdNonZero(newHeight) && !pVpt->isPerspectiveOn()) {
		widthD *= 2;
		heightD *= 2;
		OdAbstractViewPEPtr pViewPE(pVpt);
		const auto zAxis {pViewPE->direction(pVpt)};
		const auto yAxis {pViewPE->upVector(pVpt)};
		const auto xAxis {yAxis.crossProduct(zAxis).normal()};
		const auto fieldHeight {pViewPE->fieldHeight(pVpt) / heightD * newHeight};
		const auto fieldWidth {fieldHeight / heightD * widthD};
		const auto diff {(newCenter - centerPt) / newHeight * fieldHeight};
		pViewPE->setView(pVpt, pViewPE->target(pVpt) + xAxis * diff.x + yAxis * diff.y, zAxis, yAxis, fieldWidth, fieldHeight, false);
	}
	pVpt->setCenterPoint(newCenter);
	pVpt->setWidth(newWidth);
	pVpt->setHeight(newHeight);
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
