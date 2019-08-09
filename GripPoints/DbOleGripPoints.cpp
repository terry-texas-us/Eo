#include <OdaCommon.h>
#include "DbOleGripPoints.h"
#include <Ge/GeMatrix3d.h>
#include <Ge/GeScale3d.h>
#include <Ge/GeExtents3d.h>
#include <DbOle2Frame.h>

OdResult OdDbOleGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdGeExtents3d Extents;
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 5);
	OdRectangle3d rect;
	OdDbOle2FramePtr ole2Frame = entity;
	ole2Frame->position(rect);
	gripPoints[GripPointsSize + 0] = rect.lowLeft;
	gripPoints[GripPointsSize + 1] = rect.lowRight;
	gripPoints[GripPointsSize + 2] = rect.upLeft;
	gripPoints[GripPointsSize + 3] = rect.upRight;
	const auto Result {entity->getGeomExtents(Extents)};
	if (Result == eOk) {
		gripPoints[GripPointsSize + 4] = Extents.minPoint() + (Extents.maxPoint() - Extents.minPoint()) / 2.;
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
	auto GripPoints(point3dArray);
	for (auto Index : indices) {
		GripPoints[Index] += offset;
	}
	ole2Frame->position(rect);
	switch (indices[0]) {
		case 0: /*lowLeft*/ {
			if (point3dArray[0].x - point3dArray[3].x != 0 && point3dArray[0].y - point3dArray[3].y != 0) {
				const auto ScaleX {fabs((GripPoints[0].x - GripPoints[3].x) / (point3dArray[0].x - point3dArray[3].x))};
				const auto ScaleY {fabs((GripPoints[0].y - GripPoints[3].y) / (point3dArray[0].y - point3dArray[3].y))};
				const OdGeScale3d odGeScale3d(ScaleX, ScaleY, 1.0);
				Result = entity->transformBy(OdGeMatrix3d::scaling(odGeScale3d, GripPoints[3]));
			}
			break;
		}
		case 1: /*lowRight*/ {
			if (point3dArray[1].x - point3dArray[2].x != 0 && point3dArray[1].y - point3dArray[2].y != 0) {
				const auto ScaleX {fabs((GripPoints[1].x - GripPoints[2].x) / (point3dArray[1].x - point3dArray[2].x))};
				const auto ScaleY {fabs((GripPoints[1].y - GripPoints[2].y) / (point3dArray[1].y - point3dArray[2].y))};
				const OdGeScale3d odGeScale3d(ScaleX, ScaleY, 1.0);
				Result = entity->transformBy(OdGeMatrix3d::scaling(odGeScale3d, GripPoints[2]));
			}
			break;
		}
		case 2: /*upLeft*/ {
			if (point3dArray[0].x - point3dArray[3].x != 0 && point3dArray[0].y - point3dArray[3].y != 0) {
				const auto ScaleX {fabs((GripPoints[1].x - GripPoints[2].x) / (point3dArray[1].x - point3dArray[2].x))};
				const auto ScaleY {fabs((GripPoints[1].y - GripPoints[2].y) / (point3dArray[1].y - point3dArray[2].y))};
				const OdGeScale3d odGeScale3d(ScaleX, ScaleY, 1.0);
				Result = entity->transformBy(OdGeMatrix3d::scaling(odGeScale3d, GripPoints[1]));
			}
			break;
		}
		case 3: /*upRight*/ {
			if (point3dArray[1].x - point3dArray[2].x != 0 && point3dArray[1].y - point3dArray[2].y != 0) {
				const auto ScaleX {fabs((GripPoints[0].x - GripPoints[3].x) / (point3dArray[0].x - point3dArray[3].x))};
				const auto ScaleY {fabs((GripPoints[0].y - GripPoints[3].y) / (point3dArray[0].y - point3dArray[3].y))};
				const OdGeScale3d odGeScale3d(ScaleX, ScaleY, 1.0);
				Result = entity->transformBy(OdGeMatrix3d::scaling(odGeScale3d, GripPoints[0]));
			}
			break;
		}
		case 4: /* center */ {
			const auto Offset {GripPoints[4] - point3dArray[4]};
			Result = entity->transformBy(OdGeMatrix3d::translation(Offset));
			break;
		}
		default: ;
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

OdResult OdDbOleGripPointsPE::getOsnapPoints(const OdDbEntity* /*entity*/, OdDb::OsnapMode /*objectSnapMode*/, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& /*snapPoints*/) const {
	return eOk;
}
