#pragma once

class EoGripperScrollBar : public CScrollBar {
public:
	EoGripperScrollBar() noexcept {};

public:
	~EoGripperScrollBar() = default;

protected:
	LRESULT OnNcHitTest(CPoint point);

	DECLARE_MESSAGE_MAP()
};
