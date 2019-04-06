#pragma once

class EoDbPrimitive;

class EoDbDimension : public EoDbPrimitive {
	EoGeLineSeg3d m_Line;

	OdInt16	m_TextColorIndex;
	EoDbFontDefinition m_FontDefinition;
	EoGeReferenceSystem m_ReferenceSystem;
	CString m_strText;

public:	// Constructors and destructor
	EoDbDimension();
	EoDbDimension(const EoDbDimension& other);
	~EoDbDimension();

public: // Operators
	const EoDbDimension& operator=(const EoDbDimension& other);

public: // Methods - absolute virtuals
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept;
	void AssociateWith(OdDbBlockTableRecordPtr& blockTableRecord) noexcept;
	EoDbPrimitive* Clone(OdDbDatabasePtr& database) const;
	void Display(AeSysView* view, CDC* deviceContext);
	void AddReportToMessageList(const OdGePoint3d& point) const;
	void FormatExtra(CString& extra) const;
	void FormatGeometry(CString& geometry) const;
	void GetAllPoints(OdGePoint3dArray& points) const;
	OdGePoint3d GetCtrlPt() const;
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const;
	OdGePoint3d GoToNxtCtrlPt() const;
	bool Is(OdUInt16 type) const noexcept;
	bool IsEqualTo(EoDbPrimitive* primitive) const noexcept;
	bool IsInView(AeSysView* view) const;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const;
	OdGePoint3d SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const;
	/// <summary>Evaluates whether a line intersects a dimension line.</summary>
	bool SelectBy(const EoGeLineSeg3d& line, AeSysView* view, OdGePoint3dArray& intersections);
	bool SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const;
	bool SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const;
	void TransformBy(const EoGeMatrix3d& transformMatrix);
	void TranslateUsingMask(const OdGeVector3d& translate, const DWORD);
	bool Write(EoDbFile& file) const;
	void Write(CFile& file, OdUInt8* buffer) const;

public:	// Methods - virtuals
	void CutAt(const OdGePoint3d& point, EoDbGroup*, OdDbDatabasePtr database);
	void CutAt2Points(OdGePoint3d* points, EoDbGroupList*, EoDbGroupList*, OdDbDatabasePtr database) noexcept;
	void ModifyState() noexcept;

public:	// Methods
	void GetBoundingBox(OdGePoint3dArray& boundingBox, double spaceFactor) const;
	const EoDbFontDefinition& FontDef() noexcept;
	const EoGeLineSeg3d& Line() noexcept;
	void GetPts(OdGePoint3d& ptBeg, OdGePoint3d& ptEnd);
	EoGeReferenceSystem ReferenceSystem() const;
	double Length() const;
	double ParametricRelationshipOf(const OdGePoint3d& point) const;
	void SetDefaultNote();
	void SetFontDefinition(const EoDbFontDefinition& fontDefinition);
	void SetStartPoint(const OdGePoint3d& startPoint);
	void SetEndPoint(const OdGePoint3d& endPoint);
	void SetReferenceSystem(const EoGeReferenceSystem& referenceSystem) noexcept;
	void SetText(const CString& str);
	void SetTextHorizontalAlignment(EoDb::HorizontalAlignment horizontalAlignment) noexcept;
	void SetTextColorIndex(OdInt16 colorIndex) noexcept;
	void SetTextVerticalAlignment(EoDb::VerticalAlignment verticalAlignment) noexcept;
	const CString& Text() noexcept;
	const OdInt16& TextColorIndex() noexcept;

private:
	static OdUInt16 sm_wFlags;	// bit 1	clear if dimension selected at note
							//			set if dimension selected at line
public:
	static EoDbDimension* ConstructFrom(EoDbFile& file);
	static EoDbDimension* ConstructFrom(OdUInt8* primitiveBuffer, int versionNumber);
};
