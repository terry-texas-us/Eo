#include "stdafx.h"
#include "EoDialogResizeHelper.h"

void EoDialogResizeHelper::Init(HWND parent) {
	m_hParent = parent;
	m_ctrls.clear();
	if (IsWindow(m_hParent) != 0) { // keep original parent size
		GetWindowRect(m_hParent, m_origParentSize);

		// get all child windows and store their original sizes and positions
		auto hCtrl {GetTopWindow(m_hParent)};
		while (hCtrl != nullptr) {
			CtrlSize cs;
			cs.m_hCtrl = hCtrl;
			GetWindowRect(hCtrl, cs.m_origSize);
			ScreenToClient(m_hParent, &cs.m_origSize.TopLeft());
			ScreenToClient(m_hParent, &cs.m_origSize.BottomRight());
			m_ctrls.push_back(cs);
			hCtrl = GetNextWindow(hCtrl, GW_HWNDNEXT);
		}
	}
}

void EoDialogResizeHelper::Add(HWND a_hWnd) {
	if (m_hParent != nullptr && a_hWnd != nullptr) {
		CtrlSize cs;
		cs.m_hCtrl = a_hWnd;
		GetWindowRect(a_hWnd, cs.m_origSize);
		ScreenToClient(m_hParent, &cs.m_origSize.TopLeft());
		ScreenToClient(m_hParent, &cs.m_origSize.BottomRight());
		m_ctrls.push_back(cs);
	}
}

void EoDialogResizeHelper::OnSize() {
	if (IsWindow(m_hParent) != 0) {
		CRect CurrentParentSize;
		GetWindowRect(m_hParent, CurrentParentSize);
		const auto xRatio {static_cast<double>(CurrentParentSize.Width()) / m_origParentSize.Width()};
		const auto yRatio {static_cast<double>(CurrentParentSize.Height()) / m_origParentSize.Height()};
		for (CtrlCont_t::const_iterator it = m_ctrls.begin(); it != m_ctrls.end(); ++it) {
			CRect CurrentControlRectangle;
			const auto hFix {it->m_hFix};
			const auto vFix {it->m_vFix};

			// might go easier ;-)
			if (hFix & kLeft) {
				CurrentControlRectangle.left = it->m_origSize.left;
			} else {
				CurrentControlRectangle.left = hFix & kWidth && hFix & kRight ? it->m_origSize.left + CurrentParentSize.Width() - m_origParentSize.Width() : static_cast<long>(it->m_origSize.left * xRatio);
			}
			if (hFix & kRight) {
				CurrentControlRectangle.right = it->m_origSize.right + CurrentParentSize.Width() - m_origParentSize.Width();
			} else {
				CurrentControlRectangle.right = hFix & kWidth ? CurrentControlRectangle.left + it->m_origSize.Width() : static_cast<long>(it->m_origSize.right * xRatio);
			}
			if (vFix & kTop) {
				CurrentControlRectangle.top = it->m_origSize.top;
			} else {
				CurrentControlRectangle.top = vFix & kHeight && vFix & kBottom ? it->m_origSize.top + CurrentParentSize.Height() - m_origParentSize.Height() : static_cast<long>(it->m_origSize.top * yRatio);
			}
			if (vFix & kBottom) {
				CurrentControlRectangle.bottom = it->m_origSize.bottom + CurrentParentSize.Height() - m_origParentSize.Height();
			} else {
				CurrentControlRectangle.bottom = vFix & kHeight ? CurrentControlRectangle.top + it->m_origSize.Height() : static_cast<long>(it->m_origSize.bottom * yRatio);
			}
			// resize child window
			MoveWindow(it->m_hCtrl, CurrentControlRectangle.left, CurrentControlRectangle.top, CurrentControlRectangle.Width(), CurrentControlRectangle.Height(), TRUE);
		}
	}
}

BOOL EoDialogResizeHelper::Fix(HWND a_hCtrl, const EHFix a_hFix, const EVFix a_vFix) {
	for (CtrlCont_t::iterator it = m_ctrls.begin(); it != m_ctrls.end(); ++it) {
		if (it->m_hCtrl == a_hCtrl) {
			it->m_hFix = a_hFix;
			it->m_vFix = a_vFix;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL EoDialogResizeHelper::Fix(const int a_itemId, const EHFix a_hFix, const EVFix a_vFix) {
	return Fix(GetDlgItem(m_hParent, a_itemId), a_hFix, a_vFix);
}

BOOL EoDialogResizeHelper::Fix(const EHFix a_hFix, const EVFix a_vFix) {
	for (CtrlCont_t::iterator it = m_ctrls.begin(); it != m_ctrls.end(); ++it) {
		it->m_hFix = a_hFix;
		it->m_vFix = a_vFix;
	}
	return TRUE;
}

unsigned EoDialogResizeHelper::Fix(const wchar_t* a_pszClassName, const EHFix a_hFix, const EVFix a_vFix) {
	wchar_t pszCN[200];  // ToDo: size?
	unsigned cnt {0};
	for (CtrlCont_t::iterator it = m_ctrls.begin(); it != m_ctrls.end(); ++it) {
		::GetClassName(it->m_hCtrl, pszCN, sizeof pszCN);
		if (wcscmp(pszCN, a_pszClassName) == 0) {
			cnt++;
			it->m_hFix = a_hFix;
			it->m_vFix = a_vFix;
		}
	}
	return cnt;
}
