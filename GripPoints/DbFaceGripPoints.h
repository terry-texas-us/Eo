#pragma once
#include <Ge/GeLine3d.h>
#include <Ge/GePlanarEnt.h>
#include <DbFace.h>
#include <DbGripPoints.h>

class OdDbFaceGripPointsPE : public OdDbGripPointsPE {
	OdGePoint3d getPlanePoint(const OdDbFacePtr pFace, OdGePoint3d point) const {
		OdGePlane Plane; // recalculated Point in plane of pFace
		OdDb::Planarity Planarity;
		pFace->getPlane(Plane, Planarity);
		Plane.intersectWith(OdGeLine3d(point, Plane.normal()), point);
		return point;
	}

public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const override;

	OdResult moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker gsSelectionMark, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& xWorldToEye, OdGePoint3dArray& snapPoints) const override;
};
