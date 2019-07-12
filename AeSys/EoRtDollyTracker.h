#pragma once
#include <ExTrackers.h>

class RtDollyTracker : public OdEdPointTracker {
	OdGsView* m_View {nullptr};
	OdGePoint3d m_Point;
	OdGePoint3d m_Position;
public:
	RtDollyTracker() = default;

	void Reset() noexcept { m_View = nullptr; }

	void Initialize(OdGsView* view, const OdGePoint3d& point) {
		m_View = view;
		m_Position = view->position();
		m_Point = point - m_Position.asVector();
	}

	void setValue(const OdGePoint3d& value) override {
		if (m_View != nullptr) {
			auto Delta {(m_Point - (value - m_Position)).asVector()};
			m_Point = value - m_Position.asVector();
			Delta.transformBy(m_View->viewingMatrix());
			m_View->dolly(Delta.x, Delta.y, Delta.z);
			m_Position = m_View->position();
		}
	}

	int addDrawables(OdGsView* /*view*/) noexcept override { return 0; }

	void removeDrawables(OdGsView* /*view*/) noexcept override {
	}
};
