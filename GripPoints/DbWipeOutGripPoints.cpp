#include <OdaCommon.h>
#include "DbWipeOutGripPoints.h"
#include <Ge/GeMatrix3d.h>
#include <Ge/GeScale3d.h>
#include <Ge/GeCircArc3d.h>
#include <DbWipeout.h>

OdResult OdDbWipeOutGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdGePoint3dArray odGePoint3dArray;
	OdDbWipeoutPtr(entity)->getVertices(odGePoint3dArray);
	// for the closed polyline boundary last coincident point is not returned as a grip
	if (odGePoint3dArray.size() > 2 && odGePoint3dArray.last().isEqualTo(odGePoint3dArray.first())) {
		odGePoint3dArray.removeLast();
	}
	gripPoints.append(odGePoint3dArray);
	return eOk;
}

OdResult OdDbWipeOutGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	const auto IndicesSize {indices.size()};
	if (IndicesSize == 0) {
		return eOk;
	}
	OdDbWipeoutPtr Wipeout {entity};
	OdGePoint3dArray Points;
	Wipeout->getVertices(Points);
	// for the closed polyline boundary last coincident point is not returned as a grip
	const auto HasClosedPolylineBoundary {Points.size() > 2 && Points.last().isEqualTo(Points.first())};
	for (unsigned i = 0; i < IndicesSize; ++i) {
		if (indices[i] < static_cast<int>(Points.length())) {
			auto pt3D {Points.getAt(indices[i])};
			pt3D += offset;
			Points.setAt(indices[i], pt3D);
		}
	}
	if (HasClosedPolylineBoundary) {
		Points[Points.length() - 1] = Points.first();
	}
	Wipeout->setBoundary(Points);
	return eOk;
}

OdResult OdDbWipeOutGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDbWipeOutGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDbWipeOutGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
	OdDbWipeoutPtr Wipeout {entity};
	OdGePoint3dArray odGePoint3dArray;
	Wipeout->getVertices(odGePoint3dArray);
	const auto SnapPointsSize {snapPoints.size()};
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd:
			getStretchPoints(entity, snapPoints);
			break;
		case OdDb::kOsModeMid: {
			snapPoints.resize(SnapPointsSize + odGePoint3dArray.length() - 1);
			for (unsigned i = 1; i < odGePoint3dArray.length(); ++i) {
				OdGeLine3d l(odGePoint3dArray.getAt(i - 1), odGePoint3dArray.getAt(i));
				snapPoints.append(l.evalPoint(l.paramOf(lastPoint)));
			}
			break;
		}
		case OdDb::kOsModeQuad: // not implemented yet
			break;
		case OdDb::kOsModeNear: {
			snapPoints.resize(SnapPointsSize + odGePoint3dArray.length() - 1);
			for (unsigned i = 1; i < odGePoint3dArray.length(); ++i) {
				auto Start {odGePoint3dArray.getAt(i - 1)};
				auto End {odGePoint3dArray.getAt(i)};
				if (!Start.isEqualTo(End)) {
					OdGeLine3d l(Start, End);
					const auto p {l.paramOf(pickPoint)};
					if (p > 1) {
						snapPoints.append(End);
					} else if (p < 0) {
						snapPoints.append(Start);
					} else {
						snapPoints.append(l.evalPoint(p));
					}
				} else {
					snapPoints.append(Start);
				}
			}
			break;
		}
		case OdDb::kOsModeCen:
			break;
		case OdDb::kOsModeNode:
			break;
		case OdDb::kOsModeIntersec:
			break;
		case OdDb::kOsModeIns:
			break;
		case OdDb::kOsModePerp:
			break;
		case OdDb::kOsModeTan:
			break;
		case OdDb::kOsModeApint:
			break;
		case OdDb::kOsModePar:
			break;
		case OdDb::kOsModeStart:
			break;
		default: ;
	}
	return eOk;
}
