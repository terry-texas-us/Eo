#pragma once
#define btest(m, p) ((bool) (((((unsigned long) (m)) >> ((int) (p))) & 1UL) == 1 ? true : false))

class EoGeUniquePoint final : public CObject {
public:
	int m_References;
	OdGePoint3d m_Point;

	EoGeUniquePoint() noexcept {
		m_References = 0;
		m_Point = OdGePoint3d::kOrigin;
	}

	EoGeUniquePoint(const int references, const OdGePoint3d& point) {
		m_References = references;
		m_Point = point;
	}
};
