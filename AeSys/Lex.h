#pragma once

enum ETokClass {
	Other,
	Constant,
	Identifier,
	BinaryArithOp,
	BinaryRelatOp,
	BinaryLogicOp,
	UnaryLogicOp,
	AssignOp,
	OpenParen,
	CloseParen
};
struct CD { // column definition
	long lDef;				// data definition
	long lTyp;				// data type
};
struct tokent {
	int iInComPrio;
	int iInStkPrio;
	ETokClass eClass;
};

namespace lex {
// <LexTable> The following static table data (from LexTable.h) is a legacy from the removed command parser features.
// The only pieces still used are for simple expression evaluation.
// Lexgen can still produce this table but likely will be replaced by third party tools.
static int iBase[] = {
	0, 0, 195, 1, 1, 1, 215, 2, 2, 2, 
	224, 198, 199, 286, 5, 5, 5, 5, 5, 5, 
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
	5, 5, 5, 375, 5, 227, 6, 453, 234, 201, 
	155, 244, 155, 157, 158, 166, 155, 194, 200, 188, 
	193, 191, 192, 198, 199, 195, 265, 203, 387, 279, 
	229, 215, 209, 220, 306, 235, 232, 237, 251, 238, 
	245, 280, 277, 274, 266
};
static int iDefault[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 13, 13, 13, 13, 13, 
	13, 13, 13, 13, 13, 13, 13, 13, 13, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 41, 0, 
	0, 0, 13, 13, 13, 13, 13, 13, 13, 13, 
	13, 13, 13, 13, 13, 13, 0, 0, 0, 58, 
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 
	13, 13, 13, 13, 13
};
static int iTokVal[] = {
	0, 0, 39, 37, 40, 41, 27, 29, 30, 28, 
	20, 35, 33, 25, 38, 13, 5, 12, 11, 14, 
	9, 10, 8, 7, 6, 4, 3, 2, 1, 34, 
	31, 36, 22, 21, 26, 24, 32, 0, 0, 0, 
	0, 0, 25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 22, 22, 22, 22, 
	25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25
};
static int iNext[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 2, 37, 0, 0, 0, 3, 0, 
	4, 5, 6, 7, 0, 8, 38, 9, 10, 10, 
	10, 10, 10, 10, 10, 10, 10, 10, 0, 0, 
	11, 39, 12, 0, 0, 13, 13, 13, 13, 13, 
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 
	13, 0, 0, 0, 0, 0, 0, 60, 13, 61, 
	13, 62, 13, 13, 13, 63, 13, 13, 42, 13, 
	13, 13, 13, 13, 72, 64, 65, 13, 13, 13, 
	13, 13, 13, 0, 14, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 36, 34, 32, 31, 
	29, 37, 30, 56, 32, 20, 48, 15, 18, 16, 
	33, 17, 10, 10, 10, 10, 10, 10, 10, 10, 
	10, 10, 33, 33, 33, 33, 33, 33, 33, 33, 
	33, 33, 59, 59, 59, 59, 59, 59, 59, 59, 
	59, 59, 19, 21, 22, 23, 24, 28, 25, 26, 
	27, 41, 32, 58, 58, 58, 58, 58, 58, 58, 
	58, 58, 58, 40, 40, 41, 51, 52, 69, 50, 
	49, 40, 43, 57, 13, 13, 13, 13, 13, 13, 
	13, 13, 13, 13, 70, 71, 46, 45, 47, 55, 
	54, 13, 13, 13, 13, 13, 13, 13, 13, 13, 
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 
	13, 13, 13, 13, 13, 13, 13, 53, 68, 67, 
	73, 13, 0, 13, 13, 13, 13, 13, 13, 13, 
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 
	13, 13, 13, 13, 13, 13, 13, 13, 13, 32, 
	0, 44, 0, 0, 56, 0, 0, 0, 0, 66, 
	0, 59, 74, 33, 33, 33, 33, 33, 33, 33, 
	33, 33, 33, 59, 0, 58, 58, 58, 58, 58, 
	58, 58, 58, 58, 58, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 40, 40, 0, 0, 0, 0, 
	0, 0, 40, 0, 57, 37, 37, 35, 37, 37, 
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 
	37
};
static int iCheck[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 
	1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 
	1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 2, 6, 10, 11, 
	12, 35, 39, 10, 40, 42, 42, 43, 44, 45, 
	10, 46, 10, 10, 10, 10, 10, 10, 10, 10, 
	10, 10, 38, 38, 38, 38, 38, 38, 38, 38, 
	38, 38, 41, 41, 41, 41, 41, 41, 41, 41, 
	41, 41, 47, 48, 49, 50, 51, 52, 53, 54, 
	55, 56, 57, 56, 56, 56, 56, 56, 56, 56, 
	56, 56, 56, 10, 10, 59, 61, 60, 60, 62, 
	63, 10, 65, 10, 13, 13, 13, 13, 13, 13, 
	13, 13, 13, 13, 60, 60, 66, 67, 68, 69, 
	70, 13, 13, 13, 13, 13, 13, 13, 13, 13, 
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 
	13, 13, 13, 13, 13, 13, 13, 71, 72, 73, 
	74, 13, 0, 13, 13, 13, 13, 13, 13, 13, 
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 
	13, 13, 13, 13, 13, 13, 13, 13, 13, 33, 
	0, 64, 0, 0, 33, 0, 0, 0, 0, 64, 
	0, 58, 64, 33, 33, 33, 33, 33, 33, 33, 
	33, 33, 33, 58, 0, 58, 58, 58, 58, 58, 
	58, 58, 58, 58, 58, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 33, 33, 0, 0, 0, 0, 
	0, 0, 33, 0, 33, 37, 37, 37, 37, 37, 
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 
	37
};
// </LexTable> 

	const int TOKS_MAX = 128;		// maximum number of tokens
	const int VALS_MAX = 256;

	const int TOK_UNARY_OPERATOR = 1;
	const int TOK_BINARY_OPERATOR = 2;
	const int TOK_COMPARISON_OPERATOR = 4;
	const int TOK_LOGICAL_OPERATOR = 8;

	const int TOK_ABS				= 1;
	const int TOK_ACOS				= 2;
	const int TOK_ASIN				= 3;
	const int TOK_ATAN				= 4;
	const int TOK_TOSTRING			= 5;
	const int TOK_COS				= 6;
	const int TOK_EXP				= 7;
	const int TOK_TOINTEGER			= 8;
	const int TOK_LN				= 9;
	const int TOK_LOG				= 10;
	const int TOK_SIN				= 11;
	const int TOK_SQRT				= 12;
	const int TOK_TAN				= 13;
	const int TOK_TOREAL			= 14;
	const int TOK_UNARY_PLUS		= 15;
	const int TOK_UNARY_MINUS		= 16;

	const int TOK_INTEGER			= 20;
	const int TOK_REAL				= 21;
	const int TOK_LENGTH_OPERAND	= 22;
	const int TOK_AREA_OPERAND		= 23;
	const int TOK_STRING			= 24;
	const int TOK_IDENTIFIER		= 25;
	const int TOK_EXPONENTIATE		= 26;
	const int TOK_MULTIPLY			= 27;
	const int TOK_DIVIDE			= 28;
	const int TOK_BINARY_PLUS		= 29;
	const int TOK_BINARY_MINUS		= 30;
	const int TOK_EQ				= 31;
	const int TOK_NE				= 32;
	const int TOK_GT				= 33;
	const int TOK_GE				= 34;
	const int TOK_LT				= 35;
	const int TOK_LE				= 36;
	const int TOK_AND				= 37;
	const int TOK_OR				= 38;
	const int TOK_NOT				= 39;
	const int TOK_LPAREN			= 40;
	const int TOK_RPAREN			= 41;

	static tokent TokenTable[] = {
		{0,		0,		Other},			// unused
		{110,	85,		Other},			// abs
		{110,	85,		Other},			// acos
		{110,	85,		Other},			// asin
		{110,	85,		Other},			// atan
		{110,	85,		Other},			// string
		{110,	85,		Other},			// cos
		{110,	85,		Other},			// exp
		{110,	85,		Other},			// int
		{110,	85,		Other},			// ln
		{110,	85,		Other},			// log
		{110,	85,		Other},			// sin
		{110,	85,		Other},			// sqrt
		{110,	85,		Other},			// tan
		{110,	85,		Other},			// real
		{110,	85,		Other},			// unary+
		{110,	85,		Other},			// unary-
		{0,		0,		Other},			// unused
		{0,		0,		Other},			// unused
		{0,		0,		Other},			// unused
		{0,		0,		Constant},		// integer
		{0,		0,		Constant},		// real
		{0,		0,		Constant},		// length
		{0,		0,		Constant},		// area
		{0,		0,		Constant},		// string
		{0,		0,		Identifier},	// identifier
		{80,	79,		BinaryArithOp},	// **
		{70,	71,		BinaryArithOp},	// *
		{70,	71,		BinaryArithOp},	// /
		{60,	61,		BinaryArithOp},	// +
		{60,	61,		BinaryArithOp},	// -
		{40,	41,		BinaryRelatOp},	// ==
		{40,	41,		BinaryRelatOp},	// !=
		{40,	41,		BinaryRelatOp},	// >
		{40,	41,		BinaryRelatOp},	// >=
		{40,	41,		BinaryRelatOp},	// <
		{40,	41,		BinaryRelatOp},	// <=
		{20,	21,		BinaryLogicOp},	// &
		{10,	11,		BinaryLogicOp},	// |
		{30,	31,		UnaryLogicOp},	// !
		{110,	1,		OpenParen},		// (
		{0,		0,		CloseParen}		// )
	};
	/// <summary>
	///Converts a stream of tokens into a postfix stack for evaluation.
	/// </summary>
	/// <param name="firstTokenLocation">
	///	  (in)  location of first token in stream to consider
	///		 (out) location of first token not part of expression
	/// </param>
	/// <param name="numberOfTokens">number of tokens on stack</param>
	/// <param name="typeOfTokens">type of tokens on stack</param>
	/// <param name="locationOfTokens">location of tokens on stack</param>
	void BreakExpression(int& firstTokenLocation, int& numberOfTokens, int* typeOfTokens, int* locationOfTokens);
	/// <summary>Converts a literal user units string to a double precision value.</summary>
	// Notes:	Assumes that a valid liter user units string is passed with no suffix characters evaluated.
	// Effect:
	// Parameters:	aiTyp		type of value(s) required
	//				alDef		dimension (lo word) and length (hi word) of string
	//				aszVal		string to convert
	//				alDefReq	dimension (lo word) and length (hi word) of result
	//				aVal		result
	void ConvertStringToVal(int iTyp, long lDef, LPTSTR szVal, long* lDefReq, void* p);
	void ConvertValToString(LPTSTR, CD*, LPTSTR, int*) noexcept;
	/// <summary>Does value type conversion</summary>
	// Parameters:	aiTyp		type of value(s)
	//				aiTypReq	type of value(s) required
	//				alDef		dimension (lo word) and length (hi word) of result
	//				apVal		value(s)
	void ConvertValTyp(int, int, long*, void*) noexcept;
	/// <summary> Evaluates an expression.</summary>
	// Returns: 1 infix expression successfully evaluated
	//			0 unspecified syntax error
	// Parameters:	aiTokId
	//				alDef		dimension (lo word) and length (hi word) of result
	//				aiTyp		type of result
	//				apOp		result
	void EvalTokenStream(int*, long*, int*, void*);
	void Init() noexcept;
	/// <summary>Parses line into tokens.</summary>
	void Parse(LPCWSTR pszLine);
	void ParseStringOperand(LPCWSTR pszTok);
	/// <summary>Scan a buffer for a given character.</summary>
	// Notes:	If the character is found the scan pointer is updated
	//			to point to the character following the one found.
	// Returns: Pointer to the character if found,	0 if not.
	LPTSTR ScanForChar(wchar_t c, LPTSTR *ppStr) noexcept;
	/// <summary>Scan for a string.</summary>
	// Notes:	The scan pointer is updated to point past the string.  The
	//			arg buffer pointer is updated to point to the next free character.
	// Returns: Pointer tot he string or 0 if an error occurs.
	LPTSTR ScanForString(LPTSTR *ppStr, LPTSTR pszTerm, LPTSTR *ppArgBuf) noexcept;
	int Scan(LPTSTR aszTok, LPCWSTR pszLine, int& iLP);
	/// <summary>Skip over any white space characters.</summary>
	/// <param name="pszString">Pointer to the current buffer position.</param>
	/// <returns>Pointer to the first non-white character.</returns>
	LPTSTR SkipWhiteSpace(LPTSTR pszString) noexcept;
	/// <summary> Fetches specified tokens type from current token stream.</summary>
	// Returns:  token type
	//		 - 1 if token identifier out of range
	int TokType(int) noexcept;
	void UnaryOp(int, int*, long*,	double*);
	void UnaryOp(int, int*, long*, long*);
}
