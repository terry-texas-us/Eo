#pragma once

#include "EoGeReferenceSystem.h"

#include "EoDb.h"
#include "EoDbFontDefinition.h"
#include "EoDbPrimitive.h"

/* <remarks>
Text primitive in Peg files and Tracing files
  Type code <0x4000>        OdUInt16[0-1]
  Pen color                 OdUInt16[2-3]
  Line type                 OdUInt16[4-5]
  Font definition
	Text precision          OdUInt16[6-7]
	Text font name          string
	Text path               OdUInt16
	Horizontal alignment    OdUInt16
	Vertical alignment      OdUInt16
	Character spacing       double
  Insertion point           OdGePoint3d
  Local reference x-axis    OdGeVector3d
  Local reference y-axis    OdGeVector3d
  Text('\t' terminated)     string
</remarks> */

class EoDbText : public EoDbPrimitive {
	EoDbFontDefinition m_FontDefinition;
	EoGeReferenceSystem m_ReferenceSystem;
	CString m_strText;

public:	// Constructors and destructor

	EoDbText();
	EoDbText(const EoDbText& other);
	const EoDbText& operator=(const EoDbText&);

	~EoDbText();

public: // Methods - absolute virtuals

	void AddReportToMessageList(const OdGePoint3d& point) const override;
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept override;
	EoDbPrimitive* Clone(OdDbDatabasePtr& database) const override;
	void Display(AeSysView* view, CDC* deviceContext) override;
	void GetAllPoints(OdGePoint3dArray& points) const override;
	void FormatExtra(CString& extra) const override;
	void FormatGeometry(CString& geometry) const override;
	OdGePoint3d	GetCtrlPt() const noexcept override;
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const override;
	OdGePoint3d	GoToNxtCtrlPt() const noexcept override;
	bool Is(OdUInt16 type) const noexcept override { return type == EoDb::kTextPrimitive; }
	bool IsEqualTo(EoDbPrimitive* primitive) const noexcept override;
	bool IsInView(AeSysView* view) const override;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const override;
	void ModifyState() noexcept override;
	void ModifyNotes(const EoDbFontDefinition& fontDefinition, EoDbCharacterCellDefinition& characterCellDefinition, int iAtt);
	OdGePoint3d	SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const override;
	/// <summary>Evaluates whether a point lies within the bounding region of text.</summary>
	bool SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const override;
	bool SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const override;
	void TransformBy(const EoGeMatrix3d& transformMatrix) override;
	void TranslateUsingMask(const OdGeVector3d& translate, const DWORD) override;
	bool Write(EoDbFile& file) const override;
	void Write(CFile& file, OdUInt8* buffer) const override;

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
	void SetHorizontalMode(EoDb::HorizontalAlignment horizontalAlignment);
	void SetReferenceSystem(const EoGeReferenceSystem& referenceSystem) noexcept;
	void SetText(const CString& text);
	EoDbText& SetTo(const EoDbFontDefinition& fontDefinition, const EoGeReferenceSystem& referenceSystem, const CString& text);
	void SetVerticalMode(EoDb::VerticalAlignment verticalAlignment);

public: // Methods - static
	static void ConvertFractionMarkup(CString& text);
	static EoDb::HorizontalAlignment ConvertHorizontalAlignment(const OdDb::TextHorzMode horizontalMode) noexcept;
	static EoDb::VerticalAlignment ConvertVerticalAlignment(const OdDb::TextVertMode verticalMode) noexcept;
	static OdDb::TextHorzMode ConvertHorizontalMode(const OdUInt16 horizontalAlignment) noexcept;
	static OdDb::TextVertMode ConvertVerticalMode(const OdUInt16 verticalAlignment) noexcept;

	static EoDbText* ConstructFrom(OdUInt8* primitiveBuffer, int versionNumber);

	static EoDbText* Create(const EoDbText& other, OdDbBlockTableRecordPtr& blockTableRecord);
	static OdDbTextPtr Create(OdDbBlockTableRecordPtr& blockTableRecord, EoDbFile& file);
	static OdDbTextPtr Create(OdDbBlockTableRecordPtr& blockTableRecord, const OdGePoint3d& position, const OdString& textString);
	static OdDbMTextPtr CreateM(OdDbBlockTableRecordPtr& blockTableRecord, OdString text);
	static EoDbText* Create(OdDbTextPtr& text);
	static EoDbText* Create(OdDbMTextPtr& text);
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
OdGePoint3d CalculateInsertionPoint(const EoDbFontDefinition& fontDefinition, int iChrs) noexcept;
/// <summary>Returns the region boundaries of a text string applying and optional inflation factor.</summary>
void text_GetBoundingBox(const EoDbFontDefinition& fontDefinition, const EoGeReferenceSystem& referenceSystem, int numberOfCharacters, double spaceFactor, OdGePoint3dArray& boundingBox);
OdGePoint3d text_GetNewLinePos(EoDbFontDefinition& fontDefinition, EoGeReferenceSystem& referenceSystem, double dLineSpaceFac, double dChrSpaceFac);
