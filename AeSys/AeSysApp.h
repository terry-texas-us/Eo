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

#include "OdToolkit.h"
#include "DbObjectId.h"
#include "RxObjectImpl.h"
#include "ExHostAppServices.h"
#include "ExSystemServices.h"
#include "ThreadsCounter.h"

#include "DbObject.h"
#include "Gs/Gs.h"
#include "Ed/EdCommandStack.h"

#ifdef ODAMFC_EXPORT_SYMBOL
#include "ExDbCommandContext.h"
#include "EoMfcExport.h"
#endif // ODAMFC_EXPORT_SYMBOL

#include "ColorMapping.h"
#include "EoApOptions.h"

extern COLORREF ViewBackgroundColor;
extern COLORREF RubberbandColor;

extern COLORREF ColorPalette[256];
extern COLORREF GreyPalette[16];

extern COLORREF* pColTbl;

extern double dPWids[];

class EoDlgAudit;

class UserBreak {
};

class AeSysApp : public CWinAppEx
	           , public ExSystemServices
			   , public ExHostAppServices
{
protected:
	using CWinAppEx::operator new;
	using CWinAppEx::operator delete;

private:
	int m_nProgressLimit;
	int m_nProgressPos;
	int m_nPercent;
	CString m_Msg;
	BOOL m_bDiscardBackFaces;
	BOOL m_bEnableDoubleBuffer;
	BOOL m_bBlocksCache;
	BOOL m_bGsDevMultithread;
	UINT m_nMtRegenThreads;
	BOOL m_bEnablePrintPreviewViaBitmap;
	BOOL m_bUseGsModel;
	BOOL m_bEnableHLR;
	BOOL m_bContextColors;
  	BOOL m_bTTFPolyDraw;
	BOOL m_bTTFTextOut;
	ODCOLORREF m_background;
	DWORD m_thisThreadID;
	UINT m_numCustomCommands;
	DWORD m_numGSMenuItems;

	OdString m_sVectorizerPath;

#ifdef DEV_COMMAND_CONSOLE
	OdString m_sRecentCmd;
#endif // DEV_COMMAND_CONSOLE

	bool m_bPartial;
	bool m_bRecover;
	bool m_bLoading;

	// ODA_MT_DBIO_BEGIN
	bool m_bUseMTLoading;
	// ODA_MT_DBIO_END

	bool m_bRemoteGeomViewer;
	int m_pagingType;
	bool m_bUseTempFiles;
	CStringArray m_tempFilesList;
	bool m_bSupportFileSelectionViaDialog;

#ifdef ODAMFC_EXPORT_SYMBOL
public:
	void AddReactor(EoApplicationReactor* reactor);
	void RemoveReactor(EoApplicationReactor* reactor);
	std::vector<OdSmartPtr<EoApplicationReactor>> m_aAppReactors;
#endif // ODAMFC_EXPORT_SYMBOL

public:
	OdDbDatabasePtr openFile(LPCWSTR pathName);
	void setPartialOption(bool partial) noexcept {m_bPartial = partial;}
	void setRecoverOption(bool recover) noexcept {m_bRecover = recover;}
	// ODA_MT_DBIO_BEGIN
	void setMTLoadingOption(bool useMTLoading) noexcept {m_bUseMTLoading = useMTLoading;}
	// ODA_MT_DBIO_END

public:
	OdGsMarker getGSMenuItemMarker() const noexcept {return (OdGsMarker) this;}
	CMenu* CommandMenu(CMenu** ppEditMenu = 0);
	void RefreshCommandMenu() noexcept;
	UINT numCustomCommands() const noexcept {return m_numCustomCommands;}
	static CString BrowseWithPreview(HWND parentWindow, LPCWSTR filter);

	bool printingViaBitmap() const noexcept {return m_bEnablePrintPreviewViaBitmap != 0;}
	bool doubleBufferEnabled() const noexcept {return m_bEnableDoubleBuffer != 0;}
	bool blocksCacheEnabled() const noexcept {return m_bBlocksCache != 0;}
	bool gsDeviceMultithreadEnabled() const noexcept {return m_bGsDevMultithread != 0;}
	UINT mtRegenThreadsCount() const noexcept {return m_nMtRegenThreads;}
	bool useGsModel() const noexcept {return m_bUseGsModel != 0;}
	bool useSoftwareHLR() const noexcept {return m_bEnableHLR != 0;}
	bool enableContextualColors() const noexcept {return m_bContextColors != 0;}
	bool enableTTFPolyDraw() const noexcept {return m_bTTFPolyDraw != 0;}
	bool enableTTFTextOut() const noexcept {return m_bTTFTextOut != 0;}
	bool discardBackFaces() const noexcept {return m_bDiscardBackFaces != 0;}

	BOOL m_isDwgOut;
	BOOL m_bSaveRoundTrip;
	BOOL m_bSavePreview;
	BOOL m_bSaveWithPassword;

	EoDlgAudit* m_pAuditDlg;
	//CTaskBarWin7Ext m_tbExt;
	OdMutexPtr m_pMeterMutex;

public:
	AeSysApp();
	~AeSysApp();

	OdString recentGsDevicePath() const;
	void setRecentGsDevicePath(const OdString& vectorizerPath);

	void SetStatusPaneTextAt(int index, LPCWSTR newText);

	void addRef() {}
	void release() {}

	OdDbHostAppProgressMeter* newProgressMeter();
	void start(const OdString& displayString = OdString::kEmpty);
	void stop();
	void meterProgress();
	void setLimit(int max) noexcept;

	int ConfirmMessageBox(UINT stringResourceIdentifier, LPCWSTR string);
	void warning(const char* warnVisGroup, const OdString& message);
	void WarningMessageBox(UINT stringResourceIdentifier);
	void WarningMessageBox(UINT stringResourceIdentifier, LPCWSTR string);

	int messageBox(LPCWSTR caption, LPCWSTR text, UINT type);
	void reportError(LPCWSTR caption, const OdError& error);
	void reportError(LPCWSTR caption, unsigned int error);

	OdRxClass* databaseClass() const;

	OdString findFile(const OdString& fileToFind, OdDbBaseDatabase* database = NULL, OdDbBaseHostAppServices::FindFileHint hint = kDefault);
	OdString getFontMapFileName() const;
	BOOL ProcessShellCommand(CCommandLineInfo& commandLineInfo);

	void initPlotStyleSheetEnv();

	bool getSAVEROUNDTRIP() const noexcept {return (m_bSaveRoundTrip != 0);}
	void auditPrintReport(OdAuditInfo* auditInfo, const OdString& line, int printDest) const;
	OdDbUndoControllerPtr newUndoController();
	virtual OdStreamBufPtr newUndoStream();

	bool getSavePreview() noexcept {return (m_bSavePreview != 0);}
	bool getSaveWithPassword() noexcept {return (m_bSaveWithPassword != 0);}

#ifdef DEV_COMMAND_CONSOLE
	void setRecentCmd(const OdString& cmd);
	const OdString& getRecentCmd() {return m_sRecentCmd;}
#endif // DEV_COMMAND_CONSOLE

	static inline OdString objectIdAndClassName(OdDbObjectId id) {
		return objectIdAndClassName(id.openObject());
	}
	static OdString objectIdAndClassName(const OdDbObject* object);
	const ODCOLORREF activeBackground() const noexcept {return m_background;}
	void setActiveBackground(const ODCOLORREF &color) noexcept {m_background = color & 0xffffff;}
	const ODCOLORREF* curPalette() const;

	OdGsDevicePtr gsBitmapDevice();

	bool getPassword(const OdString& dwgName, bool isXref, OdPassword& password);

	OdDbPageControllerPtr newPageController();
	int setPagingType(int pagingType) noexcept;
	int pagingType() const noexcept {return m_pagingType & 0x0f;}

	bool setUndoType(bool useTempFiles) noexcept;
	bool undoType() const noexcept {return m_bUseTempFiles;}

	OdString fileDialog(int flags, const OdString& prompt = OdString::kEmpty, const OdString& defExt = OdString::kEmpty, const OdString& fileName = OdString::kEmpty, const OdString& filter = OdString::kEmpty);

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	bool remoteGeomViewer() const noexcept {return m_bRemoteGeomViewer;}
	void setRemoteGeomViewer() noexcept {m_bRemoteGeomViewer = true;}

	bool supportFileSelectionViaDialog() const noexcept {return m_bSupportFileSelectionViaDialog;}
	void setSupportFileSelectionViaDialog(bool b) noexcept {m_bSupportFileSelectionViaDialog = b;}

	static CString getApplicationPath();
	
public:
	virtual BOOL InitInstance(void);
	virtual int ExitInstance(void);
	virtual BOOL OnIdle(long count);

	enum Units {
		kArchitecturalS = - 1, // Embedded S format
		kArchitectural,
		kEngineering,
		kFeet,
		kInches,
		kMeters,
		kMillimeters,
		kCentimeters,
		kDecimeters,
		kKilometers
	};

private:
	int	m_ArchitecturalUnitsFractionPrecision;
	bool m_ClipboardDataEoGroups;
	bool m_ClipboardDataImage;
	bool m_ClipboardDataText;
	UINT m_ClipboardFormatIdentifierForEoGroups;
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
	CMultiDocTemplate* m_PegDocTemplate;
	int	m_PrimaryMode;
	CString m_ShadowFolderPath;
	char* m_SimplexStrokeFont;
	CMultiDocTemplate* m_TracingDocTemplate;
	OdInt16 m_TrapHighlightColor;
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
	void AddStringToMessageList(LPCWSTR message);
	void AddStringToMessageList(LPCWSTR message, LPCWSTR string);
	void AddStringToMessageList(UINT stringResourceIdentifier);
	void AddStringToMessageList(UINT stringResourceIdentifier, LPCWSTR string);
	void AddStringToReportList(LPCWSTR message);

	int	ArchitecturalUnitsFractionPrecision() const noexcept;
	void BuildModeSpecificAcceleratorTable(void);
	UINT ClipboardFormatIdentifierForEoGroups() noexcept;
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
	/// <summary>
	///Produces a string formatted to type units from a "length" value
	///ArchitecturalS units formatted as follows:
	///	\S[feet]'[inches].[fraction numerator]/[fraction denominator];"
	///Architectural units formatted as follows:
	///	[feet]'[inches].[fraction numerator] [fraction denominator]"
	///Engineering units formatted as follows:
	///	[feet]'[inches].[decimal inches]"
	///All other units formatted using floating decimal.
	/// </summary>
	void FormatLength_s(LPWSTR lengthAsString, const int bufSize, Units units, const double length, const int width, const int precision) const;
	OdGePoint3d GetCursorPosition();
	static EoDb::FileTypes GetFileTypeFromPath(const OdString& pathName);
	COLORREF GetHotColor(OdInt16 colorIndex) noexcept;
	HINSTANCE GetInstance() noexcept;
	HWND GetSafeHwnd();
	HMENU GetAeSysMenu() noexcept;
	HMENU GetAeSysSubMenu(int position) noexcept;
	Units GetUnits() noexcept;
	/// <summary>Finds the greatest common divisor of arbitrary integers.</summary>
	/// <returns>First number if second number is zero, greatest common divisor otherwise.</returns>
	int GreatestCommonDivisor(const int number1, const int number2) const noexcept;
	bool HighColorMode() const noexcept;
	OdGePoint3d HomePointGet(int i) noexcept;
	void HomePointSave(int i, const OdGePoint3d& point) noexcept;
	void InitGbls(CDC* deviceContext);
	BOOL InitializeTeigha();
	bool IsClipboardDataGroups() noexcept;
	bool IsClipboardDataImage() noexcept;
	bool IsClipboardDataText() noexcept;
	bool IsTrapHighlighted() noexcept;
	void LoadColorPalletFromFile(const CString& pathName);
	void LoadHatchesFromFile(const CString& strFileName);
	void LoadModeResources(int mode);
	void LoadPenWidthsFromFile(const CString& pathName);
	void LoadSimplexStrokeFont(const CString& pathName);
	CString LoadStringResource(UINT resourceIdentifier) const;
	bool ModeInformationOverView() const noexcept;
	double ParseLength(LPWSTR lengthAsString);
	double ParseLength(Units units, LPWSTR);
	double PenWidthsGet(OdInt16 colorIndex) noexcept;
	virtual void PreLoadState();
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
	OdInt16 TrapHighlightColor() const noexcept;
	void UninitializeTeigha() noexcept;
	void UpdateMDITabs(BOOL resetMDIChild);

public:
	afx_msg void OnAppAbout(void);
	afx_msg void OnEditCfGroups() noexcept;
	afx_msg void OnEditCfImage() noexcept;
	afx_msg void OnEditCfText() noexcept;
	afx_msg void OnFileOpen(void);
	afx_msg void OnFilePlotstylemanager();
	afx_msg void OnHelpContents();
	afx_msg void OnModeAnnotate();
	afx_msg void OnModeCut();
	afx_msg void OnModeDimension();
	afx_msg void OnModeDraw();
	afx_msg void OnModeDraw2();
	afx_msg void OnModeEdit();
	afx_msg void OnModeFixup();
	afx_msg void OnModeLetter();
	afx_msg void OnModeLPD();
	afx_msg void OnModeNodal();
	afx_msg void OnModePipe();
	afx_msg void OnModePower();
	afx_msg void OnModeRevise();
	afx_msg void OnModeTrap();
	afx_msg void OnToolsLoadapplications();
	afx_msg void OnTrapCommandsAddGroups();
	afx_msg void OnTrapCommandsHighlight() noexcept;
	afx_msg void OnUpdateEditCfGroups(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditCfImage(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditCfText(CCmdUI *pCmdUI);
	afx_msg void OnUpdateModeAnnotate(CCmdUI *pCmdUI);
	afx_msg void OnUpdateModeCut(CCmdUI *pCmdUI);
	afx_msg void OnUpdateModeDimension(CCmdUI *pCmdUI);
	afx_msg void OnUpdateModeDraw(CCmdUI *pCmdUI);
	afx_msg void OnUpdateModeDraw2(CCmdUI *pCmdUI);
	afx_msg void OnUpdateModeEdit(CCmdUI *pCmdUI);
	afx_msg void OnUpdateModeFixup(CCmdUI *pCmdUI);
	afx_msg void OnUpdateModeLpd(CCmdUI *pCmdUI);
	afx_msg void OnUpdateModeNodal(CCmdUI *pCmdUI);
	afx_msg void OnUpdateModePipe(CCmdUI *pCmdUI);
	afx_msg void OnUpdateModePower(CCmdUI *pCmdUI);
	afx_msg void OnUpdateModeTrap(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTrapcommandsAddgroups(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTrapcommandsHighlight(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewModeinformation(CCmdUI *pCmdUI);
	afx_msg void OnViewModeInformation();
	afx_msg void OnVectorizeAddVectorizerDLL();
	afx_msg void OnVectorizeClearmenu();
	afx_msg void OnUpdateVectorizeClearmenu(CCmdUI* pCmdUI);
	afx_msg void OnUpdateVectorizeAddvectorizerdll(CCmdUI *pCmdUI);

protected:
	DECLARE_MESSAGE_MAP()
};
extern AeSysApp theApp;

COLORREF AppGetTextCol() noexcept;