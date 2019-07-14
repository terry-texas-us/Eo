#pragma once
#include <Ps/plotstyles.h>
#include "EoCtrlBitmapPickerCombo.h"
class CPsListStyleData;
constexpr unsigned gc_PlotStyleColorMaxName = 25;
constexpr int gc_PlotStyleComboColorPosition = 8;
constexpr short gc_PlotStyleSpinMaxPen = 32;
constexpr short gc_PlotStyleSpinMaxVirtpen = 255;
constexpr short gc_PlotStyleSpinMaxScreening = 100;
static OdString g_PlotStylesLineTypes[] = {
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
static OdString g_PlotStylesFillStyles[] = {
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
static OdString g_PlotStylesLineEndStyles[] = {
	L"Butt",
	L"Square",
	L"Round",
	L"Diamond",
	L"Use object end style"
};
static OdString g_PlotStylesLineJoinStyles[] = {
	L"Miter",
	L"Bevel",
	L"Round",
	L"Diamond",
	L"Use object join style"
};

struct DeviceIndependentBitmapColor {
	DeviceIndependentBitmapColor(const unsigned char r, const unsigned char g, const unsigned char b) noexcept
		: m_R(r)
		, m_G(g)
		, m_B(b) { }

	explicit DeviceIndependentBitmapColor(const COLORREF color) noexcept
		: m_R(GetRValue(color))
		, m_G(GetGValue(color))
		, m_B(GetBValue(color)) { }

	operator unsigned long() noexcept {
		return *reinterpret_cast<unsigned long*>(this);
	}

private:
	unsigned char m_R;
	unsigned char m_G;
	unsigned char m_B;
	unsigned char m_Reserved {0};
};

class CBitmapColorInfo {
	friend CPsListStyleData;
	friend class EoDlgPlotStyleEditor_FormViewPropertyPage;
	unsigned char m_Item {0xff};
	COLORREF m_Color {0};
	CBitmap m_bitmap;
	wchar_t m_Name[gc_PlotStyleColorMaxName] {};
public:
	CBitmapColorInfo(const CBitmap* bitmap, COLORREF color, const wchar_t* name);

	CBitmapColorInfo(const CBitmap* bitmap, COLORREF color, unsigned char colorItem, int colorIndex = -1);

	CBitmapColorInfo(const wchar_t* resourceName, const wchar_t* name);

	CBitmap* CloneBitmap(const CBitmap* sourceBitmap, CBitmap* clonedBitmap);

	void PaintBitmap(CBitmap& bitmap, COLORREF color);

	bool IsColor(COLORREF color, unsigned char item) const noexcept;

	const OdCmEntityColor GetColor() const;

protected:
	void SetBitmapPixels(CBitmap& bitmap, DeviceIndependentBitmapColor* pixels);

	DeviceIndependentBitmapColor* GetBitmapPixels(CBitmap& bitmap, int& width, int& height);

	void GetBitmapSizes(CBitmap& bitmap, int& width, int& height);
};

using OdBitmapColorInfoArray = OdArray<CBitmapColorInfo*>;

class CPsListStyleData {
	OdPsPlotStyle* m_PlotStyles;
	OdBitmapColorInfoArray* m_PublicBitmapList;
	CBitmapColorInfo* m_BitmapColorInfo;
	int m_ActiveListIndex;
protected:
	int GetPublicArrayIndexByColor(COLORREF color) const;

public:
	CPsListStyleData(OdPsPlotStyle* plotStyle, OdBitmapColorInfoArray* publicBitmapList, char item);

	~CPsListStyleData();

	[[nodiscard]] OdPsPlotStyle* GetOdPsPlotStyle() const noexcept {
		return m_PlotStyles;
	}

	[[nodiscard]] CBitmapColorInfo* GetBitmapColorInfo() const noexcept {
		return m_BitmapColorInfo;
	}

	[[nodiscard]] int GetActiveListIndex() const noexcept {
		return m_ActiveListIndex;
	}

	bool ReplaceBitmapColorInfo(COLORREF color, int item);

	bool SetActiveListIndex(int index, bool bitmapInfo = false);

	const OdCmEntityColor GetColor() const;

	OdPsPlotStyle* GetOdPsPlotStyle() noexcept {
		return m_PlotStyles;
	}
};

class EoDlgPlotStyleEditor_FormViewPropertyPage final : public CPropertyPage {
DECLARE_DYNCREATE(EoDlgPlotStyleEditor_FormViewPropertyPage)

	void mtHideHelpBtn();

	CImageList m_imageList;
	OdPsPlotStyleTable* m_pPlotStyleTable {nullptr};
	OdPsPlotStyle* m_pPlotStyleActive {nullptr};
	OdBitmapColorInfoArray m_bitmapList;
	OdString m_sFileBufPath;
	bool m_bEditChanging {false};

	EoDlgPlotStyleEditor_FormViewPropertyPage();

	~EoDlgPlotStyleEditor_FormViewPropertyPage() = default;

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
	void DoDataExchange(CDataExchange* dataExchange) final;

	BOOL OnInitDialog() final;

	void InitializeBitmapList();

	void InitializeAdaptiveComboBox();

	void InitializeGrayscaleComboBox();

	void InitializeDitherComboBox();

	void InitializeLinetypeComboBox();

	void InitializeLineweightComboBox();

	void InitializeLineendstyleComboBox();

	void InitializeLinejoinstyleComboBox();

	void InitializeFillstyleComboBox();

	void InitializeColorComboBox();

	void InitializeListCtrl();

	int InsertItem(int index);

	HICON InitializeColorIcon(int width, int height, COLORREF color) noexcept;

	void InitializeImageList();

	int DeleteCustomColor();

	int AppendCustomColor(int item);

	int ReplaceCustomColor(COLORREF color, int item);

public:
	bool SetPlotStyleTable(OdPsPlotStyleTable* pPlotStyleTable) noexcept;

	BOOL DoPromptFileName(CString& fileName, unsigned nIDSTitle, unsigned long flags) const;

	void SetFileBufPath(OdString filePath);

	void AddNewPlotStyle(const wchar_t* styleName);

	[[nodiscard]] const OdPsPlotStyleTable* GetPlotStyleTable() const noexcept {
		return m_pPlotStyleTable;
	}

protected:
	void OnLineweightBtn();

	void OnAddBtnStyle();

	void OnSaveBtn();

	void OnDelBtnStyle();

	void OnUpdateEditDescription() noexcept;

	void OnChangeEditDescription();

	void OnChangeEditPen();

	void OnChangeEditVirtualPen();

	void OnChangeEditScreening();

	void OnItemChangedListStyles(NMHDR* notifyStructure, LRESULT* result);

	void OnItemChangingListStyles(NMHDR* notifyStructure, LRESULT* result);

	void OnDeltaPositionSpinPen(NMHDR* notifyStructure, LRESULT* result) noexcept;

	void OnSelectionChangeComboColor();

	void OnSelectionEndOkComboColor() noexcept;

	void OnSelectionEndOkComboDither();

	void OnSelectionEndOkComboGrayScale();

	void OnSelectionEndOkComboLineType();

	void OnSelectionEndOkComboAdaptive();

	void OnSelectionEndOkComboLineWeight();

	void OnSelectionEndOkComboLineEndStyle();

	void OnSelectionEndOkComboLineJoinStyle();

	void OnSelectionEndOkComboFillStyle();

	void OnDestroy(); // hides non-virtual function of parent
DECLARE_MESSAGE_MAP()
};
