#pragma once
class OdDbBlockReferenceGripPointsPE : public OdDbEntityGripPointsPE {
public:
	OdResult getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const override;

	OdResult moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	OdResult getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker selectionMarker, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& viewTransform, OdGePoint3dArray& snapPoints) const override;

	OdResult getGripPoints(const OdDbEntity* entity, OdDbGripDataPtrArray& grips, double currentViewUnitSize, int gripSize, const OdGeVector3d& currentViewDirection, int bitFlags) const override;

	OdResult moveGripPointsAt(OdDbEntity* entity, const OdDbVoidPtrArray& grips, const OdGeVector3d& offset, int bitFlags) override;
};

class OdDbBlockGripAppData : public OdRxObject {
public:
	OdDbBlockGripAppData() = default;

ODRX_DECLARE_MEMBERS(OdDbBlockGripAppData);

	int m_nAttributeIndex {-1};
	int m_nClipGripGripIndex {-1};
	bool m_bClipInvertGrip {false};
	OdGeVector3d m_vClipInvertOrientation;
};
