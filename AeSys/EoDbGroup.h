#pragma once

#include "EoDbFontDefinition.h"
#include "EoDbPoint.h"
#include "DbGroup.h"

using namespace std;
typedef pair<EoDbGroup*, EoDbPrimitive*> SelectionPair;
enum GroupPrimitvePair { tGroup, tPrimitive };

class AeSysDoc;
class EoDbBlock;
class EoGeLineSeg3d;

class EoDbGroup: public CObList {
	static EoDbPrimitive* sm_PrimitiveToIgnore;

public:
	EoDbGroup() noexcept;
	~EoDbGroup();
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
	int GetLinetypeIndexRefCount(short linetypeIndex);
	void InsertBefore(POSITION position, EoDbGroup* group);
	bool IsInView(AeSysView* view) const;
	/// <summary>Determines if input point lies on any primitive in the group</summary>
	bool IsOn(const EoGePoint4d& point, AeSysView* view) const;
	/// <summary>Checks if this group is persistent</summary>
	/// <returns>true if the group will be appended to the database and saved to file, false if group is temporary ie used in dynamic previews</returns>
	void ModifyNotes(EoDbFontDefinition& fontDefinition, EoDbCharacterCellDefinition& characterCellDefinition, int iAtt = 0);
	void ModifyColorIndex(short colorIndex);
	void ModifyLinetypeIndex(short linetypeIndex);
	void PenTranslation(unsigned short, short*, short*);
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
	void Write(CFile& file, unsigned char* buffer);

public: // Methods - static
	static void SetPrimitiveToIgnore(EoDbPrimitive* primitive) noexcept;

    static pair<EoDbGroup*, OdDbGroupPtr> EoDbGroup::Create(OdDbDatabasePtr database);
};
