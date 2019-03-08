#pragma once

class AeSysDoc;

class EoDbPegFile : public EoDbFile {
	OdDbDatabasePtr m_Database;
public:
	EoDbPegFile(OdDbDatabasePtr database);
	virtual ~EoDbPegFile();

	void Load(AeSysDoc* document);
	
	void ReadBlocksSection(AeSysDoc* document);
	void ReadGroupsSection(AeSysDoc* document);
	void ReadHeaderSection(AeSysDoc* document);
	void ReadLayerTable(AeSysDoc* document);
	void ReadLinetypesTable();
	void ReadTablesSection(AeSysDoc* document);
	void ReadViewportTable(AeSysDoc* document);
	void Unload(AeSysDoc* document);
	void WriteBlocksSection(AeSysDoc* document);
	void WriteEntitiesSection(AeSysDoc* document);
	void WriteHeaderSection(AeSysDoc* document);
	void WriteLayerTable(AeSysDoc* document);
	void WriteLinetypeTable(AeSysDoc* document);
	void WriteTablesSection(AeSysDoc* document);
	void WriteVPortTable(AeSysDoc* document);
};
