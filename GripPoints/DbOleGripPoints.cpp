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
	OdDbOle2FramePtr Ole2Frame {entity};
	Ole2Frame->position(rect);
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
	OdGePoint3dArray Points;
	OdRectangle3d Rectangle;
	OdDbOle2FramePtr Ole2Frame {entity};
	if (indices.empty()) {
		return eOk;
	}
	auto Result {getGripPoints(entity, Points)};
	if (Result != eOk) {
		return eOk;
	}
	auto GripPoints(Points);
	for (auto Index : indices) {
		GripPoints[Index] += offset;
	}
	Ole2Frame->position(Rectangle);
	switch (indices[0]) {
		case 0: /*lowLeft*/ {
			if (Points[0].x - Points[3].x != 0 && Points[0].y - Points[3].y != 0) {
				const auto ScaleX {fabs((GripPoints[0].x - GripPoints[3].x) / (Points[0].x - Points[3].x))};
				const auto ScaleY {fabs((GripPoints[0].y - GripPoints[3].y) / (Points[0].y - Points[3].y))};
				const OdGeScale3d Scale(ScaleX, ScaleY, 1.0);
				entity->transformBy(OdGeMatrix3d::scaling(Scale, GripPoints[3]));
			}
			break;
		}
		case 1: /*lowRight*/ {
			if (Points[1].x - Points[2].x != 0 && Points[1].y - Points[2].y != 0) {
				const auto ScaleX {fabs((GripPoints[1].x - GripPoints[2].x) / (Points[1].x - Points[2].x))};
				const auto ScaleY {fabs((GripPoints[1].y - GripPoints[2].y) / (Points[1].y - Points[2].y))};
				const OdGeScale3d Scale(ScaleX, ScaleY, 1.0);
				entity->transformBy(OdGeMatrix3d::scaling(Scale, GripPoints[2]));
			}
			break;
		}
		case 2: /*upLeft*/ {
			if (Points[0].x - Points[3].x != 0 && Points[0].y - Points[3].y != 0) {
				const auto ScaleX {fabs((GripPoints[1].x - GripPoints[2].x) / (Points[1].x - Points[2].x))};
				const auto ScaleY {fabs((GripPoints[1].y - GripPoints[2].y) / (Points[1].y - Points[2].y))};
				const OdGeScale3d Scale(ScaleX, ScaleY, 1.0);
				entity->transformBy(OdGeMatrix3d::scaling(Scale, GripPoints[1]));
			}
			break;
		}
		case 3: /*upRight*/ {
			if (Points[1].x - Points[2].x != 0 && Points[1].y - Points[2].y != 0) {
				const auto ScaleX {fabs((GripPoints[0].x - GripPoints[3].x) / (Points[0].x - Points[3].x))};
				const auto ScaleY {fabs((GripPoints[0].y - GripPoints[3].y) / (Points[0].y - Points[3].y))};
				const OdGeScale3d Scale(ScaleX, ScaleY, 1.0);
				entity->transformBy(OdGeMatrix3d::scaling(Scale, GripPoints[0]));
			}
			break;
		}
		case 4: /* center */ {
			const auto Offset {GripPoints[4] - Points[4]};
			entity->transformBy(OdGeMatrix3d::translation(Offset));
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
