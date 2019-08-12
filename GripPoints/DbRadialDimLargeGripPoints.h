#pragma once
#include "DbDimGripPoints.h"

class OdDbRadialDimLargeGripPointsPE : public OdDbDimGripPointsPE {
public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool stretch) override;

private:
	enum GripPoints {kChordPoint, kOverrideCenter, kJogPoint, kTextPosition};
};

