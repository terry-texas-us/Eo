#pragma once

// EoCtrlColorsButton
class EoCtrlColorsButton : public CMFCButton {
DECLARE_DYNAMIC(EoCtrlColorsButton)

	enum Layouts { kSimpleSingleRow, kGridDown5RowsOddOnly, kGridUp5RowsEvenOnly };

private:
	static gsl::span<COLORREF> m_Palette;
	static unsigned short m_CurrentIndex;
	static unsigned short m_SelectedIndex;
	Layouts m_Layout {kSimpleSingleRow};
	CSize m_CellSize {8, 8};
	CSize m_CellSpacing {1, 1};
	CSize m_Margins {3, 3};
	unsigned short m_BeginIndex {1};
	unsigned short m_EndIndex {1};
public:
	unsigned short m_SubItem {0};

	void DrawCell(CDC* deviceContext, unsigned short index, COLORREF color) const;

	unsigned short SubItemByPoint(const CPoint& point) const noexcept;

	void SubItemRectangleByIndex(unsigned short index, CRect& rectangle) const noexcept;

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

	unsigned OnGetDlgCode() noexcept; // hides non-virtual function of parent
	void OnKeyDown(unsigned keyCode, unsigned repeatCount, unsigned flags); // hides non-virtual function of parent
	void OnLButtonUp(unsigned flags, CPoint point); // hides non-virtual function of parent
	void OnMouseMove(unsigned flags, CPoint point); // hides non-virtual function of parent
	void OnPaint(); // hides non-virtual function of parent
	void OnSetFocus(CWnd* oldWindow); // hides non-virtual function of parent
DECLARE_MESSAGE_MAP()
};
