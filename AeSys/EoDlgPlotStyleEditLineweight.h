#pragma once
#include "Ps/plotstyles.h"

struct EoLineweightData {
	int oldIndex {-1};
	int newIndex {-1};
	double value {0.0};
};

class EoDlgPlotStyleEditLineweight final : public CDialog {
DECLARE_DYNAMIC(EoDlgPlotStyleEditLineweight)

	OdPsPlotStyleTable* plotStyleTable;
	CImageList listCtrlImages;
	unsigned initialSelection;
	EoLineweightData* lineweightData;

	EoDlgPlotStyleEditLineweight(CWnd* parent = nullptr);

	enum { IDD = IDD_PLOTSTYLE_LINEWEIGHT };

	CListCtrl lineweightsListCtrl;
	CButton millimetersButton;
	CButton inchesButton;
protected:
	BOOL DestroyWindow() final;

	void DoDataExchange(CDataExchange* dataExchange) final;

	BOOL OnInitDialog() final;

	void OnOK() final;

	void InitializeListCtrl();

	int InsertLineweightAt(int index, const OdString& lineweight, bool isUse);

	void InitializeLineweightsListCtrlImages();

public:
	bool SetPlotStyleTable(OdPsPlotStyleTable* plotStyleTable) noexcept;

	void SetUnitIntoList(bool isInchUnits);

	void SetInitialSelection(int selection) noexcept;

protected:
	void OnRadioMillimeters();

	void OnRadioInches();

	void OnButtonEditLineweight();

	void OnButtonSortLineweight();

	void OnEndlabeleditListLineweight(NMHDR* notifyStructure, LRESULT* result);

DECLARE_MESSAGE_MAP()
};
