#pragma once

#include "Gi/GiDrawableImpl.h"
#include "Gs/GsSelectionReactor.h"
#include "Si/SiSpatialIndex.h"
#include "DbEntity.h"

class EoObjectSnapManager : public OdGiDrawableImpl<OdGiDrawable>, public OdGsSelectionReactor {
	void checkSnapPoints(const OdDbEntity* entity, const OdGiPathNode& pathNode, unsigned snapModes, OdGsMarker gsMarker, const OdGeMatrix3d& xModelToWorld, const OdGeMatrix3d& xWorldToModel, const OdGeMatrix3d& xWorldToEye);
	bool checkpoint(OdDb::OsnapMode osm, const OdGePoint3d& point);

	OdGsView* m_pView;
	OdGePoint3d* m_pPickPoint;
	const OdGePoint3d* m_pLastPoint;
	unsigned m_nSnapModes;
	OdGePoint3dArray m_snapPointsBuff;

	double m_dWorldToDevice;
	double m_dNearDist;
	OdGePoint3d m_snapPoint;
	OdDb::OsnapMode m_mode;
	bool m_bRedraw;
	
	struct SubentId {
		SubentId() {
		}
		OdDbObjectIdArray m_path;
		OdGsMarker m_gsMarker;
		SubentId(const OdGiPathNode& pathNode);
		bool operator==(const SubentId& other) const;
	};

	struct HistEntry {
		HistEntry() {
		}
		HistEntry(const OdGiPathNode& pathNode, const OdGePoint3d& point) :
			m_subentId(pathNode), m_point(point) {
		}
		bool operator==(const HistEntry& other) const {
			return other.m_subentId == m_subentId;
		}
		SubentId m_subentId;
		OdGePoint3d m_point;
	};

	typedef OdArray<HistEntry> HistEntryArray;
	static bool appendToQueue(HistEntryArray& array, const HistEntry& entry);

	HistEntryArray m_centers;

	OdUInt32 subSetAttributes(OdGiDrawableTraits* drawableTraits) const noexcept override;
	bool subWorldDraw(OdGiWorldDraw* worldDraw) const noexcept override;
	void subViewportDraw(OdGiViewportDraw* viewportDraw) const override;

// <OdGsSelectionReactor> virtuals
	bool selected(const OdGiDrawableDesc& drawableDesc) noexcept override;
	OdUInt32 selected(const OdGiPathNode& pathNode, const OdGiViewport& viewport) override;
// </OdGsSelectionReactor> virtuals
	
	void invalidateViewport(const OdGePoint3d& point) const;
	void invalidateViewport(const HistEntryArray& centers) const;
protected:
	EoObjectSnapManager();

public:
	bool snap(OdGsView* view, OdGePoint3d& point, const OdGePoint3d* lastPoint);
	unsigned snapModes() const noexcept;
	void SetSnapModes(unsigned snapModes) noexcept;
	void reset();
};
