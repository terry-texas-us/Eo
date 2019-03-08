#pragma once

class EoDbPegFile;

/// <summary>Compute a not so arbitrary axis for AutoCAD entities</summary>
OdGeVector3d ComputeArbitraryAxis(const OdGeVector3d& normal);

class AeSysView;
class EoDbGroupList;
class EoDbGroup;

class EoDbPrimitive : public CObject {
public:
	static const EoUInt16 BUFFER_SIZE = 2048;

	static const EoInt16 COLORINDEX_BYBLOCK = 0x0000;
	static const EoInt16 COLORINDEX_BYLAYER = 256;
	static const EoInt16 LINETYPE_BYBLOCK = 32766;
	static const EoInt16 LINETYPE_BYLAYER = 32767;

protected:
	OdDbObjectId m_LayerId;
	EoInt16	m_ColorIndex;
	EoInt16	m_LinetypeIndex;
	OdDbObjectId m_EntityObjectId;

	static	EoInt16	sm_LayerColorIndex;
	static	EoInt16	sm_LayerLinetypeIndex;
	static	EoInt16	sm_HighlightColorIndex;
	static	EoInt16	sm_HighlightLinetypeIndex;
	static	size_t sm_ControlPointIndex;
	static	double 	sm_RelationshipOfPoint;
	static	double sm_SelectApertureSize;

public: // Constructors and destructor
	EoDbPrimitive();
	EoDbPrimitive(EoInt16 colorIndex, EoInt16 linetypeIndex);
	virtual ~EoDbPrimitive();

public: // Methods - absolute virtuals
	virtual void AddToTreeViewControl(HWND tree, HTREEITEM parent) const = 0;
	virtual void AssociateWith(OdDbBlockTableRecordPtr blockTableRecord) = 0;
	virtual EoDbPrimitive* Clone(OdDbDatabasePtr database) const = 0;
	virtual void Display(AeSysView* view, CDC* deviceContext) = 0;
	virtual void AddReportToMessageList(const OdGePoint3d& point) const = 0;
	virtual void FormatExtra(CString& extra) const = 0;
	virtual void FormatGeometry(CString& geometry) const = 0;
	virtual void GetAllPoints(OdGePoint3dArray& points) const = 0;
	virtual OdGePoint3d GetCtrlPt() const = 0;
	virtual void GetExtents(AeSysView* view, OdGeExtents3d& extents) const = 0;
	virtual OdGePoint3d GoToNxtCtrlPt() const = 0;
	virtual bool Is(EoUInt16 type) const = 0;
	virtual bool IsEqualTo(EoDbPrimitive* primitive) const = 0;
	/// <summary>Tests whether a line is wholly or partially within the current view volume.</summary>
	virtual bool IsInView(AeSysView* view) const = 0;
	/// <summary>Determines if a line is identified by a point.</summary>
	virtual bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const = 0;
	virtual OdGePoint3d SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const = 0;
	/// <summary>Determines whether a line is partially or wholly within the area defined by the two points passed.</summary>
	virtual bool SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const = 0;
	/// <summary>Evaluates whether a point lies within tolerance specified of line.</summary>
	virtual bool SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const = 0;
	virtual void TransformBy(const EoGeMatrix3d& transformMatrix) = 0;
	virtual void TranslateUsingMask(const OdGeVector3d& translate, const DWORD) = 0;
	virtual bool Write(EoDbFile& file) const = 0;
	virtual void Write(CFile& file, EoByte* buffer) const = 0;

public: // Methods - virtuals
	virtual void CutAt(const OdGePoint3d& point, EoDbGroup*, OdDbDatabasePtr database);
	/// <summary>Cuts a primitive at two points.</summary>
	/// <param name="points"></param>
	/// <param name="groups">group to place optional line not defined by the cut points</param>
	/// <param name="newGroups">group to place line defined by the cut points</param>
	virtual void CutAt2Points(OdGePoint3d* points, EoDbGroupList* groups, EoDbGroupList* newGroups, OdDbDatabasePtr database);
	virtual int IsWithinArea(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, OdGePoint3d* intersections);
	virtual void ModifyState();
	virtual bool PivotOnGripPoint(AeSysView* view, const EoGePoint4d& point);
	/// <summary>Evaluates whether a line intersects line.</summary>
	virtual bool SelectBy(const EoGeLineSeg3d& line, AeSysView* view, OdGePoint3dArray& intersections);

public: // Methods
	EoInt16 ColorIndex() const;
	OdDbObjectId EntityObjectId() const;
	CString FormatColorIndex() const;
	CString FormatLinetypeIndex() const;
	EoInt16 LinetypeIndex() const;
	EoInt16 LogicalColorIndex() const;
	EoInt16 LogicalLinetypeIndex() const;
	void SetColorIndex(EoInt16 colorIndex);
	void SetEntityObjectId(OdDbObjectId entity);
	void SetLinetypeIndex(EoInt16 linetypeIndex);

public: // Methods - static
	static size_t ControlPointIndex();
	static EoInt16 HighlightColorIndex();
	static EoInt16 HighlightLinetypeIndex();
	static bool IsSupportedLinetype(int linetype);
	static EoInt16 LayerColorIndex();
	static EoInt16 LayerLinetypeIndex();
	static double RelationshipOfPoint();
	static void SetHighlightColorIndex(EoInt16 colorIndex);
	static void SetHighlightLinetypeIndex(EoInt16 linetypeIndex);
	static void SetLayerColorIndex(EoInt16 colorIndex);
	static void SetLayerLinetypeIndex(EoInt16 linetypeIndex);
};
