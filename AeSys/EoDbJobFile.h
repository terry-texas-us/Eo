#pragma once

class EoDbJobFile {
private:
	int m_Version;
    OdUInt8* m_PrimBuf;

public:
	EoDbJobFile();
	virtual ~EoDbJobFile();

	/// <summary>Reads document data from a memory file and adds all groups to the trap. This is a data stream retrieved from the clipboard.</summary>
	void ReadMemFile(CFile& file);
	void ReadHeader(CFile& file);
	void ReadLayer(CFile& file, EoDbLayer* layer);

	bool GetNextVisibleGroup(CFile& file, EoDbGroup*& group);
	bool GetNextPrimitive(CFile& file, EoDbPrimitive*& primitve);
	bool ReadNextPrimitive(CFile &file, OdUInt8 *buffer, OdInt16& primitiveType);

	int Version();
	void WriteHeader(CFile& file);
	void WriteLayer(CFile& file, EoDbLayer* layer);
	void WriteGroup(CFile& file, EoDbGroup* group);
	void ConstructPrimitive(EoDbPrimitive *&primitive, OdInt16 PrimitiveType);
	void ConstructPrimitiveFromVersion1(EoDbPrimitive *&primitive);

	static bool IsValidPrimitive(OdInt16 primitiveType);
	static bool IsValidVersion1Primitive(OdInt16 primitiveType);
};
