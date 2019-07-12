#pragma once
#include <RxVariantValue.h>

class ViewInteractivityMode {
	bool m_Enabled;
	OdGsView* m_View;
public:
	ViewInteractivityMode(OdRxVariantValue enable, OdRxVariantValue frameRate, OdGsView* view) {
		m_Enabled = false;
		m_View = view;
		if (!enable.isNull()) {
			m_Enabled = static_cast<bool>(enable);
			if (m_Enabled && !frameRate.isNull()) {
				const auto Rate {frameRate.get()->getDouble()};
				view->beginInteractivity(Rate);
			}
		}
	}

	~ViewInteractivityMode() {
		if (m_Enabled) { m_View->endInteractivity(); }
	}
};
