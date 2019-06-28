#pragma once
class EoDbGroupList : public CObList {
public:
	EoDbGroupList() = default;
	~EoDbGroupList() = default;

	// Base class wrappers hides non-virtual function of parent
	POSITION AddHead(EoDbGroup* group) {
		return CObList::AddHead(dynamic_cast<CObject*>(group));
	}

	POSITION AddTail(EoDbGroup* group) {
		return CObList::AddTail(dynamic_cast<CObject*>(group));
	}

	void AddTail(EoDbGroupList* groupList) {
		CObList::AddTail(dynamic_cast<CObList*>(groupList));
	}

	EoDbGroup* GetNext(POSITION& position) {
		return dynamic_cast<EoDbGroup*>(CObList::GetNext(position));
	} // hides non-virtual function of parent

	EoDbGroup* GetPrev(POSITION& position) {
		return dynamic_cast<EoDbGroup*>(CObList::GetPrev(position));
	} // hides non-virtual function of parent

	EoDbGroup* RemoveHead() {
		return dynamic_cast<EoDbGroup*>(CObList::RemoveHead());
	} // hides non-virtual function of parent

	EoDbGroup* RemoveTail() {
		return dynamic_cast<EoDbGroup*>(CObList::RemoveTail());
	} // hides non-virtual function of parent

	void AddToTreeViewControl(HWND tree, HTREEITEM htiParent);
	void BreakPolylines();
	void BreakSegRefs();
	void DeleteGroupsAndRemoveAll();
	void Display(AeSysView* view, CDC* deviceContext);
	int GetBlockReferenceCount(const CString& name);
	/// <summary>Determines the extent of all groups in list.</summary>
	void GetExtents__(AeSysView* view, OdGeExtents3d& extents);
	int GetLinetypeIndexRefCount(short linetypeIndex);
	void ModifyNotes(EoDbFontDefinition& fontDefinition, EoDbCharacterCellDefinition& characterCellDefinition, int iAtt = 0);
	void ModifyColorIndex(short colorIndex);
	void ModifyLinetypeIndex(short linetypeIndex);
	void PenTranslation(unsigned numberOfColors, std::vector<int>& newColors, std::vector<int>& pCol);
	void RemoveDuplicatePrimitives();
	int RemoveEmptyNotesAndDelete();
	int RemoveEmptyGroups();
	POSITION Remove(EoDbGroup* group);
	EoDbGroup* SelectGroupBy(const OdGePoint3d& point);
	void TransformBy(const EoGeMatrix3d& transformMatrix);
	void Write(CFile& file, unsigned char* buffer);
};
