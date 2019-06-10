#pragma once

#ifndef APSTUDIO_INVOKED
#define DOUNDEF_APSTUDIO_INVOKED
#endif
#ifdef APSTUDIO_READONLY_SYMBOLS
#define DODEF_APSTUDIO_READONLY_SYMBOLS
#endif

#define APSTUDIO_INVOKED
#undef APSTUDIO_READONLY_SYMBOLS
#include "resource.h"       // main symbols

#ifdef DOUNDEF_APSTUDIO_INVOKED
#undef APSTUDIO_INVOKED
#undef DOUNDEF_APSTUDIO_INVOKED
#endif
#ifdef DODEF_APSTUDIO_READONLY_SYMBOLS
#define APSTUDIO_READONLY_SYMBOLS
#undef DODEF_APSTUDIO_READONLY_SYMBOLS
#endif

#include "MainFrm.h"

#include "ColorMapping.h"
#include "EoApOptions.h"

#include "EoDb.h"

extern COLORREF ViewBackgroundColor;
extern COLORREF RubberbandColor;

extern COLORREF ColorPalette[256];
extern COLORREF GreyPalette[16];

extern COLORREF* pColTbl;

extern double dPWids[];

/// <Open Design Alliance>

#include "OdToolkit.h"
#include "DbObjectId.h"
#include "RxObjectImpl.h"
#include "ExHostAppServices.h"
#include "ExSystemServices.h"
#include "DynamicLinker.h"
#include "DbObject.h"
#include "Gs/Gs.h"
#include "Ed/EdCommandStack.h"
//#include "TaskBarWin7Ext.h"
#include "ThreadsCounter.h"

class EoDlgAudit;

#include "ExDbCommandContext.h"
#include "OdApplication.h"

class UserBreak {
};

class AeSys
	: public CWinAppEx
	, public ExSystemServices
	, public ExHostAppServices {
protected:
	using CWinAppEx::operator new;
	using CWinAppEx::operator delete;

private:
	int m_ProgressLimit;
	int m_ProgressPosition;
	int m_ProgressPercent;
	CString m_ProgressMessage;

	bool m_DiscardBackFaces;
	bool m_EnableDoubleBuffer;
	bool m_BlocksCache;
	bool m_GsDevMultithread;
	unsigned m_nMtRegenThreads;
	bool m_EnablePrintPreviewViaBitmap;
	bool m_UseGsModel;
	bool m_EnableHLR;
	bool m_ContextColors;
	bool m_TTFPolyDraw;
	bool m_TTFTextOut;
	bool m_TTFCache;
	bool m_DynamicSubEntHlt;
	bool m_GDIGradientsAsBitmap;
	bool m_GDIGradientsAsPolys;
	unsigned char m_nGDIGradientsAsPolysThreshold;
	bool m_DisableAutoRegen;
	ODCOLORREF m_background;
	unsigned long m_thisThreadID;
	unsigned m_numCustomCommands;
	unsigned long m_numGSMenuItems;
	OdString m_sVectorizerPath;
	OdString m_RecentCommand;
	bool m_bPartial;
	bool m_bRecover;
	bool m_bLoading;

	bool m_bUseMTLoading;

	bool m_bRemoteGeomViewer;
	int m_pagingType;
	bool m_bUseTempFiles;
	CStringArray m_tempFilesList;
	bool m_bSupportFileSelectionViaDialog;

	//	void UpdateFieldDisplay();
public:
	unsigned m_ApplicationLook;

	void AddReactor(const OdApplicationReactor* reactor);
	void RemoveReactor(const OdApplicationReactor* reactor);
	std::vector<OdSmartPtr<OdApplicationReactor>> m_ApplicationReactors;

	OdDbDatabasePtr openFile(const wchar_t* pathName);
	void setPartialOption(bool partial) noexcept { m_bPartial = partial; }
	void setRecoverOption(bool recover) noexcept { m_bRecover = recover; }

	void setMTLoadingOption(bool useMTLoading) noexcept { m_bUseMTLoading = useMTLoading; }

	OdGsMarker getGSMenuItemMarker() const noexcept { return ( OdGsMarker) this; }
	CMenu* CommandMenu(CMenu** ppEditMenu = 0);
	void RefreshCommandMenu();
	unsigned numCustomCommands() const noexcept { return m_numCustomCommands; }
	static CString BrowseWithPreview(HWND parentWindow, const wchar_t* filter, bool multiple = false);

	bool printingViaBitmap() const noexcept { return m_EnablePrintPreviewViaBitmap; }
	bool doubleBufferEnabled() const noexcept { return m_EnableDoubleBuffer; }
	bool blocksCacheEnabled() const noexcept { return m_BlocksCache; }
	bool gsDeviceMultithreadEnabled() const noexcept { return m_GsDevMultithread; }
	unsigned mtRegenThreadsCount() const noexcept { return m_nMtRegenThreads; }
	bool useGsModel() const noexcept { return m_UseGsModel; }
	bool useSoftwareHLR() const noexcept { return m_EnableHLR; }
	bool enableContextualColors() const noexcept { return m_ContextColors; }
	bool enableTTFPolyDraw() const noexcept { return m_TTFPolyDraw; }
	bool enableTTFTextOut() const noexcept { return m_TTFTextOut; }
	bool enableTTFCache() const noexcept { return m_TTFCache; }
	bool enableDynamicSubEntHlt() const noexcept { return m_DynamicSubEntHlt; }
	bool enableGDIGradientsAsBitmap() const noexcept { return m_GDIGradientsAsBitmap; }
	bool enableGDIGradientsAsPolys() const noexcept { return m_GDIGradientsAsPolys; }
	unsigned char gdiGradientsAsPolysThreshold() const noexcept { return m_nGDIGradientsAsPolysThreshold; }
	bool disableAutoRegen() const noexcept { return m_DisableAutoRegen; }
	bool discardBackFaces() const noexcept { return m_DiscardBackFaces; }
	enum DisplayFields {
		kSchemaFields,
		kDxfFields,
		kDwgFields
	};
	int m_displayFields;
	bool m_SaveRoundTrip;
	bool m_SavePreview;
	bool m_SaveWithPassword;

	EoDlgAudit* m_pAuditDlg;
	//	CTaskBarWin7Ext m_tbExt;
	OdMutexPtr m_pMeterMutex;

public:
	AeSys() noexcept;

	OdString recentGsDevicePath() const;
	void setRecentGsDevicePath(const OdString& vectorizerPath);

	//	void setStatusText(const wchar_t* msg);
	//	void setStatusText(int nCol, const wchar_t* msg);

	void SetStatusPaneTextAt(int index, const wchar_t* newText);

	void addRef() noexcept override /* ExHostAppServices */ {}
	void release() noexcept override /* ExHostAppServices */ {}

	OdDbHostAppProgressMeter* newProgressMeter() override /* ExHostAppServices */;

	void start(const OdString& displayString = OdString::kEmpty) override /* ExHostAppServices */;

	void stop() override /* ExHostAppServices */;

	void meterProgress() override /* ExHostAppServices */;

	void setLimit(int max) noexcept override /* ExHostAppServices */;

	void warning(const char* warnVisGroup, const OdString& message);
	
	static int messageBox(HWND parent, const wchar_t* caption, const wchar_t* text, unsigned type) noexcept {
		return ::MessageBox(parent, text, caption, type);
	}
	
	int messageBox(const wchar_t* caption, const wchar_t* text, unsigned type) {
		auto MainWindow {GetMainWnd()};
		
		if (MainWindow == nullptr) { return 0; }

		return messageBox(MainWindow->m_hWnd, caption, text, type);
	}
	
	void reportError(HWND parent, const wchar_t* contextMessage, const OdError& error) {
		messageBox(parent, contextMessage, error.description(), MB_OK | MB_ICONERROR);
	}
	
	void reportError(const wchar_t* contextMessage, const OdError& error) {
		messageBox(contextMessage, error.description(), MB_OK | MB_ICONERROR);
	}
	
	void reportError(const wchar_t* contextMessage, unsigned error) {
		messageBox(contextMessage, getErrorDescription(error), MB_OK | MB_ICONERROR);
	}

	OdRxClass* databaseClass() const override /* ExHostAppServices */;
	OdString findFile(const OdString& fileToFind, OdDbBaseDatabase* database = NULL, OdDbBaseHostAppServices::FindFileHint hint = kDefault) override /* ExHostAppServices */;
	OdString getFontMapFileName() const override /* ExHostAppServices */;
	OdString getSubstituteFont(const OdString& fontName, OdFontType fontType) override /* ExHostAppServices */;
	const OdString product() override /* ExHostAppServices */;

	OdString getTempPath() const override /* ExSystemServices*/;

	BOOL ProcessShellCommand(CCommandLineInfo& rCmdInfo); // hides non-virtual function of parent
	
	void initPlotStyleSheetEnv();

	BOOL InitInstance() override /* CWinAppEx (CWinThread) */;
	int ExitInstance() override /* CWinAppEx (CWinThread) */;
	BOOL OnIdle(long count) override /* CWinAppEx (CWinThread) */;

	bool getSAVEROUNDTRIP() const noexcept override { return m_SaveRoundTrip; }
	void auditPrintReport(OdAuditInfo* auditInfo, const OdString& line, int printDest) const override /* ExHostAppServices */;
	OdDbUndoControllerPtr newUndoController() override /* ExHostAppServices */;
	OdStreamBufPtr newUndoStream() override /* ExHostAppServices */;

//	void OnOptionsRenderingdeviceVectorize();

	bool getSavePreview() noexcept { return m_SavePreview; }
	bool getSaveWithPassword() noexcept { return m_SaveWithPassword; }

	void SetRecentCommand(const OdString& command);
	const OdString& GetRecentCmd() noexcept { return m_RecentCommand; }

	static inline OdString objectIdAndClassName(OdDbObjectId id) {
		return objectIdAndClassName(id.openObject());
	}

	static inline OdString objectIdAndClassName(const OdDbObject* object) {
		if (object) {
			return OdString().format(L"%02I64X : <%ls>", OdUInt64(object->objectId().getHandle()), object->isA()->name().c_str());
		}
		return OdString(L"00 : < >");
	}

	const ODCOLORREF activeBackground() const noexcept { return m_background; }
	void setActiveBackground(const ODCOLORREF& color) noexcept { m_background = color & 0xffffff; }
	const ODCOLORREF* curPalette() const;

	OdGsDevicePtr gsBitmapDevice(OdRxObject* view = nullptr, OdDbBaseDatabase* database = nullptr, unsigned long flags = 0) override;

//	bool encryptData(OdBinaryData& buffer, const OdSecurityParams* securityParams);
//	bool decryptData(OdBinaryData& buffer, const OdSecurityParams* securityParams);
	bool getPassword(const OdString& dwgName, bool isXref, OdPassword& password) override;

	OdDbPageControllerPtr newPageController() override;
	int setPagingType(int pagingType) noexcept;
	int pagingType() const noexcept { return m_pagingType& 0x0f; }

	bool setUndoType(bool useTempFiles) noexcept;
	bool undoType() const noexcept { return m_bUseTempFiles; }

	OdString fileDialog(int flags, const OdString& prompt = OdString::kEmpty, const OdString& defExt = OdString::kEmpty, const OdString& fileName = OdString::kEmpty, const OdString& filter = OdString::kEmpty) override;

	BOOL PreTranslateMessage(MSG* message) override;

	bool remoteGeomViewer() const noexcept { return m_bRemoteGeomViewer; }
	void setRemoteGeomViewer() noexcept { m_bRemoteGeomViewer = true; }

	bool supportFileSelectionViaDialog() const noexcept { return m_bSupportFileSelectionViaDialog; }
	void setSupportFileSelectionViaDialog(bool supportFileSelectionViaDialog) noexcept { m_bSupportFileSelectionViaDialog = supportFileSelectionViaDialog; }

	static CString getApplicationPath();

/// </Open Design Alliance>

public:
	void PreLoadState() override;

	enum Units { kArchitecturalS = -1, kArchitectural, kEngineering, kFeet, kInches, kMeters, kMillimeters, kCentimeters, kDecimeters, kKilometers };

private:
	int	m_ArchitecturalUnitsFractionPrecision;
	bool m_ClipboardDataEoGroups;
	bool m_ClipboardDataImage;
	bool m_ClipboardDataText;
	unsigned m_ClipboardFormatIdentifierForEoGroups;
	int	m_CurrentMode;
	double m_DeviceHeightInMillimeters;
	double m_DeviceHeightInPixels;
	double m_DeviceWidthInMillimeters;
	double m_DeviceWidthInPixels;
	double m_DimensionAngle;
	double m_DimensionLength;
	double m_EngagedAngle;
	double m_EngagedLength;
	bool m_HighColorMode;
	OdGePoint3d	m_HomePoints[9];
	HMENU m_AeSysMenuHandle;
	bool m_ModeInformationOverView;
	int m_ModeResourceIdentifier;
	int	m_PrimaryMode;
	CString m_ShadowFolderPath;
	char* m_SimplexStrokeFont;
	short m_TrapHighlightColor;
	bool m_TrapHighlighted;
	bool m_TrapModeAddGroups;
	Units m_Units;

public:
	static CString CustomLButtonDownCharacters;
	static CString CustomLButtonUpCharacters;
	static CString CustomRButtonDownCharacters;
	static CString CustomRButtonUpCharacters;

	bool m_NodalModeAddGroups;
	EoApOptions m_Options;

public:
	void AddModeInformationToMessageList();
	void AddStringToMessageList(const wchar_t* message);
	void AddStringToMessageList(const wchar_t* message, const wchar_t* string);
	void AddStringToMessageList(unsigned stringResourceIdentifier);
	void AddStringToMessageList(unsigned stringResourceIdentifier, const wchar_t* string);
	void AddStringToReportList(const wchar_t* message);

	int ConfirmMessageBox(unsigned stringResourceIdentifier, const wchar_t* string);
	void WarningMessageBox(unsigned stringResourceIdentifier);
	void WarningMessageBox(unsigned stringResourceIdentifier, const wchar_t* string);

	int	ArchitecturalUnitsFractionPrecision() const noexcept;
	void BuildModeSpecificAcceleratorTable();
	unsigned ClipboardFormatIdentifierForEoGroups() noexcept;
	static OdString ConfigurationFileFor(HKEY key, const OdString& applicationName, const OdString& configType, OdString file);
	int CurrentMode() const noexcept;
	double DeviceHeightInMillimeters() const noexcept;
	double DeviceHeightInPixels() const noexcept;
	double DeviceWidthInMillimeters() const noexcept;
	double DeviceWidthInPixels() const noexcept;
	double DimensionAngle() const noexcept;
	double DimensionLength() const noexcept;
	void EditColorPalette();
	double EngagedAngle() const noexcept;
	double EngagedLength() const noexcept;
	CString FormatAngle(double angle, int width = 8, int precision = 3) const;
	CString FormatLength(double length, Units units, int width = 16, int precision = 8) const;
	
	void FormatLength_s(wchar_t* lengthAsString, const unsigned bufSize, Units units, const double length, const int width, const int precision) const;
	OdGePoint3d GetCursorPosition();
	static EoDb::FileTypes GetFileType(const OdString& file);
	COLORREF GetHotColor(short colorIndex) noexcept;
	HINSTANCE GetInstance() noexcept;
	HMENU GetAeSysMenu() noexcept;
	HMENU GetAeSysSubMenu(int position) noexcept;
	Units GetUnits() noexcept;
	
	int GreatestCommonDivisor(const int number1, const int number2) const noexcept;
	bool HighColorMode() const noexcept;
	OdGePoint3d HomePointGet(int i) noexcept;
	void HomePointSave(int i, const OdGePoint3d& point) noexcept;
	void InitGbls(CDC* deviceContext);
	bool InitializeOda();
	bool IsClipboardDataGroups() noexcept;
	bool IsClipboardDataImage() noexcept;
	bool IsClipboardDataText() noexcept;
	bool IsTrapHighlighted() noexcept;
	void LoadColorPalletFromFile(const CString& pathName);
	void LoadModeResources(int mode);
	void LoadPenWidthsFromFile(const CString& pathName);
	void LoadSimplexStrokeFont(const CString& pathName);
	CString LoadStringResource(unsigned resourceIdentifier) const;
	bool ModeInformationOverView() const noexcept;
	double ParseLength(const wchar_t* lengthAsString);
	double ParseLength(Units units, const wchar_t* lengthAsString);
	double PenWidthsGet(short colorIndex) noexcept;
	int PrimaryMode() const noexcept;
	void ReleaseSimplexStrokeFont() noexcept;
	static CString ResourceFolderPath();
	void SetArchitecturalUnitsFractionPrecision(const int precision) noexcept;
	void SetDimensionAngle(double angle) noexcept;
	void SetDimensionLength(double length) noexcept;
	void SetEngagedAngle(double angle) noexcept;
	void SetEngagedLength(double length) noexcept;
	int SetShadowFolderPath(const CString& folder);
	void SetUnits(Units units) noexcept;
	CString ShadowFolderPath() const noexcept;
	char* SimplexStrokeFont() noexcept;
	short TrapHighlightColor() const noexcept;
	void UninitializeTeigha();
	void UpdateMDITabs(BOOL resetMDIChild);

public:
	void OnAppAbout();
	void OnEditCfGroups() noexcept;
	void OnUpdateEditCfGroups(CCmdUI* pCmdUI);
	void OnEditCfImage() noexcept;
	void OnUpdateEditCfImage(CCmdUI* pCmdUI);
	void OnEditCfText() noexcept;
	void OnUpdateEditCfText(CCmdUI* pCmdUI);
	void OnFileOpen(); // hides non-virtual function of parent
	void OnFilePlotstylemanager();
	void OnHelpContents();
	void OnModeAnnotate();
	void OnUpdateModeAnnotate(CCmdUI* pCmdUI);
	void OnModeCut();
	void OnUpdateModeCut(CCmdUI* pCmdUI);
	void OnModeDimension();
	void OnUpdateModeDimension(CCmdUI* pCmdUI);
	void OnModeDraw();
	void OnUpdateModeDraw(CCmdUI* pCmdUI);
	void OnModeDraw2();
	void OnUpdateModeDraw2(CCmdUI* pCmdUI);
	void OnModeEdit();
	void OnUpdateModeEdit(CCmdUI* pCmdUI);
	void OnModeFixup();
	void OnUpdateModeFixup(CCmdUI* pCmdUI);
	void OnModeLetter();
	void OnModeLPD();
	void OnUpdateModeLpd(CCmdUI* pCmdUI);
	void OnModeNodal();
	void OnUpdateModeNodal(CCmdUI* pCmdUI);
	void OnModePipe();
	void OnUpdateModePipe(CCmdUI* pCmdUI);
	void OnModePower();
	void OnUpdateModePower(CCmdUI* pCmdUI);
	void OnModeRevise();
	void OnModeTrap();
	void OnUpdateModeTrap(CCmdUI* pCmdUI);
	void OnToolsLoadapplications();
	void OnTrapCommandsAddGroups();
	void OnUpdateTrapcommandsAddgroups(CCmdUI* pCmdUI);
	void OnTrapCommandsHighlight() noexcept;
	void OnUpdateTrapcommandsHighlight(CCmdUI* pCmdUI);
	void OnViewModeInformation();
	void OnUpdateViewModeinformation(CCmdUI* pCmdUI);
	void OnVectorizeAddVectorizerDLL();
	void OnUpdateVectorizeAddvectorizerdll(CCmdUI* pCmdUI);
	void OnVectorizeClearmenu();
	void OnUpdateVectorizeClearmenu(CCmdUI* pCmdUI);

protected:
	DECLARE_MESSAGE_MAP()
};
extern AeSys theApp;

COLORREF AppGetTextCol() noexcept;