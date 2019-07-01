#pragma once
#include "DbPlotSettings.h"
#include "DbPlotSettingsValidator.h"

class EoDlgPageSetup final : public CDialog {
	OdDbPlotSettings& m_PlotSettings;
	OdDbPlotSettingsValidatorPtr m_PlotSettingsValidator;
	OdSmartPtr<OdDbUserIO> m_UserIo;
	OdString GetCanonicalByLocaleMediaName(OdString localeMediaName);
	void SetPlotDeviceAndMediaName(OdString& deviceName, OdString canonicalMediaName, bool validNames);
	bool IsWHSwap() const;
	bool IsPaperWidthLessHeight() const;
	void UnitsConverted(OdDbPlotSettings::PlotPaperUnits prevUnits, OdDbPlotSettings::PlotPaperUnits plotPaperUnits);
	OdDbPlotSettings::PlotPaperUnits GetMediaNativePPU();
public:
	EoDlgPageSetup(OdDbPlotSettings& plotSettings, OdSmartPtr<OdDbUserIO> userIo);
	~EoDlgPageSetup() = default;
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
	CComboBox m_MmInches;
	int m_CenterThePlot {0};
	int m_DisplayPlotStyles {0};
	double m_OffsetX {0.0};
	double m_OffsetY {0.0};
	int m_DrawingOrientation {0};
	int m_PlotUpsideDown {0};
	double m_PaperScaleUnit {0.0};
	double m_DrawingScaleUnit {0.0};
	int m_FitToPaper {0};
	int m_ScaleLineweights {0};
	int m_PlotObjectLineweights {0};
	int m_PlotWithPlotStyles {0};
	int m_PlotPaperspaceLast {0};
	int m_HidePaperspaceObjects {0};
	short m_CustomDPI {0};
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
	CString m_WindowMinX;
	CString m_WindowMinY;
	CString m_WindowMaxX;
	CString m_WindowMaxY;

  protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
	void OnCancel() final;
	void OnSelChangeScaleValues();
	void OnSelChangeDeviceList();
	void OnSelChangeMediaList();
	void OnSelChangeQualityList();
	void OnSelChangeShadePlotList();
	void OnSelChangeViewsList();
	void OnSelChangeMMInchesList();
	void OnSelChangePlotAreaType();
	void OnCheckCenterThePlot();
	void OnCheckDisplayPlotStyles();
	void OnCheckFitToPaper();
	void OnCheckScaleLineweights();
	void OnChangeEditOffsetXy();
	void OnChangeEditDpi();
	void OnChangeEditScaleUnit();
	void OnClickPortraitLandscape();
	void OnClickPlotStyles();
	void OnClickPlotStyleFilesBtn();
	void OnSelChangePlotStyleFiles();
	void OnClickWindowButton();
DECLARE_MESSAGE_MAP()
	bool FillDeviceCombo();
	bool FillPaperSizes();
	void FillShadePlotQualityDpi(bool fillCombo);
	void FillScaleValues(bool fillCombo);
	void FillPlotAreaCombo(bool fillCombo);
	void FillPlotOffset();
	void FillPaperOrientation();
	void FillPlotStyles();
	void FillPlotStyleCombo(bool fillCombo);
	void FillViewCombo(bool fillCombo);
	void FillMmInches();
	bool FillArrayByPatternFile(OdArray<CString>& arrFiles, CString pattern);
	void FillWindowArea();
	bool ViewsExist() const;
	bool IsModelSpacePageSetup() const;
};
