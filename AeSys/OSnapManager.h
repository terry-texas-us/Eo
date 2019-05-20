#pragma once

// From Examples\Editor\OSnapManager.h  (last compare 19.12)

#include "Gi/GiDrawableImpl.h"
#include "Gs/Gs.h"
#include "Si/SiSpatialIndex.h"
#include "DbEntity.h"
#include "Gs/GsSelectionReactor.h"
#include "Gi/GiViewportDraw.h"
#include "Gi/GiWorldDraw.h"
#include "Gi/GiPathNode.h"
#include "Gi/GiViewport.h"

#include "DbUserIO.h"
#include "StaticRxObject.h"
#include "DbCurve.h"
#include "DbCircle.h"
#include "DbLine.h"
#include "DbArc.h"

class OdEdInputTracker;

class OdEdOSnapMan;
typedef OdSmartPtr<OdEdOSnapMan> OdEdOSnapManPtr;

class OdEdPointTrackerWithSnapInfo : public OdStaticRxObject<OdEdPointTracker> {
public:

	struct SnapContext {
		bool mValid;
		OdDbObjectId mEntityObjectId;

		OdGePoint3d mPoint;
		OdGePoint3d* mLastPoint;

		OdDb::OsnapMode mMode;
		OdGsMarker mMarker;

	}
	m_SnapContext;


	virtual bool IsTargetEntity(const OdDbEntity* entity) const {
		return m_ObjectIds.contains(entity->objectId());
	}

	virtual void GetSnapModes(const OdDbEntity* entity, OdArray<OdDb::OsnapMode>& snapModes) {
		auto Curve {OdDbCurve::cast(entity)};
		
		if (Curve.isNull()) { return; }

		if (Curve->isA()->isEqualTo(OdDbLine::desc()) || Curve->isA()->isEqualTo(OdDbArc::desc())) {
			snapModes.append(OdDb::kOsModeEnd);
			snapModes.append(OdDb::kOsModeMid);
		}

		if (Curve->isA()->isEqualTo(OdDbCircle::desc())) {
			snapModes.append(OdDb::kOsModeCen);
		}
	}

	void setValue(const OdGePoint3d& value) noexcept override {
	}

	OdEdPointTrackerWithSnapInfo(const OdDbObjectIdArray& objectIds) {
		m_ObjectIds = objectIds;
		m_SnapContext.mValid = false;
	}

	int addDrawables(OdGsView* view) override { return 0; }
	void removeDrawables(OdGsView* view) override {}

private:
	OdDbObjectIdArray m_ObjectIds;
};

class OdBaseSnapManager 
	: public OdGiDrawableImpl<OdGiDrawable>
	, public OdGsSelectionReactor {
// TODO move using of OdDbdatabase, OdDbObject & OdDbObjectId into OSnapManager

	OdGsView* m_View;
	OdGePoint3d* m_PickPoint;
	const OdGePoint3d* m_LastPoint;
	OdGePoint3dArray m_SnapPoints;
	OdEdInputTracker* m_InputTracker;

	double m_WorldToDevice;
	double m_NearDist;
	OdGePoint3d m_SnapPoint;
	OdDb::OsnapMode m_SnapMode;
	bool m_Redraw;
	double m_HitRadius;

	OdInt32 GetAperture(OdDbDatabase* database) const;

	struct SubentId {
		SubentId() {}
		OdDbObjectIdArray m_Path;
		OdGsMarker m_Marker;
		SubentId(const OdGiPathNode& pathNode);
		bool operator==(const SubentId& other) const;
	};

	struct HistEntry {
		HistEntry() {}
		HistEntry(const SubentId& subentId, const OdGePoint3d& point)
			: m_SubentId(subentId)
			, m_Point(point) {
		}
		bool operator==(const HistEntry& other) const {
			return other.m_SubentId == m_SubentId;
		}

		SubentId m_SubentId;
		OdGePoint3d m_Point;
	};

	struct SelectedEntityData {
		SubentId SubentId;
		OdGeMatrix3d ModelToWorldTransform;
		
		void set(const OdGiPathNode& pathNode) {
			SubentId = pathNode;
			
			if (pathNode.modelToWorld()) {
				ModelToWorldTransform = *pathNode.modelToWorld();
			}
		}
	};
	typedef OdArray<SelectedEntityData> SelectedEntityDataArray;

	SelectedEntityDataArray m_SelectedEntityData;

	void CheckSnapPoints(const SelectedEntityData& data, const OdGeMatrix3d& worldToEyeTransform);

	bool Checkpoint(OdDb::OsnapMode objectSnapMode, const OdGePoint3d& point);

	typedef OdArray<HistEntry> HistEntryArray;
	static bool AppendToQueue(HistEntryArray& array, const HistEntry& entry);

	HistEntryArray m_Centers;

	OdUInt32 subSetAttributes(OdGiDrawableTraits* drawableTraits) const override;
	bool subWorldDraw(OdGiWorldDraw* worldDraw) const override;
	void subViewportDraw(OdGiViewportDraw* viewportDraw) const override;

	bool selected(const OdGiDrawableDesc& drawableDesc) override;
	OdUInt32 selected(const OdGiPathNode& pathNode, const OdGiViewport& viewInfo) override;
	void InvalidateViewport(const OdGePoint3d& point) const;
	void InvalidateViewport(const HistEntryArray& centers) const;
protected:
	OdBaseSnapManager() noexcept;
public:
	void Track(OdEdInputTracker* inputTracker);

	bool Snap(OdGsView* view, OdGePoint3d& point, const OdGePoint3d* lastPoint);

	virtual unsigned SnapModes() const = 0;

	virtual unsigned ToSnapModes(OdDb::OsnapMode mode) const noexcept {
	  // was temporary moved into OSnapManager // return 1 << mode;
		return 1 << (mode + 1);
	}

	virtual OdCmEntityColor SnapTrueColor() const {
		OdCmEntityColor Color;
		Color.setColorIndex(OdCmEntityColor::kACIYellow);
		return Color;
	}

	virtual OdCmEntityColor CenterTrueColor() const {
		OdCmEntityColor Color;
		Color.setColorIndex(OdCmEntityColor::kACIforeground);
		return Color;
	}

	void Reset();

	void RecalculateEntityCenters();
	virtual bool SetEntityCenters(OdRxObject* rxObject);
	void SetEntityCenters(OdDbBlockTableRecord* blockTableRecord, const OdGeMatrix3d & matrix = OdGeMatrix3d::kIdentity);
};

class OSnapManager : public OdBaseSnapManager {
	unsigned m_SnapModes;
protected:
	OSnapManager() noexcept;
public:
	unsigned SnapModes() const noexcept override;
	void SetSnapModes(unsigned snapModes) noexcept;

	// TODO comment next override with mistake and check OdaMfcApp behaviour
	unsigned ToSnapModes(OdDb::OsnapMode mode) const noexcept override { return 1 << mode; }
};
