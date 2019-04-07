#pragma once

#include "DbSSet.h"
#include "Ed/EdCommandStack.h"
#include "ExDbCommandContext.h"
#include "DbGsManager.h"
#include "StaticRxObject.h"
#include "EoObjectSnapManager.h"
#include "EoExGripManager.h"

class OdExZoomCmd : public OdEdCommand {
public:
	const OdString groupName() const override;
	const OdString globalName() const override;
	void execute(OdEdCommandContext* commandContext) override;
};

class OdEx3dOrbitCmd : public OdEdCommand {
public:
	const OdString groupName() const override;
	const OdString globalName() const override;
	void execute(OdEdCommandContext* commandContext) override;
};

class OdExDollyCmd : public OdEdCommand {
public:
	const OdString groupName() const override;
	const OdString globalName() const override;
	void execute(OdEdCommandContext* commandContext) override;
};

class EoExEditorObject {
	OdGsLayoutHelperPtr m_pDevice;
	bool m_bSnapOn;
	OdStaticRxObject<EoObjectSnapManager> m_osnapManager;
	OdStaticRxObject<EoExGripManager> m_gripManager;
	OdGsModelPtr m_p2dModel;
	OdDbCommandContext* m_pCmdCtx;

	OdStaticRxObject<OdExZoomCmd> m_cmd_ZOOM;
	OdStaticRxObject<OdEx3dOrbitCmd> m_cmd_3DORBIT;
	OdStaticRxObject<OdExDollyCmd> m_cmd_DOLLY;

	void transformSSetBy(const OdGeMatrix3d& xfm);

	const OdGsView* activeView() const;
	OdGsView* activeView();
	const OdGsView* activeTopView() const;
	OdGsView* activeTopView();
	OdDbObjectId activeVpId() const;
	void ucsPlane(OdGePlane& plane) const;
	void dolly(int x, int y);

public:
	enum _3DViewType {
		k3DViewTop,
		k3DViewBottom,
		k3DViewLeft,
		k3DViewRight,
		k3DViewFront,
		k3DViewBack
	};
	void set3DView(_3DViewType type);
public:
	EoExEditorObject();

	void initialize(OdGsDevice* device, OdDbCommandContext* commandContext);
	void uninitialize();

	OdDbSelectionSetPtr workingSSet() const;
	void setWorkingSSet(OdDbSelectionSet* selectionSet);
	void selectionSetChanged();

	OdGiDrawablePtr snapDrawable() const;
	bool unselect();

	OdEdCommandPtr command(const OdString& sCmdName);

	OdGePoint3d toEyeToWorld(int x, int y) const;
	bool toUcsToWorld(OdGePoint3d& wcsPt) const;
	OdGePoint3d toScreenCoord(int x, int y) const;
	OdGePoint3d toScreenCoord(const OdGePoint3d& wcsPt) const;

	class OleDragCallback {
	public:
		virtual bool beginDragCallback(const OdGePoint3d& pt) = 0;
	};

	bool OnMouseLeftButtonClick(unsigned int flags, int x, int y, OleDragCallback* dragCallback = 0);
	bool OnMouseMove(unsigned int nFlags, int x, int y);
	bool OnMouseWheel(unsigned int nFlags, int x, int y, short zDelta);
	bool OnMouseLeftButtonDoubleClick(unsigned int nFlags, int x, int y);
	bool OnMouseRightButtonDoubleClick(unsigned int nFlags, int x, int y);
	bool OnCtrlClick();

	bool hasSelection() const { 
		return (workingSSet()->numEntities()>0);
	}
	bool isSnapOn() const noexcept {
		return m_bSnapOn;
	}
	bool snap(OdGePoint3d& pt, const OdGePoint3d* lastPoint);
	unsigned getSnapModes() const noexcept;
	void setSnapModes(bool snapOn, unsigned modes) noexcept;
	void resetSnapManager();
	void initSnapping(OdGsView* pView );
	void uninitSnapping(OdGsView* pView );

	OdGsModel* gsModel() { 
		return m_p2dModel.get();
	}
};
