#pragma once

// From Examples\Editor\EditorObject.h  (last compare 19.12)
#include <Ed/EdCommandStack.h>
#include <ExDbCommandContext.h>
#include <DbGsManager.h>
#include <StaticRxObject.h>
#include "OSnapManager.h"
#include "ExGripManager.h"

class OdExZoomCmd : public OdEdCommand {
public:
	[[nodiscard]] const OdString groupName() const override;
	[[nodiscard]] const OdString globalName() const override;
	void execute(OdEdCommandContext* edCommandContext) override;
};

class OdEx3dOrbitCmd : public OdEdCommand {
public:
	[[nodiscard]] const OdString groupName() const override;
	[[nodiscard]] const OdString globalName() const override;
	void execute(OdEdCommandContext* edCommandContext) override;
};

class OdExDollyCmd : public OdEdCommand {
public:
	[[nodiscard]] const OdString groupName() const override;
	[[nodiscard]] const OdString globalName() const override;
	void execute(OdEdCommandContext* edCommandContext) override;
};

class OdExInteractivityModeCmd : public OdEdCommand {
public:
	[[nodiscard]] const OdString groupName() const override;
	[[nodiscard]] const OdString globalName() const override;
	void execute(OdEdCommandContext* edCommandContext) override;
};

class OdExCollideCmd : public OdEdCommand {
public:
	[[nodiscard]] const OdString groupName() const override;
	[[nodiscard]] const OdString globalName() const override;
	void execute(OdEdCommandContext* edCommandContext) override;
};

class OdExCollideAllCmd : public OdEdCommand {
public:
	[[nodiscard]] const OdString groupName() const override;
	[[nodiscard]] const OdString globalName() const override;
	void execute(OdEdCommandContext* edCommandContext) override;
};

class OdExEditorObject {
	OdGsLayoutHelperPtr m_LayoutHelper;
	OdStaticRxObject<OSnapManager> m_ObjectSnapManager;
	OdStaticRxObject<OdExGripManager> m_GripManager;
	OdGsModelPtr m_p2dModel;
	OdDbCommandContext* m_CommandContext;
	OdStaticRxObject<OdExZoomCmd> m_cmd_ZOOM;
	OdStaticRxObject<OdEx3dOrbitCmd> m_cmd_3DORBIT;
	OdStaticRxObject<OdExDollyCmd> m_cmd_DOLLY;
	OdStaticRxObject<OdExInteractivityModeCmd> m_cmd_INTERACTIVITY;
	OdStaticRxObject<OdExCollideCmd> m_cmd_COLLIDE;
	OdStaticRxObject<OdExCollideAllCmd> m_cmd_COLLIDE_ALL;
	OdEdInputTrackerPtr m_InputTracker;
	OdGePoint3d m_basePt;
	const OdGePoint3d* m_BasePt;

	enum Flags { kSnapOn = 4, kOrbitOn = 8, kDragging = 16, kTrackerHasDrawables = 32 };

	unsigned long m_flags;
public:
	[[nodiscard]] const OdGsView* ActiveView() const;
	OdGsView* ActiveView();
	[[nodiscard]] const OdGsView* ActiveTopView() const;
	OdGsView* ActiveTopView();
	[[nodiscard]] OdDbObjectId ActiveViewportId() const;
	void UcsPlane(OdGePlane& plane) const;
	void Dolly(int x, int y);
	static void ZoomAt(OdGsView* view, int x, int y, short zDelta);
	static void Dolly(OdGsView* view, int x, int y);

	enum _3DViewType { k3DViewTop, k3DViewBottom, k3DViewLeft, k3DViewRight, k3DViewFront, k3DViewBack, k3DViewSW, k3DViewSE, k3DViewNE, k3DViewNW };

	void Set3DView(_3DViewType type);
	OdExEditorObject();
	void Initialize(OdGsDevice* device, OdDbCommandContext* dbCommandContext);
	void Uninitialize();
	[[nodiscard]] OdDbSelectionSetPtr workingSSet() const;
	void SetWorkingSelectionSet(OdDbSelectionSet* selectionSet);
	void SelectionSetChanged();
	[[nodiscard]] OdGiDrawablePtr SnapDrawable() const;
	bool Unselect();
	OdEdCommandPtr Command(const OdString& commandName);
	[[nodiscard]] OdGePoint3d ToEyeToWorld(int x, int y) const;
	bool ToUcsToWorld(OdGePoint3d& wcsPt) const;
	[[nodiscard]] OdGePoint3d ToScreenCoord(int x, int y) const;
	[[nodiscard]] OdGePoint3d ToScreenCoord(const OdGePoint3d& wcsPt) const;

	class OleDragCallback {
	public:
		virtual bool beginDragCallback(const OdGePoint3d& pt) = 0;
	};

	bool OnSize(unsigned flags, int w, int h);
	bool OnPaintFrame(unsigned flags = 0, OdGsDCRect* updatedRectangle = nullptr);
	bool OnMouseLeftButtonClick(unsigned flags, int x, int y, OleDragCallback* dragCallback = nullptr);
	bool OnMouseMove(unsigned flags, int x, int y);
	bool OnMouseWheel(unsigned flags, int x, int y, short zDelta);
	bool OnMouseLeftButtonDoubleClick(unsigned flags, int x, int y);
	bool OnMouseRightButtonDoubleClick(unsigned flags, int x, int y);
	bool OnCtrlClick();
	void OnDestroy();

	[[nodiscard]] bool HasSelection() const { return workingSSet()->numEntities() > 0; }

	[[nodiscard]] bool IsSnapOn() const noexcept { return GETBIT(m_flags, kSnapOn); }

	void SetSnapOn(bool snapOn) noexcept {
		SETBIT(m_flags, kSnapOn, snapOn);
	}

	[[nodiscard]] bool IsOrbitOn() const noexcept { return GETBIT(m_flags, kOrbitOn); }

	void TurnOrbitOn(bool orbitOn);
	bool OnOrbitBeginDrag(int x, int y);
	bool OnOrbitEndDrag(int x, int y);
	bool OnZoomWindowBeginDrag(int x, int y);
	bool OnZoomWindowEndDrag(int x, int y);
	bool Snap(OdGePoint3d& point, const OdGePoint3d* lastPoint = nullptr);
	[[nodiscard]] unsigned GetSnapModes() const;
	void SetSnapModes(bool snapOn, unsigned snapModes);
	void ResetSnapManager();
	void InitializeSnapping(OdGsView* view, OdEdInputTracker* inputTracker);
	void UninitializeSnapping(OdGsView* view);

	OdGsModel* GsModel() { return m_p2dModel.get(); }

	void RecalculateEntityCenters() {
		m_ObjectSnapManager.RecalculateEntityCenters();
	}

	void SetEntityCenters() {

		if (HasDatabase()) { m_ObjectSnapManager.SetEntityCenters(m_CommandContext->database()); }
	}

	void SetTracker(OdEdInputTracker* inputTracker);
	bool TrackString(const OdString& value);
	bool TrackPoint(const OdGePoint3d& point);
	[[nodiscard]] bool HasDatabase() const;
};

inline OdGiDrawablePtr OdExEditorObject::SnapDrawable() const {
	return &m_ObjectSnapManager;
}

inline void OdExEditorObject::ResetSnapManager() {
	m_ObjectSnapManager.Reset();
}
