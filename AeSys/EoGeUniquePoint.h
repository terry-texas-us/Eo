#pragma once
#define btest(m, b) (((static_cast<unsigned>(m) >> static_cast<unsigned>(b)) & 1UL) == 1)

struct EoGeUniquePoint final : CObject {
	int mReferences {0};
	OdGePoint3d mPoint {OdGePoint3d::kOrigin};

	EoGeUniquePoint() = default;

	EoGeUniquePoint(const int references, const OdGePoint3d& point) {
		mReferences = references;
		mPoint = point;
	}
};
