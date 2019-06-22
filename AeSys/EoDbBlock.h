#pragma once
#include "EoDbGroup.h"

class EoDbBlock : public EoDbGroup {
	unsigned short m_wBlkTypFlgs {0};	// block type flag values
	//		b0 set - anonymous block
	//		b1 set - block has attribute definitions
	//		b2 set - block is an external reference
	//		b3 set - not used
	//		b4 set - block is externally dependent
	//		b5 set - block is a resolved external reference
	//		b6 set - definition is referenced
	OdGePoint3d m_BasePoint {OdGePoint3d::kOrigin};
	OdString m_strXRefPathName; // external reference (XRef) path name
public:
	EoDbBlock() = default;
	EoDbBlock(unsigned short flags, const OdGePoint3d& basePoint);
	EoDbBlock(unsigned short flags, const OdGePoint3d& basePoint, const OdString& pathName);
	auto BasePoint() const noexcept -> OdGePoint3d;
	unsigned short GetBlkTypFlgs() noexcept;
	bool HasAttributes() noexcept;
	bool IsAnonymous() noexcept;
	bool IsFromExternalReference() noexcept;
	void SetBlkTypFlgs(unsigned short flags) noexcept;
	void SetBasePoint(const OdGePoint3d& basePoint) noexcept;
};

using EoDbBlockTable = CTypedPtrMap<CMapStringToOb, CString, EoDbBlock*>;
