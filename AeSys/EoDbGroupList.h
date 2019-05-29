#pragma once

class EoDbGroupList : public CObList {
public: // Constructors and destructor
	EoDbGroupList() noexcept {
	}
	~EoDbGroupList() {
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
	int GetLinetypeIndexRefCount(short linetypeIndex);
	void ModifyNotes(EoDbFontDefinition& cd, EoDbCharacterCellDefinition& characterCellDefinition, int iAtt = 0);
	void ModifyColorIndex(short colorIndex);
	void ModifyLinetypeIndex(short linetypeIndex);
	void PenTranslation(unsigned short, short*, short*);
	void RemoveDuplicatePrimitives();
	int RemoveEmptyNotesAndDelete();
	int RemoveEmptyGroups();
	POSITION Remove(EoDbGroup* group);
	EoDbGroup* SelectGroupBy(const OdGePoint3d& point);
	void TransformBy(const EoGeMatrix3d& transformMatrix);
	void Write(CFile& file, unsigned char* buffer);
};
