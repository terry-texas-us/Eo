#pragma once
#include "DbUnderlayGripPoints.h"

class OdDbPdfUnderlayGripPointsPE : public OdDbUnderlayGripPointsPE {
public:
	OdResult getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker gsSelectionMark, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& xWorldToEye, OdGePoint3dArray& snapPoints) const override;
};
