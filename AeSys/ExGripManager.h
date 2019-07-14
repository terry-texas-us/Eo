// From Examples\Editor\ExGripManager.h  (last compare 20.5)
#pragma once
#include <DbGsManager.h>
#include <DbDatabaseReactor.h>
#include <DbGrip.h>
#include <DbEntity.h>
#include <DbUserIO.h>
#include <Ed/EdCommandStack.h>
#include <Gi/GiDrawableImpl.h>
#include "OdBaseGripManager.h"
#include "OdExGripDrag.h"
#include "OdExGripData.h"
class OdBaseGripManager;
using OdExGripDataPtr = OdSmartPtr<class OdExGripData>;
using OdExGripDataPtrArray = OdArray<OdExGripDataPtr>;
using OdExGripDragPtr = OdSmartPtr<class OdExGripDrag>;
using OdExGripDragPtrArray = OdArray<OdExGripDragPtr>;

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

	OdGsModel* GetGsModel() noexcept override {
		return m_GsModel;
	}

	OdGsLayoutHelper* GetGsLayoutHelper() noexcept override {
		return m_LayoutHelper.get();
	}

	void Disable(bool disable) override;

	OdStaticRxObject<OdExGripDbReactor> m_DbReactor;

	// Selection set.
	[[nodiscard]] OdSelectionSetPtr WorkingSelectionSet() const override;

	GetSelectionSetPtr m_GetSelectionSet {nullptr};

	struct OdExGripCommand : OdEdCommand {
		OdExGripManager* parent {nullptr};

		const OdString groupName() const override {
			return L"EDIT";
		}

		const OdString globalName() const override {
			return L"GROUP_STRETCH";
		}

		void execute(OdEdCommandContext* edCommandContext) override;
	};

	OdStaticRxObject<OdExGripCommand> m_GripStretchCommand;
	friend struct OdExGripCommand;
};
