// From Examples\Editor\EditorObject.cpp (last compare 20.5)
#include <map>
#include <OdaCommon.h>
#include <Ge/GeRay3d.h>
#include <Gi/GiDrawableImpl.h>
#include <Gi/GiWorldDraw.h>
#include <Gs/Gs.h>
#include <Gs/GsBaseVectorizer.h>
#include <GiContextForDbDatabase.h>
#include <DbLayout.h>
#include <DbCommandContext.h>
#include <DbAbstractViewportData.h>
#include <DbViewport.h>
#include <DbBlockTableRecord.h>
#include <DbViewportTable.h>
#include <DbDictionary.h>
#include <DbVisualStyle.h>
#include <DbHostAppServices.h>
#include <OdDToStr.h>
#include <ExTrackers.h>
#include <RxVariantValue.h>
#include <Gs/GsModel.h>
#include <Gi/GiPathNode.h>
#include <DbBlockReference.h>
#include "EditorObject.h"

class ViewInteractivityMode {
	bool m_Enabled;
	OdGsView* m_View;
public:
	ViewInteractivityMode(OdRxVariantValue enable, OdRxVariantValue frameRate, OdGsView* view) {
		m_Enabled = false;
		m_View = view;
		if (!enable.isNull()) {
			m_Enabled = static_cast<bool>(enable);
			if (m_Enabled && !frameRate.isNull()) {
				const auto Rate {frameRate.get()->getDouble()};
				view->beginInteractivity(Rate);
			}
		}
	}

	~ViewInteractivityMode() {
		if (m_Enabled) { m_View->endInteractivity(); }
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

void SetWorkingSelectionSet(OdDbCommandContext* commandContext, OdDbSelectionSet* selectionSet) {
	commandContext->setArbitraryData(L"AeSys Working Selection Set", selectionSet);
}

OdDbSelectionSetPtr WorkingSelectionSet(OdDbCommandContext* commandContext) {
	OdDbSelectionSetPtr SelectionSet;
	if (commandContext != nullptr) {
		SelectionSet = commandContext->arbitraryData(L"AeSys Working Selection Set");
		if (SelectionSet.isNull()) {
			SelectionSet = OdDbSelectionSet::createObject(commandContext->database());
			SetWorkingSelectionSet(commandContext, SelectionSet);
		}
	}
	return SelectionSet;
}

class XFormDrawable : public OdGiDrawableImpl<OdGiDrawable> {
	OdGiDrawablePtr m_Drawable;
	const OdGeMatrix3d* m_TransForm {nullptr};
protected:
	XFormDrawable() = default;

public:
	static OdGiDrawablePtr CreateObject(OdGiDrawable* drawable, const OdGeMatrix3d& transform) {
		auto Resource {OdRxObjectImpl<XFormDrawable>::createObject()};
		Resource->m_Drawable = drawable;
		Resource->m_TransForm = &transform;
		return Resource;
	}

	unsigned long subSetAttributes(OdGiDrawableTraits* /*drawableTraits*/) const noexcept override {
		return kDrawableUsesNesting;
	}

	bool subWorldDraw(OdGiWorldDraw* worldDraw) const override {
		OdGiModelTransformSaver ModelTransformSaver(worldDraw->geometry(), *m_TransForm);
		worldDraw->geometry().draw(m_Drawable);
		return true;
	}

	void subViewportDraw(OdGiViewportDraw* /*viewportDraw*/) const noexcept override {
	}
};

void OdExEditorObject::Initialize(OdGsDevice* device, OdDbCommandContext* commandContext) {
	m_LayoutHelper = device;
	m_CommandContext = commandContext;
	m_2dModel = device->createModel();
	if (!m_2dModel.isNull()) {
		m_2dModel->setRenderType(OdGsModel::kDirect); // Skip Z-buffer for 2d drawables.
		m_2dModel->setEnableViewExtentsCalculation(false); // Skip extents calculation.
		m_2dModel->setRenderModeOverride(OdGsView::k2DOptimized); // Setup 2dWireframe mode for all underlying geometry.
		m_2dModel->setVisualStyle(OdDbDictionary::cast(m_CommandContext->database()->getVisualStyleDictionaryId().openObject())->getAt(OdDb::kszVS2DWireframe));
	}
	m_GripManager.Initialize(device, m_2dModel, commandContext, WorkingSelectionSet);
	SetEntityCenters();
}

void OdExEditorObject::Uninitialize() {
	auto SelectionSet {GetWorkingSelectionSet()};
	if (SelectionSet.get() != nullptr) {
		SelectionSet->clear();
		m_GripManager.SelectionSetChanged(SelectionSet);
	}
	m_GripManager.Uninitialize();
	m_LayoutHelper.release();
	m_CommandContext = nullptr;
}

void OdExEditorObject::InitializeSnapping(OdGsView* view, OdEdInputTracker* inputTracker) {
	m_ObjectSnapManager.Track(inputTracker);
	view->add(&m_ObjectSnapManager, m_2dModel);
}

void OdExEditorObject::UninitializeSnapping(OdGsView* view) {
	view->erase(&m_ObjectSnapManager);
	m_ObjectSnapManager.Track(nullptr);
}

OdDbSelectionSetPtr OdExEditorObject::GetWorkingSelectionSet() const {
	return WorkingSelectionSet(m_CommandContext);
}

void OdExEditorObject::SetWorkingSelectionSet(OdDbSelectionSet* selectionSet) const {
	::SetWorkingSelectionSet(m_CommandContext, selectionSet);
}

void OdExEditorObject::SelectionSetChanged() {
	m_GripManager.SelectionSetChanged(GetWorkingSelectionSet());
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
			if (!AbstractViewportData.isNull() && AbstractViewportData->gsView(ActiveViewport) != nullptr) {
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
	if (!OdZero(Elevation)) { ucsOrigin += ucsXAxis.crossProduct(ucsYAxis) * Elevation; }
	plane.set(ucsOrigin, ucsXAxis, ucsYAxis);
}

OdGePoint3d OdExEditorObject::ToEyeToWorld(const int x, const int y) const {
	OdGePoint3d wcsPoint(x, y, 0.0);
	const auto View {ActiveView()};
	if (View->isPerspective()) { wcsPoint.z = View->projectionMatrix()(2, 3); }
	wcsPoint.transformBy((View->screenMatrix() * View->projectionMatrix()).inverse());
	wcsPoint.z = 0.0;
	// Eye coordinate system at this point.
	wcsPoint.transformBy(OdAbstractViewPEPtr(View)->eyeToWorld(View));
	return wcsPoint;
}

bool OdExEditorObject::ToUcsToWorld(OdGePoint3d& wcsPoint) const {
	const auto View {ActiveView()};
	OdGePlane Plane;
	UcsPlane(Plane);
	if (!View->isPerspective()) { // For orthogonal projection we simply check intersection between viewing direction and UCS plane.
		const OdGeLine3d Line(wcsPoint, OdAbstractViewPEPtr(View)->direction(View));
		return Plane.intersectWith(Line, wcsPoint);
	}
	// For perspective projection we emit ray from viewer position through WCS point.
	const auto FocalLength {-1.0 / View->projectionMatrix()(3, 2)};
	const auto ViewerPosition {View->target() + OdAbstractViewPEPtr(View)->direction(View).normal() * FocalLength};
	const OdGeRay3d Ray(ViewerPosition, wcsPoint);
	return Plane.intersectWith(Ray, wcsPoint);
}

OdGePoint3d OdExEditorObject::ToScreenCoordinates(const int x, const int y) const {
	OdGePoint3d ScreenPoint(x, y, 0.0);
	const auto View {ActiveView()};
	ScreenPoint.transformBy((View->screenMatrix() * View->projectionMatrix()).inverse());
	ScreenPoint.z = 0.0;
	return ScreenPoint;
}

OdGePoint3d OdExEditorObject::ToScreenCoordinates(const OdGePoint3d& worldPoint) const {
	const auto View {ActiveView()};
	OdGsClientViewInfo ClientViewInfo;
	View->clientViewInfo(ClientViewInfo);
	OdRxObjectPtr Viewport {OdDbObjectId(ClientViewInfo.viewportObjectId).openObject()};
	OdAbstractViewPEPtr AbstractView(Viewport);
	const auto UpVector {AbstractView->upVector(Viewport)};
	const auto Direction {AbstractView->direction(Viewport)};
	const auto XAxis {UpVector.crossProduct(Direction).normal()};
	const auto ViewOffset {AbstractView->viewOffset(Viewport)};
	const auto Target {AbstractView->target(Viewport) - XAxis * ViewOffset.x - UpVector * ViewOffset.y};
	return {XAxis.dotProduct(worldPoint - Target), UpVector.dotProduct(worldPoint - Target), 0.0};
}

bool OdExEditorObject::OnSize(unsigned /*flags*/, const int w, const int h) {
	if (m_LayoutHelper.get() != nullptr) {
		m_LayoutHelper->onSize(OdGsDCRect(0, w, h, 0));
		return true;
	}
	return false;
}

bool OdExEditorObject::OnPaintFrame(unsigned /*flags*/, OdGsDCRect* updatedRectangle) {
	if (m_LayoutHelper.get() != nullptr && !m_LayoutHelper->isValid()) {
		m_LayoutHelper->update(updatedRectangle);
		return true;
	}
	return false;
}

unsigned OdExEditorObject::GetSnapModes() const {
	return m_ObjectSnapManager.SnapModes();
}

void OdExEditorObject::SetSnapModes(const bool snapOn, const unsigned snapModes) {
	snapOn ? (m_Flags |= kSnapOn) : m_Flags &= ~kSnapOn;
	m_ObjectSnapManager.SetSnapModes(snapModes);
}

OdEdCommandPtr OdExEditorObject::Command(const OdString& commandName) const {
	if (commandName == m_cmd_ZOOM.globalName()) { return &m_cmd_ZOOM; }
	if (commandName == m_cmd_3DORBIT.globalName()) { return &m_cmd_3DORBIT; }
	if (commandName == m_cmd_DOLLY.globalName()) { return &m_cmd_DOLLY; }
	if (commandName == m_cmd_INTERACTIVITY.globalName()) { return &m_cmd_INTERACTIVITY; }
	if (commandName == m_cmd_COLLIDE.globalName()) { return &m_cmd_COLLIDE; }
	if (commandName == m_cmd_COLLIDE_ALL.globalName()) { return &m_cmd_COLLIDE_ALL; }
	return OdEdCommandPtr();
}

void OdExEditorObject::Set3DView(const _3DViewType type) {
	const auto Target {OdGePoint3d::kOrigin};
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
		case k3DViewSouthWest:
			Position = OdGePoint3d::kOrigin + OdGeVector3d(-1.0, -1.0, 1.0);
			Axis = OdGeVector3d(0.5, 0.5, 1.0).normal();
			break;
		case k3DViewSouthEast:
			Position = OdGePoint3d::kOrigin + OdGeVector3d(1.0, -1.0, 1.0);
			Axis = OdGeVector3d(-0.5, 0.5, 1.0).normal();
			break;
		case k3DViewNorthEast:
			Position = OdGePoint3d::kOrigin + OdGeVector3d(1.0, 1.0, 1.0);
			Axis = OdGeVector3d(-0.5, -0.5, 1.0).normal();
			break;
		case k3DViewNorthWest:
			Position = OdGePoint3d::kOrigin + OdGeVector3d(-1.0, 1.0, 1.0);
			Axis = OdGeVector3d(0.5, -0.5, 1.0).normal();
			break;
	}
	Unselect();
	auto View {ActiveView()};
	OdGsClientViewInfo ClientViewInfo;
	View->clientViewInfo(ClientViewInfo);
	auto Viewport {OdDbObjectId(ClientViewInfo.viewportObjectId).safeOpenObject(OdDb::kForWrite)};
	OdAbstractViewPEPtr(Viewport)->setUcs(Viewport, Target, Axis.crossProduct(Position.asVector()), Axis);
	View->setView(Position, Target, Axis, View->fieldWidth(), View->fieldHeight(), View->isPerspective() ? OdGsView::kPerspective : OdGsView::kParallel);
}

bool OdExEditorObject::Snap(OdGePoint3d& point, const OdGePoint3d* /*lastPoint*/) {
	if (IsSnapOn()) {
		if (m_ObjectSnapManager.Snap(ActiveView(), point, m_BasePt)) {
			if (!m_2dModel.isNull()) { m_2dModel->onModified(&m_ObjectSnapManager, static_cast<OdGiDrawable*>(nullptr)); }
			return true;
		}
	}
	return false;
}

bool OdExEditorObject::Unselect() {
	auto Result {false};
	auto SelectionSet {GetWorkingSelectionSet()};
	OdDbSelectionSetIteratorPtr SelectionSetIterator {SelectionSet->newIterator()};
	while (!SelectionSetIterator->done()) {
		auto Entity {OdDbEntity::cast(SelectionSetIterator->objectId().openObject())};
		if (Entity.get() != nullptr) {
			Entity->highlight(false);
			Result = true;
		}
		SelectionSetIterator->next();
	}
	// Don't clear working selection set 'SelectionSet->clear()' to prevent previous selection modification
	SelectionSet = OdDbSelectionSet::createObject(SelectionSet->database());
	::SetWorkingSelectionSet(m_CommandContext, SelectionSet);
	m_GripManager.SelectionSetChanged(SelectionSet);
	return Result;
}

bool OdExEditorObject::OnCtrlClick() const {
	return m_GripManager.OnControlClick();
}

void OdExEditorObject::OnDestroy() {
	m_LayoutHelper.release();
	m_2dModel.release();
	m_CommandContext = nullptr;
}

bool OdExEditorObject::OnMouseLeftButtonClick(const unsigned flags, const int x, const int y, OleDragCallback* dragCallback) {
	const auto ShiftIsDown {(OdEdBaseIO::kShiftIsDown & flags) != 0};
	const auto ControlIsDown {(OdEdBaseIO::kControlIsDown & flags) != 0};
	const auto WorldPoint {ToEyeToWorld(x, y)};
	if (m_GripManager.OnMouseDown(x, y, ShiftIsDown)) { return true; }
	try {
		if (dragCallback != nullptr && !ShiftIsDown) {
			auto SelectionSet {GetWorkingSelectionSet()};
			const auto SubEntitiesSelectionMode {ControlIsDown ? OdDbVisualSelection::kEnableSubents : OdDbVisualSelection::kDisableSubents};
			auto SelectionSetAtPoint {OdDbSelectionSet::select(ActiveViewportId(), 1, &WorldPoint, OdDbVisualSelection::kPoint, SubEntitiesSelectionMode)};
			OdDbSelectionSetIteratorPtr SelectionSetAtPointIterator {SelectionSetAtPoint->newIterator()};
			while (!SelectionSetAtPointIterator->done()) {
				if (SelectionSet->isMember(SelectionSetAtPointIterator->objectId()) && !ControlIsDown) {
					SelectionSetAtPointIterator.release();
					break;
				}
				SelectionSetAtPointIterator->next();
			}
			if (SelectionSetAtPointIterator.isNull()) {
				if (dragCallback->BeginDragCallback(WorldPoint)) {
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
	auto UserIo {m_CommandContext->dbUserIO()};
	UserIo->setLASTPOINT(WorldPoint);
	UserIo->setPickfirst(nullptr);
	auto SelectOptions {OdEd::kSelPickLastPoint | OdEd::kSelSinglePass | OdEd::kSelLeaveHighlighted | OdEd::kSelAllowInactSpaces};
	if (HasDatabase()) {
		if (ShiftIsDown) {
			if (m_CommandContext->database()->appServices()->getPICKADD() > 0) { SelectOptions |= OdEd::kSelRemove; }
		} else {
			if (m_CommandContext->database()->appServices()->getPICKADD() == 0) { Unselect(); }
		}
	}
	if (ControlIsDown) { SelectOptions |= OdEd::kSelAllowSubents; }
	const auto SavedSnapMode {IsSnapOn()};
	try {
		SetSnapOn(false);
		OdDbSelectionSetPtr SelectionSet {UserIo->select(OdString::kEmpty, SelectOptions, GetWorkingSelectionSet())};
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

bool OdExEditorObject::OnMouseLeftButtonDoubleClick(unsigned /*flags*/, const int x, const int y) {
	const auto View {ActiveView()};
	m_LayoutHelper->setActiveViewport(OdGePoint2d(x, y));
	const auto Changed {View != ActiveView()};
	if (Changed) {
		auto ActiveViewport {ActiveViewportId().safeOpenObject()};
		const auto Database {ActiveViewport->database()};
		if (Database->getTILEMODE()) {
			OdDbViewportTable::cast(Database->getViewportTableId().safeOpenObject(OdDb::kForWrite))->SetActiveViewport(ActiveViewportId());
		} else {
			OdDbLayout::cast(OdDbBlockTableRecord::cast(Database->getPaperSpaceId().safeOpenObject())->getLayoutId().safeOpenObject(OdDb::kForWrite))->setActiveViewportId(ActiveViewportId());
		}
		Unselect();
	}
	return Changed;
}

bool OdExEditorObject::OnMouseRightButtonDoubleClick(unsigned /*flags*/, int /*x*/, int /*y*/) {
	Unselect();
	auto View {ActiveView()};

	// set target to center of the scene, keep view direction:
	const auto Target {View->target()};
	View->setView(Target + OdGeVector3d::kZAxis, Target, OdGeVector3d::kYAxis, View->fieldWidth(), View->fieldHeight());
	return true;
}

bool OdExEditorObject::OnMouseMove(const unsigned flags, const int x, const int y) {
	const auto ShiftIsDown {(OdEdBaseIO::kShiftIsDown & flags) != 0};
	return m_GripManager.OnMouseMove(x, y, ShiftIsDown);
}

void OdExEditorObject::Dolly(const int x, const int y) {
	const auto View {ActiveView()};
	Dolly(View, x, y);
}

void OdExEditorObject::Dolly(OdGsView* view, const int x, const int y) {
	OdGeVector3d TranslationToOrigin(-x, -y, 0.0);
	TranslationToOrigin.transformBy((view->screenMatrix() * view->projectionMatrix()).inverse());
	view->dolly(TranslationToOrigin);
}

bool OdExEditorObject::OnMouseWheel(unsigned /*flags*/, const int x, const int y, const short zDelta) {
	const auto View {ActiveView()};
	ZoomAt(View, x, y, zDelta);
	if (!m_2dModel.isNull()) { m_2dModel->invalidate(ActiveTopView()); }
	return true;
}

void OdExEditorObject::ZoomAt(OdGsView* view, const int x, const int y, const short zDelta) {
	auto ViewPosition {view->position()};
	ViewPosition.transformBy(view->worldToDeviceMatrix());

	// In 2d mode perspective zoom change lens length instead of fieldWidth/fieldHeight. This is non-standard mode. Practically 2d mode can't be perspective.
	if (view->isPerspective() && view->mode() == OdGsView::k2DOptimized) {
		ViewPosition = OdGePoint3d(0.5, 0.5, 0.0).transformBy(view->screenMatrix());
	}
	const auto X {x - static_cast<int>(OdRound(ViewPosition.x))};
	const auto Y {y - static_cast<int>(OdRound(ViewPosition.y))};
	Dolly(view, -X, -Y);
	view->zoom(zDelta > 0 ? 1.0 / 0.9 : 0.9);
	Dolly(view, X, Y);
}

const OdString OdExZoomCmd::groupName() const {
	return globalName();
}

const OdString OdExZoomCmd::globalName() const {
	return L"ZOOM";
}

void ZoomWindow(OdGePoint3d& firstCorner, OdGePoint3d& oppositeCorner, OdGsView* view) {
	const auto WorldToEyeTransform {OdAbstractViewPEPtr(view)->worldToEye(view)};
	firstCorner.transformBy(WorldToEyeTransform);
	oppositeCorner.transformBy(WorldToEyeTransform);
	auto Diagonal {oppositeCorner - firstCorner};
	if (OdNonZero(Diagonal.x) && OdNonZero(Diagonal.y)) {
		auto NewPosition {firstCorner + Diagonal / 2.0};
		Diagonal.x = fabs(Diagonal.x);
		Diagonal.y = fabs(Diagonal.y);
		view->dolly(NewPosition.asVector());
		const auto FieldWidth {view->fieldWidth() / Diagonal.x};
		const auto FieldHeight {view->fieldHeight() / Diagonal.y};
		view->zoom(odmin(FieldWidth, FieldHeight));
	}
}

void ZoomWindow2(const OdGePoint3d& pt1, const OdGePoint3d& pt2, OdGsView* pView) {
	auto pt1c {pt1};
	auto pt2c {pt2};
	ZoomWindow(pt1c, pt2c, pView);
}

void ZoomScale(double /*factor*/) noexcept {
}

static bool GetLayoutExtents(const OdDbObjectId& spaceId, const OdGsView* view, OdGeBoundBlock3d& boundBox) {
	OdDbBlockTableRecordPtr Space {spaceId.safeOpenObject()};
	OdSmartPtr<OdDbLayout> Layout {Space->getLayoutId().safeOpenObject()};
	OdGeExtents3d Extents;
	if (Layout->getGeomExtents(Extents) == eOk) {
		Extents.transformBy(view->viewingMatrix());
		boundBox.set(Extents.minPoint(), Extents.maxPoint());
		return Extents.minPoint() != Extents.maxPoint();
	}
	return false;
}

void ZoomExtents(OdGsView* view, OdDbObject* viewportObject) {
	const auto Database {viewportObject->database()};
	OdAbstractViewPEPtr AbstractView(view);
	OdGeBoundBlock3d BoundBox;
	auto ValidBoundBox {AbstractView->viewExtents(view, BoundBox)};

	// paper space overall view
	auto Viewport {OdDbViewport::cast(viewportObject)};
	if (Viewport.get() != nullptr && Viewport->number() == 1) {
		if (!ValidBoundBox || !(BoundBox.minPoint().x < BoundBox.maxPoint().x && BoundBox.minPoint().y < BoundBox.maxPoint().y)) {
			ValidBoundBox = GetLayoutExtents(Database->getPaperSpaceId(), view, BoundBox);
		}
	} else if (!ValidBoundBox) { // model space viewport
		ValidBoundBox = GetLayoutExtents(Database->getPaperSpaceId(), view, BoundBox);
	}
	if (!ValidBoundBox) { // set to somewhat reasonable (e.g. paper size)
		if (Database->getMEASUREMENT() == OdDb::kMetric) {
			BoundBox.set(OdGePoint3d::kOrigin, OdGePoint3d(297.0, 210.0, 0.0)); // set to paper size ISO A4 (portrait)
		} else {
			BoundBox.set(OdGePoint3d::kOrigin, OdGePoint3d(11.0, 8.5, 0.0)); // ANSI A (8.50 x 11.00) (landscape)
		}
		BoundBox.transformBy(view->viewingMatrix());
	}
	AbstractView->zoomExtents(view, &BoundBox);
}

void ZoomScaleXp(double /*factor*/) noexcept {
}

// Zoom command
class RTZoomTracker : public OdEdPointTracker {
	OdGsView* m_View {nullptr};
	double m_Base {0.0};
	double m_FieldWidth {0.0};
	double m_FieldHeight {0.0};
public:
	void Initialize(OdGsView* view, const OdGePoint3d& base) {
		m_View = view;
		m_FieldWidth = view->fieldWidth();
		m_FieldHeight = view->fieldHeight();
		m_Base = (m_View->projectionMatrix() * m_View->viewingMatrix() * base).y;
	}

	void setValue(const OdGePoint3d& value) override {
		const auto WorldToNdcTransform {m_View->projectionMatrix() * m_View->viewingMatrix()};
		const auto ndcPoint {WorldToNdcTransform * value};
		auto FieldScaleFactor {1.0 + fabs(ndcPoint.y - m_Base) * 1.5};
		if (ndcPoint.y > m_Base) { FieldScaleFactor = 1.0 / FieldScaleFactor; }
		m_View->setView(m_View->position(), m_View->target(), m_View->upVector(), m_FieldWidth * FieldScaleFactor, m_FieldHeight * FieldScaleFactor, m_View->isPerspective() ? OdGsView::kPerspective : OdGsView::kParallel);
	}

	int addDrawables(OdGsView* /*view*/) noexcept override { return 1; }

	void removeDrawables(OdGsView* /*view*/) noexcept override {
	}
};

void OdExZoomCmd::execute(OdEdCommandContext* edCommandContext) {
	OdDbCommandContextPtr CommandContext {edCommandContext};
	OdDbDatabasePtr Database {CommandContext->database()};
	OdSmartPtr<OdDbUserIO> UserIo {CommandContext->userIO()};
	const auto Keywords {L"All Center Dynamic Extents Previous Scale Window Object"};
	auto ActiveViewport {Database->activeViewportId().safeOpenObject(OdDb::kForWrite)};
	OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);
	auto ActiveView {AbstractViewportData->gsView(ActiveViewport)};
	try {
		auto FirstCorner {
			UserIo->getPoint(L"Specify corner of window, enter a scale factor (nX or nXP), or\n[All/Center/Dynamic/Extents/Previous/Scale/Window/Object] <real time>:", OdEd::kInpThrowEmpty | OdEd::kInpThrowOther | OdEd::kGptNoOSnap, nullptr, Keywords)
		};
		auto OppositeCorner {UserIo->getPoint(L"Specify opposite corner:", OdEd::kGptNoUCS | OdEd::kGptRectFrame | OdEd::kGptNoOSnap)};
		ZoomWindow(FirstCorner, OppositeCorner, ActiveView);
	} catch (const OdEdEmptyInput&) // real time
	{
		OdStaticRxObject<RTZoomTracker> Tracker;
		for (;;) {
			try {
				Tracker.Initialize(ActiveView, UserIo->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptBeginDrag | OdEd::kGptNoOSnap));
				UserIo->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptEndDrag | OdEd::kGptNoOSnap, nullptr, OdString::kEmpty, &Tracker);
			} catch (const OdEdCancel&) {
				break;
			}
		}
	} catch (const OdEdOtherInput& OtherInput) { // scale factor (nX or nXP)
		wchar_t* End;
		const auto ScaleFactor {odStrToD(OtherInput.string(), &End)};
		if (OdString(End).compare(OtherInput.string()) > 0) {
			if (OdString(End).iCompare(L"X") == 0) {
				ActiveView->zoom(ScaleFactor);
			} else if (OdString(End).iCompare(L"XP") == 0) {
				ZoomScaleXp(ScaleFactor);
			} else if (*End == 0U) {
				ActiveView->zoom(ScaleFactor);
			}
		}
		UserIo->putString(L"Requires a distance, numberX, or option keyword.");
	} catch (const OdEdKeyword& Keyword) {
		switch (Keyword.keywordIndex()) {
			case 0: // All
				break;
			case 1: // Center
				break;
			case 2: // Dynamic
				break;
			case 3: // Extents
				ZoomExtents(ActiveView, ActiveViewport);
				break;
			case 4: // Previous
				break;
			case 5: // Scale
				break;
			case 6: { // Window
				auto FirstCorner {UserIo->getPoint(L"Specify first corner:", OdEd::kGptNoUCS | OdEd::kGptNoOSnap)};
				auto OppositeCorner {UserIo->getPoint(L"Specify opposite corner:", OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptRectFrame)};
				ZoomWindow(FirstCorner, OppositeCorner, ActiveView);
				break;
			}
			case 7: // Object
				break;
			default: ;
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
	unsigned long subSetAttributes(OdGiDrawableTraits* /*drawableTraits*/) const noexcept override {
		return kDrawableIsAnEntity | kDrawableRegenDraw;
	}

	bool subWorldDraw(OdGiWorldDraw* /*worldDraw*/) const noexcept override {
		return false;
	}

	void subViewportDraw(OdGiViewportDraw* viewportDraw) const override {
		const auto& Viewport = viewportDraw->viewport();
		OdGiGeometry& Geometry = viewportDraw->geometry();
		viewportDraw->subEntityTraits().setColor(OdCmEntityColor::kACIGreen);
		viewportDraw->subEntityTraits().setFillType(kOdGiFillNever);
		OdGiModelTransformSaver ModelTransformSaver(Geometry, Viewport.getEyeToModelTransform());
		OdGiDrawFlagsHelper DrawFlagsHelper(viewportDraw->subEntityTraits(), OdGiSubEntityTraits::kDrawNoPlotstyle);
		OdGePoint3d LowerLeftCorner;
		OdGePoint2d UpperRightCorner;
		Viewport.getViewportDcCorners(reinterpret_cast<OdGePoint2d&>(LowerLeftCorner), UpperRightCorner);
		UpperRightCorner.x -= LowerLeftCorner.x;
		UpperRightCorner.y -= LowerLeftCorner.y;
		const auto Radius {odmin(UpperRightCorner.x, UpperRightCorner.y) / 9.0 * 7.0 / 2.0};
		reinterpret_cast<OdGePoint2d&>(LowerLeftCorner) += UpperRightCorner.asVector() / 2.0;
		Geometry.circle(LowerLeftCorner, Radius, OdGeVector3d::kZAxis);
		Geometry.circle(LowerLeftCorner + OdGeVector3d(0.0, Radius, 0.0), Radius / 20.0, OdGeVector3d::kZAxis);
		Geometry.circle(LowerLeftCorner + OdGeVector3d(0.0, -Radius, 0.0), Radius / 20.0, OdGeVector3d::kZAxis);
		Geometry.circle(LowerLeftCorner + OdGeVector3d(Radius, 0.0, 0.0), Radius / 20.0, OdGeVector3d::kZAxis);
		Geometry.circle(LowerLeftCorner + OdGeVector3d(-Radius, 0.0, 0.0), Radius / 20.0, OdGeVector3d::kZAxis);
	}
};

class RtOrbitTracker : public OdEdPointTracker {
	OdGsView* m_View {nullptr};
	OdGePoint3d m_Point;
	OdGiDrawablePtr m_Drawable;
	OdGePoint3d m_Position;
	OdGePoint3d m_Target;
	OdGeVector3d m_UpVector;
	OdGeVector3d m_X;
	OdGePoint3d m_ViewCenter;
	OdGeMatrix3d m_InitialViewingMatrixInverted;
	double m_D {0.0}; // diameter of orbit control in projected coordinates
	OdGsModelPtr m_Model;

	enum Axis {
		kHorizontal, kVertical, kPerpDir, // orbit around perpendicular to mouse direction
		kEye,
	} m_Axis {kHorizontal};

	void ViewportDcCorners(OdGePoint2d& lowerLeft, OdGePoint2d& upperRight) const {
		const auto Target {m_View->viewingMatrix() * m_View->target()};
		const auto HalfFieldWidth {m_View->fieldWidth() / 2.0};
		const auto HalfFieldHeight {m_View->fieldHeight() / 2.0};
		lowerLeft.x = Target.x - HalfFieldWidth;
		lowerLeft.y = Target.y - HalfFieldHeight;
		upperRight.x = Target.x + HalfFieldWidth;
		upperRight.y = Target.y + HalfFieldHeight;
	}

public:
	RtOrbitTracker() = default;

	void Reset() noexcept { m_View = nullptr; }

	void Initialize(OdGsView* view, const OdGePoint3d& pt) {
		m_View = view;
		m_Position = view->position();
		m_Target = view->target();
		m_UpVector = view->upVector();
		m_X = m_UpVector.crossProduct(view->target() - m_Position).normal();
		m_InitialViewingMatrixInverted = m_View->viewingMatrix();
		m_Point = m_InitialViewingMatrixInverted * pt;
		m_Point.z = 0.0;
		m_InitialViewingMatrixInverted.invert();
		OdGePoint3d LowerLeftPoint;
		OdGePoint2d UpperRightPoint;
		ViewportDcCorners(reinterpret_cast<OdGePoint2d&>(LowerLeftPoint), UpperRightPoint);
		UpperRightPoint.x -= LowerLeftPoint.x;
		UpperRightPoint.y -= LowerLeftPoint.y;
		const auto Radius {odmin(UpperRightPoint.x, UpperRightPoint.y) / 9.0 * 7.0 / 2.0};
		m_D = 2.0 * Radius;
		reinterpret_cast<OdGePoint2d&>(LowerLeftPoint) += UpperRightPoint.asVector() / 2.0;
		const auto r2Squared {Radius * Radius / 400.0};
		LowerLeftPoint.y += Radius;
		if ((LowerLeftPoint - m_Point).lengthSqrd() <= r2Squared) {
			m_Axis = kHorizontal;
		} else {
			LowerLeftPoint.y -= Radius;
			LowerLeftPoint.y -= Radius;
			if ((LowerLeftPoint - m_Point).lengthSqrd() <= r2Squared) {
				m_Axis = kHorizontal;
			} else {
				LowerLeftPoint.y += Radius;
				LowerLeftPoint.x += Radius;
				if ((LowerLeftPoint - m_Point).lengthSqrd() <= r2Squared) {
					m_Axis = kVertical;
				} else {
					LowerLeftPoint.x -= Radius;
					LowerLeftPoint.x -= Radius;
					if ((LowerLeftPoint - m_Point).lengthSqrd() <= r2Squared) {
						m_Axis = kVertical;
					} else {
						LowerLeftPoint.x += Radius;
						if ((LowerLeftPoint - m_Point).lengthSqrd() <= Radius * Radius) {
							m_Axis = kPerpDir;
						} else {
							m_Axis = kEye;
						}
					}
				}
			}
		}
		auto ComputeExtents {true};
		{ // Try to extract cached extents
			OdGsClientViewInfo ClientViewInfo;
			view->clientViewInfo(ClientViewInfo);
			OdDbObjectId SpaceId;
			if (!((ClientViewInfo.viewportFlags & OdGsClientViewInfo::kDependentGeometry) != 0)) {
				SpaceId = OdDbDatabasePtr(view->userGiContext()->database())->getModelSpaceId();
			} else {
				SpaceId = OdDbDatabasePtr(view->userGiContext()->database())->getPaperSpaceId();
			}
			auto BlockTableRecord {SpaceId.openObject()};
			OdGeExtents3d ExtentsWcs;
			if (BlockTableRecord->gsNode() != nullptr && BlockTableRecord->gsNode()->extents(ExtentsWcs)) {
				m_ViewCenter = ExtentsWcs.center(), ComputeExtents = false;
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

	[[nodiscard]] double Angle(const OdGePoint3d& value) const {
		const auto Point {m_View->viewingMatrix() * value};
		auto Distance {0.0};
		if (m_Axis == kHorizontal) {
			Distance = Point.y - m_Point.y;
		} else if (m_Axis == kVertical) {
			Distance = Point.x - m_Point.x;
		}
		return Distance * OdaPI / m_D;
	}

	[[nodiscard]] double AngleZ(const OdGePoint3d& value) const {
		auto Point {m_View->viewingMatrix() * value};
		auto Target {m_View->viewingMatrix() * m_ViewCenter};
		Point.z = Target.z = m_Point.z;
		return (Point - Target).angleTo(m_Point - Target, OdGeVector3d::kZAxis);
	}

	[[nodiscard]] double AnglePerpendicular(const OdGePoint3d& value) const {
		auto Point {m_View->viewingMatrix() * value};
		Point.z = 0.0;
		return Point.distanceTo(m_Point) * OdaPI / m_D;
	}

	void setValue(const OdGePoint3d& value) override {
		if (m_View != nullptr) {
			OdGeMatrix3d Transform;
			switch (m_Axis) {
				case kHorizontal:
					Transform.setToRotation(-Angle(value), m_X, m_ViewCenter);
					break;
				case kVertical:
					Transform.setToRotation(-Angle(value), m_UpVector, m_ViewCenter);
					break;
				case kEye:
					Transform.setToRotation(-AngleZ(value), m_Target - m_Position, m_ViewCenter);
					break;
				case kPerpDir: {
					auto TransformedValue {value};
					TransformedValue.transformBy(m_View->viewingMatrix());
					TransformedValue.z = 0.0;
					const auto Direction {(TransformedValue - m_Point).convert2d()};
					const auto Perpendicular {Direction.perpVector()};
					OdGeVector3d Perp3d(Perpendicular.x, Perpendicular.y, 0.0);
					Perp3d.normalizeGetLength();
					Perp3d.transformBy(m_InitialViewingMatrixInverted);
					Transform.setToRotation(-AnglePerpendicular(value), Perp3d, m_ViewCenter);
					break;
				}
			}
			auto NewPosition {Transform * m_Position};
			const auto NewTarget {Transform * m_Target};
			auto NewPositionDirection {NewPosition - NewTarget};
			NewPositionDirection.normalizeGetLength();
			NewPositionDirection *= m_Position.distanceTo(m_Target);
			NewPosition = NewTarget + NewPositionDirection;
			m_View->setView(NewPosition, NewTarget, Transform * m_UpVector, m_View->fieldWidth(), m_View->fieldHeight(), m_View->isPerspective() ? OdGsView::kPerspective : OdGsView::kParallel);
		}
	}

	int addDrawables(OdGsView* pView) override {
		m_Drawable = OdRxObjectImpl<OrbitCtrl>::createObject();
		if (m_Model.isNull()) {
			m_Model = pView->device()->createModel();
			if (!m_Model.isNull()) {
				m_Model->setRenderType(OdGsModel::kDirect); // Skip Z-buffer for 2d drawables.
				m_Model->setEnableViewExtentsCalculation(false); // Skip extents calculation.
				m_Model->setRenderModeOverride(OdGsView::k2DOptimized); // Setup 2dWireframe mode for all underlying geometry.
				const auto VisualStyleId {GraphTrackerBase::getVisualStyleOverride(pView->userGiContext()->database())};
				if (VisualStyleId != nullptr) { m_Model->setVisualStyle(VisualStyleId); } // 2dWireframe visual style.
			}
		}
		pView->add(m_Drawable, m_Model.get());
		return 1;
	}

	void removeDrawables(OdGsView* pView) override {
		pView->erase(m_Drawable);
	}
};

void OdEx3dOrbitCmd::execute(OdEdCommandContext* edCommandContext) {
	OdDbCommandContextPtr CommandContext(edCommandContext);
	OdDbDatabasePtr Database {CommandContext->database()};
	OdSmartPtr<OdDbUserIO> UserIo {CommandContext->userIO()};
	auto ActiveViewport {Database->activeViewportId().safeOpenObject(OdDb::kForWrite)};
	OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);
	auto View {AbstractViewportData->gsView(ActiveViewport)};

	// There is one special case: layout with enabled 'draw viewports first' mode
	{
		if (!Database->getTILEMODE()) {
			OdSmartPtr<OdDbLayout> Layout {Database->currentLayoutId().openObject()};
			if (Layout->drawViewportsFirst()) {
				if (View->device()->viewAt(View->device()->numViews() - 1) == View) { View = View->device()->viewAt(0); }
			}
		}
	}
	//
	const auto InteractiveMode {static_cast<OdRxVariantValue>(edCommandContext->arbitraryData(L"Bitmap InteractiveMode"))};
	const auto InteractiveFrameRate {static_cast<OdRxVariantValue>(edCommandContext->arbitraryData(L"Bitmap InteractiveFrameRate"))};
	ViewInteractivityMode Mode(InteractiveMode, InteractiveFrameRate, View);
	OdStaticRxObject<RtOrbitTracker> OrbitTracker;
	for (;;) {
		try {
			OrbitTracker.Initialize(View, UserIo->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptBeginDrag, nullptr, OdString::kEmpty, &OrbitTracker));
			UserIo->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptEndDrag, nullptr, OdString::kEmpty, &OrbitTracker);
			OrbitTracker.Reset();
		} catch (const OdEdCancel&) {
			break;
		}
	}
}

void OdExEditorObject::TurnOrbitOn(const bool orbitOn) {
	orbitOn ? (m_Flags |= kOrbitOn) : m_Flags &= ~kOrbitOn;
	SetTracker(orbitOn ? OdRxObjectImpl<RtOrbitTracker>::createObject().get() : nullptr);
}

bool OdExEditorObject::OnOrbitBeginDrag(const int x, const int y) {
	if (IsOrbitOn()) {
		dynamic_cast<RtOrbitTracker*>(m_InputTracker.get())->Initialize(ActiveView(), ToEyeToWorld(x, y));
		return true;
	}
	return false;
}

bool OdExEditorObject::OnOrbitEndDrag(int /*x*/, int /*y*/) {
	if (IsOrbitOn()) {
		dynamic_cast<RtOrbitTracker*>(m_InputTracker.get())->Reset();
		return true;
	}
	return false;
}

bool OdExEditorObject::OnZoomWindowBeginDrag(const int x, const int y) {
	const auto Point {ToEyeToWorld(x, y)};
	SetTracker(RectFrame::create(Point, GsModel()));
	TrackPoint(Point);
	return true;
}

bool OdExEditorObject::OnZoomWindowEndDrag(const int x, const int y) {
	ZoomWindow2(OdEdPointDefTrackerPtr(m_InputTracker)->basePoint(), ToEyeToWorld(x, y), ActiveView());
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

class RtDollyTracker : public OdEdPointTracker {
	OdGsView* m_View {nullptr};
	OdGePoint3d m_Point;
	OdGePoint3d m_Position;
public:
	RtDollyTracker() = default;

	void Reset() noexcept { m_View = nullptr; }

	void Initialize(OdGsView* view, const OdGePoint3d& point) {
		m_View = view;
		m_Position = view->position();
		m_Point = point - m_Position.asVector();
	}

	void setValue(const OdGePoint3d& value) override {
		if (m_View != nullptr) {
			auto Delta {(m_Point - (value - m_Position)).asVector()};
			m_Point = value - m_Position.asVector();
			Delta.transformBy(m_View->viewingMatrix());
			m_View->dolly(Delta.x, Delta.y, Delta.z);
			m_Position = m_View->position();
		}
	}

	int addDrawables(OdGsView* /*view*/) noexcept override { return 0; }

	void removeDrawables(OdGsView* /*view*/) noexcept override {
	}
};

void OdExDollyCmd::execute(OdEdCommandContext* edCommandContext) {
	OdDbCommandContextPtr CommandContext {edCommandContext};
	OdDbDatabasePtr Database {CommandContext->database()};
	OdSmartPtr<OdDbUserIO> UserIo {CommandContext->userIO()};
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
	const auto InteractiveMode {static_cast<OdRxVariantValue>(edCommandContext->arbitraryData(L"AeSys InteractiveMode"))};
	const auto InteractiveFrameRate {static_cast<OdRxVariantValue>(edCommandContext->arbitraryData(L"AeSys InteractiveFrameRate"))};
	ViewInteractivityMode Mode(InteractiveMode, InteractiveFrameRate, View);
	OdStaticRxObject<RtDollyTracker> DollyTracker;
	for (;;) {
		try {
			DollyTracker.Initialize(View, UserIo->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptBeginDrag, nullptr, OdString::kEmpty, &DollyTracker));
			UserIo->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptEndDrag, nullptr, OdString::kEmpty, &DollyTracker);
			DollyTracker.Reset();
		} catch (const OdEdCancel&) {
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

void OdExInteractivityModeCmd::execute(OdEdCommandContext* commandContext) {
	OdDbCommandContextPtr CommandContext(commandContext);
	OdSmartPtr<OdDbUserIO> UserIo {CommandContext->userIO()};
	const auto EnableInteractivity {UserIo->getInt(L"\nSet 0 to disable or non-zero to enable Interactivity Mode: ") != 0};
	if (EnableInteractivity) {
		const auto FrameRate {UserIo->getReal(L"\nSpecify frame rate (Hz): ")};
		commandContext->setArbitraryData(L"AeSys InteractiveMode", OdRxVariantValue(true));
		commandContext->setArbitraryData(L"AeSys InteractiveFrameRate", OdRxVariantValue(FrameRate));
	} else {
		commandContext->setArbitraryData(L"AeSys InteractiveMode", OdRxVariantValue(false));
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
		const Node* m_Parent;
		OdDbStub* drawableId;
		OdGiDrawablePtr drawable;
		OdGsMarker marker;

		[[nodiscard]] const OdGiPathNode* parent() const noexcept override { return m_Parent; }

		[[nodiscard]] OdDbStub* persistentDrawableId() const noexcept override { return drawableId; }

		[[nodiscard]] const OdGiDrawable* transientDrawable() const override { return drawable; }

		[[nodiscard]] OdGsMarker selectionMarker() const noexcept override { return marker; }
	};

	const Node* m_Leaf {nullptr};

	void Add(const OdGiDrawable* drawable, const OdDbObjectId& drawableId, const OdGsMarker gsMarker = -1) {
		auto NewNode {new Node()};
		NewNode->m_Parent = m_Leaf;
		m_Leaf = NewNode;
		NewNode->drawable = drawable;
		NewNode->drawableId = drawableId;
		NewNode->marker = gsMarker;
	}

	void AddNode(OdDbObjectIdArray::const_iterator& objectIterator) {
		auto Drawable {objectIterator->safeOpenObject()};
		AddNode(Drawable);
		auto Insert {OdDbBlockReference::cast(Drawable)};
		if (Insert.get() != nullptr) { AddNode(Insert->blockTableRecord()); }
		++objectIterator;
	}

public:
	OdExCollideGsPath() = default;

	~OdExCollideGsPath() {
		Clear();
	}

	OdExCollideGsPath(const OdDbFullSubentPath& path) {
		Set(path);
	}

	void Clear() {
		while (m_Leaf != nullptr) {
			const auto Node = m_Leaf;
			m_Leaf = Node->m_Parent;
			delete Node;
		}
		m_Leaf = nullptr;
	}

	void Set(const OdDbFullSubentPath& path) {
		Set(path, kNullSubentIndex);
	}

	void Set(const OdDbFullSubentPath& path, const OdGsMarker gsMarker) {
		Clear();
		const auto& PathObjectIds {path.objectIds()};
		auto PathObjectIdsIterator {PathObjectIds.begin()};
		if (PathObjectIdsIterator == PathObjectIds.end()) { throw OdError(eInvalidInput); }
		auto PathObjectId {PathObjectIdsIterator->safeOpenObject()};
		AddNode(PathObjectId->ownerId());
		for (; PathObjectIdsIterator != PathObjectIds.end() - 1; ++PathObjectIdsIterator) {
			AddNode(*PathObjectIdsIterator);
		}
		AddNode(*PathObjectIdsIterator, gsMarker);
	}

	void AddNode(const OdDbObjectId& drawableId, const OdGsMarker gsMarker = kNullSubentIndex) {
		Add(nullptr, drawableId, gsMarker);
	}

	void AddNode(const OdGiDrawable* drawable, const OdGsMarker gsMarker = kNullSubentIndex) {
		Add(drawable->isPersistent() ? nullptr : drawable, drawable->id(), gsMarker);
	}

	operator const OdGiPathNode&() const noexcept { return *m_Leaf; }
};

class CollideMoveTracker : public OdStaticRxObject<OdEdPointTracker> {
	OdArray<OdDbEntityPtr> m_SelectionSetEntities;
	OdGeMatrix3d m_LastTransform;
	OdArray<OdExCollideGsPath*> m_Paths;
	OdArray<OdExCollideGsPath*> m_PreviousHlPaths;
	OdArray<const OdGiPathNode*> m_PathNodes;
protected:
	OdGePoint3d m_BasePoint;
	OdDbDatabasePtr m_Database;
	OdGsView* m_View;
	OdGsModel* m_Model;
	bool m_DynamicHlt;

	virtual OdGeMatrix3d GetTransform(const OdGePoint3d& value) {
		OdGeMatrix3d TranslationTransform;
		TranslationTransform.setTranslation(value - m_BasePoint);
		return TranslationTransform;
	}

public:
	CollideMoveTracker(const OdGePoint3d basePoint, OdDbSelectionSet* selectionSet, OdDbDatabasePtr database, OdGsView* view, const bool dynamicHlt)
		: m_BasePoint(basePoint)
		, m_DynamicHlt(dynamicHlt) {
		m_Database = database;
		m_View = view;
		OdDbSelectionSetIteratorPtr SelectionSetIterator {selectionSet->newIterator()};
		m_Model = nullptr;

		//obtain GsModel
		while (!SelectionSetIterator->done()) {
			const auto SelectionSetObject {SelectionSetIterator->objectId()};
			OdDbEntityPtr Entity {SelectionSetObject.openObject(OdDb::kForWrite)};
			if (m_Model == nullptr && Entity->gsNode() != nullptr) { m_Model = Entity->gsNode()->model(); }
			if (!Entity.isNull()) {
				OdDbEntityPtr SubEntity;
				if (SelectionSetIterator->subentCount() == 0) {
					m_SelectionSetEntities.push_back(Entity);
				} else {
					OdDbFullSubentPath SubEntityPath;
					OdDbFullSubentPathArray SubEntitiesPaths;
					for (unsigned i = 0; i < SelectionSetIterator->subentCount(); i++) {
						SelectionSetIterator->getSubentity(i, SubEntityPath);
						SubEntity = Entity->subentPtr(SubEntityPath);
						if (!SubEntity.isNull()) { m_SelectionSetEntities.push_back(SubEntity); }
					}
				}
			}
			if (Entity.isNull()) { continue; }
			if (SelectionSetIterator->subentCount() == 0) {
				auto gsPath {new OdExCollideGsPath};
				gsPath->AddNode(SelectionSetIterator->objectId().safeOpenObject()->ownerId());
				gsPath->AddNode(SelectionSetIterator->objectId());
				m_Paths.push_back(gsPath);
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
								gsPath->Set(p, Marker);
								m_Paths.push_back(gsPath);
								auto SubEnt {Entity->subentPtr(p)};
								SubEnt->dragStatus(OdDb::kDragStart);
							}
						} else {
							auto gsPath {new OdExCollideGsPath(p)};
							m_Paths.push_back(gsPath);
						}
					}
				}
			}
			SelectionSetIterator->next();
		}
		for (auto& Path : m_Paths) {
			m_Model->highlight(Path->operator const OdGiPathNode&(), false);
			m_PathNodes.push_back(&Path->operator const OdGiPathNode&());
		}
	}

	virtual ~CollideMoveTracker() {
		if (!m_PreviousHlPaths.empty()) {
			for (auto& PreviousPath : m_PreviousHlPaths) {
				m_Model->highlight(PreviousPath->operator const OdGiPathNode&(), false);
				delete PreviousPath;
			}
			m_PreviousHlPaths.clear();
		}
		m_PathNodes.clear();
		for (auto& Path : m_Paths) {
			delete Path;
		}
		m_Paths.clear();
		m_View->invalidate();
		m_View->update();
	}

	void setValue(const OdGePoint3d& value) override {
		const auto NewTransform = GetTransform(value);
		// Compensate previous transform
		auto Transform {m_LastTransform.inverse()};
		Transform.preMultBy(NewTransform);
		// Remember last transform
		m_LastTransform = NewTransform;
		for (auto EntityIndex = static_cast<int>(m_SelectionSetEntities.size() - 1); EntityIndex >= 0; --EntityIndex) {
			m_SelectionSetEntities[EntityIndex]->transformBy(Transform);
		}
		DoCollideWithAll();
	}

	virtual void DoCollideWithAll();

	virtual void Highlight(OdArray<OdExCollideGsPath*>& newPaths);

	int addDrawables(OdGsView* view) override {
		for (auto EntityIndex = static_cast<int>(m_SelectionSetEntities.size() - 1); EntityIndex >= 0; --EntityIndex) {
			view->add(m_SelectionSetEntities[EntityIndex], nullptr);
		}
		return 1;
	}

	void removeDrawables(OdGsView* view) override {
		for (auto EntityIndex = static_cast<int>(m_SelectionSetEntities.size() - 1); EntityIndex >= 0; --EntityIndex) {
			view->erase(m_SelectionSetEntities[EntityIndex]);
		}
	}
};

bool AddNodeToPath(OdExCollideGsPath* result, const OdGiPathNode* pPath, const bool bTruncateToRef = false) {
	auto Add {true};
	if (pPath->parent() != nullptr) {
		Add = AddNodeToPath(result, pPath->parent(), bTruncateToRef);
	}
	if (Add) {
		result->AddNode(pPath->persistentDrawableId() != nullptr ? pPath->persistentDrawableId() : pPath->transientDrawable()->id(), bTruncateToRef ? 0 : pPath->selectionMarker());
		if (bTruncateToRef && pPath->persistentDrawableId() != nullptr) {
			const OdDbObjectId ObjectId(pPath->persistentDrawableId());
			auto Object {ObjectId.safeOpenObject()};
			if (!Object.isNull()) {
				if (Object->isKindOf(OdDbBlockReference::desc())) { Add = false; }
			}
		}
	}
	return Add;
}

OdExCollideGsPath* FromGiPath(const OdGiPathNode* path, const bool bTruncateToRef = false) {
	if (path == nullptr) { return nullptr; }
	const auto Result {new OdExCollideGsPath};
	AddNodeToPath(Result, path, bTruncateToRef);
	return Result;
}

void CollideMoveTracker::DoCollideWithAll() {
	class OdExCollisionDetectionReactor : public OdGsCollisionDetectionReactor {
		OdArray<OdExCollideGsPath*> m_Paths;
		bool m_DynamicHlt;
	public:
		OdExCollisionDetectionReactor(const bool dynamicHlt)
			: m_DynamicHlt(dynamicHlt) {
		}

		~OdExCollisionDetectionReactor() = default;

		unsigned long collisionDetected(const OdGiPathNode* /*pPathNode1*/, const OdGiPathNode* pPathNode2) override {
			const auto Path {FromGiPath(pPathNode2, !m_DynamicHlt)};
			if (Path != nullptr || pPathNode2->persistentDrawableId() != nullptr) { m_Paths.push_back(Path); }
			return static_cast<unsigned long>(kContinue);
		}

		OdArray<OdExCollideGsPath*>& Paths() { return m_Paths; }
	};
	OdExCollisionDetectionReactor Reactor(m_DynamicHlt);
	m_View->collide(m_PathNodes.asArrayPtr(), m_PathNodes.size(), &Reactor, nullptr, 0);
	Highlight(Reactor.Paths());
}

void CollideMoveTracker::Highlight(OdArray<OdExCollideGsPath*>& newPaths) {
	if (!m_PreviousHlPaths.empty()) { // Unhighlight old paths
		for (auto& PreviousPath : m_PreviousHlPaths) {
			m_Model->highlight(PreviousPath->operator const OdGiPathNode&(), false);
			delete PreviousPath;
		}
		m_PreviousHlPaths.clear();
	}
	for (auto& NewPath : newPaths) {
		m_Model->highlight(NewPath->operator const OdGiPathNode&(), true);
		m_PreviousHlPaths.push_back(NewPath);
	}
}

void OdExCollideCmd::execute(OdEdCommandContext* edCommandContext) {
	class OdExTransactionSaver {
		OdDbDatabasePtr m_Database;
		bool m_InTransaction;
	public:
		OdExTransactionSaver(OdDbDatabasePtr database) {
			m_Database = database;
			m_InTransaction = false;
		}

		~OdExTransactionSaver() {
			if (m_InTransaction) {
				m_Database->abortTransaction();
				m_InTransaction = false;
			}
		}

		void StartTransaction() {
			if (m_InTransaction) {
				m_Database->abortTransaction();
			}
			m_InTransaction = true;
			m_Database->startTransaction();
		}
	};
	OdDbCommandContextPtr CommandContext(edCommandContext);
	OdSmartPtr<OdDbUserIO> UserIo {CommandContext->userIO()};
	OdDbDatabasePtr Database {CommandContext->database()};
	const auto DynamicHlt {static_cast<bool>(static_cast<OdRxVariantValue>(edCommandContext->arbitraryData(L"DynamicSubEntHlt")))};

	//Get active view
	OdGsView* View {nullptr};
	if (!Database.isNull()) {
		auto ActiveViewport {Database->activeViewportId().safeOpenObject()};
		OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);
		if (!AbstractViewportData.isNull() && AbstractViewportData->gsView(ActiveViewport) != nullptr) {
			View = AbstractViewportData->gsView(ActiveViewport);
		}
	}
	if (View == nullptr) {
		ODA_ASSERT(false);
		throw OdEdCancel();
	}
	OdDbSelectionSetPtr SelectionSet {UserIo->select(L"Collide: Select objects to be checked:", OdEd::kSelAllowObjects | OdEd::kSelAllowSubents | OdEd::kSelLeaveHighlighted)};
	if (SelectionSet->numEntities() == 0U) { throw OdEdCancel(); }
	OdExTransactionSaver Saver(Database);
	Saver.StartTransaction();
	const auto BasePoint {UserIo->getPoint(L"Collide: Specify base point:")};
	CollideMoveTracker Tracker(BasePoint, SelectionSet, Database, View, DynamicHlt);
	const auto OffsetPoint {UserIo->getPoint(L"Collide: Specify second point:", OdEd::kGdsFromLastPoint | OdEd::kGptRubberBand, nullptr, OdString::kEmpty, &Tracker)};
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
		OdArray<OdExCollideGsPath*> m_paths;
		bool m_DynamicHlt;
	public:
		OdExCollisionDetectionReactor(const bool dynamicHlt)
			: m_DynamicHlt(dynamicHlt) {
		}

		~OdExCollisionDetectionReactor() = default;

		unsigned long collisionDetected(const OdGiPathNode* pPathNode1, const OdGiPathNode* pPathNode2) override {
			const auto Path1 {FromGiPath(pPathNode1, !m_DynamicHlt)};
			const auto Path2 {FromGiPath(pPathNode2, !m_DynamicHlt)};
			m_paths.push_back(Path1);
			m_paths.push_back(Path2);
			return static_cast<unsigned long>(kContinue);
		}

		OdArray<OdExCollideGsPath*>& Paths() { return m_paths; }
	};
	OdDbCommandContextPtr CommandContext(edCommandContext);
	OdSmartPtr<OdDbUserIO> UserIo {CommandContext->userIO()};
	OdDbDatabasePtr Database {CommandContext->database()};

	//Get active view
	OdGsView* View {nullptr};
	if (!Database.isNull()) {
		auto ActiveViewport {Database->activeViewportId().safeOpenObject()};
		OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);
		if (!AbstractViewportData.isNull() && AbstractViewportData->gsView(ActiveViewport) != nullptr) {
			View = AbstractViewportData->gsView(ActiveViewport);
		}
	}
	if (View == nullptr) {
		ODA_ASSERT(false);
		throw OdEdCancel();
	}
	auto Model {View->getModelList()[0]};
	const auto Choice {UserIo->getInt(L"Input 1 to detect only intersections, any other to detect all", 0, 0)};
	OdGsCollisionDetectionContext CollisionDetectionContext;
	CollisionDetectionContext.setIntersectionOnly(Choice == 1);
	auto DynamicHlt {static_cast<OdRxVariantValue>(edCommandContext->arbitraryData(L"DynamicSubEntHlt"))};
	OdExCollisionDetectionReactor Reactor(DynamicHlt);
	View->collide(nullptr, 0, &Reactor, nullptr, 0, &CollisionDetectionContext);
	auto& ReactorPaths {Reactor.Paths()};
	for (auto& ReactorPath : ReactorPaths) {
		const auto PathNode {&ReactorPath->operator const OdGiPathNode&()};
		Model->highlight(*PathNode);
		//delete ReactorPath;
	}
	UserIo->getInt(L"Specify any number to exit", 0, 0);
	for (auto& ReactorPath : ReactorPaths) {
		const auto PathNode {&ReactorPath->operator const OdGiPathNode&()};
		Model->highlight(*PathNode, false);
		delete ReactorPath;
	}
	ReactorPaths.clear();
}

void OdExEditorObject::SetTracker(OdEdInputTracker* inputTracker) {
	if (m_InputTracker.get() != nullptr) {
		m_InputTracker->removeDrawables(ActiveTopView());
	}
	m_InputTracker = inputTracker;
	m_BasePt = nullptr;
	if (inputTracker != nullptr) {
		inputTracker->addDrawables(ActiveTopView()) != 0 ? (m_Flags |= kTrackerHasDrawables) : m_Flags &= ~kTrackerHasDrawables;
		auto PointDefTracker {OdEdPointDefTracker::cast(inputTracker)};
		if (PointDefTracker.get() != nullptr) {
			m_basePt = PointDefTracker->basePoint();
			m_BasePt = &m_basePt;
		}
	} else {
		m_Flags &= ~kTrackerHasDrawables;
	}
}

bool OdExEditorObject::TrackString(const OdString& value) {
	if (m_InputTracker.get() != nullptr) {
		ODA_ASSERT(m_InputTracker->isKindOf(OdEdStringTracker::desc()));
		dynamic_cast<OdEdStringTracker*>(m_InputTracker.get())->setValue(value);
		return (m_Flags & kTrackerHasDrawables) != 0;
	}
	return false;
}

bool OdExEditorObject::TrackPoint(const OdGePoint3d& point) {
	if (m_InputTracker.get() != nullptr) {
		ODA_ASSERT(m_InputTracker->isKindOf(OdEdPointTracker::desc()));
		dynamic_cast<OdEdPointTracker*>(m_InputTracker.get())->setValue(point);
		return (m_Flags & kTrackerHasDrawables) != 0;
	}
	return false;
}

bool OdExEditorObject::HasDatabase() const {
	return m_CommandContext->baseDatabase() != nullptr;
}
