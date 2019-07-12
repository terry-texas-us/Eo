#pragma once
#include <Gi/GiPathNode.h>
#include <Gs/Gs.h>
#include <DbObjectId.h>
#include <DbSubentId.h>
#include <DbBlockReference.h>

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
		if (Insert.get() != nullptr) {
			AddNode(Insert->blockTableRecord());
		}
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
		if (PathObjectIdsIterator == PathObjectIds.end()) {
			throw OdError(eInvalidInput);
		}
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
