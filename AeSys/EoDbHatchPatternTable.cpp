#include "stdafx.h"

#include "AeSysApp.h"

#include "HatchPatternManager.h"
#include "EoDbHatchPatternTable.h"

const wchar_t* EoDbHatchPatternTable::LegacyHatchPatterns[] = {
    L"ExternalHatch", L"LINE", L"NET", L"ANGLE", L"ANSI31", L"ANSI32", L"ANSI33", L"ANSI34", L"ANSI35", L"ANSI36",
    L"ANSI37", L"ANSI38", L"BOX", L"BRICK", L"CLAY", L"CORK", L"CROSS", L"DASH", L"DOLMIT", L"DOTS",
    L"EARTH", L"ESCHER", L"FLEX", L"GRASS", L"GRATE", L"HEX", L"HONEY", L"HOUND", L"INSUL", L"MUDST",
    L"NET3", L"PLAST", L"PLASTI", L"SACNCR", L"SQUARE", L"STARS", L"SWAMP", L"TRANS", L"TRIANG", L"ZIGZAG"
};

OdUInt16 EoDbHatchPatternTable::LegacyHatchPatternIndex(const OdString& name) {
    OdUInt16 Index = 0;
    while (Index < ms_NumberOfLegacyHatchPatterns && name.iCompare(LegacyHatchPatterns[Index]) != 0) {
        Index++;
    }
    Index = (Index < ms_NumberOfLegacyHatchPatterns) ? Index : 0;
    return Index;
}

OdString EoDbHatchPatternTable::LegacyHatchPatternName(const int index) {
    const int Index = (index < ms_NumberOfLegacyHatchPatterns) ? index : 1;
    return LegacyHatchPatterns[Index];
}

/// <summary> Loads the hatch pattern definition table.</summary>
/// <remarks> No longer including the total length of all dash components in the table.</remarks>
void EoDbHatchPatternTable::LoadHatchesFromFile(const CString& fileName) {
    CFileException e;
    CStdioFile fl;

    if (!fl.Open(fileName, CFile::modeRead | CFile::typeText, &e)) {
        // <tas="failure to open and then continue Pattern file, but still continues."</tas>
        return;
    }

    OdHatchPatternManager* Manager = theApp.patternManager();

    OdString PatternName;
    OdHatchPattern HatchPattern;

    wchar_t	LineText[128];
    while (fl.ReadString(LineText, sizeof(LineText) / sizeof(wchar_t) - 1)) {
        if (LineText[0] == '*') { // New Hatch pattern
            if (!PatternName.isEmpty()) {
               Manager->appendPattern(OdDbHatch::kCustomDefined, PatternName, HatchPattern);
               HatchPattern.clear();
            }
            LPWSTR NextToken = nullptr;
            PatternName = wcstok_s(&LineText[1], L",\n", &NextToken);
            PatternName.trimRight();
        } else if (LineText[0] != ';' || LineText[1] != ';') {
            const wchar_t Delimiters[] = L",\0";
            LPWSTR NextToken = nullptr;

            OdHatchPatternLine HatchPatternLine;

            HatchPatternLine.m_dLineAngle = EoToRadian(_wtof(wcstok_s(LineText, Delimiters, &NextToken)));
            HatchPatternLine.m_basePoint.x = _wtof(wcstok_s(nullptr, Delimiters, &NextToken));
            HatchPatternLine.m_basePoint.y = _wtof(wcstok_s(nullptr, Delimiters, &NextToken));
            HatchPatternLine.m_patternOffset.x = _wtof(wcstok_s(nullptr, Delimiters, &NextToken));
            HatchPatternLine.m_patternOffset.y = _wtof(wcstok_s(nullptr, Delimiters, &NextToken));
            HatchPatternLine.m_dashes.clear();

            LPWSTR Token = wcstok_s(nullptr, Delimiters, &NextToken);
            while (Token != 0) {
                HatchPatternLine.m_dashes.append(_wtof(Token));
                Token = wcstok_s(nullptr, Delimiters, &NextToken);
            }
            HatchPattern.append(HatchPatternLine);
        }
    }
}

OdResult EoDbHatchPatternTable::RetrieveHatchPattern(const OdString& hatchPatternName, OdHatchPattern& hatchPattern) {
    OdHatchPatternManagerPtr Manager = theApp.patternManager();

    auto PatternLoaded {Manager->retrievePattern(OdDbHatch::kCustomDefined, hatchPatternName, OdDb::kEnglish, hatchPattern)};

    if (PatternLoaded != eOk) {
        // <tas="Trying to load from standard dwg hatch pattern set."/>
        PatternLoaded = Manager->retrievePattern(OdDbHatch::kPreDefined, hatchPatternName, OdDb::kEnglish, hatchPattern);
        ATLTRACE2(atlTraceGeneral, 0, L"Hatch pattern <%s> using Predefined dwg patterns.\n", (LPCWSTR) hatchPatternName);
    }
    return PatternLoaded;
}