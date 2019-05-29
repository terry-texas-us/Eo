#pragma once

// From Examples\Editor\ExGripManager.h  (last compare 19.12)

#include "DbGsManager.h"
#include "DbDatabaseReactor.h"
#include "StaticRxObject.h"
#include "DbGrip.h"
#include "DbEntity.h"
#include "DbUserIO.h"
#define STL_USING_MAP
#include "OdaSTL.h"
#include "Ed/EdCommandStack.h"
#include "Gi/GiDrawableImpl.h"

class OdBaseGripManager;
class OdExGripData;
typedef OdSmartPtr<OdExGripData> OdExGripDataPtr;

class OdExGripData : public OdGiDrawableImpl<> {
public:
	OdExGripData() noexcept;
	~OdExGripData();
	static OdExGripDataPtr createObject(OdDbStub* id, OdDbGripDataPtr gripData, const OdGePoint3d& point, OdBaseGripManager* owner);
	static OdExGripDataPtr createObject(OdDbBaseFullSubentPath entPath, OdDbGripDataPtr gripData, const OdGePoint3d& point, OdBaseGripManager* owner);

	OdUInt32 subSetAttributes(OdGiDrawableTraits* drawableTraits) const override;
	bool subWorldDraw(OdGiWorldDraw* worldDraw) const override;
	void subViewportDraw(OdGiViewportDraw* viewportDraw) const override;

	OdDbGripOperations::DrawType status() const noexcept { return m_Status; }
	bool isInvisible() const noexcept { return m_Invisible; }
	bool isShared() const noexcept { return m_Shared; }
	OdGePoint3d point() const noexcept { return m_Point; }
	OdDbGripDataPtr GripData() const { return m_GripData; }
	OdDbStub* entityId() const { return m_entPath.objectIds().last(); }
	
	bool entPath(OdDbBaseFullSubentPath* path = nullptr) const {
		
		if (path) { *path = m_entPath; }
		
		return m_entPath.subentId() != OdDbSubentId();
	}
	void setStatus(OdDbGripOperations::DrawType status) noexcept { m_Status = status; }
	void setInvisible(bool invisible) noexcept { m_Invisible = invisible; }
	void setShared(bool shared) noexcept { m_Shared = shared; }

private:
	bool computeDragPoint(OdGePoint3d& ptOverride) const;

	OdDbGripOperations::DrawType m_Status;
	bool m_Invisible;
	bool m_Shared;
	OdGePoint3d m_Point;
	OdDbGripDataPtr m_GripData;
	OdDbBaseFullSubentPath m_entPath;
	OdBaseGripManager* m_pOwner;
};

typedef OdArray<OdExGripDataPtr> OdExGripDataPtrArray;

typedef OdSmartPtr<class OdExGripDrag> OdExGripDragPtr;

class OdExGripDrag : public OdGiDrawableImpl<> {
public:
	OdExGripDrag() noexcept;
	~OdExGripDrag();
	static OdExGripDragPtr createObject(OdDbStub* id, OdBaseGripManager* owner);
	static OdExGripDragPtr createObject(OdDbBaseFullSubentPath entPath, OdBaseGripManager* owner);

	OdUInt32 subSetAttributes(OdGiDrawableTraits* drawableTraits) const override;
	bool subWorldDraw(OdGiWorldDraw* worldDraw) const override;
	void subViewportDraw(OdGiViewportDraw* viewportDraw) const override;

	void CloneEntity();
	void CloneEntity(const OdGePoint3d& ptMoveAt);
	void moveEntity(const OdGePoint3d& ptMoveAt);
	void notifyDragStarted();
	void notifyDragEnded();
	void notifyDragAborted();

	OdDbStub* entityId() const;
	bool entPath(OdDbBaseFullSubentPath* path = nullptr) const;

protected:
	bool locateActiveGrips(OdIntArray& aIndices);

	OdDbBaseFullSubentPath m_entPath;
	OdGiDrawablePtr m_pClone;
	OdBaseGripManager* m_pOwner;
};

typedef OdArray<OdExGripDragPtr> OdExGripDragPtrArray;

class OdBaseGripManager : public OdEdPointTracker {

public: // Construction. Initialization.
	OdBaseGripManager() noexcept;
	~OdBaseGripManager();

	// OdEdPointTracker protocol
	void setValue(const OdGePoint3d& ptValue) override;

	// Events from Windows.
	virtual void OnModified(OdGiDrawable* pGrip) = 0;
	virtual bool OnMouseDown(int x, int y, bool shiftIsDown);

	virtual void SelectionSetChanged(OdSelectionSet* selectionSet);

	virtual OdGiDrawablePtr CloneEntity(OdDbStub* id) = 0;
	virtual OdGiDrawablePtr OpenObject(OdDbStub* id, bool isForWriteMode = false) = 0;
	// alternative to OdDbGripPointsPE methods
	// possible TODO instead next methods
	// (redesign OdDbGripPointsPE to use OdGiDrawable & use some base class of it instead OdDgGripPointsPE) 
	virtual OdResult GetGripPointsAtSubentPath(OdGiDrawable* entity, const OdDbBaseFullSubentPath& path, OdDbGripDataPtrArray& grips, double curViewUnitSize, int gripSize, const OdGeVector3d& curViewDir, const OdUInt32 bitflags) const = 0;
	virtual OdResult GetGripPoints(OdGiDrawable* entity, OdDbGripDataPtrArray& grips, double curViewUnitSize, int gripSize, const OdGeVector3d& curViewDir, int bitFlags) const = 0;
	virtual OdResult GetGripPoints(OdGiDrawable* entity, OdGePoint3dArray& gripPoints) const = 0;
	virtual OdResult MoveGripPointsAtSubentPaths(OdGiDrawable* entity, const OdDbBaseFullSubentPathArray& paths, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, OdUInt32 bitflags) = 0;
	virtual OdResult MoveGripPointsAt(OdGiDrawable* entity, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, int bitFlags) = 0;
	virtual OdResult MoveGripPointsAt(OdGiDrawable* entity, const OdIntArray& indices, const OdGeVector3d& offset) = 0;
	virtual void SubentGripStatus(OdGiDrawable* entity, OdDb::GripStat status, const OdDbBaseFullSubentPath& subentity) = 0;
	virtual void GripStatus(OdGiDrawable* entity, OdDb::GripStat st) = 0;
	virtual void DragStatus(OdGiDrawable* entity, OdDb::DragStat st) = 0;
	virtual bool IsModel(OdGiDrawable*) noexcept { return true; }

	// Hover control.
	bool StartHover(int x, int y);
	bool EndHover();

	// Grip selection.
	void UpdateEntityGrips(OdDbStub* id);
	void RemoveEntityGrips(OdDbStub* id, bool bFireDone);
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

	virtual OdGsView* ActiveGsView() const = 0;
	virtual OdDbStub* ActiveViewportId() const;
	double ActiveViewUnitSize() const;
	OdGeVector3d ActiveViewDirection() const;
	virtual OdGePoint3d EyeToUcsPlane(const OdGePoint3d& pPoint, const OdGePoint3d& pBasePoint) const = 0;

	virtual OdGsModel* GetGsModel() noexcept { return nullptr; }
	virtual OdGsLayoutHelper* GetGsLayoutHelper() noexcept { return nullptr; }

	bool m_Disabled;
	virtual void Disable(bool disable) noexcept;

public:
	bool IsDisabled() noexcept { return m_Disabled; }

	struct OdExGripDataSubent {
		OdDbBaseFullSubentPath m_entPath;
		OdExGripDataPtrArray m_pSubData;
	};
	struct OdExGripDataExt {
		OdExGripDataPtrArray m_pDataArray;
		OdArray<OdExGripDataSubent> m_GripDataSubEntity;
	};
	typedef std::map<OdDbStub*, OdExGripDataExt> GripDataMap;
	GripDataMap m_GripData;
protected:

	OdExGripDataPtrArray m_HoverGripsData;

	OdGePoint3d m_BasePoint;
	OdGePoint3d m_LastPoint;
	OdExGripDragPtrArray m_GripDrags;

	int m_GRIPSIZE;
	int m_GRIPOBJLIMIT;
	OdCmEntityColor m_GRIPCOLOR;
	OdCmEntityColor m_GRIPHOVER;
	OdCmEntityColor m_GRIPHOT;

	// Selection set.
	virtual OdSelectionSetPtr WorkingSelectionSet() const = 0;
	friend class OdExGripData;
	friend class OdExGripDrag;
};

///////////////////////////////////////////////////////////////////////////////

class OdExGripDbReactor : public OdDbDatabaseReactor {
public:
	OdExGripDbReactor() noexcept;
	void objectAppended(const OdDbDatabase* pDb, const OdDbObject* pDbObj) noexcept override;
	void objectModified(const OdDbDatabase* pDb, const OdDbObject* pDbObj) override;
	void objectErased(const OdDbDatabase* pDb, const OdDbObject* pDbObj, bool pErased = true) override;

public:
	class OdExGripManager* m_pOwner;
};

typedef OdSmartPtr<OdExGripDbReactor> OdExGripDbReactorPtr;

class OdDbCommandContext;
typedef OdDbSelectionSetPtr(*GetSelectionSetPtr)(OdDbCommandContext* dbCommandContext);

typedef OdDbSelectionSetPtr(*GetSelectionSetPtr)(OdDbCommandContext* dbCommandContext);

class OdExGripManager : public OdBaseGripManager {

public: // Construction. Initialization.
	OdExGripManager() noexcept;
	~OdExGripManager();
	void Initialize(OdGsDevice* device, OdGsModel* gsModel, OdDbCommandContext* dbCommandContext, GetSelectionSetPtr getSSet);
	void Uninitialize();

	// OdEdPointTracker protocol
	int addDrawables(OdGsView* view) override;
	void removeDrawables(OdGsView* view) override;

	void OnModified(OdGiDrawable* pGrip) override;
	OdGiDrawablePtr CloneEntity(OdDbStub* id) override;
	OdGiDrawablePtr OpenObject(OdDbStub* id, bool isForWriteMode = false) override;
	OdResult GetGripPointsAtSubentPath(OdGiDrawable* entity, const OdDbBaseFullSubentPath& path, OdDbGripDataPtrArray& grips, double curViewUnitSize, int gripSize, const OdGeVector3d& curViewDir, const OdUInt32 bitflags) const override;
	OdResult GetGripPoints(OdGiDrawable* entity, OdDbGripDataPtrArray& grips, double curViewUnitSize, int gripSize, const OdGeVector3d& curViewDir, int bitFlags) const override;
	OdResult GetGripPoints(OdGiDrawable* entity, OdGePoint3dArray& gripPoints) const override;
	OdResult MoveGripPointsAtSubentPaths(OdGiDrawable* entity, const OdDbBaseFullSubentPathArray& paths, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, OdUInt32 bitflags) override;
	OdResult MoveGripPointsAt(OdGiDrawable* entity, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, int bitFlags) override;
	OdResult MoveGripPointsAt(OdGiDrawable* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;
	void SubentGripStatus(OdGiDrawable* entity, OdDb::GripStat status, const OdDbBaseFullSubentPath& subentity) override;
	void GripStatus(OdGiDrawable* entity, OdDb::GripStat st) override;
	void DragStatus(OdGiDrawable* entity, OdDb::DragStat st) override;
	bool IsModel(OdGiDrawable* entity) noexcept override;

	// Events from Windows.
	bool OnMouseDown(int x, int y, bool shiftIsDown) override;
	bool OnMouseMove(int x, int y);
	bool onControlClick();

	void DraggingStarted() override;
	void DraggingStopped() override;
private:
	bool handleMappedRtClk(OdExGripDataPtrArray& aActiveKeys, int x, int y);

   // Adds/Removes drawables to/from viewports.
	void ShowGrip(OdExGripData* pGrip, bool bModel) override;
	void HideGrip(OdExGripData* grip, bool model) override;

	OdGsView* ActiveGsView() const override;

	OdGePoint3d EyeToUcsPlane(const OdGePoint3d& pPoint, const OdGePoint3d& pBasePoint) const override;

	OdGsLayoutHelperPtr m_LayoutHelper;
	OdDbCommandContext* m_CommandContext;
	OdGsModel* m_pGsModel;
	OdGsModel* GetGsModel() noexcept override { return m_pGsModel; }
	OdGsLayoutHelper* GetGsLayoutHelper() noexcept override { return m_LayoutHelper.get(); }
	void Disable(bool disable) noexcept override;
private:
	OdStaticRxObject<OdExGripDbReactor> m_cDbReactor;

	// Selection set.
	OdSelectionSetPtr WorkingSelectionSet() const override;
	GetSelectionSetPtr m_pGetSelectionSetPtr;
	struct OdExGripCommand : OdEdCommand {
		OdExGripManager* m_parent;
		const OdString groupName() const override { return L"EDIT"; }
		const OdString globalName() const override { return L"GROUP_STRETCH"; }
		void execute(OdEdCommandContext* edCommandContext) override;
	};
	OdStaticRxObject<OdExGripCommand> m_gripStretchCommand;
	friend struct OdExGripCommand;
};
