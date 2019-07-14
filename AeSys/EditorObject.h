// From Examples\Editor\EditorObject.h  (last compare 20.5)
#pragma once
#include "Eo3dOrbitCmd.h"
#include "EoDollyCmd.h"
#include "ExGripManager.h"
#include "OSnapManager.h"
#include <ExDbCommandContext.h>
#include "EoZoomCmd.h"
#include "EoCollideCmd.h"
#include "EoInteractivityModeCmd.h"
#include "EoCollideAllCmd.h"

class OdExEditorObject {
	OdGsLayoutHelperPtr m_LayoutHelper;
	OdStaticRxObject<OSnapManager> m_ObjectSnapManager;
	OdStaticRxObject<OdExGripManager> m_GripManager;
	OdGsModelPtr m_2dModel;
	OdDbCommandContext* m_CommandContext {nullptr};
	OdStaticRxObject<OdExZoomCmd> m_cmd_ZOOM;
	OdStaticRxObject<OdEx3dOrbitCmd> m_cmd_3DORBIT;
	OdStaticRxObject<OdExDollyCmd> m_cmd_DOLLY;
	OdStaticRxObject<OdExInteractivityModeCmd> m_cmd_INTERACTIVITY;
	OdStaticRxObject<OdExCollideCmd> m_cmd_COLLIDE;
	OdStaticRxObject<OdExCollideAllCmd> m_cmd_COLLIDE_ALL;
	OdEdInputTrackerPtr m_InputTracker;
	OdGePoint3d m_basePt;
	const OdGePoint3d* m_BasePt {nullptr};

	enum Flags : unsigned { kSnapOn = 4, kOrbitOn = 8, kDragging = 16, kTrackerHasDrawables = 32 };

	unsigned m_Flags {kSnapOn};
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

	enum _3DViewType { k3DViewTop, k3DViewBottom, k3DViewLeft, k3DViewRight, k3DViewFront, k3DViewBack, k3DViewSouthWest, k3DViewSouthEast, k3DViewNorthEast, k3DViewNorthWest };

	void Set3DView(_3DViewType type);

	OdExEditorObject() = default;

	void Initialize(OdGsDevice* device, OdDbCommandContext* commandContext);

	void Uninitialize();

	[[nodiscard]] OdDbSelectionSetPtr GetWorkingSelectionSet() const;

	void SetWorkingSelectionSet(OdDbSelectionSet* selectionSet) const;

	void SelectionSetChanged();

	[[nodiscard]] OdGiDrawablePtr SnapDrawable() const;

	bool Unselect();

	[[nodiscard]] OdEdCommandPtr Command(const OdString& commandName) const;

	[[nodiscard]] OdGePoint3d ToEyeToWorld(int x, int y) const;

	bool ToUcsToWorld(OdGePoint3d& wcsPoint) const;

	[[nodiscard]] OdGePoint3d ToScreenCoordinates(int x, int y) const;

	[[nodiscard]] OdGePoint3d ToScreenCoordinates(const OdGePoint3d& worldPoint) const;

	class OleDragCallback {
	public:
		virtual ~OleDragCallback() = default;

		virtual bool BeginDragCallback(const OdGePoint3d& point) = 0;
	};

	bool OnSize(unsigned flags, int w, int h);

	bool OnPaintFrame(unsigned flags = 0, OdGsDCRect* updatedRectangle = nullptr);

	bool OnMouseLeftButtonClick(unsigned flags, int x, int y, OleDragCallback* dragCallback = nullptr);

	bool OnMouseMove(unsigned flags, int x, int y);

	bool OnMouseWheel(unsigned flags, int x, int y, short zDelta);

	bool OnMouseLeftButtonDoubleClick(unsigned flags, int x, int y);

	bool OnMouseRightButtonDoubleClick(unsigned flags, int x, int y);

	bool OnCtrlClick() const;

	void OnDestroy();

	[[nodiscard]] bool HasSelection() const {
		return GetWorkingSelectionSet()->numEntities() > 0;
	}

	[[nodiscard]] bool IsSnapOn() const noexcept {
		return (m_Flags & kSnapOn) != 0U ? true : false;
	}

	void SetSnapOn(const bool snapOn) noexcept {
		snapOn ? (m_Flags |= kSnapOn) : m_Flags &= ~kSnapOn;
	}

	[[nodiscard]] bool IsOrbitOn() const noexcept {
		return (m_Flags & kOrbitOn) != 0U ? true : false;
	}

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

	OdGsModel* GsModel() {
		return m_2dModel.get();
	}

	void RecalculateEntityCenters() {
		m_ObjectSnapManager.RecalculateEntityCenters();
	}

	void SetEntityCenters() {
		if (HasDatabase()) {
			m_ObjectSnapManager.SetEntityCenters(m_CommandContext->database());
		}
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
