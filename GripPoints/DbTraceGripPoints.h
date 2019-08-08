#pragma once
#include "DbEntityGripPoints.h"
class _DbTraceGPRedir;

class OdDbTraceGripPointsPE_Base {
public:
	virtual OdResult getGripPoints(const _DbTraceGPRedir* trace, OdGePoint3dArray& gripPoints) const;

	virtual OdResult moveGripPointsAt(_DbTraceGPRedir* trace, const OdIntArray& indices, const OdGeVector3d& offset);

	virtual OdResult getStretchPoints(const _DbTraceGPRedir* trace, OdGePoint3dArray& stretchPoints) const;

	virtual OdResult moveStretchPointsAt(_DbTraceGPRedir* trace, const OdIntArray& indices, const OdGeVector3d& offset);

	virtual OdResult getOsnapPoints(const _DbTraceGPRedir* trace, OdDb::OsnapMode objectSnapMode, OdGsMarker selectionMarker, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& viewXform, OdGePoint3dArray& snapPoints) const;
};

class OdDbTraceGripPointsPE : public OdDbEntityGripPointsPE, protected OdDbTraceGripPointsPE_Base {
public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const override;

	OdResult moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker selectionMarker, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& viewXform, OdGePoint3dArray& snapPoints) const override;
};

class OdDbSolidGripPointsPE : public OdDbEntityGripPointsPE, protected OdDbTraceGripPointsPE_Base {
public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const override;

	OdResult moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker selectionMarker, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& viewXform, OdGePoint3dArray& snapPoints) const override;
};
