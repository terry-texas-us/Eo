#pragma once

class EoDbHatchPatternTable {

private:
    //    static OdStringArray sm_HatchNames;
    static const wchar_t* LegacyHatchPatterns[];
    static const OdUInt16 ms_NumberOfLegacyHatchPatterns {40};

    static int sm_HatchPatternOffsets[];
    static double sm_HatchPatternTable[];

public:
    static OdUInt16 LegacyHatchPatternIndex(const OdString& name);
    static OdString LegacyHatchPatternName(const int index);

    static OdUInt16 NumberOfLegacyHatchPatterns() noexcept { return ms_NumberOfLegacyHatchPatterns; }

    static void LoadHatchesFromFile(const CString& fileName);
    static void RetrieveHatchPattern(const OdString& hatchPatternName, OdHatchPattern& hatchPattern);
};

