#include "stdafx.h"
#include "EoGripperScrollBar.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#pragma warning(push)
#pragma warning(disable : 4191) // (level 3) 'operator': unsafe conversion from 'type_of_expression' to 'type_required'
BEGIN_MESSAGE_MAP(EoGripperScrollBar, CScrollBar)
		ON_WM_NCHITTEST()
END_MESSAGE_MAP()
#pragma warning (pop)
LRESULT EoGripperScrollBar::OnNcHitTest(const CPoint point) {
	auto HitTest {CScrollBar::OnNcHitTest(point)};
	if (HitTest == HTCLIENT) { HitTest = HTBOTTOMRIGHT; }
	return HitTest;
}
