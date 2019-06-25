#include "stdafx.h"
#include <OdaCommon.h>
#include <Gs/Gs.h>
#include <winspool.h>
#include <DbPlotSettingsValidator.h>
#include <DbHostAppServices.h>
#include <DbSymbolTable.h>
#include <DbViewTable.h>
#include <DbViewTableRecord.h>
#include "AeSys.h"
#include "AeSysView.h"
#include "EoDlgPageSetup.h"
#include "EoDlgPlotStyleTableEditor.h"
std::vector<const wchar_t*> StandardPlotScaleValues = {
L"Custom",
L"1/128\" = 1'",
L"1/64\" = 1'",
L"1/32\" = 1'",
L"1/16\" = 1'",
L"3/32\" = 1'",
L"1/8\" = 1'",
L"3/16\" = 1'",
L"1/4\" = 1'",
L"3/8\" = 1'",
L"1/2\" = 1'",
L"3/4\" = 1'",
L"1\" = 1'",
L"3\" = 1'",
L"6\" = 1'",
L"1' = 1'",
L"1:1",
L"1:2",
L"1:4",
L"1:8",
L"1:10",
L"1:16",
L"1:20",
L"1:30",
L"1:40",
L"1:50",
L"1:100",
L"2:1",
L"4:1",
L"8:1",
L"10:1",
L"100:1",
L"1000:1"
};

struct EoPlotScaleSetting {
	double m_RealWorldUnits; // (dxf group 142)
	double m_DrawingUnits; // (dxf group 143)
	int m_ScaleType; // (dxf group 75)
	double m_dScaleFactor; //  (dxf group 147)
};

static EoPlotScaleSetting plotScaleSetting[] = {
	{1, 1, 0, 1},
	{1, 1536, 1, 0.0006510416666667},
	{1, 768, 2, 0.0013020833333333},
	{1, 384, 3, 0.0026041666666667},
	{1, 192, 4, 0.0052083333333333},
	{1, 128, 5, 0.0078125},
	{1, 96, 6, 0.0104166666666667},
	{1, 64, 7, 0.015625},
	{1, 48, 8, 0.0208333333333333},
	{1, 32, 9, 0.03125},
	{1, 24, 10, 0.0416666666666667},
	{1, 16, 11, 0.0625},
	{1, 12, 12, 0.0833333333333333},
	{1, 4, 13, 0.25},
	{1, 2, 14, 0.5},
	{1, 1, 15, 1},
	{1, 1, 16, 1},
	{1, 2, 17, 0.5},
	{1, 4, 18, 0.25},
	{1, 8, 19, 0.125},
	{1, 10, 20, 0.1},
	{1, 16, 21, 0.0625},
	{1, 20, 22, 0.05},
	{1, 30, 23, 0.03333333333333},
	{1, 40, 24, 0.025},
	{1, 50, 25, 0.2},
	{1, 100, 26, 0.01},
	{2, 1, 27, 2},
	{4, 1, 28, 4},
	{8, 1, 29, 8},
	{10, 1, 30, 10},
	{100, 1, 31, 100},
	{1000, 1, 32, 1000}
};

struct EoPlotUnitsInfo {
	double m_Scale;
	const wchar_t* m_Name1;
	const wchar_t* m_Name2;

	static const wchar_t* GetTextByValue(const double value, const EoPlotUnitsInfo& info) noexcept {
		return fabs(value) <= 1.0 ? info.m_Name1 : info.m_Name2;
	}
};

static EoPlotUnitsInfo PlotUnitsInfo[] = {
	{kMmPerInch, L"inch", L"inches"},
	{1.0, L"mm", L"mm"},
	{1.0, L"pixel", L"pixels"},
	{1.0, L"unit", L"units"}
};

// EoDlgPageSetup
EoDlgPageSetup::EoDlgPageSetup(OdDbPlotSettings& plotSettings, OdSmartPtr<OdDbUserIO> pIO)
	: CDialog(IDD, nullptr)
	, m_PlotSettings(plotSettings)
	, m_pIO(pIO) {
	m_CustomDPI = 0;
	m_CenterThePlot = 0;
	m_OffsetX = 0.0;
	m_OffsetY = 0.0;
	m_PaperImageOriginX = L"0.";
	m_PaperImageOriginY = L"0.";
	m_DrawingOrientation = 0;
	m_PaperScaleUnit = 0.0;
	m_DrawingScaleUnit = 0.0;
	m_FitToPaper = 0;
	m_LeftMargin = L"0.";
	m_RightMargin = L"0.";
	m_TopMargin = L"0.";
	m_BottomMargin = L"0.";
	m_DisplayPlotStyles = 0;
	m_PlotUpsideDown = 0;
	m_ScaleLW = 0;
	m_PlotObjectLW = 0;
	m_PlotWithPlotStyles = 0;
	m_PlotPaperspaceLast = 0;
	m_HidePaperspaceObjects = 0;
	m_xMin = L"0.";
	m_yMin = L"0.";
	m_xMax = L"0.";
	m_yMax = L"0.";
}

EoDlgPageSetup::~EoDlgPageSetup() {
}

void EoDlgPageSetup::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PAGESETUP_QUALITY, m_Quality);
	DDX_Control(pDX, IDC_PAGESETUP_SHADE_PLOT, m_ShadePlot);
	DDX_Control(pDX, IDC_PAGESETUP_VIEWS, m_Views);
	DDX_Control(pDX, IDC_COMBO_MM_INCHES, m_MMInches);
	DDX_Control(pDX, IDC_PAGESETUP_COMBO_PLOTSTYLEFILES, m_PlotStyleFiles);
	DDX_Control(pDX, IDC_PAGESETUP_DEVICE, m_PlotDeviceName);
	DDX_Control(pDX, IDC_PAGESETUP_SIZE, m_PaperSize);
	DDX_Control(pDX, IDC_PAGESETUP_PLOTAREATYPE, m_PlotAreaType);
	DDX_Check(pDX, IDC_CHECK_CENTERTHEPLOT, m_CenterThePlot);
	DDX_Check(pDX, IDC_CHECK_FIT_TO_PAPER, m_FitToPaper);
	DDX_Check(pDX, IDC_CHECK_SCALE_LW, m_ScaleLW);
	DDX_Check(pDX, IDC_CHECK_UPSIDEDOWN, m_PlotUpsideDown);
	DDX_Check(pDX, IDC_CHECK_DISPLAY_PLOT_STYLES, m_DisplayPlotStyles);
	DDX_Text(pDX, IDC_PAGESETUP_OFFSET_X, m_OffsetX);
	DDX_Text(pDX, IDC_PAGESETUP_OFFSET_Y, m_OffsetY);
	DDX_Text(pDX, IDC_CANONICAL_MEDIA_NAME, m_CanonicalMediaName);
	DDX_Text(pDX, IDC_PIO_XX, m_PaperImageOriginX);
	DDX_Text(pDX, IDC_PIO_YY, m_PaperImageOriginY);
	DDX_Text(pDX, IDC_PAGESETUP_DPI, m_CustomDPI);
	DDX_Text(pDX, IDC_MARGINS_L, m_LeftMargin);
	DDX_Text(pDX, IDC_MARGINS_R, m_RightMargin);
	DDX_Text(pDX, IDC_MARGINS_T, m_TopMargin);
	DDX_Text(pDX, IDC_MARGINS_B, m_BottomMargin);
	//DDX_Text(pDX, IDC_PAGESETUP_PAPER_UNIT_STATIC, m_PaperUnitText);
	DDX_Text(pDX, IDC_PAGESETUP_DRAWING_UNIT_STATIC, m_DrawingUnitText);
	DDX_Text(pDX, IDC_PAGESETUP_OFFSET_X_STATIC, m_OffsetXText);
	DDX_Text(pDX, IDC_PAGESETUP_OFFSET_Y_STATIC, m_OffsetYText);
	DDX_Text(pDX, IDC_WINDOW_MINX, m_xMin);
	DDX_Text(pDX, IDC_WINDOW_MINY, m_yMin);
	DDX_Text(pDX, IDC_WINDOW_MAXX, m_xMax);
	DDX_Text(pDX, IDC_WINDOW_MAXY, m_yMax);
	DDX_Radio(pDX, IDC_PAGESETUP_PORTRAIT, m_DrawingOrientation);
	DDX_Control(pDX, IDC_PAGESETUP_SCALE, m_ScaleValues);
	DDX_Text(pDX, IDC_PAGESETUP_PAPER_UNIT, m_PaperScaleUnit);
	DDX_Text(pDX, IDC_PAGESETUP_DRAWING_UNIT, m_DrawingScaleUnit);
	DDX_Check(pDX, IDC_CHECK_PLOT_OBJECT_LW, m_PlotObjectLW);
	DDX_Check(pDX, IDC_CHECK_PLOT_WITH_PLOTSTYLES, m_PlotWithPlotStyles);
	DDX_Check(pDX, IDC_CHECK_PLOT_PAPERSPACE_LAST, m_PlotPaperspaceLast);
	DDX_Check(pDX, IDC_CHECK_HIDE_PAPERSPACE_OBJECTS, m_HidePaperspaceObjects);
}

BEGIN_MESSAGE_MAP(EoDlgPageSetup, CDialog)
		ON_CBN_SELCHANGE(IDC_PAGESETUP_DEVICE, OnSelchangeDeviceList)
		ON_CBN_SELCHANGE(IDC_PAGESETUP_SCALE, OnSelchangeScaleValues)
		ON_CBN_SELCHANGE(IDC_PAGESETUP_SIZE, OnSelChangeMediaList)
		ON_CBN_SELCHANGE(IDC_PAGESETUP_PLOTAREATYPE, OnSelChangePlotAreaType)
		ON_CBN_SELCHANGE(IDC_PAGESETUP_QUALITY, OnSelChangeQualityList)
		ON_CBN_SELCHANGE(IDC_PAGESETUP_SHADE_PLOT, OnSelChangeShadePlotList)
		ON_CBN_SELCHANGE(IDC_PAGESETUP_COMBO_PLOTSTYLEFILES, OnSelChangePlotStyleFiles)
		ON_CBN_SELCHANGE(IDC_PAGESETUP_VIEWS, OnSelChangeViewsList)
		ON_CBN_SELCHANGE(IDC_COMBO_MM_INCHES, OnSelChangeMMInchesList)
		ON_BN_CLICKED(IDC_CHECK_CENTERTHEPLOT, OnCheckCenterThePlot)
		ON_BN_CLICKED(IDC_CHECK_FIT_TO_PAPER, OnCheckFitToPaper)
		ON_BN_CLICKED(IDC_CHECK_SCALE_LW, OnCheckScaleLW)
		ON_BN_CLICKED(IDC_PAGESETUP_PORTRAIT, OnClickPortraitLandscape)
		ON_BN_CLICKED(IDC_PAGESETUP_LANDSCAPE, OnClickPortraitLandscape)
		ON_BN_CLICKED(IDC_CHECK_UPSIDEDOWN, OnClickPortraitLandscape)
		ON_BN_CLICKED(IDC_BUTTON_WINDOW, OnClickWindowButton)
		ON_BN_CLICKED(IDC_PAGESETUP_BUTTON_PLOTSTYLEFILES, OnClickPlotStyleFilesBtn)
		ON_BN_CLICKED(IDC_CHECK_DISPLAY_PLOT_STYLES, OnCheckDisplayPlotStyles)
		ON_BN_CLICKED(IDC_CHECK_PLOT_OBJECT_LW, OnClickPlotStyles)
		ON_BN_CLICKED(IDC_CHECK_PLOT_WITH_PLOTSTYLES, OnClickPlotStyles)
		ON_BN_CLICKED(IDC_CHECK_PLOT_PAPERSPACE_LAST, OnClickPlotStyles)
		ON_BN_CLICKED(IDC_CHECK_HIDE_PAPERSPACE_OBJECTS, OnClickPlotStyles)
		ON_EN_KILLFOCUS(IDC_PAGESETUP_DPI, OnChangeEditDPI)
		ON_EN_KILLFOCUS(IDC_PAGESETUP_OFFSET_X, OnChangeEditOffsetXY)
		ON_EN_KILLFOCUS(IDC_PAGESETUP_OFFSET_Y, OnChangeEditOffsetXY)
		ON_EN_KILLFOCUS(IDC_PAGESETUP_PAPER_UNIT, OnChangeEditScaleUnit)
		ON_EN_KILLFOCUS(IDC_PAGESETUP_DRAWING_UNIT, OnChangeEditScaleUnit)
END_MESSAGE_MAP()

void EoDlgPageSetup::SetPlotDeviceAndMediaName(OdString& deviceName, OdString canonicalMediaName, const bool validNames) {
	const auto PlotCfgName {m_PlotSettings.getPlotCfgName()};
	const auto CanonicalMediaName {m_PlotSettings.getCanonicalMediaName()};
	if (validNames && deviceName == PlotCfgName && CanonicalMediaName == canonicalMediaName) { return; }
	if (m_PlotSettingsValidator->setPlotCfgName(&m_PlotSettings, deviceName, canonicalMediaName) != eOk) // good device, but wrong paper
	{
		if (m_PlotSettingsValidator->setPlotCfgName(&m_PlotSettings, deviceName) != eOk) { // wrong device
			deviceName = L"None";
			m_PlotSettingsValidator->setPlotCfgName(&m_PlotSettings, deviceName, "none_user_media");
		}
	}
}

void EoDlgPageSetup::FillMMInches() {
	m_MMInches.ResetContent();
	const auto PaperUnits {m_PlotSettings.plotPaperUnits()};
	if (PaperUnits == OdDbPlotSettings::kPixels) {
		m_MMInches.AddString(L"pixels");
		m_MMInches.EnableWindow(FALSE);
		m_MMInches.SetCurSel(0);
	} else {
		m_MMInches.AddString(L"inches");
		m_MMInches.AddString(L"mm");
		m_MMInches.EnableWindow(TRUE);
		m_MMInches.SetCurSel(PaperUnits == OdDbPlotSettings::kMillimeters);
	}
}

void EoDlgPageSetup::FillViewCombo(const bool fillCombo) {
	if (fillCombo) {
		m_Views.ResetContent();
		OdDbViewTablePtr ViewTable = m_PlotSettings.database()->getViewTableId().safeOpenObject();
		auto ViewTableIterator {ViewTable->newIterator()};
		while (!ViewTableIterator->done()) {
			OdDbViewTableRecordPtr pView = ViewTableIterator->getRecord();
			if (pView->isPaperspaceView() != IsModelSpacePageSetup()) {
				m_Views.AddString(pView->getName());
			}
			ViewTableIterator->step();
		}
	}
	GetDlgItem(IDC_PAGESETUP_VIEWS)->EnableWindow(m_Views.GetCount());
	if (m_Views.GetCount()) {
		m_Views.SetCurSel(m_Views.FindStringExact(0, m_PlotSettings.getPlotViewName()));
	}
}

void EoDlgPageSetup::FillShadePlotQualityDPI(const bool fillCombo) {
	if (fillCombo) {
		m_Quality.ResetContent();
		m_Quality.AddString(L"Draft");
		m_Quality.AddString(L"Preview");
		m_Quality.AddString(L"Normal");
		m_Quality.AddString(L"Presentation");
		m_Quality.AddString(L"Maximum");
		m_Quality.AddString(L"Custom");
		m_ShadePlot.ResetContent();
		m_ShadePlot.AddString(L"As displayed");
		m_ShadePlot.AddString(L"Wireframe");
		m_ShadePlot.AddString(L"Hidden");
		m_ShadePlot.AddString(L"Rendered");
	}
	const auto PlotType {m_PlotSettings.shadePlot()};
	m_ShadePlot.SetCurSel(static_cast<int>(PlotType));
	const auto ResLevel {m_PlotSettings.shadePlotResLevel()};
	m_Quality.SetCurSel(static_cast<int>(ResLevel));
	if (ResLevel == OdDbPlotSettings::kCustom) {
		m_CustomDPI = m_PlotSettings.shadePlotCustomDPI();
	}
	if (IsModelSpacePageSetup()) {
		const auto bEnableWindows {PlotType == OdDbPlotSettings::kAsDisplayed || PlotType == OdDbPlotSettings::kRendered};
		GetDlgItem(IDC_PAGESETUP_QUALITY)->EnableWindow(bEnableWindows);
		GetDlgItem(IDC_PAGESETUP_DPI)->EnableWindow(ResLevel == OdDbPlotSettings::kCustom && bEnableWindows);
	} else {
		GetDlgItem(IDC_PAGESETUP_SHADE_PLOT)->EnableWindow(FALSE);
		GetDlgItem(IDC_PAGESETUP_QUALITY)->EnableWindow(TRUE);
		GetDlgItem(IDC_PAGESETUP_DPI)->EnableWindow(ResLevel == OdDbPlotSettings::kCustom);
	}
	UpdateData(FALSE);
}

void EoDlgPageSetup::OnClickPlotStyles() {
	UpdateData();
	m_PlotSettings.setPrintLineweights(m_PlotObjectLW == 1);
	m_PlotSettings.setPlotPlotStyles(m_PlotWithPlotStyles == 1);
	m_PlotSettings.setDrawViewportsFirst(m_PlotPaperspaceLast == 1);
	m_PlotSettings.setPlotHidden(m_HidePaperspaceObjects == 1);
	FillPlotStyles();
}

void EoDlgPageSetup::FillPlotStyles() {
	m_PlotObjectLW = m_PlotSettings.printLineweights();
	m_PlotWithPlotStyles = m_PlotSettings.plotPlotStyles();
	m_PlotPaperspaceLast = m_PlotSettings.drawViewportsFirst();
	m_HidePaperspaceObjects = m_PlotSettings.plotHidden();
	GetDlgItem(IDC_CHECK_PLOT_PAPERSPACE_LAST)->EnableWindow(!IsModelSpacePageSetup());
	GetDlgItem(IDC_CHECK_HIDE_PAPERSPACE_OBJECTS)->EnableWindow(!IsModelSpacePageSetup());
	GetDlgItem(IDC_CHECK_PLOT_OBJECT_LW)->EnableWindow(!m_PlotWithPlotStyles);
	if (m_PlotWithPlotStyles) {
		m_PlotObjectLW = 1;
	}
	UpdateData(FALSE);
}

void EoDlgPageSetup::FillPaperOrientation() {
	const auto Rotation {m_PlotSettings.plotRotation()};
	m_DrawingOrientation = Rotation & 1;
	if (!IsPaperWidthLessHeight()) {
		m_DrawingOrientation = !m_DrawingOrientation;
	}
	m_PlotUpsideDown = (Rotation & 2) / 2;
	UpdateData(FALSE);
}

void EoDlgPageSetup::OnClickPortraitLandscape() {
	UpdateData();
	OdDbPlotSettings::PlotRotation Rotation;
	if (IsPaperWidthLessHeight()) {
		Rotation = static_cast<OdDbPlotSettings::PlotRotation>(m_DrawingOrientation + m_PlotUpsideDown * 2);
	} else {
		Rotation = static_cast<OdDbPlotSettings::PlotRotation>(!m_DrawingOrientation + m_PlotUpsideDown * 2);
	}
	m_PlotSettingsValidator->setPlotRotation(&m_PlotSettings, Rotation);
	FillPaperOrientation();
	FillScaleValues(false);
	FillPlotOffset();
}

void EoDlgPageSetup::OnChangeEditScaleUnit() {
	const auto OldPaperScaleUnit {m_PaperScaleUnit};
	const auto OldDrawingScaleUnit {m_DrawingScaleUnit};
	UpdateData();
	if (OldPaperScaleUnit != m_PaperScaleUnit || OldDrawingScaleUnit != m_DrawingScaleUnit) {
		//OdDbPlotSettings::PlotPaperUnits PaperUnits = m_PlotSettings.plotPaperUnits();
		ODA_VERIFY(m_PlotSettingsValidator->setCustomPrintScale(&m_PlotSettings, m_PaperScaleUnit, m_DrawingScaleUnit) == eOk);
		FillPaperOrientation();
		FillScaleValues(false);
		FillPlotOffset();
	}
}

void EoDlgPageSetup::OnCheckFitToPaper() {
	UpdateData();
	if (m_FitToPaper) {
		m_PlotSettingsValidator->setStdScaleType(&m_PlotSettings, OdDbPlotSettings::kScaleToFit);
	} else {
		m_PlotSettingsValidator->setUseStandardScale(&m_PlotSettings, false);
	}
	FillPaperOrientation();
	FillScaleValues(false);
	FillPlotOffset();
}

void EoDlgPageSetup::OnCheckScaleLW() {
	UpdateData();
	m_PlotSettings.setScaleLineweights(m_ScaleLW != 0);
	FillPaperOrientation();
	FillScaleValues(false);
	FillPlotOffset();
}

void EoDlgPageSetup::OnSelchangeScaleValues() {
	UpdateData();
	const auto CurrentSelection {m_ScaleValues.GetCurSel()};
	if (CurrentSelection != 0) { // skip Custom
		m_PlotSettingsValidator->setStdScaleType(&m_PlotSettings, StdScaleType(plotScaleSetting[CurrentSelection].m_ScaleType));
	}
	FillPaperOrientation();
	FillScaleValues(false);
	FillPlotOffset();
}

OdDbPlotSettings::PlotPaperUnits EoDlgPageSetup::GetMediaNativePPU() { // This method uses the backdoor way to define PPU from Media.
	OdDbPlotSettings PlotSettings;
	m_PlotSettingsValidator->setPlotCfgName(&PlotSettings, m_PlotSettings.getPlotCfgName(), m_PlotSettings.getCanonicalMediaName());
	return PlotSettings.plotPaperUnits();
}

void EoDlgPageSetup::OnSelChangeMediaList() {
	UpdateData();
	CString NewLocaleMediaName;
	const auto i {m_PaperSize.GetCurSel()};
	m_PaperSize.GetLBText(i, NewLocaleMediaName);
	const auto NewCanonicalMediaName {GetCanonicalByLocaleMediaName(static_cast<const wchar_t*>(NewLocaleMediaName))};
	m_PlotSettingsValidator->setCanonicalMediaName(&m_PlotSettings, NewCanonicalMediaName);
	const auto MediaNativeUnits {GetMediaNativePPU()};
	FillPaperSizes();
	m_PaperSize.SetCurSel(m_PaperSize.FindStringExact(0, NewLocaleMediaName));

	// change paper orientation to dialog values
	OdDbPlotSettings::PlotRotation Rotation;
	if (IsPaperWidthLessHeight()) {
		Rotation = static_cast<OdDbPlotSettings::PlotRotation>(m_DrawingOrientation + m_PlotUpsideDown * 2);
	} else {
		Rotation = static_cast<OdDbPlotSettings::PlotRotation>(!m_DrawingOrientation + m_PlotUpsideDown * 2);
	}
	m_PlotSettingsValidator->setPlotRotation(&m_PlotSettings, Rotation);
	FillPaperOrientation();
	FillScaleValues(false);
	FillPlotOffset();

	// and reset units to paper native
	if (MediaNativeUnits == OdDbPlotSettings::kInches || MediaNativeUnits == OdDbPlotSettings::kMillimeters) {
		m_MMInches.SetCurSel(MediaNativeUnits == OdDbPlotSettings::kMillimeters);
		OnSelChangeMMInchesList();
	}
}

OdString EoDlgPageSetup::GetCanonicalByLocaleMediaName(OdString localeMediaName) {
	OdArray<const OdChar*> MediaNames;
	m_PlotSettingsValidator->canonicalMediaNameList(&m_PlotSettings, MediaNames);
	OdArray<const OdChar*>::const_iterator NamesIterator = MediaNames.begin();
	const OdArray<const OdChar*>::const_iterator NamesIteratorEnd {MediaNames.end()};
	while (NamesIterator != NamesIteratorEnd) {
		if (m_PlotSettingsValidator->getLocaleMediaName(&m_PlotSettings, NamesIterator - MediaNames.begin()) == localeMediaName) {
			return *NamesIterator;
		}
		++NamesIterator;
	}
	ODA_ASSERT(0);
	return MediaNames.first();
}

void EoDlgPageSetup::OnSelchangeDeviceList() {
	UpdateData();
	CString NewDeviceName;
	const auto CurrentSelection {m_PlotDeviceName.GetCurSel()};
	m_PlotDeviceName.GetLBText(CurrentSelection, NewDeviceName);
	auto CanonicalMediaName {m_PlotSettings.getCanonicalMediaName()};
	OdString DeviceName(NewDeviceName);
	SetPlotDeviceAndMediaName(DeviceName, CanonicalMediaName, true);
	m_PlotDeviceName.SelectString(0, DeviceName);
	if (!FillPaperSizes()) { return; }
	const auto LocaleMediaName {m_PlotSettingsValidator->getLocaleMediaName(&m_PlotSettings, m_PlotSettings.getCanonicalMediaName())};
	if (m_PaperSize.SetCurSel(m_PaperSize.FindStringExact(0, LocaleMediaName)) == LB_ERR) {
		// ALEXR TODO : Autocad use paper w&h to find nearest paper or set a4 ?
		// ALEXR TODO : SelectString select by part of string. 'q' -> select 'qwe'.
		CString csLocaleMediaName;
		m_PaperSize.GetLBText(0, csLocaleMediaName);
		if (csLocaleMediaName == L"") { return; }
		CanonicalMediaName = GetCanonicalByLocaleMediaName(static_cast<const wchar_t*>(csLocaleMediaName));
		m_PlotSettingsValidator->setCanonicalMediaName(&m_PlotSettings, CanonicalMediaName);
		m_PaperSize.SetCurSel(m_PaperSize.FindStringExact(0, csLocaleMediaName));
	}
	FillPlotAreaCombo(false);
	FillPlotOffset();
	FillScaleValues(false);
	FillPaperOrientation();
	FillPlotStyles();
}

BOOL EoDlgPageSetup::OnInitDialog() {
	if (!CDialog::OnInitDialog()) {
		return FALSE;
	}
	if (!m_PlotSettings.database() || !m_PlotSettings.database()->appServices()) {
		return FALSE;
	}
	m_PlotSettingsValidator = m_PlotSettings.database()->appServices()->plotSettingsValidator();
	if (m_PlotSettingsValidator.isNull()) {
		return FALSE;
	}
	m_PlotSettingsValidator->refreshLists(&m_PlotSettings);

	// is stored device name available in system ?
	auto PlotCfgName {m_PlotSettings.getPlotCfgName()};
	const auto CanonicalMediaName {m_PlotSettings.getCanonicalMediaName()};
	SetPlotDeviceAndMediaName(PlotCfgName, CanonicalMediaName, false);
	if (!FillDeviceCombo()) {
		return FALSE;
	}
	m_PlotDeviceName.SelectString(0, PlotCfgName);
	UpdateData(FALSE);

	// select active paper from plot settings
	// au doesn't use media name stored in dxf, possible au
	// look for name by paper parameters.
	OnSelchangeDeviceList();
	FillPlotAreaCombo(true);
	FillPlotOffset();
	FillScaleValues(true);
	FillPaperOrientation();
	FillPlotStyles();
	FillShadePlotQualityDPI(true);
	FillPlotStyleCombo(true);
	FillViewCombo(true);
	FillWindowArea();
	return TRUE;
}

bool EoDlgPageSetup::FillDeviceCombo() {
	OdArray<const OdChar*> Devices;
	m_PlotSettingsValidator->plotDeviceList(Devices);
	m_PlotDeviceName.ResetContent();
	OdArray<const OdChar*>::const_iterator DeviceIterator {Devices.begin()};
	OdArray<const OdChar*>::const_iterator DeviceIteratorEnd {Devices.end()};
	while (DeviceIterator != DeviceIteratorEnd) {
		m_PlotDeviceName.AddString(static_cast<LPCTSTR>(OdString(*DeviceIterator)));
		++DeviceIterator;
	}
	UpdateData(FALSE);
	return true;
}

bool EoDlgPageSetup::FillPaperSizes() {
	OdArray<const OdChar*> CanonicalMediaNames;
	m_PlotSettingsValidator->canonicalMediaNameList(&m_PlotSettings, CanonicalMediaNames);
	m_PaperSize.ResetContent();
	for (unsigned long MediaIndex = 0; MediaIndex < CanonicalMediaNames.size(); ++MediaIndex) {
		m_PaperSize.AddString(m_PlotSettingsValidator->getLocaleMediaName(&m_PlotSettings, static_cast<int>(MediaIndex)));
	}
	UpdateData(FALSE);
	return true;
}

void EoDlgPageSetup::FillScaleValues(const bool fillCombo) {
	if (fillCombo) {
		m_ScaleValues.ResetContent();
		const auto NumberOfScaleVaules {StandardPlotScaleValues.size()};
		for (unsigned ScaleValueIndex = 0; ScaleValueIndex < NumberOfScaleVaules; ScaleValueIndex++) {
			m_ScaleValues.AddString(StandardPlotScaleValues.at(ScaleValueIndex));
		}
	}
	const auto ScaleType {m_PlotSettings.stdScaleType()};
	if (m_PlotSettings.useStandardScale() && ScaleType != OdDbPlotSettings::kScaleToFit && ScaleType >= 0 && ScaleType <= OdDbPlotSettings::k1000_1) {
		m_ScaleValues.SetCurSel(m_ScaleValues.FindStringExact(0, StandardPlotScaleValues.at(ScaleType)));
	} else {
		m_ScaleValues.SetCurSel(m_ScaleValues.FindStringExact(0, L"Custom"));
	}
	const auto IsModel {IsModelSpacePageSetup()};
	const auto IsLayout {m_PlotSettings.plotType() == OdDbPlotSettings::kLayout};
	m_FitToPaper = m_PlotSettings.useStandardScale() && !IsLayout && ScaleType == OdDbPlotSettings::kScaleToFit;
	m_ScaleLW = m_PlotSettings.scaleLineweights();
	if (IsLayout) {
		m_FitToPaper = m_CenterThePlot = false;
	}
	GetDlgItem(IDC_CHECK_SCALE_LW)->EnableWindow(!IsModel);
	GetDlgItem(IDC_CHECK_FIT_TO_PAPER)->EnableWindow(!IsLayout);
	GetDlgItem(IDC_CHECK_CENTERTHEPLOT)->EnableWindow(!IsLayout);
	GetDlgItem(IDC_PAGESETUP_SCALE)->EnableWindow(!m_FitToPaper);
	GetDlgItem(IDC_PAGESETUP_PAPER_UNIT)->EnableWindow(!m_FitToPaper);
	GetDlgItem(IDC_PAGESETUP_DRAWING_UNIT)->EnableWindow(!m_FitToPaper);
	if (m_PlotSettings.useStandardScale() && !m_FitToPaper) {
		m_PaperScaleUnit = plotScaleSetting[ScaleType].m_RealWorldUnits;
		m_DrawingScaleUnit = plotScaleSetting[ScaleType].m_DrawingUnits;
	} else {
		m_PlotSettings.getCustomPrintScale(m_PaperScaleUnit, m_DrawingScaleUnit);
	}
	FillMMInches();
	//m_PaperUnitText = CString(EoPlotUnitsInfo::GetTextByValue(m_PaperScaleUnit, PlotUnitsInfo[PaperUnits])) + L" =");
	m_DrawingUnitText = EoPlotUnitsInfo::GetTextByValue(m_DrawingScaleUnit, PlotUnitsInfo[3]);
	UpdateData(FALSE);
}

bool EoDlgPageSetup::IsWHSwap() const {
	const auto Rotation {m_PlotSettings.plotRotation()};
	return Rotation == OdDbPlotSettings::k90degrees || Rotation == OdDbPlotSettings::k270degrees;
}

void EoDlgPageSetup::OnChangeEditOffsetXY() {
	UpdateData();
	const auto PaperUnits {m_PlotSettings.plotPaperUnits()};
	if (PaperUnits == OdDbPlotSettings::kInches) {
		m_OffsetX *= PlotUnitsInfo[PaperUnits].m_Scale;
		m_OffsetY *= PlotUnitsInfo[PaperUnits].m_Scale;
	}
	if (IsWHSwap()) {
		m_PlotSettingsValidator->setPlotOrigin(&m_PlotSettings, m_OffsetY, m_OffsetX);
	} else {
		m_PlotSettingsValidator->setPlotOrigin(&m_PlotSettings, m_OffsetX, m_OffsetY);
	}
	FillPaperOrientation();
	FillPlotOffset(); // possibly offset was changed in validator
	FillScaleValues(false);
}

void EoDlgPageSetup::OnCheckCenterThePlot() {
	UpdateData();
	m_PlotSettingsValidator->setPlotCentered(&m_PlotSettings, m_CenterThePlot != 0);
	FillPaperOrientation();
	FillPlotOffset();
	FillScaleValues(false);
}

void EoDlgPageSetup::OnSelChangePlotAreaType() {
	UpdateData();
	CString NewViewType;
	const auto CurrentSelection {m_PlotAreaType.GetCurSel()};
	m_PlotAreaType.GetLBText(CurrentSelection, NewViewType);
	auto Type {OdDbPlotSettings::kDisplay};
	if (NewViewType == L"Display") {
		Type = OdDbPlotSettings::kDisplay;
	} else if (NewViewType == L"Limits") {
		Type = OdDbPlotSettings::kLimits;
	} else if (NewViewType == L"View") {
		Type = OdDbPlotSettings::kView;
	} else if (NewViewType == L"Window") {
		Type = OdDbPlotSettings::kWindow;
		OnClickWindowButton();
	} else if (NewViewType == L"Extents") {
		Type = OdDbPlotSettings::kExtents;
	} else if (NewViewType == L"Layout") {
		Type = OdDbPlotSettings::kLayout;
	}
	m_PlotSettingsValidator->setPlotType(&m_PlotSettings, Type);
	FillPlotAreaCombo(false);
	if (Type == OdDbPlotSettings::kLayout) {
		// This is differences between dialog and validator. Validator doesn't change UseStandardScale to false.
		// Example is kExtents, kFit2Paper -> kLayout ->kExtents
		// Dialog has kFit2Paper disabled, but validator don't clear kFit2Paper flag.
		// Validator also don't change PlotOrigin to 0,0, if plotsenteres was true, but it change scale to 1:1 if fittopaper was true
		if (m_CenterThePlot) {
			m_PlotSettingsValidator->setPlotOrigin(&m_PlotSettings, 0.0, 0.0);
		}
		if (m_FitToPaper) {
			m_PlotSettingsValidator->setUseStandardScale(&m_PlotSettings, false);
		}
	}
	FillPaperOrientation();
	FillScaleValues(false);
	FillPlotOffset();
}

bool EoDlgPageSetup::IsPaperWidthLessHeight() const {
	double PaperWidth;
	double PaperHeight;
	m_PlotSettings.getPlotPaperSize(PaperWidth, PaperHeight);
	return PaperWidth < PaperHeight;
}

void EoDlgPageSetup::FillWindowArea() {
	double xmin;
	double ymin;
	double xmax;
	double ymax;
	m_PlotSettings.getPlotWindowArea(xmin, ymin, xmax, ymax);
	m_xMin.Format(L"%.6f", xmin);
	m_yMin.Format(L"%.6f", ymin);
	m_xMax.Format(L"%.6f", xmax);
	m_yMax.Format(L"%.6f", ymax);
	UpdateData(FALSE);
}

void EoDlgPageSetup::FillPlotOffset() {
	m_CenterThePlot = m_PlotSettings.plotCentered();
	GetDlgItem(IDC_PAGESETUP_OFFSET_X)->EnableWindow(!m_CenterThePlot);
	GetDlgItem(IDC_PAGESETUP_OFFSET_Y)->EnableWindow(!m_CenterThePlot);
	if (IsWHSwap()) {
		m_PlotSettings.getPlotOrigin(m_OffsetY, m_OffsetX);
	} else {
		m_PlotSettings.getPlotOrigin(m_OffsetX, m_OffsetY);
	}
	const auto PaperUnits {m_PlotSettings.plotPaperUnits()};
	if (PaperUnits == OdDbPlotSettings::kInches) {
		m_OffsetX /= PlotUnitsInfo[PaperUnits].m_Scale;
		m_OffsetY /= PlotUnitsInfo[PaperUnits].m_Scale;
	}
	// it doesn't changed with IsWHSwap
	m_PaperImageOriginX.Format(L"%.6f", m_PlotSettings.getPaperImageOrigin().x);
	m_PaperImageOriginY.Format(L"%.6f", m_PlotSettings.getPaperImageOrigin().y);
	m_LeftMargin.Format(L"%.6f", m_PlotSettings.getLeftMargin());
	m_RightMargin.Format(L"%.6f", m_PlotSettings.getRightMargin());
	m_TopMargin.Format(L"%.6f", m_PlotSettings.getTopMargin());
	m_BottomMargin.Format(L"%.6f", m_PlotSettings.getBottomMargin());
	m_CanonicalMediaName = static_cast<const wchar_t*>(m_PlotSettings.getCanonicalMediaName());
	m_OffsetXText = EoPlotUnitsInfo::GetTextByValue(m_OffsetX, PlotUnitsInfo[PaperUnits]);
	m_OffsetYText = EoPlotUnitsInfo::GetTextByValue(m_OffsetY, PlotUnitsInfo[PaperUnits]);
	UpdateData(FALSE);
}

bool EoDlgPageSetup::ViewsExist() const {
	OdDbViewTablePtr pViewTable = m_PlotSettings.database()->getViewTableId().safeOpenObject();
	auto pIt {pViewTable->newIterator()};
	while (!pIt->done()) {
		OdDbViewTableRecordPtr pView = pIt->getRecord();
		if (pView->isPaperspaceView() != IsModelSpacePageSetup()) {
			return true;
		}
		pIt->step();
	}
	return false;
}

void EoDlgPageSetup::FillPlotAreaCombo(const bool fillCombo) {
	if (fillCombo) {
		m_PlotAreaType.ResetContent();
		if (IsModelSpacePageSetup()) {
			m_PlotAreaType.AddString(L"Display");
			m_PlotAreaType.AddString(L"Extents");// TODO : depends on entities existence
			m_PlotAreaType.AddString(L"Limits");
			if (ViewsExist()) {
				m_PlotAreaType.AddString(L"View");
			}
			m_PlotAreaType.AddString(L"Window");
		} else {
			m_PlotAreaType.AddString(L"Display");
			m_PlotAreaType.AddString(L"Extents");// TODO : depends on entities existence
			m_PlotAreaType.AddString(L"Layout");
			if (ViewsExist()) {
				m_PlotAreaType.AddString(L"View");
			}
			m_PlotAreaType.AddString(L"Window");
		}
	}
	const auto Type {m_PlotSettings.plotType()};
	GetDlgItem(IDC_PAGESETUP_VIEWS)->ShowWindow(Type == OdDbPlotSettings::kView ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_BUTTON_WINDOW)->ShowWindow(Type == OdDbPlotSettings::kWindow ? SW_SHOW : SW_HIDE);
	switch (Type) {
		case OdDbPlotSettings::kDisplay: {
			m_PlotAreaType.SelectString(0, L"Display");
			break;
		}
		case OdDbPlotSettings::kExtents: {
			m_PlotAreaType.SelectString(0, L"Extents");
			break;
		}
		case OdDbPlotSettings::kLimits: {
			m_PlotAreaType.SelectString(0, L"Limits");
			break;
		}
		case OdDbPlotSettings::kView: {
			m_PlotAreaType.SelectString(0, L"View");
			break;
		}
		case OdDbPlotSettings::kWindow: {
			m_PlotAreaType.SelectString(0, L"Window");
			break;
		}
		case OdDbPlotSettings::kLayout: {
			m_PlotAreaType.SelectString(0, L"Layout");
			break;
		}
	}
	UpdateData(FALSE);
}

void EoDlgPageSetup::OnOK() {
	UpdateData(); // TRUE - data is being retrieved 
	CDialog::OnOK();
}

void EoDlgPageSetup::OnCancel() {
	CDialog::OnCancel();
}

bool EoDlgPageSetup::IsModelSpacePageSetup() const {
	return m_PlotSettings.modelType();
}

bool EoDlgPageSetup::FillArrayByPatternFile(OdArray<CString>& arrFiles, CString pattern) {
	WIN32_FIND_DATA FindFileData;
	::ZeroMemory(&FindFileData, sizeof(WIN32_FIND_DATA));
	const auto Folder {pattern.Left(pattern.ReverseFind(L'\\') + 1)};
	const auto FileHandle {FindFirstFileW(pattern, &FindFileData)};
	int Find;
	auto IsFind {false};
	do {
		if (FindFileData.dwFileAttributes & ~FILE_ATTRIBUTE_DIRECTORY) {
			auto File {Folder + FindFileData.cFileName};
			arrFiles.append(File);
			IsFind = true;
		}
		Find = FindNextFile(FileHandle, &FindFileData);

	} while (Find && Find != ERROR_NO_MORE_FILES);
	FindClose(FileHandle);
	return IsFind;
}

void EoDlgPageSetup::FillPlotStyleCombo(const bool fillCombo) {
	USES_CONVERSION;
	if (fillCombo) {
		OdArray<const OdChar*> StyleList;
		m_PlotSettingsValidator->plotStyleSheetList(StyleList);
		m_PlotStyleFiles.AddString(L"None");
		for (auto& Style : StyleList) {
			m_PlotStyleFiles.AddString(W2T((wchar_t*)Style));
		}
	}
	auto StyleIndex {0};
	const auto StyleSheet {m_PlotSettings.getCurrentStyleSheet()};
	if (!StyleSheet.isEmpty()) {
		StyleIndex = m_PlotStyleFiles.FindStringExact(0, StyleSheet);
		if (StyleIndex == -1) { StyleIndex = 0; }
	}
	m_PlotStyleFiles.SetCurSel(StyleIndex);
	OnSelChangePlotStyleFiles();
	GetDlgItem(IDC_CHECK_DISPLAY_PLOT_STYLES)->EnableWindow(!IsModelSpacePageSetup());
	m_DisplayPlotStyles = m_PlotSettings.showPlotStyles();
	UpdateData(FALSE);
}

void EoDlgPageSetup::OnClickPlotStyleFilesBtn() {
	const auto CurrentSelection {m_PlotStyleFiles.GetCurSel()};
	CString tmp;
	m_PlotStyleFiles.GetLBText(CurrentSelection, tmp);
	try {
		auto bSucc(false);
		auto SystemServices {odSystemServices()};
		OdString sPath = static_cast<const wchar_t*>(tmp);
		sPath = m_PlotSettings.database()->appServices()->findFile(sPath);
		if (sPath.isEmpty()) { return; }
		OdStreamBufPtr StreamBuffer;
		if (SystemServices->accessFile(sPath, Oda::kFileRead)) {
			bSucc = true;
			StreamBuffer = SystemServices->createFile(sPath);
		}
		if (!bSucc) { return; }
		OdPsPlotStyleTablePtr PlotStyleTable;
		if (StreamBuffer.get()) {
			OdPsPlotStyleServicesPtr PlotStyleServices = odrxDynamicLinker()->loadApp(ODPS_PLOTSTYLE_SERVICES_APPNAME);
			if (PlotStyleServices.get()) {
				PlotStyleTable = PlotStyleServices->loadPlotStyleTable(StreamBuffer);
			}
		}
		EoDlgPlotStyleManager PsTableEditorDlg(theApp.GetMainWnd());
		PsTableEditorDlg.SetFileBufPath(sPath);
		PsTableEditorDlg.SetPlotStyleTable(PlotStyleTable);
		if (PsTableEditorDlg.DoModal() == IDOK) {
			PlotStyleTable->copyFrom(PsTableEditorDlg.GetPlotStyleTable());
		}
	} catch (...) {
	}
}

void EoDlgPageSetup::OnSelChangePlotStyleFiles() {
	const auto CurrentSelection {m_PlotStyleFiles.GetCurSel()};
	GetDlgItem(IDC_PAGESETUP_BUTTON_PLOTSTYLEFILES)->EnableWindow(CurrentSelection);
	if (CurrentSelection) {
		CString StyleFileName;
		m_PlotStyleFiles.GetLBText(CurrentSelection, StyleFileName);
		m_PlotSettingsValidator->setCurrentStyleSheet(&m_PlotSettings, static_cast<const wchar_t*>(StyleFileName));
	} else {
		m_PlotSettingsValidator->setCurrentStyleSheet(&m_PlotSettings, L"");
	}
}

void EoDlgPageSetup::OnSelChangeQualityList() {
	UpdateData();
	const auto CurrentSelection {m_Quality.GetCurSel()};
	m_PlotSettings.setShadePlotResLevel(static_cast<OdDbPlotSettings::ShadePlotResLevel>(CurrentSelection));
	FillShadePlotQualityDPI(false);
}

void EoDlgPageSetup::OnSelChangeShadePlotList() {
	UpdateData();
	const auto CurrentSelection {m_ShadePlot.GetCurSel()};
	m_PlotSettings.setShadePlot(static_cast<OdDbPlotSettings::ShadePlotType>(CurrentSelection));
	FillShadePlotQualityDPI(false);
}

void EoDlgPageSetup::OnSelChangeViewsList() {
	UpdateData();
	CString ViewName;
	const auto CurrentSelection {m_Views.GetCurSel()};
	m_Views.GetLBText(CurrentSelection, ViewName);
	m_PlotSettingsValidator->setPlotViewName(&m_PlotSettings, static_cast<const wchar_t*>(ViewName));
	FillViewCombo(false);
}

void EoDlgPageSetup::UnitsConverted(const OdDbPlotSettings::PlotPaperUnits prevUnits, const OdDbPlotSettings::PlotPaperUnits plotPaperUnits) {
	double ConversionFactor;
	if (plotPaperUnits == OdDbPlotSettings::kMillimeters && prevUnits == OdDbPlotSettings::kInches) {
		ConversionFactor = 25.4;
	} else if (plotPaperUnits == OdDbPlotSettings::kInches && prevUnits == OdDbPlotSettings::kMillimeters) {
		ConversionFactor = 1.0 / 25.4;
	} else {
		return;
	}
	auto PaperImageOrigin {m_PlotSettings.getPaperImageOrigin()};
	PaperImageOrigin /= ConversionFactor;
	m_PlotSettings.setPaperImageOrigin(PaperImageOrigin);
	if (m_PlotSettings.useStandardScale()) {
		double StandardScale;
		m_PlotSettings.getStdScale(StandardScale);
		if (StandardScale != 0.0) { // skip Fit to paper
			m_PlotSettingsValidator->setCustomPrintScale(&m_PlotSettings, StandardScale, 1. / ConversionFactor);
		}
	} else {
		double Numerator;
		double Denominator;
		m_PlotSettings.getCustomPrintScale(Numerator, Denominator);
		m_PlotSettingsValidator->setCustomPrintScale(&m_PlotSettings, Numerator, Denominator / ConversionFactor);
	}
}

void EoDlgPageSetup::OnSelChangeMMInchesList() {
	UpdateData();
	CString Units;
	const auto CurrentSelection {m_MMInches.GetCurSel()};
	m_MMInches.GetLBText(CurrentSelection, Units);
	auto PaperUnits {OdDbPlotSettings::kPixels};
	if (Units == "mm") {
		PaperUnits = OdDbPlotSettings::kMillimeters;
	} else if (Units == "inches") {
		PaperUnits = OdDbPlotSettings::kInches;
	}
	const auto PreviousUnits {m_PlotSettings.plotPaperUnits()};
	ODA_VERIFY(m_PlotSettingsValidator->setPlotPaperUnits(&m_PlotSettings, PaperUnits) == eOk);

	// cr 5751 fix - I didn't test kPixels way.
	UnitsConverted(PreviousUnits, PaperUnits);
	FillScaleValues(false);
	FillPlotOffset();
}

void EoDlgPageSetup::OnChangeEditDPI() {
	UpdateData();
	m_PlotSettings.setShadePlotCustomDPI(m_CustomDPI);
	FillShadePlotQualityDPI(false);
}

void EoDlgPageSetup::OnCheckDisplayPlotStyles() {
	UpdateData();
	m_PlotSettings.setShowPlotStyles(m_DisplayPlotStyles != 0);
	FillPlotStyleCombo(false);
}

void EoDlgPageSetup::OnClickWindowButton() {
	UpdateData();
	ShowWindow(SW_HIDE);
	auto ParentWindow {GetParent()};
	ParentWindow->EnableWindow(TRUE);
	ParentWindow->BringWindowToTop();
	auto FirstCorner {m_pIO->getPoint(L"Specify first corner:", OdEd::kGptNoUCS)};
	auto OppositeCorner {m_pIO->getPoint(L"Specify opposite corner:", OdEd::kGptNoUCS | OdEd::kGptRectFrame)};

	// <command_view>
	// Points are returned in eye plane, transform it back to screen plane if it is possible
	// Workaround, unfortunately can't get screen plane point from IO stream.
	const auto ChildWindow {static_cast<CMDIFrameWnd*>(theApp.GetMainWnd())->MDIGetActive()};
	const auto ActiveView {ChildWindow->GetActiveView()};
	if (CString(ActiveView->GetRuntimeClass()->m_lpszClassName).Compare(L"AeSysView") == 0) {
		auto View {dynamic_cast<AeSysView*>(ActiveView)};
		if (View->isModelSpaceView()) {
			FirstCorner = View->editorObject().ToScreenCoord(FirstCorner);
			OppositeCorner = View->editorObject().ToScreenCoord(OppositeCorner);
		}
	}
	// </command_view>
	m_PlotSettingsValidator->setPlotWindowArea(&m_PlotSettings, FirstCorner.x, FirstCorner.y, OppositeCorner.x, OppositeCorner.y);
	ParentWindow->EnableWindow(FALSE);
	EnableWindow(TRUE);
	ShowWindow(SW_SHOW);
	FillWindowArea();
}
