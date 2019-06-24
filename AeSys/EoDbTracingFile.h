#pragma once
#include "DbBlockTableRecord.h"
#include "EoDbLayer.h"

class EoDbTracingFile final : public EoDbFile {
public:
	EoDbTracingFile(OdDbDatabasePtr database);
	EoDbTracingFile(const OdString& fileName, unsigned openFlags);
	virtual ~EoDbTracingFile() = default;
	void ReadHeader();
	bool ReadLayer(OdDbBlockTableRecordPtr blockTableRecord, EoDbLayer* layer);
	EoDbGroup* ReadGroup(OdDbBlockTableRecordPtr blockTableRecord);
	void WriteHeader();
	void WriteLayer(EoDbLayer* layer);
};
