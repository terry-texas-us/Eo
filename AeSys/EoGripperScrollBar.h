#pragma once

class EoGripperScrollBar : public CScrollBar {
public:
	EoGripperScrollBar() noexcept {};

public:
	~EoGripperScrollBar() {};

protected:
	LRESULT OnNcHitTest(CPoint point);

	DECLARE_MESSAGE_MAP()
};
