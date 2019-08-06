#include <OdaCommon.h>
#include "DbOleGripPoints.h"
#include <Ge/GeMatrix3d.h>
#include <Ge/GeScale3d.h>
#include <Ge/GeExtents3d.h>

OdResult OdDbOleGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdGeExtents3d Extents;
	const auto size {gripPoints.size()};
	gripPoints.resize(size + 5);
	OdRectangle3d rect;
	OdDbOle2FramePtr ole2Frame = entity;
	ole2Frame->position(rect);
	gripPoints[size + 0] = rect.lowLeft;
	gripPoints[size + 1] = rect.lowRight;
	gripPoints[size + 2] = rect.upLeft;
	gripPoints[size + 3] = rect.upRight;
	const auto Result {entity->getGeomExtents(Extents)};
	if (Result == eOk) {
		gripPoints[size + 4] = Extents.minPoint() + (Extents.maxPoint() - Extents.minPoint()) / 2.;
	}
	return eOk;
}

OdResult OdDbOleGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	OdGePoint3dArray point3dArray;
	OdRectangle3d rect;
	OdDbOle2FramePtr ole2Frame = entity;
	if (indices.empty()) {
		return eOk;
	}
	auto Result {getGripPoints(entity, point3dArray)};
	if (Result != eOk) {
		return eOk;
	}
	auto gripPoints(point3dArray);
	//  OdGeMatrix3d x = OdGeMatrix3d::worldToPlane(ole2Frame->normal());
	for (unsigned i = 0; i < indices.size(); ++i) {
		gripPoints[indices[i]] += offset;
		//    gripPoints[indices[i]] = (x * gripPoints[indices[i]]);
	}
	ole2Frame->position(rect);
	switch (indices[0]) {
		case 0: /*lowLeft*/ {
			if (point3dArray[0].x - point3dArray[3].x != 0 && point3dArray[0].y - point3dArray[3].y != 0) {
				const auto ScaleX {fabs((gripPoints[0].x - gripPoints[3].x) / (point3dArray[0].x - point3dArray[3].x))};
				const auto ScaleY {fabs((gripPoints[0].y - gripPoints[3].y) / (point3dArray[0].y - point3dArray[3].y))};
				const OdGeScale3d odGeScale3d(ScaleX, ScaleY, 1.0);
				Result = entity->transformBy(OdGeMatrix3d::scaling(odGeScale3d, gripPoints[3]));
			}
		}
		break;
		case 1: /*lowRight*/ {
			if (point3dArray[1].x - point3dArray[2].x != 0 && point3dArray[1].y - point3dArray[2].y != 0) {
				const auto ScaleX {fabs((gripPoints[1].x - gripPoints[2].x) / (point3dArray[1].x - point3dArray[2].x))};
				const auto ScaleY {fabs((gripPoints[1].y - gripPoints[2].y) / (point3dArray[1].y - point3dArray[2].y))};
				const OdGeScale3d odGeScale3d(ScaleX, ScaleY, 1.0);
				Result = entity->transformBy(OdGeMatrix3d::scaling(odGeScale3d, gripPoints[2]));
			}
		}
		break;
		case 2: /*upLeft*/ {
			if (point3dArray[0].x - point3dArray[3].x != 0 && point3dArray[0].y - point3dArray[3].y != 0) {
				const auto ScaleX {fabs((gripPoints[1].x - gripPoints[2].x) / (point3dArray[1].x - point3dArray[2].x))};
				const auto ScaleY {fabs((gripPoints[1].y - gripPoints[2].y) / (point3dArray[1].y - point3dArray[2].y))};
				const OdGeScale3d odGeScale3d(ScaleX, ScaleY, 1.0);
				Result = entity->transformBy(OdGeMatrix3d::scaling(odGeScale3d, gripPoints[1]));
			}
		}
		break;
		case 3: /*upRight*/ {
			if (point3dArray[1].x - point3dArray[2].x != 0 && point3dArray[1].y - point3dArray[2].y != 0) {
				const auto ScaleX {fabs((gripPoints[0].x - gripPoints[3].x) / (point3dArray[0].x - point3dArray[3].x))};
				const auto ScaleY {fabs((gripPoints[0].y - gripPoints[3].y) / (point3dArray[0].y - point3dArray[3].y))};
				const OdGeScale3d odGeScale3d(ScaleX, ScaleY, 1.0);
				Result = entity->transformBy(OdGeMatrix3d::scaling(odGeScale3d, gripPoints[0]));
			}
		}
		break;
		case 4: /* center */ {
			const auto Offset {gripPoints[4] - point3dArray[4]};
			Result = entity->transformBy(OdGeMatrix3d::translation(Offset));
		}
		default:
			break;
	}
	return eOk;
}

OdResult OdDbOleGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	const auto Result {getGripPoints(entity, stretchPoints)};
	return Result;
}

OdResult OdDbOleGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDbOleGripPointsPE::getOsnapPoints(const OdDbEntity* /*entity*/, OdDb::OsnapMode /*objectSnapMode*/, OdGsMarker /*gsSelectionMark*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*xWorldToEye*/, OdGePoint3dArray& /*snapPoints*/) const {
	return eOk;
}
