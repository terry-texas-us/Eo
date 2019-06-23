#include "stdafx.h"
#include "AeSys.h"
#include "HatchPatternManager.h"
#include "EoDbHatchPatternTable.h"
const wchar_t* EoDbHatchPatternTable::LegacyHatchPatterns[] = {
L"ExternalHatch",
L"LINE",
L"NET",
L"ANGLE",
L"ANSI31",
L"ANSI32",
L"ANSI33",
L"ANSI34",
L"ANSI35",
L"ANSI36",
L"ANSI37",
L"ANSI38",
L"BOX",
L"BRICK",
L"CLAY",
L"CORK",
L"CROSS",
L"DASH",
L"DOLMIT",
L"DOTS",
L"EARTH",
L"ESCHER",
L"FLEX",
L"GRASS",
L"GRATE",
L"HEX",
L"HONEY",
L"HOUND",
L"INSUL",
L"MUDST",
L"NET3",
L"PLAST",
L"PLASTI",
L"SACNCR",
L"SQUARE",
L"STARS",
L"SWAMP",
L"TRANS",
L"TRIANG",
L"ZIGZAG"
};

unsigned short EoDbHatchPatternTable::LegacyHatchPatternIndex(const OdString& name) {
	unsigned short Index = 0;
	while (Index < ms_NumberOfLegacyHatchPatterns && name.iCompare(LegacyHatchPatterns[Index]) != 0) {
		Index++;
	}
	Index = Index < ms_NumberOfLegacyHatchPatterns ? Index : 0u;
	return Index;
}

OdString EoDbHatchPatternTable::LegacyHatchPatternName(const unsigned index) {
	const auto Index {index < ms_NumberOfLegacyHatchPatterns ? index : 1};
	return LegacyHatchPatterns[Index];
}

/// <summary> Loads the hatch pattern definition table.</summary>
/// <remarks> No longer including the total length of all dash components in the table.</remarks>
void EoDbHatchPatternTable::LoadHatchesFromFile(const CString& fileName) {
	CFileException e;
	CStdioFile StreamFile;
	if (!StreamFile.Open(fileName, CFile::modeRead | CFile::typeText, &e)) { return; }
	// <tas="failure to open and then continue Pattern file, but still continues."</tas>
	auto HatchPatternManager {theApp.patternManager()};
	OdString PatternName;
	OdHatchPattern HatchPattern;
	wchar_t LineText[128];
	while (StreamFile.ReadString(LineText, sizeof LineText / sizeof(wchar_t) - 1)) {
		if (LineText[0] == '*') { // New Hatch pattern
			if (!PatternName.isEmpty()) {
				HatchPatternManager->appendPattern(OdDbHatch::kCustomDefined, PatternName, HatchPattern);
				HatchPattern.clear();
			}
			wchar_t* NextToken {nullptr};
			PatternName = wcstok_s(&LineText[1], L",\n", &NextToken);
			PatternName.trimRight();
		} else if (LineText[0] != ';' || LineText[1] != ';') {
			const wchar_t Delimiters[] = L",\0";
			wchar_t* NextToken {nullptr};
			OdHatchPatternLine HatchPatternLine;
			HatchPatternLine.m_dLineAngle = EoToRadian(_wtof(wcstok_s(LineText, Delimiters, &NextToken)));
			HatchPatternLine.m_basePoint.x = _wtof(wcstok_s(nullptr, Delimiters, &NextToken));
			HatchPatternLine.m_basePoint.y = _wtof(wcstok_s(nullptr, Delimiters, &NextToken));
			HatchPatternLine.m_patternOffset.x = _wtof(wcstok_s(nullptr, Delimiters, &NextToken));
			HatchPatternLine.m_patternOffset.y = _wtof(wcstok_s(nullptr, Delimiters, &NextToken));
			HatchPatternLine.m_dashes.clear();
			auto Token {wcstok_s(nullptr, Delimiters, &NextToken)};
			while (Token != nullptr) {
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
		TRACE1("Hatch pattern <%s> using Predefined dwg patterns.\n", static_cast<const wchar_t*>(hatchPatternName));
	}
	return PatternLoaded;
}
