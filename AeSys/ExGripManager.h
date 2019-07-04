// From Examples\Editor\ExGripManager.h  (last compare 20.5)
#pragma once

#include <DbGsManager.h>
#include <DbDatabaseReactor.h>
#include <DbGrip.h>
#include <DbEntity.h>
#include <DbUserIO.h>
#define STL_USING_MAP
#include <OdaSTL.h>
#include <Ed/EdCommandStack.h>
#include <Gi/GiDrawableImpl.h>
#include "OdExGripDrag.h" 
#include "OdExGripData.h"

class OdBaseGripManager;

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
	virtual OdResult GetGripPointsAtSubentPath(OdGiDrawable* entity, const OdDbBaseFullSubentPath& path, OdDbGripDataPtrArray& grips, double curViewUnitSize, int gripSize, const OdGeVector3d& curViewDir, unsigned long bitFlags) const = 0;

	virtual OdResult GetGripPoints(OdGiDrawable* entity, OdDbGripDataPtrArray& grips, double curViewUnitSize, int gripSize, const OdGeVector3d& curViewDir, int bitFlags) const = 0;

	virtual OdResult GetGripPoints(OdGiDrawable* entity, OdGePoint3dArray& gripPoints) const = 0;

	virtual OdResult MoveGripPointsAtSubentPaths(OdGiDrawable* entity, const OdDbBaseFullSubentPathArray& paths, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, unsigned long bitFlags) = 0;

	virtual OdResult MoveGripPointsAt(OdGiDrawable* entity, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, int bitFlags) = 0;

	virtual OdResult MoveGripPointsAt(OdGiDrawable* entity, const OdIntArray& indices, const OdGeVector3d& offset) = 0;

	virtual void SubentGripStatus(OdGiDrawable* entity, OdDb::GripStat status, const OdDbBaseFullSubentPath& subentity) = 0;

	virtual void GripStatus(OdGiDrawable* entity, OdDb::GripStat st) = 0;

	virtual void DragStatus(OdGiDrawable* entity, OdDb::DragStat st) = 0;

	virtual bool IsModel(OdGiDrawable* /*entity*/) noexcept { return true; }

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

	virtual OdGsModel* GetGsModel() noexcept { return nullptr; }

	virtual OdGsLayoutHelper* GetGsLayoutHelper() noexcept { return nullptr; }

	bool m_Disabled {true};

	virtual void Disable(bool disable);

public:
	[[nodiscard]] bool IsDisabled() const noexcept { return m_Disabled; }

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

///////////////////////////////////////////////////////////////////////////////
struct OdExGripDbReactor : OdDbDatabaseReactor {
	OdExGripDbReactor() = default;

	void objectAppended(const OdDbDatabase* database, const OdDbObject* dbObject) noexcept override;

	void objectModified(const OdDbDatabase* database, const OdDbObject* object) override;

	void objectErased(const OdDbDatabase* database, const OdDbObject* object, bool erased = true) override;

	class OdExGripManager* gripManager {nullptr};
};

using OdExGripDbReactorPtr = OdSmartPtr<OdExGripDbReactor>;
class OdDbCommandContext;

typedef OdDbSelectionSetPtr (*GetSelectionSetPtr)(OdDbCommandContext* commandContext);

class OdExGripManager : public OdBaseGripManager {
public:
	OdExGripManager() noexcept;

	~OdExGripManager() = default;

	void Initialize(OdGsDevice* device, OdGsModel* gsModel, OdDbCommandContext* commandContext, GetSelectionSetPtr getSSet);

	void Uninitialize();

	// OdEdPointTracker protocol
	int addDrawables(OdGsView* view) override;

	void removeDrawables(OdGsView* view) override;

	void OnModified(OdGiDrawable* grip) override;

	OdGiDrawablePtr CloneEntity(OdDbStub* id) override;

	OdGiDrawablePtr OpenObject(OdDbStub* id, bool isForWriteMode = false) override;

	OdResult GetGripPointsAtSubentPath(OdGiDrawable* entity, const OdDbBaseFullSubentPath& path, OdDbGripDataPtrArray& grips, double curViewUnitSize, int gripSize, const OdGeVector3d& curViewDir, unsigned long bitFlags) const override;

	OdResult GetGripPoints(OdGiDrawable* entity, OdDbGripDataPtrArray& grips, double curViewUnitSize, int gripSize, const OdGeVector3d& curViewDir, int bitFlags) const override;

	OdResult GetGripPoints(OdGiDrawable* entity, OdGePoint3dArray& gripPoints) const override;

	OdResult MoveGripPointsAtSubentPaths(OdGiDrawable* entity, const OdDbBaseFullSubentPathArray& paths, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, unsigned long bitFlags) override;

	OdResult MoveGripPointsAt(OdGiDrawable* entity, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, int bitFlags) override;

	OdResult MoveGripPointsAt(OdGiDrawable* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;

	void SubentGripStatus(OdGiDrawable* entity, OdDb::GripStat status, const OdDbBaseFullSubentPath& subentity) override;

	void GripStatus(OdGiDrawable* entity, OdDb::GripStat status) override;

	void DragStatus(OdGiDrawable* entity, OdDb::DragStat status) override;

	bool IsModel(OdGiDrawable* entity) noexcept override;

	// Events from Windows.
	bool OnMouseDown(int x, int y, bool shiftIsDown) override;

	bool OnMouseMove(int x, int y, bool shiftIsDown);

	[[nodiscard]] bool OnControlClick() const;

	void DraggingStarted() override;

	void DraggingStopped() override;

private:
	bool HandleMappedRtClk(OdExGripDataPtrArray& activeKeys, int x, int y);
	
	void AddToDrag(OdExGripDataPtrArray& activeKeys);
	
	// Adds/Removes drawables to/from viewports.
	void ShowGrip(OdExGripData* gripData, bool model) override;

	void HideGrip(OdExGripData* gripData, bool model) override;

	[[nodiscard]] OdGsView* ActiveGsView() const override;

	[[nodiscard]] OdGePoint3d EyeToUcsPlane(const OdGePoint3d& point, const OdGePoint3d& basePoint) const override;

	OdGsLayoutHelperPtr m_LayoutHelper {};
	OdDbCommandContext* m_CommandContext {nullptr};
	OdGsModel* m_GsModel {nullptr};

	OdGsModel* GetGsModel() noexcept override { return m_GsModel; }

	OdGsLayoutHelper* GetGsLayoutHelper() noexcept override { return m_LayoutHelper.get(); }

	void Disable(bool disable) override;

	OdStaticRxObject<OdExGripDbReactor> m_DbReactor;

	// Selection set.
	[[nodiscard]] OdSelectionSetPtr WorkingSelectionSet() const override;

	GetSelectionSetPtr m_GetSelectionSet {nullptr};

	struct OdExGripCommand : OdEdCommand {
		OdExGripManager* parent {nullptr};

		const OdString groupName() const override { return L"EDIT"; }

		const OdString globalName() const override { return L"GROUP_STRETCH"; }

		void execute(OdEdCommandContext* edCommandContext) override;
	};

	OdStaticRxObject<OdExGripCommand> m_GripStretchCommand;
	friend struct OdExGripCommand;
};
