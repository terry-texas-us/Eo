#pragma once
#include <DbUnderlayReference.h>
#include "DbEntityGripPoints.h"

class OdDbUnderlayGripPointsPE : public OdDbEntityGripPointsPE {
protected:
	virtual OdResult CheckBorder(const OdDbUnderlayReferencePtr& underlayReference, OdDb::OsnapMode objectSnapMode, const OdGePoint3d& pickPoint, OdGePoint3dArray& snapPoints) const;

public:
	OdResult getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker selectionMarker, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& worldToEyeTransform, OdGePoint3dArray& snapPoints) const override;
};
