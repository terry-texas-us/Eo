#include <OdaCommon.h>
#include "DbCameraGripPoints.h"
#include <DbCamera.h>
#include <DbViewTableRecord.h>
#include <AbstractViewPE.h>

OdResult OdDbCameraGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	OdDbCameraPtr Camera {entity};
	auto View {Camera->openView(OdDb::kForRead)};
	if (!View.isNull()) {
		OdAbstractViewPEPtr AbstractView(View);
		const auto Target {AbstractView->target(View)};
		const auto Direction {AbstractView->direction(View)};
		gripPoints.resize(GripPointsSize + 3);
		gripPoints[GripPointsSize + 0] = Target + Direction;
		gripPoints[GripPointsSize + 1] = Target + Direction * 0.5;
		gripPoints[GripPointsSize + 2] = Target;
	}
	return eOk;
}

OdResult OdDbCameraGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbCameraPtr Camera {entity};
	if (indices.size() > 1 || indices[0] == 1) {
		Camera->transformBy(OdGeMatrix3d::translation(offset));
	} else {
		auto View {Camera->openView(OdDb::kForWrite)};
		if (!View.isNull()) {
			OdAbstractViewPEPtr AbstractView(View);
			auto Target {AbstractView->target(View)};
			const auto Direction {AbstractView->direction(View)};
			auto Position {Target + Direction};
			if (indices[0] == 0) {
				Position += offset;
			} else {
				Target += offset;
			}
			AbstractView->setView(View, Target, Position - Target, AbstractView->upVector(View), AbstractView->fieldWidth(View), AbstractView->fieldHeight(View), AbstractView->isPerspective(View));
			Camera->updateView();
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
