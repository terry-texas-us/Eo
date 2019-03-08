#pragma once

#include "Ps/PlotStyles.h"

struct EoLineweightData {
	int m_OldIdx;
	int m_NewIdx;
	double m_Value;
	EoLineweightData() : m_OldIdx(- 1), m_NewIdx(- 1) {}; 
};

class EoDlgPlotStyleEditLineweight : public CDialog {
	DECLARE_DYNAMIC(EoDlgPlotStyleEditLineweight)

	OdPsPlotStyleTable* m_PlotStyleTable;
	CImageList m_ListCtrlImages;
	size_t m_InitialSelection;
	EoLineweightData* m_LineweightData;

public: // Construction
	EoDlgPlotStyleEditLineweight(CWnd* parent = NULL);
	virtual ~EoDlgPlotStyleEditLineweight() {}

	enum { IDD = IDD_PLOTSTYLE_LINEWEIGHT };

	CListCtrl m_LineweightsListCtrl;
	CButton m_MillimetrsButton;
	CButton m_InchesButton;

protected:
	virtual BOOL DestroyWindow();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

protected:
	void InitializeListCtrl();
	const int InsertLineweightAt(int index, const CString& lineweight, const bool isUse);
	void InitializeLineweightsListCtrlImages();

public:
	const bool SetPlotStyleTable(OdPsPlotStyleTable* plotStyleTable);
	void SetUnitIntoList(const bool isInchUnits);
	void SetInitialSelection(int selection);

protected:
	afx_msg void OnRadioMillimetrs();
	afx_msg void OnRadioInches();
	afx_msg void OnButtonEditlineweight();
	afx_msg void OnButtonSortlineweight();
	afx_msg void OnEndlabeleditListLineweight(NMHDR* pNMHDR, LRESULT* result);

	DECLARE_MESSAGE_MAP()
};
