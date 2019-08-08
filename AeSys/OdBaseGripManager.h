// Extracted class from Examples\Editor\ExGripManager.h (last compare 20.5)
#pragma once
#include <map>
#include "OdExGripData.h"
using OdExGripDataPtr = OdSmartPtr<class OdExGripData>;
using OdExGripDataPtrArray = OdArray<OdExGripDataPtr>;
using OdExGripDragPtr = OdSmartPtr<class OdExGripDrag>;
using OdExGripDragPtrArray = OdArray<OdExGripDragPtr>;

class OdBaseGripManager : public OdEdPointTracker {
public:
	OdBaseGripManager() noexcept;

	~OdBaseGripManager();
	// OdEdPointTracker protocol
	void setValue(const OdGePoint3d& value) override;
	// Events from Windows.
	virtual void OnModified(OdGiDrawable* pGrip) = 0;

	virtual bool OnMouseDown(int x, int y, bool shiftIsDown);

	virtual void SelectionSetChanged(OdSelectionSet* selectionSet);

	virtual OdGiDrawablePtr CloneEntity(OdDbStub* id) = 0;

	virtual OdGiDrawablePtr OpenObject(OdDbStub* id, bool isForWriteMode = false) = 0;
	// alternative to OdDbGripPointsPE methods possible
	// TODO instead next methods (redesign OdDbGripPointsPE to use OdGiDrawable & use some base class of it instead OdDgGripPointsPE)
	virtual OdResult GetGripPointsAtSubentPath(OdGiDrawable* entity, const OdDbBaseFullSubentPath& path, OdDbGripDataPtrArray& grips, double currentViewUnitSize, int gripSize, const OdGeVector3d& currentViewDirection, unsigned long bitFlags) const = 0;

	virtual OdResult GetGripPoints(OdGiDrawable* entity, OdDbGripDataPtrArray& grips, double currentViewUnitSize, int gripSize, const OdGeVector3d& currentViewDirection, int bitFlags) const = 0;

	virtual OdResult GetGripPoints(OdGiDrawable* entity, OdGePoint3dArray& gripPoints) const = 0;

	virtual OdResult MoveGripPointsAtSubentPaths(OdGiDrawable* entity, const OdDbBaseFullSubentPathArray& paths, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, unsigned long bitFlags) = 0;

	virtual OdResult MoveGripPointsAt(OdGiDrawable* entity, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, int bitFlags) = 0;

	virtual OdResult MoveGripPointsAt(OdGiDrawable* entity, const OdIntArray& indices, const OdGeVector3d& offset) = 0;

	virtual void SubentGripStatus(OdGiDrawable* entity, OdDb::GripStat status, const OdDbBaseFullSubentPath& subentity) = 0;

	virtual void GripStatus(OdGiDrawable* entity, OdDb::GripStat st) = 0;

	virtual void DragStatus(OdGiDrawable* entity, OdDb::DragStat st) = 0;

	virtual bool IsModel(OdGiDrawable* /*entity*/) noexcept {
		return true;
	}
	// Hover control.
	OdResult StartHover(int x, int y, bool shiftIsDown);

	bool EndHover();
	// Grip selection.
	void UpdateEntityGrips(OdDbStub* id);

	void RemoveEntityGrips(OdDbStub* id, bool fireDone);

	void UpdateInvisibleGrips();

	virtual void DraggingStarted() = 0;

	virtual void DraggingStopped() = 0;
	//bool handleMappedRtClk( OdExGripDataPtrArray &aActiveKeys, int x, int y );
	void LocateGripsAt(int x, int y, OdExGripDataPtrArray& aResult);

	void LocateGripsByStatus(OdDbGripOperations::DrawType eStatus, OdExGripDataPtrArray& aResult);

protected:
	// Adds/Removes drawables to/from viewports.
	virtual void ShowGrip(OdExGripData* grip, bool model) = 0;

	virtual void HideGrip(OdExGripData* grip, bool model) = 0;

	[[nodiscard]] virtual OdGsView* ActiveGsView() const = 0;

	[[nodiscard]] virtual OdDbStub* ActiveViewportId() const;

	[[nodiscard]] double ActiveViewUnitSize() const;

	[[nodiscard]] OdGeVector3d ActiveViewDirection() const;

	[[nodiscard]] virtual OdGePoint3d EyeToUcsPlane(const OdGePoint3d& pPoint, const OdGePoint3d& pBasePoint) const = 0;

	virtual OdGsModel* GetGsModel() noexcept {
		return nullptr;
	}

	virtual OdGsLayoutHelper* GetGsLayoutHelper() noexcept {
		return nullptr;
	}

	bool m_Disabled {true};

	virtual void Disable(bool disable);

public:
	[[nodiscard]] bool IsDisabled() const noexcept {
		return m_Disabled;
	}

	struct OdExGripDataSubent {
		OdDbBaseFullSubentPath subentPath;
		OdExGripDataPtrArray subData;
	};

	struct OdExGripDataExt {
		OdExGripDataPtrArray dataArray;
		OdArray<OdExGripDataSubent> gripDataSubEntity;
	};

	using GripDataMap = std::map<OdDbStub*, OdExGripDataExt>;
protected:
	GripDataMap m_GripData;
	OdExGripDataPtrArray m_HoverGripsData;
	clock_t m_ClockStartHover {};
	OdGePoint3d m_BasePoint {OdGePoint3d::kOrigin};
	OdGePoint3d m_LastPoint {OdGePoint3d::kOrigin};
	OdExGripDragPtrArray m_GripDrags;
	int m_GripSize {5};
	int m_GripObjectLimit {100};
	OdCmEntityColor m_GripColor;
	OdCmEntityColor m_GripHoverColor;
	OdCmEntityColor m_GripHotColor;
	// Selection set.
	[[nodiscard]] virtual OdSelectionSetPtr WorkingSelectionSet() const = 0;

	friend class OdExGripData;
	friend class OdExGripDrag;
};
