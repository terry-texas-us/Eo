#pragma once
#include <DbEllipse.h>
#include <DbGripPoints.h>

class OdDbEllipseGripPointsPE : public OdDbGripPointsPE {
	/**
	 * \brief 
	 * \param ellipse 
	 * \param point 
	 * \return   recalculated point in plane of ellipse
	 */
	static OdGePoint3d GetPlanePoint(const OdDbEllipsePtr& ellipse, OdGePoint3d point) {
		OdGePlane Plane;
		OdDb::Planarity Planarity;
		ellipse->getPlane(Plane, Planarity);
		Plane.intersectWith(OdGeLine3d(point, ellipse->normal()), point);
		return point;
	}

public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const override;

	OdResult moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker selectionMarker, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& worldToEyeTransform, OdGePoint3dArray& snapPoints) const override;

protected:
	double m_lll {0.0};
};
