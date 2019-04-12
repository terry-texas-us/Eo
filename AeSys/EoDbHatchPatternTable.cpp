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
int EoDbHatchPatternTable::sm_HatchPatternOffsets[64];
double EoDbHatchPatternTable::sm_HatchPatternTable[1536];

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

    // <tas="failure to open and then continue leaves"</tas>
    if (!fl.Open(fileName, CFile::modeRead | CFile::typeText, &e))
        return;

    int iHatId = 0;
    int NumberOfPatternLines = 0;
    int TableOffset = 0;

    wchar_t	szLn[128];
    while (fl.ReadString(szLn, sizeof(szLn) / sizeof(wchar_t) - 1)) {
        if (szLn[0] == '*') { // New Hatch index
            if (iHatId != 0) {
                sm_HatchPatternTable[sm_HatchPatternOffsets[iHatId]] = double(NumberOfPatternLines);
            }
            sm_HatchPatternOffsets[++iHatId] = TableOffset++;
            NumberOfPatternLines = 0;

            const wchar_t Delimiters[] = L",\n";
            LPWSTR NextToken = nullptr;
            const LPWSTR Token = wcstok_s(&szLn[1], Delimiters, &NextToken);
            CString PatternName(Token);
            PatternName.TrimRight();
        } else if (szLn[0] != ';' || szLn[1] != ';') {
            int iNmbStrsId = TableOffset;
            TableOffset += 1;
            int iNmbEnts = 0;
            const wchar_t Delimiters[] = L",\0";
            LPWSTR NextToken = nullptr;
            LPWSTR Token = wcstok_s(szLn, Delimiters, &NextToken);
            while (Token != 0) {
                sm_HatchPatternTable[TableOffset++] = _wtof(Token);
                iNmbEnts++;
                Token = wcstok_s(nullptr, Delimiters, &NextToken);
            }
            sm_HatchPatternTable[iNmbStrsId++] = double(iNmbEnts) - 5.;
            NumberOfPatternLines++;
        }
    }
    OdHatchPatternManager* Manager = theApp.patternManager();

    for (int PatternIndex = 1; PatternIndex <= 2; PatternIndex++) {
        OdHatchPattern HatchPattern;
        TableOffset = sm_HatchPatternOffsets[PatternIndex];
        const int NumberOfLinePatterns = int(sm_HatchPatternTable[TableOffset++]);
        OdHatchPatternLine HatchPatternLine;
        for (int PatternLineIndex = 0; PatternLineIndex < NumberOfLinePatterns; PatternLineIndex++) {
            const int NumberOfDashesInPattern = int(sm_HatchPatternTable[TableOffset++]);
            HatchPatternLine.m_dLineAngle = sm_HatchPatternTable[TableOffset++];
            HatchPatternLine.m_basePoint.x = sm_HatchPatternTable[TableOffset++];
            HatchPatternLine.m_basePoint.y = sm_HatchPatternTable[TableOffset++];
            HatchPatternLine.m_patternOffset.x = sm_HatchPatternTable[TableOffset++] / 12.;
            HatchPatternLine.m_patternOffset.y = sm_HatchPatternTable[TableOffset++] / 12.;
            HatchPatternLine.m_dashes.clear();
            for (int DashIndex = 0; DashIndex < NumberOfDashesInPattern; DashIndex++) {
                HatchPatternLine.m_dashes.append(sm_HatchPatternTable[TableOffset++]);
            }
            HatchPattern.append(HatchPatternLine);
        }
        Manager->appendPattern(OdDbHatch::kPreDefined, EoDbHatchPatternTable::LegacyHatchPatternName(PatternIndex), HatchPattern);
    }
}

void EoDbHatchPatternTable::RetrieveHatchPattern(const OdString& hatchPatternName, OdHatchPattern& hatchPattern) {
    const auto HatchPatternIndex {EoDbHatchPatternTable::LegacyHatchPatternIndex(hatchPatternName)};
    if (HatchPatternIndex == 0) {
        // <tas="Override the dwg hatch pattern manager to load an external definition"/>
        OdHatchPatternManager* Manager = theApp.patternManager();
        ATLTRACE2(atlTraceGeneral, 0, L"Hatch pattern <%s> loaded from dwg pattern manager\n", (LPCWSTR) hatchPatternName);
    } else {
        auto TableOffset {sm_HatchPatternOffsets[HatchPatternIndex]};
        const int NumberOfPatterns = int(sm_HatchPatternTable[TableOffset++]);

        OdHatchPatternLine HatchPatternLine;
        for (int PatternIndex = 0; PatternIndex < NumberOfPatterns; PatternIndex++) {
            const int NumberOfDashesInPattern = int(sm_HatchPatternTable[TableOffset++]);

            HatchPatternLine.m_dLineAngle = sm_HatchPatternTable[TableOffset++];
            HatchPatternLine.m_basePoint.x = sm_HatchPatternTable[TableOffset++];
            HatchPatternLine.m_basePoint.y = sm_HatchPatternTable[TableOffset++];
            HatchPatternLine.m_patternOffset.x = sm_HatchPatternTable[TableOffset++];
            HatchPatternLine.m_patternOffset.y = sm_HatchPatternTable[TableOffset++];
            HatchPatternLine.m_dashes.clear();

            for (int DashIndex = 0; DashIndex < NumberOfDashesInPattern; DashIndex++) {
                HatchPatternLine.m_dashes.append(sm_HatchPatternTable[TableOffset++]);
            }
            hatchPattern.append(HatchPatternLine);
        }
    }
}
