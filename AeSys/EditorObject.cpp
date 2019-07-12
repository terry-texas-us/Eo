// From Examples\Editor\EditorObject.cpp (last compare 20.5)
#include "stdafx.h"
#include <Ge/GeRay3d.h>
#include <Gi/GiDrawableImpl.h>
#include <Gi/GiWorldDraw.h>
#include <Gs/Gs.h>
#include <Gs/GsBaseVectorizer.h>
#include <DbLayout.h>
#include <DbCommandContext.h>
#include <DbAbstractViewportData.h>
#include <DbBlockTableRecord.h>
#include <DbViewportTable.h>
#include <DbDictionary.h>
#include <DbVisualStyle.h>
#include <DbHostAppServices.h>
#include <ExTrackers.h>
#include <RxVariantValue.h>
#include <Gs/GsModel.h>
#include <DbBlockReference.h>
#include "EditorObject.h"
#include "EoRtOrbitTracker.h"
#include "EoCollideMoveTracker.h"

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
