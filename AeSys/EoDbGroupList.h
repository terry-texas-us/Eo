#pragma once

class EoDbGroupList : public CObList {
public: // Constructors and destructor
	EoDbGroupList() noexcept {
	}
	~EoDbGroupList() {
	}
public:	// Base class wrappers hides non-virtual function of parent

	POSITION EoDbGroupList::AddHead(EoDbGroup* group) { return (CObList::AddHead((CObject*)group)); }
	POSITION EoDbGroupList::AddTail(EoDbGroup* group) { return (CObList::AddTail((CObject*)group)); }
	void EoDbGroupList::AddTail(EoDbGroupList* groupList) { CObList::AddTail((CObList*)groupList); }
	EoDbGroup* EoDbGroupList::GetNext(POSITION& position) { return (EoDbGroup*)CObList::GetNext(position); }
	EoDbGroup* EoDbGroupList::GetPrev(POSITION& position) { return (EoDbGroup*)CObList::GetPrev(position); }
	EoDbGroup* EoDbGroupList::RemoveHead() { return (EoDbGroup*)CObList::RemoveHead(); }
	EoDbGroup* EoDbGroupList::RemoveTail() { return (EoDbGroup*)CObList::RemoveTail(); }

public: // Methods
	void AddToTreeViewControl(HWND tree, HTREEITEM htiParent);
	void BreakPolylines();
	void BreakSegRefs();
	void DeleteGroupsAndRemoveAll();
	void Display(AeSysView* view, CDC* deviceContext);
	int GetBlockReferenceCount(const CString& name);
	/// <summary>Determines the extent of all groups in list.</summary>
	void GetExtents__(AeSysView* view, OdGeExtents3d& extents);
	int GetLinetypeIndexRefCount(short linetypeIndex);
	void ModifyNotes(EoDbFontDefinition& cd, EoDbCharacterCellDefinition& characterCellDefinition, int iAtt = 0);
	void ModifyColorIndex(short colorIndex);
	void ModifyLinetypeIndex(short linetypeIndex);
	void PenTranslation(unsigned numberOfColors, vector<int>& newColors, vector<int>& pCol);
	void RemoveDuplicatePrimitives();
	int RemoveEmptyNotesAndDelete();
	int RemoveEmptyGroups();
	POSITION Remove(EoDbGroup* group);
	EoDbGroup* SelectGroupBy(const OdGePoint3d& point);
	void TransformBy(const EoGeMatrix3d& transformMatrix);
	void Write(CFile& file, unsigned char* buffer);
};
