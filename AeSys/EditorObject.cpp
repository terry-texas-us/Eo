// From Examples\Editor\EditorObject.cpp (last compare 19.12)

#include "OdaCommon.h"
#include "Ge/GeRay3d.h"
#include "Gi/GiDrawableImpl.h"
#include "Gi/GiWorldDraw.h"
#include "Gs/Gs.h"
#include "Gs/GsBaseVectorizer.h"
#include "EditorObject.h"
#include "GiContextForDbDatabase.h"
#include "DbLayout.h"
#include "DbCommandContext.h"
#include "DbAbstractViewportData.h"
#include "DbViewport.h"
#include "DbBlockTableRecord.h"
#include "DbViewportTable.h"
#include "DbDictionary.h"
#include "DbVisualStyle.h"
#include "DbHostAppServices.h"
#include "OdDToStr.h"
#include "SaveState.h"
#include "ExTrackers.h"
#include "RxVariantValue.h"

#include "Gs/GsModel.h"
#include "Gi/GiPathNode.h"
#include "DbBlockReference.h"

static const int SNAP_SIZE = 10;

class ViewInteractivityMode {
	bool m_enabled;
	OdGsView* m_View;

   public:
	ViewInteractivityMode(OdRxVariantValue enable, OdRxVariantValue frameRate, OdGsView* view) {
		m_enabled = false;
		m_View = view;

		if (!enable.isNull()) {
			m_enabled = static_cast<bool>(enable);
			if (m_enabled && !frameRate.isNull()) {
				const auto rate {frameRate.get()->getDouble()};
				view->beginInteractivity(rate);
			}
		}
	}
	~ViewInteractivityMode() {
		if (m_enabled) { m_View->endInteractivity(); }
	}
};

class EnableEnhRectFrame {
	OdEdCommandContext* m_CommandContext;

   public:
	EnableEnhRectFrame(OdEdCommandContext* edCommandContext)
		: m_CommandContext(edCommandContext) {
		m_CommandContext->setArbitraryData(L"ExDbCommandContext_EnhRectFrame", OdRxVariantValue(true));
	}
	~EnableEnhRectFrame() { m_CommandContext->setArbitraryData(L"ExDbCommandContext_EnhRectFrame", nullptr); }
};

void setWorkingSelectionSet(OdDbCommandContext* dbCommandContext, OdDbSelectionSet* selectionSet) {
	dbCommandContext->setArbitraryData(L"AeSys Working Selection Set", selectionSet);
}

OdDbSelectionSetPtr WorkingSelectionSet(OdDbCommandContext* dbCommandContext) {
	OdDbSelectionSetPtr SelectionSet;

	if (dbCommandContext) {
		SelectionSet = dbCommandContext->arbitraryData(L"AeSys Working Selection Set");

		if (SelectionSet.isNull()) {
			SelectionSet = OdDbSelectionSet::createObject(dbCommandContext->database());
			setWorkingSelectionSet(dbCommandContext, SelectionSet);
		}
	}
	return SelectionSet;
}

class XFormDrawable : public OdGiDrawableImpl<OdGiDrawable> {
	OdGiDrawablePtr m_Drawable;
	const OdGeMatrix3d* m_pXForm {nullptr};

   protected:
	XFormDrawable() = default;

public:
	static OdGiDrawablePtr createObject(OdGiDrawable* drawable, const OdGeMatrix3d& xForm) {
		auto pRes {OdRxObjectImpl<XFormDrawable>::createObject()};
		pRes->m_Drawable = drawable;
		pRes->m_pXForm = &xForm;
		return pRes;
	}

	unsigned long subSetAttributes(OdGiDrawableTraits* drawableTraits) const noexcept override {
		return kDrawableUsesNesting;
	}

	bool subWorldDraw(OdGiWorldDraw* worldDraw) const override {
		OdGiModelTransformSaver mt(worldDraw->geometry(), *m_pXForm);
		worldDraw->geometry().draw(m_Drawable);
		return true;
	}

	void subViewportDraw(OdGiViewportDraw* viewportDraw) const noexcept override {}
};


OdExEditorObject::OdExEditorObject() {
	SETBIT(m_flags, kSnapOn, true);
}

void OdExEditorObject::Initialize(OdGsDevice* device, OdDbCommandContext* dbCommandContext) {
	m_LayoutHelper = device;
	m_CommandContext = dbCommandContext;

	m_p2dModel = device->createModel();

	if (!m_p2dModel.isNull()) {
		m_p2dModel->setRenderType(OdGsModel::kDirect); // Skip Z-buffer for 2d drawables.
		m_p2dModel->setEnableViewExtentsCalculation(false); // Skip extents calculation.
		m_p2dModel->setRenderModeOverride(OdGsView::k2DOptimized); // Setup 2dWireframe mode for all underlying geometry.
		m_p2dModel->setVisualStyle(OdDbDictionary::cast(m_CommandContext->database()->getVisualStyleDictionaryId().openObject())->getAt(OdDb::kszVS2DWireframe));
	}
	m_GripManager.Initialize(device, m_p2dModel, dbCommandContext, WorkingSelectionSet);

	SetEntityCenters();
}

void OdExEditorObject::Uninitialize() {
	auto SelectionSet {workingSSet()};

	if (SelectionSet.get()) {
		SelectionSet->clear();
		m_GripManager.SelectionSetChanged(SelectionSet);
	}
	m_GripManager.Uninitialize();

	m_LayoutHelper.release();
	m_CommandContext = nullptr;
}

void OdExEditorObject::InitializeSnapping(OdGsView* view, OdEdInputTracker* inputTracker) {
	m_ObjectSnapManager.Track(inputTracker);
	view->add(&m_ObjectSnapManager, m_p2dModel);
}

void OdExEditorObject::UninitializeSnapping(OdGsView* view) {
	view->erase(&m_ObjectSnapManager);
	m_ObjectSnapManager.Track(nullptr);
}

OdDbSelectionSetPtr OdExEditorObject::workingSSet() const {
	return WorkingSelectionSet(m_CommandContext);
}

void OdExEditorObject::SetWorkingSelectionSet(OdDbSelectionSet* selectionSet) {
	setWorkingSelectionSet(m_CommandContext, selectionSet);
}

void OdExEditorObject::SelectionSetChanged() {
	m_GripManager.SelectionSetChanged(workingSSet());
}

const OdGsView* OdExEditorObject::ActiveView() const {
	return m_LayoutHelper->activeView();
}

OdGsView* OdExEditorObject::ActiveView() {
	return m_LayoutHelper->activeView();
}

const OdGsView* OdExEditorObject::ActiveTopView() const {
	auto View {ActiveView()};

	if (HasDatabase()) {
		if (!m_CommandContext->database()->getTILEMODE()) {
			auto ActiveViewport {m_CommandContext->database()->activeViewportId().safeOpenObject()};
			OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);

			if (!AbstractViewportData.isNull() && AbstractViewportData->gsView(ActiveViewport)) {
				View = AbstractViewportData->gsView(ActiveViewport);
			}
		}
	}
	return View;
}

OdGsView* OdExEditorObject::ActiveTopView() {
	return const_cast<OdGsView*>(const_cast<const OdExEditorObject*>(this)->ActiveTopView());
}

OdDbObjectId OdExEditorObject::ActiveViewportId() const {
	OdGsClientViewInfo ClientViewInfo;
	const_cast<OdGsView*>(ActiveView())->clientViewInfo(ClientViewInfo);
	return OdDbObjectId(ClientViewInfo.viewportObjectId);
}

void OdExEditorObject::UcsPlane(OdGePlane& plane) const {
	auto ActiveViewport {ActiveViewportId().safeOpenObject()};
	OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);
	OdGePoint3d ucsOrigin;
	OdGeVector3d ucsXAxis;
	OdGeVector3d ucsYAxis;
	AbstractViewportData->getUcs(ActiveViewport, ucsOrigin, ucsXAxis, ucsYAxis);
	const auto Elevation {AbstractViewportData->elevation(ActiveViewport)};

	if (!OdZero(Elevation)) {
		const auto vElevation = ucsXAxis.crossProduct(ucsYAxis) * Elevation;
		ucsOrigin += vElevation;
	}
	plane.set(ucsOrigin, ucsXAxis, ucsYAxis);
}

OdGePoint3d OdExEditorObject::ToEyeToWorld(int x, int y) const {
	OdGePoint3d wcsPt(x, y, 0.0);
	const auto View {ActiveView()};

	if (View->isPerspective()) { wcsPt.z = View->projectionMatrix()(2, 3); }

	wcsPt.transformBy((View->screenMatrix() * View->projectionMatrix()).inverse());
	wcsPt.z = 0.0;
	// eye CS at this point.

	wcsPt.transformBy(OdAbstractViewPEPtr(View)->eyeToWorld(View));
	return wcsPt;
}

bool OdExEditorObject::ToUcsToWorld(OdGePoint3d& wcsPt) const {
	const auto View {ActiveView()};
	OdGePlane plane;
	UcsPlane(plane);

	if (!View->isPerspective()) { // For orhogonal projection we simply check intersection between viewing direction and UCS plane.
		OdGeLine3d line(wcsPt, OdAbstractViewPEPtr(View)->direction(View));
		return plane.intersectWith(line, wcsPt);
	}
	// For perspective projection we emit ray from viewer position through WCS point.
	const double focalLength = -1.0 / View->projectionMatrix()(3, 2);
	const auto pos {View->target() + OdAbstractViewPEPtr(View)->direction(View).normal() * focalLength};
	OdGeRay3d ray(pos, wcsPt);
	return plane.intersectWith(ray, wcsPt);
}

OdGePoint3d OdExEditorObject::ToScreenCoord(int x, int y) const {
	OdGePoint3d scrPt(x, y, 0.0);
	const auto View {ActiveView()};
	scrPt.transformBy((View->screenMatrix() * View->projectionMatrix()).inverse());
	scrPt.z = 0.0;
	return scrPt;
}

OdGePoint3d OdExEditorObject::ToScreenCoord(const OdGePoint3d& wcsPt) const {
	// To DCS
	auto scrPt(wcsPt);
	const auto View {ActiveView()};
	OdGsClientViewInfo ClientViewInfo;
	View->clientViewInfo(ClientViewInfo);
	OdRxObjectPtr pObj = OdDbObjectId(ClientViewInfo.viewportObjectId).openObject();
	OdAbstractViewPEPtr AbstractView(pObj);
	const auto vecY {AbstractView->upVector(pObj)};
	const auto vecZ {AbstractView->direction(pObj)};
	const auto vecX {vecY.crossProduct(vecZ).normal()};
	const auto offset {AbstractView->viewOffset(pObj)};
	const auto prTarg {AbstractView->target(pObj) - vecX * offset.x - vecY * offset.y};

	scrPt.x = vecX.dotProduct(wcsPt - prTarg);
	scrPt.y = vecY.dotProduct(wcsPt - prTarg);
	scrPt.z = 0.0;
	return scrPt;
}

bool OdExEditorObject::OnSize(unsigned flags, int w, int h) {
	if (m_LayoutHelper.get()) {
		m_LayoutHelper->onSize(OdGsDCRect(0, w, h, 0));
		return true;
	}
	return false;
}

bool OdExEditorObject::OnPaintFrame(unsigned flags, OdGsDCRect* updatedRect) {
	if (m_LayoutHelper.get() && !m_LayoutHelper->isValid()) {
		m_LayoutHelper->update(updatedRect);
		return true;
	}
	return false;
}

unsigned OdExEditorObject::GetSnapModes() const {
	return m_ObjectSnapManager.SnapModes();
}

void OdExEditorObject::SetSnapModes(bool snapOn, unsigned snapModes) {
	SETBIT(m_flags, kSnapOn, snapOn);
	m_ObjectSnapManager.SetSnapModes(snapModes);
}

OdEdCommandPtr OdExEditorObject::Command(const OdString& commandName) {
	if (commandName == m_cmd_ZOOM.globalName()) { return &m_cmd_ZOOM; }

	if (commandName == m_cmd_3DORBIT.globalName()) { return &m_cmd_3DORBIT; }

	if (commandName == m_cmd_DOLLY.globalName()) { return &m_cmd_DOLLY; }

	if (commandName == m_cmd_INTERACTIVITY.globalName()) { return &m_cmd_INTERACTIVITY; }

	if (commandName == m_cmd_COLLIDE.globalName()) { return &m_cmd_COLLIDE; }

	if (commandName == m_cmd_COLLIDE_ALL.globalName()) { return &m_cmd_COLLIDE_ALL; }

	return OdEdCommandPtr();
}

void OdExEditorObject::Set3DView(_3DViewType type) {
	auto Target {OdGePoint3d::kOrigin};
	OdGePoint3d Position;
	OdGeVector3d Axis;
	switch (type) {
		case k3DViewTop:
			Position = OdGePoint3d::kOrigin + OdGeVector3d::kZAxis;
			Axis = OdGeVector3d::kYAxis;
			break;
		case k3DViewBottom:
			Position = OdGePoint3d::kOrigin - OdGeVector3d::kZAxis;
			Axis = OdGeVector3d::kYAxis;
			break;
		case k3DViewLeft:
			Position = OdGePoint3d::kOrigin - OdGeVector3d::kXAxis;
			Axis = OdGeVector3d::kZAxis;
			break;
		case k3DViewRight:
			Position = OdGePoint3d::kOrigin + OdGeVector3d::kXAxis;
			Axis = OdGeVector3d::kZAxis;
			break;
		case k3DViewFront:
			Position = OdGePoint3d::kOrigin - OdGeVector3d::kYAxis;
			Axis = OdGeVector3d::kZAxis;
			break;
		case k3DViewBack:
			Position = OdGePoint3d::kOrigin + OdGeVector3d::kYAxis;
			Axis = OdGeVector3d::kZAxis;
			break;
		case k3DViewSW:
			Position = OdGePoint3d::kOrigin + OdGeVector3d(-1.0, -1.0, 1.0);
			Axis = OdGeVector3d(0.5, 0.5, 1.0).normal();
			break;
		case k3DViewSE:
			Position = OdGePoint3d::kOrigin + OdGeVector3d(1.0, -1.0, 1.0);
			Axis = OdGeVector3d(-0.5, 0.5, 1.0).normal();
			break;
		case k3DViewNE:
			Position = OdGePoint3d::kOrigin + OdGeVector3d(1.0, 1.0, 1.0);
			Axis = OdGeVector3d(-0.5, -0.5, 1.0).normal();
			break;
		case k3DViewNW:
			Position = OdGePoint3d::kOrigin + OdGeVector3d(-1.0, 1.0, 1.0);
			Axis = OdGeVector3d(0.5, -0.5, 1.0).normal();
			break;
	}
	Unselect();

	auto View {ActiveView()};
	{
		OdGsClientViewInfo ClientViewInfo;
		View->clientViewInfo(ClientViewInfo);
		auto pObject {OdDbObjectId(ClientViewInfo.viewportObjectId).safeOpenObject(OdDb::kForWrite)};
		OdAbstractViewPEPtr(pObject)->setUcs(pObject, Target, Axis.crossProduct(Position.asVector()), Axis);
	}
	View->setView(Position, Target, Axis, View->fieldWidth(), View->fieldHeight(), View->isPerspective() ? OdGsView::kPerspective : OdGsView::kParallel);
}

bool OdExEditorObject::Snap(OdGePoint3d& point, const OdGePoint3d* lastPoint) {
	if (IsSnapOn()) {
		if (m_ObjectSnapManager.Snap(ActiveView(), point, m_BasePt)) {
			if (!m_p2dModel.isNull()) { m_p2dModel->onModified(&m_ObjectSnapManager, static_cast<OdGiDrawable*>(nullptr)); }

			return true;
		}
	}
	return false;
}

bool OdExEditorObject::Unselect() {
	auto Result {false};
	auto SelectionSet {workingSSet()};
	OdDbSelectionSetIteratorPtr SelectionSetIterator {SelectionSet->newIterator()};

	while (!SelectionSetIterator->done()) {
		auto Entity {OdDbEntity::cast(SelectionSetIterator->objectId().openObject())};

		if (Entity.get()) {
			Entity->highlight(false);
			Result = true;
		}
		SelectionSetIterator->next();
	}
	// Don't clear working selection set 'SelectionSet->clear()' to prevent previous selection modification
	SelectionSet = OdDbSelectionSet::createObject(SelectionSet->database());
	setWorkingSelectionSet(m_CommandContext, SelectionSet);
	m_GripManager.SelectionSetChanged(SelectionSet);
	return Result;
}

bool OdExEditorObject::OnCtrlClick() {
	return m_GripManager.onControlClick();
}

void OdExEditorObject::OnDestroy() {
	m_LayoutHelper.release();
	m_p2dModel.release();
	m_CommandContext = nullptr;
}

bool OdExEditorObject::OnMouseLeftButtonClick(unsigned flags, int x, int y, OleDragCallback* dragCallback) {
	const auto ShiftIsDown {(OdEdBaseIO::kShiftIsDown & flags) != 0};
	const auto ControlIsDown {(OdEdBaseIO::kControlIsDown & flags) != 0};
	const auto pt {ToEyeToWorld(x, y)};

	if (m_GripManager.OnMouseDown(x, y, ShiftIsDown)) { return true; }

	try {
		if (dragCallback && !ShiftIsDown) {
			auto SelectionSet {workingSSet()};
			auto SelectionSetAtPoint = OdDbSelectionSet::select(ActiveViewportId(), 1, &pt, OdDbVisualSelection::kPoint, ControlIsDown ? OdDbVisualSelection::kEnableSubents : OdDbVisualSelection::kDisableSubents);
			OdDbSelectionSetIteratorPtr SelectionSetAtPointIterator {SelectionSetAtPoint->newIterator()};
			while (!SelectionSetAtPointIterator->done()) {
				if (SelectionSet->isMember(SelectionSetAtPointIterator->objectId()) && !ControlIsDown) {
					SelectionSetAtPointIterator.release();
					break;
				}
				SelectionSetAtPointIterator->next();
			}
			if (SelectionSetAtPointIterator.isNull()) {
				if (dragCallback->beginDragCallback(pt)) {
					// Not good idea to clear selection set if already selected object has been selected, but if selection set is being cleared - items must be unhighlighted too.
					//workingSSet()->clear();
					//SelectionSetChanged();
					Unselect();
					return true;
				}
			}
		}
	} catch (const OdError&) {
		return false;
	}
	auto UserIO {m_CommandContext->dbUserIO()};
	UserIO->setLASTPOINT(pt);
	UserIO->setPickfirst(nullptr);
	auto SelectOptions {OdEd::kSelPickLastPoint | OdEd::kSelSinglePass | OdEd::kSelLeaveHighlighted | OdEd::kSelAllowInactSpaces};

	if (HasDatabase()) {
		if (ShiftIsDown) {
			if (m_CommandContext->database()->appServices()->getPICKADD() > 0) { SelectOptions |= OdEd::kSelRemove; }

		} else {
			if (m_CommandContext->database()->appServices()->getPICKADD() == 0) { Unselect(); }
		}
	}
	if (ControlIsDown) { SelectOptions |= OdEd::kSelAllowSubents; }

	OdDbSelectionSetPtr SelectionSet;
	const auto SavedSnapMode {IsSnapOn()};
	try {
		EnableEnhRectFrame _enhRect(m_CommandContext);
		SetSnapOn(false);
		SelectionSet = UserIO->select(L"", SelectOptions, workingSSet());
		SetWorkingSelectionSet(SelectionSet);
		SetSnapOn(SavedSnapMode);
	} catch (const OdError&) {
		SetSnapOn(SavedSnapMode);
		return false;
	} catch (...) {
		SetSnapOn(SavedSnapMode);
		throw;
	}
	SelectionSetChanged();

	return true;
}

bool OdExEditorObject::OnMouseLeftButtonDoubleClick(unsigned flags, int x, int y) {
	auto View {ActiveView()};
	m_LayoutHelper->setActiveViewport(OdGePoint2d(x, y));
	const auto Changed {View != ActiveView()};

	if (Changed) {
		auto ActiveViewport {ActiveViewportId().safeOpenObject()};
		auto Database {ActiveViewport->database()};

		if (Database->getTILEMODE()) {
			OdDbViewportTable::cast(Database->getViewportTableId().safeOpenObject(OdDb::kForWrite))->SetActiveViewport(ActiveViewportId());
		} else {
			OdDbLayout::cast(OdDbBlockTableRecord::cast(Database->getPaperSpaceId().safeOpenObject())->getLayoutId().safeOpenObject(OdDb::kForWrite))->setActiveViewportId(ActiveViewportId());
		}
		Unselect();
	}
	return Changed;
}

bool OdExEditorObject::OnMouseRightButtonDoubleClick(unsigned flags, int x, int y) {
	Unselect();

	auto View {ActiveView()};

	// set target to center of the scene, keep view direction:
	const auto Target {View->target()};

	View->setView(Target + OdGeVector3d::kZAxis, Target, OdGeVector3d::kYAxis, View->fieldWidth(), View->fieldHeight());

	return true;
}

bool OdExEditorObject::OnMouseMove(unsigned flags, int x, int y) {
	return m_GripManager.OnMouseMove(x, y);
}

void OdExEditorObject::Dolly(int x, int y) {
	auto View {ActiveView()};
	Dolly(View, x, y);
}

void OdExEditorObject::Dolly(OdGsView* view, int x, int y) {
	OdGeVector3d vec(-x, -y, 0.0);
	vec.transformBy((view->screenMatrix() * view->projectionMatrix()).inverse());
	view->dolly(vec);
}

bool OdExEditorObject::OnMouseWheel(unsigned flags, int x, int y, short zDelta) {
	auto View {ActiveView()};
	ZoomAt(View, x, y, zDelta);

	if (!m_p2dModel.isNull()) {
		m_p2dModel->invalidate(ActiveTopView());
	}
	return true;
}

void OdExEditorObject::ZoomAt(OdGsView* view, int x, int y, short zDelta) {
	auto pos(view->position());
	pos.transformBy(view->worldToDeviceMatrix());

	// In 2d mode perspective zoom change lens length instead of fieldWidth/fieldHeight. This is non-standard mode. Practically 2d mode can't be perspective.
	if (view->isPerspective() && view->mode() == OdGsView::k2DOptimized) {
		pos = OdGePoint3d(0.5, 0.5, 0.0).transformBy(view->screenMatrix());
	}
	auto vx {static_cast<int>(OdRound(pos.x))};
	auto vy {static_cast<int>(OdRound(pos.y))};
	vx = x - vx;
	vy = y - vy;
	Dolly(view, -vx, -vy);
	view->zoom(zDelta > 0 ? 1. / .9 : .9);
	Dolly(view, vx, vy);
}

const OdString OdExZoomCmd::groupName() const {
	return globalName();
}
const OdString OdExZoomCmd::globalName() const {
	return L"ZOOM";
}

void zoom_window(OdGePoint3d& pt1, OdGePoint3d& pt2, OdGsView* view) {
	const auto WorldToEyeTransform {OdAbstractViewPEPtr(view)->worldToEye(view)};
	pt1.transformBy(WorldToEyeTransform);
	pt2.transformBy(WorldToEyeTransform);
	auto eyeVec {pt2 - pt1};

	if (OdNonZero(eyeVec.x) && OdNonZero(eyeVec.y)) {
		auto newPos {pt1 + eyeVec / 2.};

		eyeVec.x = fabs(eyeVec.x);
		eyeVec.y = fabs(eyeVec.y);

		view->dolly(newPos.asVector());

		const auto wf {view->fieldWidth() / eyeVec.x};
		const auto hf {view->fieldHeight() / eyeVec.y};

		view->zoom(odmin(wf, hf));
	}
}

void zoom_window2(const OdGePoint3d& pt1, const OdGePoint3d& pt2, OdGsView* pView) {
	auto pt1c {pt1};
	auto pt2c {pt2};
	zoom_window(pt1c, pt2c, pView);
}

void zoom_scale(double factor) noexcept {
}

static bool getLayoutExtents(const OdDbObjectId& spaceId, const OdGsView* view, OdGeBoundBlock3d& boundBox) {
	OdDbBlockTableRecordPtr pSpace {spaceId.safeOpenObject()};
	OdSmartPtr<OdDbLayout> Layout {pSpace->getLayoutId().safeOpenObject()};
	OdGeExtents3d Extents;

	if (Layout->getGeomExtents(Extents) == eOk) {
		Extents.transformBy(view->viewingMatrix());
		boundBox.set(Extents.minPoint(), Extents.maxPoint());
		return Extents.minPoint() != Extents.maxPoint();
	}
	return false;
}

void zoom_extents(OdGsView* view, OdDbObject* viewportObject) {
	auto Database {viewportObject->database()};
	OdAbstractViewPEPtr AbstractView(view);
	OdGeBoundBlock3d BoundBox;
	auto ValidBoundBox {AbstractView->viewExtents(view, BoundBox)};

	// paper space overall view
	auto Viewport {OdDbViewport::cast(viewportObject)};

	if (Viewport.get() && Viewport->number() == 1) {
		if (!ValidBoundBox || !(BoundBox.minPoint().x < BoundBox.maxPoint().x && BoundBox.minPoint().y < BoundBox.maxPoint().y)) {
			ValidBoundBox = getLayoutExtents(Database->getPaperSpaceId(), view, BoundBox);
		}
	} else if (!ValidBoundBox) { // model space viewport
		ValidBoundBox = getLayoutExtents(Database->getPaperSpaceId(), view, BoundBox);
	}
	if (!ValidBoundBox) { // set to somewhat reasonable (e.g. paper size)

		if (Database->getMEASUREMENT() == OdDb::kMetric) {
			BoundBox.set(OdGePoint3d::kOrigin, OdGePoint3d(297.0, 210.0, 0.0)); // set to papersize ISO A4 (portrait)
		} else {
			BoundBox.set(OdGePoint3d::kOrigin, OdGePoint3d(11.0, 8.5, 0.0)); // ANSI A (8.50 x 11.00) (landscape)
		}
		BoundBox.transformBy(view->viewingMatrix());
	}
	AbstractView->zoomExtents(view, &BoundBox);
}

void zoom_scaleXP(double factor) noexcept {
}

// Zoom command

class RTZoomTracker : public OdEdPointTracker {
	OdGsView* m_View;
	double m_base;
	double m_fw;
	double m_fh;

   public:
	void init(OdGsView* view, const OdGePoint3d& base) {
		m_View = view;
		m_fw = view->fieldWidth();
		m_fh = view->fieldHeight();
		m_base = (m_View->projectionMatrix() * m_View->viewingMatrix() * base).y;
	}

	void setValue(const OdGePoint3d& value) override {
		const auto xWorldToNDC {m_View->projectionMatrix() * m_View->viewingMatrix()};
		const auto pt2 {xWorldToNDC * value};
		auto fac {1. + fabs(pt2.y - m_base) * 1.5};

		if (pt2.y > m_base) {
			fac = 1. / fac;
		}
		m_View->setView(m_View->position(), m_View->target(), m_View->upVector(), m_fw * fac, m_fh * fac, m_View->isPerspective() ? OdGsView::kPerspective : OdGsView::kParallel);
	}
	int addDrawables(OdGsView* view) noexcept override { return 1; }
	void removeDrawables(OdGsView* view) noexcept override {}
};

void OdExZoomCmd::execute(OdEdCommandContext* edCommandContext) {
	OdDbCommandContextPtr pDbCmdCtx(edCommandContext);
	OdDbDatabasePtr pDb = pDbCmdCtx->database();
	OdSmartPtr<OdDbUserIO> pIO = pDbCmdCtx->userIO();
	
	auto Keywords {L"All Center Dynamic Extents Previous Scale Window Object"};
	auto ActiveViewport {pDb->activeViewportId().safeOpenObject(OdDb::kForWrite)};
	OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);

	auto ActiveView {AbstractViewportData->gsView(ActiveViewport)};

	try {
		auto FirstCorner {pIO->getPoint(L"Specify corner of window, enter a scale factor (nX or nXP), or\n[All/Center/Dynamic/Extents/Previous/Scale/Window/Object] <real time>:", OdEd::kInpThrowEmpty | OdEd::kInpThrowOther | OdEd::kGptNoOSnap, nullptr, Keywords)};
		auto OppositeCorner {pIO->getPoint(L"Specify opposite corner:", OdEd::kGptNoUCS | OdEd::kGptRectFrame | OdEd::kGptNoOSnap)};
		zoom_window(FirstCorner, OppositeCorner, ActiveView);
	} catch (const OdEdEmptyInput) // real time
	{
		OdStaticRxObject<RTZoomTracker> tracker;
		for (;;) {
			try {
				tracker.init(ActiveView, pIO->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptBeginDrag | OdEd::kGptNoOSnap));
				pIO->getPoint(L"Press ESC or ENTER to exit."), OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptEndDrag | OdEd::kGptNoOSnap, 0, L"", &tracker;
			} catch (const OdEdCancel) {
				break;
			}
		}
	} catch (const OdEdOtherInput& otherInput) { // scale factor (nX or nXP)
		wchar_t* End;
		const auto scale {odStrToD(otherInput.string(), &End)};

		if (OdString(End).compare(otherInput.string()) > 0) {
			if (OdString(End).iCompare(L"X") == 0) {
				ActiveView->zoom(scale);
			} else if (OdString(End).iCompare(L"XP") == 0) {
				zoom_scaleXP(scale);
			} else if (!*End) {
				ActiveView->zoom(scale);
			}
		}
		pIO->putString(L"Requires a distance, numberX, or option keyword.");
	} catch (const OdEdKeyword& kw) {
		switch (kw.keywordIndex()) {
			case 0: // All
				break;
			case 1: // Center
				break;
			case 2: // Dynamic
				break;
			case 3: // Extents
				zoom_extents(ActiveView, ActiveViewport);
				break;
			case 4: // Previous
				break;
			case 5: // Scale
				break;
			case 6: { // Window
				auto FirstCorner {pIO->getPoint(L"Specify first corner:", OdEd::kGptNoUCS | OdEd::kGptNoOSnap)};
				auto OppositeCorner {pIO->getPoint(L"Specify opposite corner:", OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptRectFrame)};
				zoom_window(FirstCorner, OppositeCorner, ActiveView);
				break;
			}
			case 7: // Object
				break;
		}
	}

	AbstractViewportData->setView(ActiveViewport, ActiveView);
}

// 3d orbit command

const OdString OdEx3dOrbitCmd::groupName() const {
	return globalName();
}

const OdString OdEx3dOrbitCmd::globalName() const {
	return L"3DORBIT";
}

class OrbitCtrl : public OdGiDrawableImpl<> {
   public:
	unsigned long subSetAttributes(OdGiDrawableTraits* drawableTraits) const noexcept override {
		return kDrawableIsAnEntity | kDrawableRegenDraw;
	}
	bool subWorldDraw(OdGiWorldDraw* worldDraw) const noexcept override {
		return false;
	}
	void subViewportDraw(OdGiViewportDraw* viewportDraw) const override {
		const auto& vp = viewportDraw->viewport();
		OdGiGeometry& geom = viewportDraw->geometry();
		viewportDraw->subEntityTraits().setColor(OdCmEntityColor::kACIGreen);
		viewportDraw->subEntityTraits().setFillType(kOdGiFillNever);

		OdGiModelTransformSaver mt(geom, vp.getEyeToModelTransform());
		OdGiDrawFlagsHelper DrawFlagsHelper(viewportDraw->subEntityTraits(), OdGiSubEntityTraits::kDrawNoPlotstyle);

		OdGePoint3d pt1;
		OdGePoint2d pt2;
		vp.getViewportDcCorners(reinterpret_cast<OdGePoint2d&>(pt1), pt2);
		pt2.x -= pt1.x;
		pt2.y -= pt1.y;
		const auto Radius {odmin(pt2.x, pt2.y) / 9. * 7. / 2.};
		reinterpret_cast<OdGePoint2d&>(pt1) += pt2.asVector() / 2.;
		geom.circle(pt1, Radius, OdGeVector3d::kZAxis);

		geom.circle(pt1 + OdGeVector3d(0.0, Radius, 0.0), Radius / 20., OdGeVector3d::kZAxis);
		geom.circle(pt1 + OdGeVector3d(0.0, -Radius, 0.0), Radius / 20., OdGeVector3d::kZAxis);
		geom.circle(pt1 + OdGeVector3d(Radius, 0.0, 0.0), Radius / 20., OdGeVector3d::kZAxis);
		geom.circle(pt1 + OdGeVector3d(-Radius, 0.0, 0.0), Radius / 20., OdGeVector3d::kZAxis);
	}
};

class RTOrbitTracker : public OdEdPointTracker {
	OdGsView* m_View {nullptr};
	OdGePoint3d m_pt;
	OdGiDrawablePtr m_Drawable;
	OdGePoint3d m_Position;
	OdGePoint3d m_Target;
	OdGeVector3d m_UpVector;
	OdGeVector3d m_X;
	OdGePoint3d m_ViewCenter;
	OdGeMatrix3d m_InitialViewingMatrixInverted;
	double m_D {0.0}; // diameter of orbit control in projected coordinates
	OdGsModelPtr m_pModel;

	enum Axis {
		kHorizontal,
		kVertical,
		kPerpDir, // orbit around perpendicular to mouse direction
		kEye,
	} m_axis;

	void viewportDcCorners(OdGePoint2d& lower_left, OdGePoint2d& upper_right) const {
		const auto Target {m_View->viewingMatrix() * m_View->target()};
		const auto HalfFieldWidth {m_View->fieldWidth() / 2.0};
		const auto HalfFieldHeight {m_View->fieldHeight() / 2.0};
		lower_left.x = Target.x - HalfFieldWidth;
		lower_left.y = Target.y - HalfFieldHeight;
		upper_right.x = Target.x + HalfFieldWidth;
		upper_right.y = Target.y + HalfFieldHeight;
	}

   public:
	RTOrbitTracker() = default;

	void reset() noexcept { m_View = nullptr; }
	void init(OdGsView* view, const OdGePoint3d& pt) {
		m_View = view;
		m_Position = view->position();
		m_Target = view->target();
		m_UpVector = view->upVector();
		m_X = m_UpVector.crossProduct(view->target() - m_Position).normal();

		m_InitialViewingMatrixInverted = m_View->viewingMatrix();
		m_pt = m_InitialViewingMatrixInverted * pt;
		m_pt.z = 0.0;
		m_InitialViewingMatrixInverted.invert();

		OdGePoint3d pt1;
		OdGePoint2d pt2;
		viewportDcCorners(reinterpret_cast<OdGePoint2d&>(pt1), pt2);
		pt2.x -= pt1.x;
		pt2.y -= pt1.y;
		const auto r {odmin(pt2.x, pt2.y) / 9. * 7. / 2.};
		m_D = 2.0 * r;
		reinterpret_cast<OdGePoint2d&>(pt1) += pt2.asVector() / 2.;
		const auto r2sqrd {r * r / 400.};

		pt1.y += r;
		if ((pt1 - m_pt).lengthSqrd() <= r2sqrd) {
			m_axis = kHorizontal;
		} else {
			pt1.y -= r;
			pt1.y -= r;
			if ((pt1 - m_pt).lengthSqrd() <= r2sqrd) {
				m_axis = kHorizontal;
			} else {
				pt1.y += r;
				pt1.x += r;
				if ((pt1 - m_pt).lengthSqrd() <= r2sqrd) {
					m_axis = kVertical;
				} else {
					pt1.x -= r;
					pt1.x -= r;
					if ((pt1 - m_pt).lengthSqrd() <= r2sqrd) {
						m_axis = kVertical;
					} else {
						pt1.x += r;
						if ((pt1 - m_pt).lengthSqrd() <= r * r) {
							m_axis = kPerpDir;
						} else {
							m_axis = kEye;
						}
					}
				}
			}
		}
		auto ComputeExtents {true};
		{ // Try to extract cached extents
			OdGsClientViewInfo ClientViewInfo;
			view->clientViewInfo(ClientViewInfo);
			OdDbObjectId spaceId;
			if (!GETBIT(ClientViewInfo.viewportFlags, OdGsClientViewInfo::kDependentGeometry)) {
				spaceId = OdDbDatabasePtr(view->userGiContext()->database())->getModelSpaceId();
			} else {
				spaceId = OdDbDatabasePtr(view->userGiContext()->database())->getPaperSpaceId();
			}
			auto pBTR {spaceId.openObject()};
			OdGeExtents3d wcsExt;
			if (pBTR->gsNode() && pBTR->gsNode()->extents(wcsExt)) {
				m_ViewCenter = wcsExt.center(), ComputeExtents = false;
			}
		}
		if (ComputeExtents) { // Compute extents if no extents cached
			OdAbstractViewPEPtr AbstractView {view};
			OdGeBoundBlock3d BoundBox;
			AbstractView->viewExtents(view, BoundBox);
			m_ViewCenter = BoundBox.center();
			m_ViewCenter.transformBy(m_InitialViewingMatrixInverted);
		}
	}

	double angle(const OdGePoint3d& value) const {
		const auto pt2 {m_View->viewingMatrix() * value};
		auto dist {0.0};

		if (m_axis == kHorizontal) {
			dist = pt2.y - m_pt.y;
		} else if (m_axis == kVertical) {
			dist = pt2.x - m_pt.x;
		}
		return dist * OdaPI / m_D;
	}

	double angleZ(const OdGePoint3d& value) const {
		auto pt2 {m_View->viewingMatrix() * value};
		auto targ {m_View->viewingMatrix() * m_ViewCenter};
		pt2.z = targ.z = m_pt.z;
		return (pt2 - targ).angleTo(m_pt - targ, OdGeVector3d::kZAxis);
	}

	double anglePerp(const OdGePoint3d& value) const {
		auto pt2 {m_View->viewingMatrix() * value};
		pt2.z = 0.0;
		return pt2.distanceTo(m_pt) * OdaPI / m_D;
	}

	void setValue(const OdGePoint3d& value) override {
		if (m_View) {
			OdGeMatrix3d x;
			switch (m_axis) {
				case kHorizontal:
					x.setToRotation(-angle(value), m_X, m_ViewCenter);
					break;
				case kVertical:
					x.setToRotation(-angle(value), m_UpVector, m_ViewCenter);
					break;
				case kEye:
					x.setToRotation(-angleZ(value), m_Target - m_Position, m_ViewCenter);
					break;
				case kPerpDir: {
					auto value1 {value};
					value1.transformBy(m_View->viewingMatrix());
					value1.z = 0.0;
					const auto dir {(value1 - m_pt).convert2d()};
					const auto perp {dir.perpVector()};
					OdGeVector3d perp3d(perp.x, perp.y, 0.0);
					perp3d.normalizeGetLength();
					perp3d.transformBy(m_InitialViewingMatrixInverted);
					x.setToRotation(-anglePerp(value), perp3d, m_ViewCenter);
					break;
				}
			}
			auto newPos {x * m_Position};
			const auto newTarget {x * m_Target};
			auto newPosDir {newPos - newTarget};
			newPosDir.normalizeGetLength();
			newPosDir *= m_Position.distanceTo(m_Target);
			newPos = newTarget + newPosDir;

			m_View->setView(newPos, newTarget, x * m_UpVector, m_View->fieldWidth(), m_View->fieldHeight(), m_View->isPerspective() ? OdGsView::kPerspective : OdGsView::kParallel);
		}
	}

	int addDrawables(OdGsView* pView) override {
		m_Drawable = OdRxObjectImpl<OrbitCtrl>::createObject();
		if (m_pModel.isNull()) {
			m_pModel = pView->device()->createModel();
			if (!m_pModel.isNull()) {
				m_pModel->setRenderType(OdGsModel::kDirect); // Skip Z-buffer for 2d drawables.
				m_pModel->setEnableViewExtentsCalculation(false); // Skip extents calculation.
				m_pModel->setRenderModeOverride(OdGsView::k2DOptimized); // Setup 2dWireframe mode for all underlying geometry.
				auto visualStyleId {GraphTrackerBase::getVisualStyleOverride(pView->userGiContext()->database())};
				if (visualStyleId) m_pModel->setVisualStyle(visualStyleId); // 2dWireframe visual style.
			}
		}
		pView->add(m_Drawable, m_pModel.get());
		return 1;
	}

	void removeDrawables(OdGsView* pView) override {
		pView->erase(m_Drawable);
	}
};

void OdEx3dOrbitCmd::execute(OdEdCommandContext* edCommandContext) {
	OdDbCommandContextPtr CommandContext(edCommandContext);
	OdDbDatabasePtr Database {CommandContext->database()};
	OdSmartPtr<OdDbUserIO> UserIO {CommandContext->userIO()};

	auto ActiveViewport {Database->activeViewportId().safeOpenObject(OdDb::kForWrite)};
	OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);

	auto View {AbstractViewportData->gsView(ActiveViewport)};

	// There is one special case: layout with enabled 'draw viewports first' mode
	{
		if (!Database->getTILEMODE()) {
			OdSmartPtr<OdDbLayout> Layout {Database->currentLayoutId().openObject()};

			if (Layout->drawViewportsFirst()) {
				if (View->device()->viewAt(View->device()->numViews() - 1) == View)
					View = View->device()->viewAt(0);
			}
		}
	}
	//

	auto InteractiveMode {static_cast<OdRxVariantValue>(edCommandContext->arbitraryData(L"Bitmap InteractiveMode"))};
	auto InteractiveFrameRate {static_cast<OdRxVariantValue>(edCommandContext->arbitraryData(L"Bitmap InteractiveFrameRate"))};
	ViewInteractivityMode mode(InteractiveMode, InteractiveFrameRate, View);

	OdStaticRxObject<RTOrbitTracker> OrbitTracker;
	for (;;) {
		try {
			OrbitTracker.init(View, UserIO->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptBeginDrag, nullptr, L"", &OrbitTracker));
			UserIO->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptEndDrag, nullptr, L"", &OrbitTracker);
			OrbitTracker.reset();
		} catch (const OdEdCancel) {
			break;
		}
	}
}

void OdExEditorObject::TurnOrbitOn(bool orbitOn) {
	SETBIT(m_flags, kOrbitOn, orbitOn);
	SetTracker(orbitOn ? OdRxObjectImpl<RTOrbitTracker>::createObject().get() : nullptr);
}

bool OdExEditorObject::OnOrbitBeginDrag(int x, int y) {
	if (IsOrbitOn()) {
		static_cast<RTOrbitTracker*>(m_InputTracker.get())->init(ActiveView(), ToEyeToWorld(x, y));
		return true;
	}
	return false;
}

bool OdExEditorObject::OnOrbitEndDrag(int x, int y) {
	if (IsOrbitOn()) {
		static_cast<RTOrbitTracker*>(m_InputTracker.get())->reset();
		return true;
	}
	return false;
}

bool OdExEditorObject::OnZoomWindowBeginDrag(int x, int y) {
	const auto Point {ToEyeToWorld(x, y)};
	SetTracker(RectFrame::create(Point, GsModel()));
	TrackPoint(Point);
	return true;
}

bool OdExEditorObject::OnZoomWindowEndDrag(int x, int y) {
	zoom_window2(OdEdPointDefTrackerPtr(m_InputTracker)->basePoint(), ToEyeToWorld(x, y), ActiveView());
	SetTracker(nullptr);
	return true;
}


// Dolly command

const OdString OdExDollyCmd::groupName() const {
	return globalName();
}
const OdString OdExDollyCmd::globalName() const {
	return L"DOLLY";
}

class RTDollyTracker : public OdEdPointTracker {
	OdGsView* m_View {nullptr};
	OdGePoint3d m_Point;
	OdGePoint3d m_Position;

   public:
	RTDollyTracker() = default;
	void Reset() noexcept { m_View = nullptr; }

	void Initialize(OdGsView* view, const OdGePoint3d& point) {
		m_View = view;
		m_Position = view->position();
		m_Point = point - m_Position.asVector();
	}

	void setValue(const OdGePoint3d& value) override {
		if (m_View) {
			auto Delta {(m_Point - (value - m_Position)).asVector()};
			m_Point = value - m_Position.asVector();
			Delta.transformBy(m_View->viewingMatrix());
			m_View->dolly(Delta.x, Delta.y, Delta.z);
			m_Position = m_View->position();
		}
	}

	int addDrawables(OdGsView* view) noexcept override { return 0; }
	void removeDrawables(OdGsView* view) noexcept override {}
};

void OdExDollyCmd::execute(OdEdCommandContext* edCommandContext) {
	OdDbCommandContextPtr pDbCmdCtx(edCommandContext);
	OdDbDatabasePtr Database {pDbCmdCtx->database()};
	OdSmartPtr<OdDbUserIO> UserIO {pDbCmdCtx->userIO()};
	auto ActiveViewport {Database->activeViewportId().safeOpenObject(OdDb::kForWrite)};
	OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);

	auto View {AbstractViewportData->gsView(ActiveViewport)};

	// @@@ There is one special case: layout with enabled 'draw viewports first' mode
	{
		if (!Database->getTILEMODE()) {
			OdSmartPtr<OdDbLayout> Layout {Database->currentLayoutId().openObject()};

			if (Layout->drawViewportsFirst()) {
				if (View->device()->viewAt(View->device()->numViews() - 1) == View) {
					View = View->device()->viewAt(0);
				}
			}
		}
	}
	//
	auto InteractiveMode {static_cast<OdRxVariantValue>(edCommandContext->arbitraryData(L"AeSys InteractiveMode"))};
	auto InteractiveFrameRate {static_cast<OdRxVariantValue>(edCommandContext->arbitraryData(L"AeSys InteractiveFrameRate"))};
	ViewInteractivityMode mode(InteractiveMode, InteractiveFrameRate, View);

	OdStaticRxObject<RTDollyTracker> DollyTracker;
	for (;;) {
		try {
			DollyTracker.Initialize(View, UserIO->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptBeginDrag, nullptr, L"", &DollyTracker));
			UserIO->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptEndDrag, nullptr, L"", &DollyTracker);
			DollyTracker.Reset();
		} catch (const OdEdCancel) {
			break;
		}
	}
}

//Interactivity commands
const OdString OdExInteractivityModeCmd::groupName() const {
	return globalName();
}
const OdString OdExInteractivityModeCmd::globalName() const {
	return L"INTERACTIVITY";
}

void OdExInteractivityModeCmd::execute(OdEdCommandContext* edCommandContext) {
	OdDbCommandContextPtr pDbCmdCtx(edCommandContext);
	OdSmartPtr<OdDbUserIO> pIO = pDbCmdCtx->userIO();
	auto enable {pIO->getInt(L"\nSet 0 to disable or non-zero to enable Interactivity Mode: ") != 0};

	if (enable) {
		auto frameRate {pIO->getReal(L"\nSpecify frame rate (Hz): ")};
		edCommandContext->setArbitraryData(L"AeSys InteractiveMode", OdRxVariantValue(true));
		edCommandContext->setArbitraryData(L"AeSys InteractiveFrameRate", OdRxVariantValue(frameRate));
	} else {
		edCommandContext->setArbitraryData(L"AeSys InteractiveMode", OdRxVariantValue(false));
	}
}

//Collision detection commands
const OdString OdExCollideCmd::groupName() const {
	return globalName();
}
const OdString OdExCollideCmd::globalName() const {
	return L"COLLIDE";
}

class OdExCollideGsPath {
	struct Node : OdGiPathNode {
		const Node* m_pParent;
		OdDbStub* m_pId;
		OdGiDrawablePtr m_Drawable;
		OdGsMarker m_Marker;

		const OdGiPathNode* parent() const noexcept override { return m_pParent; }
		OdDbStub* persistentDrawableId() const noexcept override { return m_pId; }
		const OdGiDrawable* transientDrawable() const override { return m_Drawable; }
		OdGsMarker selectionMarker() const noexcept override { return m_Marker; }
	};
	const Node* m_pLeaf {nullptr};

	void add(const OdGiDrawable* drawable, const OdDbObjectId& drawableId, OdGsMarker gsMarker = -1) {
		auto pNode {new Node()};
		pNode->m_pParent = m_pLeaf;
		m_pLeaf = pNode;

		pNode->m_Drawable = drawable;
		pNode->m_pId = drawableId;
		pNode->m_Marker = gsMarker;
	}

	void addNode(OdDbObjectIdArray::const_iterator& iter) {
		auto Drawable {iter->safeOpenObject()};
		addNode(Drawable);
		auto pInsert {OdDbBlockReference::cast(Drawable)};
		if (pInsert.get())
			addNode(pInsert->blockTableRecord());
		++iter;
	}

   public:
	OdExCollideGsPath() = default;
	~OdExCollideGsPath() {
		clear();
	}
	OdExCollideGsPath(const OdDbFullSubentPath& path)  {
		set(path);
	}

	void clear() {
		while (m_pLeaf) {
			auto Node = m_pLeaf;
			m_pLeaf = Node->m_pParent;
			delete Node;
		}
		m_pLeaf = nullptr;
	}

	void set(const OdDbFullSubentPath& path) {
		set(path, kNullSubentIndex);
	}
	void set(const OdDbFullSubentPath& path, OdGsMarker gsMarker) {
		clear();
		const auto& ids {path.objectIds()};

		OdDbObjectIdArray::const_iterator iter {ids.begin()};
		if (iter == ids.end())
			throw OdError(eInvalidInput);
		auto pObj {iter->safeOpenObject()};
		addNode(pObj->ownerId());
		for (; iter != ids.end() - 1; ++iter)
			addNode(*iter);

		addNode(*iter, gsMarker);
	}

	void addNode(const OdDbObjectId& drawableId, OdGsMarker gsMarker = kNullSubentIndex) {
		add(nullptr, drawableId, gsMarker);
	}
	void addNode(const OdGiDrawable* pDrawable, OdGsMarker gsMarker = kNullSubentIndex) {
		add(pDrawable->isPersistent() ? nullptr : pDrawable, pDrawable->id(), gsMarker);
	}

	operator const OdGiPathNode&() const noexcept { return *m_pLeaf; }
};

#define STL_USING_MAP
#include "OdaSTL.h"

class CollideMoveTracker : public OdStaticRxObject<OdEdPointTracker> {
	OdArray<OdDbEntityPtr> m_ents; // Selection set entities
	OdGeMatrix3d m_xForm; // Last transformation

	OdArray<OdExCollideGsPath*> m_pathes;
	OdArray<OdExCollideGsPath*> m_prevHLPathes;

	OdArray<const OdGiPathNode*> inputArray;

   protected:
	OdGePoint3d m_ptBase;
	OdDbDatabasePtr m_pDb;
	OdGsView* m_View;
	OdGsModel* m_pModel;
	bool m_bDynHLT;

	virtual OdGeMatrix3d getTransform(const OdGePoint3d& value) {
		OdGeMatrix3d mRet;
		mRet.setTranslation(value - m_ptBase);
		return mRet;
	}

   public:
	CollideMoveTracker(OdGePoint3d ptBase, OdDbSelectionSet* selectionSet, OdDbDatabasePtr database, OdGsView* view, bool bDynHLT)
		: m_ptBase(ptBase)
		, m_bDynHLT(bDynHLT) {
		m_pDb = database;
		m_View = view;
		OdDbSelectionSetIteratorPtr SelectionSetIterator {selectionSet->newIterator()};
		m_pModel = nullptr;

		//obtain GsModel

		while (!SelectionSetIterator->done()) {
			const auto SelectionSetObject {SelectionSetIterator->objectId()};
			OdDbEntityPtr Entity {SelectionSetObject.openObject(OdDb::kForWrite)};

			if (!m_pModel && Entity->gsNode()) { m_pModel = Entity->gsNode()->model(); }

			if (!Entity.isNull()) {
				OdDbEntityPtr pSubEnt;
				if (SelectionSetIterator->subentCount() == 0) {
					m_ents.push_back(Entity);
				} else {
					OdDbFullSubentPath pathSubent;
					OdDbFullSubentPathArray arrPaths;

					for (unsigned i = 0; i < SelectionSetIterator->subentCount(); i++) {
						SelectionSetIterator->getSubentity(i, pathSubent);
						pSubEnt = Entity->subentPtr(pathSubent);

						if (!pSubEnt.isNull()) { m_ents.push_back(pSubEnt); }
					}
				}
			}
			if (Entity.isNull()) { continue; }

			if (SelectionSetIterator->subentCount() == 0) {
				auto gsPath {new OdExCollideGsPath};
				gsPath->addNode(SelectionSetIterator->objectId().safeOpenObject()->ownerId());
				gsPath->addNode(SelectionSetIterator->objectId());
				m_pathes.push_back(gsPath);
				Entity->dragStatus(OdDb::kDragStart);
			} else {
				for (unsigned i = 0; i < SelectionSetIterator->subentCount(); ++i) {
					OdDbFullSubentPath p;

					if (SelectionSetIterator->getSubentity(i, p)) {
						OdGsMarkerArray gsMarkers;
						Entity->getGsMarkersAtSubentPath(p, gsMarkers);

						if (!gsMarkers.isEmpty()) {
							for (auto& Marker : gsMarkers) {
								auto gsPath {new OdExCollideGsPath};
								gsPath->set(p, Marker);
								m_pathes.push_back(gsPath);
								auto SubEnt {Entity->subentPtr(p)};
								SubEnt->dragStatus(OdDb::kDragStart);
							}
						} else {
							auto gsPath {new OdExCollideGsPath(p)};
							m_pathes.push_back(gsPath);
						}
					}
				}
			}
			SelectionSetIterator->next();
		}
		for (auto& Path : m_pathes) {
			m_pModel->highlight(Path->operator const OdGiPathNode&(), false);
			inputArray.push_back(&Path->operator const OdGiPathNode&());
		}
	}

	virtual ~CollideMoveTracker() {
		if (!m_prevHLPathes.empty()) {
			for (auto& PreviousPath : m_prevHLPathes) {
				m_pModel->highlight(PreviousPath->operator const OdGiPathNode&(), false);
				delete PreviousPath;
			}
			m_prevHLPathes.clear();
		}
		inputArray.clear();
		for (auto& Path : m_pathes) {
			delete Path;
		}
		m_pathes.clear();
		m_View->invalidate();
		m_View->update();
	}

	void setValue(const OdGePoint3d& value) override {
		const auto matNewTransform = getTransform(value);
		// Compensate previous transform
		auto xTrans {m_xForm.inverse()};
		xTrans.preMultBy(matNewTransform);
		// Remember last transform
		m_xForm = matNewTransform;
		for (int i = m_ents.size() - 1; i >= 0; --i) {
			m_ents[i]->transformBy(xTrans);
		}
		doCollideWithAll();
	}

	virtual void doCollideWithAll();

	virtual void highlight(OdArray<OdExCollideGsPath*>& newPathes);

	int addDrawables(OdGsView* pView) override {
		for (int i = m_ents.size() - 1; i >= 0; --i) {
			pView->add(m_ents[i], nullptr);
		}
		return 1;
	}

	void removeDrawables(OdGsView* pView) override {
		for (int i = m_ents.size() - 1; i >= 0; --i) {
			pView->erase(m_ents[i]);
		}
	}
};


bool addNodeToPath(OdExCollideGsPath* result, const OdGiPathNode* pPath, bool bTruncateToRef = false) {
	auto bAdd {true};
	if (pPath->parent()) {
		bAdd = addNodeToPath(result, pPath->parent(), bTruncateToRef);
	}
	if (bAdd) {
		result->addNode(pPath->persistentDrawableId() ? pPath->persistentDrawableId() : pPath->transientDrawable()->id(), bTruncateToRef ? 0 : pPath->selectionMarker());
		if (bTruncateToRef && pPath->persistentDrawableId()) {
			const OdDbObjectId id(pPath->persistentDrawableId());
			auto pObj {id.safeOpenObject()};
			if (!pObj.isNull()) {
				if (pObj->isKindOf(OdDbBlockReference::desc())) { bAdd = false; }
			}
		}
	}
	return bAdd;
}

OdExCollideGsPath* fromGiPath(const OdGiPathNode* path, bool bTruncateToRef = false) {
	if (!path) { return nullptr; }

	auto res {new OdExCollideGsPath};
	addNodeToPath(res, path, bTruncateToRef);
	return res;
}

void CollideMoveTracker::doCollideWithAll() {
	class OdExCollisionDetectionReactor : public OdGsCollisionDetectionReactor {
		OdArray<OdExCollideGsPath*> m_pathes;
		bool m_bDynHLT;

	   public:
		OdExCollisionDetectionReactor(bool bDynHLT)
			: m_bDynHLT(bDynHLT) {}

		~OdExCollisionDetectionReactor() = default;

		unsigned long collisionDetected(const OdGiPathNode* /*pPathNode1*/, const OdGiPathNode* pPathNode2) override {
			auto p {fromGiPath(pPathNode2, !m_bDynHLT)};

			if (p || pPathNode2->persistentDrawableId()) { 	m_pathes.push_back(p); }

			return static_cast<unsigned long>(kContinue);
		}

		OdArray<OdExCollideGsPath*>& pathes() { return m_pathes; }
	};

	OdExCollisionDetectionReactor reactor(m_bDynHLT);

	m_View->collide(inputArray.asArrayPtr(), inputArray.size(), &reactor, nullptr, 0);


	highlight(reactor.pathes());
}

void CollideMoveTracker::highlight(OdArray<OdExCollideGsPath*>& newPaths) {
	if (!m_prevHLPathes.empty()) { // Unhighlight old paths
		for (auto& PreviousPath : m_prevHLPathes) {
			m_pModel->highlight(PreviousPath->operator const OdGiPathNode&(), false);
			delete PreviousPath;
		}
		m_prevHLPathes.clear();
	}
	for (auto& NewPath : newPaths) {
		m_pModel->highlight(NewPath->operator const OdGiPathNode&(), true);
		m_prevHLPathes.push_back(NewPath);
	}
}

void OdExCollideCmd::execute(OdEdCommandContext* edCommandContext) {
	class OdExTransactionSaver {
		OdDbDatabasePtr m_pDb;
		bool m_bInTransaction;

	   public:
		OdExTransactionSaver(OdDbDatabasePtr pDb) {
			m_pDb = pDb;
			m_bInTransaction = false;
		}

		~OdExTransactionSaver() {
			if (m_bInTransaction) {
				m_pDb->abortTransaction();
				m_bInTransaction = false;
			}
		}

		void startTransaction() {
			if (m_bInTransaction) {
				m_pDb->abortTransaction();
			}
			m_bInTransaction = true;
			m_pDb->startTransaction();
		}
	};

	OdDbCommandContextPtr CommandContext(edCommandContext);
	OdSmartPtr<OdDbUserIO> UserIO {CommandContext->userIO()};
	OdDbDatabasePtr Database {CommandContext->database()};

	auto dynHlt {static_cast<OdRxVariantValue>(edCommandContext->arbitraryData(L"DynamicSubEntHlt"))};
	const auto bDynHLT {static_cast<bool>(dynHlt)};

	//Get active view
	OdGsView* View {nullptr};

	if (!Database.isNull()) {
		auto ActiveViewport {Database->activeViewportId().safeOpenObject()};
		OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);

		if (!AbstractViewportData.isNull() && AbstractViewportData->gsView(ActiveViewport)) {
			View = AbstractViewportData->gsView(ActiveViewport);
		}
	}
	if (!View) {
		ODA_ASSERT(false);
		throw OdEdCancel();
	}

	OdDbSelectionSetPtr SelectionSet {UserIO->select(L"Collide: Select objects to be checked:", OdEd::kSelAllowObjects | OdEd::kSelAllowSubents | OdEd::kSelLeaveHighlighted)};

	if (!SelectionSet->numEntities()) { throw OdEdCancel(); }

	OdExTransactionSaver saver(Database);
	saver.startTransaction();

	const auto BasePoint {UserIO->getPoint(L"Collide: Specify base point:")};

	CollideMoveTracker tracker(BasePoint, SelectionSet, Database, View, bDynHLT);
	const auto ptOffset {UserIO->getPoint(L"Collide: Specify second point:", OdEd::kGdsFromLastPoint | OdEd::kGptRubberBand, nullptr, L"", &tracker)};
}


//Collision detection commands
const OdString OdExCollideAllCmd::groupName() const {
	return globalName();
}
const OdString OdExCollideAllCmd::globalName() const {
	return L"COLLIDEALL";
}

void OdExCollideAllCmd::execute(OdEdCommandContext* edCommandContext) {
	class OdExCollisionDetectionReactor : public OdGsCollisionDetectionReactor {
		OdArray<OdExCollideGsPath*> m_pathes;
		bool m_bDynHLT;

	   public:
		OdExCollisionDetectionReactor(bool bDynHLT)
			: m_bDynHLT(bDynHLT) {}

		~OdExCollisionDetectionReactor() = default;

		unsigned long collisionDetected(const OdGiPathNode* pPathNode1, const OdGiPathNode* pPathNode2) override {
			auto p1 {fromGiPath(pPathNode1, !m_bDynHLT)};
			auto p2 {fromGiPath(pPathNode2, !m_bDynHLT)};
			m_pathes.push_back(p1);
			m_pathes.push_back(p2);
			return static_cast<unsigned long>(kContinue);
		}

		OdArray<OdExCollideGsPath*>& pathes() { return m_pathes; }
	};

	OdDbCommandContextPtr CommandContext(edCommandContext);
	OdSmartPtr<OdDbUserIO> UserIO {CommandContext->userIO()};
	OdDbDatabasePtr Database {CommandContext->database()};

	//Get active view
	OdGsView* View {nullptr};

	if (!Database.isNull()) {
		auto ActiveViewport {Database->activeViewportId().safeOpenObject()};
		OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);

		if (!AbstractViewportData.isNull() && AbstractViewportData->gsView(ActiveViewport)) {
			View = AbstractViewportData->gsView(ActiveViewport);
		}
	}
	if (!View) {
		ODA_ASSERT(false);
		throw OdEdCancel();
	}
	auto Model {View->getModelList()[0]};
	auto Choice {UserIO->getInt(L"Input 1 to detect only intersections, any other to detect all", 0, 0)};

	OdGsCollisionDetectionContext CollisionDetectionContext;
	CollisionDetectionContext.setIntersectionOnly(Choice == 1);

	auto dynHlt {static_cast<OdRxVariantValue>(edCommandContext->arbitraryData(L"DynamicSubEntHlt"))};
	const auto bDynHLT {static_cast<bool>(dynHlt)};

	OdExCollisionDetectionReactor reactor(dynHlt);

	View->collide(nullptr, 0, &reactor, nullptr, 0, &CollisionDetectionContext);

	OdArray<OdExCollideGsPath*>& ReactorPaths {reactor.pathes()};
	for (auto& ReactorPath : ReactorPaths) {
		const auto PathNode {&ReactorPath->operator const OdGiPathNode&()};
		Model->highlight(*PathNode);
		//delete ReactorPath;
	}
	UserIO->getInt(L"Specify any number to exit", 0, 0);
	for (auto& ReactorPath : ReactorPaths) {
		const auto PathNode {&ReactorPath->operator const OdGiPathNode&()};
		Model->highlight(*PathNode, false);
		delete ReactorPath;
	}
	ReactorPaths.clear();
}

void OdExEditorObject::SetTracker(OdEdInputTracker* inputTracker) {
	if (m_InputTracker.get()) {
		m_InputTracker->removeDrawables(ActiveTopView());
	}
	m_InputTracker = inputTracker;

	m_BasePt = nullptr;

	if (inputTracker) {
		SETBIT(m_flags, kTrackerHasDrawables, inputTracker->addDrawables(ActiveTopView()) != 0);
		auto PointDefTracker {OdEdPointDefTracker::cast(inputTracker)};

		if (PointDefTracker.get()) {
			m_basePt = PointDefTracker->basePoint();
			m_BasePt = &m_basePt;
		}
	} else {
		SETBIT(m_flags, kTrackerHasDrawables, false);
	}
}

bool OdExEditorObject::TrackString(const OdString& value) {
	if (m_InputTracker.get()) {
		ODA_ASSERT(m_InputTracker->isKindOf(OdEdStringTracker::desc()));
		static_cast<OdEdStringTracker*>(m_InputTracker.get())->setValue(value);
		return GETBIT(m_flags, kTrackerHasDrawables);
	}
	return false;
}

bool OdExEditorObject::TrackPoint(const OdGePoint3d& point) {
	if (m_InputTracker.get()) {
		ODA_ASSERT(m_InputTracker->isKindOf(OdEdPointTracker::desc()));
		static_cast<OdEdPointTracker*>(m_InputTracker.get())->setValue(point);
		return GETBIT(m_flags, kTrackerHasDrawables);
	}
	return false;
}

bool OdExEditorObject::HasDatabase() const {
	return m_CommandContext->baseDatabase() != nullptr;
}
