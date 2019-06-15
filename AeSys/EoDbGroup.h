#pragma once

#include "EoDbFontDefinition.h"
#include "EoDbPoint.h"
#include "DbGroup.h"

using namespace std;
using SelectionPair = pair<EoDbGroup*, EoDbPrimitive*>;
enum GroupPrimitvePair { tGroup, tPrimitive };

class AeSysDoc;
class EoDbBlock;
class EoGeLineSeg3d;

class EoDbGroup: public CObList {
	static EoDbPrimitive* sm_PrimitiveToIgnore;

public:
	EoDbGroup() = default;
	~EoDbGroup() = default;
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
	EoDbPrimitive* GetAt(POSITION position); // hides non-virtual function of parent
	int GetBlockReferenceCount(const CString& name) const;
	void GetExtents_(AeSysView* view, OdGeExtents3d& extents);
	EoDbPrimitive* GetNext(POSITION& position) const; // hides non-virtual function of parent
	EoDbPoint* GetFirstDifferentPoint(EoDbPoint* pointPrimitive);
	int GetLinetypeIndexRefCount(short linetypeIndex);
	void InsertBefore(POSITION position, EoDbGroup* group); // hides non-virtual function of parent
	bool IsInView(AeSysView* view) const;
	/// <summary>Determines if input point lies on any primitive in the group</summary>
	bool IsOn(const EoGePoint4d& point, AeSysView* view) const;
	/// <summary>Checks if this group is persistent</summary>
	/// <returns>true if the group will be appended to the database and saved to file, false if group is temporary ie used in dynamic previews</returns>
	void ModifyNotes(const EoDbFontDefinition& fontDefinition, EoDbCharacterCellDefinition& characterCellDefinition, int iAtt = 0);
	void ModifyColorIndex(short colorIndex);
	void ModifyLinetypeIndex(short linetypeIndex);
	void PenTranslation(unsigned numberOfColors, vector<int>& newColors, vector<int>& pCol);
	void RemoveDuplicatePrimitives();
	int RemoveEmptyNotesAndDelete();
	/// <summary>Picks a prim if close enough to point.  Working in view space.</summary>
	EoDbPrimitive* SelPrimUsingPoint(const EoGePoint4d& point, AeSysView* view, double&, OdGePoint3d&);
	/// <summary>Determines if the line crosses any primitive in the group</summary>
	bool SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view) const;
	bool SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const;
	EoDbPrimitive* SelectControlPointBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d*);
	void SortTextOnY();
	void Square(AeSysView* view);
	void TransformBy(const EoGeMatrix3d& transformMatrix);
	void UndoErase();
	void Write(EoDbFile& file);
	void Write(CFile& file, unsigned char* buffer);

public: // Methods - static
	static void SetPrimitiveToIgnore(EoDbPrimitive* primitive) noexcept;

    static pair<EoDbGroup*, OdDbGroupPtr> Create(OdDbDatabasePtr database);
};
