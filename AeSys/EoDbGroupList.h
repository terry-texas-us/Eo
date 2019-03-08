#pragma once

class EoDbGroupList : public CObList {
public: // Constructors and destructor
	EoDbGroupList() {
	}
	virtual ~EoDbGroupList() {
	}
public:	// Base class wrappers
	POSITION AddHead(EoDbGroup* group);
	POSITION AddTail(EoDbGroup* group);
	void AddTail(EoDbGroupList* groupList);
	EoDbGroup* GetNext(POSITION& position);
	EoDbGroup* GetPrev(POSITION& position);
	EoDbGroup* RemoveHead();
	EoDbGroup* RemoveTail();

public: // Methods
	void AddToTreeViewControl(HWND tree, HTREEITEM htiParent);
	void BreakPolylines();
	void BreakSegRefs();
	void DeleteGroupsAndRemoveAll();
	void Display(AeSysView* view, CDC* deviceContext);
	int GetBlockReferenceCount(const CString& name);
	/// <summary>Determines the extent of all groups in list.</summary>
	void GetExtents__(AeSysView* view, OdGeExtents3d& extents);
	int GetLinetypeIndexRefCount(EoInt16 linetypeIndex);
	void ModifyNotes(EoDbFontDefinition& cd, EoDbCharacterCellDefinition& characterCellDefinition, int iAtt = 0);
	void ModifyColorIndex(EoInt16 colorIndex);
	void ModifyLinetypeIndex(EoInt16 linetypeIndex);
	void PenTranslation(EoUInt16, EoInt16*, EoInt16*);
	void RemoveDuplicatePrimitives();
	int RemoveEmptyNotesAndDelete();
	int RemoveEmptyGroups();
	POSITION Remove(EoDbGroup* group);
	EoDbGroup* SelectGroupBy(const OdGePoint3d& point);
	void TransformBy(const EoGeMatrix3d& transformMatrix);
	void Write(CFile& file, EoByte* buffer);
};
