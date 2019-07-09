// Extracted class from Examples\Editor\ExGripManager.cpp (last compare 20.5)
#include "stdafx.h"
#include <UInt32Array.h>
#include "ExGripManager.h"
#include "OdBaseGripManager.h"
constexpr unsigned gc_GripManagerPageEachObject = 200;

namespace
{
	OdSelectionSetIteratorPtr SearchObjectSelectionSetIterator(OdSelectionSetPtr selectionSet, OdDbStub* id) {
		auto SelectionSetIterator {selectionSet->newIterator()};
		while (!SelectionSetIterator->done()) {
			if (SelectionSetIterator->id() == id) {
				return SelectionSetIterator;
			}
			SelectionSetIterator->next();
		}
		return OdSelectionSetIteratorPtr();
	}
} // namespace
namespace
{
	struct SortGripsAlongXAxis {
		bool operator()(const OdExGripDataPtr& grA, const OdExGripDataPtr& grB) {
			return OdPositive(grA->Point().x, grB->Point().x);
		}
	};
} // namespace
OdBaseGripManager::OdBaseGripManager() noexcept {
	m_GripData.clear();
	m_HoverGripsData.clear();
	m_GripDrags.clear();
}

OdBaseGripManager::~OdBaseGripManager() {
	EndHover();
}

bool OdBaseGripManager::OnMouseDown(const int x, const int y, const bool shiftIsDown) {
	EndHover();
	OdExGripDataPtrArray aKeys;
	LocateGripsAt(x, y, aKeys);
	if (aKeys.empty()) {
		return false;
	}
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
					if (Grip->GripData()->hotGripFunc() != nullptr) {
						int Flags {OdDbGripOperations::kMultiHotGrip};
						if (Grip->IsShared()) {
							Flags |= OdDbGripOperations::kSharedGrip;
						}
						const auto Result {(*Grip->GripData()->hotGripFunc())(Grip->GripData(), Grip->EntityId(), Flags)};
						if (Result == eGripOpGripHotToWarm) {
							CurrentStatus = OdDbGripOperations::kWarmGrip;
						}
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
			for (auto Grip : aKeys) {
				auto New {OdDbGripOperations::kHotGrip};
				if (!Grip->GripData().isNull() && Grip->GripData()->hotGripFunc() != nullptr) {
					auto Flags {0};
					if (Grip->IsShared()) {
						Flags |= OdDbGripOperations::kSharedGrip;
					}
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
		if (GetNew) {
			UpdateEntityGrips(EntityIdToUpdate);
		}
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
						if (m_ClockStartHover == 0) {
							m_ClockStartHover = clock();
						}
						if ((clock() - m_ClockStartHover) * 1000 / CLOCKS_PER_SEC > 300) { // 300 ms delay before hover
							auto Flags {0};
							if (Grip->IsShared()) {
								Flags = OdDbGripOperations::kSharedGrip;
							}
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
	if (m_HoverGripsData.empty()) {
		return false;
	}
	for (auto HoverGripData : m_HoverGripsData) {
		if (HoverGripData->Status() == OdDbGripOperations::kHoverGrip) {
			HoverGripData->SetStatus(OdDbGripOperations::kWarmGrip);
			OnModified(HoverGripData);
		}
	}
	m_HoverGripsData.clear();
	return true;
}

void OdBaseGripManager::SelectionSetChanged(OdSelectionSet* selectionSet) {
	auto RestoreOld {false};
	if (selectionSet->numEntities() > static_cast<unsigned>(m_GripObjectLimit)) {
		Disable(true);
	} else {
		if (IsDisabled()) {
			RestoreOld = true;
		}
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
							auto searchPath {0U};
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
			if (i % gc_GripManagerPageEachObject && Database != nullptr) {
				Database->pageObjects();
			}
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
			if (i % gc_GripManagerPageEachObject && Database != nullptr) {
				Database->pageObjects();
			}
		}
	}
	UpdateInvisibleGrips();
}

void OdBaseGripManager::UpdateEntityGrips(OdDbStub* id) {
	RemoveEntityGrips(id, false);
	auto SelectionSet {WorkingSelectionSet()};
	if (SelectionSet.isNull() || !SelectionSet->isMember(id)) {
		return;
	}
	auto Entity {OpenObject(id)};
	if (Entity.isNull()) {
		return;
	}
	OdExGripDataPtrArray aExt;
	OdDbGripDataPtrArray aPts;
	auto SelectionSetIterator {SearchObjectSelectionSetIterator(SelectionSet, id)};
	if (SelectionSetIterator->subentCount() > 0) {
		for (unsigned se = 0; se < SelectionSetIterator->subentCount(); se++) {
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
		if (Entity.get() != nullptr) {
			GripStatus(Entity, OdDb::kGripsToBeDeleted);
		}
		const auto Model {IsModel(Entity)};
		const auto Size = GripDataIterator->second.dataArray.size();
		for (unsigned i = 0; i < Size; i++) {
			auto GripData {GripDataIterator->second.dataArray[i]};
			HideGrip(GripData, Model);
			if (!GripDataIterator->second.dataArray[i]->GripData().isNull() && GripDataIterator->second.dataArray[i]->GripData()->gripOpStatFunc() != nullptr) {
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
			if (Entity.get() != nullptr) { GripStatus(Entity, OdDb::kGripsDone); }
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
					if (CurrentPoint.isEqualTo(FirstPoint, 1E-4)) {
						aResult.push_back(aData[i]);
					}
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
				if (eStatus == aData[i]->Status()) {
					aResult.push_back(aData[i]);
				}
			}
		}
		GripDataIterator++;
	}
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
		if (Overall[i]->IsShared()) { continue; }
		OdUInt32Array aEq;
		aEq.push_back(i);
		const auto ptIni = Overall[i]->Point();
		auto iNext = i + 1;
		while (iNext < Size) {
			const auto CurrentPoint {Overall[iNext]->Point()};
			if (OdEqual(ptIni.x, CurrentPoint.x, 1E-6)) {
				if (ptIni.isEqualTo(CurrentPoint, 1E-6)) {
					aEq.push_back(iNext);
				}
				iNext++;
			} else {
				break;
			}
		}
		if (aEq.size() >= 2) {
			auto Visible {0U};
			const auto jSize = aEq.size();
			for (unsigned j = 0; j < jSize; j++) {
				auto Grip {Overall[aEq[j]]};
				auto Ok {true};
				if (!Grip->GripData().isNull()) {
					if (Grip->GripData()->skipWhenShared()) {
						Ok = false;
					}
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

void OdBaseGripManager::setValue(const OdGePoint3d& value) {
	const auto NewPoint {EyeToUcsPlane(value, m_BasePoint)};
	const auto Size {m_GripDrags.size()};
	for (unsigned GripDragIndex = 0; GripDragIndex < Size; GripDragIndex++) {
		m_GripDrags[GripDragIndex]->CloneEntity(NewPoint);
	}
	m_LastPoint = NewPoint;
}

double OdBaseGripManager::ActiveViewUnitSize() const {
	const auto ActiveView {ActiveGsView()};
	// <tas="Duplicates function of inaccessible 'OdGiViewport::getNumPixelsInUnitSquare' here."/>
	OdGePoint2d LowerLeft;
	OdGePoint2d UpperRight;
	ActiveView->getViewport(LowerLeft, UpperRight);
	OdGsDCRect ScreenRectangle;
	ActiveView->getViewport(ScreenRectangle);
	OdGePoint2d PixelDensity;
	PixelDensity.x = fabs(double(ScreenRectangle.m_max.x - ScreenRectangle.m_min.x) / ActiveView->fieldWidth() * (UpperRight.x - LowerLeft.x));
	PixelDensity.y = fabs(double(ScreenRectangle.m_max.y - ScreenRectangle.m_min.y) / ActiveView->fieldHeight() * (UpperRight.y - LowerLeft.y));
	OdGeVector3d GripEdgeSize(m_GripSize / PixelDensity.x, 0, 0);
	GripEdgeSize.transformBy(ActiveView->viewingMatrix());
	return GripEdgeSize.length() / m_GripSize;
}

OdGeVector3d OdBaseGripManager::ActiveViewDirection() const {
	const auto View {ActiveGsView()};
	return (View->position() - View->target()).normal();
}

void OdBaseGripManager::Disable(const bool disable) {
	m_Disabled = disable;
}
