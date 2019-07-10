// Extracted class from Examples\Editor\ExGripManager.cpp (last compare 20.5)
#include "stdafx.h"
#include "OdExGripDrag.h"
#include "ExGripManager.h"

namespace
{
	OdBaseGripManager::OdExGripDataSubent& GetSubentGripData(OdBaseGripManager::OdExGripDataExt& ext, const OdDbBaseFullSubentPath& entityPath) {
		for (auto& GripData : ext.gripDataSubEntity) {
			if (GripData.subentPath == entityPath) {
				return GripData;
			}
		}
		ODA_FAIL();
		return ext.gripDataSubEntity.at(0);
	}
}

OdExGripDragPtr OdExGripDrag::CreateObject(OdDbStub* id, OdBaseGripManager* gripManager) {
	auto GripDrag {RXIMPL_CONSTR(OdExGripDrag)};
	GripDrag->m_SubentPath.objectIds().append(id);
	GripDrag->m_GripManager = gripManager;
	return GripDrag;
}

OdExGripDragPtr OdExGripDrag::CreateObject(const OdDbBaseFullSubentPath& entityPath, OdBaseGripManager* gripManager) {
	auto GripDrag {RXIMPL_CONSTR(OdExGripDrag)};
	GripDrag->m_SubentPath = entityPath;
	GripDrag->m_GripManager = gripManager;
	return GripDrag;
}

OdExGripDrag::OdExGripDrag() noexcept {
	//m_SubentPath = OdDbBaseFullSubentPath();
	m_Clone = nullptr;
	m_GripManager = nullptr;
}

OdDbStub* OdExGripDrag::EntityId() const {
	return m_SubentPath.objectIds().last();
}

bool OdExGripDrag::EntityPath(OdDbBaseFullSubentPath* subentPath) const {
	if (subentPath != nullptr) {
		*subentPath = m_SubentPath;
	}
	return m_SubentPath.subentId() != OdDbSubentId();
}

bool OdExGripDrag::LocateActiveGrips(OdIntArray& indices) const {
	const auto& GripData {EntityPath() ? GetSubentGripData(m_GripManager->m_GripData[EntityId()], m_SubentPath).subData : m_GripManager->m_GripData[EntityId()].dataArray};
	auto ExMethod {true};
	indices.clear();
	for (unsigned i = 0; i < GripData.size(); i++) {
		if (GripData[i]->GripData().isNull()) {
			ExMethod = false;
		}
		if (OdDbGripOperations::kDragImageGrip == GripData[i]->Status()) {
			indices.push_back(static_cast<int>(i));
		}
	}
	ODA_ASSERT(GripData.empty() || !indices.empty());
	return ExMethod;
}

void OdExGripDrag::CloneEntity() {
	m_Clone = m_GripManager != nullptr ? m_GripManager->CloneEntity(EntityId()) : OdGiDrawablePtr();
}

void OdExGripDrag::CloneEntity(const OdGePoint3d& ptMoveAt) {
	CloneEntity();
	if (m_Clone.isNull()) {
		return;
	}
	OdIntArray Indices;
	const auto ExMethod {LocateActiveGrips(Indices)};
	const auto Offset {ptMoveAt - m_GripManager->m_BasePoint};
	if (ExMethod) {
		OdDbGripDataPtrArray CloneData;
		if (EntityPath()) {
			m_GripManager->GetGripPointsAtSubentPath(m_Clone, m_SubentPath, CloneData, m_GripManager->ActiveViewUnitSize(), m_GripManager->m_GripSize, m_GripManager->ActiveViewDirection(), 0);
		} else {
			m_GripManager->GetGripPoints(m_Clone, CloneData, m_GripManager->ActiveViewUnitSize(), m_GripManager->m_GripSize, m_GripManager->ActiveViewDirection(), 0);
		}
		OdDbVoidPtrArray aIds;
		for (auto& GripDataIndex : Indices) {
			if (gsl::narrow_cast<unsigned>(GripDataIndex) < CloneData.size()) {
				aIds.push_back(CloneData[static_cast<unsigned>(GripDataIndex)]->appData());
			} else {
				ODA_ASSERT(0);
			}
		}
		for (auto& GripData : CloneData) {
			if (GripData->gripOpStatFunc() != nullptr) {
				GripData->gripOpStatFunc()(GripData, OdDbObjectId::kNull, OdDbGripOperations::kGripStart);
			}
		}
		if (EntityPath()) {
			OdDbBaseFullSubentPathArray SubentPaths;
			SubentPaths.append(m_SubentPath);
			m_GripManager->MoveGripPointsAtSubentPaths(m_Clone, SubentPaths, aIds, Offset, 0);
			m_GripManager->SubentGripStatus(m_Clone, OdDb::kGripsToBeDeleted, m_SubentPath);
		} else {
			m_GripManager->MoveGripPointsAt(m_Clone, aIds, Offset, 0);
			m_GripManager->GripStatus(m_Clone, OdDb::kGripsToBeDeleted);
		}
		for (auto& GripData : CloneData) {
			if (GripData->gripOpStatFunc() != nullptr) {
				GripData->gripOpStatFunc()(GripData, nullptr, OdDbGripOperations::kGripEnd);
			}
		}
	} else {
		m_GripManager->MoveGripPointsAt(m_Clone, Indices, Offset);
		m_GripManager->GripStatus(m_Clone, OdDb::kGripsToBeDeleted);
	}
	m_GripManager->OnModified(this);
}

void OdExGripDrag::MoveEntity(const OdGePoint3d& moveAtPoint) const {
	OdIntArray Indices;
	const auto ExMethod {LocateActiveGrips(Indices)};
	const auto Offset {moveAtPoint - m_GripManager->m_BasePoint};
	auto Entity {m_GripManager->OpenObject(EntityId(), OdDb::kForWrite != 0)};
	ODA_ASSERT(Entity.get());
	const auto& rData {EntityPath() ? GetSubentGripData(m_GripManager->m_GripData[EntityId()], m_SubentPath).subData : m_GripManager->m_GripData[EntityId()].dataArray};
	if (ExMethod) {
		OdDbVoidPtrArray aIds;
		for (auto Index : Indices) {
			if (Index < gsl::narrow_cast<int>(rData.size())) {
				aIds.push_back(rData[static_cast<unsigned>(Index)]->GripData()->appData());
			} else {
				ODA_ASSERT(0);
			}
		}
		if (EntityPath()) {
			OdDbBaseFullSubentPathArray Paths;
			Paths.append(m_SubentPath);
			m_GripManager->MoveGripPointsAtSubentPaths(Entity, Paths, aIds, Offset, 0);
		} else {
			m_GripManager->MoveGripPointsAt(Entity, aIds, Offset, 0);
		}
	} else {
		m_GripManager->MoveGripPointsAt(Entity, Indices, Offset);
	}
}

void OdExGripDrag::NotifyDragStarted() const {
	if (m_GripManager == nullptr) { return; }
	auto Entity {m_GripManager->OpenObject(EntityId())};
	if (Entity.get() != nullptr) {
		m_GripManager->DragStatus(Entity, OdDb::kDragStart);
	}
	m_GripManager->DraggingStarted();
}

void OdExGripDrag::NotifyDragEnded() const {
	if (m_GripManager == nullptr) { return; }
	auto Entity {m_GripManager->OpenObject(EntityId())};
	if (Entity.get() != nullptr) {
		m_GripManager->DragStatus(Entity, OdDb::kDragEnd);
	}
	m_GripManager->DraggingStopped();
}

void OdExGripDrag::NotifyDragAborted() const {
	if (m_GripManager == nullptr) { return; }
	auto Entity {m_GripManager->OpenObject(EntityId())};
	if (Entity.get() != nullptr) {
		m_GripManager->DragStatus(Entity, OdDb::kDragAbort);
	}
	m_GripManager->DraggingStopped();
}

unsigned long OdExGripDrag::subSetAttributes(OdGiDrawableTraits* drawableTraits) const {
	if (m_Clone.isNull()) { return kDrawableIsInvisible; }
	const auto Result {m_Clone->setAttributes(drawableTraits)};
	auto EntityTraits {OdGiSubEntityTraits::cast(drawableTraits)};
	if (EntityTraits.get() != nullptr) { EntityTraits->setFillType(kOdGiFillNever); }
	return Result;
}

bool OdExGripDrag::subWorldDraw(OdGiWorldDraw* worldDraw) const {
	if (m_Clone.isNull()) {
		return true;
	}
	return m_Clone->worldDraw(worldDraw);
}

void OdExGripDrag::subViewportDraw(OdGiViewportDraw* viewportDraw) const {
	if (m_Clone.get() != nullptr) { m_Clone->viewportDraw(viewportDraw); }
}
