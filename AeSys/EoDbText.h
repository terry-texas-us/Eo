#pragma once
using namespace EoDb;

class EoDbText : public EoDbPrimitive {
	EoDbFontDefinition m_FontDefinition;
	EoGeReferenceSystem m_ReferenceSystem;
	CString m_strText;

public:	// Constructors and destructor
	EoDbText();
	EoDbText(const EoDbText& other);

	~EoDbText();
public: // Operators
	const EoDbText& operator=(const EoDbText&);

public: // Methods - absolute virtuals
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept;
	void AssociateWith(OdDbBlockTableRecordPtr blockTableRecord);
	EoDbPrimitive* Clone(OdDbDatabasePtr database) const;
	void Display(AeSysView* view, CDC* deviceContext);
	void AddReportToMessageList(const OdGePoint3d& point) const;
	void GetAllPoints(OdGePoint3dArray& points) const;
	void FormatExtra(CString& extra) const;
	void FormatGeometry(CString& geometry) const;
	OdGePoint3d	GetCtrlPt() const noexcept;
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const;
	OdGePoint3d	GoToNxtCtrlPt() const noexcept;
	bool Is(OdUInt16 type) const noexcept;
	bool IsEqualTo(EoDbPrimitive* primitive) const noexcept;
	bool IsInView(AeSysView* view) const;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const;
	void ModifyState() noexcept;
	void ModifyNotes(EoDbFontDefinition& fontDefinition, EoDbCharacterCellDefinition& characterCellDefinition, int iAtt);
	OdGePoint3d	SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const;
	/// <summary>Evaluates whether a point lies within the bounding region of text.</summary>
	bool SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const;
	bool SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const;
	void TransformBy(const EoGeMatrix3d& transformMatrix);
	void TranslateUsingMask(const OdGeVector3d& translate, const DWORD);
	bool Write(EoDbFile& file) const;
	void Write(CFile& file, OdUInt8* buffer) const;

public: // Methods
	void ConvertFormattingCharacters();
	/// <summary>Get the bounding box of text.</summary>
	void GetBoundingBox(OdGePoint3dArray& boundingBox, double spaceFactor) const;
	EoDbFontDefinition FontDefinition() const;
	EoGeReferenceSystem ReferenceSystem() const;
	double Rotation() const;
	const CString& Text() noexcept;
	OdGePoint3d Position() const noexcept;
	void SetFontDefinition(const EoDbFontDefinition& fontDefinition);
	void SetHorizontalMode(HorizontalAlignment horizontalAlignment);
	void SetReferenceSystem(const EoGeReferenceSystem& referenceSystem) noexcept;
	void SetText(const CString& text);
	EoDbText& SetTo(const EoDbFontDefinition& fontDefinition, const EoGeReferenceSystem& referenceSystem, const CString& text);
	void SetVerticalMode(VerticalAlignment verticalAlignment);

public: // Methods - static
    static void ConvertFractionMarkup(CString& text);
    static HorizontalAlignment ConvertHorizontalAlignment(const OdDb::TextHorzMode horizontalMode) noexcept;
	static VerticalAlignment ConvertVerticalAlignment(const OdDb::TextVertMode verticalMode) noexcept;
    static OdDb::TextHorzMode ConvertHorizontalMode(const OdUInt16 horizontalAlignment) noexcept;
    static OdDb::TextVertMode ConvertVerticalMode(const OdUInt16 verticalAlignment) noexcept;

    static OdDbTextPtr Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file);
    static EoDbText* ConstructFrom(EoDbFile& file);
	static EoDbText* ConstructFrom(OdUInt8* primitiveBuffer, int versionNumber);
	static EoDbText* Create(OdDbDatabasePtr database);
	static EoDbText* Create(const EoDbText& other, OdDbDatabasePtr database);
    static OdDbTextPtr Create(OdDbDatabasePtr database, OdDbBlockTableRecordPtr blockTableRecord);
    static OdDbMTextPtr Create(OdDbDatabasePtr database, OdDbBlockTableRecordPtr blockTableRecord, OdString text);
    static EoDbText* Create(OdDbTextPtr text);
    static EoDbText* Create(OdDbMTextPtr text);
};

void DisplayText(AeSysView* view, CDC* deviceContext, EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, const CString& text);
void DisplayTextSegment(AeSysView* view, CDC* deviceContext, EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, int startPosition, int numberOfCharacters, const CString& text);
/// <summary> Displays a text string using a stroke font.</summary>
void DisplayTextSegmentUsingStrokeFont(AeSysView* view, CDC* deviceContext, EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, int startPosition, int numberOfCharacters, const CString& text);
/// <summary> Attempts to display text is using true type font.</summary>
bool DisplayTextSegmentUsingTrueTypeFont(AeSysView* view, CDC* deviceContext, EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, int startPosition, int numberOfCharacters, const CString& text);
void DisplayTextWithFormattingCharacters(AeSysView* view, CDC* deviceContext, EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, const CString& text);
/// <summary> Determines the count of characters in string excluding formatting characters.</summary>
int TextLengthSansFormattingCharacters(const CString& text);
/// <summary> Determines the offset to the bottom left alignment position of a string of the specified number of characters and text attributes in the z=0 plane.</summary>
OdGePoint3d CalculateInsertionPoint(EoDbFontDefinition& fontDefinition, int iChrs) noexcept;
/// <summary>Returns the region boundaries of a text string applying and optional inflation factor.</summary>
void text_GetBoundingBox(const EoDbFontDefinition& fontDefinition, const EoGeReferenceSystem& referenceSystem, int numberOfCharacters, double spaceFactor,  OdGePoint3dArray& boundingBox);
OdGePoint3d text_GetNewLinePos(EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, double dLineSpaceFac, double dChrSpaceFac);
