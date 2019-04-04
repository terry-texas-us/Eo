#pragma once

class EoDbFile : public CFile {

public:
	enum Sentinals {
		kHeaderSection = 0x0101,
		kTablesSection = 0x0102,
		kBlocksSection = 0x0103,
		kGroupsSection = 0x0104,
		kEndOfSection = 0x01ff,

		kViewPortTable = 0x0201,
		kLinetypeTable = 0x0202,
		kLayerTable = 0x0203,
		kEndOfTable = 0x02ff
	};
    OdDbDatabasePtr m_Database;
public:
    EoDbFile();
    EoDbFile(OdDbDatabasePtr database);
    EoDbFile(const OdString& fileName, UINT openFlags);
	virtual ~EoDbFile();

	void ConstructBlockReferencePrimitiveFromInsertPrimitive(EoDbPrimitive*& primitive) noexcept;
	void ConstructPointPrimitiveFromTagPrimitive(EoDbPrimitive *&primitive);
	void ConstructPolylinePrimitiveFromCSplinePrimitive(EoDbPrimitive*& primitive);

	double ReadDouble();
	OdInt16 ReadInt16();
	OdGePoint3d ReadPoint3d();
	EoDbPrimitive* ReadPrimitive(OdDbBlockTableRecordPtr blockTable);
	void ReadString(CString& string);
	void ReadString(OdString& string);
	OdUInt16 ReadUInt16();
	OdGeVector3d ReadVector3d();

	void WriteDouble(double number);
	void WriteInt16(OdInt16 number);
	void WritePoint3d(const OdGePoint3d& point);
	void WriteString(const CString& string);
	void WriteString(const OdString& string);
	void WriteUInt16(OdUInt16 number);
	void WriteVector3d(const OdGeVector3d& vector);
};
