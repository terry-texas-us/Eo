#include "stdafx.h"
#include <Gi/GiDrawableImpl.h>
#include <Gs/Gs.h>
#include <Gs/GsBaseVectorizer.h>
#include <DbCommandContext.h>
#include <DbAbstractViewportData.h>
#include <DbBlockTableRecord.h>
#include <DbViewportTable.h>
#include <DbHostAppServices.h>
#include <RxVariantValue.h>
#include <Gs/GsModel.h>
#include <Gi/GiPathNode.h>
#include "EditorObject.h"
#include "EoRtOrbitTracker.h"
#include "EoCollideMoveTracker.h"
#include "EoCollideGsPath.h"
#include "EoCollideAllCmd.h"

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
