#include "stdafx.h"

#include "EoDbGroup.h"
#include "EoDbFile.h"
#include "EoDbTracingFile.h"

EoDbTracingFile::EoDbTracingFile(OdDbDatabasePtr database) {
}

EoDbTracingFile::EoDbTracingFile(const OdString& file, unsigned openFlags)
	: EoDbFile(file, openFlags) {
}

void EoDbTracingFile::ReadHeader() {

	if (ReadUInt16() != kHeaderSection) { throw L"Exception EoDbTracingFile: Expecting sentinel kHeaderSection."; }

	// 	with addition of info here will loop key-value pairs till kEndOfSection sentinel

	if (ReadUInt16() != kEndOfSection) { throw L"Exception EoDbTracingFile: Expecting sentinel kEndOfSection."; }
}

bool EoDbTracingFile::ReadLayer(OdDbBlockTableRecordPtr blockTableRecord, EoDbLayer * layer) {
	
	if (ReadUInt16() != kGroupsSection) { throw L"Exception EoDbTracingFile: Expecting sentinel kGroupsSection."; }

	const auto NumberOfGroups {ReadUInt16()};

	for (unsigned GroupIndex = 0; GroupIndex < NumberOfGroups; GroupIndex++) {
		auto Group {ReadGroup(blockTableRecord)};
		layer->AddTail(Group);
	}
	if (ReadUInt16() != kEndOfSection) { throw L"Exception EoDbTracingFile: Expecting sentinel kEndOfSection."; }

	return true;
}

EoDbGroup* EoDbTracingFile::ReadGroup(OdDbBlockTableRecordPtr blockTableRecord) {
	const auto NumberOfPrimitives {ReadUInt16()};

	auto Group {new EoDbGroup};

	for (unsigned PrimitiveIndex = 0; PrimitiveIndex < NumberOfPrimitives; PrimitiveIndex++) {
		auto Primitive {ReadPrimitive(blockTableRecord)};
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

	WriteUInt16(static_cast<unsigned short>(layer->GetCount()));

	auto Position {layer->GetHeadPosition()};
	while (Position != nullptr) {
		auto Group {layer->GetNext(Position)};
		Group->Write(*this);
	}
	WriteUInt16(kEndOfSection);
}
