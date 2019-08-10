#include <OdaCommon.h>
#include <DbSolid.h>
#include "DbSolidGripPoints.h"

OdResult OdDbSolidGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbSolidPtr Solid {entity};
	gripPoints.resize(4);
	for (auto PointIndex = 0; PointIndex < 4; PointIndex++) {
		Solid->getPointAt(PointIndex, gripPoints[PointIndex]);
	}
	if (OdNonZero(Solid->thickness())) {
		const auto ThicknessPath(Solid->normal().normal() * Solid->thickness());
		gripPoints.resize(8);
		for (auto PointIndex = 4; PointIndex < 8; PointIndex++) {
			gripPoints[PointIndex] = gripPoints[PointIndex - 4] + ThicknessPath;
		}
	}
	return eOk;
}

OdResult OdDbSolidGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	OdDbSolidPtr Solid {entity};
	OdGeVector3d PlanarBasis[3];
	double PlanarCoefficients[3];
	int PointIndex;
	PlanarBasis[2] = Solid->normal();
	PlanarBasis[0] = PlanarBasis[2].perpVector().normal();
	PlanarBasis[1] = PlanarBasis[2].crossProduct(PlanarBasis[0]).normal();
	for (PointIndex = 0; PointIndex < 3; PointIndex++) {
		PlanarCoefficients[PointIndex] = PlanarBasis[PointIndex].dotProduct(offset);
	}
	if (OdNonZero(PlanarCoefficients[0]) || OdNonZero(PlanarCoefficients[1])) {
		bool MarkIds[4] = {false, false, false, false};
		const auto Delta {PlanarBasis[0] * PlanarCoefficients[0] + PlanarBasis[1] * PlanarCoefficients[1]};
		for (PointIndex = 0; PointIndex < static_cast<int>(indices.size()); PointIndex++) {
			MarkIds[indices[PointIndex] % 4] = true;
		}
		for (PointIndex = 0; PointIndex < 4; PointIndex++) {
			if (MarkIds[PointIndex]) {
				OdGePoint3d MovePoint;
				Solid->getPointAt(PointIndex, MovePoint);
				MovePoint += Delta;
				Solid->setPointAt(PointIndex, MovePoint);
			}
		}
	}
	if (OdNonZero(PlanarCoefficients[2])) {
		const auto Delta {PlanarBasis[2] * PlanarCoefficients[2]};
		for (PointIndex = 0; PointIndex < 4; PointIndex++) {
			OdGePoint3d MovePoint;
			Solid->getPointAt(PointIndex, MovePoint);
			MovePoint += Delta;
			Solid->setPointAt(PointIndex, MovePoint);
		}
	}
	return eOk;
}

OdResult OdDbSolidGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	OdDbSolidPtr Solid {entity};
	return getGripPoints(Solid, stretchPoints);
}

OdResult OdDbSolidGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	OdDbSolidPtr Solid {entity};
	return moveGripPointsAt(Solid, indices, offset);
}

OdResult OdDbSolidGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
	OdDbSolidPtr Solid {entity};
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd: {
			const auto SnapPointsSize {snapPoints.size()};
			snapPoints.resize(SnapPointsSize + 4);
			for (auto PointIndex = 0; PointIndex < 4; PointIndex++) {
				Solid->getPointAt(PointIndex, snapPoints[SnapPointsSize + PointIndex]);
			}
			break;
		}
		default: ;
	}
	return eOk;
}
