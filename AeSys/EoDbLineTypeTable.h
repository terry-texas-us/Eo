#pragma once
class EoDbLinetypeTable {
	static const wchar_t* m_LegacyLinetypes[];
	static const unsigned short c_NumberOfLegacyLinetypes {42};

  public:
	static unsigned short LegacyLinetypeIndex(const OdString& name);
	static OdString LegacyLinetypeName(int index);
	static void LoadLinetypesFromTxtFile(OdDbDatabasePtr database, const CString& fileName);
};
