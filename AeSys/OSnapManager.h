#pragma once

// From Examples\Editor\OSnapManager.h  (last compare 19.12)
#include <Gi/GiDrawableImpl.h>
#include <Gs/Gs.h>
#include <Si/SiSpatialIndex.h>
#include <DbEntity.h>
#include <Gs/GsSelectionReactor.h>
#include <Gi/GiWorldDraw.h>
#include <Gi/GiPathNode.h>
#include <DbUserIO.h>
#include <DbCurve.h>
#include <DbCircle.h>
#include <DbLine.h>
#include <DbArc.h>
class OdEdInputTracker;
class OdEdOSnapMan;
using OdEdOSnapManPtr = OdSmartPtr<OdEdOSnapMan>;

class OdEdPointTrackerWithSnapInfo : public OdStaticRxObject<OdEdPointTracker> {
public:
	struct SnapContext {
		bool mValid {false};
		OdDbObjectId mEntityObjectId;
		OdGePoint3d mPoint;
		OdGePoint3d* mLastPoint {nullptr};
		OdDb::OsnapMode mMode;
		OdGsMarker mMarker {0};
	} m_SnapContext;

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

	int addDrawables(OdGsView* view) noexcept override { return 0; }

	void removeDrawables(OdGsView* view) noexcept override {
	}

private:
	OdDbObjectIdArray m_ObjectIds;
};

#define hitradius 15

class OdBaseSnapManager : public OdGiDrawableImpl<OdGiDrawable>, public OdGsSelectionReactor {
	// TODO move using of OdDbdatabase, OdDbObject & OdDbObjectId into OSnapManager
	OdGsView* m_View {nullptr};
	OdGePoint3d* m_PickPoint {nullptr};
	const OdGePoint3d* m_LastPoint {nullptr};
	OdGePoint3dArray m_SnapPoints;
	OdEdInputTracker* m_InputTracker {nullptr};
	double m_WorldToDevice {0.0};
	double m_NearDist;
	OdGePoint3d m_SnapPoint;
	OdDb::OsnapMode m_SnapMode;
	bool m_Redraw;
	double m_HitRadius {hitradius};
	long GetAperture(OdDbDatabase* database) const;

	struct SubentId {
		SubentId() = default;
		OdDbObjectIdArray m_Path;
		OdGsMarker m_Marker {0};
		SubentId(const OdGiPathNode& pathNode);
		bool operator==(const SubentId& other) const;
	};

	struct HistEntry {
		HistEntry() = default;

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

	using SelectedEntityDataArray = OdArray<SelectedEntityData>;
	SelectedEntityDataArray m_SelectedEntityData;
	void CheckSnapPoints(const SelectedEntityData& data, const OdGeMatrix3d& worldToEyeTransform);
	bool Checkpoint(OdDb::OsnapMode objectSnapMode, const OdGePoint3d& point);
	using HistEntryArray = OdArray<HistEntry>;
	static bool AppendToQueue(HistEntryArray& array, const HistEntry& entry);
	HistEntryArray m_Centers;
	unsigned long subSetAttributes(OdGiDrawableTraits* drawableTraits) const override;
	bool subWorldDraw(OdGiWorldDraw* worldDraw) const override;
	void subViewportDraw(OdGiViewportDraw* viewportDraw) const override;
	bool selected(const OdGiDrawableDesc& drawableDesc) override;
	unsigned long selected(const OdGiPathNode& pathNode, const OdGiViewport& viewInfo) override;
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
		return static_cast<unsigned>(1 << mode + 1);
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
	void SetEntityCenters(OdDbBlockTableRecord* blockTableRecord, const OdGeMatrix3d& matrix = OdGeMatrix3d::kIdentity);
};

class OSnapManager : public OdBaseSnapManager {
	unsigned m_SnapModes {0xFFFFFFFF};
protected:
	OSnapManager() = default;
public:
	unsigned SnapModes() const noexcept override;
	void SetSnapModes(unsigned snapModes) noexcept;

	// TODO comment next override with mistake and check OdaMfcApp behaviour
	unsigned ToSnapModes(OdDb::OsnapMode mode) const noexcept override { return static_cast<unsigned>(1 << mode); }
};
