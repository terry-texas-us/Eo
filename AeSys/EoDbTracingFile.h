#pragma once

#include "EoDbLayer.h"

class EoDbTracingFile : public EoDbFile {
public:
	EoDbTracingFile(OdDbDatabasePtr database);
	EoDbTracingFile(const OdString& fileName, unsigned openFlags);
	virtual ~EoDbTracingFile();

	void ReadHeader();
	bool ReadLayer(OdDbBlockTableRecordPtr blockTableRecord, EoDbLayer* layer);
	EoDbGroup* ReadGroup(OdDbBlockTableRecordPtr blockTableRecord);

	void WriteHeader();
	void WriteLayer(EoDbLayer* layer);
};
