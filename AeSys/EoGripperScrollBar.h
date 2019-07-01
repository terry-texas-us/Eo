#pragma once
class EoGripperScrollBar : public CScrollBar {
public:
	EoGripperScrollBar() = default;
	~EoGripperScrollBar() = default;
protected:
	LRESULT OnNcHitTest(CPoint point); // hides non-virtual function of parent
DECLARE_MESSAGE_MAP()
};
