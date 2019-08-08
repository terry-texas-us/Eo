#include <OdaCommon.h>
#include "DbGeoPositionMarkerGripPoints.h"
#include <DbGeoPositionMarker.h>
#include <Ge/GeCircArc3d.h>
#include <DbViewTableRecord.h>
#include <cfloat>

OdResult OdDbGeoPositionMarkerPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbGeoPositionMarkerPtr pMarker = entity;
	gripPoints.reserve(2);
	gripPoints.append(pMarker->position());
	auto pMText {pMarker->mtext()};
	if (!pMText.isNull()) {
		OdGePoint3d closestPoint;
		OdGePoint3dArray framePoints;
		pMText->getActualBoundingPoints(framePoints, pMarker->landingGap(), pMarker->landingGap());
		framePoints.append(framePoints[0]);
		framePoints.swap(2, 3);
		auto module {DBL_MAX};
		for (auto i = 0; i < 4; ++i) {
			auto pntMiddle {(framePoints[i] + framePoints[i + 1].asVector()) / 2.0};
			if (module >= pMarker->position().distanceTo(pntMiddle)) {
				closestPoint = pntMiddle;
				module = pMarker->position().distanceTo(pntMiddle);
			}
		}
		gripPoints.append(closestPoint);
	}
	return eOk;
}

OdResult OdDbGeoPositionMarkerPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	OdDbGeoPositionMarkerPtr pMarker = entity;
	for (unsigned i = 0; i < indices.length(); ++i) {
		switch (indices[i]) {
			case 0: {
				auto pMTextOld {pMarker->mtext()};
				pMarker->setPosition(pMarker->position() + offset);
				pMarker->setMText(pMTextOld);
				break;
			}
			case 1: {
				auto pMText {pMarker->mtext()};
				pMText->setLocation(pMText->location() + offset);
				pMarker->setMText(pMText);
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

OdResult OdDbGeoPositionMarkerPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
	OdDbGeoPositionMarkerPtr pMarker = entity;
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd:
			if (!pMarker->mtext().isNull()) {
				snapPoints.reserve(2);
				OdGePoint3dArray gripPoints;
				pMarker->getGripPoints(gripPoints);
				snapPoints.append(gripPoints[1]);
				const OdGeCircArc3d CircularArc(pMarker->position(), pMarker->normal(), pMarker->radius());
				snapPoints.append(CircularArc.closestPointTo(gripPoints[1]));
			}
			break;
		case OdDb::kOsModeMid:
			break;
		case OdDb::kOsModeCen:
			break;
		case OdDb::kOsModeNode: {
			if (!pMarker->mtext().isNull()) {
				snapPoints.reserve(5);
				pMarker->mtext()->getBoundingPoints(snapPoints);
			}
			snapPoints.append(pMarker->position());
		}
		break;
		case OdDb::kOsModeQuad: case OdDb::kOsModeIntersec: case OdDb::kOsModeIns: case OdDb::kOsModePerp: case OdDb::kOsModeTan: case OdDb::kOsModeNear: case OdDb::kOsModeApint: case OdDb::kOsModePar: case OdDb::kOsModeStart: default:
			break;
	}
	return eOk;
}
