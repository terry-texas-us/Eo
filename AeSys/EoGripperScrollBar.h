#pragma once

class EoGripperScrollBar : public CScrollBar {
public:
	EoGripperScrollBar() noexcept {
	};

public:
	virtual ~EoGripperScrollBar() {
	};

protected:
	afx_msg LRESULT OnNcHitTest(CPoint point);

	DECLARE_MESSAGE_MAP()
};
