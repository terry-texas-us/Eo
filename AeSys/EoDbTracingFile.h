#pragma once

class EoDbTracingFile : public EoDbFile {
public:
	EoDbTracingFile(const OdString& fileName, UINT openFlags);
	virtual ~EoDbTracingFile();

	void ReadHeader();
	bool ReadLayer(EoDbLayer* layer);
	EoDbGroup* ReadGroup();
	
	void WriteHeader();
	void WriteLayer(EoDbLayer* layer);
};
