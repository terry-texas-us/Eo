#pragma once
#include <DbGripPoints.h>

class OdDbMleaderGripPointsPE : public OdDbGripPointsPE {
public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const override;

	OdResult moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getGripPointsAtSubentPath(const OdDbEntity* entity, const OdDbFullSubentPath& path, OdDbGripDataPtrArray& grips, double curViewUnitSize, int gripSize, const OdGeVector3d& curViewDir, OdUInt32 bitFlags) const override;

	OdResult moveGripPointsAtSubentPaths(OdDbEntity* entity, const OdDbFullSubentPathArray& paths, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, OdUInt32 bitFlags) override;

	OdResult getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker gsSelectionMark, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& xWorldToEye, OdGePoint3dArray& snapPoints) const override;
};
