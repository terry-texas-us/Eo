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
	OdGePoint3d point() const noexcept { return m_point; }
	OdDbGripDataPtr GripData() const { return m_GripData; }
	OdDbStub* entityId() const { return m_entPath.objectIds().last(); }
	
	bool entPath(OdDbBaseFullSubentPath* pPath = NULL) const {
		if (pPath)
			* pPath = m_entPath;
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
	OdGePoint3d m_point;
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

	void cloneEntity();
	void cloneEntity(const OdGePoint3d& ptMoveAt);
	void moveEntity(const OdGePoint3d& ptMoveAt);
	void notifyDragStarted();
	void notifyDragEnded();
	void notifyDragAborted();

	OdDbStub* entityId() const;
	bool entPath(OdDbBaseFullSubentPath* pPath = NULL) const;

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
	virtual void onModified(OdGiDrawable* pGrip) = 0;
	virtual bool onMouseDown(int x, int y, bool bShift);

	virtual void selectionSetChanged(OdSelectionSet* selectionSet);

	virtual OdGiDrawablePtr cloneEntity(OdDbStub* id) = 0;
	virtual OdGiDrawablePtr openObject(OdDbStub* id, bool isForWriteMode = false) = 0;
	// alternative to OdDbGripPointsPE methods
	// possible TODO instead next methods
	// (redesign OdDbGripPointsPE to use OdGiDrawable & use some base class of it instead OdDgGripPointsPE) 
	virtual OdResult getGripPointsAtSubentPath(OdGiDrawable* entity, const OdDbBaseFullSubentPath& path, OdDbGripDataPtrArray& grips, double curViewUnitSize, int gripSize, const OdGeVector3d& curViewDir, const OdUInt32 bitflags) const = 0;
	virtual OdResult getGripPoints(OdGiDrawable* entity, OdDbGripDataPtrArray& grips, double curViewUnitSize, int gripSize, const OdGeVector3d& curViewDir, int bitFlags) const = 0;
	virtual OdResult getGripPoints(OdGiDrawable* entity, OdGePoint3dArray& gripPoints) const = 0;
	virtual OdResult moveGripPointsAtSubentPaths(OdGiDrawable* entity, const OdDbBaseFullSubentPathArray& paths, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, OdUInt32 bitflags) = 0;
	virtual OdResult moveGripPointsAt(OdGiDrawable* entity, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, int bitFlags) = 0;
	virtual OdResult moveGripPointsAt(OdGiDrawable* entity, const OdIntArray& indices, const OdGeVector3d& offset) = 0;
	virtual void subentGripStatus(OdGiDrawable* entity, OdDb::GripStat status, const OdDbBaseFullSubentPath& subentity) = 0;
	virtual void gripStatus(OdGiDrawable* entity, OdDb::GripStat st) = 0;
	virtual void dragStatus(OdGiDrawable* entity, OdDb::DragStat st) = 0;
	virtual bool isModel(OdGiDrawable*) noexcept { return true; }

	// Hover control.
	bool startHover(int x, int y);
	bool endHover();

	// Grip selection.
	void updateEntityGrips(OdDbStub* id);
	void removeEntityGrips(OdDbStub* id, bool bFireDone);
	void updateInvisibleGrips();

	// Dragging.
	virtual void draggingStarted() = 0;
	virtual void draggingStopped() = 0;

	//bool handleMappedRtClk( OdExGripDataPtrArray &aActiveKeys, int x, int y );

	void locateGripsAt(int x, int y, OdExGripDataPtrArray& aResult);
	void locateGripsByStatus(OdDbGripOperations::DrawType eStatus, OdExGripDataPtrArray& aResult);
protected:
  // Adds/Removes drawables to/from viewports.
	virtual void showGrip(OdExGripData* pGrip, bool bModel) = 0;
	virtual void hideGrip(OdExGripData* pGrip, bool bModel) = 0;

	virtual OdGsView* activeGsView() const = 0;
	virtual OdDbStub* activeVpId() const;
	double activeViewUnitSize() const;
	OdGeVector3d activeViewDirection() const;
	virtual OdGePoint3d eyeToUcsPlane(const OdGePoint3d& pPoint, const OdGePoint3d& pBasePoint) const = 0;

	virtual OdGsModel* getGsModel() noexcept { return NULL; }
	virtual OdGsLayoutHelper* getGsLayoutHelper() noexcept { return NULL; }

	bool m_Disabled;
	virtual void disable(bool disable) noexcept;

public:
	bool isDisabled() noexcept { return m_Disabled; }

	struct OdExGripDataSubent {
		OdDbBaseFullSubentPath m_entPath;
		OdExGripDataPtrArray m_pSubData;
	};
	struct OdExGripDataExt {
		OdExGripDataPtrArray m_pDataArray;
		OdArray<OdExGripDataSubent> m_GripDataSubEntity;
	};
	//
	typedef std::map<OdDbStub*, OdExGripDataExt> GripDataMap;
	GripDataMap m_aGripData;
protected:

	OdExGripDataPtrArray m_aHoverGrips;

	OdGePoint3d m_ptBasePoint;
	OdGePoint3d m_ptLastPoint;
	OdExGripDragPtrArray m_aDrags;

	int m_GRIPSIZE;
	int m_GRIPOBJLIMIT;
	OdCmEntityColor m_GRIPCOLOR;
	OdCmEntityColor m_GRIPHOVER;
	OdCmEntityColor m_GRIPHOT;

	// Selection set.
	virtual OdSelectionSetPtr workingSelectionSet() const = 0;
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
	void init(OdGsDevice* pDevice, OdGsModel* pGsModel, OdDbCommandContext* dbCommandContext, GetSelectionSetPtr getSSet);
	void uninit();

	// OdEdPointTracker protocol
	int addDrawables(OdGsView* pView) override;
	void removeDrawables(OdGsView* pView) override;

	void onModified(OdGiDrawable* pGrip) override;

	OdGiDrawablePtr cloneEntity(OdDbStub* id) override;
	OdGiDrawablePtr openObject(OdDbStub* id, bool isForWriteMode = false) override;
	OdResult getGripPointsAtSubentPath(OdGiDrawable* entity, const OdDbBaseFullSubentPath& path, OdDbGripDataPtrArray& grips, double curViewUnitSize, int gripSize, const OdGeVector3d& curViewDir, const OdUInt32 bitflags) const override;
	OdResult getGripPoints(OdGiDrawable* entity, OdDbGripDataPtrArray& grips, double curViewUnitSize, int gripSize, const OdGeVector3d& curViewDir, int bitFlags) const override;
	OdResult getGripPoints(OdGiDrawable* entity, OdGePoint3dArray& gripPoints) const override;
	OdResult moveGripPointsAtSubentPaths(OdGiDrawable* entity, const OdDbBaseFullSubentPathArray& paths, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, OdUInt32 bitflags) override;
	OdResult moveGripPointsAt(OdGiDrawable* entity, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, int bitFlags) override;
	OdResult moveGripPointsAt(OdGiDrawable* entity, const OdIntArray& indices, const OdGeVector3d& offset) override;
	void subentGripStatus(OdGiDrawable* entity, OdDb::GripStat status, const OdDbBaseFullSubentPath& subentity) override;
	void gripStatus(OdGiDrawable* entity, OdDb::GripStat st) override;
	void dragStatus(OdGiDrawable* entity, OdDb::DragStat st) override;
	bool isModel(OdGiDrawable* entity) noexcept override;

	// Events from Windows.
	bool onMouseDown(int x, int y, bool bShift) override;
	bool onMouseMove(int x, int y);
	bool onControlClick();

	// Dragging.
	void draggingStarted() override;
	void draggingStopped() override;
private:
	bool handleMappedRtClk(OdExGripDataPtrArray& aActiveKeys, int x, int y);

   // Adds/Removes drawables to/from viewports.
	void showGrip(OdExGripData* pGrip, bool bModel) override;
	void hideGrip(OdExGripData* pGrip, bool bModel) override;

	OdGsView* activeGsView() const override;

	OdGePoint3d eyeToUcsPlane(const OdGePoint3d& pPoint, const OdGePoint3d& pBasePoint) const override;

	OdGsLayoutHelperPtr m_pDevice;
	OdDbCommandContext* m_pCmdCtx;
	OdGsModel* m_pGsModel;
	OdGsModel* getGsModel() noexcept override { return m_pGsModel; }
	OdGsLayoutHelper* getGsLayoutHelper() noexcept override { return m_pDevice.get(); }
	void disable(bool disable) noexcept override;
private:
	OdStaticRxObject<OdExGripDbReactor> m_cDbReactor;

	// Selection set.
	OdSelectionSetPtr workingSelectionSet() const override;
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
