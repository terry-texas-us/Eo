#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"

EoDbPegFile::EoDbPegFile(OdDbDatabasePtr database)
	: m_Database(database) {
}
EoDbPegFile::~EoDbPegFile() {
}

void EoDbPegFile::Load(AeSysDoc* document) {
	try {
		ReadHeaderSection(document);
		ReadTablesSection(document);
		ReadBlocksSection(document);
		ReadGroupsSection(document);
	}
	catch(LPWSTR szMessage) {
		::MessageBoxW(0, szMessage, L"EoDbPegFile", MB_ICONWARNING | MB_OK);
	}
}

void EoDbPegFile::ReadHeaderSection(AeSysDoc*) {
	if (ReadUInt16() != kHeaderSection) {
		throw L"Exception ReadHeaderSection: Expecting sentinel EoDb::kHeaderSection.";
	}
	// <tas="All database settings can be set here ??"</tas>
	m_Database->setPDMODE(64); // Point display mode - square
	m_Database->setPDSIZE(.015625); 
	if (ReadUInt16() != kEndOfSection) {
		throw L"Exception ReadHeaderSection: Expecting sentinel EoDb::kEndOfSection.";
	}
}
void EoDbPegFile::ReadTablesSection(AeSysDoc* document) {
	if (ReadUInt16() != kTablesSection) {
		throw L"Exception ReadTablesSection: Expecting sentinel EoDb::kTablesSection.";
	}
	ReadViewportTable(document);
	ReadLinetypesTable();
	ReadLayerTable(document);
	if (ReadUInt16() != kEndOfSection) {
		throw L"Exception ReadTablesSection: Expecting sentinel EoDb::kEndOfSection.";
	}
}
void EoDbPegFile::ReadViewportTable(AeSysDoc*) {
	if (ReadUInt16() != kViewPortTable) {
		throw L"Exception ReadViewportTable: Expecting sentinel EoDb::kViewPortTable.";
	}
	ReadUInt16();
	if (ReadUInt16() != kEndOfTable) {
		throw L"Exception ReadViewportTable: Expecting sentinel EoDb::kEndOfTable.";
	}
}
void EoDbPegFile::ReadLinetypesTable() {
	if (ReadUInt16() != kLinetypeTable)
		throw L"Exception ReadLinetypesTable: Expecting sentinel EoDb::kLinetypeTable.";

	OdDbLinetypeTablePtr Linetypes = m_Database->getLinetypeTableId().safeOpenObject(OdDb::kForWrite);

	EoUInt16 NumberOfLinetypes = ReadUInt16();
	double* DashLength = new double[32];

	for (EoUInt16 n = 0; n < NumberOfLinetypes; n++) {
		OdString Name;
		ReadString(Name);
				
		/* EoUInt16 Flags = */ ReadUInt16();
		
		OdString Comments;
		ReadString(Comments);
				
		EoUInt16 NumberOfDashes = ReadUInt16();
		double PatternLength;
		PatternLength = ReadDouble();

		for (EoUInt16 n = 0; n < NumberOfDashes; n++) {
			DashLength[n] = ReadDouble();
		}
		if (Linetypes->getAt(Name).isNull()) {
			OdDbLinetypeTableRecordPtr Linetype = OdDbLinetypeTableRecord::createObject();
			
			Linetype->setName(Name);
			Linetype->setComments(Comments);
			Linetype->setNumDashes(NumberOfDashes);
			Linetype->setPatternLength(PatternLength);
		
			if (NumberOfDashes > 0) {
				for (int DashIndex = 0; DashIndex < NumberOfDashes; DashIndex++) {
					Linetype->setDashLengthAt(DashIndex, DashLength[DashIndex]);
					Linetype->setShapeStyleAt(DashIndex, OdDbObjectId::kNull);
					Linetype->setShapeNumberAt(DashIndex, 0);
					Linetype->setTextAt(DashIndex, L" ");
					Linetype->setShapeScaleAt(DashIndex, 1.);
					Linetype->setShapeOffsetAt(DashIndex, OdGeVector2d(0., 0.));
					Linetype->setShapeRotationAt(DashIndex, 0.);
					Linetype->setShapeIsUcsOrientedAt(DashIndex, false);
				}
			}
			Linetypes->add(Linetype);
		}
	}
	delete [] DashLength;

	if (ReadUInt16() != kEndOfTable)
		throw L"Exception ReadLinetypesTable: Expecting sentinel EoDb::kEndOfTable.";
}
void EoDbPegFile::ReadLayerTable(AeSysDoc* document) {
	if (ReadUInt16() != kLayerTable) {
		throw L"Exception ReadLayerTable: Expecting sentinel EoDb::kLayerTable.";
	}
	OdDbLayerTablePtr Layers = document->LayerTable(OdDb::kForWrite);
	EoUInt16 NumberOfLayers = ReadUInt16();
	for (EoUInt16 LayerIndex = 0; LayerIndex < NumberOfLayers; LayerIndex++) {
		OdString Name;
		ReadString(Name);
		/* EoUInt16 TracingFlags = */ ReadUInt16();
		
		EoUInt16 StateFlags = ReadUInt16();
		StateFlags |= EoDbLayer::kIsResident;
		if ((StateFlags & EoDbLayer::kIsInternal) != EoDbLayer::kIsInternal) {
			if (Name.find('.') == - 1) {
				Name += L".jb1";
			}
		}
		EoInt16 ColorIndex = ReadInt16();
		OdString LinetypeName;
		ReadString(LinetypeName);

		EoDbLayer* Layer;
		OdDbLayerTableRecordPtr LayerTableRecord;
		if (Layers->has(Name)) { // should be the default layer (0) on a load
			LayerTableRecord = Layers->getAt(Name).safeOpenObject(OdDb::kForWrite);
			Layer = document->GetLayerAt(Name);
		}
		else {
			LayerTableRecord = OdDbLayerTableRecord::createObject();
			Layer = new EoDbLayer(LayerTableRecord);
			LayerTableRecord->setName(Name);
			LayerTableRecord = document->AddLayerTo(Layers, Layer).safeOpenObject(OdDb::kForWrite);
		}	
		Layer->SetStateFlags(StateFlags);
			
		if ((StateFlags & EoDbLayer::kIsCurrent) == EoDbLayer::kIsCurrent) {
			document->SetCurrentLayer(LayerTableRecord);
		}
		if ((StateFlags & EoDbLayer::kIsLocked) == EoDbLayer::kIsLocked) {
			Layer->SetIsLocked(true);
		}
		Layer->SetColorIndex(ColorIndex);
		OdDbObjectId Linetype;
		if (LinetypeName.iCompare(L"Continuous") == 0) {
			Linetype = m_Database->getLinetypeContinuousId();
		}
		else {
			OdDbLinetypeTablePtr Linetypes = m_Database->getLinetypeTableId().safeOpenObject(OdDb::kForRead);
			Linetype = Linetypes->getAt(LinetypeName);	
		}
		Layer->SetLinetype(Linetype);
	}
	if (ReadUInt16() != kEndOfTable) {
		throw L"Exception ReadLayerTable: Expecting sentinel EoDb::kEndOfTable.";
	}
}
void EoDbPegFile::ReadBlocksSection(AeSysDoc* document) {
	if (ReadUInt16() != kBlocksSection) {
		throw L"Exception ReadBlocksSection: Expecting sentinel EoDb::kBlocksSection.";
	}
	OdDbBlockTablePtr BlockTable = m_Database->getBlockTableId().safeOpenObject(OdDb::kForWrite);

	OdString Name;
	OdString PathName;

	EoUInt16 NumberOfBlocks = ReadUInt16();

	for (EoUInt16 BlockIndex = 0; BlockIndex < NumberOfBlocks; BlockIndex++) {
		EoUInt16 NumberOfPrimitives = ReadUInt16();

		ReadString(Name);
		EoUInt16 BlockTypeFlags = ReadUInt16();
		OdGePoint3d BasePoint = ReadPoint3d();
		EoDbBlock* Block = new EoDbBlock(BlockTypeFlags, BasePoint, PathName);

		document->InsertBlock(Name, Block);
				
		OdDbBlockTableRecordPtr BlockTableRecord = m_Database->getModelSpaceId().safeOpenObject();
		if (BlockTable->getAt(Name).isNull()) {
			BlockTableRecord = OdDbBlockTableRecord::createObject();
			BlockTableRecord->setName(Name);
			BlockTableRecord->setOrigin(BasePoint);
			BlockTableRecord->setPathName(PathName);
			BlockTable->add(BlockTableRecord);
		}
		bool LayoutBlock = BlockTableRecord->isLayout();

		for (EoUInt16 PrimitiveIndex = 0; PrimitiveIndex < NumberOfPrimitives; PrimitiveIndex++) {
			EoDbPrimitive* Primitive = ReadPrimitive();
			Block->AddTail(Primitive);
			if (!LayoutBlock) {
				Primitive->AssociateWith(BlockTableRecord);
			}
		}
	}
	if (ReadUInt16() != kEndOfSection) {
		throw L"Exception ReadBlocksSection: Expecting sentinel EoDb::kEndOfSection.";
	}
}
void EoDbPegFile::ReadGroupsSection(AeSysDoc* document) {
	if (ReadUInt16() != kGroupsSection) {
		throw L"Exception ReadGroupsSection: Expecting sentinel EoDb::kGroupsSection.";
	}
	OdDbBlockTableRecordPtr ModelSpaceBlock = m_Database->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
	OdDbObjectId CurrentLayerObjectId = m_Database->getCLAYER();
	OdDbLayerTablePtr Layers = document->LayerTable(OdDb::kForRead);

	EoUInt16 NumberOfLayers = ReadUInt16();
	
	for (EoUInt16 LayerIndex = 0; LayerIndex < NumberOfLayers; LayerIndex++) {
		EoDbLayer* Layer = document->GetLayerAt(LayerIndex);
		
		if (Layer == 0)
			return;

		OdString LayerName = Layer->Name();
		OdDbObjectId LayerObjectId = Layers->getAt(LayerName);
		m_Database->setCLAYER(LayerObjectId);

		EoUInt16 NumberOfGroups = ReadUInt16();

		if (Layer->IsInternal()) {
			for (EoUInt16 GroupIndex = 0; GroupIndex < NumberOfGroups; GroupIndex++) {
				size_t NumberOfPrimitives = ReadUInt16();
				
				EoDbGroup* Group = new EoDbGroup;
				EoDbPrimitive* Primitive;
				
				for (size_t PrimitiveIndex = 0; PrimitiveIndex < NumberOfPrimitives; PrimitiveIndex++) {
					Primitive = ReadPrimitive();
					Primitive->AssociateWith(ModelSpaceBlock);
					Group->AddTail(Primitive);
				}
				Layer->AddTail(Group);
			}
		}
		else {
			OdString PathName = GetFilePath();
			PathName.replace(GetFileName(), Layer->Name());
			document->TracingLoadLayer(PathName, Layer);
		}
	}
	m_Database->setCLAYER(CurrentLayerObjectId);

	if (ReadUInt16() != kEndOfSection) {
		throw L"Exception ReadGroupsSection: Expecting sentinel EoDb::kEndOfSection.";
	}
}
void EoDbPegFile::Unload(AeSysDoc* document) {
	CFile::SetLength(0);
	CFile::SeekToBegin();

	WriteHeaderSection(document);
	WriteTablesSection(document);
	WriteBlocksSection(document);
	WriteEntitiesSection(document);
	WriteString(OdString(L"EOF"));

	CFile::Flush();
}
void EoDbPegFile::WriteHeaderSection(AeSysDoc*) {
	WriteUInt16(kHeaderSection);

	// header variable items go here

	WriteUInt16(kEndOfSection);
}
void EoDbPegFile::WriteTablesSection(AeSysDoc* document) {
	WriteUInt16(kTablesSection);

	WriteVPortTable(document);
	WriteLinetypeTable(document);
	WriteLayerTable(document);
	WriteUInt16(kEndOfSection);
}
void EoDbPegFile::WriteVPortTable(AeSysDoc*) {
	WriteUInt16(kViewPortTable);
	WriteUInt16(0);
	WriteUInt16(kEndOfTable);
}
void EoDbPegFile::WriteLinetypeTable(AeSysDoc* document) {
	WriteUInt16(kLinetypeTable);
	OdDbLinetypeTablePtr Linetypes = m_Database->getLinetypeTableId().safeOpenObject(OdDb::kForRead);

	OdDbSymbolTableIteratorPtr Iterator = Linetypes->newIterator();
	EoUInt16 NumberOfLinetypes = 0;
	for (Iterator->start(); !Iterator->done(); Iterator->step()) {
		NumberOfLinetypes++;
	}
	WriteUInt16(NumberOfLinetypes);

	Iterator = Linetypes->newIterator();
	for (Iterator->start(); !Iterator->done(); Iterator->step()) {
		OdDbLinetypeTableRecordPtr Linetype = Iterator->getRecordId().safeOpenObject(OdDb::kForRead);
		WriteString(Linetype->getName());
		WriteUInt16(0);
		WriteString(Linetype->comments());

		EoUInt16 DefinitionLength = static_cast<EoUInt16>(Linetype->numDashes());
		WriteUInt16(DefinitionLength);

		double PatternLength = Linetype->patternLength();
		WriteDouble(PatternLength);

		for (EoUInt16 DashIndex = 0; DashIndex < DefinitionLength; DashIndex++) {
			WriteDouble(Linetype->dashLengthAt(DashIndex));
		}
	}	
	WriteUInt16(kEndOfTable);
}
void EoDbPegFile::WriteLayerTable(AeSysDoc* document) {
	int NumberOfLayers = document->GetLayerTableSize();

	WriteUInt16(kLayerTable);

	ULONGLONG SavedFilePosition = CFile::GetPosition();
	WriteUInt16(EoUInt16(NumberOfLayers));

	for (int LayerIndex = 0; LayerIndex < document->GetLayerTableSize(); LayerIndex++) {
		EoDbLayer* Layer = document->GetLayerAt(LayerIndex);
		if (Layer->IsResident()) {
			WriteString(Layer->Name());
			WriteUInt16(Layer->StateFlags() & 0x003c); // used to be separate set of state flags for tracings (only used bits 3-6)
			WriteUInt16(Layer->StateFlags());
			WriteInt16(Layer->ColorIndex());
			WriteString(Layer->LinetypeName());
		}
		else
			NumberOfLayers--;
	}
	WriteUInt16(kEndOfTable);

	if (NumberOfLayers != document->GetLayerTableSize()) {
		ULONGLONG CurrentFilePosition = CFile::GetPosition();
		CFile::Seek(SavedFilePosition, CFile::begin);
		WriteUInt16(EoUInt16(NumberOfLayers));
		CFile::Seek(CurrentFilePosition, CFile::begin);
	}
}
void EoDbPegFile::WriteBlocksSection(AeSysDoc* document) {
	WriteUInt16(kBlocksSection);

	EoUInt16 NumberOfBlocks = document->BlockTableSize();
	WriteUInt16(NumberOfBlocks);

	CString Name;
	EoDbBlock* Block;

	POSITION Position = document->GetFirstBlockPosition();
	while (Position != 0) {
		document->GetNextBlock(Position, Name, Block);

		ULONGLONG SavedFilePosition = CFile::GetPosition();
		WriteUInt16(0);
		EoUInt16 NumberOfPrimitives = 0;

		WriteString(Name);
		WriteUInt16(Block->GetBlkTypFlgs());
		WritePoint3d(Block->BasePoint());

		POSITION PrimitivePosition = Block->GetHeadPosition();
		while (PrimitivePosition != 0) {
			EoDbPrimitive* Primitive = Block->GetNext(PrimitivePosition);
			if (Primitive->Write(*this))
				NumberOfPrimitives++;
		}
		ULONGLONG CurrentFilePosition = CFile::GetPosition();
		CFile::Seek(SavedFilePosition, CFile::begin);
		WriteUInt16(NumberOfPrimitives);
		CFile::Seek(CurrentFilePosition, CFile::begin);
	}

	WriteUInt16(kEndOfSection);
}
void EoDbPegFile::WriteEntitiesSection(AeSysDoc* document) {
	WriteUInt16(kGroupsSection);

	int NumberOfLayers = document->GetLayerTableSize();
	WriteUInt16(EoUInt16(NumberOfLayers));

	for (int LayerIndex = 0; LayerIndex < NumberOfLayers; LayerIndex++) {
		EoDbLayer* Layer = document->GetLayerAt(LayerIndex);
		if (Layer->IsInternal()) {
			WriteUInt16(EoUInt16(Layer->GetCount()));

			POSITION Position = Layer->GetHeadPosition();
			while (Position != 0) {
				EoDbGroup* Group = Layer->GetNext(Position);
				Group->Write(*this);
			}
		}
		else
			WriteUInt16(0);
	}
	WriteUInt16(kEndOfSection);
}
