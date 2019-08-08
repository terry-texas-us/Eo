#pragma once
#include <DbCircle.h>
#include <DbGripPoints.h>

class OdDbCircleGripPointsPE : public OdDbGripPointsPE {
	/**
	 * \brief 
	 * \param circle 
	 * \param point 
	 * \return  recalculated point in plane of circle
	 */
	static OdGePoint3d GetPlanePoint(const OdDbCirclePtr& circle, OdGePoint3d point) {
		OdGePlane Plane;
		OdDb::Planarity Planarity;
		circle->getPlane(Plane, Planarity);
		Plane.intersectWith(OdGeLine3d(point, circle->normal()), point);
		return point;
	}

public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const override;

	OdResult moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker selectionMarker, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& worldToEyeTransform, OdGePoint3dArray& snapPoints) const override;
};
