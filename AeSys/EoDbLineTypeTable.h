#pragma once

class EoDbLinetypeTable {
private:
	static const wchar_t* LegacyLinetypes[];
	static const unsigned short NumberOfLegacyLinetypes = 42;

public:
	static unsigned short LegacyLinetypeIndex(const OdString& name);
	static OdString LegacyLinetypeName(int index);
public:
	void LoadLinetypesFromTxtFile(OdDbDatabasePtr database, const CString& fileName);
};
