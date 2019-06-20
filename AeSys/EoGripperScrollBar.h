#pragma once

class EoGripperScrollBar : public CScrollBar {
public:
	EoGripperScrollBar() = default;
	~EoGripperScrollBar() = default;

protected:
	LRESULT OnNcHitTest(CPoint point);

	DECLARE_MESSAGE_MAP()
};
