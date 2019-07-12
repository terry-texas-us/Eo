#include "stdafx.h"
#include <Gi/GiDrawableImpl.h>
#include <Gs/GsBaseVectorizer.h>
#include <DbBlockTableRecord.h>
#include <Gs/GsModel.h>
#include <Gi/GiPathNode.h>
#include "EoCollideMoveTracker.h"
#include "EoCollideGsPath.h"

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
				if (Object->isKindOf(OdDbBlockReference::desc())) {
					Add = false;
				}
			}
		}
	}
	return Add;
}

OdExCollideGsPath* FromGiPath(const OdGiPathNode* path, const bool truncateToRef) {
	if (path == nullptr) { return nullptr; }
	const auto Result {new OdExCollideGsPath};
	AddNodeToPath(Result, path, truncateToRef);
	return Result;
}

CollideMoveTracker::CollideMoveTracker(const OdGePoint3d basePoint, OdDbSelectionSet* selectionSet, OdDbDatabasePtr database, OdGsView* view, const bool dynamicHlt)
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
		if (m_Model == nullptr && Entity->gsNode() != nullptr) {
			m_Model = Entity->gsNode()->model();
		}
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
					if (!SubEntity.isNull()) {
						m_SelectionSetEntities.push_back(SubEntity);
					}
				}
			}
		}
		if (Entity.isNull()) {
			continue;
		}
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

CollideMoveTracker::~CollideMoveTracker() {
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

void CollideMoveTracker::setValue(const OdGePoint3d& value) {
	const auto NewTransform = GetTransform(value);
	// Compensate previous transform
	auto Transform {m_LastTransform.inverse()};
	Transform.preMultBy(NewTransform);
	// Remember last transform
	m_LastTransform = NewTransform;
	for (auto EntityIndex = static_cast<int>(m_SelectionSetEntities.size() - 1); EntityIndex >= 0; --EntityIndex) {
		m_SelectionSetEntities[static_cast<unsigned>(EntityIndex)]->transformBy(Transform);
	}
	DoCollideWithAll();
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
			if (Path != nullptr || pPathNode2->persistentDrawableId() != nullptr) {
				m_Paths.push_back(Path);
			}
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

int CollideMoveTracker::addDrawables(OdGsView* view) {
	for (auto EntityIndex = static_cast<int>(m_SelectionSetEntities.size() - 1); EntityIndex >= 0; --EntityIndex) {
		view->add(m_SelectionSetEntities[static_cast<unsigned>(EntityIndex)], nullptr);
	}
	return 1;
}
