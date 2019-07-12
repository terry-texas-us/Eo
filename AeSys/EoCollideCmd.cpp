#include "stdafx.h"
#include <Gi/GiDrawableImpl.h>
#include <Gs/Gs.h>
#include <Gs/GsBaseVectorizer.h>
#include <DbCommandContext.h>
#include <DbAbstractViewportData.h>
#include <DbBlockTableRecord.h>
#include <DbViewportTable.h>
#include <DbHostAppServices.h>
#include <ExTrackers.h>
#include <RxVariantValue.h>
#include "EditorObject.h"
#include "EoRtOrbitTracker.h"
#include "EoCollideMoveTracker.h"

const OdString OdExCollideCmd::groupName() const {
	return globalName();
}

const OdString OdExCollideCmd::globalName() const {
	return L"COLLIDE";
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
	if (SelectionSet->numEntities() == 0U) {
		throw OdEdCancel();
	}
	OdExTransactionSaver Saver(Database);
	Saver.StartTransaction();
	const auto BasePoint {UserIo->getPoint(L"Collide: Specify base point:")};
	CollideMoveTracker Tracker(BasePoint, SelectionSet, Database, View, DynamicHlt);
	const auto OffsetPoint {UserIo->getPoint(L"Collide: Specify second point:", OdEd::kGdsFromLastPoint | OdEd::kGptRubberBand, nullptr, OdString::kEmpty, &Tracker)};
}
