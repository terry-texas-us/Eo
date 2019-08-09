#pragma once
#include <DbGripPoints.h>

class OdDbMleaderGripPointsPE : public OdDbGripPointsPE {
public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const override;

	OdResult moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getGripPointsAtSubentPath(const OdDbEntity* entity, const OdDbFullSubentPath& path, OdDbGripDataPtrArray& grips, double currentViewUnitSize, int gripSize, const OdGeVector3d& currentViewDirection, unsigned long bitFlags) const override;

	OdResult moveGripPointsAtSubentPaths(OdDbEntity* entity, const OdDbFullSubentPathArray& paths, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, unsigned long bitFlags) override;

	OdResult getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker selectionMarker, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& worldToEyeTransform, OdGePoint3dArray& snapPoints) const override;
};
