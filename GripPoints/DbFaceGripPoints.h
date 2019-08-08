#pragma once
#include <DbFace.h>
#include <DbGripPoints.h>

class OdDbFaceGripPointsPE : public OdDbGripPointsPE {
	/**
	 * \brief 
	 * \param face 
	 * \param point 
	 * \return  recalculated point in plane of face
	 */
	static OdGePoint3d GetPlanePoint(const OdDbFacePtr& face, OdGePoint3d point) {
		OdGePlane Plane;
		OdDb::Planarity Planarity;
		face->getPlane(Plane, Planarity);
		Plane.intersectWith(OdGeLine3d(point, Plane.normal()), point);
		return point;
	}

public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const override;

	OdResult moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker selectionMarker, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& worldToEyeTransform, OdGePoint3dArray& snapPoints) const override;
};
