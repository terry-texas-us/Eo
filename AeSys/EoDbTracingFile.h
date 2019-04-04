#pragma once

class EoDbTracingFile : public EoDbFile {
public:
    EoDbTracingFile(OdDbDatabasePtr database);
    EoDbTracingFile(const OdString& fileName, UINT openFlags);
    virtual ~EoDbTracingFile();

	void ReadHeader();
	bool ReadLayer(OdDbBlockTableRecordPtr blockTable, EoDbLayer* layer);
	EoDbGroup* ReadGroup(OdDbBlockTableRecordPtr blockTable);
	
	void WriteHeader();
	void WriteLayer(EoDbLayer* layer);
};
