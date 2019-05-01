#pragma once

#include "EoDbGroup.h"

class EoDbBlock : public EoDbGroup {
private:

	OdUInt16 m_wBlkTypFlgs;		// block type flag values
									//		b0 set - anonymous block
									//		b1 set - block has attribute definitions
									//		b2 set - block is an external reference
									//		b3 set - not used
									//		b4 set - block is externally dependent
									//		b5 set - block is a resolved external reference
									//		b6 set - definition is referenced
	OdGePoint3d m_BasePoint;		// block base point
	OdString m_strXRefPathName; 	// external reference (XRef) path name

public:
	EoDbBlock();
	EoDbBlock(OdUInt16 flags, const OdGePoint3d& basePoint);
	EoDbBlock(OdUInt16 flags, const OdGePoint3d& basePoint, const OdString& pathName);

	OdGePoint3d	BasePoint() const noexcept;
	OdUInt16 GetBlkTypFlgs() noexcept;
	bool HasAttributes() noexcept;
	bool IsAnonymous() noexcept;
	bool IsFromExternalReference() noexcept;
	void SetBlkTypFlgs(OdUInt16 flags) noexcept;
	void SetBasePoint(const OdGePoint3d& basePoint) noexcept;
};

typedef CTypedPtrMap<CMapStringToOb, CString, EoDbBlock*> EoDbBlockTable;