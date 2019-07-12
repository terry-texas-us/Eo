#pragma once
#include <Gs/GsViewLocalId.h>
#include <Gi/GiDrawableImpl.h>
#include <StaticRxObject.h>
#include <ExTrackers.h>
#include <DbEntity.h>
#include <DbSSet.h>
#include "EoCollideGsPath.h"

OdExCollideGsPath* FromGiPath(const OdGiPathNode* path, bool bTruncateToRef = false);

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
	CollideMoveTracker(const OdGePoint3d basePoint, OdDbSelectionSet* selectionSet, OdDbDatabasePtr database, OdGsView* view, const bool dynamicHlt);

	virtual ~CollideMoveTracker();

	void setValue(const OdGePoint3d& value) override;

	virtual void DoCollideWithAll();

	virtual void Highlight(OdArray<OdExCollideGsPath*>& newPaths);

	int addDrawables(OdGsView* view) override;

	void removeDrawables(OdGsView* view) override {
		for (auto EntityIndex = static_cast<int>(m_SelectionSetEntities.size() - 1); EntityIndex >= 0; --EntityIndex) {
			view->erase(m_SelectionSetEntities[static_cast<unsigned>(EntityIndex)]);
		}
	}
};
