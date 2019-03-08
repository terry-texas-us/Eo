#pragma once

class AeSysDoc;
class EoDbBlock;

class EoDbGroup: public CObList {
	static EoDbPrimitive* sm_PrimitiveToIgnore;

public:
	AeSysDoc* m_Document;

public:
	EoDbGroup();
	virtual ~EoDbGroup();
	EoDbGroup(const EoDbGroup& group);
	EoDbGroup(const EoDbBlock& block);

	void AddPrimsToTreeViewControl(HWND tree, HTREEITEM parent);
	HTREEITEM AddToTreeViewControl(HWND tree, HTREEITEM parent);
	void BreakPolylines();
	void BreakSegRefs();
	void DeletePrimitivesAndRemoveAll();
	void Display(AeSysView* view, CDC* deviceContext);
	void Erase();
	POSITION FindAndRemovePrimitive(EoDbPrimitive* primitive);
	EoDbPrimitive* GetAt(POSITION position);
	int GetBlockReferenceCount(const CString& name) const;
	void GetExtents_(AeSysView* view, OdGeExtents3d& extents);
	EoDbPrimitive* GetNext(POSITION& position) const;
	EoDbPoint* GetFirstDifferentPoint(EoDbPoint* pointPrimitive);
	int GetLinetypeIndexRefCount(EoInt16 linetypeIndex);
	void InsertBefore(POSITION position, EoDbGroup* group);
	bool IsInView(AeSysView* view) const;
	/// <summary>Determines if input point lies on any primitive in the group</summary>
	bool IsOn(const EoGePoint4d& point, AeSysView* view) const;
	/// <summary>Checks if this group is persistent</summary>
	/// <returns>true if the group will be appended to the database and saved to file, false if group is temporary ie used in dynamic previews</returns>
	bool IsPersistent() const;
	void ModifyNotes(EoDbFontDefinition& fontDefinition, EoDbCharacterCellDefinition& characterCellDefinition, int iAtt = 0);
	void ModifyColorIndex(EoInt16 colorIndex);
	void ModifyLinetypeIndex(EoInt16 linetypeIndex);
	void PenTranslation(EoUInt16, EoInt16*, EoInt16*);
	void RemoveDuplicatePrimitives();
	int RemoveEmptyNotesAndDelete();
	/// <summary>Picks a prim if close enough to point.  Working in view space.</summary>
	EoDbPrimitive* SelPrimUsingPoint(const EoGePoint4d& point, AeSysView* view, double&, OdGePoint3d&);
	/// <summary>Determines if the line crosses any primitive in the group</summary>
	bool SelectBy(const EoGeLineSeg3d& line, AeSysView* view) const;
	bool SelectBy(const OdGePoint3d& pt1, const OdGePoint3d& pt2, AeSysView* view) const;
	EoDbPrimitive* SelectControlPointBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d*);
	void SortTextOnY();
	void Square(AeSysView* view);
	void TransformBy(const EoGeMatrix3d& transformMatrix);
	void UndoErase();
	void Write(EoDbFile& file);
	void Write(CFile& file, EoByte* buffer);

public: // static methods
	static void SetPrimitiveToIgnore(EoDbPrimitive* primitive);
};
