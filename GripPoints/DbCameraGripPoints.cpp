#include <OdaCommon.h>
#include "DbCameraGripPoints.h"
#include <DbCamera.h>
#include <DbViewTableRecord.h>
#include <AbstractViewPE.h>

OdResult OdDbCameraGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	OdDbCameraPtr Camera {entity};
	auto pView {Camera->openView(OdDb::kForRead)};
	if (!pView.isNull()) {
		OdAbstractViewPEPtr pAvd(pView);
		const auto Target {pAvd->target(pView)};
		const auto Direction {pAvd->direction(pView)};
		gripPoints.resize(GripPointsSize + 3);
		gripPoints[GripPointsSize + 0] = Target + Direction;
		gripPoints[GripPointsSize + 1] = Target + Direction * 0.5;
		gripPoints[GripPointsSize + 2] = Target;
	}
	return eOk;
}

OdResult OdDbCameraGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	const auto IndicesSize {indices.size()};
	if (IndicesSize == 0) {
		return eOk;
	}
	OdDbCameraPtr pCamera = entity;
	if (IndicesSize > 1 || indices[0] == 1) {
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
