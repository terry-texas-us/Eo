#include "stdafx.h"

#include "EoDbGroup.h"
#include "EoDbFile.h"
#include "EoDbTracingFile.h"

EoDbTracingFile::EoDbTracingFile(OdDbDatabasePtr database) {
}

EoDbTracingFile::EoDbTracingFile(const OdString& file, UINT openFlags)
	: EoDbFile(file, openFlags) {
}

EoDbTracingFile::~EoDbTracingFile() {
}

void EoDbTracingFile::ReadHeader() {
	if (ReadUInt16() != kHeaderSection)
		throw L"Exception EoDbTracingFile: Expecting sentinel kHeaderSection.";

	// 	with addition of info here will loop key-value pairs till kEndOfSection sentinel

	if (ReadUInt16() != kEndOfSection)
		throw L"Exception EoDbTracingFile: Expecting sentinel kEndOfSection.";
}

bool EoDbTracingFile::ReadLayer(OdDbBlockTableRecordPtr blockTableRecord, EoDbLayer * layer) {
	if (ReadUInt16() != kGroupsSection)
		throw L"Exception EoDbTracingFile: Expecting sentinel kGroupsSection.";

	const auto NumberOfGroups = ReadUInt16();

	for (unsigned GroupIndex = 0; GroupIndex < NumberOfGroups; GroupIndex++) {
		EoDbGroup* Group = ReadGroup(blockTableRecord);
		layer->AddTail(Group);
	}
	if (ReadUInt16() != kEndOfSection) { throw L"Exception EoDbTracingFile: Expecting sentinel kEndOfSection."; }

	return true;
}

EoDbGroup* EoDbTracingFile::ReadGroup(OdDbBlockTableRecordPtr blockTableRecord) {
	const auto NumberOfPrimitives {ReadUInt16()};

	EoDbGroup* Group {new EoDbGroup};

	for (unsigned PrimitiveIndex = 0; PrimitiveIndex < NumberOfPrimitives; PrimitiveIndex++) {
		EoDbPrimitive* Primitive = ReadPrimitive(blockTableRecord);
		Group->AddTail(Primitive);
	}
	return Group;
}

void EoDbTracingFile::WriteHeader() {
	WriteUInt16(kHeaderSection);

	WriteUInt16(kEndOfSection);
}
void EoDbTracingFile::WriteLayer(EoDbLayer * layer) {
	WriteUInt16(kGroupsSection);

	WriteUInt16(OdUInt16(layer->GetCount()));

	POSITION Position = layer->GetHeadPosition();
	while (Position != 0) {
		EoDbGroup* Group = layer->GetNext(Position);
		Group->Write(*this);
	}
	WriteUInt16(kEndOfSection);
}

