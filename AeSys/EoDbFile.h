#pragma once
#include "EoDbPrimitive.h"

class EoDbFile : public CFile {
public:
	enum Sentinels {
		kHeaderSection = 0x0101, kTablesSection = 0x0102, kBlocksSection = 0x0103, kGroupsSection = 0x0104, kEndOfSection = 0x01ff, kViewPortTable = 0x0201, kLinetypeTable = 0x0202, kLayerTable = 0x0203, kEndOfTable = 0x02ff
	};

	OdDbDatabasePtr m_Database;

	EoDbFile() = default;

	EoDbFile(OdDbDatabasePtr database);

	EoDbFile(const OdString& fileName, unsigned openFlags);

	void ConstructBlockReferencePrimitiveFromInsertPrimitive(EoDbPrimitive*& primitive) noexcept;

	void ConstructPointPrimitiveFromTagPrimitive(EoDbPrimitive*& primitive);

	void ConstructPolylinePrimitiveFromCSplinePrimitive(EoDbPrimitive*& primitive);

	double ReadDouble();

	short ReadInt16();

	OdGePoint2d ReadPoint2d();

	OdGePoint3d ReadPoint3d();

	EoDbPrimitive* ReadPrimitive(OdDbBlockTableRecordPtr blockTableRecord);

	void ReadString(CString& string);

	void ReadString(OdString& string);

	unsigned short ReadUInt16();

	OdGeVector3d ReadVector3d();

	void WriteDouble(double number);

	void WriteInt16(short number);

	void WritePoint2d(const OdGePoint2d& point);

	void WritePoint3d(const OdGePoint3d& point);

	void WriteString(const CString& string);

	void WriteString(const OdString& string);

	void WriteUInt16(unsigned short number);

	void WriteVector3d(const OdGeVector3d& vector);
};
