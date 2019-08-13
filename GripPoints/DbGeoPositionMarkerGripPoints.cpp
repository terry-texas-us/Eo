#include <OdaCommon.h>
#include "DbGeoPositionMarkerGripPoints.h"
#include <DbGeoPositionMarker.h>
#include <Ge/GeCircArc3d.h>
#include <DbViewTableRecord.h>
#include <cfloat>

OdResult OdDbGeoPositionMarkerPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbGeoPositionMarkerPtr GeoPositionMarker {entity};
	gripPoints.reserve(2);
	gripPoints.append(GeoPositionMarker->position());
	auto pMText {GeoPositionMarker->mtext()};
	if (!pMText.isNull()) {
		OdGePoint3d closestPoint;
		OdGePoint3dArray framePoints;
		pMText->getActualBoundingPoints(framePoints, GeoPositionMarker->landingGap(), GeoPositionMarker->landingGap());
		framePoints.append(framePoints[0]);
		framePoints.swap(2, 3);
		auto module {DBL_MAX};
		for (auto i = 0; i < 4; ++i) {
			auto pntMiddle {(framePoints[i] + framePoints[i + 1].asVector()) / 2.0};
			if (module >= GeoPositionMarker->position().distanceTo(pntMiddle)) {
				closestPoint = pntMiddle;
				module = GeoPositionMarker->position().distanceTo(pntMiddle);
			}
		}
		gripPoints.append(closestPoint);
	}
	return eOk;
}

OdResult OdDbGeoPositionMarkerPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	OdDbGeoPositionMarkerPtr GeoPositionMarker {entity};
	for (auto Index : indices) {
		switch (Index) {
			case 0: {
				auto pMTextOld {GeoPositionMarker->mtext()};
				GeoPositionMarker->setPosition(GeoPositionMarker->position() + offset);
				GeoPositionMarker->setMText(pMTextOld);
				break;
			}
			case 1: {
				auto pMText {GeoPositionMarker->mtext()};
				pMText->setLocation(pMText->location() + offset);
				GeoPositionMarker->setMText(pMText);
				break;
			}
			default:
				return eNotImplementedYet;
		}
	}
	return eOk;
}

OdResult OdDbGeoPositionMarkerPE::getStretchPoints(const OdDbEntity* /*entity*/, OdGePoint3dArray& /*stretchPoints*/) const {
	return eNotImplementedYet;
}

OdResult OdDbGeoPositionMarkerPE::moveStretchPointsAt(OdDbEntity* /*entity*/, const OdIntArray& /*indices*/, const OdGeVector3d& /*offset*/) {
	return eOk;
}

OdResult OdDbGeoPositionMarkerPE::getOsnapPoints(const OdDbEntity* entity, const OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
	OdDbGeoPositionMarkerPtr GeoPositionMarker {entity};
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd:
			if (!GeoPositionMarker->mtext().isNull()) {
				snapPoints.reserve(2);
				OdGePoint3dArray gripPoints;
				GeoPositionMarker->getGripPoints(gripPoints);
				snapPoints.append(gripPoints[1]);
				const OdGeCircArc3d CircularArc(GeoPositionMarker->position(), GeoPositionMarker->normal(), GeoPositionMarker->radius());
				snapPoints.append(CircularArc.closestPointTo(gripPoints[1]));
			}
			break;
		case OdDb::kOsModeMid:
			break;
		case OdDb::kOsModeCen:
			break;
		case OdDb::kOsModeNode: {
			if (!GeoPositionMarker->mtext().isNull()) {
				snapPoints.reserve(5);
				GeoPositionMarker->mtext()->getBoundingPoints(snapPoints);
			}
			snapPoints.append(GeoPositionMarker->position());
		}
		break;
		case OdDb::kOsModeQuad: case OdDb::kOsModeIntersec: case OdDb::kOsModeIns: case OdDb::kOsModePerp: case OdDb::kOsModeTan: case OdDb::kOsModeNear: case OdDb::kOsModeApint: case OdDb::kOsModePar: case OdDb::kOsModeStart: default:
			break;
	}
	return eOk;
}
