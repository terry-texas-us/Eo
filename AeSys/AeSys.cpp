#include "stdafx.h"

//#include "afxwinappex.h"

#include "ChildFrm.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

#include "DbLayoutPaperPE.h"
#include "RxDynamicModule.h"
#include "OdStreamBuf.h"

#include "Gi/GiWorldDraw.h"
#include "Gi/GiMaterialItem.h"

#include "ExFileUndoController.h"
#include "ExPageController.h"
#include "ExUndoController.h"
#include "MemFileStreamImpl.h"

#include "FileDlgExt.h"

#include "EoAppAuditInfo.h"
#include "EoDbHatchPatternTable.h"
#include "EoDlgAudit.h"
#include "EoDlgModeLetter.h"
#include "EoDlgModeRevise.h"
#include "EoDlgPassword.h"
#include "EoDlgPlotStyleTableEditor.h"
#include "EoLoadApps.h"
#include "EoPreviewDib.h"
#include "Lex.h"

#include "EoDbHatch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ATOM WINAPI RegisterPreviewWindowClass(HINSTANCE instance);

double dPWids[] = {
	0.0, 0.0075, 0.015, 0.02, 0.03, 0.0075, 0.015, 0.0225, 0.03, 0.0075, 0.015, 0.0225, 0.03, 0.0075, 0.015, 0.0225};

#include "PegColors.h"

COLORREF* pColTbl = ColorPalette;

CPrimState pstate;

// This is a legacy feature. All values are empty strings now for normal MouseButton command processing.
// User may still change through user interface, so must not assume empty.

CString AeSys::CustomLButtonDownCharacters(L"");
CString AeSys::CustomLButtonUpCharacters(L"" /* L"{13}" for VK_RETURN */);
CString AeSys::CustomRButtonDownCharacters(L"");
CString AeSys::CustomRButtonUpCharacters(L"" /* L"{27}" for VK_ESCAPE */);

#ifdef OD_OLE_SUPPORT
void rxInit_COleClientItem_handler();
void rxUninit_COleClientItem_handler();
#endif // OD_OLE_SUPPORT

OdStaticRxObject<Cmd_VIEW> g_Cmd_VIEW;
OdStaticRxObject<Cmd_SELECT> g_Cmd_SELECT;

static void addPaperDrawingCustomization() {
	static class OdDbLayoutPaperPEImpl : public OdStaticRxObject<OdDbLayoutPaperPE> {
	   public:
		bool drawPaper(const OdDbLayout*, OdGiWorldDraw* worldDraw, OdGePoint3d* points) override {
			worldDraw->geometry().polygon(4, points);
			return true;
		}
		bool drawBorder(const OdDbLayout*, OdGiWorldDraw* worldDraw, OdGePoint3d* points) override {
			worldDraw->geometry().polygon(4, points);
			return true;
		}
		bool drawMargins(const OdDbLayout*, OdGiWorldDraw* worldDraw, OdGePoint3d* points) override {
			if (points[0] == points[1] || points[1] == points[2]) { return true; }

			int NumberOfDashes {15};
			OdGiGeometry& Geometry {worldDraw->geometry()};
			OdGePoint3d Dash1[2];
			OdGePoint3d Dash2[2];
			auto Step {(points[1] - points[0]) / (static_cast<double>(NumberOfDashes) * 2. + 1.0)};
			Dash1[0] = points[0];
			Dash2[0] = points[2];
			for (int i = 0; i <= NumberOfDashes; ++i) {
				Dash1[1] = Dash1[0] + Step;
				Geometry.polyline(2, Dash1);
				Dash1[0] = Dash1[1] + Step;
				Dash2[1] = Dash2[0] - Step;
				Geometry.polyline(2, Dash2);
				Dash2[0] = Dash2[1] - Step;
			}
			NumberOfDashes = static_cast<int>((points[2] - points[1]).length() / Step.length() - 1.0) / 2;
			Step = (points[2] - points[1]) / (static_cast<double>(NumberOfDashes) * 2. + 1.0);
			Dash1[0] = points[1];
			Dash2[0] = points[3];
			for (int i = 0; i <= NumberOfDashes; ++i) {
				Dash1[1] = Dash1[0] + Step;
				Geometry.polyline(2, Dash1);
				Dash1[0] = Dash1[1] + Step;
				Dash2[1] = Dash2[0] - Step;
				Geometry.polyline(2, Dash2);
				Dash2[0] = Dash2[1] - Step;
			}
			return true;
		}
	} s_PaperDrawExt;

	OdDbLayout::desc()->addX(OdDbLayoutPaperPE::desc(), &s_PaperDrawExt);
}
static void removePaperDrawingCustomization() {
	OdDbLayout::desc()->delX(OdDbLayoutPaperPE::desc());
}

/// <section="Material textures loading monitor protocol extension">

static void addMaterialTextureLoadingMonitor() {
	static class OdGiMaterialTextureLoadPEImpl : public OdStaticRxObject<OdGiMaterialTextureLoadPE> {
	   public:
		void startTextureLoading(OdString& fileName, OdDbBaseDatabase* database) noexcept override {
			// Material texture to be loaded. Correct loading path here.
		}
		void textureLoaded(const OdString& fileName, OdDbBaseDatabase* database) override {
			TRACE1("Material texture loaded: %s\n", fileName.c_str());
		}
		/// <remarks> Called by texture loader after file loading, only if texture loading failed. </remarks>
		void textureLoadingFailed(const OdString& fileName, OdDbBaseDatabase* database) override {
			TRACE1("Failed to load material texture: %s\n", fileName.c_str());
		}
	} s_MatLoadExt;

	OdGiMaterialTextureEntry::desc()->addX(OdGiMaterialTextureLoadPE::desc(), &s_MatLoadExt);
}

void removeMaterialTextureLoadingMonitor() {
	OdGiMaterialTextureEntry::desc()->delX(OdGiMaterialTextureLoadPE::desc());
}
/// </section>

#include "DbLibraryInfo.h"
#include "summinfo.h"

class EoDlgAbout : public CDialog {
   public:
	EoDlgAbout() noexcept;

	enum { IDD = IDD_ABOUTBOX };

   protected:
	void DoDataExchange(CDataExchange* pDX) final;

   protected:
	DECLARE_MESSAGE_MAP()
   public:
	BOOL OnInitDialog() final;
};

EoDlgAbout::EoDlgAbout() noexcept
	: CDialog(EoDlgAbout::IDD) {
}

BOOL EoDlgAbout::OnInitDialog() {
	auto LibraryInfo {oddbGetLibraryInfo()};

	auto BuildComments {LibraryInfo->getBuildComments()};
	auto CopyRight {LibraryInfo->getCopyright()};

	CDialog::OnInitDialog();

	SetDlgItemTextW(IDC_INFO_BUILD, BuildComments);

	return TRUE; // return TRUE unless you set the focus to a control
		// EXCEPTION: OCX Property Pages should return FALSE
}

void EoDlgAbout::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(EoDlgAbout, CDialog)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(AeSys, CWinAppEx)
ON_COMMAND(ID_APP_ABOUT, &AeSys::OnAppAbout)
// Standard file based document commands
ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
ON_COMMAND(ID_FILE_OPEN, &AeSys::OnFileOpen)
// Standard print setup command
ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)

ON_COMMAND(ID_EDIT_CF_GROUPS, &AeSys::OnEditCfGroups)
ON_UPDATE_COMMAND_UI(ID_EDIT_CF_GROUPS, &AeSys::OnUpdateEditCfGroups)
ON_COMMAND(ID_EDIT_CF_IMAGE, &AeSys::OnEditCfImage)
ON_UPDATE_COMMAND_UI(ID_EDIT_CF_IMAGE, &AeSys::OnUpdateEditCfImage)
ON_COMMAND(ID_EDIT_CF_TEXT, &AeSys::OnEditCfText)
ON_UPDATE_COMMAND_UI(ID_EDIT_CF_TEXT, &AeSys::OnUpdateEditCfText)
ON_COMMAND(ID_HELP_CONTENTS, &AeSys::OnHelpContents)
ON_COMMAND(ID_MODE_ANNOTATE, &AeSys::OnModeAnnotate)
ON_UPDATE_COMMAND_UI(ID_MODE_ANNOTATE, &AeSys::OnUpdateModeAnnotate)
ON_COMMAND(ID_MODE_CUT, &AeSys::OnModeCut)
ON_UPDATE_COMMAND_UI(ID_MODE_CUT, &AeSys::OnUpdateModeCut)
ON_COMMAND(ID_MODE_DIMENSION, &AeSys::OnModeDimension)
ON_UPDATE_COMMAND_UI(ID_MODE_DIMENSION, &AeSys::OnUpdateModeDimension)
ON_COMMAND(ID_MODE_DRAW, OnModeDraw)
ON_UPDATE_COMMAND_UI(ID_MODE_DRAW, &AeSys::OnUpdateModeDraw)
ON_COMMAND(ID_MODE_DRAW2, &AeSys::OnModeDraw2)
ON_UPDATE_COMMAND_UI(ID_MODE_DRAW2, &AeSys::OnUpdateModeDraw2)
ON_COMMAND(ID_MODE_EDIT, &AeSys::OnModeEdit)
ON_UPDATE_COMMAND_UI(ID_MODE_EDIT, &AeSys::OnUpdateModeEdit)
ON_COMMAND(ID_MODE_FIXUP, &AeSys::OnModeFixup)
ON_UPDATE_COMMAND_UI(ID_MODE_FIXUP, &AeSys::OnUpdateModeFixup)
ON_COMMAND(ID_MODE_LETTER, &AeSys::OnModeLetter)
ON_COMMAND(ID_MODE_LPD, &AeSys::OnModeLPD)
ON_UPDATE_COMMAND_UI(ID_MODE_LPD, &AeSys::OnUpdateModeLpd)
ON_COMMAND(ID_MODE_NODAL, &AeSys::OnModeNodal)
ON_UPDATE_COMMAND_UI(ID_MODE_NODAL, &AeSys::OnUpdateModeNodal)
ON_COMMAND(ID_MODE_PIPE, &AeSys::OnModePipe)
ON_UPDATE_COMMAND_UI(ID_MODE_PIPE, &AeSys::OnUpdateModePipe)
ON_COMMAND(ID_MODE_POWER, &AeSys::OnModePower)
ON_UPDATE_COMMAND_UI(ID_MODE_POWER, &AeSys::OnUpdateModePower)
ON_COMMAND(ID_MODE_REVISE, &AeSys::OnModeRevise)
ON_COMMAND(ID_MODE_TRAP, &AeSys::OnModeTrap)
ON_UPDATE_COMMAND_UI(ID_MODE_TRAP, &AeSys::OnUpdateModeTrap)
ON_COMMAND(ID_TRAPCOMMANDS_ADDGROUPS, &AeSys::OnTrapCommandsAddGroups)
ON_UPDATE_COMMAND_UI(ID_TRAPCOMMANDS_ADDGROUPS, &AeSys::OnUpdateTrapcommandsAddgroups)
ON_COMMAND(ID_TRAPCOMMANDS_HIGHLIGHT, &AeSys::OnTrapCommandsHighlight)
ON_UPDATE_COMMAND_UI(ID_TRAPCOMMANDS_HIGHLIGHT, &AeSys::OnUpdateTrapcommandsHighlight)
ON_COMMAND(ID_VIEW_MODEINFORMATION, &AeSys::OnViewModeInformation)
ON_UPDATE_COMMAND_UI(ID_VIEW_MODEINFORMATION, &AeSys::OnUpdateViewModeinformation)
ON_COMMAND(ID_FILE_PLOTSTYLEMANAGER, &AeSys::OnFilePlotstylemanager)
ON_COMMAND(ID_TOOLS_LOADAPPLICATIONS, &AeSys::OnToolsLoadapplications)
ON_COMMAND(ID_VECTORIZE_ADDVECTORIZERDLL, &AeSys::OnVectorizeAddVectorizerDLL)
ON_UPDATE_COMMAND_UI(ID_VECTORIZE_ADDVECTORIZERDLL, &AeSys::OnUpdateVectorizeAddvectorizerdll)
ON_COMMAND(ID_VECTORIZE_CLEARMENU, OnVectorizeClearmenu)
ON_UPDATE_COMMAND_UI(ID_VECTORIZE_CLEARMENU, OnUpdateVectorizeClearmenu)

END_MESSAGE_MAP()


/// <remarks> Specialization of CCommandLineInfo to add the following switches: bat:, ld:, scr:, exe:, s:, and exit</remarks>

class CFullCommandLineInfo : public CCommandLineInfo {
   public:
	CString m_SaveName;
	bool m_Exit;
	CStringArray m_AppsToLoad;
	CStringArray m_CommandsToExecute;
	CString m_ScriptToExecute;
	CString m_BatToExecute;

	CFullCommandLineInfo() noexcept
		: CCommandLineInfo()
		, m_Exit {false} {
	}

	void ParseParam(const wchar_t* parameter, BOOL flag, BOOL last) override /* CCommandLineInfo */ {
		bool is {false};

		if (flag) {
			if (!_wcsnicmp(parameter, L"bat:", 4)) {
				m_BatToExecute = &parameter[4];
				is = true;
			} else if (!_wcsnicmp(parameter, L"ld:", 3)) {
				m_AppsToLoad.Add(&parameter[3]);
				is = true;
			} else if (!_wcsnicmp(parameter, L"scr:", 4)) {
				m_ScriptToExecute = &parameter[4];
				is = true;
			} else if (!_wcsnicmp(parameter, L"ex:", 3)) {
				m_CommandsToExecute.Add(&parameter[3]);
				is = true;
			} else if (!_wcsnicmp(parameter, L"s:", 2)) {
				m_SaveName = &parameter[2];
				is = true;
			} else if (!_wcsicmp(parameter, L"exit")) {
				m_Exit = true;
				is = true;
			}
		}
		if (!is || last) {
			CCommandLineInfo::ParseParam(parameter, flag, last);
		}
	}
	void ParseParam(const char* parameter, BOOL flag, BOOL last) override {} // CCommandLineInfo (to suppress C4266 warning)
};

BOOL AeSys::ProcessShellCommand(CCommandLineInfo& commandLineInfo) {
	CFullCommandLineInfo& FullCommandLineInfo {(CFullCommandLineInfo&) commandLineInfo};

	if (!FullCommandLineInfo.m_BatToExecute.IsEmpty()) {
		_wsystem(FullCommandLineInfo.m_BatToExecute);
	}

	for (auto Index = 0; Index < FullCommandLineInfo.m_AppsToLoad.GetCount(); ++Index) {
		odrxDynamicLinker()->loadModule(OdString(FullCommandLineInfo.m_AppsToLoad.GetAt(Index)), false);
	}
	AeSysDoc* TemporaryDocument {nullptr};

	if (commandLineInfo.m_nShellCommand == CCommandLineInfo::FileOpen) {
		TemporaryDocument = dynamic_cast<AeSysDoc*>(OpenDocumentFile(commandLineInfo.m_strFileName));

		if (!TemporaryDocument) { return FALSE; }

		if (!FullCommandLineInfo.m_ScriptToExecute.IsEmpty()) {
			CStdioFile scrFile(FullCommandLineInfo.m_ScriptToExecute, CFile::modeRead);
			CString strCmd;

			while (scrFile.ReadString(strCmd)) {
				if (!strCmd.IsEmpty() && strCmd[0] != _T('#')) {
					TemporaryDocument->ExecuteCommand((const wchar_t*) strCmd);
				}
			}
		}
		for (int idx = 0; idx < FullCommandLineInfo.m_CommandsToExecute.GetCount(); ++idx) {
			TemporaryDocument->ExecuteCommand((const wchar_t*) FullCommandLineInfo.m_CommandsToExecute.GetAt(idx));
		}
	} else {
		CWinAppEx::ProcessShellCommand(commandLineInfo);
	}
	if (!FullCommandLineInfo.m_SaveName.IsEmpty()) {
		if (!TemporaryDocument->OnSaveDocument(FullCommandLineInfo.m_SaveName)) { return FALSE; }
	}
	if (FullCommandLineInfo.m_Exit) { PostQuitMessage(0); }
	return TRUE;
}

AeSys::AeSys() noexcept
	: m_ProgressLimit(100)
	, m_ProgressPosition(0)
	, m_ProgressPercent(0)
	, m_DiscardBackFaces(true)
	, m_EnableDoubleBuffer(true)
	, m_BlocksCache(false)
	, m_GsDevMultithread(false)
	, m_nMtRegenThreads(4)
	, m_EnablePrintPreviewViaBitmap(true)
	, m_UseGsModel(true)
	, m_EnableHLR(false)
	, m_ContextColors(true)
	, m_TTFPolyDraw(false)
	, m_TTFTextOut(false)
	, m_TTFCache(false)
	, m_DynamicSubEntHlt(false)
	, m_GDIGradientsAsBitmap(false)
	, m_GDIGradientsAsPolys(false)
	, m_nGDIGradientsAsPolysThreshold(10)
	, m_DisableAutoRegen(false)
	, m_background(ViewBackgroundColor)
	, m_thisThreadID {0}
	, m_numCustomCommands(0)
	, m_numGSMenuItems(0)
	, m_bPartial(false)
	, m_bRecover(false)
	, m_bLoading(false)
	// ODA_MT_DBIO_BEGIN
	, m_bUseMTLoading(false)
	// ODA_MT_DBIO_END
	, m_bRemoteGeomViewer(false)
	, m_pagingType(0)
	, m_bUseTempFiles(false)
	, m_bSupportFileSelectionViaDialog(true)
	, m_ApplicationLook(ID_VIEW_APPLOOK_OFF_2007_BLACK)
	, m_displayFields(0)
	, m_SaveRoundTrip(true)
	, m_SavePreview(false)
	, m_SaveWithPassword(false)
	, m_pAuditDlg(nullptr) {
	EnableHtmlHelp();

	CClientDC ClientDeviceContext(AfxGetMainWnd());
	m_ArchitecturalUnitsFractionPrecision = 16;
	m_HighColorMode = ClientDeviceContext.GetDeviceCaps(BITSPIXEL) > 8; // Detect color depth. 256 color toolbars can be used in the high or true color modes only
	m_ClipboardDataEoGroups = true;
	m_ClipboardDataImage = false;
	m_ClipboardDataText = true;
	m_ClipboardFormatIdentifierForEoGroups = 0;
	m_ModeInformationOverView = false;
	m_TrapModeAddGroups = true;
	m_NodalModeAddGroups = true;
	m_CurrentMode = 0;
	m_DeviceHeightInMillimeters = 0.0;
	m_DeviceHeightInPixels = 0.0;
	m_DeviceWidthInMillimeters = 0.0;
	m_DeviceWidthInPixels = 0.0;
	m_DimensionAngle = 45.;
	m_DimensionLength = 0.125;
	m_EngagedAngle = 0.0;
	m_EngagedLength = 0.0;

	m_AeSysMenuHandle = nullptr;
	m_ModeResourceIdentifier = 0;
	m_PrimaryMode = 0;

	m_SimplexStrokeFont = nullptr;
	m_TrapHighlightColor = 15;
	m_TrapHighlighted = true;
	m_Units = kInches;
}
#define EO_REGISTRY_BUFFER_SIZE 1040
#define EO_REGISTRY_MAX_PROFILE_NAME 128
#define EO_REGISTRY_MAX_PATH 1024

CString GetRegistryAcadLocation();
CString GetRegistryAcadProfilesKey();
bool GetRegistryString(HKEY key, const wchar_t* subkey, const wchar_t* name, wchar_t* value, int size) noexcept;

// get the value for the ACAD entry in the registry
static CString FindConfigPath(const CString& configType) {
	auto SubKey {GetRegistryAcadProfilesKey()};

	if (!SubKey.IsEmpty()) {
		SubKey += L"\\General";

		wchar_t SearchValue[EO_REGISTRY_MAX_PATH] {L"\0"};

		if (::GetRegistryString(HKEY_CURRENT_USER, SubKey, configType, SearchValue, EO_REGISTRY_MAX_PATH)) {
			wchar_t ExpandedPath[EO_REGISTRY_MAX_PATH] {L"\0"};
			ExpandEnvironmentStringsW(SearchValue, ExpandedPath, EO_REGISTRY_MAX_PATH);
			return CString(ExpandedPath);
		}
	}
	return CString(L"\0");
}

static CString FindConfigFile(const CString& configType, CString file, OdDbSystemServices* systemServices) {
	auto ConfigurationPath {FindConfigPath(configType)};

	if (!ConfigurationPath.IsEmpty()) {
		file = ConfigurationPath + L"\\" + file;

		if (systemServices->accessFile(file.GetString(), Oda::kFileRead)) { return file; }
	}
	return CString(L"\0");
}

AeSys theApp;

const ODCOLORREF* AeSys::curPalette() const {
	return odcmAcadPalette(m_background);
}

OdGsDevicePtr AeSys::gsBitmapDevice(OdRxObject* view, OdDbBaseDatabase* database, unsigned long flags) {
	try {
		OdGsModulePtr Module;

		if (GETBIT(flags, kFor2dExportRender)) { // Don't export HiddenLine viewports as bitmap in Pdf/Dwf/Svg exports.

			if (GETBIT(flags, kFor2dExportRenderHLR)) { return OdGsDevicePtr(); }

			// Try to export shaded viewports using OpenGL device.
			Module = ::odrxDynamicLinker()->loadModule(OdWinOpenGLModuleName);
		}
		// Use currently selected device for thumbnails and etc.
		if (Module.isNull()) {
			Module = ::odrxDynamicLinker()->loadModule(m_sVectorizerPath);
		}
		if (Module.isNull()) { return OdGsDevicePtr(); }

		return Module->createBitmapDevice();
	} catch (const OdError&) {
	}
	return OdGsDevicePtr();
}

bool AeSys::getPassword(const OdString& drawingName, bool /*isXref*/, OdPassword& password) {
	EoDlgPassword PasswordDialog;
	PasswordDialog.m_sFileName = static_cast<const wchar_t*>(drawingName);

	if (PasswordDialog.DoModal() == IDOK) {
		password = PasswordDialog.m_password;

		if (drawingName.right(4) == L".dwg") { password.makeUpper(); }

		return true;
	}
	return false;
}

OdDbPageControllerPtr AeSys::newPageController() {
	switch (m_pagingType & 0x0f) {
		case 1: // OdDb::kUnload - Simple unloading of objects for partially loaded database.
			return OdRxObjectImpl<ExUnloadController>::createObject();
		case 2: // OdDb::kPage
		case 3: // OdDb::kUnload - Unloading of objects for partially loaded database and paging of objects thru ExPageController.
			return OdRxObjectImpl<ExPageController>::createObject();
	}
	// Paging is not used.
	return (OdDbPageController*) nullptr;
}

int AeSys::setPagingType(int pagingType) noexcept {
	const int oldType = m_pagingType;
	m_pagingType = pagingType;
	return oldType;
}

bool AeSys::setUndoType(bool useTempFiles) noexcept {
	const bool oldType = m_bUseTempFiles;
	m_bUseTempFiles = useTempFiles;
	return oldType;
}

OdString AeSys::fileDialog(int flags, const OdString& prompt, const OdString& defExt, const OdString& fileName, const OdString& filter) {
	if (!supportFileSelectionViaDialog()) { return OdString(L"*unsupported*"); }

	CFileDialog FileDialog(flags == OdEd::kGfpForOpen, defExt, fileName, OFN_HIDEREADONLY | OFN_EXPLORER | OFN_PATHMUSTEXIST, filter, ::AfxGetMainWnd());

	FileDialog.m_ofn.lpstrTitle = prompt;

	if (FileDialog.DoModal() == IDOK) { return OdString(FileDialog.GetPathName()); }

	throw OdEdCancel();
}

OdRxClass* AeSys::databaseClass() const {
	return OdDbDatabaseDoc::desc();
}

OdString AeSys::findFile(const OdString& fileToFind, OdDbBaseDatabase* database, OdDbBaseHostAppServices::FindFileHint hint) {
	CString FilePathAndName;
	FilePathAndName.SetString(ExHostAppServices::findFile(fileToFind, database, hint));

	if (FilePathAndName.IsEmpty()) {
		auto SystemServices {odSystemServices()};

		CString FileToFind;
		FileToFind.SetString(fileToFind);

		auto Extension {FileToFind.Right(4)};
		Extension.MakeLower();

		if (Extension == L".pc3") {
			return FindConfigFile(L"PrinterConfigDir", FileToFind, SystemServices).GetString();
		}
		if (Extension == L".stb" || Extension == L".ctb") {
			return FindConfigFile(L"PrinterStyleSheetDir", FileToFind, SystemServices).GetString();
		}
		if (Extension == L".pmp") {
			return FindConfigFile(L"PrinterDescDir", FileToFind, SystemServices).GetString();
		}
		switch (hint) {
			case kFontFile:
			case kCompiledShapeFile:
			case kTrueTypeFontFile:
			case kPatternFile:
			case kFontMapFile:
			case kTextureMapFile:
				break;
			case kEmbeddedImageFile:
				if (FileToFind.Left(5).CompareNoCase(L"http:") == 0 || FileToFind.Left(6).CompareNoCase(L"https:") == 0) {
					// <tas="code section removed"</tas>
				}
				// fall through
			case kXRefDrawing:
			case kTXApplication:
			case kUnderlayFile:
			case kDefault:
			case kPhotometricWebFile:
			default:
				return FilePathAndName.GetString();
		}
		if (hint != kTextureMapFile && Extension != L".shx" && Extension != L".pat" && Extension != L".ttf" && Extension != L".ttc" && Extension != L".otf") {
			FileToFind += L".shx";
		} else if (hint == kTextureMapFile) {
			FileToFind.Replace(L"/", L"\\");
			FileToFind.Delete(0, FileToFind.ReverseFind(L'\\') + 1);
		}
		FilePathAndName = (hint != kTextureMapFile) ? FindConfigPath(L"ACAD") : FindConfigPath(L"AVEMAPS");

		CString Path;
		while (!FilePathAndName.IsEmpty()) {
			const int PathDelimiter {FilePathAndName.Find(L";")};

			if (PathDelimiter == -1) {
				Path = FilePathAndName;
				FilePathAndName.Empty();
			} else {
				Path = FilePathAndName.Left(PathDelimiter);
				Path += L"\\" + FileToFind;

				if (SystemServices->accessFile(Path.GetString(), Oda::kFileRead)) { return Path.GetString(); }

				FilePathAndName = FilePathAndName.Right(FilePathAndName.GetLength() - PathDelimiter - 1);
			}
		}
		if (hint == kTextureMapFile) { return FilePathAndName.GetString(); }

		if (FilePathAndName.IsEmpty()) {
			auto AcadLocation {GetRegistryAcadLocation()};

			if (!AcadLocation.IsEmpty()) {
				FilePathAndName = AcadLocation + L"\\Fonts\\" + FileToFind;

				if (!SystemServices->accessFile(FilePathAndName.GetString(), Oda::kFileRead)) {
					FilePathAndName = AcadLocation + L"\\Support\\" + FileToFind;

					if (!SystemServices->accessFile(FilePathAndName.GetString(), Oda::kFileRead)) { FilePathAndName.Empty(); }
				}
			}
		}
	}
	return FilePathAndName.GetString();
}

CString AeSys::getApplicationPath() {
	wchar_t FileName[MAX_PATH];

	if (::GetModuleFileNameW(::GetModuleHandleW(nullptr), FileName, MAX_PATH)) {
		CString FilePath(FileName);
		const int Delimiter {FilePath.ReverseFind('\\')};
		return (FilePath.Left(Delimiter));
	}
	return L"";
}

void AeSys::auditPrintReport(OdAuditInfo* auditInfo, const OdString& line, int printDest) const {
	if (m_pAuditDlg) { m_pAuditDlg->printReport(dynamic_cast<OdDbAuditInfo*>(auditInfo)); }
}

OdDbUndoControllerPtr AeSys::newUndoController() {
	if (undoType()) {
		ExFileUndoControllerPtr FileUndoController = OdRxObjectImpl<ExFileUndoController>::createObject();
		FileUndoController->setStorage(newUndoStream());
		return FileUndoController;
	}
	return OdRxObjectImpl<ExUndoController>::createObject();
}

OdStreamBufPtr AeSys::newUndoStream() {
	// OdMemFileStreamImpl = mix of memory and file streams
	return OdRxObjectImpl<OdMemFileStreamImpl<OdStreamBuf> >::createObject();
}

void AeSys::SetRecentCommand(const OdString& command) {
	if (!command.isEmpty() && command != m_RecentCommand) {
		m_RecentCommand = command;
		WriteProfileStringW(L"options", L"Recent Command", m_RecentCommand);
	}
}

CMenu* AeSys::CommandMenu(CMenu** toolsSubMenu) {
	MENUITEMINFO MenuItemInfo;
	MenuItemInfo.cbSize = sizeof(MENUITEMINFO);
	MenuItemInfo.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_SUBMENU | MIIM_ID;

	CString MenuName;
	CMenu* ToolsSubMenu {nullptr};
	CMenu* TopMenu {CMenu::FromHandle(theApp.GetAeSysMenu())};

	for (int Item = TopMenu->GetMenuItemCount() - 1; Item >= 0; Item--) {
		MenuItemInfo.dwTypeData = nullptr;
		TopMenu->GetMenuItemInfoW(static_cast<unsigned>(Item), &MenuItemInfo, TRUE);

		const auto SizeOfMenuName {++MenuItemInfo.cch};
		MenuItemInfo.dwTypeData = MenuName.GetBuffer(static_cast<int>(SizeOfMenuName));
		TopMenu->GetMenuItemInfoW(static_cast<unsigned>(Item), &MenuItemInfo, TRUE);
		MenuName.ReleaseBuffer();

		if (MenuItemInfo.fType == MFT_STRING && MenuName.Compare(L"&Tools") == 0) {
			ToolsSubMenu = CMenu::FromHandle(MenuItemInfo.hSubMenu);
			break;
		}
	}
	ASSERT(ToolsSubMenu != nullptr);

	if (toolsSubMenu) { *toolsSubMenu = ToolsSubMenu; }

	CMenu* RegisteredCommandsSubMenu {nullptr};

	for (int ToolsMenuItem = 0; ToolsMenuItem < ToolsSubMenu->GetMenuItemCount(); ToolsMenuItem++) {
		MenuItemInfo.dwTypeData = nullptr;
		ToolsSubMenu->GetMenuItemInfoW(static_cast<unsigned>(ToolsMenuItem), &MenuItemInfo, TRUE);

		const auto SizeOfMenuName {++MenuItemInfo.cch};
		MenuItemInfo.dwTypeData = MenuName.GetBuffer(static_cast<int>(SizeOfMenuName));
		ToolsSubMenu->GetMenuItemInfoW(static_cast<unsigned>(ToolsMenuItem), &MenuItemInfo, TRUE);
		MenuName.ReleaseBuffer();

		if (MenuItemInfo.fType == MFT_STRING && MenuName.CompareNoCase(L"Registered &Commands") == 0) {
			RegisteredCommandsSubMenu = CMenu::FromHandle(MenuItemInfo.hSubMenu);
			break;
		}
	}
	ENSURE(RegisteredCommandsSubMenu != nullptr);
	return (RegisteredCommandsSubMenu);
}

void AeSys::RefreshCommandMenu() {
	CMenu* ToolsSubMenu {nullptr};
	CMenu* RegisteredCommandsSubMenu {CommandMenu(&ToolsSubMenu)};

	for (int Item = RegisteredCommandsSubMenu->GetMenuItemCount() - 1; Item >= 0; Item--) {
		CMenu* SubMenu {RegisteredCommandsSubMenu->GetSubMenu(Item)};

		if (SubMenu) { SubMenu->DestroyMenu(); }

		RegisteredCommandsSubMenu->DeleteMenu(static_cast<unsigned>(Item), MF_BYPOSITION);
	}
	ENSURE(RegisteredCommandsSubMenu->GetMenuItemCount() == 0);

	MENUITEMINFO MenuItemInfo;
	MenuItemInfo.cbSize = sizeof(MENUITEMINFO);
	MenuItemInfo.fMask = MIIM_DATA;

	auto CommandStack {::odedRegCmds()};
	bool HasNoCommand {CommandStack->newIterator()->done()};

	const unsigned ToolsMenuItem {8}; // <tas="Until calculated ToolsMenu position finished. Menu resource which change the location of Registered Commands location break."</tas>
	ToolsSubMenu->EnableMenuItem(ToolsMenuItem, static_cast<unsigned>(MF_BYPOSITION | (HasNoCommand ? MF_GRAYED : MF_ENABLED)));

	unsigned CommandId {_APS_NEXT_COMMAND_VALUE + 100};

	if (!HasNoCommand) {
		auto CommandStackGroupIterator = CommandStack->newGroupIterator();

		while (!CommandStackGroupIterator->done()) {
			OdRxDictionaryPtr Group {CommandStackGroupIterator->object()};
			CMenu GroupMenu;
			GroupMenu.CreateMenu();
			OdRxIteratorPtr GroupCommandIterator {Group->newIterator(OdRx::kDictSorted)};
			OdString GroupName;

			while (!GroupCommandIterator->done()) {
				OdEdCommandPtr Command {GroupCommandIterator->object().get()};

				if (GroupName.isEmpty()) { GroupName = Command->groupName(); }

				OdString CommandName(Command->globalName());
				GroupMenu.AppendMenuW(MF_STRING, CommandId, CommandName);

				MenuItemInfo.dwItemData = (LPARAM) Command.get();
				::SetMenuItemInfoW(GroupMenu.m_hMenu, CommandId, FALSE, &MenuItemInfo);

				GroupCommandIterator->next();
				CommandId++;
			}
			ENSURE(RegisteredCommandsSubMenu->AppendMenuW(MF_STRING | MF_POPUP, (LPARAM) GroupMenu.Detach(), GroupName) != 0);

			CommandStackGroupIterator->next();
			GroupName.empty();
		}
	}
	m_numCustomCommands = CommandId - _APS_NEXT_COMMAND_VALUE - 100;
}

void AeSys::AddReactor(const OdApplicationReactor* reactor) {
	if (m_ApplicationReactors.end() == std::find(m_ApplicationReactors.begin(), m_ApplicationReactors.end(), OdApplicationReactorPtr(reactor))) {
		m_ApplicationReactors.push_back(reactor);
	}
}

void AeSys::RemoveReactor(const OdApplicationReactor* reactor) {
	m_ApplicationReactors.erase(std::remove(m_ApplicationReactors.begin(), m_ApplicationReactors.end(), OdApplicationReactorPtr(reactor)), m_ApplicationReactors.end());
}

OdDbDatabasePtr AeSys::openFile(const wchar_t* pathName) {
	auto MainFrame {dynamic_cast<CMainFrame*>(GetMainWnd())};
	OdDbDatabasePtr Database;

	short nMode = getMtMode();
	SETBIT(nMode, 1, m_bUseMTLoading);
	setMtMode(nMode);

	// open an existing document
	MainFrame->StartTimer();
	try {
		CWaitCursor wait;
		OdString PathName(pathName);
		if (PathName.right(4).iCompare(L".dgn") == 0) {
			// <tas="Likely will never support dgn"</tas>
			MainFrame->StopTimer(L"Loading");
		} else if (PathName.right(4).iCompare(L".dwf") == 0 || PathName.right(5).iCompare(L".dwfx") == 0) {
			// <tas="Will add support for dwf"</tas>
			MainFrame->StopTimer(L"Loading");
		} else if (m_bRecover) {
			ODA_ASSERT(!m_pAuditDlg);
			m_pAuditDlg = new EoDlgAudit();

			ODA_ASSERT(m_pAuditDlg);
			ODA_VERIFY(m_pAuditDlg->Create(IDD_AUDITINFO));

			EoAppAuditInfo ApplicationAuditInformation;
			ApplicationAuditInformation.setHostAppServices(&theApp);

			Database = recoverFile(createFile(PathName), &ApplicationAuditInformation);
			MainFrame->StopTimer(L"Recovering");
			m_pAuditDlg->SetWindowTextW(L"Recover info " + PathName);
			m_pAuditDlg->ShowWindow(SW_SHOW);

			m_pAuditDlg = nullptr;
		} else {
			m_bLoading = true;
			Database = readFile(PathName, false, m_bPartial);
			MainFrame->StopTimer(L"Loading");
			m_bLoading = false;
		}
	} catch (const OdError& Error) {
		Database = nullptr;
		MainFrame->SetStatusPaneTextAt(0, L"");
		reportError(L"Loading Error...", Error);
	} catch (const UserBreak&) {
		Database = nullptr;
		MainFrame->SetStatusPaneTextAt(0, L"");
		SetStatusPaneTextAt(1, L"Operation was canceled by user.");
	} catch (std::bad_alloc&) {
		Database = nullptr;
		MainFrame->SetStatusPaneTextAt(0, L"");
		SetStatusPaneTextAt(1, L"Memory Allocation Error...");
	}
	if (m_pAuditDlg) { // Destroy audit dialog if recover failed
		delete m_pAuditDlg;
		m_pAuditDlg = nullptr;
	}
	return Database;
}

void AeSys::AddModeInformationToMessageList() {
	auto ResourceString {LoadStringResource(m_CurrentMode)};
	int NextToken {0};
	ResourceString = ResourceString.Tokenize(L"\n", NextToken);
	AddStringToMessageList(ResourceString);
}

void AeSys::AddStringToMessageList(const wchar_t* message) {
	auto MainFrame {dynamic_cast<CMainFrame*>(AfxGetMainWnd())};

	MainFrame->GetOutputPane().AddStringToMessageList(message);

	if (!MainFrame->GetOutputPane().IsWindowVisible()) {
		MainFrame->SetStatusPaneTextAt(nStatusInfo, message);
	}
}

void AeSys::AddStringToMessageList(const wchar_t* message, const wchar_t* string) {
	CString FormatString;
	FormatString.Format(message, string);
	AddStringToMessageList(FormatString);
}

void AeSys::AddStringToMessageList(unsigned stringResourceIdentifier) {
	auto ResourceString {LoadStringResource(stringResourceIdentifier)};
	AddStringToMessageList(ResourceString);
}

void AeSys::AddStringToMessageList(unsigned stringResourceIdentifier, const wchar_t* string) {
	auto FormatSpecification {LoadStringResource(stringResourceIdentifier)};
	AddStringToMessageList(FormatSpecification, string);
}

void AeSys::AddStringToReportList(const wchar_t* message) {
	auto MainFrame {dynamic_cast<CMainFrame*>(AfxGetMainWnd())};

	MainFrame->GetOutputPane().AddStringToReportsList(message);

	if (!MainFrame->GetOutputPane().IsWindowVisible()) {
		MainFrame->SetStatusPaneTextAt(nStatusInfo, message);
	}
}

int AeSys::ArchitecturalUnitsFractionPrecision() const noexcept {
	return (m_ArchitecturalUnitsFractionPrecision);
}
// Modifies the base accelerator table by defining the mode specific keys.
void AeSys::BuildModeSpecificAcceleratorTable() {
	auto MainFrame {dynamic_cast<CMainFrame*>(AfxGetMainWnd())};

	auto AcceleratorTableHandle {MainFrame->m_hAccelTable};
	::DestroyAcceleratorTable(AcceleratorTableHandle);

	auto ModeAcceleratorTableHandle {::LoadAcceleratorsW(m_hInstance, MAKEINTRESOURCEW(m_ModeResourceIdentifier))};
	auto ModeAcceleratorTableEntries {::CopyAcceleratorTableW(ModeAcceleratorTableHandle, nullptr, 0)};

	AcceleratorTableHandle = ::LoadAcceleratorsW(m_hInstance, MAKEINTRESOURCEW(IDR_MAINFRAME));
	const int AcceleratorTableEntries {::CopyAcceleratorTableW(AcceleratorTableHandle, nullptr, 0)};

	LPACCEL ModifiedAcceleratorTable {new ACCEL[static_cast<unsigned>(AcceleratorTableEntries + ModeAcceleratorTableEntries)]};

	::CopyAcceleratorTableW(ModeAcceleratorTableHandle, ModifiedAcceleratorTable, ModeAcceleratorTableEntries);
	::CopyAcceleratorTableW(AcceleratorTableHandle, &ModifiedAcceleratorTable[ModeAcceleratorTableEntries], AcceleratorTableEntries);

	MainFrame->m_hAccelTable = ::CreateAcceleratorTable(ModifiedAcceleratorTable, AcceleratorTableEntries + ModeAcceleratorTableEntries);

	delete[] ModifiedAcceleratorTable;
}

unsigned AeSys::ClipboardFormatIdentifierForEoGroups() noexcept {
	return (m_ClipboardFormatIdentifierForEoGroups);
}

unsigned AeSys::CurrentMode() const noexcept {
	return m_CurrentMode;
}

double AeSys::DeviceHeightInMillimeters() const noexcept {
	return m_DeviceHeightInMillimeters;
}

double AeSys::DeviceHeightInPixels() const noexcept {
	return m_DeviceHeightInPixels;
}

double AeSys::DeviceWidthInMillimeters() const noexcept {
	return m_DeviceWidthInMillimeters;
}

double AeSys::DeviceWidthInPixels() const noexcept {
	return m_DeviceWidthInPixels;
}

double AeSys::DimensionAngle() const noexcept {
	return (m_DimensionAngle);
}

double AeSys::DimensionLength() const noexcept {
	return (m_DimensionLength);
}

void AeSys::EditColorPalette() {
	CHOOSECOLOR cc;
	::ZeroMemory(&cc, sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);

	cc.rgbResult = ColorPalette[pstate.ColorIndex()];
	cc.lpCustColors = ColorPalette;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_SOLIDCOLOR;
	::ChooseColorW(&cc);

	cc.rgbResult = GreyPalette[pstate.ColorIndex()];
	cc.lpCustColors = GreyPalette;
	::ChooseColorW(&cc);

	::MessageBoxW(nullptr, L"The background color is no longer associated with the pen Color Palette.", L"Deprecation Notice", MB_OK | MB_ICONINFORMATION);

	AeSysDoc::GetDoc()->UpdateAllViews(nullptr);
}

double AeSys::EngagedAngle() const noexcept {
	return (m_EngagedAngle);
}

double AeSys::EngagedLength() const noexcept {
	return (m_EngagedLength);
}

CString AeSys::BrowseWithPreview(HWND parentWindow, const wchar_t* filter, bool multiple) {
	CString FileName;
	const unsigned long Flags(OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST);
	CString LibraryFileName(L"FileDlgExt" TD_DLL_VERSION_SUFFIX_STR L".dll");
	HINSTANCE hinstLib {LoadLibraryW(LibraryFileName)};

	if (hinstLib != nullptr) {
		auto fpDlgProc {(ODA_OPEN_DLGPROC) GetProcAddress(hinstLib, "CreateOpenWithPreviewDlg")};

		if (fpDlgProc != nullptr) {
			EoPreviewDib statDib;
			OpenWithPreviewDlg* OpenWithPreviewDialog;
			(fpDlgProc)(&statDib, parentWindow, NULL, filter, Flags, &OpenWithPreviewDialog);

			if (IDOK == OpenWithPreviewDialog->ShowModal()) {
				long BufferLength {MAX_PATH};
				OpenWithPreviewDialog->GetFullFileName(FileName.GetBuffer(BufferLength), BufferLength);
				FileName.ReleaseBuffer();
			}
			OpenWithPreviewDialog->ReleaseDlg();
		}
		FreeLibrary(hinstLib);
	} else {
		CString Filter(filter);
		Filter.Replace('|', '\0');

		OPENFILENAME OpenFileName;
		::ZeroMemory(&OpenFileName, sizeof(OPENFILENAME));
		OpenFileName.lStructSize = sizeof(OPENFILENAME);
		OpenFileName.hwndOwner = parentWindow;
		OpenFileName.lpstrFilter = Filter;
		OpenFileName.nFilterIndex = 1;
		OpenFileName.lpstrFile = FileName.GetBuffer(MAX_PATH);
		OpenFileName.nMaxFile = MAX_PATH;
		OpenFileName.lpstrInitialDir = nullptr;
		OpenFileName.Flags = Flags;

		GetOpenFileNameW(&OpenFileName);
		FileName.ReleaseBuffer();
	}
	return FileName;
}

int AeSys::ExitInstance() {
	SetRegistryBase(L"ODA View");
	theApp.WriteInt(L"Discard Back Faces", m_DiscardBackFaces);
	theApp.WriteInt(L"Enable Double Buffer", m_EnableDoubleBuffer);
	theApp.WriteInt(L"Enable Blocks Cache", m_BlocksCache);
	theApp.WriteInt(L"Gs Device Multithread", m_GsDevMultithread);
	theApp.WriteInt(L"Mt Regen Threads Count", static_cast<int>(m_nMtRegenThreads));
	theApp.WriteInt(L"Print/Preview via bitmap device", m_EnablePrintPreviewViaBitmap);
	theApp.WriteInt(L"UseGsModel", m_UseGsModel);
	theApp.WriteInt(L"Enable Software HLR", m_EnableHLR);
	theApp.WriteInt(L"Contextual Colors", m_ContextColors);
	theApp.WriteInt(L"TTF PolyDraw", m_TTFPolyDraw);
	theApp.WriteInt(L"TTF TextOut", m_TTFTextOut);

	theApp.WriteInt(L"TTF Cache", m_TTFCache);
	theApp.WriteInt(L"Dynamic Subentities Highlight", m_DynamicSubEntHlt);
	theApp.WriteInt(L"GDI Gradients as Bitmaps", m_GDIGradientsAsBitmap);
	theApp.WriteInt(L"GDI Gradients as Polys", m_GDIGradientsAsPolys);
	theApp.WriteInt(L"GDI Gradients as Polys Threshold", m_nGDIGradientsAsPolysThreshold);

	theApp.WriteInt(L"Disable Auto-Regen", m_DisableAutoRegen);

	theApp.WriteInt(L"Save round trip information", m_SaveRoundTrip);
	theApp.WriteInt(L"Save Preview", m_SavePreview);
	theApp.WriteInt(L"Background colour", static_cast<int>(m_background));
	theApp.WriteInt(L"Save DWG with password", m_SaveWithPassword);
	theApp.WriteString(L"recent GS", m_sVectorizerPath);
	theApp.WriteString(L"Recent Command", m_RecentCommand);
	theApp.WriteInt(L"Fill TTF text", (int) getTEXTFILL());

	SetRegistryBase(L"Options");
	m_Options.Save();
	SetRegistryBase(L"MFC Auto");

	ReleaseSimplexStrokeFont();
	UninitializeTeigha();

	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

CString AeSys::FormatAngle(double angle, int width, int precision) const {
	CString FormatSpecification;
	FormatSpecification.Format(L"%%%i.%if°", width, precision);
	CString AngleAsString;
	AngleAsString.Format(FormatSpecification, EoToDegree(angle));
	return (AngleAsString);
}

CString AeSys::FormatLength(double length, Units units, int width, int precision) const {
	wchar_t LengthAsString[32];
	FormatLength_s(LengthAsString, 32, units, length, width, precision);
	return CString(LengthAsString).TrimLeft();
}

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

void AeSys::FormatLength_s(wchar_t* lengthAsString, const unsigned bufSize, Units units, const double length, const int width, const int precision) const {
	wchar_t szBuf[16] {L"\0"};

	double ScaledLength {length * AeSysView::GetActiveView()->WorldScale()};

	CString FormatSpecification;
	FormatSpecification.Format(L"%%%i.%if", width, precision);

	switch (units) {
		case kArchitectural:
		case kArchitecturalS: {
			wcscpy_s(lengthAsString, bufSize, (length >= 0.0) ? L" " : L"-");
			ScaledLength = fabs(ScaledLength);

			auto Feet {static_cast<int>(ScaledLength / 12.)};
			auto Inches {abs(static_cast<int>(fmod(ScaledLength, 12.)))};

			const auto FractionPrecision {ArchitecturalUnitsFractionPrecision()};
			auto Numerator {int(fabs(fmod(ScaledLength, 1.0)) * static_cast<double>(FractionPrecision) + 0.5)}; // Numerator of fractional component of inches

			if (Numerator == FractionPrecision) {
				if (Inches == 11) {
					Feet++;
					Inches = 0;
				} else {
					Inches++;
				}
				Numerator = 0;
			}
			_itow_s(Feet, szBuf, 16, 10);
			wcscat_s(lengthAsString, bufSize, szBuf);
			wcscat_s(lengthAsString, bufSize, L"'");

			_itow_s(Inches, szBuf, 16, 10);
			wcscat_s(lengthAsString, bufSize, szBuf);

			if (Numerator > 0) {
				wcscat_s(lengthAsString, bufSize, (units == kArchitecturalS) ? L"\\S" : L"·" /* middle dot [U+00B7] */);
				const int iGrtComDivisor {GreatestCommonDivisor(Numerator, FractionPrecision)};
				Numerator /= iGrtComDivisor;
				const int Denominator {FractionPrecision / iGrtComDivisor}; // Add fractional component of inches
				_itow_s(Numerator, szBuf, 16, 10);
				wcscat_s(lengthAsString, bufSize, szBuf);
				wcscat_s(lengthAsString, bufSize, L"/");
				_itow_s(Denominator, szBuf, 16, 10);
				wcscat_s(lengthAsString, bufSize, szBuf);

				if (units == kArchitecturalS) { wcscat_s(lengthAsString, bufSize, L";"); }
			}
			wcscat_s(lengthAsString, bufSize, L"\"");
			break;
		}
		case kEngineering: {
			wcscpy_s(lengthAsString, bufSize, (length >= 0.0) ? L" " : L"-");
			ScaledLength = fabs(ScaledLength);

			const int Precision {(ScaledLength >= 1.0) ? precision - int(log10(ScaledLength)) - 1 : precision};

			if (Precision >= 0) {
				_itow_s(int(ScaledLength / 12.), szBuf, 16, 10);
				wcscat_s(lengthAsString, bufSize, szBuf);
				ScaledLength = fmod(ScaledLength, 12.);
				wcscat_s(lengthAsString, bufSize, L"'");

				_itow_s(int(ScaledLength), szBuf, 16, 10);
				wcscat_s(lengthAsString, bufSize, szBuf);

				if (Precision > 0) {
					CString FormatSpecification;
					FormatSpecification.Format(L"%%%i.%if", width, Precision);

					CString FractionalInches;
					FractionalInches.Format(FormatSpecification, ScaledLength);
					const int DecimalPointPosition = FractionalInches.Find('.');
					FractionalInches = FractionalInches.Mid(DecimalPointPosition) + L"\"";

					wcscat_s(lengthAsString, bufSize, FractionalInches);
				}
			}
			break;
		}
		case kFeet:
			FormatSpecification.Append(L"'");
			swprintf_s(lengthAsString, bufSize, FormatSpecification, ScaledLength / 12.0);
			break;
		case kInches:
			FormatSpecification.Append(L"\"");
			swprintf_s(lengthAsString, bufSize, FormatSpecification, ScaledLength);
			break;
		case kMeters:
			FormatSpecification.Append(L"m");
			swprintf_s(lengthAsString, bufSize, FormatSpecification, ScaledLength * 0.0254);
			break;
		case kMillimeters:
			FormatSpecification.Append(L"mm");
			swprintf_s(lengthAsString, bufSize, FormatSpecification, ScaledLength * 25.4);
			break;
		case kCentimeters:
			FormatSpecification.Append(L"cm");
			swprintf_s(lengthAsString, bufSize, FormatSpecification, ScaledLength * 2.54);
			break;
		case kDecimeters:
			FormatSpecification.Append(L"dm");
			swprintf_s(lengthAsString, bufSize, FormatSpecification, ScaledLength * 0.254);
			break;
		case kKilometers:
			FormatSpecification.Append(L"km");
			swprintf_s(lengthAsString, bufSize, FormatSpecification, ScaledLength * 0.0000254);
			break;
		default:
			lengthAsString[0] = '\0';
			break;
	}
}

OdGePoint3d AeSys::GetCursorPosition() {
	auto ActiveView {AeSysView::GetActiveView()};
	return (ActiveView) ? ActiveView->GetCursorPosition() : OdGePoint3d::kOrigin;
}

EoDb::FileTypes AeSys::GetFileType(const OdString& file) {
	EoDb::FileTypes Type(EoDb::kUnknown);
	auto Extension {file.right(3)};

	if (!Extension.isEmpty()) {
		if (Extension.iCompare(L"peg") == 0) {
			Type = EoDb::kPeg;
		} else if (Extension.iCompare(L"tra") == 0) {
			Type = EoDb::kTracing;
		} else if (Extension.iCompare(L"jb1") == 0) {
			Type = EoDb::kJob;
		} else if (Extension.iCompare(L"dwg") == 0) {
			Type = EoDb::kDwg;
		} else if (Extension.iCompare(L"dxf") == 0) {
			Type = EoDb::kDxf;
		} else if (Extension.iCompare(L"dxb") == 0) {
			Type = EoDb::kDxb;
		}
	}
	return Type;
}

COLORREF AeSys::GetHotColor(short colorIndex) noexcept {
	return (ColorPalette[colorIndex]);
}

HINSTANCE AeSys::GetInstance() noexcept {
	return (m_hInstance);
}

HMENU AeSys::GetAeSysMenu() noexcept {
	return (m_AeSysMenuHandle);
}

HMENU AeSys::GetAeSysSubMenu(int position) noexcept {
	return (::GetSubMenu(m_AeSysMenuHandle, position));
}

AeSys::Units AeSys::GetUnits() noexcept {
	return (m_Units);
}

/// <summary>Finds the greatest common divisor of arbitrary integers.</summary>
/// <returns>First number if second number is zero, greatest common divisor otherwise.</returns>

int AeSys::GreatestCommonDivisor(const int number1, const int number2) const noexcept {
	int ReturnValue = abs(number1);
	int Divisor = abs(number2);
	while (Divisor != 0) {
		const int Remainder = ReturnValue % Divisor;
		ReturnValue = Divisor;
		Divisor = Remainder;
	}
	return (ReturnValue);
}

bool AeSys::HighColorMode() const noexcept {
	return m_HighColorMode;
}

OdGePoint3d AeSys::HomePointGet(int i) noexcept {
	if (i >= 0 && i < 9) { return (m_HomePoints[i]); }

	return (OdGePoint3d::kOrigin);
}

void AeSys::HomePointSave(int i, const OdGePoint3d& point) noexcept {
	if (i >= 0 && i < 9) { m_HomePoints[i] = point; }
}

void AeSys::InitGbls(CDC* deviceContext) {
	pstate.SetHatchInteriorStyle(EoDbHatch::kHatch);
	pstate.SetHatchInteriorStyleIndex(1);

	EoDbHatch::sm_PatternScaleX = .1;
	EoDbHatch::sm_PatternScaleY = .1;
	EoDbHatch::sm_PatternAngle = 0.0;

	const EoDbCharacterCellDefinition CharacterCellDefinition;
	pstate.SetCharacterCellDefinition(CharacterCellDefinition);

	EoDbFontDefinition FontDefinition;
	pstate.SetFontDefinition(deviceContext, FontDefinition);

	SetUnits(kInches);
	SetArchitecturalUnitsFractionPrecision(8);
	SetDimensionLength(.125);
	SetDimensionAngle(45.);

	m_TrapHighlighted = true;
	m_TrapHighlightColor = 15;

	//Document->InitializeGroupAndPrimitiveEdit();
	pstate.SetPen(nullptr, deviceContext, 1, 1);
	pstate.SetPointDisplayMode(1);
}

bool AeSys::InitializeOda() {
	try {
		::odInitialize(this);

		EoLoadApps::rxInit();

		OdApplicationReactor::rxInit();
		OdApplicationDocument::rxInit();

		::odrxDynamicLinker()->loadModule(OdGripPointsModuleName); // GripPoints module
		::odrxDynamicLinker()->loadModule(OdDbCommandsModuleName); // DbCommands module (ERASE,EXPLODE,PURGE, etc.)
		::odrxDynamicLinker()->loadModule(OdExCommandsModuleName);
		::odrxDynamicLinker()->loadModule(OdPlotSettingsValidatorModuleName); // PlotSettingsValidator module (To include support for plot settings)

		addPaperDrawingCustomization();
		addMaterialTextureLoadingMonitor();

		OdDbDatabaseDoc::rxInit();

#ifdef OD_OLE_SUPPORT
		::rxInit_COleClientItem_handler();
#endif // OD_OLE_SUPPORT

		auto CommandStack {::odedRegCmds()};
		CommandStack->addCommand(&g_Cmd_VIEW);
		CommandStack->addCommand(&g_Cmd_SELECT);

		/* <tas>
		rxInitMaterialsEditorObjects();
		   </tas> */
	} catch (const OdError& Error) {
		theApp.reportError(L"odInitialize error", Error);
		return false;
	} catch (...) {
		::MessageBoxW(nullptr, L"odInitialize error", L"Teigha", MB_ICONERROR | MB_OK);
		return false;
	}
	return true;
}

BOOL AeSys::InitInstance() {
	// InitCommonControlsEx() is required on Windows XP if an application manifest specifies use of ComCtl32.dll version 6 or later to enable visual styles. Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);

	// Load animate control, header, hot key, list-view, progress bar, status bar, tab, tooltip, toolbar, trackbar, tree-view, and up-down control classes.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	if (!AfxOleInit()) { // Failed to initialize OLE support for the application.
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();

	EnableTaskbarInteraction();

	// Standard initialization

	// Application settings to be stored in the registry instead of INI files.
	SetRegistryKey(L"Engineers Office");
	LoadStdProfileSettings(8U); // Load the list of most recently used (MRU) files and last preview state.
	SetRegistryBase(L"Options");
	m_Options.Load();

	SetRegistryBase(L"ODA View");

	m_DiscardBackFaces = GetInt(L"Discard Back Faces", true);
	m_EnableDoubleBuffer = GetInt(L"Enable Double Buffer", true); // <tas="true unless debugging"</tas>
	m_BlocksCache = GetInt(L"Enable Blocks Cache", false);
	m_GsDevMultithread = GetInt(L"Gs Device Multithread", false);
	m_nMtRegenThreads = static_cast<unsigned>(GetInt(L"Mt Regen Threads Count", 4));
	m_EnablePrintPreviewViaBitmap = GetInt(L"Print/Preview via bitmap device", true);
	m_UseGsModel = GetInt(L"UseGsModel", true);
	m_EnableHLR = GetInt(L"Enable Software HLR", false);
	m_ContextColors = GetInt(L"Contextual Colors", true);
	m_TTFPolyDraw = GetInt(L"TTF PolyDraw", false);
	m_TTFTextOut = GetInt(L"TTF TextOut", false);

	m_TTFCache = GetInt(L"TTF Cache", false);
	m_DynamicSubEntHlt = GetInt(L"Dynamic Subentities Highlight", false);
	m_GDIGradientsAsBitmap = GetInt(L"GDI Gradients as Bitmaps", false);
	m_GDIGradientsAsPolys = GetInt(L"GDI Gradients as Polys", false);
	m_nGDIGradientsAsPolysThreshold = gsl::narrow_cast<unsigned char>(GetInt(L"GDI Gradients as Polys Threshold", 10));

	m_DisableAutoRegen = GetInt(L"Disable Auto-Regen", false);

	//	m_displayFields = GetProfileInt(_T("options"), _T("Field display format"), 0);

	m_SaveRoundTrip = GetInt(L"Save round trip information", true);
	m_SavePreview = GetInt(L"Save Preview", false);
	m_background = static_cast<unsigned>(GetInt(L"Background colour", static_cast<int>(ViewBackgroundColor)));
	m_SaveWithPassword = GetInt(L"Save DWG with password", false);
	m_sVectorizerPath = GetString(L"recent GS", OdWinDirectXModuleName);
	m_RecentCommand = GetString(L"Recent Command", L"");
	int nFillTtf = GetInt(L"Fill TTF text", 1);
	setTEXTFILL(nFillTtf != 0);

	SetRegistryBase(L"MFC Auto");

	lex::Init();

	// Initialize application managers for usage. They are automatically constructed if not yet present
	InitContextMenuManager(); // Manages shortcut menus, also known as context menus.
	InitKeyboardManager(); // Manages shortcut key tables for the main frame window and child frame windows.
	InitTooltipManager(); // Maintains runtime information about tooltips.

	CMFCToolTipInfo params;
	params.m_bVislManagerTheme = TRUE;
	GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS(CMFCToolTipCtrl), &params);

	EnableUserTools(ID_TOOLS_ENTRY, ID_USER_TOOL1, ID_USER_TOOL10, RUNTIME_CLASS(CUserTool), IDR_MENU_ARGS, IDR_MENU_DIRS);

	CMFCButton::EnableWindowsTheming();

	CWinAppEx::InitInstance();

	if (!InitializeOda()) { return FALSE; }

	// Register the application's document templates.  Document templates serve as the connection between documents, frame windows and views.
	auto AeSysDocTemplate {new CMultiDocTemplate(IDR_AESYSTYPE, RUNTIME_CLASS(AeSysDoc), RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(AeSysView))};

	if (!AeSysDocTemplate) { return FALSE; }
	AddDocTemplate(AeSysDocTemplate);

	auto TracingDocTemplate {new CMultiDocTemplate(IDR_TRACINGTYPE, RUNTIME_CLASS(AeSysDoc), RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(AeSysView))};

	if (!TracingDocTemplate) { return FALSE; }
	AddDocTemplate(TracingDocTemplate);

	// Create main MDI Frame window
	auto MainFrame {new CMainFrame};

	if (!MainFrame || !MainFrame->LoadFrame(IDR_MAINFRAME)) {
		delete MainFrame;
		return FALSE;
	}
	m_pMainWnd = MainFrame;

	m_pMainWnd->DragAcceptFiles();

	auto DeviceContext {MainFrame->GetDC()};

	m_DeviceWidthInPixels = static_cast<double>(DeviceContext->GetDeviceCaps(HORZRES));
	m_DeviceHeightInPixels = static_cast<double>(DeviceContext->GetDeviceCaps(VERTRES));
	m_DeviceWidthInMillimeters = static_cast<double>(DeviceContext->GetDeviceCaps(HORZSIZE));
	m_DeviceHeightInMillimeters = static_cast<double>(DeviceContext->GetDeviceCaps(VERTSIZE));

	InitGbls(DeviceContext);
	MainFrame->ReleaseDC(DeviceContext);

	m_AeSysMenuHandle = ::LoadMenuW(m_hInstance, MAKEINTRESOURCEW(IDR_AESYSTYPE));

	RefreshCommandMenu();

	// Parse command line for standard shell commands, DDE, file open
	CFullCommandLineInfo CommandLineInfo;
	ParseCommandLine(CommandLineInfo);

	if (CommandLineInfo.m_nShellCommand == CCommandLineInfo::FileNew) { CommandLineInfo.m_nShellCommand = CCommandLineInfo::FileNothing; }

	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(CommandLineInfo)) { return FALSE; }

	if (!RegisterPreviewWindowClass(m_hInstance)) { return FALSE; }

	SetShadowFolderPath(L"AeSys Shadow Folder");

	LoadSimplexStrokeFont(ResourceFolderPath() + L"Simplex.psf");
	EoDbHatchPatternTable::LoadHatchesFromFile(ResourceFolderPath() + L"Hatches\\DefaultSet.pat");
	LoadPenWidthsFromFile(ResourceFolderPath() + L"Pens\\Widths.txt");
	//LoadColorPalletFromFile(ResourceFolder + L"Pens\\Colors\\Default.txt"));

	// This is the private data format used to pass EoGroups from one instance to another
	m_ClipboardFormatIdentifierForEoGroups = RegisterClipboardFormatW(L"EoGroups");

	m_thisThreadID = ::GetCurrentThreadId();

	m_pMainWnd->ShowWindow(m_nCmdShow);
	m_pMainWnd->UpdateWindow();

	initPlotStyleSheetEnv();

	return TRUE;
}

bool AeSys::IsClipboardDataGroups() noexcept {
	return m_ClipboardDataEoGroups;
}

bool AeSys::IsClipboardDataImage() noexcept {
	return m_ClipboardDataImage;
}

bool AeSys::IsClipboardDataText() noexcept {
	return m_ClipboardDataText;
}

bool AeSys::IsTrapHighlighted() noexcept {
	return m_TrapHighlighted;
}

void AeSys::LoadColorPalletFromFile(const CString& fileName) {
	CStdioFile StreamFile;

	if (StreamFile.Open(fileName, CFile::modeRead | CFile::typeText)) {
		wchar_t Line[128] {L"\0"};

		wchar_t* Index {nullptr};
		wchar_t* Red {nullptr};
		wchar_t* Green {nullptr};
		wchar_t* Blue {nullptr};

		while (StreamFile.ReadString(Line, sizeof(Line) / sizeof(wchar_t) - 1) && _tcsnicmp(Line, L"<Colors>", 8) != 0)
			;

		while (StreamFile.ReadString(Line, sizeof(Line) / sizeof(wchar_t) - 1) && *Line != '<') {
			wchar_t* NextToken {nullptr};

			Index = wcstok_s(Line, L"=", &NextToken);
			Red = wcstok_s(nullptr, L",", &NextToken);
			Green = wcstok_s(nullptr, L",", &NextToken);
			Blue = wcstok_s(nullptr, L",", &NextToken);
			ColorPalette[_wtoi(Index)] = RGB(_wtoi(Red), _wtoi(Green), _wtoi(Blue));
			Red = wcstok_s(nullptr, L",", &NextToken);
			Green = wcstok_s(nullptr, L",", &NextToken);
			Blue = wcstok_s(nullptr, L"\n", &NextToken);
			GreyPalette[_wtoi(Index)] = RGB(_wtoi(Red), _wtoi(Green), _wtoi(Blue));
		}
	}
}

void AeSys::LoadModeResources(unsigned mode) {
	BuildModeSpecificAcceleratorTable();

	m_CurrentMode = mode;
	AddModeInformationToMessageList();

	auto ActiveView {AeSysView::GetActiveView()};

	if (ActiveView != nullptr) {
		ActiveView->SetModeCursor(m_CurrentMode);
		ActiveView->ModeLineDisplay();
		ActiveView->RubberBandingDisable();
	}
}

void AeSys::LoadPenWidthsFromFile(const CString& fileName) {
	CStdioFile StreamFile;

	if (StreamFile.Open(fileName, CFile::modeRead | CFile::typeText)) {
		wchar_t PenWidths[64];

		while (StreamFile.ReadString(PenWidths, sizeof(PenWidths) / sizeof(wchar_t) - 1)) {
			wchar_t* NextToken {nullptr};

			int PenIndex {_wtoi(wcstok_s(PenWidths, L"=", &NextToken))};
			const double Width {_wtof(wcstok_s(nullptr, L",\n", &NextToken))};

			if (PenIndex >= 0 && PenIndex < sizeof(dPWids) / sizeof(dPWids[0])) { dPWids[PenIndex] = Width; }
		}
	}
}
/// <remarks> Font stroke table encoded as follows:
/// b0 - b11  relative y displacement
/// b12 - b23 relative x displacement
/// b24 - b31 operation code (5 for line, else move)
/// The font is exactly 16384 bytes and defines a 96 character font set with a maximum of 4096 stokes
/// </remarks>
void AeSys::LoadSimplexStrokeFont(const CString& pathName) {
	auto OpenHandle {CreateFileW(pathName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)};

	if (OpenHandle != INVALID_HANDLE_VALUE) {
		if (SetFilePointer(OpenHandle, 0, nullptr, FILE_BEGIN) != INVALID_SET_FILE_POINTER) {
			if (!m_SimplexStrokeFont) { m_SimplexStrokeFont = new char[16384]; }

			unsigned long NumberOfBytesRead;

			if (!ReadFile(OpenHandle, m_SimplexStrokeFont, 16384U, &NumberOfBytesRead, nullptr)) { ReleaseSimplexStrokeFont(); }
		}
		CloseHandle(OpenHandle);
	} else {
		auto ResourceHandle {FindResourceW(nullptr, MAKEINTRESOURCEW(IDR_PEGSTROKEFONT), L"STROKEFONT")};

		if (ResourceHandle != nullptr) {
			const auto ResourceSize {SizeofResource(nullptr, ResourceHandle)};
			m_SimplexStrokeFont = new char[ResourceSize];
			auto Resource {LockResource(LoadResource(nullptr, ResourceHandle))};
			memcpy_s(m_SimplexStrokeFont, ResourceSize, Resource, ResourceSize);
		}
	}
}

CString AeSys::LoadStringResource(unsigned resourceIdentifier) const {
	CString String;
	VERIFY(String.LoadStringW(resourceIdentifier) == TRUE);
	return String;
}

bool AeSys::ModeInformationOverView() const noexcept {
	return m_ModeInformationOverView;
}

void AeSys::OnAppAbout() {
	EoDlgAbout dlg;
	dlg.DoModal();
}

void AeSys::OnEditCfGroups() noexcept {
	m_ClipboardDataEoGroups = !m_ClipboardDataEoGroups;
}

void AeSys::OnEditCfImage() noexcept {
	m_ClipboardDataImage = !m_ClipboardDataImage;
}

void AeSys::OnEditCfText() noexcept {
	m_ClipboardDataText = !m_ClipboardDataText;
}

void AeSys::OnFileOpen() {
	const unsigned long Flags {OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST};
	auto Filter {LoadStringResource(IDS_OPENFILE_FILTER)};
	CFileDialog FileDialog(TRUE, nullptr, nullptr, Flags, Filter);

	CString FileName;
	FileDialog.m_ofn.lpstrFile = FileName.GetBuffer(MAX_PATH);

	auto Title {LoadStringResource(AFX_IDS_OPENFILE)};
	FileDialog.m_ofn.lpstrTitle = Title;

	const int Result = FileDialog.DoModal();
	FileName.ReleaseBuffer();

	if (Result == IDOK) { OpenDocumentFile(FileName); }
}

void AeSys::OnFilePlotstylemanager() {
	OPENFILENAME OpenFileName;
	::ZeroMemory(&OpenFileName, sizeof(OPENFILENAME));
	OpenFileName.lStructSize = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner = nullptr;
	OpenFileName.hInstance = theApp.GetInstance();
	OpenFileName.lpstrFilter = L"Plot Style Files\0*.ctb;*.stb\0All Files\0*.*\0\0";
	OpenFileName.lpstrFile = new wchar_t[MAX_PATH];
	*OpenFileName.lpstrFile = 0;
	OpenFileName.nMaxFile = MAX_PATH;
	OpenFileName.lpstrTitle = L"Select Plot Style File";
	OpenFileName.Flags = OFN_FILEMUSTEXIST;
	OpenFileName.lpstrDefExt = L"stb";

	if (GetOpenFileNameW(&OpenFileName)) {
		OdString FileName(OpenFileName.lpstrFile);
		delete OpenFileName.lpstrFile;

		auto SystemServices {odSystemServices()};
		try {
			if (SystemServices->accessFile(FileName, Oda::kFileRead)) {
				OdStreamBufPtr StreamBuffer(SystemServices->createFile(FileName));
				OdPsPlotStyleTablePtr pPlotStyleTable;
				if (StreamBuffer.get()) {
					OdPsPlotStyleServicesPtr PlotStyleServices {odrxDynamicLinker()->loadApp(ODPS_PLOTSTYLE_SERVICES_APPNAME)};
					if (PlotStyleServices.get()) {
						pPlotStyleTable = PlotStyleServices->loadPlotStyleTable(StreamBuffer);
					}
				}
				EoDlgPlotStyleManager PsTableEditorDlg(theApp.GetMainWnd());
				PsTableEditorDlg.SetFileBufPath(FileName);
				PsTableEditorDlg.SetPlotStyleTable(pPlotStyleTable);
				if (PsTableEditorDlg.DoModal() == IDOK) {
					pPlotStyleTable->copyFrom(PsTableEditorDlg.GetPlotStyleTable());
				}
			}
		} catch (...) {
		}
	}
}

void AeSys::OnHelpContents() {
	::HtmlHelpW(AfxGetMainWnd()->GetSafeHwnd(), L"..\\AeSys\\hlp\\AeSys.chm", HH_DISPLAY_TOPIC, NULL);
}

void AeSys::OnModeAnnotate() {
	m_ModeResourceIdentifier = IDR_ANNOTATE_MODE;
	m_PrimaryMode = ID_MODE_ANNOTATE;
	LoadModeResources(ID_MODE_ANNOTATE);
}

void AeSys::OnModeCut() {
	m_ModeResourceIdentifier = IDR_CUT_MODE;
	m_PrimaryMode = ID_MODE_CUT;
	LoadModeResources(ID_MODE_CUT);
}

void AeSys::OnModeDimension() {
	m_ModeResourceIdentifier = IDR_DIMENSION_MODE;
	m_PrimaryMode = ID_MODE_DIMENSION;
	LoadModeResources(ID_MODE_DIMENSION);
}

void AeSys::OnModeDraw() {
	m_ModeResourceIdentifier = IDR_DRAW_MODE;
	m_PrimaryMode = ID_MODE_DRAW;
	LoadModeResources(ID_MODE_DRAW);
}

void AeSys::OnModeDraw2() {
	m_ModeResourceIdentifier = IDR_DRAW2_MODE;
	m_PrimaryMode = ID_MODE_DRAW2;
	LoadModeResources(ID_MODE_DRAW2);
}

void AeSys::OnModeEdit() {
	m_ModeResourceIdentifier = IDR_EDIT_MODE;
	LoadModeResources(ID_MODE_EDIT);
}

void AeSys::OnModeFixup() {
	m_ModeResourceIdentifier = IDR_FIXUP_MODE;
	LoadModeResources(ID_MODE_FIXUP);
}

void AeSys::OnModeLetter() {
	EoDlgModeLetter Dialog;
	Dialog.DoModal();
}

void AeSys::OnModeLPD() {
	m_ModeResourceIdentifier = IDR_LPD_MODE;
	LoadModeResources(ID_MODE_LPD);
}

void AeSys::OnModeNodal() {
	m_ModeResourceIdentifier = IDR_NODAL_MODE;
	LoadModeResources(ID_MODE_NODAL);
}

void AeSys::OnModePipe() {
	m_ModeResourceIdentifier = IDR_PIPE_MODE;
	LoadModeResources(ID_MODE_PIPE);
}

void AeSys::OnModePower() {
	m_ModeResourceIdentifier = IDR_POWER_MODE;
	LoadModeResources(ID_MODE_POWER);
}

void AeSys::OnModeRevise() {
	EoDlgModeRevise Dialog;
	Dialog.DoModal();
}

void AeSys::OnModeTrap() {
	if (m_TrapModeAddGroups) {
		m_ModeResourceIdentifier = IDR_TRAP_MODE;
		LoadModeResources(ID_MODE_TRAP);
	} else {
		m_ModeResourceIdentifier = IDR_TRAPR_MODE;
		LoadModeResources(ID_MODE_TRAPR);
	}
}

void AeSys::OnToolsLoadapplications() {
	EoLoadApps LoadAppsDialog;
	LoadAppsDialog.DoModal();
}

void AeSys::OnTrapCommandsAddGroups() {
	m_TrapModeAddGroups = !m_TrapModeAddGroups;
	m_CurrentMode = m_TrapModeAddGroups ? ID_MODE_TRAP : ID_MODE_TRAPR;

	OnModeTrap();
}

void AeSys::OnTrapCommandsHighlight() noexcept {
	m_TrapHighlighted = !m_TrapHighlighted;
	//LPARAM Hint = m_TrapHighlighted ? kGroupsSafeTrap : kGroupsSafe;
	//UpdateGroupsInAllViews(Hint, &m_TrappedGroupList);
}

void AeSys::OnUpdateEditCfGroups(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_ClipboardDataEoGroups);
}

void AeSys::OnUpdateEditCfImage(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_ClipboardDataImage);
}

void AeSys::OnUpdateEditCfText(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_ClipboardDataText);
}

void AeSys::OnUpdateModeAnnotate(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_ANNOTATE);
}

void AeSys::OnUpdateModeCut(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_CUT);
}

void AeSys::OnUpdateModeDimension(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_DIMENSION);
}

void AeSys::OnUpdateModeDraw(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_DRAW);
}

void AeSys::OnUpdateModeDraw2(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_DRAW2);
}

void AeSys::OnUpdateModeEdit(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_EDIT);
}

void AeSys::OnUpdateModeFixup(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_FIXUP);
}

void AeSys::OnUpdateModeLpd(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_LPD);
}

void AeSys::OnUpdateModeNodal(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_NODAL);
}

void AeSys::OnUpdateModePipe(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_PIPE);
}

void AeSys::OnUpdateModePower(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_POWER);
}

void AeSys::OnUpdateModeTrap(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_TRAP);
}

void AeSys::OnUpdateTrapcommandsAddgroups(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_TrapModeAddGroups);
}

void AeSys::OnUpdateTrapcommandsHighlight(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_TrapHighlighted);
}

void AeSys::OnUpdateViewModeinformation(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_ModeInformationOverView);
}


COLORREF AppGetTextCol() noexcept {
	return (~(ViewBackgroundColor | 0xff000000));
}

void AeSys::OnViewModeInformation() {
	m_ModeInformationOverView = !m_ModeInformationOverView;
	AeSysDoc::GetDoc()->UpdateAllViews(nullptr);
}

double AeSys::ParseLength(const wchar_t* lengthAsString) {
	wchar_t* StopString;

	auto ReturnValue {wcstod(lengthAsString, &StopString)};

	switch (toupper((int) StopString[0])) {
		case '\'': // Feet and maybe inches
			ReturnValue *= 12.0; // Reduce to inches
			ReturnValue += wcstod(&StopString[1], &StopString); // Begin scan for inches at character following foot delimeter
			break;

		case 'M': // meters or millimeters
			if (toupper((int) StopString[1]) == 'M') {
				ReturnValue *= 0.03937007874015748;
			} else {
				ReturnValue *= 39.37007874015748;
			}
			break;

		case 'C': // centimenters
			ReturnValue *= 0.3937007874015748;
			break;

		case 'D': // decimeters
			ReturnValue *= 3.937007874015748;
			break;

		case 'K': // kilometers
			ReturnValue *= 39370.07874015748;
	}
	return (ReturnValue / AeSysView::GetActiveView()->WorldScale());
}

double AeSys::ParseLength(Units units, const wchar_t* lengthAsString) {
	try {
		int iTokId {0};
		long DataDefinition;
		int iTyp;
		double ReturnValue[32];

		lex::Parse(lengthAsString);
		lex::EvalTokenStream(&iTokId, &DataDefinition, &iTyp, (void*) ReturnValue);

		if (iTyp == lex::TOK_LENGTH_OPERAND) {
			return (ReturnValue[0]);
		} else {
			lex::ConvertValTyp(iTyp, lex::TOK_REAL, &DataDefinition, ReturnValue);

			switch (units) {
				case kArchitectural:
				case kArchitecturalS:
				case kEngineering:
				case kFeet:
					ReturnValue[0] *= 12.;
					break;

				case kMeters:
					ReturnValue[0] *= 39.37007874015748;
					break;

				case kMillimeters:
					ReturnValue[0] *= .03937007874015748;
					break;

				case kCentimeters:
					ReturnValue[0] *= .3937007874015748;
					break;

				case kDecimeters:
					ReturnValue[0] *= 3.937007874015748;
					break;

				case kKilometers:
					ReturnValue[0] *= 39370.07874015748;
					break;

				case kInches:
				default:
					break;
			}
			ReturnValue[0] /= AeSysView::GetActiveView()->WorldScale();
		}
		return (ReturnValue[0]);
	} catch (const wchar_t* Message) {
		::MessageBoxW(nullptr, Message, nullptr, MB_ICONWARNING | MB_OK);
		return (0.0);
	}
}

double AeSys::PenWidthsGet(short colorIndex) noexcept {
	return (dPWids[colorIndex]);
}
/// <remarks> Processing occurs immediately before the framework loads the application state from the registry. </remarks>
void AeSys::PreLoadState() {
	GetContextMenuManager()->AddMenu(L"My menu", IDR_CONTEXT_MENU);

	// <tas="add other context menus here"/>
}

int AeSys::PrimaryMode() const noexcept {
	return m_PrimaryMode;
}

const OdString AeSys::product() {
	return L"AeSys Application";
}

bool GetRegistryString(HKEY key, const wchar_t* subkey, const wchar_t* name, wchar_t* value, int size) noexcept {
	bool ReturnValue {false};
	HKEY OpenedKey;

	if (RegOpenKeyExW(key, subkey, 0, KEY_READ, &OpenedKey) == ERROR_SUCCESS) {
		unsigned long RegistryBufferSize {EO_REGISTRY_BUFFER_SIZE};
		unsigned char data[EO_REGISTRY_BUFFER_SIZE] {0};

		wchar_t data_t[EO_REGISTRY_BUFFER_SIZE] {L"\0"};

		if (RegQueryValueExW(OpenedKey, name, nullptr, nullptr, data, &RegistryBufferSize) == ERROR_SUCCESS) {
			memcpy_s(&data_t, EO_REGISTRY_BUFFER_SIZE, &data, RegistryBufferSize);
			ReturnValue = true;
		} else {
			if (ERROR_SUCCESS == RegEnumKeyExW(OpenedKey, 0, data_t, &RegistryBufferSize, nullptr, nullptr, nullptr, nullptr)) { ReturnValue = true; }
		}
		if (size < EO_REGISTRY_BUFFER_SIZE) {
			swprintf_s(value, static_cast<size_t>(size), L"%s\0", data_t);
		} else {
			wcsncpy(value, data_t, static_cast<size_t>(size - 1));
			value[size - 1] = '\0';
		}
		RegCloseKey(OpenedKey);
	}
	return ReturnValue;
}

CString GetRegistryAcadLocation() {
	CString SubKey {L"SOFTWARE\\Autodesk\\AutoCAD"};

	wchar_t Version[32] {L"\0"};

	if (GetRegistryString(HKEY_LOCAL_MACHINE, SubKey, L"CurVer", Version, 32) == 0) { return L""; }

	SubKey += L"\\";
	SubKey += Version;

	wchar_t SubVersion[32] {L"\0"};

	if (GetRegistryString(HKEY_LOCAL_MACHINE, SubKey, L"CurVer", SubVersion, 32) == 0) { return L""; }

	SubKey += L"\\";
	SubKey += SubVersion;

	wchar_t searchPaths[EO_REGISTRY_MAX_PATH] {L"\0"};

	if (GetRegistryString(HKEY_LOCAL_MACHINE, SubKey, L"AcadLocation", searchPaths, EO_REGISTRY_MAX_PATH) == 0) { return L""; }

	return CString(searchPaths);
}

CString GetRegistryAcadProfilesKey() {
	CString Subkey {L"SOFTWARE\\Autodesk\\AutoCAD"};

	wchar_t Version[32] {L"\0"};

	if (GetRegistryString(HKEY_CURRENT_USER, Subkey, L"CurVer", Version, 32) == 0) { return L""; }

	Subkey += L"\\";
	Subkey += Version;

	wchar_t SubVersion[32] {L"\0"};

	if (GetRegistryString(HKEY_CURRENT_USER, Subkey, L"CurVer", SubVersion, 32) == 0) { return L""; }

	Subkey += L"\\";
	Subkey += SubVersion;
	Subkey += L"\\Profiles";

	wchar_t Profile[EO_REGISTRY_MAX_PROFILE_NAME] {L"\0"};

	if (GetRegistryString(HKEY_CURRENT_USER, Subkey, L"", Profile, EO_REGISTRY_MAX_PROFILE_NAME) == 0) { return L""; }

	Subkey += L"\\";
	Subkey += Profile;

	return Subkey;
}

OdString AeSys::getSubstituteFont(const OdString& fontName, OdFontType fontType) {
	return OdString(L"simplex.shx");
}

OdString AeSys::getFontMapFileName() const {
	OdString subkey;
	wchar_t fontMapFile[EO_REGISTRY_MAX_PATH] {L"\0"};
	wchar_t expandedPath[EO_REGISTRY_MAX_PATH] {L"\0"};

	subkey = GetRegistryAcadProfilesKey();
	if (!subkey.isEmpty()) {
		subkey += L"\\Editor Configuration";

		if (GetRegistryString(HKEY_CURRENT_USER, subkey, L"FontMappingFile", fontMapFile, EO_REGISTRY_MAX_PATH) == 0) {
			return L"";
		}
		ExpandEnvironmentStringsW(fontMapFile, expandedPath, EO_REGISTRY_MAX_PATH);
		return OdString(expandedPath);
	} else {
		return L"C:\\acad.fmp";
	}
}

OdString AeSys::getTempPath() const {
	OdString subkey;
	wchar_t TempPath[MAX_PATH] {L"\0"};

	subkey = GetRegistryAcadProfilesKey();

	if (!subkey.isEmpty()) {
		subkey += L"\\General Configuration";

		if (GetRegistryString(HKEY_CURRENT_USER, subkey, L"TempDirectory", TempPath, MAX_PATH) == 0) {
			return OdDbHostAppServices::getTempPath();
		}
		if (_waccess(TempPath, 0)) {
			return OdDbHostAppServices::getTempPath();
		} else {
			CString Result(TempPath, (int) wcslen(TempPath));
			if (Result.GetAt(Result.GetLength() - 1) != '\\') { Result += '\\'; }
			return Result.GetString();
		}
	} else {
		return OdDbHostAppServices::getTempPath();
	}
}

void AeSys::ReleaseSimplexStrokeFont() noexcept {
	if (m_SimplexStrokeFont) {
		delete[] m_SimplexStrokeFont;
	}
}

OdString AeSys::recentGsDevicePath() const {
	return m_sVectorizerPath;
}

void AeSys::setRecentGsDevicePath(const OdString& vectorizerPath) {
	WriteProfileStringW(L"options", L"recent GS", vectorizerPath);
	m_sVectorizerPath = vectorizerPath;
}

void AeSys::SetStatusPaneTextAt(int index, const wchar_t* newText) {
	dynamic_cast<CMainFrame*>(GetMainWnd())->SetStatusPaneTextAt(index, newText);
}

OdDbHostAppProgressMeter* AeSys::newProgressMeter() {
	if (m_thisThreadID != ::GetCurrentThreadId()) { return nullptr; }

	return ExHostAppServices::newProgressMeter();
}

void AeSys::start(const OdString& displayString) {
	m_ProgressMessage = (const wchar_t*) displayString;
	m_ProgressPosition = 0;
	m_ProgressPercent = -1;
	// <tas="m_tbExt.SetProgressState(::AfxGetMainWnd()->GetSafeHwnd(), CTaskBarWin7Ext::PS_Normal);"</tas>
	// <tas="m_tbExt.SetProgressValue(::AfxGetMainWnd()->GetSafeHwnd(), 0, 100);"</tas>
}

void AeSys::stop() {
	m_ProgressPosition = m_ProgressLimit;
	meterProgress();
	// <tas="m_tbExt.SetProgressState(::AfxGetMainWnd()->GetSafeHwnd(), CTaskBarWin7Ext::PS_NoProgress);"</tas>
	// <tas="m_tbExt.FlashWindow(::AfxGetMainWnd()->GetSafeHwnd());"</tas>
}

void AeSys::meterProgress() {
	bool UpdateProgress {false};
	int Percent {0};
	{
		TD_AUTOLOCK_P_DEF(m_pMeterMutex);
		const int OldPercent {m_ProgressPercent};
		m_ProgressPercent = static_cast<int>((static_cast<double>(m_ProgressPosition++) / static_cast<double>(m_ProgressLimit)) * 100);
		Percent = m_ProgressPercent;
		UpdateProgress = (OldPercent != m_ProgressPercent);
	}
	if (UpdateProgress) {
		struct StatusUpdater {
			int m_Percent;
			CMainFrame* m_MainFrame;
			AeSys* m_Application;
			StatusUpdater(int percent, CMainFrame* mainFrame, AeSys* application) noexcept
				: m_Percent(percent)
				, m_MainFrame(mainFrame)
				, m_Application(application) {
			}
			static void Exec(void* statusUpdater) {
				auto pExec {static_cast<StatusUpdater*>(statusUpdater)};
				CString str;
				str.Format(L"%s %d", pExec->m_Application->m_ProgressMessage.GetString(), pExec->m_Percent);
				// <tas="pExec->m_MainFrame->m_wndStatusBar.SetPaneText(0, str);:</tas>
				// <tas="pExec->m_Application->m_tbExt.SetProgressValue(::AfxGetMainWnd()->GetSafeHwnd(), (ULONG) pExec->m_ProgressPercent, 100);"</tas>
				MSG Message;
				while (::PeekMessageW(&Message, pExec->m_MainFrame->m_hWnd, WM_KEYUP, WM_KEYUP, 1)) {
					bool bDup {false};

					if (Message.wParam == VK_ESCAPE && !bDup) {
						bDup = true;
						str.Format(L"Are you sure you want to terminate\n%s ?", pExec->m_Application->m_ProgressMessage.GetString());
						// <tas="pExec->m_Application->m_tbExt.SetProgressState(::AfxGetMainWnd()->GetSafeHwnd(), CTaskBarWin7Ext::PS_Paused);"</tas>
						if (AfxMessageBox(str, MB_YESNO | MB_ICONQUESTION) == IDYES) {
							// <tas="pExec->m_Application->m_tbExt.SetProgressState(::AfxGetMainWnd()->GetSafeHwnd(), CTaskBarWin7Ext::PS_NoProgress);"</tas>
							throw UserBreak();
						} else {
							// <tas="pExec->m_Application->m_tbExt.SetProgressState(::AfxGetMainWnd()->GetSafeHwnd(), CTaskBarWin7Ext::PS_Normal);:</tas>
						}
					}
				}
			}
		} execArg(Percent, dynamic_cast<CMainFrame*>(GetMainWnd()), this);
		odExecuteMainThreadAction(StatusUpdater::Exec, &execArg);
	}
}

void AeSys::setLimit(int max) noexcept {
	m_ProgressLimit = max ? max : 1;
}

int AeSys::ConfirmMessageBox(unsigned stringResourceIdentifier, const wchar_t* string) {
	auto FormatSpecification {LoadStringResource(stringResourceIdentifier)};

	CString FormattedResourceString;
	FormattedResourceString.Format(FormatSpecification, string);

	int NextToken = 0;
	CString Text = FormattedResourceString.Tokenize(L"\t", NextToken);
	CString Caption = FormattedResourceString.Tokenize(L"\n", NextToken);

	return (::MessageBoxW(nullptr, Text, Caption, MB_ICONINFORMATION | MB_YESNOCANCEL | MB_DEFBUTTON2));
}

void AeSys::warning(const char* warnVisGroup, const OdString& text) {
	if (m_bLoading && (!warnVisGroup || !*warnVisGroup) && !m_bUseMTLoading) {
		if (::MessageBoxW(nullptr, text + L"\n\nDo you want to proceed ?", L"Warning!", MB_ICONWARNING | MB_YESNO) == IDNO) { throw UserBreak(); }
	}
}

void AeSys::WarningMessageBox(unsigned stringResourceIdentifier) {
	auto ResourceString {LoadStringResource(stringResourceIdentifier)};

	int NextToken = 0;
	CString Text = ResourceString.Tokenize(L"\t", NextToken);
	CString Caption = ResourceString.Tokenize(L"\n", NextToken);

	::MessageBoxW(nullptr, Text, Caption, MB_ICONWARNING | MB_OK);
}

void AeSys::WarningMessageBox(unsigned stringResourceIdentifier, const wchar_t* string) {
	auto FormatSpecification {LoadStringResource(stringResourceIdentifier)};

	CString FormattedResourceString;
	FormattedResourceString.Format(FormatSpecification, string);

	int NextToken = 0;
	CString Text = FormattedResourceString.Tokenize(L"\t", NextToken);
	CString Caption = FormattedResourceString.Tokenize(L"\n", NextToken);

	::MessageBoxW(nullptr, Text, Caption, MB_ICONWARNING | MB_OK);
}

void AeSys::initPlotStyleSheetEnv() {
	auto StyleSheetFiles {FindConfigPath(L"PrinterStyleSheetDir")};
	_wputenv_s(L"DDPLOTSTYLEPATHS", StyleSheetFiles);
}

CString AeSys::ResourceFolderPath() {
	return (getApplicationPath() + L"\\res\\");
}

void AeSys::SetArchitecturalUnitsFractionPrecision(const int precision) noexcept {
	if (precision > 0) m_ArchitecturalUnitsFractionPrecision = precision;
}

void AeSys::SetDimensionAngle(double angle) noexcept {
	m_DimensionAngle = angle;
}

void AeSys::SetDimensionLength(double length) noexcept {
	m_DimensionLength = length;
}

void AeSys::SetEngagedAngle(double angle) noexcept {
	m_EngagedAngle = angle;
}

void AeSys::SetEngagedLength(double length) noexcept {
	m_EngagedLength = length;
}

int AeSys::SetShadowFolderPath(const CString& folder) {
	wchar_t Path[MAX_PATH];

	if (SHGetSpecialFolderPathW(m_pMainWnd->GetSafeHwnd(), Path, CSIDL_PERSONAL, TRUE)) {
		m_ShadowFolderPath = Path;
	} else {
		m_ShadowFolderPath.Empty();
	}
	m_ShadowFolderPath += L"\\" + folder + L"\\";

	return (_wmkdir(m_ShadowFolderPath));
}

void AeSys::SetUnits(Units units) noexcept {
	m_Units = units;
}

CString AeSys::ShadowFolderPath() const noexcept {
	return m_ShadowFolderPath;
}

char* AeSys::SimplexStrokeFont() noexcept {
	return m_SimplexStrokeFont;
}

short AeSys::TrapHighlightColor() const noexcept {
	return m_TrapHighlightColor;
}

void AeSys::UninitializeTeigha() {
	OdApplicationReactor::rxUninit();
	OdApplicationDocument::rxUninit();

	EoLoadApps::rxUninit();

	try {
		/* <tas>
		rxUninitMaterialsEditorObjects();
		   </tas> */

		auto CommandStack {::odedRegCmds()};
		CommandStack->removeCmd(&g_Cmd_SELECT);
		CommandStack->removeCmd(&g_Cmd_VIEW);

		OdDbDatabaseDoc::rxUninit();

#ifdef OD_OLE_SUPPORT
		::rxUninit_COleClientItem_handler();
#endif // OD_OLE_SUPPORT

		removePaperDrawingCustomization();
		removeMaterialTextureLoadingMonitor();

		::odUninitialize();
	} catch (const OdError& Error) {
		theApp.reportError(L"", Error);
	}
}

void AeSys::UpdateMDITabs(BOOL resetMDIChild) {
	dynamic_cast<CMainFrame*>(AfxGetMainWnd())->UpdateMDITabs(resetMDIChild);
}

BOOL AeSys::OnIdle(long count) {
	for (unsigned ReactorIndex = 0; ReactorIndex < m_ApplicationReactors.size(); ++ReactorIndex) {
		m_ApplicationReactors.at(ReactorIndex)->OnIdle(count);
	}
	return __super::OnIdle(count);
}

BOOL AeSys::PreTranslateMessage(MSG* message) {
	for (unsigned ReactorIndex = 0; ReactorIndex < m_ApplicationReactors.size(); ++ReactorIndex) {
		m_ApplicationReactors.at(ReactorIndex)->OnPreTranslateMessage(message);
	}
	return __super::PreTranslateMessage(message);
}

/// <section="vectorizer menu - add new and clear all">

bool addGsMenuItem(CMenu* vectorizePopupMenu, unsigned long& numberOfVectorizers, const wchar_t* vectorizerPath) {
	if (ID_VECTORIZER_FIRST + numberOfVectorizers <= ID_VECTORIZER_LAST) {
		vectorizePopupMenu->InsertMenuW(numberOfVectorizers, MF_BYPOSITION, ID_VECTORIZER_FIRST + numberOfVectorizers, vectorizerPath);

		MENUITEMINFO menuItemInfo;
		menuItemInfo.cbSize = sizeof(menuItemInfo);
		menuItemInfo.fMask = MIIM_DATA;
		menuItemInfo.dwItemData = static_cast<unsigned long>(theApp.getGSMenuItemMarker());
		VERIFY(::SetMenuItemInfoW(vectorizePopupMenu->m_hMenu, numberOfVectorizers, TRUE, &menuItemInfo));

		if (theApp.recentGsDevicePath().iCompare(OdString(vectorizerPath)) == 0) {
			vectorizePopupMenu->CheckMenuItem(numberOfVectorizers, MF_BYPOSITION | MF_CHECKED);
		}
		++numberOfVectorizers;
		return true;
	}
	return false;
}

void AeSys::OnVectorizeAddVectorizerDLL() {
	const unsigned long Flags {OFN_HIDEREADONLY | OFN_EXPLORER | OFN_PATHMUSTEXIST};
	CString Filter {L"Graphic System DLL (*." VECTORIZATION_MODULE_EXTENSION_W L")|*." VECTORIZATION_MODULE_EXTENSION_W L"|Windows DLL (*.dll)|*.dll||"};

	CFileDialog FileDialog(TRUE, VECTORIZATION_MODULE_EXTENSION_W, L"", Flags, Filter, ::AfxGetMainWnd());
	FileDialog.m_ofn.lpstrTitle = L"Select Graphic System DLL";

	auto ApplicationPath {getApplicationPath()};
	FileDialog.m_ofn.lpstrInitialDir = ApplicationPath.GetBuffer(ApplicationPath.GetLength());

	if (FileDialog.DoModal() == IDOK) {
		m_sVectorizerPath = (const wchar_t*) FileDialog.GetFileName();
		m_sVectorizerPath.replace(TD_DLL_VERSION_SUFFIX_STR, L"");

		const auto TopMenu {CMenu::FromHandle(theApp.GetAeSysMenu())};
		auto VectorizePopupMenu {TopMenu->GetSubMenu(3)};

		::addGsMenuItem(VectorizePopupMenu, m_numGSMenuItems, m_sVectorizerPath);
		WriteProfileStringW(L"options\\vectorizers", m_sVectorizerPath, L"");
		GetMainWnd()->SendMessageW(WM_COMMAND, ID_VECTORIZE);
	}
	ApplicationPath.ReleaseBuffer();
}

void AeSys::OnUpdateVectorizeAddvectorizerdll(CCmdUI* pCmdUI) {
	if (m_numGSMenuItems == 0) {
		const auto TopMenu {CMenu::FromHandle(theApp.GetAeSysMenu())};
		auto VectorizePopupMenu {TopMenu->GetSubMenu(3)};

		CRegKey RegistryKey;
		RegistryKey.Create(HKEY_CURRENT_USER, L"Software\\Engineers Office\\AeSys\\options\\vectorizers");

		CString Path;
		unsigned long PathSize;

		for (;;) {
			PathSize = _MAX_FNAME + _MAX_EXT;
			const auto Status {::RegEnumValueW(RegistryKey, m_numGSMenuItems, Path.GetBuffer(static_cast<int>(PathSize)), &PathSize, nullptr, nullptr, nullptr, nullptr)};
			Path.ReleaseBuffer();

			if (Status == ERROR_SUCCESS) {
				if (!::addGsMenuItem(VectorizePopupMenu, m_numGSMenuItems, Path)) { break; }

			} else {
				break;
			}
		}
	}
}

void AeSys::OnVectorizeClearmenu() {
	const auto TopMenu {CMenu::FromHandle(theApp.GetAeSysMenu())};
	auto VectorizePopupMenu {TopMenu->GetSubMenu(3)};

	while (VectorizePopupMenu->GetMenuItemCount() > 3) {
		VectorizePopupMenu->RemoveMenu(0, MF_BYPOSITION);
	}
	CRegKey RegistryKey;
	RegistryKey.Create(HKEY_CURRENT_USER, L"Software\\Engineers Office\\AeSys\\options");
	RegistryKey.RecurseDeleteKey(L"vectorizers");
	setRecentGsDevicePath(OdString::kEmpty);
	m_numGSMenuItems = 0;
}

void AeSys::OnUpdateVectorizeClearmenu(CCmdUI* pCmdUI) {
	pCmdUI->Enable(m_numGSMenuItems > 0);
}
/// </section>
