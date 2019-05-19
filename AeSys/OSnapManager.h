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
		bool bValid;
		OdDbObjectId entId;

		OdGePoint3d point;
		OdGePoint3d* pLastPoint;

		OdDb::OsnapMode mode;
		OdGsMarker marker;

	}
	m_SnapContext;


	virtual bool isTargetEntity(const OdDbEntity* pEnt) const {
		return m_srcObj.contains(pEnt->objectId());
	}

	virtual void getSnapModes(const OdDbEntity* pEnt, OdArray<OdDb::OsnapMode>& snapModes) {
		OdDbCurvePtr pCurve = OdDbCurve::cast(pEnt);
		
		if (pCurve.isNull()) { return; }

		if (pCurve->isA()->isEqualTo(OdDbLine::desc()) || pCurve->isA()->isEqualTo(OdDbArc::desc())) {
			snapModes.append(OdDb::kOsModeEnd);
			snapModes.append(OdDb::kOsModeMid);
		}

		if (pCurve->isA()->isEqualTo(OdDbCircle::desc())) {
			snapModes.append(OdDb::kOsModeCen);
		}
	}

	void setValue(const OdGePoint3d& value) noexcept override {
	}

	OdEdPointTrackerWithSnapInfo(const OdDbObjectIdArray& srcObj) {
		m_srcObj = srcObj;
		m_SnapContext.bValid = false;
	}

	int addDrawables(OdGsView*) override { return 0; }
	void removeDrawables(OdGsView*) override {}

private:
	OdDbObjectIdArray m_srcObj;
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
	OdGePoint3d m_snapPoint;
	OdDb::OsnapMode m_mode;
	bool m_Redraw;
	double m_HitRadius;

	OdInt32 GetAperture(OdDbDatabase* database) const;

	struct SubentId {
		SubentId() {}
		OdDbObjectIdArray m_path;
		OdGsMarker m_gsMarker;
		SubentId(const OdGiPathNode& giPath);
		bool operator== (const SubentId& op) const;
	};

	struct HistEntry {
		HistEntry() {
		}
		HistEntry(const SubentId& subentId, const OdGePoint3d& point)
			: m_subentId(subentId)
			, m_point(point) {
		}
		bool operator== (const HistEntry& op) const {
			return op.m_subentId == m_subentId;
		}

		SubentId m_subentId;
		OdGePoint3d m_point;
	};

	struct SelectedEntityData {
		SubentId subentId;
		OdGeMatrix3d xModelToWorld;
		void set(const OdGiPathNode& gipath) {
			subentId = gipath;
			if (gipath.modelToWorld())
				xModelToWorld = *gipath.modelToWorld();
		}
	};
	typedef OdArray<SelectedEntityData> SelectedEntityDataArray;

	SelectedEntityDataArray m_selectedEntityDataArray;

	void CheckSnapPoints(const SelectedEntityData& data, const OdGeMatrix3d& xWorldToEye);

	bool Checkpoint(OdDb::OsnapMode objectSnapMode, const OdGePoint3d& point);

	typedef OdArray<HistEntry> HistEntryArray;
	static bool AppendToQueue(HistEntryArray& array, const HistEntry& entry);

	HistEntryArray m_centers;

	OdUInt32 subSetAttributes(OdGiDrawableTraits* drawableTraits) const override;
	bool subWorldDraw(OdGiWorldDraw* worldDraw) const override;
	void subViewportDraw(OdGiViewportDraw* viewportDraw) const override;

	bool selected(const OdGiDrawableDesc& pDrawableDesc) override;
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
		OdCmEntityColor color;
		color.setColorIndex(OdCmEntityColor::kACIYellow);
		return color;
	}

	virtual OdCmEntityColor CenterTrueColor() const {
		OdCmEntityColor color;
		color.setColorIndex(OdCmEntityColor::kACIforeground);
		return color;
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
