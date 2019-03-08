#include "stdafx.h"
#include "AeSysApp.h"

#pragma warning(push)
#pragma warning(disable: 4996)

#include "Lex.h"
using namespace lex;

namespace lex {
	int iTokenType[TOKS_MAX];	// token type identifier
	int iToks;					// number of tokens in current token stream
	int iValsCount;				// number of values
	int iValLoc[TOKS_MAX];		// location of value
	long lValues[VALS_MAX];
}
void lex::BreakExpression(int& firstTokenLocation, int& numberOfTokens, int* typeOfTokens, int* locationOfTokens) {
	int NumberOfOpenParentheses = 0;
	int PreviousTokenType = 0;

	int OperatorStack[32];
	int TopOfOperatorStack = 1;

	OperatorStack[TopOfOperatorStack] = TOK_IDENTIFIER;

	numberOfTokens = 0;

	int CurrentTokenType = TokType(firstTokenLocation);
	while (CurrentTokenType != - 1) {
		switch (TokenTable[CurrentTokenType].eClass) {
		case Constant:
			typeOfTokens[numberOfTokens] = CurrentTokenType;
			locationOfTokens[numberOfTokens++] = firstTokenLocation;
			break;

		case OpenParen:
			OperatorStack[++TopOfOperatorStack] = CurrentTokenType;	// Push to operator stack
			NumberOfOpenParentheses++;
			break;

		case CloseParen:
			if (NumberOfOpenParentheses == 0)
				break;

			while (OperatorStack[TopOfOperatorStack] != TOK_LPAREN) { // Move operator to token stack
				typeOfTokens[numberOfTokens++] = OperatorStack[TopOfOperatorStack--];
			}
			TopOfOperatorStack--;		// Discard open parentheses
			NumberOfOpenParentheses--; 	// One less open parentheses
			break;

		case BinaryArithOp:
		case Other:
			if (CurrentTokenType == TOK_BINARY_PLUS || CurrentTokenType == TOK_BINARY_MINUS) {
				ETokClass eClassPrv = TokenTable[PreviousTokenType].eClass;
				if (eClassPrv != Constant && eClassPrv != Identifier && eClassPrv != CloseParen) {
					CurrentTokenType = (CurrentTokenType == TOK_BINARY_PLUS) ? TOK_UNARY_PLUS : TOK_UNARY_MINUS;
				}
			}
			// Pop higher priority operators from stack
			while (TokenTable[OperatorStack[TopOfOperatorStack]].iInStkPrio >= TokenTable[CurrentTokenType].iInComPrio) {
				typeOfTokens[numberOfTokens++] = OperatorStack[TopOfOperatorStack--];
			}
			// Push new operator onto stack
			OperatorStack[++TopOfOperatorStack] = CurrentTokenType;
			break;

			// TODO .. classes of tokens which might be implemented
		case Identifier:
		case BinaryRelatOp:
		case BinaryLogicOp:
		case UnaryLogicOp:
		case AssignOp:

		default:
			break;
		}
		PreviousTokenType = CurrentTokenType;
		CurrentTokenType = TokType(++firstTokenLocation);
	}
	if (NumberOfOpenParentheses > 0)
		throw L"Unbalanced parentheses";

	while (TopOfOperatorStack > 1) typeOfTokens[numberOfTokens++] = OperatorStack[TopOfOperatorStack--];

	if (numberOfTokens == 0)
		throw L"Syntax error";
}
void lex::ConvertValToString(LPTSTR acVal, CD* arCD, LPTSTR acPic, int* aiLen) {
	long lTyp = arCD->lTyp;
	int iDim = LOWORD(arCD->lDef);

	if (lTyp == TOK_STRING) {
		*aiLen = iDim;
		acPic[0] = '\'';
		memmove(&acPic[1], acVal, *aiLen);
		acPic[++*aiLen] = '\'';
		acPic[++*aiLen] = '\0';
	}
	else {
		WCHAR cVal[32];
		long* lVal = (long*) cVal;
		double* dVal = (double*) cVal;

		LPTSTR szpVal;
		int iLoc;

		int iVLen = 0;
		int iValId = 0;
		int iLnLoc = 0;
		int iLen = HIWORD(arCD->lDef);

		if (lTyp != TOK_INTEGER) iLen = iLen / 2;

		if (iDim != iLen) { // Matrix
			acPic[0] = '[';
			iLnLoc++;
		}
		for (int i1 = 0; i1 < iLen; i1++) {
			iLnLoc++;
			if (iLen != 1 && (i1 % iDim) == 0)
				acPic[iLnLoc++] = '[';
			if (lTyp == TOK_INTEGER) {
				memcpy(lVal, &acVal[iValId], 4);
				iValId += 4;
				_ltow(*lVal, &acPic[iLnLoc], 10);
				iVLen = (int) wcslen(&acPic[iLnLoc]);
				iLnLoc += iVLen;
			}
			else {
				memcpy(dVal, &acVal[iValId], 8);
				iValId += 8;
				if (lTyp == TOK_REAL) {
					iLoc = 1;
					// pCvtDoubToFltDecTxt(*dVal, 7, iLoc, cVal);
					LPTSTR NextToken = NULL;
					szpVal = wcstok_s(cVal, L" ", &NextToken);
					wcscpy(&acPic[iLnLoc], szpVal);
					iLnLoc += (int) wcslen(szpVal);
				}
				else if (lTyp == TOK_LENGTH_OPERAND) {
					//TODO: Length to length string
					iLnLoc += iVLen;
				}
				else if (lTyp == TOK_AREA_OPERAND) {
					// pCvtWrldToUsrAreaStr(*dVal, &acPic[iLnLoc], iVLen);
					iLnLoc += iVLen;
				}
			}
			if (iLen != 1 && (i1 % iDim) == iDim - 1)
				acPic[iLnLoc++] = ']';
		}
		if (iDim == iLen)
			*aiLen = iLnLoc - 1;
		else {
			acPic[iLnLoc] = ']';
			*aiLen = iLnLoc;
		}
	}
}
void lex::ConvertValTyp(int aiTyp, int aiTypReq, long* alDef, void* apVal) {
	if (aiTyp == aiTypReq) return;

	double* pdVal = (double*) apVal;
	long*	piVal = (long*) apVal;

	if (aiTyp == TOK_STRING) {
		WCHAR szVal[256];

		wcscpy_s(szVal, 256, (LPTSTR) apVal);
		if (aiTypReq == TOK_INTEGER) {
			piVal[0] = _wtoi(szVal);
			*alDef = MAKELONG(1, 1);
		}
		else {
			pdVal[0] = _wtof(szVal);
			*alDef = MAKELONG(1, 2);
		}
	}
	else if (aiTyp == TOK_INTEGER) {
		if (aiTypReq == TOK_STRING) {
			// integer to string
		}
		else {
			pdVal[0] = (double) piVal[0];
			*alDef = MAKELONG(1, 2);
		}
	}
	else {
		if (aiTypReq == TOK_STRING) {
			// real to string
		}
		else if (aiTypReq == TOK_INTEGER) {
			// real to integer
		}
	}
}
void lex::ConvertStringToVal(int aiTyp, long alDef, LPTSTR aszVal, long* alDefReq, void* aVal) {
	if (LOWORD(alDef) <= 0) throw L"Empty string";

	WCHAR szTok[64];
	int iNxt = 0;

	int iTyp = Scan(szTok, aszVal, iNxt);
	if (aiTyp == TOK_INTEGER) { // Conversion to integer
		long *pVal = (long *) aVal;

		if (iTyp == TOK_INTEGER)
			*pVal = _wtol(szTok);
		else if (iTyp == TOK_REAL)
			*pVal = (long) _wtof(szTok);
		else
			throw L"String format conversion error";
		*alDefReq = MAKELONG(1, 1);
	}
	else {
		double *pVal = (double *) aVal;

		if (iTyp == TOK_INTEGER)
			*pVal = (double) _wtoi(szTok);
		else if (iTyp == TOK_REAL)
			*pVal = _wtof(szTok);
		else
			throw L"String format conversion error";
		*alDefReq = MAKELONG(1, 2);
	}
}
void lex::EvalTokenStream(int* aiTokId, long* alDef, int* aiTyp, void* apOp) {
	WCHAR szTok[256];

	int iDim;
	int iTyp;

	long lDef1 = MAKELONG(1, 1);
	int iDim1;
	int iLen1;
	int iTyp1 = TOK_INTEGER;

	long lDef2;
	int iDim2;
	int iLen2;
	int iTyp2;

	int NumberOfTokens;
	int iExprTokTyp[32];
	int iExprTokLoc[32];

	BreakExpression(*aiTokId, NumberOfTokens, iExprTokTyp, iExprTokLoc);

	int iOpStkTyp[32];
	long lOpStk[32][32];
	long lOpStkDef[32];

	LPTSTR cOp1 = (LPTSTR) apOp;
	double* dOp1 = (double*) apOp;
	long* lOp1 = (long*) apOp;

	WCHAR cOp2[256];
	double* dOp2 = (double*) cOp2;
	long* lOp2 = (long*) cOp2;

	int iOpStkTop = 0;												// Empty operand stack
	int iTokStkId = 0;												// Start with first token

	while (iTokStkId < NumberOfTokens) {
		int iTokTyp = iExprTokTyp[iTokStkId];
		int iTokLoc = iExprTokLoc[iTokStkId];
		if (TokenTable[iTokTyp].eClass == Identifier) {
			// symbol table stuff if desired
			throw L"Identifier token class not implemented";
		}
		else if (TokenTable[iTokTyp].eClass == Constant) {
			iTyp1 = iTokTyp;
			lDef1 = lValues[iValLoc[iTokLoc]];
			memcpy(cOp1, &lValues[iValLoc[iTokLoc] + 1], HIWORD(lDef1) * 4);
		}
		else { // Token is an operator .. Pop an operand from operand stack
			if (iOpStkTop == 0) throw L"Operand stack is empty";

			iTyp1 = iOpStkTyp[iOpStkTop];
			lDef1 = lOpStkDef[iOpStkTop];
			iLen1 = HIWORD(lDef1);
			memcpy(cOp1, &lOpStk[iOpStkTop--][0], iLen1 * 4);

			if (TokenTable[iTokTyp].eClass == Other) { // intrinsics and oddball unary minus/plus
				if (iTyp1 == TOK_STRING) {
					iDim1 = LOWORD(lDef1);
					wcscpy_s(szTok, 256, cOp1);
					if (iTokTyp == TOK_TOINTEGER) {
						iTyp1 = TOK_INTEGER;
						ConvertStringToVal(TOK_INTEGER, lDef1, szTok, &lDef1, cOp1);
					}
					else if (iTokTyp == TOK_REAL) {
						iTyp1 = TOK_REAL;
						ConvertStringToVal(TOK_REAL, lDef1, szTok, &lDef1, cOp1);
					}
					else if (iTokTyp == TOK_STRING)
						;
					else
						throw L"String operand conversions error: unknown";
				}
				else if (iTyp1 == TOK_INTEGER)
					UnaryOp(iTokTyp, &iTyp1, &lDef1, lOp1);
				else
					UnaryOp(iTokTyp, &iTyp1, &lDef1, dOp1);
			}
			else if (TokenTable[iTokTyp].eClass == BinaryArithOp) { // Binary arithmetic operator
				if (iOpStkTop == 0) throw L"Binary Arithmetic: Only one operand.";
				iTyp2 = iOpStkTyp[iOpStkTop];				// Pop second operand from operand stack
				lDef2 = lOpStkDef[iOpStkTop];
				iLen2 = HIWORD(lDef2);
				memcpy(cOp2, &lOpStk[iOpStkTop--][0], iLen2 * 4);
				iTyp = EoMin(iTyp2, TOK_REAL);
				if (iTyp1 < iTyp) { // Convert first operand
					ConvertValTyp(iTyp1, iTyp, &lDef1, lOp1);
					iTyp1 = iTyp;
					iLen1 = HIWORD(lDef1);
				}
				else {
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
					}
					else {
						if (iTyp1 == TOK_INTEGER)
							lOp1[0] += lOp2[0];
						else
							dOp1[0] += dOp2[0];
					}
				}
				else if (iTokTyp == TOK_BINARY_MINUS) {
					if (iTyp1 == TOK_STRING) throw L"Can not subtract strings";
					if (iTyp1 == TOK_INTEGER)
						lOp1[0] = lOp2[0] - lOp1[0];
					else
						dOp1[0] = dOp2[0] - dOp1[0];
				}
				else if (iTokTyp == TOK_MULTIPLY) {
					if (iTyp1 == TOK_STRING) throw L"Can not mutiply strings";
					if (iTyp1 == TOK_INTEGER)
						lOp1[0] *= lOp2[0];
					else {
						if (iTyp1 == TOK_REAL)
							iTyp1 = iTyp2;
						else if (iTyp2 == TOK_REAL)
							;
						else if (iTyp1 == TOK_LENGTH_OPERAND && iTyp2 == TOK_LENGTH_OPERAND)
							iTyp1 = TOK_AREA_OPERAND;
						else
							throw L"Invalid mix of multiplicands";

						dOp1[0] *= dOp2[0];
					}
				}
				else if (iTokTyp == TOK_DIVIDE) {
					if (iTyp1 == TOK_STRING) throw L"Can not divide strings";
					if (iTyp1 == TOK_INTEGER) {
						if (lOp1[0] == 0) throw L"Attempting to divide by 0";
						lOp1[0] = lOp2[0] / lOp1[0];
					}
					else if (iTyp1 <= iTyp2) {
						if (dOp1[0] == 0.) throw L"Attempting to divide by 0.";
						if (iTyp1 == iTyp2)
							iTyp1 = TOK_REAL;
						else if (iTyp1 == TOK_REAL)
							iTyp1 = iTyp2;
						else
							iTyp1 = TOK_LENGTH_OPERAND;
						dOp1[0] = dOp2[0] / dOp1[0];
					}
					else
						throw L"Division type error";
				}
				else if (iTokTyp == TOK_EXPONENTIATE) {
					if (iTyp1 == TOK_INTEGER) {
						if ((lOp1[0] >= 0 && lOp1[0] > DBL_MAX_10_EXP) || (lOp1[0] < 0 && lOp1[0] < DBL_MIN_10_EXP))
							throw L"Exponentiation error";

						lOp1[0] = (int) pow((double) lOp2[0], lOp1[0]);
					}
					else if (iTyp1 == TOK_REAL) {
						int iExp = (int) dOp1[0];

						if ((iExp >= 0 && iExp > DBL_MAX_10_EXP) || (iExp < 0 && iExp < DBL_MIN_10_EXP))
							throw L"Exponentiation error";
						dOp1[0] = pow(dOp2[0], dOp1[0]);
					}
				}
			}
			else if (TokenTable[iTokTyp].eClass == BinaryRelatOp) {
				// if support for binary relational operators desired (== != > >= < <=)
				throw L"Binary relational operators not implemented";
			}
			else if (TokenTable[iTokTyp].eClass == BinaryLogicOp) {
				// if support for binary logical operators desired (& |)
				throw L"Binary logical operators not implemented";
			}
			else if (TokenTable[iTokTyp].eClass == UnaryLogicOp) {
				// if support for unary logical operator desired (!)
				throw L"Unary logical operator not implemented";
			}
		}
		iOpStkTop++;						// Increment opernad stack pointer
		iOpStkTyp[iOpStkTop] = iTyp1;		// Push operand onto operand stack
		lOpStkDef[iOpStkTop] = lDef1;
		memcpy(&lOpStk[iOpStkTop][0], cOp1, HIWORD(lDef1) * 4);
		iTokStkId++;
	}
	*aiTyp = iTyp1;
	*alDef = lDef1;
}
void lex::Init() {
	iToks = 0;
	iValsCount = 0;
}
void lex::Parse(LPCWSTR szLine) {
	iToks = 0;
	iValsCount = 0;

	WCHAR szTok[256];

	int iBeg = 0;
	int iLnLen = (int) wcslen(szLine);

	while (iBeg < iLnLen) {
		int iTyp = Scan(szTok, szLine, iBeg);

		if (iTyp == - 1) return;
		if (iToks == TOKS_MAX) return;

		iTokenType[iToks] = iTyp;
		int iLen = (int) wcslen(szTok);
		int iDim;
		double dVal;

		switch (iTyp) {
		case TOK_IDENTIFIER:
			iDim = (int) wcslen(szTok);
			iLen = 1 + (iDim - 1) /  4;

			iValLoc[iToks] = iValsCount + 1;
			lValues[iValsCount + 1] = iDim + iLen * 65536;
			memcpy(&lValues[iValsCount + 2], szTok, iDim);
			iValsCount = iValsCount + 1 + iLen;
			break;

		case TOK_STRING:
			ParseStringOperand(szTok);
			break;

		case TOK_INTEGER:
			iValLoc[iToks] = iValsCount;
			lValues[iValsCount++] = MAKELONG(1, 1);
			lValues[iValsCount++] = _wtoi(szTok);
			break;

		case TOK_REAL:
		case TOK_LENGTH_OPERAND:
			dVal = (iTyp == TOK_REAL) ? _wtof(szTok) : theApp.ParseLength(szTok);

			iValLoc[iToks] = iValsCount;
			lValues[iValsCount++] = MAKELONG(1, 2);
			memcpy(&lValues[iValsCount++], &dVal, sizeof(double));
			iValsCount++;
			break;
		}
		iToks++;
	}
}
void lex::ParseStringOperand(LPCWSTR pszTok) {
	if (wcslen(pszTok) < 3) {
		theApp.AddStringToMessageList(IDS_MSG_ZERO_LENGTH_STRING);
		return;
	}

	LPTSTR pszValues = (LPTSTR) &lValues[iValsCount + 2];

	int iDim = 0;
	int iNxt = 1;
	while (pszTok[iNxt] != '\0') {
		if (pszTok[iNxt] == '"' && pszTok[iNxt + 1] == '"') iNxt++;
		pszValues[iDim++] = pszTok[iNxt++];
	}
	pszValues[--iDim] = '\0';
	int iLen = 1 + (iDim - 1) /  4;
	iValLoc[iToks] = ++iValsCount;
	lValues[iValsCount] = MAKELONG(iDim, iLen);
	iValsCount += iLen;
}

int lex::Scan(LPTSTR aszTok, LPCWSTR szLine, int& iLP) {
	int iLen;

	while (szLine[iLP] == ' ') {iLP++;}

	int iBegLoc = iLP;
	int iTokLoc = iLP;
	int iRetVal = - 1;
	int iS = 1;

	bool bDone = false;
	while (!bDone) {
		int iAddr = iBase[iS] + szLine[iLP];

		if (iCheck[iAddr] == iS) {
			iS = iNext[iAddr];
			if (iTokVal[iS] != 0) {
				iRetVal = iTokVal[iS];
				iTokLoc = iLP;
			}
			iLP++;
		}
		else if (iDefault[iS] != 0)
			iS = iDefault[iS];
		else
			bDone = true;
	}

	iLen = iTokLoc - iBegLoc + 1;
	_tcsncpy(aszTok, &szLine[iBegLoc], iLen);
	aszTok[iLen] = '\0';
	ATLTRACE2(atlTraceGeneral, 1, L"LinePointer = %d, TokenID = %d\n", iLP, iRetVal);
	if (iRetVal == - 1) {iLP = iBegLoc + 1;}
	return (iRetVal);
}
int lex::TokType(int aiTokId) {
	return (aiTokId >= 0 && aiTokId < lex::iToks) ? iTokenType[aiTokId] : - 1;
}
void lex::UnaryOp(int aiTokTyp, int* aiTyp, long* alDef, double* adOp) {
	CD		cd;
	WCHAR szTok[32];
	int 	i;

	int iDim = LOWORD(*alDef);
	int iLen = HIWORD(*alDef);

	switch (aiTokTyp) {
	case TOK_UNARY_MINUS:
		for (i = 0; i < iLen / 2; i++)
			adOp[i] = - adOp[i];
		break;

	case TOK_UNARY_PLUS:
		break;

	case TOK_ABS:
		adOp[0] = fabs(adOp[0]);
		break;

	case TOK_ACOS:
		if (fabs(adOp[0]) > 1.)
			throw L"Math error: acos of a value greater than 1.";
		else
			adOp[0] = acos(EoToDegree(adOp[0]));
		break;

	case TOK_ASIN:
		if (fabs(adOp[0]) > 1.)
			throw L"Math error: asin of a value greater than 1.";
		else
			adOp[0] = asin(EoToDegree(adOp[0]));
		break;

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

	case TOK_TOINTEGER:		// Conversion to integer
		ConvertValTyp(TOK_REAL, TOK_INTEGER, alDef, (void*) adOp);
		*aiTyp = TOK_INTEGER;
		break;

	case TOK_LN:
		if (adOp[0] <= 0.)
			throw L"Math error: ln of a non-positive number";
		else
			adOp[0] = log(adOp[0]);
		break;

	case TOK_LOG:
		if (adOp[0] <= 0.)
			throw L"Math error: log of a non-positive number";
		else
			adOp[0] = log10(adOp[0]);
		break;

	case TOK_SIN:
		adOp[0] = sin(EoToRadian(adOp[0]));
		break;

	case TOK_SQRT:
		if (adOp[0] < 0.)
			throw L"Math error: sqrt of a negative number";
		else
			adOp[0] = sqrt(adOp[0]);
		break;

	case TOK_TAN:
		adOp[0] = tan(EoToRadian(adOp[0]));
		break;

	case TOK_TOSTRING:	// Conversion to string
		*aiTyp = TOK_STRING;
		cd.lTyp = TOK_REAL;
		cd.lDef = *alDef;
		ConvertValToString((LPTSTR) adOp, &cd, szTok, &iDim);
		iLen = 1 + (iDim - 1) / 4;
		wcscpy((LPTSTR) adOp, szTok);
		*alDef = MAKELONG(iDim, iLen);
		break;

	default:
		throw L"Unknown operation";
	}
}
void lex::UnaryOp(int aiTokTyp, int* aiTyp, long* alDef, long* alOp) {
	CD		cd;
	WCHAR szTok[32];

	int iDim = LOWORD(*alDef);
	int iLen = HIWORD(*alDef);

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
		ConvertValTyp(TOK_INTEGER, TOK_REAL, alDef, (void*) alOp);
		*aiTyp = TOK_REAL;
		break;

	case TOK_TOSTRING:
		*aiTyp = TOK_STRING;
		cd.lTyp = TOK_INTEGER;
		cd.lDef = *alDef;
		ConvertValToString((LPTSTR) alOp, &cd, szTok, &iDim);
		iLen = 1 + (iDim - 1) / 4;
		wcscpy((LPTSTR) alOp, szTok);
		*alDef = MAKELONG(iDim, iLen);
		break;

	default:
		throw L"Unknown operation";
	}
}
LPTSTR lex::ScanForChar(WCHAR c, LPTSTR *ppStr) {
	LPTSTR p = lex::SkipWhiteSpace(*ppStr);

	if (*p == c) {
		*ppStr = p + 1;
		return p;
	}
	return 0; // not found
}
LPTSTR lex::SkipWhiteSpace(LPTSTR pszString) {
	while (pszString && *pszString && isspace(*pszString))
		pszString++;

	return pszString;
}
LPTSTR lex::ScanForString(LPTSTR *ppStr, LPTSTR pszTerm, LPTSTR *ppArgBuf) {
	LPTSTR pIn = lex::SkipWhiteSpace(*ppStr);
	LPTSTR pStart = *ppArgBuf;
	LPTSTR pOut = pStart;

	bool bInQuotes = *pIn == '"';

	if (bInQuotes)
		pIn++;

	do {
		if (bInQuotes) {
			if ((*pIn == '"') && (*(pIn + 1) != '"')) { // Skip over the quote
				pIn++;
				break;
			}
		}
		else if (isalnum(*pIn))
			;
		else { // allow some peg specials
			if (!(*pIn == '_' || *pIn == '$' || *pIn == '.' || *pIn == '-' || *pIn == ':' || *pIn == '\\'))
				break;
		}
		if ((*pIn == '"') && (*(pIn + 1) == '"'))
			// Skip the escaping first quote
				pIn++;

		if (*pIn == '\\' && *(pIn + 1) == '\\')
			// Skip the escaping backslash
				pIn++;

		*pOut++ = *pIn++;				// the char to the arg buffer

	} while (*pIn);

	*pOut++ = '\0'; 					// Set up the terminating char and update the scan pointer
	*pszTerm = *pIn;
	if (*pIn)
		*ppStr = pIn+1;
	else
		*ppStr = pIn;

	*ppArgBuf = pOut;					// Update the arg buffer to the next free bit

	return pStart;
}
#pragma warning(pop)
