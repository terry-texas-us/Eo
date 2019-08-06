#pragma once
#include <DbText.h>
#include <DbGripPoints.h>

/* This class is an implementation of the OdDbGripPointsPE class for    */
/* OdDbText entities.                                                 */
class OdDbTextGripPointsPE : public OdDbGripPointsPE {
	bool is_Justify_Left(const OdDbText* pText) const;

	bool is_Justify_Aligned(const OdDbText* pText) const;

	bool is_Justify_Fit(const OdDbText* pText) const;

public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const override;

	OdResult moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker gsSelectionMark, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& xWorldToEye, OdGePoint3dArray& snapPoints) const override;
};
