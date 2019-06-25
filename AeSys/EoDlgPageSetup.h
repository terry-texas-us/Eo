#pragma once
#include "DbPlotSettings.h"
#include "DbPlotSettingsValidator.h"

class EoDlgPageSetup final : public CDialog {
	OdDbPlotSettings& m_PlotSettings;
	OdDbPlotSettingsValidatorPtr m_PlotSettingsValidator;
	OdSmartPtr<OdDbUserIO> m_pIO;
	OdString GetCanonicalByLocaleMediaName(OdString localeMediaName);
	void SetPlotDeviceAndMediaName(OdString& deviceName, OdString canonicalMediaName, bool validNames);
	[[nodiscard]] bool IsWHSwap() const;
	[[nodiscard]] bool IsPaperWidthLessHeight() const;
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
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
	void OnCancel() final;
	void OnSelchangeScaleValues();
	void OnSelchangeDeviceList();
	void OnSelChangeMediaList();
	void OnSelChangeQualityList();
	void OnSelChangeShadePlotList();
	void OnSelChangeViewsList();
	void OnSelChangeMMInchesList();
	void OnSelChangePlotAreaType();
	void OnCheckCenterThePlot();
	void OnCheckDisplayPlotStyles();
	void OnCheckFitToPaper();
	void OnCheckScaleLW();
	void OnChangeEditOffsetXY();
	void OnChangeEditDPI();
	void OnChangeEditScaleUnit();
	void OnClickPortraitLandscape();
	void OnClickPlotStyles();
	void OnClickPlotStyleFilesBtn();
	void OnSelChangePlotStyleFiles();
	void OnClickWindowButton();
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
	bool FillArrayByPatternFile(OdArray<CString>& arrFiles, CString pattern);
	void FillWindowArea();
	[[nodiscard]] bool ViewsExist() const;
	[[nodiscard]] bool IsModelSpacePageSetup() const;
};
