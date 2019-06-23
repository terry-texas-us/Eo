#include "stdafx.h"
#include "AeSys.h"
#include "Lex.h"
using namespace lex;

namespace lex
{
	int TokenTypes[MaximumNumberOfTokens];
	int iToks;
	int NumberOfValues;
	int LocationOfValue[MaximumNumberOfTokens];
	long Values[VALS_MAX];
}

void lex::BreakExpression(int& firstTokenLocation, int& numberOfTokens, int* typeOfTokens, int* locationOfTokens) {
	auto NumberOfOpenParentheses {0};
	auto PreviousTokenType {0};
	int OperatorStack[32] {0};
	auto TopOfOperatorStack {1};
	OperatorStack[TopOfOperatorStack] = TOK_IDENTIFIER;
	numberOfTokens = 0;
	auto CurrentTokenType {TokenType(firstTokenLocation)};
	while (CurrentTokenType != - 1) {
		switch (TokenTable[CurrentTokenType].Class) {
			case Constant:
				typeOfTokens[numberOfTokens] = CurrentTokenType;
				locationOfTokens[numberOfTokens++] = firstTokenLocation;
				break;
			case OpenParen:
				OperatorStack[++TopOfOperatorStack] = CurrentTokenType;	// Push to operator stack
				NumberOfOpenParentheses++;
				break;
			case CloseParen:
				if (NumberOfOpenParentheses == 0) { break; }
				while (OperatorStack[TopOfOperatorStack] != TOK_LPAREN) { // Move operator to token stack
					typeOfTokens[numberOfTokens++] = OperatorStack[TopOfOperatorStack--];
				}
				TopOfOperatorStack--;		// Discard open parentheses
				NumberOfOpenParentheses--; 	// One less open parentheses
				break;
			case BinaryArithOp: case Other:
				if (CurrentTokenType == TOK_BINARY_PLUS || CurrentTokenType == TOK_BINARY_MINUS) {
					const auto eClassPrv {TokenTable[PreviousTokenType].Class};
					if (eClassPrv != Constant && eClassPrv != Identifier && eClassPrv != CloseParen) {
						CurrentTokenType = CurrentTokenType == TOK_BINARY_PLUS ? TOK_UNARY_PLUS : TOK_UNARY_MINUS;
					}
				}
				// Pop higher priority operators from stack
				while (TokenTable[OperatorStack[TopOfOperatorStack]].InStackPriority >= TokenTable[CurrentTokenType].InComingPriority) {
					typeOfTokens[numberOfTokens++] = OperatorStack[TopOfOperatorStack--];
				}
				// Push new operator onto stack
				OperatorStack[++TopOfOperatorStack] = CurrentTokenType;
				break;

				// TODO .. classes of tokens which might be implemented
			case Identifier: case BinaryRelatOp: case BinaryLogicOp: case UnaryLogicOp: case AssignOp: default:
				break;
		}
		PreviousTokenType = CurrentTokenType;
		CurrentTokenType = TokenType(++firstTokenLocation);
	}
	if (NumberOfOpenParentheses > 0) { throw L"Unbalanced parentheses"; }
	while (TopOfOperatorStack > 1) typeOfTokens[numberOfTokens++] = OperatorStack[TopOfOperatorStack--];
	if (numberOfTokens == 0) { throw L"Syntax error"; }
}

void lex::ConvertValToString(wchar_t* acVal, LexColumnDefinition* columnDefinition, wchar_t* acPic, int* aiLen) noexcept {
	const auto DataType {columnDefinition->DataType};
	const int DataDimension {LOWORD(columnDefinition->DataDefinition)};
	if (DataType == TOK_STRING) {
		*aiLen = DataDimension;
		acPic[0] = '\'';
		memmove(&acPic[1], acVal, static_cast<unsigned>(*aiLen));
		acPic[++*aiLen] = '\'';
		acPic[++*aiLen] = '\0';
	} else {
		wchar_t cVal[32] {L"\0"};
		const auto lVal {reinterpret_cast<long*>(cVal)};
		const auto dVal {reinterpret_cast<double*>(cVal)};
		wchar_t* szpVal {nullptr};
		auto iLoc {0};
		auto ValueLength {0};
		auto ValueIndex {0};
		auto iLnLoc {0};
		int DataLength {HIWORD(columnDefinition->DataDefinition)};
		if (DataType != TOK_INTEGER) { DataLength = DataLength / 2; }
		if (DataDimension != DataLength) { // Matrix
			acPic[0] = '[';
			iLnLoc++;
		}
		for (auto i1 = 0; i1 < DataLength; i1++) {
			iLnLoc++;
			if (DataLength != 1 && i1 % DataDimension == 0) { acPic[iLnLoc++] = '['; }
			if (DataType == TOK_INTEGER) {
				memcpy(lVal, &acVal[ValueIndex], 4);
				ValueIndex += 4;
				_ltow(*lVal, &acPic[iLnLoc], 10);
				ValueLength = static_cast<int>(wcslen(&acPic[iLnLoc]));
				iLnLoc += ValueLength;
			} else {
				memcpy(dVal, &acVal[ValueIndex], 8);
				ValueIndex += 8;
				if (DataType == TOK_REAL) {
					iLoc = 1;
					// pCvtDoubToFltDecTxt(*dVal, 7, iLoc, cVal);
					wchar_t* NextToken {nullptr};
					szpVal = wcstok_s(cVal, L" ", &NextToken);
					wcscpy(&acPic[iLnLoc], szpVal);
					iLnLoc += static_cast<int>(wcslen(szpVal));
				} else if (DataType == TOK_LENGTH_OPERAND) {
					//TODO: Length to length string
					iLnLoc += ValueLength;
				} else if (DataType == TOK_AREA_OPERAND) {
					// pCvtWrldToUsrAreaStr(*dVal, &acPic[iLnLoc], iVLen);
					iLnLoc += ValueLength;
				}
			}
			if (DataLength != 1 && i1 % DataDimension == DataDimension - 1) { acPic[iLnLoc++] = ']'; }
		}
		if (DataDimension == DataLength) {
			*aiLen = iLnLoc - 1;
		} else {
			acPic[iLnLoc] = ']';
			*aiLen = iLnLoc;
		}
	}
}

void lex::ConvertValTyp(const int valueType, const int requiredType, long* definition, void* apVal) noexcept {
	if (valueType == requiredType) { return; }
	const auto pdVal {static_cast<double*>(apVal)};
	const auto piVal {static_cast<long*>(apVal)};
	if (valueType == TOK_STRING) {
		wchar_t szVal[256];
		wcscpy_s(szVal, 256, static_cast<wchar_t*>(apVal));
		if (requiredType == TOK_INTEGER) {
			piVal[0] = _wtoi(szVal);
			*definition = MAKELONG(1, 1);
		} else { // real
			pdVal[0] = _wtof(szVal);
			*definition = MAKELONG(1, 2);
		}
	} else if (valueType == TOK_INTEGER) {
		if (requiredType == TOK_STRING) {
			// <tas="integer to string not implemented"/>
		} else {
			pdVal[0] = static_cast<double>(piVal[0]);
			*definition = MAKELONG(1, 2);
		}
	} else { // real
		if (requiredType == TOK_STRING) {
		} else if (requiredType == TOK_INTEGER) {
		}
	}
}

void lex::ConvertStringToVal(const int valueType, const long definition, wchar_t* szVal, long* lDefReq, void* p) {
	if (LOWORD(definition) <= 0) { throw L"Empty string"; }
	wchar_t szTok[64];
	auto iNxt {0};
	const auto iTyp {Scan(szTok, szVal, iNxt)};
	if (valueType == TOK_INTEGER) { // Conversion to integer
		const auto pVal {static_cast<long*>(p)};
		if (iTyp == TOK_INTEGER) {
			*pVal = _wtol(szTok);
		} else if (iTyp == TOK_REAL) {
			*pVal = static_cast<long>(_wtof(szTok));
		} else {
			throw L"String format conversion error";
		}
		*lDefReq = MAKELONG(1, 1);
	} else {
		const auto pVal {static_cast<double*>(p)};
		if (iTyp == TOK_INTEGER) {
			*pVal = static_cast<double>(_wtoi(szTok));
		} else if (iTyp == TOK_REAL) {
			*pVal = _wtof(szTok);
		} else {
			throw L"String format conversion error";
		}
		*lDefReq = MAKELONG(1, 2);
	}
}

void lex::EvalTokenStream(int* aiTokId, long* definition, int* valueType, void* apOp) {
	wchar_t szTok[256] {L"\0"};
	auto iDim {0};
	auto iTyp {0};
	auto lDef1 = MAKELONG(1, 1);
	auto iDim1 {0};
	auto iLen1 {0};
	auto iTyp1 {TOK_INTEGER};
	long lDef2 {0};
	auto iDim2 {0};
	auto iLen2 {0};
	auto iTyp2 {0};
	int NumberOfTokens;
	int TypeOfTokens[32];
	int LocationOfTokens[32];
	BreakExpression(*aiTokId, NumberOfTokens, TypeOfTokens, LocationOfTokens);
	int iOpStkTyp[32] {0};
	long lOpStk[32][32] {0};
	long lOpStkDef[32] {0};
	const auto cOp1 {static_cast<wchar_t*>(apOp)};
	const auto dOp1 {static_cast<double*>(apOp)};
	const auto lOp1 {static_cast<long*>(apOp)};
	wchar_t cOp2[256] {L"\0"};
	const double* dOp2 {reinterpret_cast<double*>(cOp2)};
	const auto lOp2 {reinterpret_cast<long*>(cOp2)};
	auto OperandStackTop {0};
	auto TokenStackIndex {0}; // Start with first token
	while (TokenStackIndex < NumberOfTokens) {
		const auto iTokTyp {TypeOfTokens[TokenStackIndex]};
		const auto iTokLoc {LocationOfTokens[TokenStackIndex]};
		if (TokenTable[iTokTyp].Class == Identifier) { // symbol table stuff if desired
			throw L"Identifier token class not implemented";
		}
		if (TokenTable[iTokTyp].Class == Constant) {
			iTyp1 = iTokTyp;
			lDef1 = Values[LocationOfValue[iTokLoc]];
			memcpy(cOp1, &Values[LocationOfValue[iTokLoc] + 1], static_cast<unsigned>(HIWORD(lDef1) * 4));
		} else { // Token is an operator .. Pop an operand from operand stack
			if (OperandStackTop == 0) { throw L"Operand stack is empty"; }
			iTyp1 = iOpStkTyp[OperandStackTop];
			lDef1 = lOpStkDef[OperandStackTop];
			iLen1 = HIWORD(lDef1);
			memcpy(cOp1, &lOpStk[OperandStackTop--][0], static_cast<unsigned>(iLen1 * 4));
			if (TokenTable[iTokTyp].Class == Other) { // intrinsics and oddball unary minus/plus
				if (iTyp1 == TOK_STRING) {
					iDim1 = LOWORD(lDef1);
					wcscpy_s(szTok, 256, cOp1);
					if (iTokTyp == TOK_TOINTEGER) {
						iTyp1 = TOK_INTEGER;
						ConvertStringToVal(TOK_INTEGER, lDef1, szTok, &lDef1, cOp1);
					} else if (iTokTyp == TOK_REAL) {
						iTyp1 = TOK_REAL;
						ConvertStringToVal(TOK_REAL, lDef1, szTok, &lDef1, cOp1);
					} else if (iTokTyp == TOK_STRING) {
					} else {
						throw L"String operand conversions error: unknown";
					}
				} else if (iTyp1 == TOK_INTEGER) {
					UnaryOp(iTokTyp, &iTyp1, &lDef1, lOp1);
				} else {
					UnaryOp(iTokTyp, &iTyp1, &lDef1, dOp1);
				}
			} else if (TokenTable[iTokTyp].Class == BinaryArithOp) { // Binary arithmetic operator
				if (OperandStackTop == 0) { throw L"Binary Arithmetic: Only one operand."; }
				iTyp2 = iOpStkTyp[OperandStackTop]; // Pop second operand from operand stack
				lDef2 = lOpStkDef[OperandStackTop];
				iLen2 = HIWORD(lDef2);
				memcpy(cOp2, &lOpStk[OperandStackTop--][0], static_cast<unsigned>(iLen2 * 4));
				iTyp = EoMin(iTyp2, TOK_REAL);
				if (iTyp1 < iTyp) { // Convert first operand
					ConvertValTyp(iTyp1, iTyp, &lDef1, lOp1);
					iTyp1 = iTyp;
					iLen1 = HIWORD(lDef1);
				} else {
					iTyp = EoMin(iTyp1, TOK_REAL);
					if (iTyp2 < iTyp) { // Convert second operand
						ConvertValTyp(iTyp2, iTyp, &lDef2, lOp2);
						iTyp2 = iTyp;
						iLen2 = HIWORD(lDef2);
					}
				}
				if (iTokTyp == TOK_BINARY_PLUS) {
					if (iTyp1 == TOK_STRING) {
						iDim1 = LOWORD(lDef1);
						iDim2 = LOWORD(lDef2);
						iDim = iDim2 + iDim1;
						wcscpy(cOp1, _tcscat(cOp2, cOp1));
						iLen1 = 1 + (iDim - 1) / 4;
						lDef1 = MAKELONG(iDim, iLen1);
					} else {
						if (iTyp1 == TOK_INTEGER) {
							lOp1[0] += lOp2[0];
						} else {
							dOp1[0] += dOp2[0];
						}
					}
				} else if (iTokTyp == TOK_BINARY_MINUS) {

					if (iTyp1 == TOK_STRING) { throw L"Can not subtract strings"; }
					if (iTyp1 == TOK_INTEGER) {
						lOp1[0] = lOp2[0] - lOp1[0];
					} else {
						dOp1[0] = dOp2[0] - dOp1[0];
					}
				} else if (iTokTyp == TOK_MULTIPLY) {

					if (iTyp1 == TOK_STRING) { throw L"Can not mutiply strings"; }
					if (iTyp1 == TOK_INTEGER) {
						lOp1[0] *= lOp2[0];
					} else {
						if (iTyp1 == TOK_REAL) {
							iTyp1 = iTyp2;
						} else if (iTyp2 == TOK_REAL) {
						} else if (iTyp1 == TOK_LENGTH_OPERAND && iTyp2 == TOK_LENGTH_OPERAND) {
							iTyp1 = TOK_AREA_OPERAND;
						} else {
							throw L"Invalid mix of multiplicands";
						}
						dOp1[0] *= dOp2[0];
					}
				} else if (iTokTyp == TOK_DIVIDE) {

					if (iTyp1 == TOK_STRING) { throw L"Can not divide strings"; }
					if (iTyp1 == TOK_INTEGER) {
						if (lOp1[0] == 0) throw L"Attempting to divide by 0";
						lOp1[0] = lOp2[0] / lOp1[0];
					} else if (iTyp1 <= iTyp2) {
						if (dOp1[0] == 0.0) { throw L"Attempting to divide by 0."; }
						if (iTyp1 == iTyp2) {
							iTyp1 = TOK_REAL;
						} else if (iTyp1 == TOK_REAL) {
							iTyp1 = iTyp2;
						} else {
							iTyp1 = TOK_LENGTH_OPERAND;
						}
						dOp1[0] = dOp2[0] / dOp1[0];
					} else {
						throw L"Division type error";
					}
				} else if (iTokTyp == TOK_EXPONENTIATE) {
					if (iTyp1 == TOK_INTEGER) {
						if (lOp1[0] >= 0 && lOp1[0] > DBL_MAX_10_EXP || lOp1[0] < 0 && lOp1[0] < DBL_MIN_10_EXP) { throw L"Exponentiation error"; }
						lOp1[0] = static_cast<int>(pow(static_cast<double>(lOp2[0]), lOp1[0]));
					} else if (iTyp1 == TOK_REAL) {
						const auto iExp {static_cast<int>(dOp1[0])};
						if (iExp >= 0 && iExp > DBL_MAX_10_EXP || iExp < 0 && iExp < DBL_MIN_10_EXP) { throw L"Exponentiation error"; }
						dOp1[0] = pow(dOp2[0], dOp1[0]);
					}
				}
			} else if (TokenTable[iTokTyp].Class == BinaryRelatOp) {
				// if support for binary relational operators desired (== != > >= < <=)
				throw L"Binary relational operators not implemented";
			} else if (TokenTable[iTokTyp].Class == BinaryLogicOp) {
				// if support for binary logical operators desired (& |)
				throw L"Binary logical operators not implemented";
			} else if (TokenTable[iTokTyp].Class == UnaryLogicOp) {
				// if support for unary logical operator desired (!)
				throw L"Unary logical operator not implemented";
			}
		}
		OperandStackTop++; // Increment operand stack pointer
		iOpStkTyp[OperandStackTop] = iTyp1; // Push operand onto operand stack
		lOpStkDef[OperandStackTop] = lDef1;
		memcpy(&lOpStk[OperandStackTop][0], cOp1, static_cast<unsigned>(HIWORD(lDef1) * 4));
		TokenStackIndex++;
	}
	*valueType = iTyp1;
	*definition = lDef1;
}

void lex::Init() noexcept {
	iToks = 0;
	NumberOfValues = 0;
}

void lex::Parse(const wchar_t* szLine) {
	iToks = 0;
	NumberOfValues = 0;
	wchar_t szTok[256] {L"\0"};
	auto iBeg {0};
	const auto iLnLen {static_cast<int>(wcslen(szLine))};
	while (iBeg < iLnLen) {
		const auto iTyp {Scan(szTok, szLine, iBeg)};
		if (iTyp == -1) { return; }
		if (iToks == MaximumNumberOfTokens) { return; }
		TokenTypes[iToks] = iTyp;
		auto iLen {static_cast<int>(wcslen(szTok))};
		int iDim;
		double dVal;
		switch (iTyp) {
			case TOK_IDENTIFIER:
				iDim = static_cast<int>(wcslen(szTok));
				iLen = 1 + (iDim - 1) / 4;
				LocationOfValue[iToks] = NumberOfValues + 1;
				Values[NumberOfValues + 1] = iDim + iLen * 65536;
				memcpy(&Values[NumberOfValues + 2], szTok, static_cast<unsigned>(iDim));
				NumberOfValues = NumberOfValues + 1 + iLen;
				break;
			case TOK_STRING:
				ParseStringOperand(szTok);
				break;
			case TOK_INTEGER:
				LocationOfValue[iToks] = NumberOfValues;
				Values[NumberOfValues++] = MAKELONG(1, 1);
				Values[NumberOfValues++] = _wtoi(szTok);
				break;
			case TOK_REAL: case TOK_LENGTH_OPERAND:
				dVal = iTyp == TOK_REAL ? _wtof(szTok) : theApp.ParseLength(szTok);
				LocationOfValue[iToks] = NumberOfValues;
				Values[NumberOfValues++] = MAKELONG(1, 2);
				memcpy(&Values[NumberOfValues++], &dVal, sizeof(double));
				NumberOfValues++;
				break;
		}
		iToks++;
	}
}

void lex::ParseStringOperand(const wchar_t* pszTok) {
	if (wcslen(pszTok) < 3) {
		theApp.AddStringToMessageList(IDS_MSG_ZERO_LENGTH_STRING);
		return;
	}
	const auto pszValues {reinterpret_cast<wchar_t*>(& Values[NumberOfValues + 2])};
	auto iDim {0};
	auto iNxt {1};
	while (pszTok[iNxt] != '\0') {
		if (pszTok[iNxt] == '"' && pszTok[iNxt + 1] == '"') { iNxt++; }
		pszValues[iDim++] = pszTok[iNxt++];
	}
	pszValues[--iDim] = '\0';
	const auto iLen = 1 + (iDim - 1) / 4;
	LocationOfValue[iToks] = ++NumberOfValues;
	Values[NumberOfValues] = MAKELONG(iDim, iLen);
	NumberOfValues += iLen;
}

int lex::Scan(wchar_t* token, const wchar_t* line, int& linePosition) {
	auto iLen {0};
	while (line[linePosition] == ' ') { linePosition++; }
	const auto iBegLoc {linePosition};
	auto iTokLoc {linePosition};
	auto Result {-1};
	auto iS {1};
	auto bDone {false};
	while (!bDone) {
		const auto iAddr {iBase[iS] + line[linePosition]};
		if (iCheck[iAddr] == iS) {
			iS = iNext[iAddr];
			if (iTokVal[iS] != 0) {
				Result = iTokVal[iS];
				iTokLoc = linePosition;
			}
			linePosition++;
		} else if (iDefault[iS] != 0) {
			iS = iDefault[iS];
		} else {
			bDone = true;
		}
	}
	iLen = iTokLoc - iBegLoc + 1;
	wcsncpy(token, &line[iBegLoc], static_cast<unsigned>(iLen));
	token[iLen] = '\0';
	TRACE2("LinePointer = %d, TokenID = %d\n", linePosition, Result);
	if (Result == - 1) { linePosition = iBegLoc + 1; }
	return Result;
}

int lex::TokenType(const int aiTokId) noexcept {
	return aiTokId >= 0 && aiTokId < iToks ? TokenTypes[aiTokId] : - 1;
}

void lex::UnaryOp(const int aiTokTyp, int* valueType, long* definition, double* adOp) {
	LexColumnDefinition cd;
	wchar_t szTok[32];
	int i;
	int iDim {LOWORD(*definition)};
	int iLen {HIWORD(*definition)};
	switch (aiTokTyp) {
		case TOK_UNARY_MINUS:
			for (i = 0; i < iLen / 2; i++) {
				adOp[i] = -adOp[i];
			}
			break;
		case TOK_UNARY_PLUS:
			break;
		case TOK_ABS:
			adOp[0] = fabs(adOp[0]);
			break;
		case TOK_ACOS: {
			if (fabs(adOp[0]) > 1.0) { throw L"Math error: acos of a value greater than 1."; }
			adOp[0] = acos(EoToDegree(adOp[0]));
			break;
		}
		case TOK_ASIN: {
			if (fabs(adOp[0]) > 1.0) { throw L"Math error: asin of a value greater than 1."; }
			adOp[0] = asin(EoToDegree(adOp[0]));
			break;
		}
		case TOK_ATAN:
			adOp[0] = atan(EoToDegree(adOp[0]));
			break;
		case TOK_COS:
			adOp[0] = cos(EoToRadian(adOp[0]));
			break;
		case TOK_TOREAL:
			break;
		case TOK_EXP:
			adOp[0] = exp(adOp[0]);
			break;
		case TOK_TOINTEGER: // Conversion to integer
			ConvertValTyp(TOK_REAL, TOK_INTEGER, definition, static_cast<void*>(adOp));
			*valueType = TOK_INTEGER;
			break;
		case TOK_LN: {
			if (adOp[0] <= 0.0) { throw L"Math error: ln of a non-positive number"; }
			adOp[0] = log(adOp[0]);
			break;
		}
		case TOK_LOG: {
			if (adOp[0] <= 0.0) { throw L"Math error: log of a non-positive number"; }
			adOp[0] = log10(adOp[0]);
			break;
		}
		case TOK_SIN:
			adOp[0] = sin(EoToRadian(adOp[0]));
			break;
		case TOK_SQRT: {
			if (adOp[0] < 0.0) { throw L"Math error: sqrt of a negative number"; }
			adOp[0] = sqrt(adOp[0]);
			break;
		}
		case TOK_TAN:
			adOp[0] = tan(EoToRadian(adOp[0]));
			break;
		case TOK_TOSTRING:	// Conversion to string
			*valueType = TOK_STRING;
			cd.DataType = TOK_REAL;
			cd.DataDefinition = *definition;
			ConvertValToString(reinterpret_cast<wchar_t*>(adOp), &cd, szTok, &iDim);
			iLen = 1 + (iDim - 1) / 4;
			wcscpy(reinterpret_cast<wchar_t*>(adOp), szTok);
			*definition = MAKELONG(iDim, iLen);
			break;
		default:
			throw L"Unknown operation";
	}
}

void lex::UnaryOp(const int aiTokTyp, int* valueType, long* definition, long* alOp) {
	LexColumnDefinition cd;
	wchar_t szTok[32];
	int iDim {LOWORD(*definition)};
	int iLen {HIWORD(*definition)};
	switch (aiTokTyp) {
		case TOK_UNARY_MINUS:
			alOp[0] = - alOp[0];
			break;
		case TOK_UNARY_PLUS:
			break;
		case TOK_ABS:
			alOp[0] = labs(alOp[0]);
			break;
		case TOK_TOINTEGER:
			break;
		case TOK_TOREAL:
			ConvertValTyp(TOK_INTEGER, TOK_REAL, definition, static_cast<void*>(alOp));
			*valueType = TOK_REAL;
			break;
		case TOK_TOSTRING:
			*valueType = TOK_STRING;
			cd.DataType = TOK_INTEGER;
			cd.DataDefinition = *definition;
			ConvertValToString(reinterpret_cast<wchar_t*>(alOp), &cd, szTok, &iDim);
			iLen = 1 + (iDim - 1) / 4;
			wcscpy(reinterpret_cast<wchar_t*>(alOp), szTok);
			*definition = MAKELONG(iDim, iLen);
			break;
		default:
			throw L"Unknown operation";
	}
}

wchar_t* lex::ScanForChar(const wchar_t c, wchar_t* * ppStr) noexcept {
	const auto p {SkipWhiteSpace(*ppStr)};
	if (*p == c) {
		*ppStr = p + 1;
		return p;
	}
	return nullptr; // not found
}

wchar_t* lex::SkipWhiteSpace(wchar_t* pszString) noexcept {
	while (pszString && *pszString && isspace(*pszString)) {
		pszString++;
	}
	return pszString;
}

wchar_t* lex::ScanForString(wchar_t* * ppStr, wchar_t* pszTerm, wchar_t* * ppArgBuf) noexcept {
	auto pIn {SkipWhiteSpace(*ppStr)};
	const auto pStart {*ppArgBuf};
	auto pOut {pStart};
	const auto bInQuotes {*pIn == '"'};
	if (bInQuotes) { pIn++; }
	do {
		if (bInQuotes) {
			if (*pIn == '"' && *(pIn + 1) != '"') { // Skip over the quote
				pIn++;
				break;
			}
		} else if (isalnum(*pIn)) {
		} else { // allow some peg specials
			if (!(*pIn == '_' || *pIn == '$' || *pIn == '.' || *pIn == '-' || *pIn == ':' || *pIn == '\\')) { break; }
		}
		if (*pIn == '"' && *(pIn + 1) == '"') { // Skip the escaping first quote
			pIn++;
		}
		if (*pIn == '\\' && *(pIn + 1) == '\\') { // Skip the escaping backslash
			pIn++;
		}
		*pOut++ = *pIn++; // the char to the arg buffer
	} while (*pIn);
	*pOut++ = '\0'; // Set up the terminating char and update the scan pointer
	*pszTerm = *pIn;
	if (*pIn) {
		*ppStr = pIn + 1;
	} else {
		*ppStr = pIn;
	}
	*ppArgBuf = pOut; // Update the arg buffer to the next free bit
	return pStart;
}
