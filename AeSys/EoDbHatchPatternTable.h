#pragma once
class EoDbHatchPatternTable {

	// static OdStringArray sm_HatchNames;
	static const wchar_t* LegacyHatchPatterns[];
	static const unsigned short ms_NumberOfLegacyHatchPatterns {40};
public:
	static unsigned short LegacyHatchPatternIndex(const OdString& name);

	static OdString LegacyHatchPatternName(unsigned index);

	static unsigned short NumberOfLegacyHatchPatterns() noexcept {
		return ms_NumberOfLegacyHatchPatterns;
	}

	static void LoadHatchesFromFile(const CString& fileName);

	static OdResult RetrieveHatchPattern(const OdString& hatchPatternName, OdHatchPattern& hatchPattern);
};
