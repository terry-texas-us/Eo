#include "stdafx.h"
#include "DbViewportTable.h"
#include "DbViewportTableRecord.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "DbDictionary.h"
#include "EoDbDwgToPegFile.h"
#include "EoDbEntityToPrimitiveProtocolExtension.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
EoDbDwgToPegFile::EoDbDwgToPegFile(OdDbDatabasePtr database) {
	m_DatabasePtr_ = database;
}

void EoDbDwgToPegFile::ConvertToPeg(AeSysDoc* document) {
	if (!m_DatabasePtr_.isNull()) {
		ConvertHeaderSection(document);
		ConvertViewportTable(document);
		ConvertLayerTable(document);
		ConvertBlockTable(document);
		ConvertBlocks(document);
		ConvertEntities(document);
	}
}

void EoDbDwgToPegFile::ConvertBlockTable(gsl::not_null<AeSysDoc*> document) {
	OdDbBlockTablePtr BlockTable {m_DatabasePtr_->getBlockTableId().safeOpenObject(OdDb::kForRead)};
	OdString ReportItem;
	theApp.AddStringToReportList(ReportItem.format(L"<%s> Loading block table\n", static_cast<const wchar_t*>(BlockTable->desc()->name())));
	auto Iterator {BlockTable->newIterator()};
	for (Iterator->start(); !Iterator->done(); Iterator->step()) {
		OdDbBlockTableRecordPtr Block {Iterator->getRecordId().safeOpenObject(OdDb::kForRead)};
		EoDbBlock* pBlock;
		if (document->LookupBlock(Block->getName(), pBlock)) {
			// <tas="Block already defined? Should not occur. This is always an empty peg container?"</tas>
		}
		const unsigned short BlockFlags {Block->isAnonymous() ? 1U : 0U};
		pBlock = new EoDbBlock(BlockFlags, Block->origin(), Block->pathName());
		document->InsertBlock(Block->getName(), pBlock);
	}
}

void EoDbDwgToPegFile::ConvertHeaderSection(AeSysDoc* document) noexcept {
}

void EoDbDwgToPegFile::ConvertLayerTable(AeSysDoc* document) {
	OdDbLayerTablePtr Layers {m_DatabasePtr_->getLayerTableId().safeOpenObject(OdDb::kForWrite)};
	OdString ReportItem;
	theApp.AddStringToReportList(ReportItem.format(L"<%s> Loading layer definitions ...\n", static_cast<const wchar_t*>(Layers->desc()->name())));
	auto Iterator {Layers->newIterator()};
	for (Iterator->start(); !Iterator->done(); Iterator->step()) {
		OdDbLayerTableRecordPtr LayerTableRecord {Iterator->getRecordId().safeOpenObject(OdDb::kForWrite)};
		auto Name {LayerTableRecord->getName()};
		theApp.AddStringToReportList(ReportItem.format(L"<%s>  %s\n", static_cast<const wchar_t*>(LayerTableRecord->desc()->name()), static_cast<const wchar_t*>(Name)));
		if (document->FindLayerAt(Name) < 0) {
			auto Layer {new EoDbLayer(LayerTableRecord)};
			document->AddLayer(Layer);
			if (LayerTableRecord->isFrozen() || LayerTableRecord->isOff()) {
				document->GetLayerAt(LayerTableRecord->getName())->SetIsOff(true);
			}
			const auto ObjectId {LayerTableRecord->extensionDictionary()};
			if (!ObjectId.isNull()) {
				auto ObjectPtr {ObjectId.safeOpenObject(OdDb::kForRead)};
				OdDbDictionaryPtr Dictionary {ObjectPtr};
				auto DictionaryIterator {Dictionary->newIterator()};
				for (; !DictionaryIterator->done(); DictionaryIterator->next()) {
					theApp.AddStringToReportList(ReportItem.format(L"Layer Dictionary name: %s\n", static_cast<const wchar_t*>(DictionaryIterator->name())));
				}
			}
		}
	}
}

void EoDbDwgToPegFile::ConvertViewportTable(AeSysDoc* document) {
	OdDbViewportTablePtr ViewportTable {m_DatabasePtr_->getViewportTableId().safeOpenObject(OdDb::kForRead)};
	OdString ReportItem;
	theApp.AddStringToReportList(ReportItem.format(L"<%s> Loading viewport definitions ...\n", static_cast<const wchar_t*>(ViewportTable->desc()->name())));
	auto Iterator {ViewportTable->newIterator()};
	for (Iterator->start(); !Iterator->done(); Iterator->step()) {
		OdDbViewportTableRecordPtr Viewport {Iterator->getRecordId().safeOpenObject(OdDb::kForRead)};
		theApp.AddStringToReportList(ReportItem.format(L"<%s>  %s\n", static_cast<const wchar_t*>(Viewport->desc()->name()), static_cast<const wchar_t*>(Viewport->getName())));
		if (Viewport->extensionDictionary()) {
		}
	}
}

void EoDbDwgToPegFile::ConvertBlocks(AeSysDoc* document) {
	OdDbBlockTablePtr BlockTable = m_DatabasePtr_->getBlockTableId().safeOpenObject(OdDb::kForRead);
	OdString ReportItem;
	theApp.AddStringToReportList(ReportItem.format(L"<%s> Loading block definitions ...\n", static_cast<const wchar_t*>(BlockTable->desc()->name())));
	auto Iterator {BlockTable->newIterator()};
	for (Iterator->start(); !Iterator->done(); Iterator->step()) {
		OdDbBlockTableRecordPtr Block = Iterator->getRecordId().safeOpenObject(OdDb::kForRead);
		ReportItem.format(L"<%s>  %s", static_cast<const wchar_t*>(Block->desc()->name()), static_cast<const wchar_t*>(Block->getName()));
		if (Block->isAnonymous()) {
			ReportItem += L" (Anonymous block)";
		}
		if (Block->isLayout()) {
			ReportItem += L" (Layout block)";
		}
		theApp.AddStringToReportList(ReportItem + L"\n");
		if (Block->xrefStatus() != OdDb::kXrfNotAnXref) {
			if (Block->isFromExternalReference()) {
				theApp.AddStringToReportList(ReportItem.format(L"(External reference to [%s] not loaded)\n", static_cast<const wchar_t*>(Block->pathName())));
			}
		}
		if (Block->objectId() != m_DatabasePtr_->getModelSpaceId()) {
			ConvertBlock(Block, document);
		}
	}
}

void EoDbDwgToPegFile::ConvertBlock(OdDbBlockTableRecordPtr block, AeSysDoc* document) {
	EoDbBlock* Block;
	document->LookupBlock(block->getName(), Block);
	ConvertEntityToPrimitiveProtocolExtension ProtocolExtensions(document);
	ProtocolExtensions.Initialize();
	OdString ReportItem;
	theApp.AddStringToReportList(ReportItem.format(L"Loading Block %s entity definitions ...\n", static_cast<const wchar_t*>(block->getName())));
	if (block->isFromExternalReference()) {
		// External reference blocks have no entities. It points to a block in an external drawing.
		// Either the external drawing's *Model_space or any non-layout block.
	}
	auto EntitiesNotLoaded {0};
	auto EntityIterator {block->newIterator()};
	for (; !EntityIterator->done(); EntityIterator->step()) {
		const auto EntityObjectId {EntityIterator->objectId()};
		OdDbEntityPtr Entity = EntityObjectId.safeOpenObject(OdDb::kForRead);
		const auto NumberOfPrimitivesInBlock {Block->GetSize()};
		OdSmartPtr<EoDbConvertEntityToPrimitive> EntityConverter = Entity;
		EntityConverter->Convert(Entity, Block);
		if (NumberOfPrimitivesInBlock == Block->GetSize()) {
			EntitiesNotLoaded++;
		}
		if (Entity->extensionDictionary()) {
			theApp.AddStringToReportList(L"Entity extension dictionary not loaded\n");
		}
	}
	if (EntitiesNotLoaded != 0) {
		theApp.AddStringToReportList(ReportItem.format(L" %d entitities not loaded\n", EntitiesNotLoaded));
	}
	const auto ObjectId {block->extensionDictionary()};
	if (!ObjectId.isNull()) {
		auto ObjectPtr {ObjectId.safeOpenObject(OdDb::kForRead)};
		OdDbDictionaryPtr Dictionary = ObjectPtr;
		auto Iterator {Dictionary->newIterator()};
		for (; !Iterator->done(); Iterator->next()) {
			theApp.AddStringToReportList(ReportItem.format(L"Dictionary name: %s\n", static_cast<const wchar_t*>(Iterator->name())));
		}
	}
}

void EoDbDwgToPegFile::ConvertEntities(AeSysDoc* document) {
	ConvertEntityToPrimitiveProtocolExtension ProtocolExtensions(document);
	ProtocolExtensions.Initialize();
	OdDbBlockTableRecordPtr Modelspace = m_DatabasePtr_->getModelSpaceId().safeOpenObject(OdDb::kForRead);
	OdString ReportItem;
	theApp.AddStringToReportList(ReportItem.format(L"<%s> Loading Layout Object definitions ...\n", static_cast<const wchar_t*>(Modelspace->desc()->name())));
	theApp.AddStringToReportList(ReportItem.format(L"Loading %s entity definitions ...\n", static_cast<const wchar_t*>(Modelspace->getName())));
	auto EntitiesNotLoaded {0};
	auto EntityIterator {Modelspace->newIterator()};
	for (; !EntityIterator->done(); EntityIterator->step()) {
		const auto EntityObjectId {EntityIterator->objectId()};
		OdDbEntityPtr Entity = EntityObjectId.safeOpenObject(OdDb::kForRead);
		auto Layer {document->GetLayerAt(Entity->layer())};
		auto Group {new EoDbGroup()};
		OdSmartPtr<EoDbConvertEntityToPrimitive> EntityConverter = Entity;
		EntityConverter->Convert(Entity, Group);
		if (Group->IsEmpty()) {
			delete Group;
			EntitiesNotLoaded++;
		} else {
			Layer->AddTail(Group);
		}
	}
	theApp.AddStringToReportList(ReportItem.format(L" %d Modelspace entitities not loaded\n", EntitiesNotLoaded));
	const auto ObjectId {Modelspace->extensionDictionary()};
	if (!ObjectId.isNull()) {
		auto ObjectPtr {ObjectId.safeOpenObject(OdDb::kForRead)};
		OdDbDictionaryPtr Dictionary {ObjectPtr};
		auto Iterator {Dictionary->newIterator()};
		for (; !Iterator->done(); Iterator->next()) {
			theApp.AddStringToReportList(ReportItem.format(L"Dictionary name: %s\n", static_cast<const wchar_t*>(Iterator->name())));
		}
	}
	// <tas="Paperspace entities are loaded with blocks already
	/*
		EntitiesNotLoaded = 0;
	
		OdDbBlockTableRecordPtr Paperspace = m_DatabasePtr_->getPaperSpaceId().safeOpenObject(OdDb::kForRead);
	
		EntityIterator = Paperspace->newIterator();
		for (; !EntityIterator->done(); EntityIterator->step()) {
			OdDbObjectId EntityObjectId = EntityIterator->objectId();
			OdDbEntityPtr Entity = EntityObjectId.safeOpenObject(OdDb::kForRead);
	
			auto Layer {document->GetLayerAt(Entity->layer())};
	
			EoDbGroup* Group = new EoDbGroup();
			OdSmartPtr<EoDbConvertEntityToPrimitive> EntityConverter = Entity;
			EntityConverter->Convert(Entity, Group);
	
			if (Group->IsEmpty()) {
				delete Group;
				EntitiesNotLoaded++;
			} else {
				Layer->AddTail(Group);
			}
		}
		TRACE1(" %d Paperspace entitities not loaded\n", EntitiesNotLoaded);
	*/
	// </tas>
}
