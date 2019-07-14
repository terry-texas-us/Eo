#pragma once
#include <Gs/GsViewLocalId.h>
#include <ExTrackers.h>

class RTZoomTracker : public OdEdPointTracker {
	OdGsView* m_View {nullptr};
	double m_Base {0.0};
	double m_FieldWidth {0.0};
	double m_FieldHeight {0.0};
public:
	void Initialize(OdGsView* view, const OdGePoint3d& base) {
		m_View = view;
		m_FieldWidth = view->fieldWidth();
		m_FieldHeight = view->fieldHeight();
		m_Base = (m_View->projectionMatrix() * m_View->viewingMatrix() * base).y;
	}

	void setValue(const OdGePoint3d& value) override {
		const auto WorldToNdcTransform {m_View->projectionMatrix() * m_View->viewingMatrix()};
		const auto ndcPoint {WorldToNdcTransform * value};
		auto FieldScaleFactor {1.0 + fabs(ndcPoint.y - m_Base) * 1.5};
		if (ndcPoint.y > m_Base) {
			FieldScaleFactor = 1.0 / FieldScaleFactor;
		}
		m_View->setView(m_View->position(), m_View->target(), m_View->upVector(), m_FieldWidth * FieldScaleFactor, m_FieldHeight * FieldScaleFactor, m_View->isPerspective() ? OdGsView::kPerspective : OdGsView::kParallel);
	}

	int addDrawables(OdGsView* /*view*/) noexcept override {
		return 1;
	}

	void removeDrawables(OdGsView* /*view*/) noexcept override { }
};
