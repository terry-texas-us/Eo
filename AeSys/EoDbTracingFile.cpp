#include "stdafx.h"
#include "AeSysApp.h"

EoDbTracingFile::EoDbTracingFile(const OdString& fileName, UINT openFlags)
	: EoDbFile(fileName, openFlags) {
}
EoDbTracingFile::~EoDbTracingFile() {
}
void EoDbTracingFile::ReadHeader() {
	if (ReadUInt16() != kHeaderSection)
		throw L"Exception EoDbTracingFile: Expecting sentinel EoDb::kHeaderSection.";

	// 	with addition of info here will loop key-value pairs till EoDb::kEndOfSection sentinel

	if (ReadUInt16() != kEndOfSection)
		throw L"Exception EoDbTracingFile: Expecting sentinel EoDb::kEndOfSection.";
}
bool EoDbTracingFile::ReadLayer(EoDbLayer* layer) {
	if (ReadUInt16() != kGroupsSection)
		throw L"Exception EoDbTracingFile: Expecting sentinel EoDb::kGroupsSection.";

	const OdUInt16 NumberOfGroups = ReadUInt16();

	for (OdUInt16 n = 0; n < NumberOfGroups; n++) {
		EoDbGroup* Group = ReadGroup();
		layer->AddTail(Group);
	}
	if (ReadUInt16() != kEndOfSection)
		throw L"Exception EoDbTracingFile: Expecting sentinel EoDb::kEndOfSection.";

	return true;
}
EoDbGroup* EoDbTracingFile::ReadGroup() {
	const size_t NumberOfPrimitives = ReadUInt16();

	EoDbGroup* Group = new EoDbGroup;
	EoDbPrimitive* Primitive;

	for (size_t PrimitiveIndex = 0; PrimitiveIndex < NumberOfPrimitives; PrimitiveIndex++) {
		Primitive = ReadPrimitive();
		Group->AddTail(Primitive);
	}
	return Group;
}
void EoDbTracingFile::WriteHeader() {
	WriteUInt16(kHeaderSection);

	WriteUInt16(kEndOfSection);
}
void EoDbTracingFile::WriteLayer(EoDbLayer* layer) {
	WriteUInt16(kGroupsSection);

	WriteUInt16(OdUInt16(layer->GetCount()));

	POSITION Position = layer->GetHeadPosition();
	while (Position != 0) {
		EoDbGroup* Group = layer->GetNext(Position);
		Group->Write(*this);
	}
	WriteUInt16(kEndOfSection);
}

