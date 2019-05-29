#pragma once

#pragma warning (disable: 4786)
#include <list>

class EoDialogResizeHelper {
public:

	enum EHFix { // fix horizontal dimension/position
		kNoHFix = 0,
		kWidth = 1,
		kLeft = 2,
		kRight = 4,
		kWidthLeft = 3,
		kWidthRight = 5,
		kLeftRight = 6
	};
	enum EVFix { // fix vertical dimension/position
		kNoVFix = 0,
		kHeight = 1,
		kTop = 2,
		kBottom = 4,
		kHeightTop = 3,
		kHeightBottom = 5,
		kTopBottom = 6
	};
	// initialize with parent window, all child windows must already have their original position/size
	void Init(HWND a_hParent);

	// explicitly add a window to the list of child windows (e.g. a sibling window)
	// Note: you've got to call Init() before you can add a window
	void Add(HWND a_hWnd);

	// fix position/dimension for a child window, determine child by...
	// ...HWND...
	BOOL Fix(HWND a_hCtrl, EHFix a_hFix, EVFix a_vFix);
	// ...item ID (if it's a dialog item)...
	BOOL Fix(int a_itemId, EHFix a_hFix, EVFix a_vFix);
	// ...all child windows with a common class name (e.g. "Edit")
	UINT Fix(LPCWSTR a_pszClassName, EHFix a_hFix, EVFix a_vFix);
	// ...or all registered windows
	BOOL Fix(EHFix a_hFix, EVFix a_vFix);

	// resize child windows according to changes of parent window and fix attributes
	void OnSize();
private:
	struct CtrlSize {
		CRect m_origSize;
		HWND  m_hCtrl;
		EHFix  m_hFix;
		EVFix  m_vFix;
		CtrlSize() noexcept
			: m_hCtrl(nullptr)
			, m_hFix(kNoHFix)
			, m_vFix(kNoVFix) {
		}
	};
	typedef std::list<CtrlSize> CtrlCont_t;
	CtrlCont_t m_ctrls;
	HWND       m_hParent;
	CRect      m_origParentSize;
};
