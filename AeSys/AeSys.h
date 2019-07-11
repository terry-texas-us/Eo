#pragma once
#ifndef APSTUDIO_INVOKED
#define DOUNDEF_APSTUDIO_INVOKED
#endif
#ifdef APSTUDIO_READONLY_SYMBOLS
#define DODEF_APSTUDIO_READONLY_SYMBOLS
#endif
#define APSTUDIO_INVOKED
#undef APSTUDIO_READONLY_SYMBOLS
#include "Resource.h"       // main symbols
#ifdef DOUNDEF_APSTUDIO_INVOKED
#undef APSTUDIO_INVOKED
#undef DOUNDEF_APSTUDIO_INVOKED
#endif
#ifdef DODEF_APSTUDIO_READONLY_SYMBOLS
#define APSTUDIO_READONLY_SYMBOLS
#undef DODEF_APSTUDIO_READONLY_SYMBOLS
#endif
#include "MainFrm.h"
#include <ColorMapping.h>
#include "EoApOptions.h"
#include "EoDb.h"
extern COLORREF g_ViewBackgroundColor;
extern COLORREF g_RubberBandColor;
extern COLORREF g_ColorPalette[256];
extern COLORREF g_GreyPalette[16];
extern COLORREF* g_CurrentPalette;

/// <Open Design Alliance>
#include <ExHostAppServices.h>
#include <ExSystemServices.h>
#include <ThreadsCounter.h>
class EoDlgAudit;
#include "OdApplication.h"

class UserBreak {
};

class AeSys final : public CWinAppEx, public ExSystemServices, public ExHostAppServices {
protected:
	using CWinAppEx::operator new;
	using CWinAppEx::operator delete;
private:
	int m_ProgressLimit {100};
	int m_ProgressPosition {0};
	int m_ProgressPercent {0};
	CString m_ProgressMessage;
	bool m_DiscardBackFaces {true};
	bool m_EnableDoubleBuffer {true};
	bool m_BlocksCache {false};
	bool m_GsDevMultithreading {false};
	unsigned m_MtRegenerateThreads {4};
	bool m_EnablePrintPreviewViaBitmap {true};
	bool m_UseGsModel {true};
	bool m_EnableHlr {false};
	bool m_ContextColors {true};
	bool m_TtfPolyDraw {false};
	bool m_TtfTextOut {false};
	bool m_TtfCache {false};
	bool m_DynamicSubEntHlt {false};
	bool m_GdiGradientsAsBitmap {false};
	bool m_GdiGradientsAsPolys {false};
	unsigned char m_NGdiGradientsAsPolysThreshold {10};
	bool m_DisableAutoRegenerate {false};
	ODCOLORREF m_BackgroundColor {g_ViewBackgroundColor};
	unsigned long m_ThisThreadId {0};
	unsigned m_NumCustomCommands {0};
	unsigned long m_NumGsMenuItems {0};
	OdString m_VectorizerPath;
	OdString m_RecentCommand;
	bool m_Partial {false};
	bool m_Recover {false};
	bool m_Loading {false};
	// ODA_MT_DBIO_BEGIN
	bool m_UseMtLoading {false};
	// ODA_MT_DBIO_END
	bool m_RemoteGeometryViewer {false};
	bool m_UseTempFiles {false};
	unsigned m_PagingType {0};
	CStringArray m_TempFilesList;
	bool m_SupportFileSelectionViaDialog {true};

	//void UpdateFieldDisplay();
public:
	unsigned applicationLook {ID_VIEW_APPLOOK_OFF_2007_BLACK};

	void AddReactor(const OdApplicationReactor* reactor);

	void RemoveReactor(const OdApplicationReactor* reactor);

	std::vector<OdSmartPtr<OdApplicationReactor> > applicationReactors;

	OdDbDatabasePtr OpenFile(const wchar_t* pathName);

	void SetPartialOption(const bool partial) noexcept { m_Partial = partial; }

	void SetRecoverOption(const bool recover) noexcept { m_Recover = recover; }

	void SetMtLoadingOption(const bool useMtLoading) noexcept { m_UseMtLoading = useMtLoading; }

	OdGsMarker GetGsMenuItemMarker() const noexcept { return reinterpret_cast<OdGsMarker>(this); }

	CMenu* CommandMenu(CMenu** toolsSubMenu = nullptr);

	void RefreshCommandMenu();

	unsigned NumberOfCustomCommands() const noexcept { return m_NumCustomCommands; }

	static CString BrowseWithPreview(HWND parentWindow, const wchar_t* filter, bool multiple = false);

	bool PrintingViaBitmap() const noexcept { return m_EnablePrintPreviewViaBitmap; }

	bool DoubleBufferEnabled() const noexcept { return m_EnableDoubleBuffer; }

	bool BlocksCacheEnabled() const noexcept { return m_BlocksCache; }

	bool GsDeviceMultithreadingEnabled() const noexcept { return m_GsDevMultithreading; }

	unsigned MtRegenerateThreadsCount() const noexcept { return m_MtRegenerateThreads; }

	bool UseGsModel() const noexcept { return m_UseGsModel; }

	bool UseSoftwareHlr() const noexcept { return m_EnableHlr; }

	bool EnableContextualColors() const noexcept { return m_ContextColors; }

	bool EnableTtfPolyDraw() const noexcept { return m_TtfPolyDraw; }

	bool EnableTtfTextOut() const noexcept { return m_TtfTextOut; }

	bool EnableTtfCache() const noexcept { return m_TtfCache; }

	bool EnableDynamicSubEntHlt() const noexcept { return m_DynamicSubEntHlt; }

	bool EnableGdiGradientsAsBitmap() const noexcept { return m_GdiGradientsAsBitmap; }

	bool EnableGdiGradientsAsPolys() const noexcept { return m_GdiGradientsAsPolys; }

	unsigned char GdiGradientsAsPolysThreshold() const noexcept { return m_NGdiGradientsAsPolysThreshold; }

	bool DisableAutoRegenerate() const noexcept { return m_DisableAutoRegenerate; }

	bool DiscardBackFaces() const noexcept { return m_DiscardBackFaces; }

	enum DisplayFields { kSchemaFields, kDxfFields, kDwgFields };

	int displayFields {0};
	bool saveRoundTrip {true};
	bool savePreview {false};
	bool saveWithPassword {false};
	EoDlgAudit* auditDialog {nullptr};
	//CTaskBarWin7Ext m_tbExt;
	OdMutexPtr meterMutex;

	AeSys() noexcept;

	OdString RecentGsDevicePath() const;

	void SetRecentGsDevicePath(const OdString& vectorizerPath);

	//void setStatusText(const wchar_t* msg);
	//void setStatusText(int nCol, const wchar_t* msg);
	void SetStatusPaneTextAt(int index, const wchar_t* newText);

	void addRef() noexcept override {
	}

	void release() noexcept override {
	}

	OdDbHostAppProgressMeter* newProgressMeter() override;

	void start(const OdString& displayString = OdString::kEmpty) override;

	void stop() override;

	void meterProgress() override;

	void setLimit(int max) noexcept override;

	void warning(const char* warnVisGroup, const OdString& text) override;

	OdRxClass* databaseClass() const override;

	OdString findFile(const OdString& fileToFind, OdDbBaseDatabase* database = nullptr, FindFileHint hint = kDefault) override;

	OdString getFontMapFileName() const override;

	OdString getSubstituteFont(const OdString& fontName, OdFontType fontType) override;

	const OdString product() override;

	OdString getTempPath() const override;

	BOOL ProcessShellCommand(CCommandLineInfo& commandLineInfo); // hides non-virtual function of parent
	static void InitPlotStyleSheetEnv();

	BOOL InitInstance() override;

	int ExitInstance() override;

	BOOL OnIdle(long count) override;

	bool getSAVEROUNDTRIP() const noexcept override { return saveRoundTrip; }

	void auditPrintReport(OdAuditInfo* auditInfo, const OdString& line, int printDest) const override;

	OdDbUndoControllerPtr newUndoController() override;

	OdStreamBufPtr newUndoStream() override;

	//	void OnOptionsRenderingDeviceVectorize();
	bool GetSavePreview() const noexcept { return savePreview; }

	bool GetSaveWithPassword() const noexcept { return saveWithPassword; }

	void SetRecentCommand(const OdString& command);

	const OdString& GetRecentCommand() const noexcept { return m_RecentCommand; }

	static OdString ObjectIdAndClassName(const OdDbObjectId id) {
		return ObjectIdAndClassName(id.openObject());
	}

	static OdString ObjectIdAndClassName(const OdDbObject* object) {
		if (object != nullptr) {
			return OdString().format(L"%02I64X : <%ls>", OdUInt64(object->objectId().getHandle()), object->isA()->name().c_str());
		}
		return OdString(L"00 : < >");
	}

	ODCOLORREF ActiveBackground() const noexcept { return m_BackgroundColor; }

	void SetActiveBackground(const ODCOLORREF& color) noexcept { m_BackgroundColor = color & 0xffffffU; }

	const ODCOLORREF* CurrentPalette() const;

	OdGsDevicePtr gsBitmapDevice(OdRxObject* view = nullptr, OdDbBaseDatabase* database = nullptr, unsigned long flags = 0) override;

	//	bool encryptData(OdBinaryData& buffer, const OdSecurityParams* securityParams);
	//	bool decryptData(OdBinaryData& buffer, const OdSecurityParams* securityParams);
	bool getPassword(const OdString& drawingName, bool isXref, OdPassword& password) override;

	OdDbPageControllerPtr newPageController() override;

	unsigned SetPagingType(unsigned pagingType) noexcept;

	unsigned PagingType() const noexcept { return m_PagingType & 0x0fU; }

	bool SetUndoType(bool useTempFiles) noexcept;

	bool UndoType() const noexcept { return m_UseTempFiles; }

	OdString fileDialog(int flags, const OdString& prompt = OdString::kEmpty, const OdString& defExt = OdString::kEmpty, const OdString& fileName = OdString::kEmpty, const OdString& filter = OdString::kEmpty) override;

	BOOL PreTranslateMessage(MSG* message) override;

	bool RemoteGeometryViewer() const noexcept { return m_RemoteGeometryViewer; }

	void SetRemoteGeometryViewer() noexcept { m_RemoteGeometryViewer = true; }

	bool SupportFileSelectionViaDialog() const noexcept { return m_SupportFileSelectionViaDialog; }

	void SetSupportFileSelectionViaDialog(const bool supportFileSelectionViaDialog) noexcept { m_SupportFileSelectionViaDialog = supportFileSelectionViaDialog; }

	static CString GetApplicationPath();

	/// </Open Design Alliance>
	void PreLoadState() override;

	enum Units { kArchitecturalS = -1, kArchitectural, kEngineering, kFeet, kInches, kMeters, kMillimeters, kCentimeters, kDecimeters, kKilometers };

private:
	int m_ArchitecturalUnitsFractionPrecision {16};
	bool m_HighColorMode;
	bool m_ClipboardDataEoGroups {true};
	bool m_ClipboardDataImage {false};
	bool m_ClipboardDataText {true};
	unsigned m_ClipboardFormatIdentifierForEoGroups {0};
	bool m_ModeInformationOverView {false};
	bool m_TrapModeAddGroups {true};
	unsigned m_CurrentMode {0};
	double m_DeviceHeightInMillimeters {0.0};
	double m_DeviceHeightInPixels {0.0};
	double m_DeviceWidthInMillimeters {0.0};
	double m_DeviceWidthInPixels {0.0};
	double m_DimensionAngle {45.0};
	double m_DimensionLength {0.125};
	double m_EngagedAngle {0.0};
	double m_EngagedLength {0.0};
	OdGePoint3d m_HomePoints[9];
	HMENU m_AeSysMenuHandle {nullptr};
	int m_ModeResourceIdentifier {0};
	int m_PrimaryMode {0};
	CString m_ShadowFolderPath;
	char* m_SimplexStrokeFont {nullptr};
	short m_TrapHighlightColor {15};
	bool m_TrapHighlighted {true};
	Units m_Units {kInches};
public:
	static CString customLButtonDownCharacters;
	static CString customLButtonUpCharacters;
	static CString customRButtonDownCharacters;
	static CString customRButtonUpCharacters;
	bool nodalModeAddGroups {true};
	EoApOptions applicationOptions;

	void AddModeInformationToMessageList() const;

	static void AddStringToMessageList(const wchar_t* message);

	void AddStringToMessageList(const wchar_t* message, const wchar_t* string) const;

	void AddStringToMessageList(unsigned stringResourceIdentifier) const;

	void AddStringToMessageList(unsigned stringResourceIdentifier, const wchar_t* string) const;

	static void AddStringToReportList(const wchar_t* message);

	static int ConfirmMessageBox(unsigned stringResourceIdentifier, const wchar_t* string);

	static void WarningMessageBox(unsigned stringResourceIdentifier);

	static void WarningMessageBox(unsigned stringResourceIdentifier, const wchar_t* string);

	void ErrorMessageBox(const wchar_t* caption, const OdError& error);

	int ArchitecturalUnitsFractionPrecision() const noexcept;

	void BuildModeSpecificAcceleratorTable() const;

	unsigned ClipboardFormatIdentifierForEoGroups() const noexcept;

	unsigned CurrentMode() const noexcept;

	double DeviceHeightInMillimeters() const noexcept;

	double DeviceHeightInPixels() const noexcept;

	double DeviceWidthInMillimeters() const noexcept;

	double DeviceWidthInPixels() const noexcept;

	double DimensionAngle() const noexcept;

	double DimensionLength() const noexcept;

	static void EditColorPalette();

	double EngagedAngle() const noexcept;

	double EngagedLength() const noexcept;

	static CString FormatAngle(double angle, int width = 8, int precision = 3);

	CString FormatLength(double length, Units units, int width = 16, int precision = 8) const;

	void FormatLengthStacked(wchar_t* lengthAsString, unsigned bufSize, Units units, double length, int width, int precision) const;

	static OdGePoint3d GetCursorPosition();

	static EoDb::FileTypes GetFileType(const OdString& file);

	static COLORREF GetHotColor(short colorIndex) noexcept;

	HINSTANCE GetInstance() const noexcept;

	HMENU GetAeSysMenu() const noexcept;

	HMENU GetAeSysSubMenu(int position) const noexcept;

	Units GetUnits() const noexcept;

	int GreatestCommonDivisor(int number1, int number2) const noexcept;

	bool HighColorMode() const noexcept;

	OdGePoint3d HomePointGet(int i) noexcept;

	void HomePointSave(int i, const OdGePoint3d& point) noexcept;

	void InitializeGlobals(CDC* deviceContext);

	bool InitializeOda();

	bool IsClipboardDataGroups() const noexcept;

	bool IsClipboardDataImage() const noexcept;

	bool IsClipboardDataText() const noexcept;

	bool IsTrapHighlighted() const noexcept;

	static void LoadColorPalletFromFile(const CString& fileName);

	void LoadModeResources(unsigned mode);

	static void LoadPenWidthsFromFile(const CString& fileName);

	void LoadSimplexStrokeFont(const CString& pathName);

	static CString LoadStringResource(unsigned resourceIdentifier);

	bool ModeInformationOverView() const noexcept;

	static double ParseLength(const wchar_t* lengthAsString);

	static double ParseLength(Units units, const wchar_t* lengthAsString);

	static double PenWidthsGet(short colorIndex) noexcept;

	int PrimaryMode() const noexcept;

	void ReleaseSimplexStrokeFont() const noexcept;

	static CString ResourceFolderPath();

	void SetArchitecturalUnitsFractionPrecision(int precision) noexcept;

	void SetDimensionAngle(double angle) noexcept;

	void SetDimensionLength(double length) noexcept;

	void SetEngagedAngle(double angle) noexcept;

	void SetEngagedLength(double length) noexcept;

	int SetShadowFolderPath(const CString& folder);

	void SetUnits(Units units) noexcept;

	CString ShadowFolderPath() const noexcept;

	char* SimplexStrokeFont() const noexcept;

	short TrapHighlightColor() const noexcept;

	static void UninitializeTeigha();

	static void UpdateMdiTabs(BOOL resetMdiChild);

	void OnAppAbout();

	void OnEditCfGroups() noexcept;

	void OnUpdateEditCfGroups(CCmdUI* commandUserInterface);

	void OnEditCfImage() noexcept;

	void OnUpdateEditCfImage(CCmdUI* commandUserInterface);

	void OnEditCfText() noexcept;

	void OnUpdateEditCfText(CCmdUI* commandUserInterface);

	void OnFileOpen(); // hides non-virtual function of parent
	void OnFilePlotStyleManager();

	void OnHelpContents();

	void OnModeAnnotate();

	void OnUpdateModeAnnotate(CCmdUI* commandUserInterface);

	void OnModeCut();

	void OnUpdateModeCut(CCmdUI* commandUserInterface);

	void OnModeDimension();

	void OnUpdateModeDimension(CCmdUI* commandUserInterface);

	void OnModeDraw();

	void OnUpdateModeDraw(CCmdUI* commandUserInterface);

	void OnModeDraw2();

	void OnUpdateModeDraw2(CCmdUI* commandUserInterface);

	void OnModeEdit();

	void OnUpdateModeEdit(CCmdUI* commandUserInterface);

	void OnModeFixup();

	void OnUpdateModeFixup(CCmdUI* commandUserInterface);

	void OnModeLetter();

	void OnModeLpd();

	void OnUpdateModeLpd(CCmdUI* commandUserInterface);

	void OnModeNodal();

	void OnUpdateModeNodal(CCmdUI* commandUserInterface);

	void OnModePipe();

	void OnUpdateModePipe(CCmdUI* commandUserInterface);

	void OnModePower();

	void OnUpdateModePower(CCmdUI* commandUserInterface);

	void OnModeRevise();

	void OnModeTrap();

	void OnUpdateModeTrap(CCmdUI* commandUserInterface);

	void OnToolsLoadApplications();

	void OnTrapCommandsAddGroups();

	void OnUpdateTrapCommandsAddGroups(CCmdUI* commandUserInterface);

	void OnTrapCommandsHighlight() noexcept;

	void OnUpdateTrapCommandsHighlight(CCmdUI* commandUserInterface);

	void OnViewModeInformation();

	void OnUpdateViewModeInformation(CCmdUI* commandUserInterface);

	void OnVectorizerTypeAddVectorizerDll();

	void OnUpdateVectorizerTypeAddVectorizerDll(CCmdUI* commandUserInterface);

	void OnVectorizerTypeClearMenu();

	void OnUpdateVectorizerTypeClearMenu(CCmdUI* commandUserInterface);

DECLARE_MESSAGE_MAP()
#pragma warning (suppress: 4266) // (level 4) 'function': no override available for virtual member function from base 'type'; function is hidden
};

extern AeSys theApp;

COLORREF AppGetTextCol() noexcept;
