#pragma once
#include <Ge/GeLine3d.h>
#include <DbEllipse.h>
#include <DbGripPoints.h>

class OdDbEllipseGripPointsPE : public OdDbGripPointsPE {
	OdGePoint3d getPlanePoint(const OdDbEllipsePtr circle, OdGePoint3d Point) const {
		OdGePlane plane; // recalculated Point in plane of pCircle
		OdDb::Planarity planarity;
		circle->getPlane(plane, planarity);
		plane.intersectWith(OdGeLine3d(Point, circle->normal()), Point);
		return Point;
	}

public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const override;

	OdResult moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker gsSelectionMark, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& xWorldToEye, OdGePoint3dArray& snapPoints) const override;

protected:
	double m_lll;
};
