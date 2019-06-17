#pragma once

#include <vector>

#include "EoCtrlBitmapPickerCombo.h"

#include "Ps/PlotStyles.h"

#define  PS_COLOR_MAX_NAME 25
#define  PS_COMBO_COLOR_POSITION 8
#define  PS_SPIN_MAX_PEN 32
#define  PS_SPIN_MAX_VIRTPEN 255
#define  PS_SPIN_MAX_SCREENING 100

#define MMTOINCH(mm) (double(mm)  / kMmPerInch)
#define INCHTOMM(inch) (double(inch) * kMmPerInch)

// plotstyle's linetype
static OdString StringLineType[] = {
	L"Solid",
	L"Dashed",
	L"Dotted",
	L"Dash Dot",
	L"Short Dash",
	L"Medium Dash",
	L"Long Dash",
	L"Short Dash X2",
	L"Medium Dash X2",
	L"Long Dash X2",
	L"Medium Long Dash",
	L"Medium Dash Short Dash Short Dash",
	L"Long Dash Short Dash",
	L"Long Dash Dot Dot",
	L"Long Dash Dot",
	L"Medium Dash Dot Short Dash Dot",
	L"Sparse Dot",
	L"ISO Dash",
	L"ISO Dash Space",
	L"ISO Long Dash Dot",
	L"ISO Long Dash Double Dot",
	L"ISO Long Dash Triple Dot",
	L"ISO Dot",
	L"ISO Long Dash Short Dash",
	L"ISO Long Dash Double Short Dash",
	L"ISO Dash Dot",
	L"ISO Double Dash Dot",
	L"ISO Dash Double Dot",
	L"ISO Double Dash Double Dot",
	L"ISO Dash Triple Dot",
	L"ISO Double Dash Triple Dot",
	L"Use object linetype"
};
static OdString StringFillStyle[] = {
	L"Solid",
	L"Checkerboard",
	L"Crosshatch",
	L"Diamonds",
	L"HorizontalBars",
	L"SlantLeft",
	L"SlantRight",
	L"SquareDots",
	L"VerticalBars",
	L"Use object fill style"
};
static OdString StringLineEndStyle[] = {
	L"Butt",
	L"Square",
	L"Round",
	L"Diamond",
	L"Use object end style"
};
static OdString StringLineJoinStyle[] = {
	L"Miter",
	L"Bevel",
	L"Round",
	L"Diamond",
	L"Use object join style"
};


struct DIBCOLOR {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char reserved;
	DIBCOLOR(unsigned char ar, unsigned char ag, unsigned char ab)  noexcept
		: r(ar)
		, g(ag)
		, b(ab)
		, reserved(0) {}
	DIBCOLOR(COLORREF color) noexcept
		: r(GetRValue(color))
		, g(GetGValue(color))
		, b(GetBValue(color))
		, reserved(0) {}
	operator unsigned long() noexcept {
		return *reinterpret_cast<unsigned long*>(this);
	}
};

class CBitmapColorInfo {
public:
	unsigned char m_iItem;
	COLORREF m_color;
	CBitmap m_bitmap;
	wchar_t m_name[PS_COLOR_MAX_NAME];

	CBitmapColorInfo(const CBitmap* bitmap, COLORREF color, const wchar_t* name);
	CBitmapColorInfo(const CBitmap* bitmap, COLORREF color, unsigned char colorItem, int colorIndex = -1);
	CBitmapColorInfo(const wchar_t* resourceName, const wchar_t* name);

protected:
	void SetBitmapPixels(CBitmap& bitmap, DIBCOLOR* pixels);
	DIBCOLOR* GetBitmapPixels(CBitmap& bitmap, int& width, int& height);
	void GetBitmapSizes(CBitmap& bitmap, int& width, int& height);

public:
	CBitmap* CloneBitmap(const CBitmap* sourceBitmap, CBitmap* clonedBitmap);
	void PaintBitmap(CBitmap& bitmap, COLORREF color);
	bool IsColor(COLORREF color, unsigned char item) noexcept;
	const OdCmEntityColor GetColor();
};

using OdBitmapColorInfoArray = OdArray<CBitmapColorInfo*>;

class CPsListStyleData {
	OdPsPlotStyle* m_pPlotStyles;
	OdBitmapColorInfoArray* m_pPublicBitmapList;
	CBitmapColorInfo* m_pBitmapColorInfo;
	int m_iActiveListIndex;

protected:
	int getPublicArrayIndexByColor(COLORREF color);

public:
	CPsListStyleData(OdPsPlotStyle* plotStyle, OdBitmapColorInfoArray* publicBitmapList, const char item);

	~CPsListStyleData();

	OdPsPlotStyle* GetOdPsPlotStyle() const noexcept { return m_pPlotStyles; }
	CBitmapColorInfo* GetBitmapColorInfo() const noexcept { return m_pBitmapColorInfo; }
	int GetActiveListIndex() const noexcept { return m_iActiveListIndex; }
	bool ReplaceBitmapColorInfo(COLORREF color, const int item);
	bool SetActiveListIndex(const int index, const bool bBmpInfo = false);
	const OdCmEntityColor GetColor();
	OdPsPlotStyle* GetOdPsPlotStyle() noexcept { return m_pPlotStyles; }
};

class EoDlgPlotStyleEditor_FormViewPropertyPage : public CPropertyPage {
	DECLARE_DYNCREATE(EoDlgPlotStyleEditor_FormViewPropertyPage)

	void mtHideHelpBtn();

	CImageList m_imageList;
	OdPsPlotStyleTable* m_pPlotStyleTable;
	OdPsPlotStyle* m_pPlotStyleActive;
	OdBitmapColorInfoArray m_bitmapList;
	OdString m_sFileBufPath;
	bool m_bEditChanging;

public:
	EoDlgPlotStyleEditor_FormViewPropertyPage();
	~EoDlgPlotStyleEditor_FormViewPropertyPage();

	enum { IDD = IDD_PLOTSTYLE_FORMVIEW_PROPERTY_PAGE };
	CComboBox m_Dither;
	CComboBox m_Grayscale;
	CComboBox m_Linetype;
	CComboBox m_Lineweight;
	CComboBox m_Lineendstyle;
	CComboBox m_Linejoinstyle;
	CComboBox m_Fillstyle;
	EoCtrlBitmapPickerCombo m_Color;
	CEdit m_editDescription;
	CEdit m_editPen;
	CEdit m_editVirtpen;
	CEdit m_editScreening;
	CComboBox m_Adaptive;
	CSpinButtonCtrl m_spinPen;
	CSpinButtonCtrl m_spinVirtpen;
	CSpinButtonCtrl m_spinScreening;
	CListCtrl m_listStyles;
	CButton m_AddstyleButton;
	CButton m_DelstyleButton;
	CButton m_LineweightButton;
	CButton m_SaveButton;

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;

protected:
	void initBitmapList();
	void initAdaptiveComboBox();
	void initGrayscaleComboBox();
	void initDitherComboBox();
	void initLinetypeComboBox();
	void initLineweightComboBox();
	void initLineendstyleComboBox();
	void initLinejoinstyleComboBox();
	void initFillstyleComboBox();
	void initColorComboBox();
	void initListCtrl();
	const int insertItem(int index);
	HICON initColorIcon(int width, int height, COLORREF color) noexcept;
	void initImageList();

	int deleteCustomColor();
	int appendCustomColor(const int item);
	int replaceCustomColor(COLORREF color, const int item);

public:
	bool SetPlotStyleTable(OdPsPlotStyleTable* pPlotStyleTable) noexcept;
	BOOL DoPromptFileName(CString& fileName, unsigned nIDSTitle, unsigned long flags);
	void SetFileBufPath(const OdString filePath);
	void AddNewPlotStyle(const wchar_t* styleName);
	const OdPsPlotStyleTable* GetPlotStyleTable() const noexcept { return m_pPlotStyleTable; }

protected:
	void OnLineweightBtn();
	void OnAddBtnStyle();
	void OnSaveBtn();
	void OnDelBtnStyle();
	void OnUpdateEditDescription() noexcept;
	void OnChangeEditDescription();
	void OnChangeEditPen();
	void OnChangeEditVirtPen();
	void OnChangeEditScreening();
	void OnItemchangedListStyles(NMHDR* notifyStructure, LRESULT* result);
	void OnItemchangingListStyles(NMHDR* notifyStructure, LRESULT* result);
	void OnDeltaposSpinPen(NMHDR* notifyStructure, LRESULT* result) noexcept;
	void OnSelchangeComboColor();
	void OnSelendokComboColor() noexcept;
	void OnSelendokComboDither();
	void OnSelendokComboGrayScale();
	void OnSelendokComboLineType();
	void OnSelendokComboAdaptive();
	void OnSelendokComboLineWeight();
	void OnSelendokComboLineEndStyle();
	void OnSelendokComboLineJoinStyle();
	void OnSelendokComboFillStyle();
	void OnDestroy();

	DECLARE_MESSAGE_MAP()
};
