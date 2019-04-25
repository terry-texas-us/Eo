#pragma once

#include "DbGsManager.h"
#include "DbDatabaseReactor.h"
#include "StaticRxObject.h"
#include "DbEntity.h"
#include "DbGrip.h"
#include "DbUserIO.h"
#include "Gi/GiDrawableImpl.h"

class EoExGripManager;
class EoExGripData;

typedef OdSmartPtr<EoExGripData> EoExGripDataPtr;

class EoExGripData : public OdGiDrawableImpl<> {
public:
	EoExGripData();
	virtual ~EoExGripData();
	static EoExGripDataPtr createObject(OdDbObjectId id, OdDbGripDataPtr gripData, const OdGePoint3d& point,  EoExGripManager* gripManager);
	static EoExGripDataPtr createObject(OdDbFullSubentPath fullSentPath, OdDbGripDataPtr gripData, const OdGePoint3d& point,  EoExGripManager* gripManager);

	OdUInt32 subSetAttributes(OdGiDrawableTraits* traits) const override;
	bool subWorldDraw(OdGiWorldDraw* worldDraw) const override;
	void subViewportDraw(OdGiViewportDraw* viewportDraw) const override;

	OdDbGripOperations::DrawType status() const noexcept;
	bool isInvisible() const noexcept;
	bool isShared() const noexcept;
	OdGePoint3d point() const noexcept;
	OdDbGripDataPtr data() const;
	OdDbObjectId entityId() const;
	bool entPath(OdDbFullSubentPath* path = NULL) const;
	void setStatus(OdDbGripOperations::DrawType val) noexcept;
	void setInvisible(bool val) noexcept;
	void setShared(bool val) noexcept;

private:
	bool computeDragPoint(OdGePoint3d& ptOverride)  const;

	OdDbGripOperations::DrawType m_status;
	bool m_bInvisible;
	bool m_bShared;
	OdGePoint3d m_point;
	OdDbGripDataPtr m_pData;
	OdDbFullSubentPath m_entPath;
	EoExGripManager* m_pOwner;
};

typedef OdArray<EoExGripDataPtr> EoExGripDataPtrArray;

class EoExGripDrag;
typedef OdSmartPtr<EoExGripDrag> EoExGripDragPtr;

class EoExGripDrag : public OdGiDrawableImpl<> {
public:
	EoExGripDrag();
	virtual ~EoExGripDrag();
	static EoExGripDragPtr createObject(OdDbObjectId entPath, EoExGripManager* gripManager);
	static EoExGripDragPtr createObject(OdDbFullSubentPath entPath, EoExGripManager* gripManager);

	OdUInt32 subSetAttributes(OdGiDrawableTraits* traits) const override;
	bool subWorldDraw(OdGiWorldDraw* worldDraw) const override;
	void subViewportDraw(OdGiViewportDraw* viewportDraw) const override;

	void cloneEntity();
	void cloneEntity(const OdGePoint3d& ptMoveAt);
	void moveEntity(const OdGePoint3d& ptMoveAt);
	void notifyDragStarted();
	void notifyDragEnded();
	void notifyDragAborted();

	OdDbObjectId entityId() const;
	bool entPath(OdDbFullSubentPath *pPath = NULL) const;

private:
	bool locateActiveGrips(OdIntArray& aIndices);

	OdDbFullSubentPath m_entPath;
	OdDbEntityPtr m_pClone;
	EoExGripManager* m_pOwner;
};

typedef OdArray<EoExGripDragPtr> EoExGripDragPtrArray;

class EoExGripDbReactor : public OdDbDatabaseReactor {
public:
	EoExGripDbReactor();
	void objectAppended(const OdDbDatabase* pDb, const OdDbObject* pDbObj) noexcept override;
	void objectModified(const OdDbDatabase* pDb, const OdDbObject* pDbObj) override;
	void objectErased(const OdDbDatabase* pDb, const OdDbObject* pDbObj, bool pErased = true) override;

public:
	EoExGripManager* m_pOwner;
};

typedef OdSmartPtr<EoExGripDbReactor> EoExGripDbReactorPtr;

typedef OdDbSelectionSetPtr (*GetSelectionSetPtr) (OdDbCommandContext* commandContext);

class EoExGripManager : public OdEdPointTracker {
public:
	EoExGripManager();
	virtual ~EoExGripManager();
	void init(OdGsDevice* device, OdGsModel* pGsModel, OdDbCommandContext* commandContext, GetSelectionSetPtr selectionSet);
	void uninit();

	// OdEdPointTracker protocol
	void setValue(const OdGePoint3d& pointValue) override;
	int addDrawables(OdGsView* pView) override;
	void removeDrawables(OdGsView* pView) override;

	// Events from Windows.
	bool onMouseDown(int x, int y, bool shiftIsDown);
	bool onMouseMove(int x, int y);
	bool onControlClick();

	void selectionSetChanged(OdDbSelectionSetPtr selectionSet);

	bool startHover(int x, int y);
	bool endHover();

	// Grip selection.
	void updateEntityGrips(const OdDbObjectId& id);
	void removeEntityGrips(const OdDbObjectId& id, bool bFireDone);
	void updateInvisibleGrips();

private:
	bool handleMappedRtClk(EoExGripDataPtrArray &aActiveKeys, int x, int y);

	void locateGripsAt(int x, int y, EoExGripDataPtrArray& aResult);
	void locateGripsByStatus(OdDbGripOperations::DrawType status, EoExGripDataPtrArray& aResult);

	// Adds/Removes drawables to/from viewports.
	void showGrip(EoExGripData* pGrip, bool isModel);
	void hideGrip(EoExGripData* pGrip, bool isModel);


	OdDbObjectId activeVpId() const;
	double activeViewUnitSize() const;
	OdGeVector3d activeViewDirection() const;
	OdGePoint3d eyeToUcsPlane(const OdGePoint3d &point, const OdGePoint3d &pBasePoint) const;

	OdGsLayoutHelperPtr m_pDevice;
	OdDbCommandContext* m_pCmdCtx;
	OdGsModel* m_pGsModel;

	bool m_bDisabled;
	void disable(bool isDisabled);

public:
	bool isDisabled() noexcept {
		return m_bDisabled;
	}
	struct EoExGripDataSubent {
		OdDbFullSubentPath m_entPath;
		EoExGripDataPtrArray m_pSubData;
	};
	struct EoExGripDataExt {
		EoExGripDataPtrArray m_pDataArray;
		OdArray<EoExGripDataSubent> m_pDataSub;
	};

private:
	typedef std::map<OdDbObjectId, EoExGripDataExt> GripDataMap;
	GripDataMap m_aGripData;

	EoExGripDataPtrArray m_aHoverGrips;

	OdGePoint3d m_ptBasePoint;
	OdGePoint3d m_ptLastPoint;
	EoExGripDragPtrArray m_aDrags;

	OdStaticRxObject<EoExGripDbReactor> m_cDbReactor;

	int m_GRIPSIZE;
	int m_GRIPOBJLIMIT;
	OdCmEntityColor m_GRIPCOLOR;
	OdCmEntityColor m_GRIPHOVER;
	OdCmEntityColor m_GRIPHOT;

	OdDbSelectionSetPtr workingSelectionSet() const;
	GetSelectionSetPtr m_pGetSelectionSetPtr;

	friend class EoExGripData;
	friend class EoExGripDrag;
};
