#pragma once

#define btest(m, p) ((bool) (((((unsigned long) m) >> ((int) p)) & 1UL) == 1 ? true : false))

class EoGeUniquePoint : public CObject {
public:
	OdGePoint3d	m_Point;
	int m_References;

public:
	EoGeUniquePoint() noexcept {
		m_References = 0;
		m_Point = OdGePoint3d::kOrigin;
	}
	EoGeUniquePoint(int references, const OdGePoint3d& point) {
		m_References = references;
		m_Point = point;
	}
};
