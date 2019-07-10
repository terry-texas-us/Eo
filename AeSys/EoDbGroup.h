#pragma once
#include <DbGroup.h>
#include "EoDbFontDefinition.h"
#include "EoDbPoint.h"
using SelectionPair = std::pair<EoDbGroup*, EoDbPrimitive*>;

enum GroupPrimitivePair { tGroup, tPrimitive };

class AeSysDoc;
class EoDbBlock;
class EoGeLineSeg3d;

class EoDbGroup : public CObList {
	static EoDbPrimitive* sm_PrimitiveToIgnore;
public:
	EoDbGroup() = default;

	~EoDbGroup() = default;

	EoDbGroup(const EoDbGroup& group);

	EoDbGroup(const EoDbBlock& block);

	void AddPrimitivesToTreeViewControl(HWND tree, HTREEITEM parent) const;

	HTREEITEM AddToTreeViewControl(HWND tree, HTREEITEM parent);

	void BreakPolylines();

	void BreakSegRefs();

	void DeletePrimitivesAndRemoveAll();

	void Display(AeSysView* view, CDC* deviceContext) const;

	void Erase() const;

	POSITION FindAndRemovePrimitive(EoDbPrimitive* primitive);

	EoDbPrimitive* GetAt(POSITION position); // hides non-virtual function of parent
	[[nodiscard]] int GetBlockReferenceCount(const CString& name) const;

	void GetExtents_(AeSysView* view, OdGeExtents3d& extents) const;

	EoDbPrimitive* GetNext(POSITION& position) const; // hides non-virtual function of parent
	EoDbPoint* GetFirstDifferentPoint(EoDbPoint* pointPrimitive) const;

	int GetLinetypeIndexRefCount(short linetypeIndex) const;

	void InsertBefore(POSITION position, EoDbGroup* group); // hides non-virtual function of parent
	bool IsInView(AeSysView* view) const;
	/// <summary>Determines if input point lies on any primitive in the group</summary>
	bool IsOn(const EoGePoint4d& point, AeSysView* view) const;
	/// <summary>Checks if this group is persistent</summary>
	/// <returns>true if the group will be appended to the database and saved to file, false if group is temporary ie used in dynamic previews</returns>
	void ModifyNotes(const EoDbFontDefinition& fontDefinition, EoDbCharacterCellDefinition& characterCellDefinition, int iAtt = 0) const;

	void ModifyColorIndex(short colorIndex) const;

	void ModifyLinetypeIndex(short linetypeIndex) const;

	void PenTranslation(unsigned numberOfColors, std::vector<int>& newColors, std::vector<int>& pCol) const;

	void RemoveDuplicatePrimitives();

	int RemoveEmptyNotesAndDelete();
	/// <summary>Picks a prim if close enough to point.  Working in view space.</summary>
	EoDbPrimitive* SelectPrimitiveUsingPoint(const EoGePoint4d& point, AeSysView* view, double& pickAperture, OdGePoint3d&) const;
	/// <summary>Determines if the line crosses any primitive in the group</summary>
	bool SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view) const;

	bool SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const;

	EoDbPrimitive* SelectControlPointBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d*) const;

	void SortTextOnY();

	void Square(AeSysView* view) const;

	void TransformBy(const EoGeMatrix3d& transformMatrix) const;

	void UndoErase() const;

	void Write(EoDbFile& file) const;

	void Write(CFile& file, unsigned char* buffer) const;

	static void SetPrimitiveToIgnore(EoDbPrimitive* primitive) noexcept;

	static std::pair<EoDbGroup*, OdDbGroupPtr> Create(OdDbDatabasePtr database);
};
