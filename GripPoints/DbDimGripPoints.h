#pragma once
#include <DbGripPoints.h>
#include <DbDimension.h>

/* This class is an implementation of the OdDbGripPointsPE class for    */
/* entities derived from OdDbDimension.                                 */
class OdDbDimGripPointsPE : public OdDbGripPointsPE {
protected:
	const bool isAngleDirectionBetweenVectors(OdGeVector3d v1, OdGeVector3d v2);

public:
	OdResult getGripPoints(const OdDbEntity* /*entity*/, OdGePoint3dArray& /*gripPoints*/) const override {
		return eOk;
	}

	OdResult moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override {
		if (indices.empty()) {
			return eOk;
		}
		try {
			OdGePoint3dArray gripPoints;
			getGripPoints(entity, gripPoints);
			for (unsigned i = 0; i < indices.size(); ++i) {
				if (indices[i] < (int)gripPoints.size()) {
					gripPoints[indices[i]] += offset;
				}
			}
			moveGripPoint(entity, gripPoints, indices, false);
			OdDbDimensionPtr pDimPtr = entity;
			pDimPtr->recomputeDimBlock();
		} catch (const OdError& e) {
			return e.code();
		}
		return eOk;
	}

	virtual OdResult moveGripPoint(OdDbEntity* /*entity*/, const OdGePoint3dArray& /*gripPoints*/, const OdIntArray& /*indices */, bool bStretch) = 0;

	OdResult getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const override;

	OdResult moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker selectionMarker, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& worldToEyeTransform, OdGePoint3dArray& snapPoints) const override;
};
/* This class is an specialization of the OdDbGripPointsPE class for    */
/* OdDbRotatedDimension entities.                                       */
class OdDbRotatedDimGripPointsPE : public OdDbDimGripPointsPE {
public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) override;

	//  virtual OdResult moveGripPointsAt( OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset );
};
/* This class is an specialization of the OdDbGripPointsPE class for    */
/* OdDbAlignedDimension entities.                                       */

class OdDbAlignedDimGripPointsPE : public OdDbDimGripPointsPE {
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) override;

	//  virtual OdResult moveGripPointsAt( OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset );
};
/* This class is an specialization of the OdDbGripPointsPE class for    */
/* OdDbRadialDimension entities.                                        */

class OdDbRadialDimGripPointsPE : public OdDbDimGripPointsPE {
public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) override;

	//virtual OdResult moveGripPointsAt( OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices );
};

// This class is an specialization of the OdDbGripPointsPE class for OdDbDiametricDimension entities.
class OdDbDiametricDimGripPointsPE : public OdDbDimGripPointsPE {
public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) override;

	//  virtual OdResult moveGripPointsAt( OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset );
};
/* This class is an specialization of the OdDbGripPointsPE class for    */
/* OdDbAngularDimension entities.                                       */
class OdDb3PointAngularDimGripPointsPE : public OdDbDimGripPointsPE {
public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) override;

	//  virtual OdResult moveGripPointsAt( OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset );
};
/* This class is an specialization of the OdDbGripPointsPE class for    */
/* OdDbOrdinateDimension entities.                                      */

class OdDbOrdinateDimGripPointsPE : public OdDbDimGripPointsPE {
public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) override;

	//  virtual OdResult moveGripPointsAt( OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset );
};
/* This class is an specialization of the OdDbGripPointsPE class for    */
/* OdDb2LineAngularDimension entities.                                  */
class OdDb2LineAngularDimGripPointsPE : public OdDbDimGripPointsPE {
public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) override;

	//  virtual OdResult moveGripPointsAt( OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset );
};
/* This class is an specialization of the OdDbGripPointsPE class for    */
/* OdDbArcDimension entities.                                           */
class OdDbArcDimGripPointsPE : public OdDbDimGripPointsPE {
public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) override;

	//  virtual OdResult moveGripPointsAt( OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset );
};
/* This class is an specialization of the OdDbGripPointsPE class for    */
/* OdDbRadialDimensionLarge entities.                                   */
class OdDbRadialDimLargeGripPointsPE : public OdDbDimGripPointsPE {
public:
	OdResult getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) override;

	//  virtual OdResult moveGripPointsAt( OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset );
};
