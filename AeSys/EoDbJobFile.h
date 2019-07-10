#pragma once
class EoDbJobFile {
	int m_Version;
	unsigned char* m_PrimBuf;
public:
	EoDbJobFile();

	~EoDbJobFile();

	/// <summary>Reads document data from a memory file and adds all groups to the trap. This is a data stream retrieved from the clipboard.</summary>
	void ReadMemFile(OdDbBlockTableRecordPtr blockTableRecord, CFile& file);

	void ReadHeader(CFile& file);

	void ReadLayer(OdDbBlockTableRecordPtr blockTableRecord, CFile& file, EoDbLayer* layer);

	bool GetNextVisibleGroup(OdDbBlockTableRecordPtr blockTableRecord, CFile& file, EoDbGroup*& group);

	bool GetNextPrimitive(OdDbBlockTableRecordPtr blockTableRecord, CFile& file, EoDbPrimitive*& primitive);

	bool ReadNextPrimitive(CFile& file, unsigned char* buffer, short& primitiveType) const;

	int Version() noexcept;

	void WriteHeader(CFile& file) const;

	void WriteLayer(CFile& file, EoDbLayer* layer);

	void WriteGroup(CFile& file, EoDbGroup* group) const;

	void ConstructPrimitive(const OdDbBlockTableRecordPtr& blockTableRecord, EoDbPrimitive*& primitive, short primitiveType);

	void ConstructPrimitiveFromVersion1(const OdDbBlockTableRecordPtr& blockTableRecord, EoDbPrimitive*& primitive) const;

	static bool IsValidPrimitive(short primitiveType) noexcept;

	static bool IsValidVersion1Primitive(short primitiveType) noexcept;

	static void ConvertFormattingCharacters(OdString& textString) noexcept;
};
