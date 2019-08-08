#pragma once
#include "DbUnderlayGripPoints.h"

class OdDbDgnUnderlayGripPointsPE : public OdDbUnderlayGripPointsPE {
public:
	OdResult getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker selectionMarker, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& worldToEyeTransform, OdGePoint3dArray& snapPoints) const override;
};
