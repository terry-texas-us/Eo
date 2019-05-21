#include "stdafx.h"

#include "afxwinappex.h"

#include "ChildFrm.h"
#include "AeSysApp.h"
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

#include "..\win\ExtDialog\FileDlgExt.h"

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
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

ATOM WINAPI RegisterPreviewWindowClass(HINSTANCE instance);

double dPWids[] = {
	0., .0075, .015, .02, .03, .0075, .015, .0225, .03, .0075, .015, .0225, .03, .0075, .015, .0225
};

#include "PegColors.h"

COLORREF* pColTbl = ColorPalette;

CPrimState pstate;

// This is a legacy feature. All values are empty strings now for normal MouseButton command processing.
// User may still change through user interface, so must not assume empty.

CString AeSysApp::CustomLButtonDownCharacters(L"");
CString AeSysApp::CustomLButtonUpCharacters(L"" /* L"{13}" for VK_RETURN */);
CString AeSysApp::CustomRButtonDownCharacters(L"");
CString AeSysApp::CustomRButtonUpCharacters(L"" /* L"{27}" for VK_ESCAPE */);

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
			if (points[0] == points[1] || points[1] == points[2]) {
				return true;
			}
			int NumberOfDashes = 15;
			OdGiGeometry& Geometry = worldDraw->geometry();
			OdGePoint3d Dash1[2];
			OdGePoint3d Dash2[2];
			OdGeVector3d Step = (points[1] - points[0]) / (double(NumberOfDashes) * 2. + 1.);
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
			NumberOfDashes = int((points[2] - points[1]).length() / Step.length() - 1) / 2;
			Step = (points[2] - points[1]) / (double(NumberOfDashes) * 2. + 1.);
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
	}
	s_PaperDrawExt;

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
			ATLTRACE2(atlTraceGeneral, 0, L"Material texture loaded: %s\n", fileName.c_str());
		}
		/// <remarks> Called by texture loader after file loading, only if texture loading failed. </remarks>
		void textureLoadingFailed(const OdString& fileName, OdDbBaseDatabase* database) override {
			ATLTRACE2(atlTraceGeneral, 0, L"Failed to load material texture: %s\n", fileName.c_str());
		}
	}
	s_MatLoadExt;

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

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void EoDlgAbout::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(EoDlgAbout, CDialog)
END_MESSAGE_MAP()

// AeSys

BEGIN_MESSAGE_MAP(AeSysApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &AeSysApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &AeSysApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)

	ON_COMMAND(ID_EDIT_CF_GROUPS, &AeSysApp::OnEditCfGroups)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CF_GROUPS, &AeSysApp::OnUpdateEditCfGroups)
	ON_COMMAND(ID_EDIT_CF_IMAGE, &AeSysApp::OnEditCfImage)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CF_IMAGE, &AeSysApp::OnUpdateEditCfImage)
	ON_COMMAND(ID_EDIT_CF_TEXT, &AeSysApp::OnEditCfText)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CF_TEXT, &AeSysApp::OnUpdateEditCfText)
	ON_COMMAND(ID_HELP_CONTENTS, &AeSysApp::OnHelpContents)
	ON_COMMAND(ID_MODE_ANNOTATE, &AeSysApp::OnModeAnnotate)
	ON_UPDATE_COMMAND_UI(ID_MODE_ANNOTATE, &AeSysApp::OnUpdateModeAnnotate)
	ON_COMMAND(ID_MODE_CUT, &AeSysApp::OnModeCut)
	ON_UPDATE_COMMAND_UI(ID_MODE_CUT, &AeSysApp::OnUpdateModeCut)
	ON_COMMAND(ID_MODE_DIMENSION, &AeSysApp::OnModeDimension)
	ON_UPDATE_COMMAND_UI(ID_MODE_DIMENSION, &AeSysApp::OnUpdateModeDimension)
	ON_COMMAND(ID_MODE_DRAW, OnModeDraw)
	ON_UPDATE_COMMAND_UI(ID_MODE_DRAW, &AeSysApp::OnUpdateModeDraw)
	ON_COMMAND(ID_MODE_DRAW2, &AeSysApp::OnModeDraw2)
	ON_UPDATE_COMMAND_UI(ID_MODE_DRAW2, &AeSysApp::OnUpdateModeDraw2)
	ON_COMMAND(ID_MODE_EDIT, &AeSysApp::OnModeEdit)
	ON_UPDATE_COMMAND_UI(ID_MODE_EDIT, &AeSysApp::OnUpdateModeEdit)
	ON_COMMAND(ID_MODE_FIXUP, &AeSysApp::OnModeFixup)
	ON_UPDATE_COMMAND_UI(ID_MODE_FIXUP, &AeSysApp::OnUpdateModeFixup)
	ON_COMMAND(ID_MODE_LETTER, &AeSysApp::OnModeLetter)
	ON_COMMAND(ID_MODE_LPD, &AeSysApp::OnModeLPD)
	ON_UPDATE_COMMAND_UI(ID_MODE_LPD, &AeSysApp::OnUpdateModeLpd)
	ON_COMMAND(ID_MODE_NODAL, &AeSysApp::OnModeNodal)
	ON_UPDATE_COMMAND_UI(ID_MODE_NODAL, &AeSysApp::OnUpdateModeNodal)
	ON_COMMAND(ID_MODE_PIPE, &AeSysApp::OnModePipe)
	ON_UPDATE_COMMAND_UI(ID_MODE_PIPE, &AeSysApp::OnUpdateModePipe)
	ON_COMMAND(ID_MODE_POWER, &AeSysApp::OnModePower)
	ON_UPDATE_COMMAND_UI(ID_MODE_POWER, &AeSysApp::OnUpdateModePower)
	ON_COMMAND(ID_MODE_REVISE, &AeSysApp::OnModeRevise)
	ON_COMMAND(ID_MODE_TRAP, &AeSysApp::OnModeTrap)
	ON_UPDATE_COMMAND_UI(ID_MODE_TRAP, &AeSysApp::OnUpdateModeTrap)
	ON_COMMAND(ID_TRAPCOMMANDS_ADDGROUPS, &AeSysApp::OnTrapCommandsAddGroups)
	ON_UPDATE_COMMAND_UI(ID_TRAPCOMMANDS_ADDGROUPS, &AeSysApp::OnUpdateTrapcommandsAddgroups)
	ON_COMMAND(ID_TRAPCOMMANDS_HIGHLIGHT, &AeSysApp::OnTrapCommandsHighlight)
	ON_UPDATE_COMMAND_UI(ID_TRAPCOMMANDS_HIGHLIGHT, &AeSysApp::OnUpdateTrapcommandsHighlight)
	ON_COMMAND(ID_VIEW_MODEINFORMATION, &AeSysApp::OnViewModeInformation)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MODEINFORMATION, &AeSysApp::OnUpdateViewModeinformation)
	ON_COMMAND(ID_FILE_PLOTSTYLEMANAGER, &AeSysApp::OnFilePlotstylemanager)
	ON_COMMAND(ID_TOOLS_LOADAPPLICATIONS, &AeSysApp::OnToolsLoadapplications)
	ON_COMMAND(ID_VECTORIZE_ADDVECTORIZERDLL, &AeSysApp::OnVectorizeAddVectorizerDLL)
	ON_UPDATE_COMMAND_UI(ID_VECTORIZE_ADDVECTORIZERDLL, &AeSysApp::OnUpdateVectorizeAddvectorizerdll)
	ON_COMMAND(ID_VECTORIZE_CLEARMENU, OnVectorizeClearmenu)
	ON_UPDATE_COMMAND_UI(ID_VECTORIZE_CLEARMENU, OnUpdateVectorizeClearmenu)

END_MESSAGE_MAP()

AeSysApp::AeSysApp() noexcept
	: m_nProgressPos(0)
	, m_nProgressLimit(100)
	, m_nPercent(0)
	, m_pAuditDlg(nullptr)
	, m_bUseGsModel(TRUE)
	, m_numGSMenuItems(0)
	, m_bDiscardBackFaces(1)
	, m_bEnableHLR(0)
	, m_bContextColors(1)
	, m_bTTFPolyDraw(0)
	, m_bTTFTextOut(0)
	, m_bTTFCache(0)
	, m_bDynamicSubEntHlt(0)
	, m_bGDIGradientsAsBitmap(0)
	, m_bGDIGradientsAsPolys(0)
	, m_nGDIGradientsAsPolysThreshold(10)
	, m_bDisableAutoRegen(0)
	, m_bLoading(false)
	, m_bRemoteGeomViewer(false)
	, m_bSupportFileSelectionViaDialog(true)
	// ODA_MT_DBIO_BEGIN
	, m_bUseMTLoading(false)
	// ODA_MT_DBIO_END
	, m_bUseTempFiles(false)
	, m_pagingType(0)

	, m_bEnableDoubleBuffer(1)
	, m_bBlocksCache(0)
	, m_bGsDevMultithread(0)
	, m_nMtRegenThreads(4)
	, m_bEnablePrintPreviewViaBitmap(1)
	, m_background(ViewBackgroundColor)
	, m_thisThreadID(0)
	, m_numCustomCommands(0)
	, m_bSaveRoundTrip(1)
	, m_bSavePreview(0)
	, m_bSaveWithPassword(0)
	, m_bPartial(false)
	, m_bRecover(false) {

	EnableHtmlHelp();

	// Detect color depth. 256 color toolbars can be used in the high or true color modes only (bits per pixel is > 8):
	CClientDC dc(AfxGetMainWnd());
	m_HighColorMode = dc.GetDeviceCaps(BITSPIXEL) > 8;

	m_ClipboardDataImage = false;
	m_ClipboardDataEoGroups = true;
	m_ClipboardDataText = true;
	m_ModeInformationOverView = false;
	m_TrapModeAddGroups = true;
	m_NodalModeAddGroups = true;
	m_ClipboardFormatIdentifierForEoGroups = 0;
	m_CurrentMode = 0;
	m_EngagedLength = 0.;
	m_EngagedAngle = 0.;
	m_DimensionLength = 0.125;
	m_DimensionAngle = 45.;
	m_Units = kInches;
	m_ArchitecturalUnitsFractionPrecision = 16;
	m_SimplexStrokeFont = nullptr;
	m_DeviceHeightInMillimeters = 0.;
	m_DeviceHeightInPixels = 0.;
	m_DeviceWidthInMillimeters = 0.;
	m_DeviceWidthInPixels = 0.;

	m_AeSysMenuHandle = nullptr;
	m_ModeResourceIdentifier = 0;
	m_PrimaryMode = 0;

	m_TrapHighlightColor = 15;
	m_TrapHighlighted = true;

}
#define EO_REGISTRY_BUFFER_SIZE 1040
#define EO_REGISTRY_MAX_PROFILE_NAME 128
#define EO_REGISTRY_MAX_PATH 1024

OdString GetRegistryAcadLocation();
OdString GetRegistryAcadProfilesKey();
bool GetRegistryString(HKEY key, const wchar_t* subkey, const wchar_t* name, wchar_t* value, int size) noexcept;

static OdString FindConfigPath(const OdString& configType) {
	wchar_t searchPath[EO_REGISTRY_MAX_PATH];
	wchar_t expandedPath[EO_REGISTRY_MAX_PATH];

	OdString subkey = GetRegistryAcadProfilesKey();
	if (!subkey.isEmpty()) {
		subkey += L"\\General";
		// get the value for the ACAD entry in the registry
		if (::GetRegistryString(HKEY_CURRENT_USER, (LPCTSTR) subkey, (LPCTSTR) configType, searchPath, EO_REGISTRY_MAX_PATH)) {
			ExpandEnvironmentStrings(searchPath, expandedPath, EO_REGISTRY_MAX_PATH);
			return OdString(expandedPath);
		}
	}
	return OdString::kEmpty;
}

static OdString FindConfigFile(const OdString& configType, OdString file, OdDbSystemServices* pSs) {
	OdString searchPath = FindConfigPath(configType);
	if (!searchPath.isEmpty()) {
		file = searchPath + L"\\" + file;
		if (pSs->accessFile(file, Oda::kFileRead)) {
			return file;
		}
	}
	return OdString::kEmpty;
}

AeSysApp theApp; // The one and only AeSys object

const ODCOLORREF* AeSysApp::curPalette() const {
	return odcmAcadPalette(m_background);
}

OdGsDevicePtr AeSysApp::gsBitmapDevice(OdRxObject* view, OdDbBaseDatabase* database, OdUInt32 flags) {
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
	}
	catch (const OdError& Error) {
	}
	return OdGsDevicePtr();
}

bool AeSysApp::getPassword(const OdString& dwgName, bool /*isXref*/, OdPassword& password) {
	EoDlgPassword pwdDlg;
	pwdDlg.m_sFileName = (LPCWSTR) dwgName;
	if (pwdDlg.DoModal() == IDOK) {
		password = pwdDlg.m_password;
		if (dwgName.right(4) == L".dwg") {
			password.makeUpper();
		}
		return true;
	}
	return false;
}

OdDbPageControllerPtr AeSysApp::newPageController() {
	switch (m_pagingType & 0x0f) {
		case 1: // OdDb::kUnload - Simple unloading of objects for partially loaded database.
			return OdRxObjectImpl<ExUnloadController>::createObject();
		case 2: // OdDb::kPage
		case 3: // OdDb::kUnload - Unloading of objects for partially loaded database and paging of objects thru ExPageController.
			return OdRxObjectImpl<ExPageController>::createObject();
	}
	// Paging is not used.
	return (OdDbPageController*) 0;
}

int AeSysApp::setPagingType(int pagingType) noexcept {
	const int oldType = m_pagingType;
	m_pagingType = pagingType;
	return oldType;
}

bool AeSysApp::setUndoType(bool useTempFiles) noexcept {
	const bool oldType = m_bUseTempFiles;
	m_bUseTempFiles = useTempFiles;
	return oldType;
}

OdString AeSysApp::fileDialog(int flags, const OdString& prompt, const OdString& defExt, const OdString& fileName, const OdString& filter) {
	if (!supportFileSelectionViaDialog()) {
		return OdString(L"*unsupported*");
	}
	CFileDialog dlg(flags == OdEd::kGfpForOpen, defExt, fileName, OFN_HIDEREADONLY | OFN_EXPLORER | OFN_PATHMUSTEXIST, filter, ::AfxGetMainWnd());

	dlg.m_ofn.lpstrTitle = prompt;

	if (dlg.DoModal() == IDOK) {
		return OdString((LPCWSTR) dlg.GetPathName());
	}
	throw OdEdCancel();
}

OdRxClass* AeSysApp::databaseClass() const {
	return OdDbDatabaseDoc::desc();
}
OdString AeSysApp::findFile(const OdString& fileToFind, OdDbBaseDatabase* database, OdDbBaseHostAppServices::FindFileHint hint) {
	OdString FilePathAndName = ExHostAppServices::findFile(fileToFind, database, hint);

	if (FilePathAndName.isEmpty()) {
		const OdString ApplicationName(L"SOFTWARE\\Autodesk\\AutoCAD");

		OdString FileToFind(fileToFind);
		OdString Extension(fileToFind.right(4));
		Extension.makeLower();
		if (!Extension.iCompare(L".pc3")) {
			return ConfigurationFileFor(HKEY_CURRENT_USER, ApplicationName, L"PrinterConfigDir", fileToFind);
		}
		if (!Extension.iCompare(L".stb") || !Extension.iCompare(L".ctb")) {
			return ConfigurationFileFor(HKEY_CURRENT_USER, ApplicationName, L"PrinterStyleSheetDir", fileToFind);
		}
		if (!Extension.iCompare(L".pmp")) {
			return ConfigurationFileFor(HKEY_CURRENT_USER, ApplicationName, L"PrinterDescDir", fileToFind);
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
				if (FileToFind.left(5).iCompare(L"http:") == 0 || FileToFind.left(6).iCompare(L"https:") == 0) {
					// <tas="code section removed"</tas>
				}
				// fall through
			default:
				return FilePathAndName;
		}
		if (hint != kTextureMapFile && Extension != L".shx" && Extension != L".pat" && Extension != L".ttf" && Extension != L".ttc" && Extension != L".otf") {
			FileToFind += L".shx";
		} else if (hint == kTextureMapFile) {
			FileToFind.replace(L"/", L"\\");
			FileToFind.deleteChars(0, FileToFind.reverseFind(L'\\') + 1);
		}
		FilePathAndName = (hint != kTextureMapFile) ? FindConfigPath(L"ACAD") : FindConfigPath(L"AVEMAPS");

		OdDbSystemServices * SystemServices = odSystemServices();

		OdString Path;
		while (!FilePathAndName.isEmpty()) {
			const int PathDelimiter = FilePathAndName.find(L";");
			if (PathDelimiter == -1) {
				Path = FilePathAndName;
				FilePathAndName.empty();
			} else {
				Path = FilePathAndName.left(PathDelimiter);
				Path += L"\\" + FileToFind;
				if (SystemServices->accessFile(Path, Oda::kFileRead)) {
					return Path;
				}
				FilePathAndName = FilePathAndName.right(FilePathAndName.getLength() - PathDelimiter - 1);
			}
		}
		if (hint == kTextureMapFile) {
			return FilePathAndName;
		}
		if (FilePathAndName.isEmpty()) {
			OdString AcadLocation(GetRegistryAcadLocation());
			if (!AcadLocation.isEmpty()) {
				FilePathAndName = AcadLocation + L"\\Fonts\\" + FileToFind;
				if (!SystemServices->accessFile(FilePathAndName, Oda::kFileRead)) {
					FilePathAndName = AcadLocation + L"\\Support\\" + FileToFind;
					if (!SystemServices->accessFile(FilePathAndName, Oda::kFileRead)) {
						FilePathAndName.empty();
					}
				}
			}
		}
	}
	return FilePathAndName;
}

OdString AeSysApp::getFontMapFileName() const {
	OdString subkey;
	wchar_t fontMapFile[EO_REGISTRY_MAX_PATH];
	wchar_t expandedPath[EO_REGISTRY_MAX_PATH];

	subkey = GetRegistryAcadProfilesKey();
	if (!subkey.isEmpty()) {
		subkey += L"\\Editor Configuration";
		// get the value for the ACAD entry in the registry

		if (GetRegistryString(HKEY_CURRENT_USER, subkey, L"FontMappingFile", fontMapFile, EO_REGISTRY_MAX_PATH) == 0) {
			return L"";
		}
		ExpandEnvironmentStringsW(fontMapFile, expandedPath, EO_REGISTRY_MAX_PATH);
		return OdString(expandedPath);
	} else {
		return L"";
	}
}

CString AeSysApp::getApplicationPath() {
	wchar_t FileName[MAX_PATH];
	if (::GetModuleFileNameW(::GetModuleHandleW(nullptr), FileName, MAX_PATH)) {
		CString FilePath(FileName);
		const int Delimiter = FilePath.ReverseFind('\\');
		return (FilePath.Left(Delimiter));
	}
	return L"";
}

void AeSysApp::auditPrintReport(OdAuditInfo * auditInfo, const OdString & line, int printDest) const {
	if (m_pAuditDlg) {
		m_pAuditDlg->printReport((OdDbAuditInfo*) auditInfo);
	}
}
OdDbUndoControllerPtr AeSysApp::newUndoController() {
	if (undoType()) {
		ExFileUndoControllerPtr FileUndoController = OdRxObjectImpl<ExFileUndoController>::createObject();
		FileUndoController->setStorage(newUndoStream());
		return FileUndoController;
	}
	return OdRxObjectImpl<ExUndoController>::createObject();
}
OdStreamBufPtr AeSysApp::newUndoStream() {
	// OdMemFileStreamImpl = mix of memory and file streams
	return OdRxObjectImpl<OdMemFileStreamImpl<OdStreamBuf> >::createObject();
}

void AeSysApp::SetRecentCommand(const OdString& command) {

	if (!command.isEmpty() && command != m_RecentCommand) {
		m_RecentCommand = command;
		WriteProfileStringW(L"options", L"Recent Command", m_RecentCommand);
	}
}

CMenu* AeSysApp::CommandMenu(CMenu * *toolsSubMenu) {
	MENUITEMINFO MenuItemInfo;
	MenuItemInfo.cbSize = sizeof(MENUITEMINFO);
	MenuItemInfo.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_SUBMENU | MIIM_ID;

	CString MenuName;
	CMenu* ToolsSubMenu(NULL);
	CMenu* TopMenu = CMenu::FromHandle(theApp.GetAeSysMenu());

	for (int Item = TopMenu->GetMenuItemCount() - 1; Item >= 0; Item--) {
		MenuItemInfo.dwTypeData = NULL;
		TopMenu->GetMenuItemInfoW(Item, &MenuItemInfo, TRUE);

		const int SizeOfMenuName = ++MenuItemInfo.cch;
		MenuItemInfo.dwTypeData = MenuName.GetBuffer(SizeOfMenuName);
		TopMenu->GetMenuItemInfoW(Item, &MenuItemInfo, TRUE);
		MenuName.ReleaseBuffer();

		if (MenuItemInfo.fType == MFT_STRING && MenuName.Compare(L"&Tools") == 0) {
			ToolsSubMenu = CMenu::FromHandle(MenuItemInfo.hSubMenu);
			break;
		}
	}
	ASSERT(ToolsSubMenu != NULL);
	if (toolsSubMenu) {
		*toolsSubMenu = ToolsSubMenu;
	}

	CMenu* RegisteredCommandsSubMenu(NULL);

	for (int ToolsMenuItem = 0; ToolsMenuItem < ToolsSubMenu->GetMenuItemCount(); ToolsMenuItem++) {
		MenuItemInfo.dwTypeData = NULL;
		ToolsSubMenu->GetMenuItemInfoW(size_t(ToolsMenuItem), &MenuItemInfo, TRUE);

		const int SizeOfMenuName = ++MenuItemInfo.cch;
		MenuItemInfo.dwTypeData = MenuName.GetBuffer(SizeOfMenuName);
		ToolsSubMenu->GetMenuItemInfoW(size_t(ToolsMenuItem), &MenuItemInfo, TRUE);
		MenuName.ReleaseBuffer();

		if (MenuItemInfo.fType == MFT_STRING && MenuName.CompareNoCase(L"Registered &Commands") == 0) {
			RegisteredCommandsSubMenu = CMenu::FromHandle(MenuItemInfo.hSubMenu);
			break;
		}
	}
	ENSURE(RegisteredCommandsSubMenu != NULL);
	return (RegisteredCommandsSubMenu);
}

void AeSysApp::RefreshCommandMenu() {
	CMenu* ToolsSubMenu(NULL);
	CMenu* RegisteredCommandsSubMenu = CommandMenu(&ToolsSubMenu);

	for (int Item = RegisteredCommandsSubMenu->GetMenuItemCount() - 1; Item >= 0; Item--) {
		CMenu* SubMenu = RegisteredCommandsSubMenu->GetSubMenu(Item);
		if (SubMenu) {
			SubMenu->DestroyMenu();
		}
		RegisteredCommandsSubMenu->DeleteMenu(Item, MF_BYPOSITION);
	}
	ENSURE(RegisteredCommandsSubMenu->GetMenuItemCount() == 0);

	MENUITEMINFO MenuItemInfo;
	MenuItemInfo.cbSize = sizeof(MENUITEMINFO);
	MenuItemInfo.fMask = MIIM_DATA;

	auto CommandStack {::odedRegCmds()};
	bool bHasNoCommand {CommandStack->newIterator()->done()};

	const size_t ToolsMenuItem(8); // <tas="Until calculated position finished"</tas>
	ToolsSubMenu->EnableMenuItem(ToolsMenuItem, MF_BYPOSITION | (bHasNoCommand ? MF_GRAYED : MF_ENABLED));

	int CommandId = _APS_NEXT_COMMAND_VALUE + 100;
	if (!bHasNoCommand) {
		OdRxIteratorPtr CommandStackGroupIterator = CommandStack->newGroupIterator();
		while (!CommandStackGroupIterator->done()) {
			OdRxDictionaryPtr Group = CommandStackGroupIterator->object();
			CMenu GroupMenu;
			GroupMenu.CreateMenu();
			OdRxIteratorPtr GroupCommandIterator = Group->newIterator(OdRx::kDictSorted);
			OdString GroupName;
			while (!GroupCommandIterator->done()) {
				OdEdCommandPtr pCmd = GroupCommandIterator->object().get();
				if (GroupName.isEmpty()) {
					GroupName = pCmd->groupName();
				}
				OdString CommandName(pCmd->globalName());
				GroupMenu.AppendMenuW(MF_STRING, CommandId, CommandName);

				MenuItemInfo.dwItemData = (LPARAM) pCmd.get();
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

void AeSysApp::AddReactor(OdApplicationReactor* reactor) {
	if (m_aAppReactors.end() == std::find(m_aAppReactors.begin(), m_aAppReactors.end(), OdApplicationReactorPtr(reactor))) {
		m_aAppReactors.push_back(reactor);
	}
}

void AeSysApp::RemoveReactor(OdApplicationReactor* reactor) {
	m_aAppReactors.erase(std::remove(m_aAppReactors.begin(), m_aAppReactors.end(), OdApplicationReactorPtr(reactor)), m_aAppReactors.end());
}

OdDbDatabasePtr AeSysApp::openFile(LPCWSTR pathName) {
	CMainFrame* MainFrame = (CMainFrame*) GetMainWnd();
	OdDbDatabasePtr Database;

	OdInt16 nMode = getMtMode();
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
	} catch (const OdError & Error) {
		Database = 0;
		MainFrame->SetStatusPaneTextAt(0, L"");
		reportError(L"Loading Error...", Error);
	} catch (const UserBreak&) {
		Database = 0;
		MainFrame->SetStatusPaneTextAt(0, L"");
		SetStatusPaneTextAt(1, L"Operation was canceled by user.");
	} catch (std::bad_alloc&) {
		Database = 0;
		MainFrame->SetStatusPaneTextAt(0, L"");
		SetStatusPaneTextAt(1, L"Memory Allocation Error...");
	}
	if (m_pAuditDlg) { // Destroy audit dialog if recover failed
		delete m_pAuditDlg;
		m_pAuditDlg = nullptr;
	}
	return Database;
}

void AeSysApp::AddModeInformationToMessageList() {
	CString ResourceString = LoadStringResource(m_CurrentMode);
	int NextToken = 0;
	ResourceString = ResourceString.Tokenize(L"\n", NextToken);
	AddStringToMessageList(ResourceString);
}
void AeSysApp::AddStringToMessageList(LPCWSTR message) {
	CMainFrame* MainFrame = (CMainFrame*) (AfxGetMainWnd());

	MainFrame->GetOutputPane().AddStringToMessageList(message);
	if (!MainFrame->GetOutputPane().IsWindowVisible()) {
		MainFrame->SetStatusPaneTextAt(nStatusInfo, message);
	}
}
void AeSysApp::AddStringToMessageList(LPCWSTR message, LPCWSTR string) {
	CString FormatString;
	FormatString.Format(message, string);
	AddStringToMessageList(FormatString);
}
void AeSysApp::AddStringToMessageList(UINT stringResourceIdentifier) {
	CString ResourceString = LoadStringResource(stringResourceIdentifier);
	AddStringToMessageList(ResourceString);
}
void AeSysApp::AddStringToMessageList(UINT stringResourceIdentifier, LPCWSTR string) {
	CString FormatSpecification = LoadStringResource(stringResourceIdentifier);
	AddStringToMessageList(FormatSpecification, string);
}
void AeSysApp::AddStringToReportList(LPCWSTR message) {
	CMainFrame* MainFrame = (CMainFrame*) (AfxGetMainWnd());

	MainFrame->GetOutputPane().AddStringToReportsList(message);
	if (!MainFrame->GetOutputPane().IsWindowVisible()) {
		MainFrame->SetStatusPaneTextAt(nStatusInfo, message);
	}
}
int	AeSysApp::ArchitecturalUnitsFractionPrecision() const  noexcept {
	return (m_ArchitecturalUnitsFractionPrecision);
}
// Modifies the base accelerator table by defining the mode specific keys.
void AeSysApp::BuildModeSpecificAcceleratorTable() {
	CMainFrame* MainFrame = (CMainFrame*) AfxGetMainWnd();

	HACCEL AcceleratorTableHandle = MainFrame->m_hAccelTable;
	::DestroyAcceleratorTable(AcceleratorTableHandle);

	HACCEL ModeAcceleratorTableHandle = ::LoadAccelerators(m_hInstance, MAKEINTRESOURCE(m_ModeResourceIdentifier));
	int ModeAcceleratorTableEntries = ::CopyAcceleratorTableW(ModeAcceleratorTableHandle, nullptr, 0);

	AcceleratorTableHandle = ::LoadAccelerators(m_hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));
	const int AcceleratorTableEntries = ::CopyAcceleratorTableW(AcceleratorTableHandle, nullptr, 0);

	LPACCEL ModifiedAcceleratorTable = new ACCEL[AcceleratorTableEntries + ModeAcceleratorTableEntries];

	::CopyAcceleratorTableW(ModeAcceleratorTableHandle, ModifiedAcceleratorTable, ModeAcceleratorTableEntries);
	::CopyAcceleratorTableW(AcceleratorTableHandle, &ModifiedAcceleratorTable[ModeAcceleratorTableEntries], AcceleratorTableEntries);

	MainFrame->m_hAccelTable = ::CreateAcceleratorTable(ModifiedAcceleratorTable, AcceleratorTableEntries + ModeAcceleratorTableEntries);

	delete[] ModifiedAcceleratorTable;
}
UINT AeSysApp::ClipboardFormatIdentifierForEoGroups()  noexcept {
	return (m_ClipboardFormatIdentifierForEoGroups);
}
OdString AeSysApp::ConfigurationFileFor(HKEY key, const OdString & applicationName, const OdString & configType, OdString file) {
	OdString ConfigPath = FindConfigPath(configType);
	if (!ConfigPath.isEmpty()) {
		file = ConfigPath + L"\\" + file;
		if (odSystemServices()->accessFile(file, Oda::kFileRead)) {
			return file;
		}
	}
	return OdString::kEmpty;
}

int AeSysApp::CurrentMode() const  noexcept {
	return m_CurrentMode;
}
double AeSysApp::DeviceHeightInMillimeters() const  noexcept {
	return m_DeviceHeightInMillimeters;
}
double AeSysApp::DeviceHeightInPixels() const  noexcept {
	return m_DeviceHeightInPixels;
}
double AeSysApp::DeviceWidthInMillimeters() const  noexcept {
	return m_DeviceWidthInMillimeters;
}
double AeSysApp::DeviceWidthInPixels() const  noexcept {
	return m_DeviceWidthInPixels;
}
double AeSysApp::DimensionAngle() const  noexcept {
	return (m_DimensionAngle);
}
double AeSysApp::DimensionLength() const  noexcept {
	return (m_DimensionLength);
}
void AeSysApp::EditColorPalette() {
	CHOOSECOLOR 	cc;
	::ZeroMemory(&cc, sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);

	cc.rgbResult = ColorPalette[pstate.ColorIndex()];
	cc.lpCustColors = ColorPalette;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_SOLIDCOLOR;
	::ChooseColor(&cc);

	cc.rgbResult = GreyPalette[pstate.ColorIndex()];
	cc.lpCustColors = GreyPalette;
	::ChooseColor(&cc);

	::MessageBoxW(0, L"The background color is no longer associated with the pen Color Palette.", L"Deprecation Notice", MB_OK | MB_ICONINFORMATION);

	AeSysDoc::GetDoc()->UpdateAllViews(nullptr);
}
double AeSysApp::EngagedAngle() const noexcept {
	return (m_EngagedAngle);
}
double AeSysApp::EngagedLength() const noexcept {
	return (m_EngagedLength);
}

CString AeSysApp::BrowseWithPreview(HWND parentWindow, LPCWSTR filter, bool multiple) {
	CString FileName;
	const DWORD Flags(OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST);
	CString LibraryFileName(L"FileDlgExt" TD_DLL_VERSION_SUFFIX_STR L".dll");
	HINSTANCE hinstLib = LoadLibraryW(LibraryFileName);
	if (NULL != hinstLib) {
		ODA_OPEN_DLGPROC fpDlgProc = (ODA_OPEN_DLGPROC) GetProcAddress(hinstLib, "CreateOpenWithPreviewDlg");
		if (NULL != fpDlgProc) {
			EoPreviewDib statDib;
			OpenWithPreviewDlg* OpenWithPreviewDialog;
			(fpDlgProc) (&statDib, parentWindow, NULL, filter, Flags, &OpenWithPreviewDialog);
			if (IDOK == OpenWithPreviewDialog->ShowModal()) {
				long nSize = MAX_PATH;
				OpenWithPreviewDialog->GetFullFileName(FileName.GetBuffer(nSize), nSize);
				FileName.ReleaseBuffer();
			}
			OpenWithPreviewDialog->ReleaseDlg();
		}
		FreeLibrary(hinstLib);
	} else {
		CString Filter(filter);
		Filter.Replace('|', '\0');

		OPENFILENAME of;
		::ZeroMemory(&of, sizeof(OPENFILENAME));
		of.lStructSize = sizeof(OPENFILENAME);
		of.hwndOwner = parentWindow;
		of.lpstrFilter = Filter;
		of.nFilterIndex = 1;
		of.lpstrFile = FileName.GetBuffer(MAX_PATH);
		of.nMaxFile = MAX_PATH;
		of.lpstrInitialDir = NULL;
		of.Flags = Flags;

		GetOpenFileNameW(&of);
		FileName.ReleaseBuffer();
	}
	return FileName;
}
int AeSysApp::ExitInstance() {
	SetRegistryBase(L"ODA View");
	theApp.WriteInt(L"Discard Back Faces", m_bDiscardBackFaces);
	theApp.WriteInt(L"Enable Double Buffer", m_bEnableDoubleBuffer);
	theApp.WriteInt(L"Enable Blocks Cache", m_bBlocksCache);
	theApp.WriteInt(L"Gs Device Multithread", m_bGsDevMultithread);
	theApp.WriteInt(L"Mt Regen Threads Count", m_nMtRegenThreads);
	theApp.WriteInt(L"Print/Preview via bitmap device", m_bEnablePrintPreviewViaBitmap);
	theApp.WriteInt(L"UseGsModel", m_bUseGsModel);
	theApp.WriteInt(L"Enable Software HLR", m_bEnableHLR);
	theApp.WriteInt(L"Contextual Colors", m_bContextColors);
	theApp.WriteInt(L"TTF PolyDraw", m_bTTFPolyDraw);
	theApp.WriteInt(L"TTF TextOut", m_bTTFTextOut);
	theApp.WriteInt(L"Save round trip information", m_bSaveRoundTrip);
	theApp.WriteInt(L"Save Preview", m_bSavePreview);
	theApp.WriteInt(L"Background colour", m_background);
	theApp.WriteInt(L"Save DWG with password", m_bSaveWithPassword);
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
CString AeSysApp::FormatAngle(double angle, int width, int precision) const {
	CString FormatSpecification;
	FormatSpecification.Format(L"%%%i.%if°", width, precision);
	CString AngleAsString;
	AngleAsString.Format(FormatSpecification, EoToDegree(angle));
	return (AngleAsString);
}
CString AeSysApp::FormatLength(double length, Units units, int width, int precision) const {
	wchar_t LengthAsString[32];
	FormatLength_s(LengthAsString, 32, units, length, width, precision);
	return CString(LengthAsString).TrimLeft();
}
void AeSysApp::FormatLength_s(LPWSTR lengthAsString, const int bufSize, Units units, const double length, const int width, const int precision) const {
	wchar_t szBuf[16];

	double ScaledLength = length * AeSysView::GetActiveView()->WorldScale();

	if (units == kArchitectural || units == kArchitecturalS) {
		wcscpy_s(lengthAsString, bufSize, (length >= 0.) ? L" " : L"-");
		ScaledLength = fabs(ScaledLength);

		int Feet = int(ScaledLength / 12.);
		int Inches = abs(int(fmod(ScaledLength, 12.)));

		const int FractionPrecision = ArchitecturalUnitsFractionPrecision();
		int Numerator = int(fabs(fmod(ScaledLength, 1.)) * (double) (FractionPrecision) +.5);	// Numerator of fractional component of inches

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
			const int	iGrtComDivisor = GreatestCommonDivisor(Numerator, FractionPrecision);
			Numerator /= iGrtComDivisor;
			const int Denominator = FractionPrecision / iGrtComDivisor; // Add fractional component of inches
			_itow_s(Numerator, szBuf, 16, 10);
			wcscat_s(lengthAsString, bufSize, szBuf);
			wcscat_s(lengthAsString, bufSize, L"/");
			_itow_s(Denominator, szBuf, 16, 10);
			wcscat_s(lengthAsString, bufSize, szBuf);
			if (units == kArchitecturalS) wcscat_s(lengthAsString, bufSize, L";");
		}
		wcscat_s(lengthAsString, bufSize, L"\"");
	} else if (units == kEngineering) {
		wcscpy_s(lengthAsString, bufSize, (length >= 0.) ? L" " : L"-");
		ScaledLength = fabs(ScaledLength);

		const int Precision = (ScaledLength >= 1.) ? precision - int(log10(ScaledLength)) - 1 : precision;

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
	} else {
		CString FormatSpecification;
		FormatSpecification.Format(L"%%%i.%if", width, precision);

		switch (units) {
			case kFeet:
				FormatSpecification.Append(L"'");
				swprintf_s(lengthAsString, bufSize, FormatSpecification, ScaledLength / 12.);
				break;
			case kInches:
				FormatSpecification.Append(L"\"");
				swprintf_s(lengthAsString, bufSize, FormatSpecification, ScaledLength);
				break;
			case kMeters:
				FormatSpecification.Append(L"m");
				swprintf_s(lengthAsString, bufSize, FormatSpecification, ScaledLength * .0254);
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
				swprintf_s(lengthAsString, bufSize, FormatSpecification, ScaledLength * .254);
				break;
			case kKilometers:
				FormatSpecification.Append(L"km");
				swprintf_s(lengthAsString, bufSize, FormatSpecification, ScaledLength * .0000254);
				break;
			default:
				lengthAsString[0] = '\0';
				break;
		}
	}
}

OdGePoint3d AeSysApp::GetCursorPosition() {
	AeSysView* ActiveView = AeSysView::GetActiveView();
	return (ActiveView) ? ActiveView->GetCursorPosition() : OdGePoint3d::kOrigin;
}

EoDb::FileTypes AeSysApp::GetFileType(const OdString & file) {
	EoDb::FileTypes Type(EoDb::kUnknown);
	OdString Extension = file.right(3);

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

COLORREF AeSysApp::GetHotColor(OdInt16 colorIndex) noexcept {
	return (ColorPalette[colorIndex]);
}
HINSTANCE AeSysApp::GetInstance() noexcept {
	return (m_hInstance);
}

HWND AeSysApp::GetSafeHwnd() {
	return (AfxGetMainWnd()->GetSafeHwnd());
}
HMENU AeSysApp::GetAeSysMenu() noexcept {
	return (m_AeSysMenuHandle);
}
HMENU AeSysApp::GetAeSysSubMenu(int position) noexcept {
	return (::GetSubMenu(m_AeSysMenuHandle, position));
}
AeSysApp::Units AeSysApp::GetUnits() noexcept {
	return (m_Units);
}
int AeSysApp::GreatestCommonDivisor(const int number1, const int number2) const noexcept {
	int ReturnValue = abs(number1);
	int Divisor = abs(number2);
	while (Divisor != 0) {
		const int Remainder = ReturnValue % Divisor;
		ReturnValue = Divisor;
		Divisor = Remainder;
	}
	return (ReturnValue);
}
bool AeSysApp::HighColorMode() const noexcept {
	return m_HighColorMode;
}
OdGePoint3d AeSysApp::HomePointGet(int i) noexcept {
	if (i >= 0 && i < 9)
		return (m_HomePoints[i]);

	return (OdGePoint3d::kOrigin);
}
void AeSysApp::HomePointSave(int i, const OdGePoint3d & point) noexcept {
	if (i >= 0 && i < 9)
		m_HomePoints[i] = point;
}
void AeSysApp::InitGbls(CDC * deviceContext) {
	pstate.SetHatchInteriorStyle(EoDbHatch::kHatch);
	pstate.SetHatchInteriorStyleIndex(1);

	EoDbHatch::sm_PatternScaleX = .1;
	EoDbHatch::sm_PatternScaleY = .1;
	EoDbHatch::sm_PatternAngle = 0.;

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
BOOL AeSysApp::InitializeTeigha() {
	try {
		::odInitialize(this);

		EoLoadApps::rxInit();

		OdApplicationReactor::rxInit();
		OdApplicationDocument::rxInit();

		::odrxDynamicLinker()->loadModule(OdGripPointsModuleName); // GripPoints module
		::odrxDynamicLinker()->loadModule(OdDbCommandsModuleName); // DbCommands module (ERASE,EXPLODE,PURGE, etc.)
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
	} catch (const OdError & Error) {
		theApp.reportError(L"odInitialize error", Error);
		return FALSE;
	} catch (...) {
		::MessageBoxW(0, L"odInitialize error", L"Teigha", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	return TRUE;
}

BOOL AeSysApp::InitInstance() {

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
	LoadStdProfileSettings(8U);  // Load the list of most recently used (MRU) files and last preview state.
	SetRegistryBase(L"Options");
	m_Options.Load();

	SetRegistryBase(L"ODA View");

	m_bDiscardBackFaces = theApp.GetInt(L"Discard Back Faces", 1);
	m_bEnableDoubleBuffer = theApp.GetInt(L"Enable Double Buffer", 1); // <tas="TRUE unless debugging"</tas>
	m_bBlocksCache = theApp.GetInt(L"Enable Blocks Cache", 0); // 1
	m_bGsDevMultithread = theApp.GetInt(L"Gs Device Multithread", 0);
	m_nMtRegenThreads = theApp.GetInt(L"Mt Regen Threads Count", 4);
	m_bEnablePrintPreviewViaBitmap = theApp.GetInt(L"Print/Preview via bitmap device", 1);
	m_bUseGsModel = theApp.GetInt(L"UseGsModel", TRUE);
	m_bEnableHLR = theApp.GetInt(L"Enable Software HLR", 0);
	m_bContextColors = theApp.GetInt(L"Contextual Colors", 1);
	m_bTTFPolyDraw = theApp.GetInt(L"TTF PolyDraw", 0);
	m_bTTFTextOut = theApp.GetInt(L"TTF TextOut", 0);

	m_bTTFCache = theApp.GetInt(L"TTF Cache", 0);
	m_bDynamicSubEntHlt = GetInt(L"Dynamic Subentities Highlight", 0);
	m_bGDIGradientsAsBitmap = GetInt(L"GDI Gradients as Bitmaps", 1);
	m_bGDIGradientsAsPolys = GetInt(L"GDI Gradients as Polys", 0);
	m_nGDIGradientsAsPolysThreshold = (BYTE)GetInt(L"GDI Gradients as Polys Threshold", 10);

	m_bDisableAutoRegen = theApp.GetInt(L"Disable Auto-Regen", 0);

//	m_displayFields = GetProfileInt(_T("options"), _T("Field display format"), 0);

	m_bSaveRoundTrip = theApp.GetInt(L"Save round trip information", 1);
	m_bSavePreview = theApp.GetInt(L"Save Preview", 0);
	m_background = theApp.GetInt(L"Background colour", ViewBackgroundColor);
	m_bSaveWithPassword = theApp.GetInt(L"Save DWG with password", 0);
	m_sVectorizerPath = theApp.GetString(L"recent GS", OdWinDirectXModuleName);
	m_RecentCommand = theApp.GetString(L"Recent Command", L"");
	int nFillTtf = theApp.GetInt(L"Fill TTF text", 1);
	setTEXTFILL(theApp.GetInt(L"Fill TTF text", 1) != 0);
	
	SetRegistryBase(L"MFC Auto");

	lex::Init();

	// Initialize all Managers for usage. They are automatically constructed if not yet present
	InitContextMenuManager();
	InitKeyboardManager(); // Manages shortcut key tables for the main frame window and child frame windows.
	InitTooltipManager();

	CMFCToolTipInfo params;
	params.m_bVislManagerTheme = TRUE;
	GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS(CMFCToolTipCtrl), &params);

	EnableUserTools(ID_TOOLS_ENTRY, ID_USER_TOOL1, ID_USER_TOOL10, RUNTIME_CLASS(CUserTool), IDR_MENU_ARGS, IDR_MENU_DIRS);

	CMFCButton::EnableWindowsTheming();

	CWinAppEx::InitInstance();

	if (InitializeTeigha() == FALSE) {
		return FALSE;
	}
	// Register the application's document templates.  Document templates serve as the connection between documents, frame windows and views.
	CMultiDocTemplate* DocTemplate;
	DocTemplate = new CMultiDocTemplate(IDR_AESYSTYPE, RUNTIME_CLASS(AeSysDoc), RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(AeSysView));
	AddDocTemplate(DocTemplate);

	DocTemplate = new CMultiDocTemplate(IDR_TRACINGTYPE, RUNTIME_CLASS(AeSysDoc), RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(AeSysView));
	AddDocTemplate(DocTemplate);

	// Create main MDI Frame window
	auto MainFrame {new CMainFrame};

	if (!MainFrame || !MainFrame->LoadFrame(IDR_MAINFRAME)) {
		delete MainFrame;
		return FALSE;
	}
	m_pMainWnd = MainFrame;

	MainFrame->DragAcceptFiles();

	auto DeviceContext {MainFrame->GetDC()};

	m_DeviceWidthInPixels = static_cast<double>(DeviceContext->GetDeviceCaps(HORZRES));
	m_DeviceHeightInPixels = static_cast<double>(DeviceContext->GetDeviceCaps(VERTRES));
	m_DeviceWidthInMillimeters = static_cast<double>(DeviceContext->GetDeviceCaps(HORZSIZE));
	m_DeviceHeightInMillimeters = static_cast<double>(DeviceContext->GetDeviceCaps(VERTSIZE));

	InitGbls(DeviceContext);
	MainFrame->ReleaseDC(DeviceContext);

	m_AeSysMenuHandle = ::LoadMenu(m_hInstance, MAKEINTRESOURCE(IDR_AESYSTYPE));

	RefreshCommandMenu();

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo CommandLineInfo;
	ParseCommandLine(CommandLineInfo);

	if (CommandLineInfo.m_nShellCommand == CCommandLineInfo::FileNew) { CommandLineInfo.m_nShellCommand = CCommandLineInfo::FileNothing; }
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
bool AeSysApp::IsClipboardDataGroups() noexcept {
	return m_ClipboardDataEoGroups;
}
bool AeSysApp::IsClipboardDataImage() noexcept {
	return m_ClipboardDataImage;
}
bool AeSysApp::IsClipboardDataText() noexcept {
	return m_ClipboardDataText;
}
bool AeSysApp::IsTrapHighlighted() noexcept {
	return m_TrapHighlighted;
}
void AeSysApp::LoadColorPalletFromFile(const CString & fileName) {
	CStdioFile fl;

	if (fl.Open(fileName, CFile::modeRead | CFile::typeText)) {
		wchar_t Line[128];
		wmemset(Line, 0, 128);
		LPWSTR pId = nullptr;
		LPWSTR pRed = nullptr;
		LPWSTR pGreen = nullptr;
		LPWSTR pBlue = nullptr;

		while (fl.ReadString(Line, sizeof(Line) / sizeof(wchar_t) - 1) && _tcsnicmp(Line, L"<Colors>", 8) != 0);

		while (fl.ReadString(Line, sizeof(Line) / sizeof(wchar_t) - 1) && *Line != '<') {
			LPWSTR NextToken = nullptr;
			pId = wcstok_s(Line, L"=", &NextToken);
			pRed = wcstok_s(nullptr, L",", &NextToken);
			pGreen = wcstok_s(nullptr, L",", &NextToken);
			pBlue = wcstok_s(nullptr, L",", &NextToken);
			ColorPalette[_wtoi(pId)] = RGB(_wtoi(pRed), _wtoi(pGreen), _wtoi(pBlue));
			pRed = wcstok_s(nullptr, L",", &NextToken);
			pGreen = wcstok_s(nullptr, L",", &NextToken);
			pBlue = wcstok_s(nullptr, L"\n", &NextToken);
			GreyPalette[_wtoi(pId)] = RGB(_wtoi(pRed), _wtoi(pGreen), _wtoi(pBlue));
		}
	}
}

void AeSysApp::LoadModeResources(int mode) {
	BuildModeSpecificAcceleratorTable();

	m_CurrentMode = mode;
	AddModeInformationToMessageList();

	AeSysView* ActiveView = AeSysView::GetActiveView();
	if (ActiveView != 0) {
		ActiveView->SetModeCursor(m_CurrentMode);
		ActiveView->ModeLineDisplay();
		ActiveView->RubberBandingDisable();
	}
}
void AeSysApp::LoadPenWidthsFromFile(const CString & fileName) {
	CStdioFile fl;

	if (fl.Open(fileName, CFile::modeRead | CFile::typeText)) {
		wchar_t PenWidths[64];

		while (fl.ReadString(PenWidths, sizeof(PenWidths) / sizeof(wchar_t) - 1)) {
			LPWSTR NextToken = nullptr;

			int PenIndex = _wtoi(wcstok_s(PenWidths, L"=", &NextToken));
			const double Width = _wtof(wcstok_s(nullptr, L",\n", &NextToken));

			if (PenIndex >= 0 && PenIndex < sizeof(dPWids) / sizeof(dPWids[0]))
				dPWids[PenIndex] = Width;
		}
	}
}
/// <remarks> Font stroke table encoded as follows:
/// b0 - b11  relative y displacement
/// b12 - b23 relative x displacement
/// b24 - b31 operation code (5 for line, else move)
/// The font is exactly 16384 bytes and defines a 96 character font set with a maximum of 4096 stokes
/// </remarks>
void AeSysApp::LoadSimplexStrokeFont(const CString & pathName) {
	HANDLE OpenHandle = CreateFileW(pathName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (OpenHandle != INVALID_HANDLE_VALUE) {
		if (SetFilePointer(OpenHandle, 0, 0, FILE_BEGIN) != (DWORD) -1) {
			if (!m_SimplexStrokeFont) {
				m_SimplexStrokeFont = new char[16384];
			}
			DWORD NumberOfBytesRead;
			if (!ReadFile(OpenHandle, m_SimplexStrokeFont, 16384U, &NumberOfBytesRead, 0)) {
				ReleaseSimplexStrokeFont();
			}
		}
		CloseHandle(OpenHandle);
	} else {
		HRSRC ResourceHandle = FindResourceW(NULL, MAKEINTRESOURCE(IDR_PEGSTROKEFONT), L"STROKEFONT");
		if (ResourceHandle != NULL) {
			const int ResourceSize = SizeofResource(NULL, ResourceHandle);
			m_SimplexStrokeFont = new char[ResourceSize];
			LPVOID Resource = LockResource(LoadResource(NULL, ResourceHandle));
			memcpy_s(m_SimplexStrokeFont, ResourceSize, Resource, ResourceSize);
		}
	}
}
CString AeSysApp::LoadStringResource(UINT resourceIdentifier) const {
	CString String;
	VERIFY(String.LoadStringW(resourceIdentifier) == TRUE);
	return String;
}
bool AeSysApp::ModeInformationOverView() const noexcept {
	return m_ModeInformationOverView;
}
void AeSysApp::OnAppAbout() {
	EoDlgAbout dlg;
	dlg.DoModal();
}
void AeSysApp::OnEditCfGroups() noexcept {
	m_ClipboardDataEoGroups = !m_ClipboardDataEoGroups;
}
void AeSysApp::OnEditCfImage() noexcept {
	m_ClipboardDataImage = !m_ClipboardDataImage;
}
void AeSysApp::OnEditCfText() noexcept {
	m_ClipboardDataText = !m_ClipboardDataText;
}

void AeSysApp::OnFileOpen() {
	const DWORD Flags(OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);
	CString Filter = LoadStringResource(IDS_OPENFILE_FILTER);
	CFileDialog FileDialog(TRUE, nullptr, nullptr, Flags, Filter);

	CString FileName;
	FileDialog.m_ofn.lpstrFile = FileName.GetBuffer(MAX_PATH);

	CString Title = LoadStringResource(AFX_IDS_OPENFILE);
	FileDialog.m_ofn.lpstrTitle = Title;

	const int Result = FileDialog.DoModal();
	FileName.ReleaseBuffer();

	if (Result == IDOK) {
		OpenDocumentFile(FileName);
	}
}

void AeSysApp::OnFilePlotstylemanager() {
	OPENFILENAME of;
	::ZeroMemory(&of, sizeof(OPENFILENAME));
	of.lStructSize = sizeof(OPENFILENAME);
	of.hwndOwner = 0;
	of.hInstance = theApp.GetInstance();
	of.lpstrFilter = L"Plot Style Files\0*.ctb;*.stb\0All Files\0*.*\0\0";
	of.lpstrFile = new wchar_t[MAX_PATH];
	*of.lpstrFile = 0;
	of.nMaxFile = MAX_PATH;
	of.lpstrTitle = L"Select Plot Style File";
	of.Flags = OFN_FILEMUSTEXIST;
	of.lpstrDefExt = L"stb";

	if (GetOpenFileNameW(&of)) {
		OdString FileName(of.lpstrFile);
		delete of.lpstrFile;

		OdDbSystemServices* SystemServices = odSystemServices();
		try {
			if (SystemServices->accessFile(FileName, Oda::kFileRead)) {
				OdStreamBufPtr StreamBuffer(SystemServices->createFile(FileName));
				OdPsPlotStyleTablePtr pPlotStyleTable;
				if (StreamBuffer.get()) {
					OdPsPlotStyleServicesPtr PlotStyleServices = odrxDynamicLinker()->loadApp(ODPS_PLOTSTYLE_SERVICES_APPNAME);
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
void AeSysApp::OnHelpContents() {
	::HtmlHelpW(AfxGetMainWnd()->GetSafeHwnd(), L"..\\AeSys\\hlp\\AeSys.chm", HH_DISPLAY_TOPIC, NULL);
}
void AeSysApp::OnModeAnnotate() {
	m_ModeResourceIdentifier = IDR_ANNOTATE_MODE;
	m_PrimaryMode = ID_MODE_ANNOTATE;
	LoadModeResources(ID_MODE_ANNOTATE);
}
void AeSysApp::OnModeCut() {
	m_ModeResourceIdentifier = IDR_CUT_MODE;
	m_PrimaryMode = ID_MODE_CUT;
	LoadModeResources(ID_MODE_CUT);
}
void AeSysApp::OnModeDimension() {
	m_ModeResourceIdentifier = IDR_DIMENSION_MODE;
	m_PrimaryMode = ID_MODE_DIMENSION;
	LoadModeResources(ID_MODE_DIMENSION);
}
void AeSysApp::OnModeDraw() {
	m_ModeResourceIdentifier = IDR_DRAW_MODE;
	m_PrimaryMode = ID_MODE_DRAW;
	LoadModeResources(ID_MODE_DRAW);
}
void AeSysApp::OnModeDraw2() {
	m_ModeResourceIdentifier = IDR_DRAW2_MODE;
	m_PrimaryMode = ID_MODE_DRAW2;
	LoadModeResources(ID_MODE_DRAW2);
}
void AeSysApp::OnModeEdit() {
	m_ModeResourceIdentifier = IDR_EDIT_MODE;
	LoadModeResources(ID_MODE_EDIT);
}
void AeSysApp::OnModeFixup() {
	m_ModeResourceIdentifier = IDR_FIXUP_MODE;
	LoadModeResources(ID_MODE_FIXUP);
}
void AeSysApp::OnModeLetter() {
	EoDlgModeLetter Dialog;
	Dialog.DoModal();
}
void AeSysApp::OnModeLPD() {
	m_ModeResourceIdentifier = IDR_LPD_MODE;
	LoadModeResources(ID_MODE_LPD);
}
void AeSysApp::OnModeNodal() {
	m_ModeResourceIdentifier = IDR_NODAL_MODE;
	LoadModeResources(ID_MODE_NODAL);
}
void AeSysApp::OnModePipe() {
	m_ModeResourceIdentifier = IDR_PIPE_MODE;
	LoadModeResources(ID_MODE_PIPE);
}
void AeSysApp::OnModePower() {
	m_ModeResourceIdentifier = IDR_POWER_MODE;
	LoadModeResources(ID_MODE_POWER);
}
void AeSysApp::OnModeRevise() {
	EoDlgModeRevise Dialog;
	Dialog.DoModal();
}
void AeSysApp::OnModeTrap() {
	if (m_TrapModeAddGroups) {
		m_ModeResourceIdentifier = IDR_TRAP_MODE;
		LoadModeResources(ID_MODE_TRAP);
	} else {
		m_ModeResourceIdentifier = IDR_TRAPR_MODE;
		LoadModeResources(ID_MODE_TRAPR);
	}
}
void AeSysApp::OnToolsLoadapplications() {
	EoLoadApps LoadAppsDialog;
	LoadAppsDialog.DoModal();
}
void AeSysApp::OnTrapCommandsAddGroups() {
	m_TrapModeAddGroups = !m_TrapModeAddGroups;
	m_CurrentMode = m_TrapModeAddGroups ? ID_MODE_TRAP : ID_MODE_TRAPR;

	OnModeTrap();
}
void AeSysApp::OnTrapCommandsHighlight() noexcept {
	m_TrapHighlighted = !m_TrapHighlighted;
	//LPARAM Hint = m_TrapHighlighted ? kGroupsSafeTrap : kGroupsSafe;
	//UpdateGroupsInAllViews(Hint, &m_TrappedGroupList);
}
void AeSysApp::OnUpdateEditCfGroups(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_ClipboardDataEoGroups);
}
void AeSysApp::OnUpdateEditCfImage(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_ClipboardDataImage);
}
void AeSysApp::OnUpdateEditCfText(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_ClipboardDataText);
}
void AeSysApp::OnUpdateModeAnnotate(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_ANNOTATE);
}
void AeSysApp::OnUpdateModeCut(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_CUT);
}
void AeSysApp::OnUpdateModeDimension(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_DIMENSION);
}
void AeSysApp::OnUpdateModeDraw(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_DRAW);
}
void AeSysApp::OnUpdateModeDraw2(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_DRAW2);
}
void AeSysApp::OnUpdateModeEdit(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_EDIT);
}
void AeSysApp::OnUpdateModeFixup(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_FIXUP);
}
void AeSysApp::OnUpdateModeLpd(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_LPD);
}
void AeSysApp::OnUpdateModeNodal(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_NODAL);
}
void AeSysApp::OnUpdateModePipe(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_PIPE);
}
void AeSysApp::OnUpdateModePower(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_POWER);
}
void AeSysApp::OnUpdateModeTrap(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_CurrentMode == ID_MODE_TRAP);
}
void AeSysApp::OnUpdateTrapcommandsAddgroups(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_TrapModeAddGroups);
}
void AeSysApp::OnUpdateTrapcommandsHighlight(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_TrapHighlighted);
}
void AeSysApp::OnUpdateViewModeinformation(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_ModeInformationOverView);
}


COLORREF AppGetTextCol() noexcept {
	return (~(ViewBackgroundColor | 0xff000000));
}
void AeSysApp::OnViewModeInformation() {
	m_ModeInformationOverView = !m_ModeInformationOverView;
	AeSysDoc::GetDoc()->UpdateAllViews(nullptr);
}
double AeSysApp::ParseLength(LPWSTR aszLen) {
	LPWSTR	szEndPtr;

	double dRetVal = _tcstod(aszLen, &szEndPtr);

	switch (toupper((int) szEndPtr[0])) {
		case '\'':												// Feet and maybe inches
			dRetVal *= 12.; 										// Reduce to inches
			dRetVal += _tcstod(&szEndPtr[1], &szEndPtr); 			// Begin scan for inches at character following foot delimeter
			break;

		case 'M':
			if (toupper((int) szEndPtr[1]) == 'M')
				dRetVal *= .03937007874015748;
			else
				dRetVal *= 39.37007874015748;
			break;

		case 'C':
			dRetVal *= .3937007874015748;
			break;

		case 'D':
			dRetVal *= 3.937007874015748;
			break;

		case 'K':
			dRetVal *= 39370.07874015748;

	}
	return (dRetVal / AeSysView::GetActiveView()->WorldScale());
}
double AeSysApp::ParseLength(Units units, LPWSTR aszLen) {
	try {
		int iTokId = 0;
		long lDef;
		int iTyp;
		double dVal[32];

		lex::Parse(aszLen);
		lex::EvalTokenStream(&iTokId, &lDef, &iTyp, (void*) dVal);

		if (iTyp == lex::TOK_LENGTH_OPERAND)
			return (dVal[0]);
		else {
			lex::ConvertValTyp(iTyp, lex::TOK_REAL, &lDef, dVal);

			switch (units) {
				case kArchitectural:
				case kEngineering:
				case kFeet:
					dVal[0] *= 12.;
					break;

				case kMeters:
					dVal[0] *= 39.37007874015748;
					break;

				case kMillimeters:
					dVal[0] *= .03937007874015748;
					break;

				case kCentimeters:
					dVal[0] *= .3937007874015748;
					break;

				case kDecimeters:
					dVal[0] *= 3.937007874015748;
					break;

				case kKilometers:
					dVal[0] *= 39370.07874015748;
			}
			dVal[0] /= AeSysView::GetActiveView()->WorldScale();
		}
		return (dVal[0]);
	} catch (const LPWSTR szMessage) {
		::MessageBoxW(0, szMessage, 0, MB_ICONWARNING | MB_OK);
		return (0.);
	}
}
double AeSysApp::PenWidthsGet(OdInt16 colorIndex) noexcept {
	return (dPWids[colorIndex]);
}
/// <remarks> Processing occurs immediately before the framework loads the application state from the registry. </remarks>
void AeSysApp::PreLoadState() {
	GetContextMenuManager()->AddMenu(L"My menu", IDR_CONTEXT_MENU);

	// <tas="add other context menus here"/>
}
int AeSysApp::PrimaryMode() const noexcept {
	return m_PrimaryMode;
}

bool GetRegistryString(HKEY key, const wchar_t* subkey, const wchar_t* name, wchar_t* value, int size) noexcept {
	bool rv = false;
	HKEY hKey;
	if (RegOpenKeyExW(key, subkey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		DWORD dwSize = EO_REGISTRY_BUFFER_SIZE;
		unsigned char data[EO_REGISTRY_BUFFER_SIZE];
		memset(data, 0, EO_REGISTRY_BUFFER_SIZE);

		wchar_t data_t[EO_REGISTRY_BUFFER_SIZE];
		wmemset(data_t, 0, EO_REGISTRY_BUFFER_SIZE);

		if (RegQueryValueExW(hKey, name, 0, 0, data, &dwSize) == ERROR_SUCCESS) {
			memcpy_s(&data_t, EO_REGISTRY_BUFFER_SIZE, &data, dwSize);
			rv = true;
		} else {
			if (ERROR_SUCCESS == RegEnumKeyExW(hKey, 0, data_t, &dwSize, NULL, NULL, NULL, NULL)) {
				rv = true;
			}
		}
		if (size < EO_REGISTRY_BUFFER_SIZE) {

			swprintf(value, L"%s\0", data_t);
		} else {
			wcsncpy(value, data_t, size - 1);
			value[size - 1] = '\0';
		}
		RegCloseKey(hKey);
	}
	return rv;
}

OdString GetRegistryAcadLocation() {
	OdString subkey = L"SOFTWARE\\Autodesk\\AutoCAD";
	wchar_t version[32];
	wchar_t subVersion[32];
	wchar_t searchPaths[EO_REGISTRY_MAX_PATH];

	// get the version and concatenate onto subkey
	if (GetRegistryString(HKEY_LOCAL_MACHINE, (LPCTSTR) subkey, _T("CurVer"), version, 32) == 0) {
		return L"";
	}
	subkey += L"\\";
	subkey += version;

	// get the sub-version and concatenate onto subkey
	if (GetRegistryString(HKEY_LOCAL_MACHINE, (LPCTSTR) subkey, _T("CurVer"), subVersion, 32) == 0) {
		return L"";
	}
	subkey += L"\\";
	subkey += subVersion;

	// get the value for the AcadLocation entry in the registry
	if (GetRegistryString(HKEY_LOCAL_MACHINE, (LPCTSTR) subkey, _T("AcadLocation"), searchPaths, EO_REGISTRY_MAX_PATH) == 0) {
		return L"";
	}
	return OdString(searchPaths);
}

OdString GetRegistryAcadProfilesKey() {
	OdString subkey = L"SOFTWARE\\Autodesk\\AutoCAD";
	wchar_t version[32];
	wchar_t subVersion[32];
	wchar_t profile[EO_REGISTRY_MAX_PROFILE_NAME];

	if (GetRegistryString(HKEY_CURRENT_USER, (LPCWSTR) subkey, L"CurVer", version, 32) == 0) {
		return L"";
	}
	subkey += L"\\";
	subkey += version;

	// get the sub-version and concatenate onto subkey
	if (GetRegistryString(HKEY_CURRENT_USER, (LPCWSTR) subkey, L"CurVer", subVersion, 32) == 0) {
		return L"";
	}
	subkey += L"\\";
	subkey += subVersion;
	subkey += L"\\Profiles";

	// get the value for the (Default) entry in the registry
	if (GetRegistryString(HKEY_CURRENT_USER, (LPCWSTR) subkey, L"", profile, EO_REGISTRY_MAX_PROFILE_NAME) == 0) {
		return L"";
	}
	subkey += L"\\";
	subkey += profile;

	return subkey;
}

void AeSysApp::ReleaseSimplexStrokeFont() noexcept {
	if (m_SimplexStrokeFont) {
		delete[] m_SimplexStrokeFont;
	}
}

OdString AeSysApp::recentGsDevicePath() const {
	return m_sVectorizerPath;
}

void AeSysApp::setRecentGsDevicePath(const OdString & vectorizerPath) {
	WriteProfileStringW(L"options", L"recent GS", vectorizerPath);
	m_sVectorizerPath = vectorizerPath;
}

void AeSysApp::SetStatusPaneTextAt(int index, LPCWSTR newText) {
	((CMainFrame*) GetMainWnd())->SetStatusPaneTextAt(index, newText);
}
OdDbHostAppProgressMeter* AeSysApp::newProgressMeter() {
	if (m_thisThreadID != ::GetCurrentThreadId()) { // disable access from other threads
		return 0;
	}
	return ExHostAppServices::newProgressMeter();
}
void AeSysApp::start(const OdString & displayString) {
	m_Msg = (LPCWSTR) displayString;
	m_nProgressPos = 0;
	m_nPercent = -1;
	// <tas="m_tbExt.SetProgressState(::AfxGetMainWnd()->GetSafeHwnd(), CTaskBarWin7Ext::PS_Normal);"</tas>
	// <tas="m_tbExt.SetProgressValue(::AfxGetMainWnd()->GetSafeHwnd(), 0, 100);"</tas>
}
void AeSysApp::stop() {
	m_nProgressPos = m_nProgressLimit;
	meterProgress();
	// <tas="m_tbExt.SetProgressState(::AfxGetMainWnd()->GetSafeHwnd(), CTaskBarWin7Ext::PS_NoProgress);"</tas>
	// <tas="m_tbExt.FlashWindow(::AfxGetMainWnd()->GetSafeHwnd());"</tas>
}
void AeSysApp::meterProgress() {
	bool UpdateProgress;
	int Percent;
	{
		TD_AUTOLOCK_P_DEF(m_pMeterMutex);
		const int OldPercent = m_nPercent;
		Percent = m_nPercent = int((double(m_nProgressPos++) / double(m_nProgressLimit)) * 100);
		UpdateProgress = (OldPercent != m_nPercent);
	}
	if (UpdateProgress) {
		struct StatUpdater {
			int m_Percent;
			CMainFrame* m_MainFrame;
			AeSysApp* m_Application;
			StatUpdater(int percent, CMainFrame* mainFrame, AeSysApp* application)  noexcept
				: m_Percent(percent)
				, m_MainFrame(mainFrame)
				, m_Application(application) {
			}
			static void Exec(void* statusUpdater) {
				StatUpdater* pExec = reinterpret_cast<StatUpdater*>(statusUpdater);
				CString str;
				str.Format(L"%s %d", (LPCWSTR) pExec->m_Application->m_Msg, pExec->m_Percent);
				// <tas="pExec->m_MainFrame->m_wndStatusBar.SetPaneText(0, str);:</tas>
				// <tas="pExec->m_Application->m_tbExt.SetProgressValue(::AfxGetMainWnd()->GetSafeHwnd(), (ULONG) pExec->m_nPercent, 100);"</tas>
				MSG Message;
				while (::PeekMessageW(&Message, pExec->m_MainFrame->m_hWnd, WM_KEYUP, WM_KEYUP, 1)) {
					bool bDup = false;
					if (Message.wParam == VK_ESCAPE && !bDup) {
						bDup = true;
						str.Format(L"Are you sure you want to terminate\n%s ?", (LPCWSTR) pExec->m_Application->m_Msg);
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
		} execArg(Percent, (CMainFrame*) GetMainWnd(), this);
		odExecuteMainThreadAction(StatUpdater::Exec, &execArg);
	}
}
void AeSysApp::setLimit(int max) noexcept {
	m_nProgressLimit = max ? max : 1;
}
int AeSysApp::ConfirmMessageBox(UINT stringResourceIdentifier, LPCWSTR string) {
	CString FormatSpecification = LoadStringResource(stringResourceIdentifier);

	CString FormattedResourceString;
	FormattedResourceString.Format(FormatSpecification, string);

	int NextToken = 0;
	CString Text = FormattedResourceString.Tokenize(L"\t", NextToken);
	CString Caption = FormattedResourceString.Tokenize(L"\n", NextToken);

	return (::MessageBoxW(0, Text, Caption, MB_ICONINFORMATION | MB_YESNOCANCEL | MB_DEFBUTTON2));
}
void AeSysApp::warning(const char* warnVisGroup, const OdString & text) {
	if (m_bLoading && (!warnVisGroup || !*warnVisGroup) && !m_bUseMTLoading) {
		if (::MessageBoxW(NULL, text + L"\n\nDo you want to proceed ?", L"Warning!", MB_ICONWARNING | MB_YESNO) == IDNO) {
			throw UserBreak();
		}
	}
}
void AeSysApp::WarningMessageBox(UINT stringResourceIdentifier) {
	CString ResourceString = LoadStringResource(stringResourceIdentifier);

	int NextToken = 0;
	CString Text = ResourceString.Tokenize(L"\t", NextToken);
	CString Caption = ResourceString.Tokenize(L"\n", NextToken);

	::MessageBoxW(0, Text, Caption, MB_ICONWARNING | MB_OK);
}
void AeSysApp::WarningMessageBox(UINT stringResourceIdentifier, LPCWSTR string) {
	CString FormatSpecification = LoadStringResource(stringResourceIdentifier);

	CString FormattedResourceString;
	FormattedResourceString.Format(FormatSpecification, string);

	int NextToken = 0;
	CString Text = FormattedResourceString.Tokenize(L"\t", NextToken);
	CString Caption = FormattedResourceString.Tokenize(L"\n", NextToken);

	::MessageBoxW(0, Text, Caption, MB_ICONWARNING | MB_OK);
}
int AeSysApp::messageBox(LPCWSTR caption, LPCWSTR text, UINT type) {
	CWnd* MainWnd = GetMainWnd();
	HWND hWnd = 0;
	if (MainWnd) {
		hWnd = MainWnd->m_hWnd;
	}
	return ::MessageBoxW(hWnd, text, caption, type);
}
void AeSysApp::reportError(LPCWSTR caption, unsigned int error) {
	messageBox(caption, (LPCWSTR) getErrorDescription(error), MB_OK | MB_ICONERROR);
}

void AeSysApp::initPlotStyleSheetEnv() {
	OdString StyleSheetFiles = FindConfigPath(L"PrinterStyleSheetDir");
	_wputenv_s(L"DDPLOTSTYLEPATHS", StyleSheetFiles);
}
CString AeSysApp::ResourceFolderPath() {
	return (getApplicationPath() + L"\\res\\");
}
void AeSysApp::SetArchitecturalUnitsFractionPrecision(const int precision) noexcept {
	if (precision > 0) m_ArchitecturalUnitsFractionPrecision = precision;
}
void AeSysApp::SetDimensionAngle(double angle) noexcept {
	m_DimensionAngle = angle;
}
void AeSysApp::SetDimensionLength(double length) noexcept {
	m_DimensionLength = length;
}
void AeSysApp::SetEngagedAngle(double angle) noexcept {
	m_EngagedAngle = angle;
}
void AeSysApp::SetEngagedLength(double length) noexcept {
	m_EngagedLength = length;
}
int AeSysApp::SetShadowFolderPath(const CString & folder) {
	wchar_t Path[MAX_PATH];

	if (SHGetSpecialFolderPathW(m_pMainWnd->GetSafeHwnd(), Path, CSIDL_PERSONAL, TRUE)) {
		m_ShadowFolderPath = Path;
	} else {
		m_ShadowFolderPath.Empty();
	}
	m_ShadowFolderPath += L"\\" + folder + L"\\";

	return (_wmkdir(m_ShadowFolderPath));
}
void AeSysApp::SetUnits(Units units) noexcept {
	m_Units = units;
}
CString AeSysApp::ShadowFolderPath() const noexcept {
	return m_ShadowFolderPath;
}
char* AeSysApp::SimplexStrokeFont() noexcept {
	return m_SimplexStrokeFont;
}
OdInt16 AeSysApp::TrapHighlightColor() const noexcept {
	return m_TrapHighlightColor;
}
void AeSysApp::UninitializeTeigha() {
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
	} catch (const OdError & Error) {
		theApp.reportError(L"", Error);
	}
}
void AeSysApp::UpdateMDITabs(BOOL resetMDIChild) {
	((CMainFrame*) AfxGetMainWnd())->UpdateMDITabs(resetMDIChild);
}
BOOL AeSysApp::OnIdle(long count) {

	for (size_t ReactorIndex = 0; ReactorIndex < m_aAppReactors.size(); ++ReactorIndex) {
		m_aAppReactors[ReactorIndex]->OnIdle(count);
	}
	return __super::OnIdle(count);
}

BOOL AeSysApp::PreTranslateMessage(MSG * message) {

	for (size_t ReactorIndex = 0; ReactorIndex < m_aAppReactors.size(); ++ReactorIndex) {
		m_aAppReactors[ReactorIndex]->OnPreTranslateMessage(message);
	}
	return __super::PreTranslateMessage(message);
}

/// <section="vectorizer menu - add new and clear all">

bool addGsMenuItem(CMenu * vectorizePopupMenu, DWORD & numberOfVectorizers, LPCWSTR vectorizerPath) {
	if (ID_VECTORIZER_FIRST + numberOfVectorizers <= ID_VECTORIZER_LAST) {
		vectorizePopupMenu->InsertMenuW(numberOfVectorizers, MF_BYPOSITION, ID_VECTORIZER_FIRST + numberOfVectorizers, vectorizerPath);

		MENUITEMINFO menuItemInfo;
		menuItemInfo.cbSize = sizeof(menuItemInfo);
		menuItemInfo.fMask = MIIM_DATA;
		menuItemInfo.dwItemData = theApp.getGSMenuItemMarker();
		VERIFY(::SetMenuItemInfoW(vectorizePopupMenu->m_hMenu, numberOfVectorizers, TRUE, &menuItemInfo));

		if (theApp.recentGsDevicePath().iCompare(OdString(vectorizerPath)) == 0) {
			vectorizePopupMenu->CheckMenuItem(numberOfVectorizers, MF_BYPOSITION | MF_CHECKED);
		}
		++numberOfVectorizers;
		return true;
	}
	return false;
}

void AeSysApp::OnVectorizeAddVectorizerDLL() {
	const DWORD Flags {OFN_HIDEREADONLY | OFN_EXPLORER | OFN_PATHMUSTEXIST};
	CString Filter {L"Graphic System DLL (*."  VECTORIZATION_MODULE_EXTENSION_W  L")|*." VECTORIZATION_MODULE_EXTENSION_W  L"|Windows DLL (*.dll)|*.dll||"};

	CFileDialog dlg(TRUE, VECTORIZATION_MODULE_EXTENSION_W, L"", Flags, Filter, ::AfxGetMainWnd());
	dlg.m_ofn.lpstrTitle = L"Select Graphic System DLL";

	auto s_path {getApplicationPath()};
	dlg.m_ofn.lpstrInitialDir = s_path.GetBuffer(s_path.GetLength());

	if (dlg.DoModal() == IDOK) {
		m_sVectorizerPath = (LPCWSTR) dlg.GetFileName();
		m_sVectorizerPath.replace(TD_DLL_VERSION_SUFFIX_STR, L"");

		const auto TopMenu {CMenu::FromHandle(theApp.GetAeSysMenu())};
		auto VectorizePopupMenu {TopMenu->GetSubMenu(3)};

		::addGsMenuItem(VectorizePopupMenu, m_numGSMenuItems, m_sVectorizerPath);
		WriteProfileStringW(L"options\\vectorizers", m_sVectorizerPath, L"");
		GetMainWnd()->SendMessage(WM_COMMAND, ID_VECTORIZE);
}
}

void AeSysApp::OnUpdateVectorizeAddvectorizerdll(CCmdUI * pCmdUI) {
	if (m_numGSMenuItems == 0) {
		const auto TopMenu {CMenu::FromHandle(theApp.GetAeSysMenu())};
		auto VectorizePopupMenu {TopMenu->GetSubMenu(3)};

		CRegKey RegistryKey;
		RegistryKey.Create(HKEY_CURRENT_USER, L"Software\\Engineers Office\\AeSys\\options\\vectorizers");

		CString path;
		DWORD pathSize;
		for (;;) {
			pathSize = _MAX_FNAME + _MAX_EXT;
			const auto Status {::RegEnumValueW(RegistryKey, m_numGSMenuItems, path.GetBuffer(pathSize), &pathSize, NULL, NULL, NULL, NULL)};
			path.ReleaseBuffer();
			if (Status == ERROR_SUCCESS) {
				if (!::addGsMenuItem(VectorizePopupMenu, m_numGSMenuItems, path)) {
					break;
				}
			} else
				break;
		}
	}
}

void AeSysApp::OnVectorizeClearmenu() {
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

void AeSysApp::OnUpdateVectorizeClearmenu(CCmdUI * pCmdUI) {
	pCmdUI->Enable(m_numGSMenuItems > 0);
}
/// </section>
