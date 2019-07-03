#pragma once

// From Examples\Editor\ExGripManager.h  (last compare 20.5)
#include <DbGsManager.h>
#include <DbDatabaseReactor.h>
#include <DbGrip.h>
#include <DbEntity.h>
#include <DbUserIO.h>
#define STL_USING_MAP
#include <OdaSTL.h>
#include <Ed/EdCommandStack.h>
#include <Gi/GiDrawableImpl.h>
class OdBaseGripManager;
class OdExGripData;
using OdExGripDataPtr = OdSmartPtr<OdExGripData>;

class OdExGripData : public OdGiDrawableImpl<> {
public:
	OdExGripData() noexcept;

	~OdExGripData();

	static OdExGripDataPtr CreateObject(OdDbStub* id, OdDbGripDataPtr gripData, const OdGePoint3d& point, OdBaseGripManager* gripManager);

	static OdExGripDataPtr CreateObject(OdDbBaseFullSubentPath entityPath, OdDbGripDataPtr gripData, const OdGePoint3d& point, OdBaseGripManager* gripManager);

	unsigned long subSetAttributes(OdGiDrawableTraits* drawableTraits) const override;

	bool subWorldDraw(OdGiWorldDraw* worldDraw) const override;

	void subViewportDraw(OdGiViewportDraw* viewportDraw) const override;

	[[nodiscard]] OdDbGripOperations::DrawType Status() const noexcept { return m_Status; }

	[[nodiscard]] bool IsInvisible() const noexcept { return m_Invisible; }

	[[nodiscard]] bool IsShared() const noexcept { return m_Shared; }

	[[nodiscard]] OdGePoint3d Point() const noexcept { return m_Point; }

	[[nodiscard]] OdDbGripDataPtr GripData() const { return m_GripData; }

	[[nodiscard]] OdDbStub* EntityId() const { return m_SubentPath.objectIds().last(); }

	bool EntityPath(OdDbBaseFullSubentPath* path = nullptr) const {
		if (path) { *path = m_SubentPath; }
		return m_SubentPath.subentId() != OdDbSubentId();
	}

	void SetStatus(const OdDbGripOperations::DrawType status) noexcept { m_Status = status; }

	void SetInvisible(const bool invisible) noexcept { m_Invisible = invisible; }

	void SetShared(const bool shared) noexcept { m_Shared = shared; }

private:
	bool ComputeDragPoint(OdGePoint3d& computedPoint) const;

	OdDbGripOperations::DrawType m_Status;
	bool m_Invisible;
	bool m_Shared;
	OdGePoint3d m_Point;
	OdDbGripDataPtr m_GripData;
	OdDbBaseFullSubentPath m_SubentPath;
	OdBaseGripManager* m_GripManager;
};

using OdExGripDataPtrArray = OdArray<OdExGripDataPtr>;
using OdExGripDragPtr = OdSmartPtr<class OdExGripDrag>;

class OdExGripDrag : public OdGiDrawableImpl<> {
public:
	OdExGripDrag() noexcept;

	~OdExGripDrag() = default;

	static OdExGripDragPtr CreateObject(OdDbStub* id, OdBaseGripManager* gripManager);

	static OdExGripDragPtr CreateObject(OdDbBaseFullSubentPath entityPath, OdBaseGripManager* gripManager);

	unsigned long subSetAttributes(OdGiDrawableTraits* drawableTraits) const override;

	bool subWorldDraw(OdGiWorldDraw* worldDraw) const override;

	void subViewportDraw(OdGiViewportDraw* viewportDraw) const override;

	void CloneEntity();

	void CloneEntity(const OdGePoint3d& ptMoveAt);

	void MoveEntity(const OdGePoint3d& ptMoveAt);

	void NotifyDragStarted();

	void NotifyDragEnded();

	void NotifyDragAborted();

	[[nodiscard]] OdDbStub* EntityId() const;

	bool EntityPath(OdDbBaseFullSubentPath* subentPath = nullptr) const;

protected:
	bool LocateActiveGrips(OdIntArray& indices);

	OdDbBaseFullSubentPath m_SubentPath;
	OdGiDrawablePtr m_Clone;
	OdBaseGripManager* m_GripManager;
};

using OdExGripDragPtrArray = OdArray<OdExGripDragPtr>;

class OdBaseGripManager : public OdEdPointTracker {
public: // Construction. Initialization.
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

	bool m_Disabled;

	virtual void Disable(bool disable);

public:
	[[nodiscard]] bool IsDisabled() const noexcept { return m_Disabled; }

	struct OdExGripDataSubent {
		OdDbBaseFullSubentPath subentPath;
		OdExGripDataPtrArray m_pSubData;
	};

	struct OdExGripDataExt {
		OdExGripDataPtrArray m_pDataArray;
		OdArray<OdExGripDataSubent> m_GripDataSubEntity;
	};

	using GripDataMap = std::map<OdDbStub*, OdExGripDataExt>;
	GripDataMap m_GripData;
protected:
	OdExGripDataPtrArray m_HoverGripsData;
	clock_t clockStartHover;
	OdGePoint3d m_BasePoint;
	OdGePoint3d m_LastPoint;
	OdExGripDragPtrArray m_GripDrags;
	int m_GRIPSIZE;
	int m_GRIPOBJLIMIT;
	OdCmEntityColor m_GRIPCOLOR;
	OdCmEntityColor m_GRIPHOVER;
	OdCmEntityColor m_GRIPHOT;

	// Selection set.
	[[nodiscard]] virtual OdSelectionSetPtr WorkingSelectionSet() const = 0;

	friend class OdExGripData;
	friend class OdExGripDrag;
};

///////////////////////////////////////////////////////////////////////////////
class OdExGripDbReactor : public OdDbDatabaseReactor {
public:
	OdExGripDbReactor() = default;

	void objectAppended(const OdDbDatabase* database, const OdDbObject* dbObject) noexcept override;

	void objectModified(const OdDbDatabase* database, const OdDbObject* dbObject) override;

	void objectErased(const OdDbDatabase* database, const OdDbObject* dbObject, bool erased = true) override;

	class OdExGripManager* m_GripManager {nullptr};
};

using OdExGripDbReactorPtr = OdSmartPtr<OdExGripDbReactor>;
class OdDbCommandContext;

typedef OdDbSelectionSetPtr (*GetSelectionSetPtr)(OdDbCommandContext* commandContext);

typedef OdDbSelectionSetPtr (*GetSelectionSetPtr)(OdDbCommandContext* commandContext);

class OdExGripManager : public OdBaseGripManager {
public: // Construction. Initialization.
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

	bool OnControlClick();

	void DraggingStarted() override;

	void DraggingStopped() override;

private:
	bool HandleMappedRtClk(OdExGripDataPtrArray& activeKeys, int x, int y);
	
	void AddToDrag(OdExGripDataPtrArray& aActiveKeys);
	
	// Adds/Removes drawables to/from viewports.
	void ShowGrip(OdExGripData* gripData, bool model) override;

	void HideGrip(OdExGripData* gripData, bool model) override;

	[[nodiscard]] OdGsView* ActiveGsView() const override;

	[[nodiscard]] OdGePoint3d EyeToUcsPlane(const OdGePoint3d& point, const OdGePoint3d& basePoint) const override;

	OdGsLayoutHelperPtr m_LayoutHelper;
	OdDbCommandContext* m_CommandContext {nullptr};
	OdGsModel* m_pGsModel {nullptr};

	OdGsModel* GetGsModel() noexcept override { return m_pGsModel; }

	OdGsLayoutHelper* GetGsLayoutHelper() noexcept override { return m_LayoutHelper.get(); }

	void Disable(bool disable) override;

	OdStaticRxObject<OdExGripDbReactor> m_cDbReactor;

	// Selection set.
	[[nodiscard]] OdSelectionSetPtr WorkingSelectionSet() const override;

	GetSelectionSetPtr m_pGetSelectionSetPtr;

	struct OdExGripCommand : OdEdCommand {
		OdExGripManager* m_parent {nullptr};

		const OdString groupName() const override { return L"EDIT"; }

		const OdString globalName() const override { return L"GROUP_STRETCH"; }

		void execute(OdEdCommandContext* edCommandContext) override;
	};

	OdStaticRxObject<OdExGripCommand> m_gripStretchCommand;
	friend struct OdExGripCommand;
};
