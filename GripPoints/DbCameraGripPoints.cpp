#include <OdaCommon.h>
#include "DbCameraGripPoints.h"
#include <DbCamera.h>
#include <DbViewTableRecord.h>
#include <AbstractViewPE.h>

OdResult OdDbCameraGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto size {gripPoints.size()};
	OdDbCameraPtr pCamera = entity;
	auto pView {pCamera->openView(OdDb::kForRead)};
	if (!pView.isNull()) {
		OdAbstractViewPEPtr pAvd(pView);
		const auto target {pAvd->target(pView)};
		const auto dir {pAvd->direction(pView)};
		gripPoints.resize(size + 3);
		gripPoints[size + 0] = target + dir;
		gripPoints[size + 1] = target + dir * 0.5;
		gripPoints[size + 2] = target;
	}
	return eOk;
}

OdResult OdDbCameraGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	const auto size {indices.size()};
	if (size == 0) {
		return eOk;
	}
	OdDbCameraPtr pCamera = entity;
	if (size > 1 || indices[0] == 1) {
		pCamera->transformBy(OdGeMatrix3d::translation(offset));
	} else {
		auto pView {pCamera->openView(OdDb::kForWrite)};
		if (!pView.isNull()) {
			OdAbstractViewPEPtr pAvd(pView);
			auto target {pAvd->target(pView)};
			const auto dir {pAvd->direction(pView)};
			auto position {target + dir};
			if (indices[0] == 0) {
				position += offset;
			} else {
				target += offset;
			}
			pAvd->setView(pView, target, position - target, pAvd->upVector(pView), pAvd->fieldWidth(pView), pAvd->fieldHeight(pView), pAvd->isPerspective(pView));
			pCamera->updateView();
		}
	}
	return eOk;
}

OdResult OdDbCameraGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDbCameraGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDbCameraGripPointsPE::getOsnapPoints(const OdDbEntity* /*entity*/, OdDb::OsnapMode /*objectSnapMode*/, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& /*snapPoints*/) const {
	return eOk;
}
