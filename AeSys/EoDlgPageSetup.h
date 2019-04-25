#pragma once

#include "DbPlotSettings.h"
#include "DbPlotSettingsValidator.h"

class EoDlgPageSetup : public CDialog {
	OdDbPlotSettings &m_PlotSettings;
	OdDbPlotSettingsValidatorPtr m_PlotSettingsValidator;
	OdSmartPtr<OdDbUserIO> m_pIO;

	OdString GetCanonicalByLocaleMediaName(OdString localeMediaName);
	void SetPlotDeviceAndMediaName(OdString &deviceName, OdString canonicalMediaName, bool validNames);
	bool IsWHSwap() const;
	bool IsPaperWidthLessHeight() const;
	void UnitsConverted(OdDbPlotSettings::PlotPaperUnits prevUnits, OdDbPlotSettings::PlotPaperUnits plotPaperUnits);
	OdDbPlotSettings::PlotPaperUnits GetMediaNativePPU();
public:
	EoDlgPageSetup(OdDbPlotSettings& plotSettings, OdSmartPtr<OdDbUserIO> pIO);
	~EoDlgPageSetup();

private:
	enum { IDD = IDD_PAGE_SETUP };
	CComboBox m_PlotStyleFiles;
	CComboBox m_PlotDeviceName;
	CComboBox m_PaperSize;
	CComboBox m_PlotAreaType;
	CComboBox m_ScaleValues;
	CComboBox m_Quality;
	CComboBox m_ShadePlot;
	CComboBox m_Views;
	CComboBox m_MMInches;
	int m_CenterThePlot;
	int m_DisplayPlotStyles;
	double m_OffsetX;
	double m_OffsetY;
	int m_DrawingOrientation;
	int m_PlotUpsideDown;
	double m_PaperScaleUnit;
	double m_DrawingScaleUnit;
	int m_FitToPaper;
	int m_ScaleLW;
	int m_PlotObjectLW;
	int m_PlotWithPlotStyles;
	int m_PlotPaperspaceLast;
	int m_HidePaperspaceObjects;
	short m_CustomDPI;
	//CString m_PaperUnitText;
	CString m_DrawingUnitText;
	CString m_OffsetXText;
	CString m_OffsetYText;

	CString m_CanonicalMediaName;

	CString m_PaperImageOriginX;
	CString m_PaperImageOriginY;

	CString m_LeftMargin;
	CString m_RightMargin;
	CString m_TopMargin;
	CString m_BottomMargin;

	CString m_xMin;
	CString m_yMin;
	CString m_xMax;
	CString m_yMax;

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
	void OnCancel() final;
	
    afx_msg void OnSelchangeScaleValues();
	afx_msg void OnSelchangeDeviceList();
	afx_msg void OnSelChangeMediaList();
	afx_msg void OnSelChangeQualityList();
	afx_msg void OnSelChangeShadePlotList();
	afx_msg void OnSelChangeViewsList();
	afx_msg void OnSelChangeMMInchesList();
	afx_msg void OnSelChangePlotAreaType();
	afx_msg void OnCheckCenterThePlot();
	afx_msg void OnCheckDisplayPlotStyles();
	afx_msg void OnCheckFitToPaper();
	afx_msg void OnCheckScaleLW();
	afx_msg void OnChangeEditOffsetXY();
	afx_msg void OnChangeEditDPI();
	afx_msg void OnChangeEditScaleUnit();
	afx_msg void OnClickPortraitLandscape();
	afx_msg void OnClickPlotStyles();
	afx_msg void OnClickPlotStyleFilesBtn();
	afx_msg void OnSelChangePlotStyleFiles();
	afx_msg void OnClickWindowButton();

protected:
	DECLARE_MESSAGE_MAP()

	bool FillDeviceCombo();
	bool FillPaperSizes();
	void FillShadePlotQualityDPI(bool fillCombo);
	void FillScaleValues(bool fillCombo);
	void FillPlotAreaCombo(bool fillCombo);
	void FillPlotOffset();
	void FillPaperOrientation();
	void FillPlotStyles();
	void FillPlotStyleCombo(bool fillCombo);
	void FillViewCombo(bool fillCombo);
	void FillMMInches();
	bool FillArrayByPatternFile(OdArray<CString> &arrFiles, const CString pattern);
	void FillWindowArea();

	bool ViewsExist() const;
	bool IsModelSpacePageSetup() const;
};
