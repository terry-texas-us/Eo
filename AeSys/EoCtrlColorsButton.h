#pragma once

// EoCtrlColorsButton
class EoCtrlColorsButton : public CMFCButton {
DECLARE_DYNAMIC(EoCtrlColorsButton)
	static gsl::span<COLORREF> m_Palette;
	static unsigned short m_CurrentIndex;
	static unsigned short m_SelectedIndex;

	enum Layouts { SimpleSingleRow, GridDown5RowsOddOnly, GridUp5RowsEvenOnly };

	Layouts m_Layout {SimpleSingleRow};
	CSize m_CellSize {8, 8};
	CSize m_CellSpacing {1, 1};
	CSize m_Margins {3, 3};
	unsigned short m_BeginIndex {1};
	unsigned short m_EndIndex {1};
	unsigned short m_SubItem {0};
	void DrawCell(CDC* deviceContext, unsigned short index, COLORREF color);
	unsigned short SubItemByPoint(const CPoint& point) noexcept;
	void SubItemRectangleByIndex(unsigned short index, CRect& rectangle) noexcept;
	EoCtrlColorsButton() = default;
	~EoCtrlColorsButton() = default;

	static void SetCurrentIndex(const unsigned short index) noexcept {
		m_CurrentIndex = index;
	}

	static void SetPalette(const gsl::span<COLORREF> palette) noexcept {
		m_Palette = palette;
	}

	void SetLayout(const Layouts layout, const CSize& cellSize) noexcept {
		m_Layout = layout;
		m_CellSize = cellSize;
	}

	void SetSequenceRange(const unsigned short beginIndex, const unsigned short endIndex) noexcept {
		m_BeginIndex = beginIndex;
		m_EndIndex = endIndex;
	}

	void OnDraw(CDC* deviceContext, const CRect& rectangle, unsigned state) override;
	CSize SizeToContent(BOOL calculateOnly = FALSE) override;
	unsigned OnGetDlgCode() noexcept;
	void OnKeyDown(unsigned keyCode, unsigned repeatCount, unsigned flags); // hides non-virtual function of parent
	void OnLButtonUp(unsigned flags, CPoint point); // hides non-virtual function of parent
	void OnMouseMove(unsigned flags, CPoint point); // hides non-virtual function of parent
	void OnPaint();
	void OnSetFocus(CWnd* oldWindow);
protected:
DECLARE_MESSAGE_MAP()
};
