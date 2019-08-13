#include <OdaCommon.h>
#include <DbTrace.h>
#include "DbTraceGripPoints.h"

OdResult OdDbTraceGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbTracePtr Trace {entity};
	gripPoints.resize(4);
	for (auto PointIndex = 0; PointIndex < 4; PointIndex++) {
		Trace->getPointAt(PointIndex, gripPoints[PointIndex]);
	}
	if (OdNonZero(Trace->thickness())) {
		const auto ThicknessPath(Trace->normal().normal() * Trace->thickness());
		gripPoints.resize(8);
		for (auto PointIndex = 4; PointIndex < 8; PointIndex++) {
			gripPoints[PointIndex] = gripPoints[PointIndex - 4] + ThicknessPath;
		}
	}
	return eOk;
}

OdResult OdDbTraceGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	OdDbTracePtr Trace {entity};
	OdGeVector3d PlanarBasis[3];
	double PlanarCoefficients[3];
	int PointIndex;
	PlanarBasis[2] = Trace->normal();
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
				Trace->getPointAt(PointIndex, MovePoint);
				MovePoint += Delta;
				Trace->setPointAt(PointIndex, MovePoint);
			}
		}
	}
	if (OdNonZero(PlanarCoefficients[2])) {
		const auto Delta {PlanarBasis[2] * PlanarCoefficients[2]};
		for (PointIndex = 0; PointIndex < 4; PointIndex++) {
			OdGePoint3d MovePoint;
			Trace->getPointAt(PointIndex, MovePoint);
			MovePoint += Delta;
			Trace->setPointAt(PointIndex, MovePoint);
		}
	}
	return eOk;
}

OdResult OdDbTraceGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	OdDbTracePtr Trace {entity};
	return getGripPoints(Trace, stretchPoints);
}

OdResult OdDbTraceGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	OdDbTracePtr Trace {entity};
	return moveGripPointsAt(Trace, indices, offset);
}

OdResult OdDbTraceGripPointsPE::getOsnapPoints(const OdDbEntity* entity, const OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
	OdDbTracePtr Trace {entity};
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd: {
			const auto SnapPointsSize {snapPoints.size()};
			snapPoints.resize(SnapPointsSize + 4);
			for (auto PointIndex = 0; PointIndex < 4; PointIndex++) {
				Trace->getPointAt(PointIndex, snapPoints[SnapPointsSize + PointIndex]);
			}
			break;
		}
		default: ;
	}
	return eOk;
}
