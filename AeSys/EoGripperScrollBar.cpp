#include "stdafx.h"
#include "EoGripperScrollBar.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
BEGIN_MESSAGE_MAP(EoGripperScrollBar, CScrollBar)
		ON_WM_NCHITTEST()
END_MESSAGE_MAP()

LRESULT EoGripperScrollBar::OnNcHitTest(const CPoint point) {
	auto HitTest {CScrollBar::OnNcHitTest(point)};
	if (HitTest == HTCLIENT) { HitTest = HTBOTTOMRIGHT; }
	return HitTest;
}
