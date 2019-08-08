#pragma once
#include <DbGripPoints.h>

class OdDbRasterImageGripPointsPE : public OdDbGripPointsPE {
public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const override;

	OdResult moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker selectionMarker, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& worldToEyeTransform, OdGePoint3dArray& snapPoints) const override;
};
