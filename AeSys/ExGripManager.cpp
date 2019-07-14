// From Examples\Editor\ExGripManager.cpp (last compare 20.5)
#include "stdafx.h"
#include <OdaCommon.h>
#include <UInt32Array.h>
#include <Ge/GePoint3d.h>
#include <Gi/GiDrawableImpl.h>
#include <DbHostAppServices.h>
#include <DbCommandContext.h>
#include <DbEntity.h>
#include <DbAbstractViewportData.h>
#include <RxVariantValue.h>
#include <Gs/GsModel.h>
#include "ExGripManager.h"
#include "OdExGripDrag.h"
#include "OdExGripData.h"

// Menu animation flags
#if !defined(ODA_UNIXOS)
#ifndef TPM_VERPOSANIMATION
static const unsigned TPM_VERPOSANIMATION = 0x1000L;
#endif
#ifndef TPM_NOANIMATION
static const unsigned TPM_NOANIMATION = 0x4000L;
#endif
#endif // ODA_UNIXOS
OdGiDrawablePtr OdExGripManager::CloneEntity(OdDbStub* id) {
	auto Entity {OdDbEntity::cast(OdDbObjectId(id).openObject())};
	if (Entity.isNull()) {
		return OdGiDrawablePtr();
	}
	OdDbEntityPtr Clone;
	if (Entity->cloneMeForDragging()) {
		Clone = OdDbEntity::cast(Entity->clone());
	}
	if (Clone.get() != nullptr) {
		Clone->disableUndoRecording(true);
		Clone->setPropertiesFrom(Entity.get(), false);
	}
	return OdGiDrawable::cast(Clone);
}

void OdExGripManager::OnModified(OdGiDrawable* grip) {
	if (GetGsModel() != nullptr) {
		GetGsModel()->onModified(grip, static_cast<OdGiDrawable*>(nullptr));
	} else if (GetGsLayoutHelper() != nullptr) {
		GetGsLayoutHelper()->invalidate();
	}
}

OdExGripManager::OdExGripManager() noexcept {
	m_DbReactor.gripManager = this;
}

void OdExGripManager::Initialize(OdGsDevice* device, OdGsModel* gsModel, OdDbCommandContext* commandContext, GetSelectionSetPtr getSSet) {
	m_LayoutHelper = device;
	m_GsModel = gsModel;
	m_CommandContext = commandContext;
	if (m_CommandContext->baseDatabase() != nullptr) {
		const auto Database {m_CommandContext->database()};
		Disable(false);
		const auto HostApplicationServices {Database->appServices()};
		m_GripSize = HostApplicationServices->getGRIPSIZE();
		m_GripObjectLimit = HostApplicationServices->getGRIPOBJLIMIT();
		m_GripColor.setColorIndex(HostApplicationServices->getGRIPCOLOR());
		m_GripHoverColor.setColorIndex(HostApplicationServices->getGRIPHOVER());
		m_GripHotColor.setColorIndex(HostApplicationServices->getGRIPHOT());
	}
	m_GetSelectionSet = getSSet;
	m_GripStretchCommand.parent = this;
}

void OdExGripManager::Uninitialize() {
	if (m_CommandContext != nullptr) {
		Disable(true);
		m_CommandContext = nullptr;
	}
	m_LayoutHelper = nullptr;
}

void OdExGripManager::OdExGripCommand::execute(OdEdCommandContext* /*edCommandContext*/) {
	auto Ok = true;
	try {
		const auto FinalPoint {
			parent->m_CommandContext->dbUserIO()->getPoint(L"Specify stretch point or [Base point/Copy/Undo/eXit]:", OdEd::kGptNoLimCheck | OdEd::kGptDefault | OdEd::kGptNoUCS, &parent->m_BasePoint, L"Base Copy Undo eXit", parent)
		};
		for (auto& ParentGripDrag : parent->m_GripDrags) {
			ParentGripDrag->MoveEntity(parent->EyeToUcsPlane(FinalPoint, parent->m_BasePoint));
		}
	} catch (const OdEdCancel&) {
		Ok = false;
		for (auto& ParentGripDrag : parent->m_GripDrags) {
			ParentGripDrag->NotifyDragAborted();
		}
	}
	for (auto& ParentGripDrag : parent->m_GripDrags) {
		if (Ok) {
			ParentGripDrag->NotifyDragEnded();
			parent->UpdateEntityGrips(ParentGripDrag->EntityId());
		} else {
			ParentGripDrag->NotifyDragAborted();
		}
	}
	parent->m_GripDrags.clear();
	if (Ok) {
		parent->UpdateInvisibleGrips();
	}
}

void OdExGripManager::AddToDrag(OdExGripDataPtrArray& activeKeys) {
	for (auto& ActiveKey : activeKeys) {
		ActiveKey->SetStatus(OdDbGripOperations::kDragImageGrip);
	}
	GripDataMap::const_iterator it = m_GripData.begin();
	while (it != m_GripData.end()) {
		auto Active {false};
		OdExGripDragPtr Drag;
		{
			const auto& Data = it->second.dataArray;
			for (const auto& Datum : Data) {
				if (OdDbGripOperations::kDragImageGrip == Datum->Status()) {
					Active = true;
					Drag = OdExGripDrag::CreateObject(it->first, this);
					break;
				}
			}
			for (unsigned i = 0; i < it->second.gripDataSubEntity.size() && !Active; i++) {
				const auto& SubEntityData {it->second.gripDataSubEntity.at(i).subData};
				for (const auto& SubEntityDatum : SubEntityData) {
					if (OdDbGripOperations::kDragImageGrip == SubEntityDatum->Status()) {
						Active = true;
						Drag = OdExGripDrag::CreateObject(it->second.gripDataSubEntity.at(i).subentPath, this);
						break;
					}
				}
			}
		}
		if (Active) {
			m_GripDrags.push_back(Drag);
		}
		it++;
	}
	for (auto& GripDrag : m_GripDrags) {
		GripDrag->NotifyDragStarted();
		GripDrag->CloneEntity();
	}
}

bool OdExGripManager::OnMouseDown(const int x, const int y, const bool shiftIsDown) {
	if (!OdBaseGripManager::OnMouseDown(x, y, shiftIsDown)) {
		return false;
	}
	if (shiftIsDown) {
		return true;
	}
	OdExGripDataPtrArray Keys;
	LocateGripsAt(x, y, Keys);
	if (Keys.empty()) {
		return true;
	}
	OdExGripDataPtrArray ActiveKeys;
	LocateGripsByStatus(OdDbGripOperations::kHotGrip, ActiveKeys);
	if (ActiveKeys.empty()) {
		return false;
	} // Valid situation. If trigger grip performed entity modification and returned eGripHotToWarm then nothing is to be done cause entity modification will cause reactor to regen grips.
	if (HandleMappedRtClk(ActiveKeys, x, y)) {
		return true;
	}
	AddToDrag(ActiveKeys);
	m_BasePoint = Keys.first()->Point();
	m_LastPoint = m_BasePoint;
	auto FirstData {Keys.first()->GripData()};
	if (FirstData.get() != nullptr && FirstData->alternateBasePoint() != nullptr) { // Use alternative point
		m_BasePoint = *FirstData->alternateBasePoint();
	}
	m_CommandContext->database()->startUndoRecord();
	odedRegCmds()->executeCommand(&m_GripStretchCommand, m_CommandContext);
	for (auto& ActiveKey : ActiveKeys) {
		ActiveKey->SetStatus(OdDbGripOperations::kWarmGrip);
	}
	return true;
}

bool OdExGripManager::OnMouseMove(const int x, const int y, const bool shiftIsDown) {
	// restart hover operation
	const auto Result {StartHover(x, y, shiftIsDown)};
	if (Result == eGripOpFailure) {
		return false;
	}
	if (Result == eGripOpGetNewGripPoints) {
		OdExGripDataPtrArray ActiveKeys;
		LocateGripsByStatus(OdDbGripOperations::kHotGrip, ActiveKeys);
		if (ActiveKeys.empty()) {
			return false;
		} // Valid situation. If trigger grip performed entity modification and returned eGripHotToWarm then nothing is to be done cause entity modification will cause reactor to regen grips.
		AddToDrag(ActiveKeys);
		m_CommandContext->database()->startUndoRecord();
		odedRegCmds()->executeCommand(&m_GripStretchCommand, m_CommandContext);
		for (auto& ActiveKey : ActiveKeys) {
			ActiveKey->SetStatus(OdDbGripOperations::kWarmGrip);
		}
	}
	return true;
}

bool OdExGripManager::OnControlClick() const {
	// TODO: Notify active grips.
	// AEC grips use CTRL key to change mode, but how to pass it threw standard interface is currently unknown.
	return !m_GripDrags.empty();
}

void OdExGripManager::ShowGrip(OdExGripData* gripData, bool /*model*/) {
	auto PaperLayoutHelper {OdGsPaperLayoutHelper::cast(m_LayoutHelper)};
	const auto NumberOfViews {m_LayoutHelper->numViews()};
	if (PaperLayoutHelper.get() != nullptr) {
		auto ActiveViewport {m_CommandContext->database()->activeViewportId().openObject()};
		OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);
		if (!AbstractViewportData.isNull() && AbstractViewportData->gsView(ActiveViewport) != nullptr) {
			AbstractViewportData->gsView(ActiveViewport)->add(gripData, m_GsModel);
		} else {
			PaperLayoutHelper->overallView()->add(gripData, m_GsModel);
		}
	} else {
		for (auto ViewIndex = 0; ViewIndex < NumberOfViews; ViewIndex++) {
			m_LayoutHelper->viewAt(ViewIndex)->add(gripData, m_GsModel);
		}
	}
}

void OdExGripManager::HideGrip(OdExGripData* gripData, bool /*model*/) {
	auto PaperLayoutHelper {OdGsPaperLayoutHelper::cast(m_LayoutHelper)};
	const auto NumberOfViews {m_LayoutHelper->numViews()};
	if (PaperLayoutHelper.get() != nullptr) {
		for (auto ViewIndex = 0; ViewIndex < NumberOfViews; ViewIndex++) {
			m_LayoutHelper->viewAt(ViewIndex)->erase(gripData);
		}
	} else {
		for (auto ViewIndex = 0; ViewIndex < NumberOfViews; ViewIndex++) {
			m_LayoutHelper->viewAt(ViewIndex)->erase(gripData);
		}
	}
}

int OdExGripManager::addDrawables(OdGsView* view) {
	ODA_ASSERT(view->device() == m_LayoutHelper->underlyingDevice().get());
	const auto Size = m_GripDrags.size();
	for (unsigned i = 0; i < Size; i++) {
		view->add(m_GripDrags[i].get(), /*m_pGsModel*/ nullptr);
	}
	return gsl::narrow_cast<int>(Size);
}

void OdExGripManager::removeDrawables(OdGsView* view) {
	const auto Size {m_GripDrags.size()};
	for (unsigned i = 0; i < Size; i++) {
		view->erase(m_GripDrags[i].get());
	}
}

inline void ResetDragging(OdGsDevice* device, const bool option) {
	if (device == nullptr) {
		return;
	}
	auto Properties {device->properties()};
	if (Properties.isNull()) {
		return;
	}
	if (!Properties->has(L"DrawDragging")) {
		return;
	}
	Properties->putAt(L"DrawDragging", OdRxVariantValue(option));
}

void OdExGripManager::DraggingStarted() {
	ResetDragging(m_LayoutHelper, true);
}

void OdExGripManager::DraggingStopped() {
	ResetDragging(m_LayoutHelper, false);
}

OdSelectionSetPtr OdExGripManager::WorkingSelectionSet() const {
	if (m_GetSelectionSet != nullptr) {
		return OdSelectionSet::cast(m_GetSelectionSet(m_CommandContext));
	}
	return OdSelectionSetPtr();
}

OdGsView* OdExGripManager::ActiveGsView() const {
	return m_LayoutHelper->activeView();
}

OdDbStub* OdBaseGripManager::ActiveViewportId() const {
	OdGsClientViewInfo ClientViewInfo;
	ActiveGsView()->clientViewInfo(ClientViewInfo);
	return ClientViewInfo.viewportObjectId;
}

OdGePoint3d OdExGripManager::EyeToUcsPlane(const OdGePoint3d& point, const OdGePoint3d& basePoint) const {
	auto ActiveViewport {OdDbObjectId(ActiveViewportId()).safeOpenObject()};
	OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);
	OdGePoint3d UcsOrigin;
	OdGeVector3d UcsXAxis;
	OdGeVector3d UcsYAxis;
	AbstractViewportData->getUcs(ActiveViewport, UcsOrigin, UcsXAxis, UcsYAxis);
	const OdGePlane Plane {basePoint, UcsXAxis, UcsYAxis};
	OdGeLine3d Line(point, ActiveViewDirection());
	OdGePoint3d NewPoint;
	if (!Plane.intersectWith(Line, NewPoint)) {
		Line.set(point, UcsXAxis.crossProduct(UcsYAxis));
		if (!Plane.intersectWith(Line, NewPoint)) {
			NewPoint = basePoint;
		}
	}
	return NewPoint;
}

bool OdExGripManager::HandleMappedRtClk(OdExGripDataPtrArray& activeKeys, const int x, const int y) {
#if !defined(ODA_UNIXOS) && !(defined(ODA_WINDOWS) && !defined(OD_WINDOWS_DESKTOP))
	const auto Size {activeKeys.size()};
	auto RightClickIndex {-1};
	for (unsigned Index = 0; Index < Size; Index++) {
		if (!activeKeys[Index]->GripData().isNull() && activeKeys[Index]->GripData()->rtClk() != nullptr && activeKeys[Index]->GripData()->mapGripHotToRtClk() && !activeKeys[Index]->IsShared()) {
			RightClickIndex = static_cast<int>(Index);
			break;
		}
	}
	if (RightClickIndex != -1) {
		OdDbStubPtrArray Entities;
		OdDbGripDataArray HotGrips;
		for (unsigned i = 0; i < Size; i++) {
			HotGrips.append(*activeKeys[i]->GripData());
			if (!Entities.contains(activeKeys[i]->EntityId())) {
				Entities.append(activeKeys[i]->EntityId());
			}
		}
		OdString MenuName;
		ODHMENU Menu {nullptr};
		ContextMenuItemIndexPtr cb {nullptr};
		const auto Result {(*activeKeys[static_cast<unsigned>(RightClickIndex)]->GripData()->rtClk())(HotGrips, Entities, MenuName, Menu, cb)};
		if (Result == eOk && Menu != nullptr && cb != nullptr) {
			const auto ActiveWindow {GetActiveWindow()};
			POINT pt = {x, y};
			ClientToScreen(ActiveWindow, &pt);
			const unsigned Flags {TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_NOANIMATION};
			(*cb)(TrackPopupMenu(static_cast<HMENU>(Menu), Flags, pt.x, pt.y, 0, ActiveWindow, nullptr));
			DestroyMenu(static_cast<HMENU>(Menu));
			for (unsigned i = 0; i < Size; i++) {
				activeKeys[i]->SetStatus(OdDbGripOperations::kWarmGrip);
			}
			UpdateEntityGrips(activeKeys[static_cast<unsigned>(RightClickIndex)]->EntityId());
			return true;
		}
	}
#endif // ODA_UNIXOS
	return false;
}

void OdExGripManager::Disable(const bool disable) {
	if (m_Disabled != disable) {
		const auto Database {m_CommandContext->database()};
		m_Disabled = disable;
		if (disable) {
			Database->removeReactor(&m_DbReactor);
		} else {
			Database->addReactor(&m_DbReactor);
		}
	}
}

OdGiDrawablePtr OdExGripManager::OpenObject(OdDbStub* id, const bool isForWriteMode) {
	OdGiDrawablePtr Drawable;
	if (id == nullptr) {
		return Drawable;
	}
	Drawable = OdGiDrawable::cast(OdDbObjectId(id).openObject(isForWriteMode ? OdDb::kForWrite : OdDb::kForRead));
	return Drawable;
}

OdResult OdExGripManager::GetGripPointsAtSubentPath(OdGiDrawable* entity, const OdDbBaseFullSubentPath& path, OdDbGripDataPtrArray& grips, const double curViewUnitSize, const int gripSize, const OdGeVector3d& curViewDir, const unsigned long bitFlags) const {
	return OdDbEntity::cast(entity)->getGripPointsAtSubentPath(*static_cast<const OdDbFullSubentPath*>(&path), grips, curViewUnitSize, gripSize, curViewDir, bitFlags);
}

OdResult OdExGripManager::GetGripPoints(OdGiDrawable* entity, OdDbGripDataPtrArray& grips, const double curViewUnitSize, const int gripSize, const OdGeVector3d& curViewDir, const int bitFlags) const {
	OdDbEntity* Entity {OdDbEntity::cast(entity)};
	if (Entity == nullptr) {
		return eNotApplicable;
	}
	return Entity->getGripPoints(grips, curViewUnitSize, gripSize, curViewDir, bitFlags);
}

OdResult OdExGripManager::GetGripPoints(OdGiDrawable* entity, OdGePoint3dArray& gripPoints) const {
	OdDbEntity* Entity {OdDbEntity::cast(entity)};
	if (Entity == nullptr) {
		return eNotApplicable;
	}
	return Entity->getGripPoints(gripPoints);
}

OdResult OdExGripManager::MoveGripPointsAtSubentPaths(OdGiDrawable* entity, const OdDbBaseFullSubentPathArray& paths, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, const unsigned long bitFlags) {
	ODA_ASSERT_ONCE(sizeof(OdDbFullSubentPath) == sizeof(OdDbBaseFullSubentPath));
	return OdDbEntity::cast(entity)->moveGripPointsAtSubentPaths(*reinterpret_cast<const OdDbFullSubentPathArray*>(&paths), gripAppData, offset, bitFlags);
}

OdResult OdExGripManager::MoveGripPointsAt(OdGiDrawable* entity, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, const int bitFlags) {
	return OdDbEntity::cast(entity)->moveGripPointsAt(gripAppData, offset, bitFlags);
}

OdResult OdExGripManager::MoveGripPointsAt(OdGiDrawable* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return OdDbEntity::cast(entity)->moveGripPointsAt(indices, offset);
}

void OdExGripManager::SubentGripStatus(OdGiDrawable* entity, const OdDb::GripStat status, const OdDbBaseFullSubentPath& subentity) {
	OdDbEntity::cast(entity)->subentGripStatus(status, *static_cast<const OdDbFullSubentPath*>(&subentity));
}

void OdExGripManager::GripStatus(OdGiDrawable* entity, const OdDb::GripStat status) {
	OdDbEntity::cast(entity)->gripStatus(status);
}

void OdExGripManager::DragStatus(OdGiDrawable* entity, const OdDb::DragStat status) {
	OdDbEntity::cast(entity)->dragStatus(status);
}

bool OdExGripManager::IsModel(OdGiDrawable* entity) noexcept {
	OdDbEntity* Entity {OdDbEntity::cast(entity)};
	return Entity == nullptr || Entity->database()->getTILEMODE();
}

void OdExGripDbReactor::objectAppended(const OdDbDatabase* /*database*/, const OdDbObject* /*dbObject*/) noexcept {
	// New object.
}

void OdExGripDbReactor::objectModified(const OdDbDatabase* /*database*/, const OdDbObject* object) {
	gripManager->UpdateEntityGrips(object->objectId());
	gripManager->UpdateInvisibleGrips();
}

void OdExGripDbReactor::objectErased(const OdDbDatabase* /*database*/, const OdDbObject* object, const bool erased) {
	if (erased) {
		gripManager->RemoveEntityGrips(object->objectId(), true);
		gripManager->UpdateInvisibleGrips();
	}
}
