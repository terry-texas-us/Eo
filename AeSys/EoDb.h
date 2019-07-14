#pragma once

//class EoDbPrimitive;
namespace EoDb {
	enum FileTypes {
		kDwg = 0x00, kDxf = 0x01, kDxb = 0x02, kPeg = 0x20, kJob = 0x21, kTracing = 0x22, kUnknown = -1
	};

	enum PrimitiveTypes {
		kPointPrimitive = 0x0100, kInsertPrimitive = 0x0101, kGroupReferencePrimitive = 0x0102, kLinePrimitive = 0x0200, kHatchPrimitive = 0x0400, kEllipsePrimitive = 0x1003, kSplinePrimitive = 0x2000, kCSplinePrimitive = 0x2001, kPolylinePrimitive = 0x2002, kTextPrimitive = 0x4000, kTagPrimitive = 0x4100, kDimensionPrimitive = 0x4200
	};

	enum UpdateViewHints {
		kPrimitive = 0x0001, kGroup = 0x0002, kGroups = 0x0004, kLayer = 0x0008, kSafe = 0x0100, kPrimitiveSafe = kPrimitive | kSafe, kGroupSafe = kGroup | kSafe, kGroupsSafe = kGroups | kSafe, kLayerSafe = kLayer | kSafe, kErase = 0x0200, kLayerErase = kLayer | kErase, kPrimitiveEraseSafe = kPrimitive | kErase | kSafe, kGroupEraseSafe = kGroup | kErase | kSafe, kTrap = 0x0400, kGroupsTrap = kGroups | kTrap, kGroupSafeTrap = kGroup | kSafe | kTrap, kGroupsSafeTrap = kGroups | kSafe | kTrap, kGroupEraseSafeTrap = kGroup | kErase | kSafe | kTrap, kGroupsEraseSafeTrap = kGroups | kErase | kSafe | kTrap
	};

	enum Path : unsigned { kPathRight, kPathLeft, kPathUp, kPathDown };

	enum HorizontalAlignment : unsigned { kAlignLeft = 1, kAlignCenter, kAlignRight };

	enum VerticalAlignment : unsigned { kAlignTop = 2, kAlignMiddle, kAlignBottom };

	enum Precision : unsigned { kTrueType = 1, kStrokeType };
}
