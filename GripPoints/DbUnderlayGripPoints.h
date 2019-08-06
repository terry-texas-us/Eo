#pragma once
#include <DbUnderlayReference.h>
#include "DbEntityGripPoints.h"

class OdDbUnderlayGripPointsPE : public OdDbEntityGripPointsPE {
protected:
	virtual OdResult checkBorder(const OdDbUnderlayReferencePtr& pRef, OdDb::OsnapMode objectSnapMode, const OdGePoint3d& pickPoint, OdGePoint3dArray& snapPoints) const;

public:
	OdResult getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker gsSelectionMark, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& xWorldToEye, OdGePoint3dArray& snapPoints) const override;
};
