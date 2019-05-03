#pragma once

class EoDbJobFile {
private:
	int m_Version;
    OdUInt8* m_PrimBuf;

public:
	EoDbJobFile();
	virtual ~EoDbJobFile();

	/// <summary>Reads document data from a memory file and adds all groups to the trap. This is a data stream retrieved from the clipboard.</summary>
	void ReadMemFile(OdDbBlockTableRecordPtr blockTableRecord, CFile& file);
	void ReadHeader(CFile& file);
	void ReadLayer(OdDbBlockTableRecordPtr blockTableRecord, CFile& file, EoDbLayer* layer);

	bool GetNextVisibleGroup(OdDbBlockTableRecordPtr blockTableRecord, CFile& file, EoDbGroup*& group);
	bool GetNextPrimitive(OdDbBlockTableRecordPtr blockTableRecord, CFile& file, EoDbPrimitive*& primitve);
	bool ReadNextPrimitive(CFile &file, OdUInt8 *buffer, OdInt16& primitiveType);

	int Version() noexcept;
	void WriteHeader(CFile& file);
	void WriteLayer(CFile& file, EoDbLayer* layer);
	void WriteGroup(CFile& file, EoDbGroup* group);
	void ConstructPrimitive(OdDbBlockTableRecordPtr blockTableRecord, EoDbPrimitive *&primitive, OdInt16 PrimitiveType);
	void ConstructPrimitiveFromVersion1(OdDbBlockTableRecordPtr blockTableRecord, EoDbPrimitive *&primitive);

	static bool IsValidPrimitive(OdInt16 primitiveType) noexcept;
	static bool IsValidVersion1Primitive(OdInt16 primitiveType) noexcept;

	static void ConvertFormattingCharacters(OdString& textString) noexcept;
};
