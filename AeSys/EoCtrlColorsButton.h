#pragma once

// EoCtrlColorsButton

class EoCtrlColorsButton : public CMFCButton {
	DECLARE_DYNAMIC(EoCtrlColorsButton)

	static COLORREF* m_Palette;
	static OdUInt16 m_CurrentIndex;
	static OdUInt16 m_SelectedIndex;

	enum Layouts {
		SimpleSingleRow,
		GridDown5RowsOddOnly,
		GridUp5RowsEvenOnly
	};
	Layouts m_Layout;
	CSize m_CellSize;
	CSize m_CellSpacing;
	CSize m_Margins;
	OdUInt16 m_BeginIndex;
	OdUInt16 m_EndIndex;
	OdUInt16 m_SubItem;

	void DrawCell(CDC* deviceContext, OdUInt16 index, COLORREF color);
	OdUInt16 SubItemByPoint(const CPoint& point) noexcept;
	void SubItemRectangleByIndex(OdUInt16 index, CRect& rectangle) noexcept;

public:

	EoCtrlColorsButton();

	virtual ~EoCtrlColorsButton();

	static void SetCurrentIndex(const OdUInt16 index) noexcept {
		m_CurrentIndex = index;
	}
	static void SetPalette(COLORREF* palette) noexcept {
		m_Palette = palette;
	}
	void SetLayout(Layouts layout, const CSize& cellSize) noexcept {
		m_Layout = layout;
		m_CellSize = cellSize;
	}
	void SetSequenceRange(const OdUInt16 beginIndex, const OdUInt16 endIndex) noexcept {
		m_BeginIndex = beginIndex;
		m_EndIndex = endIndex;
	}

	virtual void OnDraw(CDC* deviceContext, const CRect& rectangle, UINT state) override;
	virtual CSize SizeToContent(BOOL calculateOnly = FALSE) override;

	afx_msg UINT OnGetDlgCode() noexcept;
	afx_msg void OnKeyDown(UINT keyCode, UINT repeatCount, UINT flags);
	afx_msg void OnLButtonUp(UINT flags, CPoint point);
	afx_msg void OnMouseMove(UINT flags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* oldWindow);

protected:

	DECLARE_MESSAGE_MAP()
};
