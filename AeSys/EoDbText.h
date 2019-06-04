#pragma once

#include "DbText.h"

#include "EoGeReferenceSystem.h"

#include "EoDb.h"
#include "EoDbFontDefinition.h"
#include "EoDbPrimitive.h"

/* <remarks>
Text primitive in Peg files and Tracing files
  Type code <0x4000>        unsigned short[0-1]
  Pen color                 unsigned short[2-3]
  Line type                 unsigned short[4-5]
  Font definition
	Text precision          unsigned short[6-7]
	Text font name          string
	Text path               unsigned short
	Horizontal alignment    unsigned short
	Vertical alignment      unsigned short
	Character spacing       double
  Insertion point           OdGePoint3d
  Local reference x-axis    OdGeVector3d
  Local reference y-axis    OdGeVector3d
  Text('\t' terminated)     string
</remarks> */

class EoDbText : public EoDbPrimitive {
	DECLARE_DYNAMIC(EoDbText)

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
	EoDbPrimitive* Clone(OdDbBlockTableRecordPtr blockTableRecord) const override;
	void Display(AeSysView* view, CDC* deviceContext) override;
	void GetAllPoints(OdGePoint3dArray& points) const override;
	void FormatExtra(CString& extra) const override;
	void FormatGeometry(CString& geometry) const override;
	OdGePoint3d	GetCtrlPt() const noexcept override;
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const override;
	OdGePoint3d	GoToNxtCtrlPt() const noexcept override;
	bool Is(unsigned short type) const noexcept override { return type == EoDb::kTextPrimitive; }
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
	void TranslateUsingMask(const OdGeVector3d& translate, const unsigned long mask) override;
	bool Write(EoDbFile& file) const override;
	void Write(CFile& file, unsigned char* buffer) const override;

public: // Methods
	/// <summary>Get the bounding box of text.</summary>
	void GetBoundingBox(OdGePoint3dArray& boundingBox, double spaceFactor) const;
	EoDbFontDefinition FontDefinition() const;
	EoGeReferenceSystem ReferenceSystem() const;
	double Rotation() const;
	const CString& Text() noexcept;
	OdGePoint3d Position() const noexcept;
	void SetFontDefinition(const EoDbFontDefinition& fontDefinition);
	void SetReferenceSystem(const EoGeReferenceSystem& referenceSystem) noexcept;
	void SetText(const CString& text);

public: // Methods - static
	static EoDb::HorizontalAlignment ConvertHorizontalAlignment(const OdDb::TextHorzMode horizontalMode) noexcept;
	static EoDb::VerticalAlignment ConvertVerticalAlignment(const OdDb::TextVertMode verticalMode) noexcept;
	static OdDb::TextHorzMode ConvertHorizontalMode(const unsigned horizontalAlignment) noexcept;
	static OdDb::TextVertMode ConvertVerticalMode(const unsigned verticalAlignment) noexcept;

	static EoDbText* Create(OdDbTextPtr& text);
	static EoDbText* Create(OdDbMTextPtr& text);

	static OdDbTextPtr Create(OdDbBlockTableRecordPtr& blockTableRecord, EoDbFile& file);
	static OdDbTextPtr Create(OdDbBlockTableRecordPtr blockTableRecord, unsigned char* primitiveBuffer, int versionNumber);

	static OdDbTextPtr Create(OdDbBlockTableRecordPtr& blockTableRecord, const OdGePoint3d& position, const OdString& textString);
	static OdDbMTextPtr CreateM(OdDbBlockTableRecordPtr& blockTableRecord, OdString text);
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
