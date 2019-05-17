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
		if (pCurve.isNull())
			return;

		if (pCurve->isA()->isEqualTo(OdDbLine::desc()) || pCurve->isA()->isEqualTo(OdDbArc::desc())) {
			snapModes.append(OdDb::kOsModeEnd);
			snapModes.append(OdDb::kOsModeMid);
		}

		if (pCurve->isA()->isEqualTo(OdDbCircle::desc())) {
			snapModes.append(OdDb::kOsModeCen);
		}
	}

	void setValue(const OdGePoint3d& value) override {
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

	OdGsView* m_pView;
	OdGePoint3d* m_pPickPoint;
	const OdGePoint3d* m_pLastPoint;
	OdGePoint3dArray m_snapPointsBuff;
	OdEdInputTracker* m_pTracker;

	double m_dWorldToDevice;
	double m_dNearDist;
	OdGePoint3d m_snapPoint;
	OdDb::OsnapMode m_mode;
	bool m_bRedraw;
	double m_hitRadius;

	OdInt32 getAperture(OdDbDatabase* pDb) const;

	struct SubentId {
		SubentId() {}
		OdDbObjectIdArray m_path;
		OdGsMarker m_gsMarker;
		SubentId(const OdGiPathNode& giPath);
		bool operator == (const SubentId& op) const;
	};

	struct HistEntry {
		HistEntry() {
		}
		HistEntry(const SubentId& subentId, const OdGePoint3d& point)
			: m_subentId(subentId)
			, m_point(point) {
		}
		bool operator == (const HistEntry& op) const {
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

	void checkSnapPoints(const SelectedEntityData& data, const OdGeMatrix3d& xWorldToEye);

	bool checkpoint(OdDb::OsnapMode osm, const OdGePoint3d& point);

	typedef OdArray<HistEntry> HistEntryArray;
	static bool appendToQueue(HistEntryArray& array, const HistEntry& entry);

	HistEntryArray m_centers;

	OdUInt32 subSetAttributes(OdGiDrawableTraits*) const override;
	bool subWorldDraw(OdGiWorldDraw* pWd) const override;
	void subViewportDraw(OdGiViewportDraw*) const override;

	bool selected(const OdGiDrawableDesc& pDrawableDesc) override;
	OdUInt32 selected(const OdGiPathNode& pathNode, const OdGiViewport& viewInfo) override;
	void invalidateViewport(const OdGePoint3d& point) const;
	void invalidateViewport(const HistEntryArray& centers) const;
protected:
	OdBaseSnapManager();
public:
	void track(OdEdInputTracker* pTracker);

	bool snap(OdGsView* pView, OdGePoint3d& point, const OdGePoint3d* pLastPoint);

	virtual unsigned snapModes() const = 0;
	virtual unsigned toSnapModes(OdDb::OsnapMode mode) const {
	  // was temporary moved into OSnapManager // return 1 << mode;
		return 1 << (mode + 1);
	}
	virtual OdCmEntityColor snapTrueColor() const {
		OdCmEntityColor color;
		color.setColorIndex(OdCmEntityColor::kACIYellow);
		return color;
	}
	virtual OdCmEntityColor centerTrueColor() const {
		OdCmEntityColor color;
		color.setColorIndex(OdCmEntityColor::kACIforeground);
		return color;
	}

	void reset();

	void Recalc_Entity_centers();
	virtual bool Set_Entity_centers(OdRxObject * pRxDb);
	void Set_Entity_centers(OdDbBlockTableRecord * pBTR, const OdGeMatrix3d & matrix = OdGeMatrix3d::kIdentity);
};

class OSnapManager : public OdBaseSnapManager {
	unsigned m_nSnapModes;
protected:
	OSnapManager();
public:
	unsigned snapModes() const override;
	void setSnapModes(unsigned nSnapModes);

	// TODO comment next override with mistake and check OdaMfcApp behaviour
	unsigned toSnapModes(OdDb::OsnapMode mode) const override { return 1 << mode; }
};
