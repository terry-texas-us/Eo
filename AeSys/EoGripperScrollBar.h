#pragma once
class EoGripperScrollBar final : public CScrollBar {
protected:
	LRESULT OnNcHitTest(CPoint point); // hides non-virtual function of parent
DECLARE_MESSAGE_MAP()
};
