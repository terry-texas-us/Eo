#pragma once
#include "EoGePoint3d.h"
#include "EoGeReferenceSystem.h"
class EoGeLineSeg3d;

/// <summary>Compute a not so arbitrary axis for AutoCAD entities</summary>
OdGeVector3d ComputeArbitraryAxis(const OdGeVector3d& normal);

double ComputeElevation(const OdGePoint3d& point, const OdGeVector3d& normal);

OdGeVector3d ComputeNormal(const OdGePoint3d& pointU, const OdGePoint3d& origin, const OdGePoint3d& pointV);

class AeSysView;
class EoDbGroupList;
class EoDbGroup;

class EoDbPrimitive : public CObject {
DECLARE_DYNAMIC(EoDbPrimitive)

	static const unsigned short mc_BufferSize = 2048;
	static const short mc_ColorindexByblock = 0x0000;
	static const short mc_ColorindexBylayer = 256;
	static const short mc_LinetypeByblock = 32766;
	static const short mc_LinetypeBylayer = 32767;
protected:
	OdDbObjectId m_LayerId {nullptr};
	short m_ColorIndex {1};
	short m_LinetypeIndex {1};
	OdDbObjectId m_EntityObjectId {nullptr};
	static short ms_LayerColorIndex;
	static short ms_LayerLinetypeIndex;
	static short ms_HighlightColorIndex;
	static short ms_HighlightLinetypeIndex;
	static unsigned ms_ControlPointIndex;
	static double ms_RelationshipOfPoint;
	static double ms_SelectApertureSize;
public:
	EoDbPrimitive() = default;

	virtual ~EoDbPrimitive() = default;

	virtual void AddToTreeViewControl(HWND tree, HTREEITEM parent) const = 0;

	virtual EoDbPrimitive* Clone(OdDbBlockTableRecordPtr blockTableRecord) const = 0;

	virtual void Display(AeSysView* view, CDC* deviceContext) = 0;

	virtual void AddReportToMessageList(const OdGePoint3d& point) const = 0;

	virtual void FormatExtra(CString& extra) const = 0;

	virtual void FormatGeometry(CString& geometry) const = 0;

	virtual void GetAllPoints(OdGePoint3dArray& points) const = 0;

	virtual OdGePoint3d GetCtrlPt() const = 0;

	virtual void GetExtents(AeSysView* view, OdGeExtents3d& extents) const = 0;

	virtual OdGePoint3d GoToNxtCtrlPt() const = 0;

	virtual bool IsEqualTo(EoDbPrimitive* primitive) const = 0;
	/// <summary>Tests whether a line is wholly or partially within the current view volume.</summary>
	virtual bool IsInView(AeSysView* view) const = 0;
	/// <summary>Determines if a line is identified by a point.</summary>
	virtual bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const = 0;

	virtual OdGePoint3d SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const = 0;
	/// <summary>Evaluates whether a line intersects line.</summary>
	virtual bool SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view, OdGePoint3dArray& intersections) = 0;
	/// <summary>Determines whether a line is partially or wholly within the area defined by the two points passed.</summary>
	virtual bool SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const = 0;
	/// <summary>Evaluates whether a point lies within tolerance specified of line.</summary>
	virtual bool SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const = 0;

	virtual void TransformBy(const EoGeMatrix3d& transformMatrix) = 0;

	virtual void TranslateUsingMask(const OdGeVector3d& translate, unsigned long mask) = 0;

	virtual bool Write(EoDbFile& file) const = 0;

	virtual void Write(CFile& file, unsigned char* buffer) const = 0;

	virtual void CutAt(const OdGePoint3d& point, EoDbGroup* newGroup);
	/// <summary>Cuts a primitive at two points.</summary>
	/// <param name="points"></param>
	/// <param name="groups">group to place optional line not defined by the cut points</param>
	/// <param name="newGroups">group to place line defined by the cut points</param>
	/// <param name="database"></param>
	virtual void CutAt2Points(OdGePoint3d* points, EoDbGroupList* groups, EoDbGroupList* newGroups, OdDbDatabasePtr database);

	virtual int IsWithinArea(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, OdGePoint3d* intersections);

	virtual void ModifyState() noexcept;

	virtual bool PivotOnGripPoint(AeSysView* view, const EoGePoint4d& point) noexcept;

	short ColorIndex() const noexcept { return m_ColorIndex; }

	OdDbObjectId EntityObjectId() const noexcept { return m_EntityObjectId; }

	CString FormatColorIndex() const;

	CString FormatLinetypeIndex() const;

	short LinetypeIndex() const noexcept { return m_LinetypeIndex; }

	short LogicalColorIndex() const noexcept;

	short LogicalLinetypeIndex() const noexcept;

	void SetColorIndex(const short colorIndex) noexcept { m_ColorIndex = colorIndex; }

	void SetColorIndex2(short colorIndex);

	void SetEntityObjectId(const OdDbObjectId entityObjectId) noexcept { m_EntityObjectId = entityObjectId; }

	void SetLinetypeIndex(const short linetypeIndex) noexcept { m_LinetypeIndex = linetypeIndex; }

	void SetLinetypeIndex2(short linetypeIndex);

	static unsigned ControlPointIndex() noexcept;

	static short HighlightColorIndex() noexcept;

	static short HighlightLinetypeIndex() noexcept;

	static bool IsSupportedLinetype(int linetype) noexcept;

	static short LayerColorIndex() noexcept;

	static short LayerLinetypeIndex() noexcept;

	static double RelationshipOfPoint() noexcept;

	static void SetHighlightColorIndex(short colorIndex) noexcept;

	static void SetHighlightLinetypeIndex(short linetypeIndex) noexcept;

	static void SetLayerColorIndex(short colorIndex) noexcept;

	static void SetLayerLinetypeIndex(short linetypeIndex) noexcept;

	static OdDbObjectId LinetypeObjectFromIndex(short linetypeIndex);

	static OdDbObjectId LinetypeObjectFromIndex0(OdDbDatabasePtr database, short linetypeIndex);
};
