// From Examples\Editor\ExGripManager.cpp (last compare 20.5)
#include "stdafx.h"
#include <OdaCommon.h>
#define STL_USING_MAP
#define STL_USING_ALGORITHM
#include <OdaSTL.h>
#include <UInt32Array.h>
#include <Ge/GePoint3d.h>
#include <Gi/GiDrawableImpl.h>
#include <Gi/GiWorldDraw.h>
#include <Gi/GiViewportDraw.h>
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
//
constexpr unsigned gc_GripManagerPageEachObject = 200;

namespace
{
	OdSelectionSetIteratorPtr SearchObjectSelectionSetIterator(OdSelectionSetPtr selectionSet, OdDbStub* id) {
		auto SelectionSetIterator {selectionSet->newIterator()};
		while (!SelectionSetIterator->done()) {
			if (SelectionSetIterator->id() == id) { return SelectionSetIterator; }
			SelectionSetIterator->next();
		}
		return OdSelectionSetIteratorPtr();
	}
}

OdGiDrawablePtr OdExGripManager::CloneEntity(OdDbStub* id) {
	auto Entity {OdDbEntity::cast(OdDbObjectId(id).openObject())};
	if (Entity.isNull()) { return OdGiDrawablePtr(); }
	OdDbEntityPtr Clone;
	if (Entity->cloneMeForDragging()) {
		Clone = OdDbEntity::cast(Entity->clone());
	}
	if (Clone.get()) {
		Clone->disableUndoRecording(true);
		Clone->setPropertiesFrom(Entity.get(), false);
	}
	return OdGiDrawable::cast(Clone);
}

void OdExGripManager::OnModified(OdGiDrawable* grip) {
	if (GetGsModel()) {
		GetGsModel()->onModified(grip, static_cast<OdGiDrawable*>(nullptr));
	} else if (GetGsLayoutHelper()) {
		GetGsLayoutHelper()->invalidate();
	}
}

OdBaseGripManager::OdBaseGripManager() noexcept {
	m_GripData.clear();
	m_HoverGripsData.clear();
	m_GripDrags.clear();
}

OdExGripManager::OdExGripManager() noexcept {
	m_DbReactor.gripManager = this;
}

OdBaseGripManager::~OdBaseGripManager() {
	EndHover();
}

void OdExGripManager::Initialize(OdGsDevice* device, OdGsModel* gsModel, OdDbCommandContext* commandContext, GetSelectionSetPtr getSSet) {
	m_LayoutHelper = device;
	m_GsModel = gsModel;
	m_CommandContext = commandContext;
	if (m_CommandContext->baseDatabase()) {
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
	if (m_CommandContext) {
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

bool OdBaseGripManager::OnMouseDown(const int x, const int y, const bool shiftIsDown) {
	EndHover();
	OdExGripDataPtrArray aKeys;
	LocateGripsAt(x, y, aKeys);
	if (aKeys.empty()) { return false; }
	if (shiftIsDown) { // Modify Grip  status().
		auto NewStatus {OdDbGripOperations::kHotGrip};
		for (auto& Key : aKeys) {
			if (OdDbGripOperations::kHotGrip == Key->Status()) {
				NewStatus = OdDbGripOperations::kWarmGrip;
				break;
			}
		}
		for (auto& Key : aKeys) {
			auto CurrentStatus {NewStatus};
			auto Grip {Key};
			if (!Grip->GripData().isNull()) {
				if (Grip->GripData()->triggerGrip()) {
					CurrentStatus = OdDbGripOperations::kWarmGrip;
				} else {
					if (Grip->GripData()->hotGripFunc()) {
						int Flags {OdDbGripOperations::kMultiHotGrip};
						if (Grip->IsShared()) { Flags |= OdDbGripOperations::kSharedGrip; }
						const auto Result {(*Grip->GripData()->hotGripFunc())(Grip->GripData(), Grip->EntityId(), Flags)};
						if (Result == eGripOpGripHotToWarm) { CurrentStatus = OdDbGripOperations::kWarmGrip; }
					}
				}
			}
			Key->SetStatus(CurrentStatus);
		}
	} else { // Launch Grip Edit.
		auto MakeHot {true};
		{
			GripDataMap::const_iterator GripDataIterator {m_GripData.begin()};
			while (GripDataIterator != m_GripData.end() && MakeHot) {
				const auto& Data {GripDataIterator->second.dataArray};
				for (const auto& Datum : Data) {
					if (OdDbGripOperations::kHotGrip == Datum->Status()) {
						MakeHot = false;
						break;
					}
				}
				for (unsigned i = 0; i < GripDataIterator->second.gripDataSubEntity.size() && MakeHot; i++) {
					const auto& Data {GripDataIterator->second.gripDataSubEntity.at(i).subData};
					for (const auto& Datum : Data) {
						if (OdDbGripOperations::kHotGrip == Datum->Status()) {
							MakeHot = false;
							break;
						}
					}
				}
				GripDataIterator++;
			}
		}
		auto GetNew {false};
		OdDbObjectId EntityIdToUpdate;
		if (MakeHot) {
			for (unsigned i = 0; i < aKeys.size(); i++) {
				auto Grip {aKeys[i]};
				auto New {OdDbGripOperations::kHotGrip};
				if (!Grip->GripData().isNull() && Grip->GripData()->hotGripFunc()) {
					auto Flags {0};
					if (Grip->IsShared()) { Flags |= OdDbGripOperations::kSharedGrip; }
					if (Grip->GripData()->triggerGrip()) {
						if (!Grip->IsShared()) {
							const auto Result {(*Grip->GripData()->hotGripFunc())(Grip->GripData(), Grip->EntityId(), Flags)};
							if (Result == eOk || Result == eGripOpGripHotToWarm) {
								New = OdDbGripOperations::kWarmGrip;
							} else if (Result == eGripOpGetNewGripPoints) {
								GetNew = true;
								EntityIdToUpdate = Grip->EntityId();
							}
						}
					} else {
						const auto Result {(*Grip->GripData()->hotGripFunc())(Grip->GripData(), Grip->EntityId(), Flags)};
						if (!Grip->IsShared()) {
							if (Result == eGripOpGripHotToWarm) {
								New = OdDbGripOperations::kWarmGrip;
							} else if (Result == eGripOpGetNewGripPoints) {
								GetNew = true;
								EntityIdToUpdate = Grip->EntityId();
							}
						}
					}
				}
				Grip->SetStatus(New);
			}
		}
		if (GetNew) { UpdateEntityGrips(EntityIdToUpdate); }
	}
	return true;
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
		if (Active) { m_GripDrags.push_back(Drag); }
		it++;
	}
	for (auto& GripDrag : m_GripDrags) {
		GripDrag->NotifyDragStarted();
		GripDrag->CloneEntity();
	}
}

bool OdExGripManager::OnMouseDown(const int x, const int y, const bool shiftIsDown) {
	if (!OdBaseGripManager::OnMouseDown(x, y, shiftIsDown)) { return false; }
	if (shiftIsDown) { return true; }
	OdExGripDataPtrArray Keys;
	LocateGripsAt(x, y, Keys);
	if (Keys.empty()) { return true; }
	OdExGripDataPtrArray ActiveKeys;
	LocateGripsByStatus(OdDbGripOperations::kHotGrip, ActiveKeys);
	if (ActiveKeys.empty()) { return false; } // Valid situation. If trigger grip performed entity modification and returned eGripHotToWarm then nothing is to be done cause entity modification will cause reactor to regen grips.
	if (HandleMappedRtClk(ActiveKeys, x, y)) { return true; }
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

OdResult OdBaseGripManager::StartHover(const int x, const int y, const bool shiftIsDown) {
	auto Result {eOk};
	if (!EndHover()) {
		Result = eGripOpFailure;
		m_ClockStartHover = 0;
	}
	OdExGripDataPtrArray Keys;
	LocateGripsAt(x, y, Keys);
	if (!Keys.empty()) {
		m_HoverGripsData = Keys;
		for (unsigned i = 0; i < m_HoverGripsData.size(); i++) {
			auto Grip {m_HoverGripsData[i]};
			if (Grip->Status() == OdDbGripOperations::kWarmGrip) {
				Grip->SetStatus(OdDbGripOperations::kHoverGrip);
				if (!Grip->GripData().isNull()) {
					if (Grip->GripData()->hoverFunc() != nullptr && !shiftIsDown) {
						if (!m_ClockStartHover) { m_ClockStartHover = clock(); }
						if ((clock() - m_ClockStartHover) * 1000 / CLOCKS_PER_SEC > 300) { // 300 ms delay before hover
							auto Flags {0};
							if (Grip->IsShared()) { Flags = OdDbGripOperations::kSharedGrip; }
							Result = (*Grip->GripData()->hoverFunc())(Grip->GripData(), Grip->EntityId(), Flags);
							if (Result == eGripOpGetNewGripPoints) {
								m_ClockStartHover = 0;
								Keys[i]->SetStatus(OdDbGripOperations::kHotGrip);
								m_BasePoint = Keys.first()->Point();
								m_LastPoint = m_BasePoint;
								auto FirstData {Keys.first()->GripData()};
								if (FirstData.get() != nullptr && FirstData->alternateBasePoint() != nullptr) { // Use alternative point
									m_BasePoint = *FirstData->alternateBasePoint();
								}
							}
						}
					}
				}
				OnModified(Grip);
			}
		}
	}
	return Result;
}

bool OdBaseGripManager::EndHover() {
	if (m_HoverGripsData.empty()) { return false; }
	for (auto HoverGripData : m_HoverGripsData) {
		if (HoverGripData->Status() == OdDbGripOperations::kHoverGrip) {
			HoverGripData->SetStatus(OdDbGripOperations::kWarmGrip);
			OnModified(HoverGripData);
		}
	}
	m_HoverGripsData.clear();
	return true;
}

bool OdExGripManager::OnMouseMove(const int x, const int y, const bool shiftIsDown) {
	// restart hover operation
	const auto Result {StartHover(x, y, shiftIsDown)};
	if (Result == eGripOpFailure) { return false; }
	if (Result == eGripOpGetNewGripPoints) {
		OdExGripDataPtrArray ActiveKeys;
		LocateGripsByStatus(OdDbGripOperations::kHotGrip, ActiveKeys);
		if (ActiveKeys.empty()) { return false; } // Valid situation. If trigger grip performed entity modification and returned eGripHotToWarm then nothing is to be done cause entity modification will cause reactor to regen grips.
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

void OdBaseGripManager::SelectionSetChanged(OdSelectionSet* selectionSet) {
	auto RestoreOld {false};
	if (selectionSet->numEntities() > static_cast<unsigned>(m_GripObjectLimit)) {
		Disable(true);
	} else {
		if (IsDisabled()) { RestoreOld = true; }
		Disable(false);
	}
	auto Database {OdDbDatabase::cast(selectionSet->baseDatabase()).get()};
	{ // Old Entities.
		OdDbStubPtrArray aOld;
		auto GripDataIterator {m_GripData.begin()};
		while (GripDataIterator != m_GripData.end()) {
			if (IsDisabled()) {
				aOld.push_back(GripDataIterator->first);
			} else {
				if (!selectionSet->isMember(GripDataIterator->first)) {
					aOld.push_back(GripDataIterator->first);
				} else {
					// Remove if subentities changed
					auto Removed {false};
					for (unsigned se = 0; se < GripDataIterator->second.gripDataSubEntity.size(); se++) {
						if (!selectionSet->isMember(GripDataIterator->second.gripDataSubEntity[se].subentPath)) {
							aOld.push_back(GripDataIterator->first);
							Removed = true;
							break;
						}
					}
					// Remove if new paths added also (workaround. technically new paths must be added on second step)
					if (!Removed) {
						auto SelectionSetIterator {SearchObjectSelectionSetIterator(selectionSet, GripDataIterator->first)};
						for (unsigned SubEntityIndex = 0; SubEntityIndex < SelectionSetIterator->subentCount(); SubEntityIndex++) {
							OdDbBaseFullSubentPath FullSubEntityPath;
							SelectionSetIterator->getSubentity(SubEntityIndex, FullSubEntityPath);
							auto searchPath {0u};
							auto Found {false};
							for (; searchPath < GripDataIterator->second.gripDataSubEntity.size(); searchPath++) {
								if (GripDataIterator->second.gripDataSubEntity.at(searchPath).subentPath == FullSubEntityPath) {
									Found = true;
									break;
								}
							}
							if (!Found) {
								aOld.push_back(GripDataIterator->first);
								break;
							}
						}
					}
				}
			}
			GripDataIterator++;
		}
		const auto Size {aOld.size()};
		for (unsigned i = 0; i < Size; i++) {
			RemoveEntityGrips(aOld[i], true);
			if (i % gc_GripManagerPageEachObject && Database) { Database->pageObjects(); }
		}
	}
	{ // New Entities.
		OdDbStubPtrArray aNew;
		auto SelectionSetIterator {selectionSet->newIterator()};
		while (!SelectionSetIterator->done()) {
			if (!IsDisabled() && m_GripData.end() == m_GripData.find(SelectionSetIterator->id())) {
				aNew.push_back(SelectionSetIterator->id());
			}
			SelectionSetIterator->next();
		}
		const auto Size {aNew.size()};
		for (unsigned i = 0; i < Size; i++) {
			UpdateEntityGrips(aNew[i]);
			if (i % gc_GripManagerPageEachObject && Database) { Database->pageObjects(); }
		}
	}
	UpdateInvisibleGrips();
}

void OdBaseGripManager::UpdateEntityGrips(OdDbStub* id) {
	RemoveEntityGrips(id, false);
	auto SelectionSet {WorkingSelectionSet()};
	if (SelectionSet.isNull() || !SelectionSet->isMember(id)) { return; }
	auto Entity {OpenObject(id)};
	if (Entity.isNull()) { return; }
	OdExGripDataPtrArray aExt;
	OdDbGripDataPtrArray aPts;
	auto SelectionSetIterator {SearchObjectSelectionSetIterator(SelectionSet, id)};
	if (SelectionSetIterator->subentCount() > 0) {
		for (unsigned long se = 0; se < SelectionSetIterator->subentCount(); se++) {
			OdDbBaseFullSubentPath subEntPath;
			SelectionSetIterator->getSubentity(se, subEntPath);
			aPts.clear();
			if (GetGripPointsAtSubentPath(Entity, subEntPath, aPts, ActiveViewUnitSize(), m_GripSize, ActiveViewDirection(), 0) == eOk) {
				const auto PreviousSize {aExt.size()};
				aExt.resize(PreviousSize + aPts.size());
				for (unsigned i = 0; i < aPts.size(); i++) {
					aExt[i + PreviousSize] = OdExGripData::CreateObject(subEntPath, aPts[i], aPts[i]->gripPoint(), this);
				}
			}
		}
	} else {
		if (eOk == GetGripPoints(Entity, aPts, ActiveViewUnitSize(), m_GripSize, ActiveViewDirection(), 0)) {
			aExt.resize(aPts.size());
			const auto Size {aExt.size()};
			for (unsigned i = 0; i < Size; i++) {
				aExt[i] = OdExGripData::CreateObject(id, aPts[i], aPts[i]->gripPoint(), this);
			}
		} else {
			OdGePoint3dArray OldPoints;
			if (eOk == GetGripPoints(Entity, OldPoints)) {
				aExt.resize(OldPoints.size());
				const auto Size {aExt.size()};
				for (unsigned i = 0; i < Size; i++) {
					aExt[i] = OdExGripData::CreateObject(id, nullptr, OldPoints[i], this);
				}
			}
		}
	}
	const auto bModel {IsModel(Entity)};
	if (!aExt.empty()) {
		const auto Size {aExt.size()};
		OdExGripDataExt dExt;
		for (unsigned i = 0; i < Size; i++) {
			OdDbBaseFullSubentPath EntityPath;
			if (aExt[i]->EntityPath(&EntityPath)) {
				auto Found {false};
				for (auto& GripDatum : dExt.gripDataSubEntity) {
					if (GripDatum.subentPath == EntityPath) {
						Found = true;
						GripDatum.subData.append(aExt[i]);
						break;
					}
				}
				if (!Found) {
					OdExGripDataSubent se;
					se.subentPath = EntityPath;
					se.subData.append(aExt[i]);
					dExt.gripDataSubEntity.append(se);
				}
			} else {
				dExt.dataArray.append(aExt[i]);
			}
		}
		m_GripData.insert(std::make_pair(id, dExt));
		for (unsigned i = 0; i < Size; i++) {
			ShowGrip(aExt[i], bModel);
		}
	}
}

void OdBaseGripManager::RemoveEntityGrips(OdDbStub* id, const bool fireDone) {
	auto GripDataIterator {m_GripData.find(id)};
	if (GripDataIterator != m_GripData.end()) {
		auto Entity {OpenObject(id)};
		if (Entity.get()) { GripStatus(Entity, OdDb::kGripsToBeDeleted); }
		const auto Model {IsModel(Entity)};
		const auto Size = GripDataIterator->second.dataArray.size();
		for (unsigned i = 0; i < Size; i++) {
			auto GripData {GripDataIterator->second.dataArray[i]};
			HideGrip(GripData, Model);
			if (!GripDataIterator->second.dataArray[i]->GripData().isNull() && GripDataIterator->second.dataArray[i]->GripData()->gripOpStatFunc()) {
				(*GripDataIterator->second.dataArray[i]->GripData()->gripOpStatFunc())(GripDataIterator->second.dataArray[i]->GripData(), id, OdDbGripOperations::kGripEnd);
			}
			GripDataIterator->second.dataArray[i] = nullptr;
		}
		for (auto& GripDataSubEntity : GripDataIterator->second.gripDataSubEntity) {
			for (auto& GripData : GripDataSubEntity.subData) {
				auto GripDataCopy {GripData};
				HideGrip(GripDataCopy, Model);
				GripData = nullptr;
			}
		}
		if (fireDone) {
			if (Entity.get()) { GripStatus(Entity, OdDb::kGripsDone); }
		}
		m_GripData.erase(GripDataIterator);
	}
}

void OdBaseGripManager::LocateGripsAt(const int x, const int y, OdExGripDataPtrArray& aResult) {
	aResult.clear();
	const auto X {static_cast<double>(x)};
	const auto Y {static_cast<double>(y)};
	OdGePoint3d FirstPoint;
	GripDataMap::const_iterator GripDataIterator {m_GripData.begin()};
	while (GripDataIterator != m_GripData.end()) {
		for (unsigned se = 0; se < GripDataIterator->second.gripDataSubEntity.size() + 1; se++) {
			const auto& aData = se == 0 ? GripDataIterator->second.dataArray : GripDataIterator->second.gripDataSubEntity[se - 1].subData;
			const auto DataSize {aData.size()};
			for (unsigned i = 0; i < DataSize; i++) {
				const auto& CurrentPoint {aData[i]->Point()};
				if (aResult.empty()) { // First grip is obtained by comparing grip point device position with cursor position.
					auto ptDC {CurrentPoint};
					ptDC.transformBy(ActiveGsView()->worldToDeviceMatrix());
					const auto DeltaX {fabs(X - ptDC.x)};
					const auto DeltaY {fabs(Y - ptDC.y)};
					const auto Ok {DeltaX <= m_GripSize && DeltaY <= m_GripSize};
					if (Ok) {
						FirstPoint = CurrentPoint;
						aResult.push_back(aData[i]);
					}
				} else { // Other grips are obtained by comparing world coordinates. The approach here is quite raw.
					if (CurrentPoint.isEqualTo(FirstPoint, 1E-4)) { aResult.push_back(aData[i]); }
				}
			}
		}
		GripDataIterator++;
	}
}

void OdBaseGripManager::LocateGripsByStatus(const OdDbGripOperations::DrawType eStatus, OdExGripDataPtrArray& aResult) {
	aResult.clear();
	GripDataMap::const_iterator GripDataIterator {m_GripData.begin()};
	while (GripDataIterator != m_GripData.end()) {
		for (unsigned se = 0; se < GripDataIterator->second.gripDataSubEntity.size() + 1; se++) {
			const auto& aData {se == 0 ? GripDataIterator->second.dataArray : GripDataIterator->second.gripDataSubEntity[se - 1].subData};
			const auto Size {aData.size()};
			for (unsigned i = 0; i < Size; i++) {
				if (eStatus == aData[i]->Status()) { aResult.push_back(aData[i]); }
			}
		}
		GripDataIterator++;
	}
}

namespace
{
	struct SortGripsAlongXAxis {
		bool operator()(const OdExGripDataPtr& grA, const OdExGripDataPtr& grB) {
			return OdPositive(grA->Point().x, grB->Point().x);
		}
	};
}

void OdBaseGripManager::UpdateInvisibleGrips() {
	OdExGripDataPtrArray Overall;
	GripDataMap::const_iterator GripDataIterator {m_GripData.begin()};
	while (GripDataIterator != m_GripData.end()) {
		Overall.insert(Overall.end(), GripDataIterator->second.dataArray.begin(), GripDataIterator->second.dataArray.end());
		for (const auto& GripDatum : GripDataIterator->second.gripDataSubEntity) {
			Overall.insert(Overall.end(), GripDatum.subData.begin(), GripDatum.subData.end());
		}
		GripDataIterator++;
	}
	for (auto& Grip : Overall) {
		Grip->SetInvisible(false);
		Grip->SetShared(false);
	}
	std::sort(Overall.begin(), Overall.end(), SortGripsAlongXAxis());
	const auto Size {Overall.size()};
	for (unsigned i = 0; i < Size; i++) {
		if (Overall[i]->IsShared()) continue;
		OdUInt32Array aEq;
		aEq.push_back(i);
		const auto ptIni = Overall[i]->Point();
		auto iNext = i + 1;
		while (iNext < Size) {
			const auto CurrentPoint {Overall[iNext]->Point()};
			if (OdEqual(ptIni.x, CurrentPoint.x, 1E-6)) {
				if (ptIni.isEqualTo(CurrentPoint, 1E-6)) { aEq.push_back(iNext); }
				iNext++;
			} else {
				break;
			}
		}
		if (aEq.size() >= 2) {
			auto Visible {0u};
			const auto jSize = aEq.size();
			for (unsigned j = 0; j < jSize; j++) {
				auto Grip {Overall[aEq[j]]};
				auto Ok {true};
				if (!Grip->GripData().isNull()) {
					if (Grip->GripData()->skipWhenShared()) { Ok = false; }
				} else {
					Ok = false;
				}
				if (Ok) {
					Visible = j;
					break;
				}
			}
			for (unsigned j = 0; j < jSize; j++) {
				auto Grip {Overall[aEq[j]]};
				Grip->SetShared(true);
				Grip->SetInvisible(j != Visible);
			}
		}
	}
}

void OdExGripManager::ShowGrip(OdExGripData* gripData, bool /*model*/) {
	auto PaperLayoutHelper {OdGsPaperLayoutHelper::cast(m_LayoutHelper)};
	const auto NumberOfViews {m_LayoutHelper->numViews()};
	if (PaperLayoutHelper.get()) {
		auto ActiveViewport {m_CommandContext->database()->activeViewportId().openObject()};
		OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);
		if (!AbstractViewportData.isNull() && AbstractViewportData->gsView(ActiveViewport)) {
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
	if (PaperLayoutHelper.get()) {
		for (auto ViewIndex = 0; ViewIndex < NumberOfViews; ViewIndex++) {
			m_LayoutHelper->viewAt(ViewIndex)->erase(gripData);
		}
	} else {
		for (auto ViewIndex = 0; ViewIndex < NumberOfViews; ViewIndex++) {
			m_LayoutHelper->viewAt(ViewIndex)->erase(gripData);
		}
	}
}

void OdBaseGripManager::setValue(const OdGePoint3d& value) {
	const auto NewPoint {EyeToUcsPlane(value, m_BasePoint)};
	const auto Size {m_GripDrags.size()};
	for (unsigned GripDragIndex = 0; GripDragIndex < Size; GripDragIndex++) {
		m_GripDrags[GripDragIndex]->CloneEntity(NewPoint);
	}
	m_LastPoint = NewPoint;
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

inline void resetDragging(OdGsDevice* device, const bool option) {
	if (!device) { return; }
	auto Properties {device->properties()};
	if (Properties.isNull()) { return; }
	if (!Properties->has(L"DrawDragging")) { return; }
	Properties->putAt(L"DrawDragging", OdRxVariantValue(option));
}

void OdExGripManager::DraggingStarted() {
	resetDragging(m_LayoutHelper, true);
}

void OdExGripManager::DraggingStopped() {
	resetDragging(m_LayoutHelper, false);
}

OdSelectionSetPtr OdExGripManager::WorkingSelectionSet() const {
	if (m_GetSelectionSet) {
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

double OdBaseGripManager::ActiveViewUnitSize() const {
	const auto ActiveView {ActiveGsView()};

	// <tas="Duplicates function of inaccessible 'OdGiViewport::getNumPixelsInUnitSquare' here."/>
	OdGePoint2d LowerLeft;
	OdGePoint2d UpperRight;
	ActiveView->getViewport(LowerLeft, UpperRight);
	OdGsDCRect ScreenRectangle;
	ActiveView->getViewport(ScreenRectangle);
	OdGePoint2d ptDim;
	ptDim.x = fabs(double(ScreenRectangle.m_max.x - ScreenRectangle.m_min.x) / ActiveView->fieldWidth() * (UpperRight.x - LowerLeft.x));
	ptDim.y = fabs(double(ScreenRectangle.m_max.y - ScreenRectangle.m_min.y) / ActiveView->fieldHeight() * (UpperRight.y - LowerLeft.y));
	OdGeVector3d v(m_GripSize / ptDim.x, 0, 0);
	v.transformBy(ActiveView->viewingMatrix());
	return v.length() / m_GripSize;
}

OdGeVector3d OdBaseGripManager::ActiveViewDirection() const {
	const auto View {ActiveGsView()};
	return (View->position() - View->target()).normal();
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
		if (!Plane.intersectWith(Line, NewPoint)) { NewPoint = basePoint; }
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

void OdBaseGripManager::Disable(const bool disable) {
	m_Disabled = disable;
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
	if (!id) { return Drawable; }
	Drawable = OdGiDrawable::cast(OdDbObjectId(id).openObject(isForWriteMode ? OdDb::kForWrite : OdDb::kForRead));
	return Drawable;
}

OdResult OdExGripManager::GetGripPointsAtSubentPath(OdGiDrawable* entity, const OdDbBaseFullSubentPath& path, OdDbGripDataPtrArray& grips, const double curViewUnitSize, const int gripSize, const OdGeVector3d& curViewDir, const unsigned long bitFlags) const {
	return OdDbEntity::cast(entity)->getGripPointsAtSubentPath(*static_cast<const OdDbFullSubentPath*>(&path), grips, curViewUnitSize, gripSize, curViewDir, bitFlags);
}

OdResult OdExGripManager::GetGripPoints(OdGiDrawable* entity, OdDbGripDataPtrArray& grips, const double curViewUnitSize, const int gripSize, const OdGeVector3d& curViewDir, const int bitFlags) const {
	OdDbEntity* Entity {OdDbEntity::cast(entity)};
	if (!Entity) { return eNotApplicable; }
	return Entity->getGripPoints(grips, curViewUnitSize, gripSize, curViewDir, bitFlags);
}

OdResult OdExGripManager::GetGripPoints(OdGiDrawable* entity, OdGePoint3dArray& gripPoints) const {
	OdDbEntity* Entity {OdDbEntity::cast(entity)};
	if (!Entity) { return eNotApplicable; }
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
	return !Entity || Entity->database()->getTILEMODE();
}

void OdExGripDbReactor::objectAppended(const OdDbDatabase* /*database*/, const OdDbObject* /*dbObject*/) noexcept {
	// New object.
}

void OdExGripDbReactor::objectModified(const OdDbDatabase*, const OdDbObject* object) {
	gripManager->UpdateEntityGrips(object->objectId());
	gripManager->UpdateInvisibleGrips();
}

void OdExGripDbReactor::objectErased(const OdDbDatabase* /*database*/, const OdDbObject* object, const bool erased) {
	if (erased) {
		gripManager->RemoveEntityGrips(object->objectId(), true);
		gripManager->UpdateInvisibleGrips();
	}
}
