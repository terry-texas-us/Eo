#pragma once

#include <vector>
using namespace std;

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
	BYTE b;
	BYTE g;
	BYTE r;
	BYTE reserved;
	DIBCOLOR(BYTE ar,BYTE ag,BYTE ab):
		r(ar), g(ag), b(ab), reserved(0) {}
	DIBCOLOR(COLORREF color):
		r(GetRValue(color)), g(GetGValue(color)), b(GetBValue(color)), reserved(0) {}
	operator DWORD() {return *reinterpret_cast<DWORD*>(this);}
};

class CBitmapColorInfo {
public:
	BYTE m_iItem;
	COLORREF m_color;
	CBitmap m_bitmap;
	wchar_t m_name[PS_COLOR_MAX_NAME];

	CBitmapColorInfo(const CBitmap *pBitmap, COLORREF color, const wchar_t* name);
	CBitmapColorInfo(const CBitmap *pBitmap, COLORREF color, BYTE cColorItem, int colorIndex = -1);
	CBitmapColorInfo(LPCWSTR lpszResourceName, const wchar_t* name);

	// Implementation
protected:
	void SetBitmapPixels(CBitmap &Bmp, DIBCOLOR *pPixels);
	DIBCOLOR *GetBitmapPixels(CBitmap &Bmp, int &W, int &H);
	void GetBitmapSizes(CBitmap &Bmp, int &W, int &H);

public:
	CBitmap* CloneBitmap(const CBitmap* pBmpSource, CBitmap* pBmpClone);
	void PaintBitmap(CBitmap &Bmp, COLORREF color);
	const bool IsColor(COLORREF color, BYTE item);
	const OdCmEntityColor GetColor();
};

typedef OdArray<CBitmapColorInfo*> OdBitmapColorInfoArray;

class CPsListStyleData {
	OdPsPlotStyle* m_pPlotStyles;
	OdBitmapColorInfoArray* m_pPublicBitmapList;
	CBitmapColorInfo* m_pBitmapColorInfo; 
	int m_iActiveListIndex;

protected:
	const int getPublicArrayIndexByColor(COLORREF color);

public:
	CPsListStyleData(OdPsPlotStyle* pPs, OdBitmapColorInfoArray* pPublicBitmapList, const char item);

	~CPsListStyleData();

	OdPsPlotStyle* GetOdPsPlotStyle() const { return m_pPlotStyles; };
	CBitmapColorInfo*  GetBitmapColorInfo() const { return m_pBitmapColorInfo; };
	const int GetActiveListIndex() const { return m_iActiveListIndex; };
	const bool ReplaceBitmapColorInfo(COLORREF color, const int item);
	const bool SetActiveListIndex(const int index, const bool bBmpInfo = false);
	const OdCmEntityColor GetColor();
	OdPsPlotStyle* GetOdPsPlotStyle(){ return m_pPlotStyles; };
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
	virtual void DoDataExchange(CDataExchange* pDX);

	// Implementation
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
	HICON initColorIcon(int width,int height, COLORREF color);
	void initImageList();

	const int deleteCustomColor();
	const int appendCustomColor(const int item);
	const int replaceCustomColor(COLORREF color, const int item);

public:
	const bool SetPlotStyleTable(OdPsPlotStyleTable* pPlotStyleTable);
	BOOL DoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags);
	void SetFileBufPath(const OdString sFilePath);
	void AddNewPlotStyle(LPCWSTR lpStyleName);
	const OdPsPlotStyleTable *GetPlotStyleTable() const { return m_pPlotStyleTable; };

protected:
	afx_msg void OnLineweightBtn();
	afx_msg void OnAddBtnStyle();
	afx_msg void OnSaveBtn();
	afx_msg void OnDelBtnStyle();
	afx_msg void OnUpdateEditDescription();
	afx_msg void OnChangeEditDescription();
	afx_msg void OnChangeEditPen();
	afx_msg void OnChangeEditVirtPen();
	afx_msg void OnChangeEditScreening();
	virtual BOOL OnInitDialog();
	afx_msg void OnItemchangedListStyles(NMHDR* pNMHDR, LRESULT* result);
	afx_msg void OnItemchangingListStyles(NMHDR* pNMHDR, LRESULT* result);
	afx_msg void OnDeltaposSpinPen(NMHDR* pNMHDR, LRESULT* result);
	afx_msg void OnSelchangeComboColor();
	afx_msg void OnSelendokComboColor();
	afx_msg void OnSelendokComboDither();
	afx_msg void OnSelendokComboGrayScale();
	afx_msg void OnSelendokComboLineType();
	afx_msg void OnSelendokComboAdaptive();
	afx_msg void OnSelendokComboLineWeight();
	afx_msg void OnSelendokComboLineEndStyle();
	afx_msg void OnSelendokComboLineJoinStyle();
	afx_msg void OnSelendokComboFillStyle();
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
};
