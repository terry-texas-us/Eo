#pragma once

class EoDbLinetypeTable {
private:
	static const wchar_t* LegacyLinetypes[];
	static const OdUInt16 NumberOfLegacyLinetypes = 42;

public:
	static OdUInt16 LegacyLinetypeIndex(const OdString& name);
	static OdString LegacyLinetypeName(const int index);
public:
	void LoadLinetypesFromTxtFile(OdDbDatabasePtr database, const CString& fileName);
};