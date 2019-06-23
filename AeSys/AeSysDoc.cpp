#include "stdafx.h"
#include <atlbase.h>
#include <strsafe.h>
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "PrimState.h"
#include <ColorMapping.h>
#include <DbLayerTable.h>
#include <DbLinetypeTable.h>
#include <DbLinetypeTableRecord.h>
#include <DbRegAppTable.h>
#include <DbRegAppTableRecord.h>
#include <DbDimStyleTable.h>
#include <DbDimStyleTableRecord.h>
#include <DbTextStyleTable.h>
#include <DbTextStyleTableRecord.h>
#include <Ed/EdLispEngine.h>
#include <DbObjectContextCollection.h>
#include <DbObjectContextManager.h>
#include <SaveState.h>
#include <ExStringIO.h>
#include <ExPageController.h>
#include "EoDbHatch.h"
#include "EoDbPolyline.h"
#include "EoDbPegFile.h"
#include "EoDbTracingFile.h"
#include "EoAppAuditInfo.h"
#include "EoDlgUserIOConsole.h"
#include "EoGePoint4d.h"
#include "EoGeMatrix3d.h"
#include "EoDlgAudit.h"
#include "EoDlgEditProperties.h"
#include "EoDbDwgToPegFile.h"
#include "EoDlgLayerPropertiesManager.h"
#include "EoDlgPageSetup.h"
#include "EoDlgNamedViews.h"
#include "EoDbBlockReference.h"
#include "EoDbDimension.h"
#include "EoDbJobFile.h"
#include "EoDlgDrawOptions.h"
#include "EoDlgEditTrapCommandsQuery.h"
#include "EoDlgFileManage.h"
#include "EoDlgSelectGotoHomePoint.h"
#include "EoDlgSetActiveLayout.h"
#include "EoDlgSetHomePoint.h"
#include "EoDlgSetPastePosition.h"
#include "EoDlgSetupColor.h"
#include "EoDlgSetupHatch.h"
#include "EoDlgSetupNote.h"
#include "EoDlgSetupLinetype.h"
#include "EoDlgTrapFilter.h"
#include "lex.h"
unsigned CALLBACK OfnHookProcFileTracing(HWND, unsigned, WPARAM, LPARAM);

unsigned AFXAPI HashKey(const CString& string) noexcept {
	auto String {static_cast<const wchar_t*>(string)};
	unsigned Hash {0};
	while (*String) {
		Hash = (Hash << 5) + Hash + *String++;
	}
	return Hash;
}

// AeSysDoc
IMPLEMENT_DYNCREATE(AeSysDoc, COleDocument)

#define NEW_CONSTR(CLASS) OdSmartPtr<CLASS>(new CLASS, kOdRxObjAttach)
ODRX_CONS_DEFINE_MEMBERS(OdDbDatabaseDoc, OdDbDatabase, NEW_CONSTR);

AeSysDoc* OdDbDatabaseDoc::g_pDoc = nullptr;

OdDbDatabaseDoc::OdDbDatabaseDoc() noexcept
	: m_pDoc(g_pDoc) {
	g_pDoc = nullptr;
}

AeSysDoc* OdDbDatabaseDoc::document() const noexcept {
	return m_pDoc;
}

void OdDbDatabaseDoc::setDocToAssign(AeSysDoc* document) noexcept {
	g_pDoc = document;
}

BEGIN_MESSAGE_MAP(AeSysDoc, CDocument)
		ON_COMMAND(ID_PURGE_UNREFERENCEDBLOCKS, OnPurgeUnreferencedBlocks)
		ON_COMMAND(ID_CLEAR_ACTIVELAYERS, OnClearActiveLayers)
		ON_COMMAND(ID_CLEAR_ALLLAYERS, OnClearAllLayers)
		ON_COMMAND(ID_CLEAR_ALLTRACINGS, OnClearAllTracings)
		ON_COMMAND(ID_CLEAR_MAPPEDTRACINGS, OnClearMappedTracings)
		ON_COMMAND(ID_CLEAR_VIEWEDTRACINGS, OnClearViewedTracings)
		ON_COMMAND(ID_CLEAR_WORKINGLAYER, OnClearWorkingLayer)
		ON_COMMAND(ID_EDIT_IMAGETOCLIPBOARD, OnEditImageToClipboard)
		ON_COMMAND(ID_EDIT_SEGTOWORK, OnEditSegToWork)
		ON_COMMAND(ID_EDIT_TRACE, OnEditTrace)
		ON_COMMAND(ID_EDIT_TRAPCOPY, OnEditTrapCopy)
		ON_COMMAND(ID_EDIT_TRAPCUT, OnEditTrapCut)
		ON_COMMAND(ID_EDIT_TRAPDELETE, OnEditTrapDelete)
		ON_COMMAND(ID_EDIT_TRAPPASTE, OnEditTrapPaste)
		ON_COMMAND(ID_EDIT_TRAPQUIT, OnEditTrapQuit)
		ON_COMMAND(ID_EDIT_TRAPWORK, OnEditTrapWork)
		ON_COMMAND(ID_EDIT_TRAPWORKANDACTIVE, OnEditTrapWorkAndActive)
		ON_COMMAND(ID_FILE, OnFile)
		ON_COMMAND(ID_FILE_QUERY, OnFileQuery)
		ON_COMMAND(ID_FILE_MANAGE, OnFileManage)
		ON_COMMAND(ID_FILE_SEND_MAIL, CDocument::OnFileSendMail)
		ON_COMMAND(ID_FILE_TRACING, OnFileTracing)
#ifdef OD_OLE_SUPPORT
	ON_COMMAND(ID_INSERT_OLEOBJECT, OnInsertOleobject)
#endif // OD_OLE_SUPPORT
		ON_COMMAND(ID_LAYER_ACTIVE, OnLayerActive)
		ON_COMMAND(ID_LAYER_LOCK, OnLayerLock)
		ON_COMMAND(ID_LAYER_OFF, OnLayerOff)
		ON_COMMAND(ID_LAYER_MELT, OnLayerMelt)
		ON_COMMAND(ID_LAYER_CURRENT, OnLayerCurrent)
		ON_COMMAND(ID_LAYERS_SETALLACTIVE, OnLayersSetAllActive)
		ON_COMMAND(ID_LAYERS_SETALLLOCKED, OnLayersSetAllLocked)
		ON_COMMAND(ID_PURGE_UNUSEDLAYERS, OnPurgeUnusedLayers)
		ON_COMMAND(ID_PURGE_DUPLICATEOBJECTS, &AeSysDoc::OnPurgeDuplicateObjects)
		ON_COMMAND(ID_PURGE_EMPTYNOTES, OnPurgeEmptyNotes)
		ON_COMMAND(ID_PURGE_EMPTYGROUPS, OnPurgeEmptyGroups)
		ON_COMMAND(ID_PENS_REMOVEUNUSEDSTYLES, OnPensRemoveUnusedLinetypes)
		ON_COMMAND(ID_PENS_EDITCOLORS, OnPensEditColors)
		ON_COMMAND(ID_PENS_LOADCOLORS, OnPensLoadColors)
		ON_COMMAND(ID_PENS_TRANSLATE, OnPensTranslate)
		ON_COMMAND(ID_PRIM_BREAK, OnPrimBreak)
		ON_COMMAND(ID_PRIM_EXTRACTNUM, OnPrimExtractNum)
		ON_COMMAND(ID_PRIM_EXTRACTSTR, OnPrimExtractStr)
		ON_COMMAND(ID_TOOLS_PRIMITIVE_SNAPTOENDPOINT, OnToolsPrimitiveSnapToEndPoint)
		ON_COMMAND(ID_PRIM_GOTOCENTERPOINT, OnPrimGotoCenterPoint)
		ON_COMMAND(ID_TOOLS_PRIMITVE_DELETE, OnToolsPrimitiveDelete)
		ON_COMMAND(ID_PRIM_MODIFY_ATTRIBUTES, OnPrimModifyAttributes)
		ON_COMMAND(ID_TOOLS_GROUP_BREAK, OnToolsGroupBreak)
		ON_COMMAND(ID_TOOLS_GROUP_DELETE, OnToolsGroupDelete)
		ON_COMMAND(ID_TOOLS_GROUP_DELETELAST, OnToolsGroupDeleteLast)
		ON_COMMAND(ID_TOOLS_GROUP_EXCHANGE, OnToolsGroupExchange)
		ON_COMMAND(ID_TOOLS_GROUP_RESTORE, OnToolsGroupRestore)
		ON_COMMAND(ID_SETUP_PENCOLOR, OnSetupPenColor)
		ON_COMMAND(ID_SETUP_LINETYPE, OnSetupLinetype)
		ON_COMMAND(ID_SETUP_FILL_HOLLOW, OnSetupFillHollow)
		ON_COMMAND(ID_SETUP_FILL_SOLID, OnSetupFillSolid)
		ON_COMMAND(ID_SETUP_FILL_PATTERN, OnSetupFillPattern)
		ON_COMMAND(ID_SETUP_FILL_HATCH, OnSetupFillHatch)
		ON_COMMAND(ID_SETUP_NOTE, OnSetupNote)
		ON_COMMAND(ID_SETUP_SAVEPOINT, OnSetupSavePoint)
		ON_COMMAND(ID_SETUP_GOTOPOINT, OnSetupGotoPoint)
		ON_COMMAND(ID_SETUP_OPTIONS_DRAW, OnSetupOptionsDraw)
		ON_COMMAND(ID_TRACING_ACTIVE, OnTracingActive)
		ON_COMMAND(ID_TRACING_LOCK, OnTracingLock)
		ON_COMMAND(ID_TRACING_OFF, OnTracingOff)
		ON_COMMAND(ID_TRACING_FUSE, OnTracingFuse)
		ON_COMMAND(ID_TRACING_CURRENT, OnTracingCurrent)
		ON_COMMAND(ID_TRAPCOMMANDS_COMPRESS, OnTrapCommandsCompress)
		ON_COMMAND(ID_TRAPCOMMANDS_EXPAND, OnTrapCommandsExpand)
		ON_COMMAND(ID_TRAPCOMMANDS_INVERT, OnTrapCommandsInvert)
		ON_COMMAND(ID_TRAPCOMMANDS_SQUARE, OnTrapCommandsSquare)
		ON_COMMAND(ID_TRAPCOMMANDS_QUERY, OnTrapCommandsQuery)
		ON_COMMAND(ID_TRAPCOMMANDS_FILTER, OnTrapCommandsFilter)
		ON_COMMAND(ID_TRAPCOMMANDS_BLOCK, OnTrapCommandsBlock)
		ON_COMMAND(ID_TRAPCOMMANDS_UNBLOCK, OnTrapCommandsUnblock)
		ON_COMMAND(ID_HELP_KEY, OnHelpKey)
		ON_COMMAND(ID_SETUP_LAYERPROPERTIES, &AeSysDoc::OnSetupLayerProperties)
		ON_COMMAND(ID_INSERT_TRACING, &AeSysDoc::OnInsertTracing)
		ON_COMMAND(ID_FILE_PAGESETUP, &AeSysDoc::OnFilePageSetup)
		ON_COMMAND(ID_VIEW_SETACTIVELAYOUT, &AeSysDoc::OnViewSetActiveLayout)
		ON_COMMAND(ID_DRAWINGUTILITIES_AUDIT, &AeSysDoc::OnDrawingUtilitiesAudit)
		ON_COMMAND(ID_SELECTIONSETCOMMANDS_CLEAR, &AeSysDoc::OnEditClearSelection)
		ON_COMMAND(ID_EDIT_CONSOLE, &AeSysDoc::OnEditConsole)
		ON_COMMAND(ID_VIEW_NAMEDVIEWS, &AeSysDoc::OnViewNamedViews)
		ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
		ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
		ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
		ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
		ON_COMMAND(ID_SELECTIONSETCOMMANDS_SELECTALL, &AeSysDoc::OnEditSelectAll)
		ON_COMMAND(ID_SELECTIONSETCOMMANDS_ENTGET, &AeSysDoc::OnEditEntget)
		ON_COMMAND(ID_VECTORIZERTYPE, &AeSysDoc::OnVectorizerType)
		ON_UPDATE_COMMAND_UI(ID_VECTORIZERTYPE, &AeSysDoc::OnUpdateVectorizerType)
END_MESSAGE_MAP()
unsigned short AeSysDoc::ClipboardData::m_FormatR15 = static_cast<unsigned short>(RegisterClipboardFormatW(L"AutoCAD.r15"));
unsigned short AeSysDoc::ClipboardData::m_FormatR16 = static_cast<unsigned short>(RegisterClipboardFormatW(L"AutoCAD.r16"));
unsigned short AeSysDoc::ClipboardData::m_FormatR17 = static_cast<unsigned short>(RegisterClipboardFormatW(L"AutoCAD.r17"));
unsigned short AeSysDoc::ClipboardData::m_FormatR18 = static_cast<unsigned short>(RegisterClipboardFormatW(L"AutoCAD.r18"));
unsigned short AeSysDoc::ClipboardData::m_FormatR19 = static_cast<unsigned short>(RegisterClipboardFormatW(L"AutoCAD.r19"));
AeSysDoc* g_pDoc {nullptr};

AeSysDoc::AeSysDoc() noexcept {
	g_pDoc = this;
	m_pRefDocument = OdApplicationDocumentImpl::createObject(this);
}
// <tas="crash with smart pointer m_DatabasePtr release"\>
AeSysDoc::~AeSysDoc() {
	m_pRefDocument->m_pImp->SetNull();
}

BOOL AeSysDoc::DoSave(const wchar_t* pathName, const BOOL replace) {
	m_SaveAsVer = m_DatabasePtr->originalFileVersion();
	CString PathName(pathName);
	if (PathName.IsEmpty()) { // Save As
		const auto DocTemplate {GetDocTemplate()};
		PathName = m_strPathName;
		if (replace && PathName.IsEmpty()) {
			PathName = m_strTitle;
			const auto BadCharacterPosition {PathName.FindOneOf(L" #%;/\\")};
			if (BadCharacterPosition != -1) { PathName.ReleaseBuffer(BadCharacterPosition); }
			CString Extension;
			if (DocTemplate->GetDocString(Extension, CDocTemplate::filterExt) && !Extension.IsEmpty()) {
				ASSERT(Extension[0] == '.');
				PathName += Extension;
			}
		}
		if (!DoPromptFileName(PathName, static_cast<unsigned>(replace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY), OFN_HIDEREADONLY | OFN_EXPLORER | OFN_PATHMUSTEXIST, FALSE, DocTemplate)) {
			return FALSE;
		} // don't even attempt to save 
	}
	if (!OnSaveDocument(PathName)) {
		if (pathName == nullptr) {
			TRY
				{
					CFile::Remove(PathName);
				}
			CATCH_ALL(Errors)
				{
					theApp.AddStringToMessageList(L"Warning: Failed to delete file <%s> after failed/aborted SaveAs", PathName);
					do {
						Errors->Delete();
					} while (0);
				}
			END_CATCH_ALL
		}
		return FALSE;
	}
	if (replace) { SetPathName(PathName); }
	return TRUE;
}

void AeSysDoc::DeleteContents() {
	RemoveAllBlocks();
	RemoveAllLayers();
	m_WorkLayer = nullptr;
	DeletedGroupsRemoveGroups();
	RemoveAllTrappedGroups();
	RemoveAllGroupsFromAllViews();
	DeleteNodalResources();
	ResetAllViews();
	const auto NumberOfReactors {theApp.m_ApplicationReactors.size()};
	for (unsigned ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
		theApp.m_ApplicationReactors.at(ReactorIndex)->DocumentToBeDestroyed(this);
	}
	if (!m_DatabasePtr.isNull()) {
		m_DatabasePtr->appServices()->layoutManager()->removeReactor(this);
	}
	// <tas="crash with smart pointer m_DatabasePtr release"> m_DatabasePtr.release();"</tas>
	m_DatabasePtr.release();
	COleDocument::DeleteContents();
	for (unsigned ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
		theApp.m_ApplicationReactors.at(ReactorIndex)->DocumentDestroyed(static_cast<const wchar_t*>(GetPathName()));
	}
}

BOOL AeSysDoc::CanCloseFrame(CFrameWnd* frame) {
	const auto ActiveView {frame->GetActiveView()};
	if (ActiveView->IsKindOf(&AeSysView::classAeSysView)) {

		if (!dynamic_cast<AeSysView*>(ActiveView)->canClose()) { return FALSE; }

	}
	return CDocument::CanCloseFrame(frame);
}

AeSysView* AeSysDoc::getViewer() noexcept {
	return m_pViewer;
}

void AeSysDoc::OnViewSetActiveLayout() {
	EoDlgSetActiveLayout ActiveLayoutDialog(m_DatabasePtr, theApp.GetMainWnd());
	m_LayoutSwitchable = true;
	if (ActiveLayoutDialog.DoModal() == IDOK) {
		try {
			m_DatabasePtr->startUndoRecord();
			m_DatabasePtr->setCurrentLayout(OdString(ActiveLayoutDialog.m_NewLayoutName));
		} catch (const OdError& Error) {
			theApp.reportError(L"Error Setting Layout...", Error);
			m_DatabasePtr->disableUndoRecording(true);
			m_DatabasePtr->undo();
			m_DatabasePtr->disableUndoRecording(false);
		}
	}
	m_LayoutSwitchable = false;
}

void AeSysDoc::layoutSwitched(const OdString& newLayoutName, const OdDbObjectId& newLayout) {
	if (!m_LayoutSwitchable) { return; }

	// This test can be exchanged by remove/add reactor in layout manager, but this operations must be added into all functions which can call setCurrentLayout (but where vectorization no need to be changed).
	auto ViewPosition {GetFirstViewPosition()};
	while (ViewPosition != nullptr) {
		const auto View {GetNextView(ViewPosition)};
		if (OdString(View->GetRuntimeClass()->m_lpszClassName).compare(L"AeSysView") == 0) {

			if (View->GetDocument() == this) {
				const auto Parent {View->GetParent()};
				// Get prev params
				const auto Iconic {Parent->IsIconic() != FALSE};
				const auto Zoomed {Parent->IsZoomed() != FALSE};
				CRect ParentRectangle;
				Parent->GetWindowRect(&ParentRectangle);
				CPoint TopLeftPoint(ParentRectangle.left, ParentRectangle.top);
				CPoint BottomRightPoint(ParentRectangle.right, ParentRectangle.bottom);
				Parent->GetParent()->ScreenToClient(&TopLeftPoint);
				Parent->GetParent()->ScreenToClient(&BottomRightPoint);
				ParentRectangle.left = TopLeftPoint.x;
				ParentRectangle.top = TopLeftPoint.y;
				ParentRectangle.right = BottomRightPoint.x;
				ParentRectangle.bottom = BottomRightPoint.y;
				View->GetParent()->SendMessageW(WM_CLOSE);
				OnVectorizerType();
				// Search again for new view
				auto ViewPosition {GetFirstViewPosition()};
				while (ViewPosition != nullptr) {
					const auto view {GetNextView(ViewPosition)};
					if (OdString(view->GetRuntimeClass()->m_lpszClassName).compare(L"AeSysView") == 0) {

						if (view->GetDocument() == this) {
							auto Parent {view->GetParent()};
							if (Zoomed) {

								if (!Parent->IsZoomed()) { dynamic_cast<CMDIChildWnd*>(Parent)->MDIMaximize(); }
							} else {
								dynamic_cast<CMDIChildWnd*>(Parent)->MDIRestore();
								if (!Iconic) { Parent->SetWindowPos(nullptr, ParentRectangle.left, ParentRectangle.top, ParentRectangle.Width(), ParentRectangle.Height(), SWP_NOZORDER); }
							}
							break;
						}
					}
				}
			}
		}
	}
}

const OdString Cmd_VIEW::groupName() const { return L"AeSys"; }

const OdString Cmd_VIEW::name() { return L"VIEW"; }

const OdString Cmd_VIEW::globalName() const { return name(); }

void Cmd_VIEW::execute(OdEdCommandContext* commandContext) {
	OdDbCommandContextPtr CommandContext(commandContext);
	OdDbDatabaseDocPtr Database = CommandContext->database();
	EoDlgNamedViews NamedViewsDialog(Database->document(), theApp.GetMainWnd());
	if (NamedViewsDialog.DoModal() != IDOK) {
		throw OdEdCancel();
	}
}

const OdString Cmd_SELECT::groupName() const { return L"AeSys"; }

const OdString Cmd_SELECT::name() { return L"SELECT"; }

const OdString Cmd_SELECT::globalName() const { return name(); }

void Cmd_SELECT::execute(OdEdCommandContext* commandContext) {
	OdDbCommandContextPtr CommandContext(commandContext);
	OdDbDatabaseDocPtr Database = CommandContext->database();
	auto Document {Database->document()};
	auto View {Document->getViewer()};
	if (View == nullptr) { throw OdEdCancel(); }
	Document->OnEditClearSelection();
	Document->UpdateAllViews(nullptr);
	auto UserIO {CommandContext->dbUserIO()};
	UserIO->setPickfirst(nullptr);
	const auto SelectOptions {OdEd::kSelLeaveHighlighted | OdEd::kSelAllowEmpty};
	try {
		OdDbSelectionSetPtr SelectionSet {UserIO->select(L"", SelectOptions, View->editorObject().workingSSet())};
		View->editorObject().SetWorkingSelectionSet(SelectionSet);
	} catch (const OdError&) {
		throw OdEdCancel();
	}
	View->editorObject().SelectionSetChanged();
	Database->pageObjects();
}

struct CDocTemplateEx : CDocTemplate {
	void SetViewToCreate(CRuntimeClass* viewClass) noexcept {
		m_pViewClass = viewClass;
	}
};

void AeSysDoc::OnVectorize(const OdString& vectorizerPath) {
	theApp.SetRecentGsDevicePath(vectorizerPath);
	auto DocTemplate {dynamic_cast<CDocTemplateEx*>(GetDocTemplate())};
	ASSERT_VALID(DocTemplate);
	DocTemplate->SetViewToCreate(RUNTIME_CLASS(AeSysView));
	const auto NewFrame {DocTemplate->CreateNewFrame(this, nullptr)};
	DocTemplate->InitialUpdateFrame(NewFrame, this);
	m_pViewer = dynamic_cast<AeSysView*>(NewFrame->GetActiveView());
}

void AeSysDoc::OnCloseVectorizer(AeSysView* view) {
	if (view != m_pViewer) { TRACE0("Vectorizer does not match expected viewer\n"); }
	m_pViewer = nullptr;
}

void AeSysDoc::setVectorizer(AeSysView* view) {
	// <tas="limits the command context to a single view per document. So New window crashes."/>"
	ODA_ASSERT(m_pViewer == nullptr);
	m_pViewer = view;
}

void AeSysDoc::OnVectorizerType() {
	OnVectorize(theApp.RecentGsDevicePath());
}

void AeSysDoc::OnUpdateVectorizerType(CCmdUI* commandUserInterface) {
	commandUserInterface->Enable(m_pViewer == nullptr && !theApp.RecentGsDevicePath().isEmpty());
}

OdDbCommandContextPtr AeSysDoc::CommandContext0() {
	if (m_CommandContext.isNull()) { m_CommandContext = ExDbCommandContext::createObject(BaseIO(), m_DatabasePtr); }
	return m_CommandContext;
}

OdDbSelectionSetPtr AeSysDoc::SelectionSet() const {
	OdDbCommandContext* CommandContext {const_cast<AeSysDoc*>(this)->CommandContext0()};
	OdDbSelectionSetPtr SelectionSet {CommandContext->arbitraryData(L"AeSys Working Selection Set")};
	if (SelectionSet.isNull()) {
		SelectionSet = OdDbSelectionSet::createObject(m_DatabasePtr);
		CommandContext->setArbitraryData(L"AeSys Working Selection Set", SelectionSet);
	}
	TRACE1("Working Selection set contains %d items\n", SelectionSet->numEntities());
	return SelectionSet;
}

OdEdBaseIO* AeSysDoc::BaseIO() noexcept {
	return this;
}

EoDlgUserIOConsole* AeSysDoc::UserIOConsole() {
	if (m_UserIOConsole.isNull()) { m_UserIOConsole = EoDlgUserIOConsole::create(theApp.GetMainWnd()); }
	return m_UserIOConsole;
}

unsigned long AeSysDoc::getKeyState() noexcept {
	unsigned long KeyState(0);
	if (GetKeyState(VK_CONTROL) != 0) { KeyState |= MK_CONTROL; }
	if (GetKeyState(VK_SHIFT) != 0) { KeyState |= MK_SHIFT; }
	return KeyState;
}

OdGePoint3d AeSysDoc::getPoint(const OdString& prompt, const int options, OdEdPointTracker* tracker) {
	if (m_pMacro.get() && !m_pMacro->isEof()) {
		const auto strRes {getString(prompt, options, nullptr)};
		throw OdEdOtherInput(strRes);
	}
	if (m_bConsole) { return m_UserIOConsole->getPoint(prompt, options, tracker); }
	if (m_pViewer) {
		UserIOConsole()->putString(prompt);
		return m_pViewer->getPoint(prompt, options, tracker);
	}
	return UserIOConsole()->getPoint(prompt, options, tracker);
}

OdString AeSysDoc::getString(const OdString& prompt, const int options, OdEdStringTracker* tracker) {
	OdString Result;
	if (m_pMacro.get() && !m_pMacro->isEof()) {
		Result = m_pMacro->getString(prompt, options, tracker);
		putString(prompt + L" " + Result);
		return Result;
	}
	if (m_bConsole) { return UserIOConsole()->getString(prompt, options, tracker); }
	if (m_pViewer) {
		m_ConsoleResponded = prompt.isEmpty();
		Result = m_pViewer->getString(prompt, options, tracker);
		if (!m_ConsoleResponded) { putString(OdString(prompt) + L" " + Result); }
		return Result;
	}
	return UserIOConsole()->getString(prompt, options, tracker);
}

void AeSysDoc::putString(const OdString& string) {
	if (m_pViewer) { m_pViewer->putString(string); }
	theApp.AddStringToMessageList(string);
	UserIOConsole()->putString(string);
}

OdString AeSysDoc::RecentCommand() {
	return theApp.GetRecentCmd();
}

OdString AeSysDoc::RecentCommandName() {
	return theApp.GetRecentCmd().spanExcluding(L" \n");
}

OdString AeSysDoc::CommandPrompt() {
	return L"Command:";
}

void AeSysDoc::OnEditConsole() {
	auto CommandStack {odedRegCmds()};
	OdDbCommandContextPtr CommandContext(CommandContext0());
	OdSaveState<bool> saveConsoleMode(m_bConsole, true);
	try {
		if (m_pViewer && m_pViewer->isGettingString()) {

			m_pViewer->respond(UserIOConsole()->getString(m_pViewer->prompt(), m_pViewer->inpOptions(), nullptr));
			m_ConsoleResponded = true;

		} else {
			for (;;) {
				auto CommandName {CommandContext->userIO()->getString(CommandPrompt(), 0, L"")};
				if (CommandName.isEmpty()) {
					CommandName = RecentCommandName();
					if (!CommandName.isEmpty()) {
						CommandContext->userIO()->putString(CommandName);
						ExecuteCommand(CommandName, false);
					}
				} else {
					ExecuteCommand(CommandName, false);
				}
			}
		}
	} catch (const OdEdCancel&) {
	}
}

OdString commandMessageCaption(const OdString& command) {
	OdString Caption;
	Caption.format(L"Command: %ls", command.c_str());
	return Caption;
}

class CmdReactor : public OdStaticRxObject<OdEdCommandStackReactor>, public OdStaticRxObject<OdDbDatabaseReactor> {
	ODRX_NO_HEAP_OPERATORS()

	OdDbCommandContext* m_CommandContext;
	bool m_Modified;
	OdString m_LastInput;

	void setModified() {
		m_Modified = true;
		m_CommandContext->database()->removeReactor(this);
	}

public:
	CmdReactor(OdDbCommandContext* dbCommandContext)
		: m_CommandContext(dbCommandContext)
		, m_Modified(false) {
		ODA_ASSERT(m_CommandContext);
		odedRegCmds()->addReactor(this);
		m_CommandContext->database()->addReactor(this);
	}

	~CmdReactor() {
		odedRegCmds()->removeReactor(this);
		if (!m_Modified) { m_CommandContext->database()->removeReactor(this); }
	}

	void SetLastInput(const OdString& lastInput) {
		m_LastInput = lastInput;
	}

	[[nodiscard]] const OdString& LastInput() const noexcept {
		return m_LastInput;
	}

	[[nodiscard]] bool isDatabaseModified() const noexcept {
		return m_Modified;
	}

	void objectOpenedForModify(const OdDbDatabase* database, const OdDbObject* object) override {
		setModified();
	}

	void headerSysVarWillChange(const OdDbDatabase* database, const OdString& variableName) override {
		setModified();
	}

	OdEdCommandPtr unknownCommand(const OdString& commandName, OdEdCommandContext* commandContext) override {
		auto pViewer {OdDbDatabaseDocPtr(m_CommandContext->database())->document()->getViewer()};
		if (pViewer) {
			auto Command {pViewer->command(commandName)};
			if (Command.get()) { return Command; }
		}
		OdString String;
		String.format(L"Unknown command \"%ls\".", commandName.c_str());
		m_CommandContext->userIO()->putString(String);
		return OdEdCommandPtr();
	}

	void commandWillStart(OdEdCommand* command, OdEdCommandContext* edCommandContext) override {
		m_LastInput.makeUpper();
		if (!GETBIT(command->flags(), OdEdCommand::kNoHistory)) { theApp.SetRecentCommand(m_LastInput); }
		if (!GETBIT(command->flags(), OdEdCommand::kNoUndoMarker)) { m_CommandContext->database()->startUndoRecord(); }
	}

	void commandCancelled(OdEdCommand* command, OdEdCommandContext* edCommandContext) override {
		undoCmd();
	}

	void commandFailed(OdEdCommand* command, OdEdCommandContext* edCommandContext) override {
		undoCmd();
	}

private:
	void undoCmd() {
		auto Database {m_CommandContext->database()};
		try {
			Database->disableUndoRecording(true);
			Database->undo();
			Database->disableUndoRecording(false);
		} catch (const OdError& Error) {
			theApp.reportError(L"Can't repair database", Error);
		}
#ifndef _DEBUG
		catch (...) {
			::MessageBoxW(nullptr, L"Unknown error occurred...", L"Can't repair database", MB_OK | MB_ICONERROR);
		}
#endif //_DEBUG
	}
};

void AeSysDoc::ExecuteCommand(const OdString& command, const bool echo) {
	OdSaveState<int> save_m_nCmdActive(m_nCmdActive);
	++m_nCmdActive;
	OdDbCommandContextPtr CommandContext(CommandContext0());
	CmdReactor CommandReactor(CommandContext);
	try {
		auto CommandStack {odedRegCmds()};
		auto pExCmdCtx {dynamic_cast<ExDbCommandContext*>(CommandContext.get())};
		if (m_DatabasePtr->appServices()->getPICKFIRST()) {
			pExCmdCtx->setPickfirst(SelectionSet());
		}
		if (command[0] == '(') {
			OdEdLispModulePtr lspMod = odrxDynamicLinker()->loadApp(OdLspModuleName);
			if (!lspMod.isNull()) { lspMod->createLispEngine()->execute(pExCmdCtx, command); }

		} else {
			auto s {command.spanExcluding(L" \t\r\n")};
			if (s.getLength() == command.getLength()) {

				if (echo) { CommandContext->userIO()->putString(CommandPrompt() + L" " + s); }
				s.makeUpper();
				CommandReactor.SetLastInput(s);
				CommandStack->executeCommand(s, CommandContext);
			} else {
				m_pMacro = ExStringIO::create(command);
				while (!m_pMacro->isEof()) {
					try {
						s = CommandContext->userIO()->getString(CommandPrompt());
						s.makeUpper();
						CommandReactor.SetLastInput(s);
					} catch (const OdEdEmptyInput) {
						s = RecentCommandName();
					}
					CommandStack->executeCommand(s, CommandContext);
				}
			}
		}
		if (getViewer()) { getViewer()->propagateActiveViewChanges(); }

	} catch (const OdEdEmptyInput) {
	} catch (const OdEdCancel) {
	} catch (const OdError& Error) {

		if (!m_bConsole) { theApp.reportError(commandMessageCaption(command), Error); }
		BaseIO()->putString(Error.description());
	}
	if (CommandReactor.isDatabaseModified() || SelectionSet()->numEntities()) {

		if (0 != CommandReactor.LastInput().iCompare(L"SELECT") || CommandContext->database()->appServices()->getPICKADD() != 2) { OnEditClearSelection(); }
		UpdateAllViews(nullptr);
	}
}

BOOL AeSysDoc::OnCmdMsg(const unsigned commandId, const int messageCategory, void* commandObject, AFX_CMDHANDLERINFO* handlerInfo) {
	if (handlerInfo == nullptr) {
		auto TopMenu {CMenu::FromHandle(theApp.GetAeSysMenu())};
		if (TopMenu) { // Check if it is a theApp's dynamic menu item
			MENUITEMINFO MenuItemInfo;
			MenuItemInfo.cbSize = sizeof MenuItemInfo;
			MenuItemInfo.fMask = MIIM_DATA;
			if (TopMenu->GetMenuItemInfoW(commandId, &MenuItemInfo, FALSE)) {

				// <tas="Will not use. Need to decide if/how to select a vectorizer. Possible OpenGL is a desired option to Directx">
				if (MenuItemInfo.dwItemData == gsl::narrow_cast<unsigned>(theApp.GetGsMenuItemMarker())) {
					CString Vectorizer;
					TopMenu->GetSubMenu(3)->GetMenuStringW(commandId, Vectorizer, MF_BYCOMMAND);
					if (messageCategory == CN_COMMAND) {
						OnVectorize(static_cast<const wchar_t*>(Vectorizer));
					} else if (messageCategory == CN_UPDATE_COMMAND_UI) {
						static_cast<CCmdUI*>(commandObject)->Enable(m_pViewer == nullptr);
						static_cast<CCmdUI*>(commandObject)->SetCheck(Vectorizer == static_cast<const wchar_t*>(theApp.RecentGsDevicePath()));
					}
					return TRUE;
				}
				// </tas>
				if (commandId >= _APS_NEXT_COMMAND_VALUE + 100 && commandId <= _APS_NEXT_COMMAND_VALUE + theApp.NumberOfCustomCommands() + 100) { // custom commands
					OdRxObjectPtr ItemData(reinterpret_cast<OdRxObject*>(MenuItemInfo.dwItemData));
					if (ItemData.get()) {

						if (messageCategory == CN_COMMAND) {
							auto EdCommand {OdEdCommand::cast(ItemData)};
							if (EdCommand.get()) {
								ExecuteCommand(EdCommand->globalName());
								return TRUE;
							}
						} else if (messageCategory == CN_UPDATE_COMMAND_UI) {
							static_cast<CCmdUI*>(commandObject)->Enable(TRUE);
						}
						return TRUE;
					}
				} else if (commandId >= _APS_NEXT_COMMAND_VALUE && commandId < _APS_NEXT_COMMAND_VALUE + 100) { // annotation scales
					if (messageCategory == CN_COMMAND) {
						const auto SelectedScale {commandId - _APS_NEXT_COMMAND_VALUE - 1};
						auto ScalesCollectionIterator {m_DatabasePtr->objectContextManager()->contextCollection(ODDB_ANNOTATIONSCALES_COLLECTION)->newIterator()};
						for (unsigned ScaleIndex = 0; !ScalesCollectionIterator->done(); ScalesCollectionIterator->next()) {

							if (ScaleIndex++ == SelectedScale) {
								m_DatabasePtr->setCANNOSCALE(OdDbAnnotationScalePtr(ScalesCollectionIterator->getContext()));
								MenuItemInfo.fMask = MIIM_STATE;
								MenuItemInfo.fState = MFS_CHECKED;
								TopMenu->SetMenuItemInfoW(commandId, &MenuItemInfo, FALSE);
								ExecuteCommand(L"REGEN");
								UpdateAllViews(nullptr);
							} else {
								MenuItemInfo.fMask = MIIM_STATE;
								MenuItemInfo.fState = MFS_UNCHECKED;
								TopMenu->SetMenuItemInfoW(_APS_NEXT_COMMAND_VALUE + ScaleIndex, &MenuItemInfo, FALSE);
							}
						}
					} else if (messageCategory == CN_UPDATE_COMMAND_UI) {
						static_cast<CCmdUI*>(commandObject)->Enable(TRUE);
					}
					return TRUE;
				}
			}
		}
	}
	return COleDocument::OnCmdMsg(commandId, messageCategory, commandObject, handlerInfo);
}

void AeSysDoc::DeleteSelection(const bool force) {

	if (m_DatabasePtr->appServices()->getPICKFIRST() && SelectionSet()->numEntities()) {

		if (force) {
			ExecuteCommand(L"ForceErase");
		} else {
			ExecuteCommand(L"erase");
		}
		if (m_pViewer) {
			m_pViewer->editorObject().SetEntityCenters();
		}
	}
}

void AeSysDoc::startDrag(const OdGePoint3d& point) {
	DataSource ds;
	ds.Create(this, point);
	if (ds.DoDragDrop()) {
		UpdateAllViews(nullptr);
	}
}

OdDbTextStyleTableRecordPtr AeSysDoc::AddNewTextStyle(OdString name, OdDbTextStyleTablePtr& textStyles) {
	auto TextStyle {OdDbTextStyleTableRecord::createObject()};
	try {
		TextStyle->setName(name);
		textStyles->add(TextStyle);
	} catch (const OdError& Error) {
		theApp.reportError(L"Error adding new text style...", Error);
		TextStyle->erase();
	}
	return TextStyle;
}

OdDbTextStyleTableRecordPtr AeSysDoc::AddStandardTextStyle() {
	OdDbTextStyleTablePtr TextStyles = m_DatabasePtr->getTextStyleTableId().safeOpenObject(OdDb::kForWrite);
	OdDbTextStyleTableRecordPtr TextStyle;
	if (TextStyles->has(L"EoStandard")) {
		TextStyle = TextStyles->getAt(L"EoStandard").safeOpenObject();
	} else {
		TextStyle = AddNewTextStyle(L"EoStandard", TextStyles);
		TextStyle->setFont(L"Simplex", false, false, DEFAULT_CHARSET, DEFAULT_PITCH);
	}
	return TextStyle;
}

OdDbDimStyleTableRecordPtr AeSysDoc::AddStandardDimensionStyle() {
	OdDbDimStyleTablePtr DimStyleTable = m_DatabasePtr->getDimStyleTableId().safeOpenObject(OdDb::kForWrite);
	if (DimStyleTable->has(L"EoStandard")) {
		OdDbDimStyleTableRecordPtr DimStyle = DimStyleTable->getAt(L"EoStandard").safeOpenObject(OdDb::kForRead);
		return DimStyle;
	}
	auto DimStyle {OdDbDimStyleTableRecord::createObject()};
	DimStyle->setName(L"EoStandard");
	auto dimStyleId {DimStyleTable->add(DimStyle)};
	DimStyle->setDimtxsty(m_DatabasePtr->getTextStyleStandardId());
	DimStyle->setDimse1(true);
	DimStyle->setDimse2(true);
	DimStyle->setDimtad(1); // Dimension text will be placed above the dimension line.
	DimStyle->setDimtih(false);
	DimStyle->setDimtoh(false);
	DimStyle->setDimsah(true);
	OdCmColor Color;
	Color.setColorIndex(1);
	DimStyle->setDimclrd(Color);
	Color.setColorIndex(5);
	DimStyle->setDimclrt(Color); // Text color
	//	DimStyle->setDimtxt(.1); // Text Height
	OdDbTextStyleTablePtr TextStyles = m_DatabasePtr->getTextStyleTableId().safeOpenObject(OdDb::kForRead);
	if (TextStyles->has(L"EoStandard")) {
		const auto TextStyle {TextStyles->getAt(L"EoStandard")};
		DimStyle->setDimtxsty(TextStyle);
	}
	DimStyle->setDimasz(0.0);

	//	DimStyle->setDimblk(L"_None");
	return DimStyle;
}

void AeSysDoc::AddRegisteredApp(const OdString& name) {
	OdDbRegAppTablePtr RegisteredApps = m_DatabasePtr->getRegAppTableId().safeOpenObject(OdDb::kForWrite);
	if (!RegisteredApps->has(name)) {
		auto RegisteredApp {OdDbRegAppTableRecord::createObject()};
		try {
			RegisteredApp->setName(name);
			RegisteredApps->add(RegisteredApp);
		} catch (const OdError& Error) {
			RegisteredApp->erase();
			theApp.reportError(L"ODA Error - AeSysDoc::AddRegisteredApp", Error);
		}
	}
}

BOOL AeSysDoc::OnNewDocument() {
	const auto NumberOfReactors {theApp.m_ApplicationReactors.size()};
	for (unsigned ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
		theApp.m_ApplicationReactors.at(ReactorIndex)->DocumentCreateStarted(this);
	}
	if (COleDocument::OnNewDocument()) {
		OdDbDatabaseDoc::setDocToAssign(this);
		try { // create *database* populated with the default set of objects(all tables, ModelSpace and PaperSpace blocks etc.)
			m_DatabasePtr = theApp.createDatabase(true, OdDb::kEnglish);
		} catch (const OdError& Error) {
			m_DatabasePtr = nullptr;
			theApp.reportError(L"Database Creating Error...", Error);
			return FALSE;
		}
		auto TextStyle {AddStandardTextStyle()};
		m_DatabasePtr->setTEXTSTYLE(TextStyle->objectId());
		AddStandardDimensionStyle();
		AddRegisteredApp(L"AeSys");
		m_DatabasePtr->startUndoRecord();
		const auto DarkPalette {odcmAcadDarkPalette()};
		ODCOLORREF LocalDarkPalette[256];
		memcpy(LocalDarkPalette, DarkPalette, 256 * sizeof(ODCOLORREF));
		EoDbDwgToPegFile DwgToPegFile(m_DatabasePtr);
		DwgToPegFile.ConvertToPeg(this);

		// <tas="Continuous Linetype initialization ??"</tas>
		m_LinetypeTable.LoadLinetypesFromTxtFile(m_DatabasePtr, AeSys::ResourceFolderPath() + L"Pens\\Linetypes.txt");
		m_SaveAsType_ = EoDb::kPeg;
		SetCurrentLayer(m_DatabasePtr->getCLAYER().safeOpenObject());
		InitializeGroupAndPrimitiveEdit();
		if (!m_DatabasePtr.isNull()) {
			m_DatabasePtr->appServices()->layoutManager()->addReactor(this);
		}
		for (unsigned ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
			theApp.m_ApplicationReactors.at(ReactorIndex)->DocumentCreated(this);
		}
		return TRUE;
	}
	for (unsigned ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
		theApp.m_ApplicationReactors.at(ReactorIndex)->DocumentCreateCanceled(this);
	}
	return FALSE;
}

BOOL AeSysDoc::OnOpenDocument(const wchar_t* file) {
	OdDbDatabaseDoc::setDocToAssign(this);
	const auto FileType {AeSys::GetFileType(file)};
	switch (FileType) {
		case EoDb::kDwg: case EoDb::kDxf: {
			m_DatabasePtr = theApp.readFile(file, false, false);

			//<tas="disable lineweight display until lineweight by default is properly defined"</tas>
			if (m_DatabasePtr->getLWDISPLAY()) { m_DatabasePtr->setLWDISPLAY(false); }
			auto TextStyle {AddStandardTextStyle()};
			AddStandardDimensionStyle();
			m_DatabasePtr->startUndoRecord();
			OdString FileAndVersion;
			FileAndVersion.format(L"Opened <%s> (Version: %d)\n", static_cast<const wchar_t*>(m_DatabasePtr->getFilename()), m_DatabasePtr->originalFileVersion());
			theApp.AddStringToMessageList(FileAndVersion);
			EoDbDwgToPegFile DwgToPegFile(m_DatabasePtr);
			DwgToPegFile.ConvertToPeg(this);
			m_SaveAsType_ = FileType;
			SetCurrentLayer(m_DatabasePtr->getCLAYER().safeOpenObject());
			break;
		}
		case EoDb::kPeg: {
			m_DatabasePtr = theApp.createDatabase(true, OdDb::kEnglish);
			auto TextStyle {AddStandardTextStyle()};
			AddStandardDimensionStyle();
			m_DatabasePtr->setTEXTSTYLE(TextStyle->objectId());
			AddRegisteredApp(L"AeSys");
			m_DatabasePtr->startUndoRecord();
			EoDbDwgToPegFile DwgToPegFile(m_DatabasePtr);
			DwgToPegFile.ConvertToPeg(this);
			SetCurrentLayer(m_DatabasePtr->getCLAYER().safeOpenObject());
			EoDbPegFile PegFile(m_DatabasePtr);
			CFileException e;
			if (PegFile.Open(file, CFile::modeRead | CFile::shareDenyNone, &e)) {
				PegFile.Load(this);
				m_SaveAsType_ = EoDb::kPeg;
			}
			break;
		}
		case EoDb::kTracing: case EoDb::kJob:
			TracingOpen(file);
			break;
		case EoDb::kDxb: case EoDb::kUnknown: default:
			return CDocument::OnOpenDocument(file);
	}
	return TRUE;
}

BOOL AeSysDoc::OnSaveDocument(const wchar_t* pathName) {
	auto ReturnStatus {FALSE};
	switch (m_SaveAsType_) {
		case EoDb::kPeg: {
			// <tas="shadow files disabled"/> WriteShadowFile();
			EoDbPegFile DwgToPegFile(m_DatabasePtr);
			CFileException e;
			if (DwgToPegFile.Open(pathName, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive, &e)) {
				DwgToPegFile.Unload(this);
				ReturnStatus = TRUE;
			}
			break;
		}
		case EoDb::kJob: {
			const auto Layer {GetLayerAt(pathName)};
			if (Layer != nullptr) {
				CFile File(pathName, CFile::modeCreate | CFile::modeWrite);
				if (File == CFile::hFileNull) {
					theApp.WarningMessageBox(IDS_MSG_TRACING_WRITE_FAILURE, pathName);
					return FALSE;
				}
				EoDbJobFile JobFile;
				JobFile.WriteHeader(File);
				JobFile.WriteLayer(File, Layer);
				theApp.AddStringToMessageList(IDS_MSG_TRACING_SAVE_SUCCESS, pathName);
				ReturnStatus = TRUE;
			}
			break;
		}
		case EoDb::kTracing: {
			const auto Layer {GetLayerAt(pathName)};
			if (Layer != nullptr) {
				EoDbTracingFile TracingFile(pathName, CFile::modeCreate | CFile::modeWrite);
				if (TracingFile == CFile::hFileNull) {
					theApp.WarningMessageBox(IDS_MSG_TRACING_WRITE_FAILURE, pathName);
					return FALSE;
				}
				TracingFile.WriteHeader();
				TracingFile.WriteLayer(Layer);
				theApp.AddStringToMessageList(IDS_MSG_TRACING_SAVE_SUCCESS, pathName);
				ReturnStatus = TRUE;
			}
			break;
		}
		case EoDb::kDxf:
			m_DatabasePtr->writeFile(pathName, OdDb::kDxf, OdDb::kDHL_PRECURR);
			ReturnStatus = TRUE;
			break;
		case EoDb::kDwg:
			m_DatabasePtr->writeFile(pathName, OdDb::kDwg, OdDb::kDHL_PRECURR);
			ReturnStatus = TRUE;
			break;
		case EoDb::kDxb: case EoDb::kUnknown: default:
			theApp.WarningMessageBox(IDS_MSG_NOTHING_TO_SAVE);
	}
	return ReturnStatus;
}

// AeSysDoc diagnostics
#ifdef _DEBUG
void AeSysDoc::AssertValid() const {
	COleDocument::AssertValid();
}

void AeSysDoc::Dump(CDumpContext& dc) const {
	COleDocument::Dump(dc);
}
#endif //_DEBUG
void AeSysDoc::UpdateGroupInAllViews(const LPARAM hint, EoDbGroup* group) {
	UpdateAllViews(nullptr, hint, group);
}

void AeSysDoc::UpdateGroupsInAllViews(const LPARAM hint, EoDbGroupList* groups) {
	UpdateAllViews(nullptr, hint, groups);
}

void AeSysDoc::UpdateLayerInAllViews(const LPARAM hint, EoDbLayer* layer) {
	UpdateAllViews(nullptr, hint, layer);
}

void AeSysDoc::UpdatePrimitiveInAllViews(const LPARAM hint, EoDbPrimitive* primitive) {
	UpdateAllViews(nullptr, hint, primitive);
}

void AeSysDoc::AddTextBlock(wchar_t* text) {
	const auto CurrentPnt {theApp.GetCursorPosition()};
	auto FontDefinition {g_PrimitiveState.FontDefinition()};
	const auto CharacterCellDefinition {g_PrimitiveState.CharacterCellDefinition()};
	EoGeReferenceSystem ReferenceSystem(CurrentPnt, AeSysView::GetActiveView(), CharacterCellDefinition);
	OdGeVector3d PlaneNormal;
	ReferenceSystem.GetUnitNormal(PlaneNormal);
	OdDbBlockTableRecordPtr BlockTableRecord {m_DatabasePtr->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	wchar_t* NextToken {nullptr};
	auto TextLine {wcstok_s(text, L"\r", &NextToken)};
	while (TextLine != nullptr) {
		if (wcslen(TextLine) > 0) {
			auto Group {new EoDbGroup};
			auto Text {EoDbText::Create(BlockTableRecord, ReferenceSystem.Origin(), TextLine)};
			Text->setNormal(PlaneNormal);
			Text->setRotation(ReferenceSystem.Rotation());
			Text->setHeight(ReferenceSystem.YDirection().length());
			Text->setAlignmentPoint(ReferenceSystem.Origin());
			Text->setHorizontalMode(EoDbText::ConvertHorizontalMode(FontDefinition.HorizontalAlignment()));
			Text->setVerticalMode(EoDbText::ConvertVerticalMode(FontDefinition.VerticalAlignment()));
			Group->AddTail(EoDbText::Create(Text));
			AddWorkLayerGroup(Group);
			UpdateGroupInAllViews(EoDb::kGroup, Group);
		}
		ReferenceSystem.SetOrigin(text_GetNewLinePos(FontDefinition, ReferenceSystem, 1.0, 0));
		TextLine = wcstok_s(nullptr, L"\r", &NextToken);
		if (TextLine == nullptr) { break; }
		TextLine++;
	}
}

POSITION AeSysDoc::DeletedGroupsAddHead(EoDbGroup* group) {
	m_DatabasePtr->disableUndoRecording(false);
	group->Erase();
	m_DatabasePtr->disableUndoRecording(true);
	return m_DeletedGroupList.AddHead(group);
}

POSITION AeSysDoc::DeletedGroupsAddTail(EoDbGroup* group) {
	m_DatabasePtr->disableUndoRecording(false);
	group->Erase();
	m_DatabasePtr->disableUndoRecording(true);
	return m_DeletedGroupList.AddTail(group);
}

EoDbGroup* AeSysDoc::DeletedGroupsRemoveHead() {
	EoDbGroup* Group {nullptr};
	if (!m_DeletedGroupList.IsEmpty()) {
		Group = m_DeletedGroupList.RemoveHead();
		Group->UndoErase();
	}
	return Group;
}

void AeSysDoc::DeletedGroupsRemoveGroups() {
	m_DeletedGroupList.DeleteGroupsAndRemoveAll();
}

EoDbGroup* AeSysDoc::DeletedGroupsRemoveTail() {
	EoDbGroup* Group {nullptr};
	if (!m_DeletedGroupList.IsEmpty()) {
		Group = m_DeletedGroupList.RemoveTail();
		Group->UndoErase();
	}
	return Group;
}

void AeSysDoc::DeletedGroupsRestore() {
	// <tas="UndoErase group is restored to original layer. If this is desired behavior need to revise AddWorkLayerGroup call."</tas>
	if (!m_DeletedGroupList.IsEmpty()) {
		const auto Group {DeletedGroupsRemoveTail()};
		AddWorkLayerGroup(Group);
		UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
	}
}

int AeSysDoc::LinetypeIndexReferenceCount(const short linetypeIndex) {
	auto Count {0};
	for (auto LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		auto Layer {GetLayerAt(LayerIndex)};
		Count += Layer->GetLinetypeIndexRefCount(linetypeIndex);
	}
	CString Key;
	EoDbBlock* Block {nullptr};
	auto Position {m_BlockTable.GetStartPosition()};
	while (Position != nullptr) {
		m_BlockTable.GetNextAssoc(Position, Key, Block);
		Count += Block->GetLinetypeIndexRefCount(linetypeIndex);
	}
	return Count;
}

void AeSysDoc::GetExtents___(AeSysView* view, OdGeExtents3d& extents) {

	for (auto LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		auto Layer {GetLayerAt(LayerIndex)};
		if (!Layer->IsOff()) {
			Layer->GetExtents__(view, extents);
		}
	}
}

int AeSysDoc::NumberOfGroupsInWorkLayer() {
	auto Count {0};
	for (auto LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		const auto Layer {GetLayerAt(LayerIndex)};
		if (Layer->IsCurrent()) {
			Count += Layer->GetCount();
		}
	}
	return Count;
}

int AeSysDoc::NumberOfGroupsInActiveLayers() {
	auto Count {0};
	for (auto LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		const auto Layer {GetLayerAt(LayerIndex)};
		if (Layer->IsActive()) {
			Count += Layer->GetCount();
		}
	}
	return Count;
}

void AeSysDoc::BuildVisibleGroupList(AeSysView* view) {
	RemoveAllGroupsFromAllViews();
	for (auto LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		auto Layer {GetLayerAt(LayerIndex)};
		Layer->BuildVisibleGroupList(view);
	}
}

void AeSysDoc::DisplayAllLayers(AeSysView* view, CDC* deviceContext) {
	try {
		const auto IdentifyTrap {theApp.IsTrapHighlighted() && !IsTrapEmpty()};
		RemoveAllGroupsFromAllViews();
		const auto BackgroundColor {deviceContext->GetBkColor()};
		deviceContext->SetBkColor(g_ViewBackgroundColor);
		const auto PrimitiveState {g_PrimitiveState.Save()};
		for (auto LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
			auto Layer {GetLayerAt(LayerIndex)};
			Layer->Display_(view, deviceContext, IdentifyTrap);
		}
		g_PrimitiveState.Restore(*deviceContext, PrimitiveState);
		deviceContext->SetBkColor(BackgroundColor);
	} catch (CException* Exception) {
		Exception->Delete();
	}
}

OdDbObjectId AeSysDoc::AddLayerTo(OdDbLayerTablePtr layers, EoDbLayer* layer) {
	m_LayerTable.Add(layer);
	return layers->add(layer->TableRecord());
}

void AeSysDoc::AddLayer(EoDbLayer* layer) {
	m_LayerTable.Add(layer);
}

int AeSysDoc::GetLayerTableSize() const {
	return m_LayerTable.GetSize();
}

EoDbLayer* AeSysDoc::GetLayerAt(const OdString& name) {
	const auto i {FindLayerAt(name)};
	return i < 0 ? static_cast<EoDbLayer*>(nullptr) : m_LayerTable.GetAt(i);
}

EoDbLayer* AeSysDoc::GetLayerAt(const int layerIndex) {
	return layerIndex >= static_cast<int>(m_LayerTable.GetSize()) ? nullptr : m_LayerTable.GetAt(layerIndex);
}

int AeSysDoc::FindLayerAt(const OdString& name) const {
	for (unsigned short LayerIndex = 0; LayerIndex < m_LayerTable.GetSize(); LayerIndex++) {
		const auto Layer {m_LayerTable.GetAt(LayerIndex)};
		if (name.iCompare(Layer->Name()) == 0) { return LayerIndex; }
	}
	return -1;
}

OdDbLayerTablePtr AeSysDoc::LayerTable(const OdDb::OpenMode openMode) {
	return m_DatabasePtr->getLayerTableId().safeOpenObject(openMode);
}

void AeSysDoc::RemoveAllLayers() {
	for (unsigned short LayerIndex = 0; LayerIndex < m_LayerTable.GetSize(); LayerIndex++) {
		auto Layer {m_LayerTable.GetAt(LayerIndex)};
		if (Layer) {
			Layer->DeleteGroupsAndRemoveAll();
			delete Layer;
		}
	}
	m_LayerTable.RemoveAll();
}

void AeSysDoc::RemoveLayerAt(const int layerIndex) {
	auto Layer {GetLayerAt(layerIndex)};
	Layer->DeleteGroupsAndRemoveAll();
	delete Layer;
	m_LayerTable.RemoveAt(layerIndex);
}

void AeSysDoc::RemoveEmptyLayers() {
	for (auto LayerIndex = GetLayerTableSize() - 1; LayerIndex > 0; LayerIndex--) {
		auto Layer {GetLayerAt(LayerIndex)};
		if (Layer && Layer->IsEmpty()) {
			Layer->DeleteGroupsAndRemoveAll();
			delete Layer;
			m_LayerTable.RemoveAt(LayerIndex);
		}
	}
}

bool AeSysDoc::LayerMelt(OdString& name) {
	auto Layer {GetLayerAt(name)};
	if (Layer == nullptr) { return false; }
	auto bRetVal {false};
	OPENFILENAME OpenFileName;
	::ZeroMemory(&OpenFileName, sizeof(OPENFILENAME));
	OpenFileName.lStructSize = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner = nullptr;
	OpenFileName.hInstance = theApp.GetInstance();
	OpenFileName.lpstrFilter = L"Tracing Files\0*.tra;*.jb1\0\0";
	OpenFileName.lpstrFile = new wchar_t[MAX_PATH];
	wcscpy_s(OpenFileName.lpstrFile, MAX_PATH, name);
	OpenFileName.nMaxFile = MAX_PATH;
	OpenFileName.lpstrTitle = L"Melt As";
	OpenFileName.Flags = OFN_OVERWRITEPROMPT;
	OpenFileName.lpstrDefExt = L"tra";
	if (GetSaveFileNameW(&OpenFileName)) {
		name = OpenFileName.lpstrFile;
		const auto FileType {AeSys::GetFileType(name)};
		if (FileType == EoDb::kTracing || FileType == EoDb::kJob) {
			if (FileType == EoDb::kJob) {
				CFile File(name, CFile::modeWrite | CFile::modeCreate);
				if (File == CFile::hFileNull) {
					theApp.WarningMessageBox(IDS_MSG_TRACING_WRITE_FAILURE, name);
					return false;
				}
				EoDbJobFile JobFile;
				JobFile.WriteHeader(File);
				JobFile.WriteLayer(File, Layer);
			} else {
				EoDbTracingFile TracingFile(name, CFile::modeWrite | CFile::modeCreate);
				if (TracingFile == CFile::hFileNull) {
					theApp.WarningMessageBox(IDS_MSG_TRACING_WRITE_FAILURE, name);
					return false;
				}
				TracingFile.WriteHeader();
				TracingFile.WriteLayer(Layer);
			}
			name = name.mid(OpenFileName.nFileOffset);
			Layer->MakeResident(true);
			Layer->MakeInternal(false);
			Layer->SetIsLocked(true);
			Layer->SetName(name);
			bRetVal = true;
		}
	}
	delete[] OpenFileName.lpstrFile;
	return bRetVal;
}

void AeSysDoc::PenTranslation(const unsigned numberOfColors, std::vector<int>& newColors, std::vector<int>& pCol) {
	for (auto LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		auto Layer {GetLayerAt(LayerIndex)};
		Layer->PenTranslation(numberOfColors, newColors, pCol);
	}
}

EoDbLayer* AeSysDoc::SelectLayerBy(const OdGePoint3d& point) {
	const auto Group {AeSysView::GetActiveView()->SelectGroupAndPrimitive(point)};
	if (Group != nullptr) {
		for (auto LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
			const auto Layer {GetLayerAt(LayerIndex)};
			if (Layer->Find(Group)) { return Layer; }
		}
	}
	for (auto LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		auto Layer {GetLayerAt(LayerIndex)};
		if (Layer->SelectGroupBy(point) != nullptr) { return Layer; }
	}
	return nullptr;
}

void AeSysDoc::PurgeDuplicateObjects() {
	for (auto LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		auto Layer {GetLayerAt(LayerIndex)};
		Layer->RemoveDuplicatePrimitives();
	}
}

int AeSysDoc::RemoveEmptyNotesAndDelete() {
	auto Count {0};
	for (auto LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		auto Layer {GetLayerAt(LayerIndex)};
		Count += Layer->RemoveEmptyNotesAndDelete();
	}

	//Note: remove empty notes from blocks
	CString Key;
	EoDbBlock* Block {nullptr};
	auto Position {m_BlockTable.GetStartPosition()};
	while (Position != nullptr) {
		m_BlockTable.GetNextAssoc(Position, Key, Block);
	}
	return Count;
}

int AeSysDoc::RemoveEmptyGroups() {
	auto Count {0};
	for (auto LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		auto Layer {GetLayerAt(LayerIndex)};
		Count += Layer->RemoveEmptyGroups();
	}

	//Note: remove empty groups from blocks
	CString Key;
	EoDbBlock* Block {nullptr};
	auto Position {m_BlockTable.GetStartPosition()};
	while (Position != nullptr) {
		m_BlockTable.GetNextAssoc(Position, Key, Block);
	}
	return Count;
}
// Work Layer interface
void AeSysDoc::AddWorkLayerGroup(EoDbGroup* group) {
	m_WorkLayer->AddTail(group);
	AddGroupToAllViews(group);
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::kWorkCount);
	SetModifiedFlag(TRUE);
}

void AeSysDoc::AddWorkLayerGroups(EoDbGroupList* groups) {
	m_WorkLayer->AddTail(groups);
	AddGroupsToAllViews(groups);
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::kWorkCount);
	SetModifiedFlag(TRUE);
}

POSITION AeSysDoc::FindWorkLayerGroup(EoDbGroup* group) const {
	return m_WorkLayer->Find(group);
}

POSITION AeSysDoc::GetFirstWorkLayerGroupPosition() const {
	return m_WorkLayer->GetHeadPosition();
}

EoDbGroup* AeSysDoc::GetLastWorkLayerGroup() const {
	auto Position {m_WorkLayer->GetTailPosition()};
	return static_cast<EoDbGroup*>(Position != nullptr ? m_WorkLayer->GetPrev(Position) : nullptr);
}

POSITION AeSysDoc::GetLastWorkLayerGroupPosition() const {
	return m_WorkLayer->GetTailPosition();
}

EoDbGroup* AeSysDoc::GetNextWorkLayerGroup(POSITION& position) const {
	return m_WorkLayer->GetNext(position);
}

EoDbGroup* AeSysDoc::GetPreviousWorkLayerGroup(POSITION& position) const {
	return m_WorkLayer->GetPrev(position);
}

EoDbLayer* AeSysDoc::GetWorkLayer() const noexcept {
	return m_WorkLayer;
}

void AeSysDoc::InitializeWorkLayer() {
	m_WorkLayer->DeleteGroupsAndRemoveAll();
	RemoveAllTrappedGroups();
	RemoveAllGroupsFromAllViews();
	ResetAllViews();
	m_DeletedGroupList.DeleteGroupsAndRemoveAll();
}

OdDbObjectId AeSysDoc::SetCurrentLayer(OdDbLayerTableRecordPtr layerTableRecord) {
	auto PreviousLayer {m_DatabasePtr->getCLAYER()};
	m_DatabasePtr->setCLAYER(layerTableRecord->objectId());
	m_WorkLayer = GetLayerAt(layerTableRecord->getName());
	m_WorkLayer->MakeCurrent();
	return PreviousLayer;
}

// Locates the layer containing a group and removes it.
// The group itself is not deleted.
EoDbLayer* AeSysDoc::AnyLayerRemove(EoDbGroup* group) {
	for (auto LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		auto Layer {GetLayerAt(LayerIndex)};
		if (Layer->IsCurrent() || Layer->IsActive()) {

			if (Layer->Remove(group) != nullptr) {
				AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::kWorkCount);
				SetModifiedFlag(TRUE);
				return Layer;
			}
		}
	}
	return nullptr;
}

void AeSysDoc::TracingFuse(OdString& nameAndLocation) {
	auto Layer {GetLayerAt(nameAndLocation)};
	if (Layer != nullptr) {
		const auto Title {new wchar_t[MAX_PATH]};
		GetFileTitle(nameAndLocation, Title, MAX_PATH);
		wchar_t* NextToken {nullptr};
		wcstok_s(Title, L".", &NextToken);
		nameAndLocation = Title;
		delete[] Title;
		Layer->MakeResident(true);
		Layer->MakeInternal(true);
		Layer->SetIsLocked(true);
		Layer->SetName(nameAndLocation);
	}
}

bool AeSysDoc::TracingLoadLayer(const OdString& file, EoDbLayer* layer) {
	if (!layer) return false;
	const auto FileType {AeSys::GetFileType(file)};
	if (FileType != EoDb::kTracing && FileType != EoDb::kJob) {
		return false;
	}
	const auto FileOpen {false};
	if (FileType == EoDb::kTracing) {
		CFileException e;
		EoDbTracingFile TracingFile(file, CFile::modeRead | CFile::shareDenyNone);
		if (TracingFile != CFile::hFileNull) {

			m_SaveAsType_ = EoDb::kTracing;
			SetCurrentLayer(m_DatabasePtr->getCLAYER().safeOpenObject());
			TracingFile.ReadHeader();
			TracingFile.ReadLayer(m_DatabasePtr->getModelSpaceId().safeOpenObject(OdDb::kForWrite), layer);
			return true;
		}
	} else {
		CFile File(file, CFile::modeRead | CFile::shareDenyNone);
		if (File != nullptr) {
			EoDbJobFile JobFile;
			JobFile.ReadHeader(File);
			JobFile.ReadLayer(m_DatabasePtr->getModelSpaceId().safeOpenObject(OdDb::kForWrite), File, layer);
			return true;
		}
		theApp.WarningMessageBox(IDS_MSG_TRACING_OPEN_FAILURE, file);
	}
	return FileOpen;
}

bool AeSysDoc::TracingOpen(const OdString& fileName) {
	m_DatabasePtr = theApp.createDatabase(true, OdDb::kEnglish);
	auto TextStyle {AddStandardTextStyle()};
	m_DatabasePtr->setTEXTSTYLE(TextStyle->objectId());
	AddRegisteredApp(L"AeSys");
	m_DatabasePtr->startUndoRecord();
	EoDbDwgToPegFile DwgToPegFile(m_DatabasePtr);
	DwgToPegFile.ConvertToPeg(this);
	OdDbLayerTableRecordPtr LayerTableRecord = m_DatabasePtr->getCLAYER().safeOpenObject(OdDb::kForWrite);
	SetCurrentLayer(LayerTableRecord);
	LayerTableRecord->setIsReconciled(true);
	auto Layer {GetLayerAt(0)};
	Layer->MakeResident(false);
	TracingLoadLayer(fileName, Layer);
	AddGroupsToAllViews(Layer);
	Layer->MakeCurrent();
	m_SaveAsType_ = EoDb::kTracing;
	UpdateAllViews(nullptr);
	return true;
}

void AeSysDoc::WriteShadowFile() {
	if (m_SaveAsType_ == EoDb::kPeg) {
		auto ShadowFilePath(theApp.ShadowFolderPath());
		ShadowFilePath += GetTitle();
		const auto nExt {ShadowFilePath.Find('.')};
		if (nExt > 0) {
			CFileStatus fs;
			CFile::GetStatus(GetPathName(), fs);
			ShadowFilePath.Truncate(nExt);
			ShadowFilePath += fs.m_mtime.Format(L"_%Y%m%d%H%M");
			ShadowFilePath += L".peg";
			CFileException e;
			EoDbPegFile PegFile(m_DatabasePtr);
			if (!PegFile.Open(ShadowFilePath, CFile::modeWrite, &e)) {
				PegFile.Open(ShadowFilePath, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive, &e);
				PegFile.Unload(this);
				theApp.WarningMessageBox(IDS_MSG_FILE_SHADOWED_AS, ShadowFilePath);
				return;
			}
			theApp.WarningMessageBox(IDS_MSG_SHADOW_FILE_CREATE_FAILURE);
		}
	}
}
// AeSysDoc commands
void AeSysDoc::OnClearActiveLayers() {
	InitializeGroupAndPrimitiveEdit();
	for (auto LayerIndex = GetLayerTableSize() - 1; LayerIndex > 0; LayerIndex--) {
		auto Layer {GetLayerAt(LayerIndex)};
		if (Layer->IsActive()) {
			UpdateLayerInAllViews(EoDb::kLayerErase, Layer);
			Layer->DeleteGroupsAndRemoveAll();
		}
	}
}

void AeSysDoc::OnClearAllLayers() {
	InitializeGroupAndPrimitiveEdit();
	for (auto LayerIndex = GetLayerTableSize() - 1; LayerIndex > 0; LayerIndex--) {
		auto Layer {GetLayerAt(LayerIndex)};
		if (Layer->IsInternal()) {
			UpdateLayerInAllViews(EoDb::kLayerErase, Layer);
			Layer->DeleteGroupsAndRemoveAll();
		}
	}
	UpdateAllViews(nullptr);
}

void AeSysDoc::OnClearWorkingLayer() {
	InitializeGroupAndPrimitiveEdit();
	InitializeWorkLayer();
}

void AeSysDoc::OnClearAllTracings() {
	InitializeGroupAndPrimitiveEdit();
	for (auto LayerIndex = GetLayerTableSize() - 1; LayerIndex > 0; LayerIndex--) {
		auto Layer {GetLayerAt(LayerIndex)};
		if (!Layer->IsInternal()) {
			UpdateLayerInAllViews(EoDb::kLayerErase, Layer);
			Layer->DeleteGroupsAndRemoveAll();
		}
	}
	UpdateAllViews(nullptr);
}

void AeSysDoc::OnClearMappedTracings() {
	InitializeGroupAndPrimitiveEdit();
	for (auto LayerIndex = GetLayerTableSize() - 1; LayerIndex > 0; LayerIndex--) {
		auto Layer {GetLayerAt(LayerIndex)};
		if (Layer->IsActive()) {
			UpdateLayerInAllViews(EoDb::kLayerErase, Layer);
			if (Layer->IsResident()) {
				Layer->SetIsOff(true);
			} else {
				RemoveLayerAt(LayerIndex);
			}
		}
	}
}

void AeSysDoc::OnClearViewedTracings() {
	InitializeGroupAndPrimitiveEdit();
	for (auto LayerIndex = GetLayerTableSize() - 1; LayerIndex > 0; LayerIndex--) {
		auto Layer {GetLayerAt(LayerIndex)};
		if (Layer->IsLocked()) {
			UpdateLayerInAllViews(EoDb::kLayerErase, Layer);
			if (Layer->IsResident()) {
				Layer->SetIsOff(true);
			} else {
				RemoveLayerAt(LayerIndex);
			}
		}
	}
}

void AeSysDoc::OnPrimBreak() {
	auto ActiveView {AeSysView::GetActiveView()};
	auto Database {ActiveView->Database()};
	const OdDbBlockTableRecordPtr BlockTableRecord {Database->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	auto Group {ActiveView->SelectGroupAndPrimitive(ActiveView->GetCursorPosition())};
	if (Group != nullptr && ActiveView->EngagedPrimitive() != nullptr) {
		const auto Primitive {ActiveView->EngagedPrimitive()};
		if (Primitive->IsKindOf(RUNTIME_CLASS(EoDbPolyline))) {
			const auto PolylinePrimitive {dynamic_cast<EoDbPolyline*>(Primitive)};
			Group->FindAndRemovePrimitive(Primitive);
			OdGePoint3dArray Points;
			PolylinePrimitive->GetAllPoints(Points);
			OdDbLinePtr Line;
			for (unsigned w = 0; w < Points.size() - 1; w++) {
				Line = EoDbLine::Create(BlockTableRecord, Points[w], Points[w + 1]);
				Line->setColorIndex(static_cast<unsigned short>(Primitive->ColorIndex()));
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(Primitive->LinetypeIndex()));
				Group->AddTail(EoDbLine::Create(Line));
			}
			if (PolylinePrimitive->IsClosed()) {
				Line = EoDbLine::Create(BlockTableRecord, Points[Points.size() - 1], Points[0]);
				Line->setColorIndex(static_cast<unsigned short>(Primitive->ColorIndex()));
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(Primitive->LinetypeIndex()));
				Group->AddTail(EoDbLine::Create(Line));
			}
			delete Primitive;
			ResetAllViews();
		} else if (Primitive->IsKindOf(RUNTIME_CLASS(EoDbBlockReference))) {
			const auto BlockReference {dynamic_cast<EoDbBlockReference*>(Primitive)};
			EoDbBlock* Block;
			if (LookupBlock(BlockReference->Name(), Block) != 0) {
				Group->FindAndRemovePrimitive(Primitive);
				const auto BlockTransform {BlockReference->BlockTransformMatrix(Block->BasePoint())};
				auto pSegT {new EoDbGroup(*Block)};
				pSegT->TransformBy(BlockTransform);
				Group->AddTail(pSegT);
				delete Primitive;
				ResetAllViews();
			}
		}
	}
}

void AeSysDoc::OnEditSegToWork() {
	const auto Point {theApp.GetCursorPosition()};
	auto Layer {SelectLayerBy(Point)};
	if (Layer != nullptr) {
		if (Layer->IsInternal()) {
			const auto Group {Layer->SelectGroupBy(Point)};
			if (Group != nullptr) {
				Layer->Remove(Group);
				UpdateGroupInAllViews(EoDb::kGroupEraseSafe, Group);
				AddWorkLayerGroup(Group);
				UpdateGroupInAllViews(EoDb::kGroup, Group);
			}
		}
	}
}

void AeSysDoc::OnFileQuery() {
	const auto pt {theApp.GetCursorPosition()};
	const auto Layer {SelectLayerBy(pt)};
	if (Layer != nullptr) {
		CPoint CurrentPosition;
		GetCursorPos(&CurrentPosition);
		m_IdentifiedLayerName = Layer->Name();
		const auto MenuResource {Layer->IsInternal() ? IDR_LAYER : IDR_TRACING};
		const auto LayerTracingMenu {LoadMenuW(theApp.GetInstance(), MAKEINTRESOURCEW(MenuResource))};
		auto SubMenu {CMenu::FromHandle(GetSubMenu(LayerTracingMenu, 0))};
		SubMenu->ModifyMenu(0, MF_BYPOSITION | MF_STRING, 0, m_IdentifiedLayerName);
		if (MenuResource == IDR_LAYER) {
			SubMenu->CheckMenuItem(ID_LAYER_CURRENT, static_cast<unsigned>(MF_BYCOMMAND | Layer->IsCurrent() ? MF_CHECKED : MF_UNCHECKED));
			SubMenu->CheckMenuItem(ID_LAYER_ACTIVE, static_cast<unsigned>(MF_BYCOMMAND | Layer->IsActive() ? MF_CHECKED : MF_UNCHECKED));
			SubMenu->CheckMenuItem(ID_LAYER_LOCK, static_cast<unsigned>(MF_BYCOMMAND | Layer->IsLocked() ? MF_CHECKED : MF_UNCHECKED));
			SubMenu->CheckMenuItem(ID_LAYER_OFF, static_cast<unsigned>(MF_BYCOMMAND | Layer->IsOff() ? MF_CHECKED : MF_UNCHECKED));
		} else {
			SubMenu->CheckMenuItem(ID_TRACING_CURRENT, static_cast<unsigned>(MF_BYCOMMAND | Layer->IsCurrent() ? MF_CHECKED : MF_UNCHECKED));
			SubMenu->CheckMenuItem(ID_TRACING_ACTIVE, static_cast<unsigned>(MF_BYCOMMAND | Layer->IsActive() ? MF_CHECKED : MF_UNCHECKED));
			SubMenu->CheckMenuItem(ID_TRACING_LOCK, static_cast<unsigned>(MF_BYCOMMAND | Layer->IsLocked() ? MF_CHECKED : MF_UNCHECKED));
			SubMenu->CheckMenuItem(ID_TRACING_OFF, static_cast<unsigned>(MF_BYCOMMAND | Layer->IsOff() ? MF_CHECKED : MF_UNCHECKED));
		}
		SubMenu->TrackPopupMenuEx(0, CurrentPosition.x, CurrentPosition.y, AfxGetMainWnd(), nullptr);
		DestroyMenu(LayerTracingMenu);
	}
}

void AeSysDoc::OnLayerActive() {
	auto Layer {GetLayerAt(m_IdentifiedLayerName)};
	if (Layer == nullptr) {
	} else {
		if (Layer->IsCurrent()) {
			theApp.WarningMessageBox(IDS_MSG_LAYER_NO_ACTIVE, m_IdentifiedLayerName);
		} else {
			Layer->MakeActive();
			UpdateLayerInAllViews(EoDb::kLayerSafe, Layer);
		}
	}
}

void AeSysDoc::OnLayerLock() {
	auto Layer {GetLayerAt(m_IdentifiedLayerName)};
	if (Layer != nullptr) {

		if (Layer->IsCurrent()) {
			theApp.WarningMessageBox(IDS_MSG_LAYER_NO_STATIC, m_IdentifiedLayerName);
		} else {
			Layer->SetIsLocked(true);
			UpdateLayerInAllViews(EoDb::kLayerSafe, Layer);
		}
	}
}

void AeSysDoc::OnLayerOff() {
	auto Layer {GetLayerAt(m_IdentifiedLayerName)};
	if (Layer != nullptr) {

		if (Layer->IsCurrent()) {
			theApp.WarningMessageBox(IDS_MSG_LAYER_NO_HIDDEN, m_IdentifiedLayerName);
		} else {
			UpdateLayerInAllViews(EoDb::kLayerErase, Layer);
			Layer->SetIsOff(true);
		}
	}
}

void AeSysDoc::OnLayerMelt() {
	LayerMelt(m_IdentifiedLayerName);
	theApp.AddStringToMessageList(IDS_MSG_LAYER_CONVERTED_TO_TRACING, m_IdentifiedLayerName);
}

void AeSysDoc::OnLayerCurrent() {
	auto Layers {LayerTable(OdDb::kForRead)};
	SetCurrentLayer(Layers->getAt(m_IdentifiedLayerName).safeOpenObject(OdDb::kForRead));
}

void AeSysDoc::OnTracingActive() {
	auto Layer {GetLayerAt(m_IdentifiedLayerName)};
	if (Layer->IsCurrent()) {
		theApp.WarningMessageBox(IDS_MSG_CLOSE_TRACING_FIRST, m_IdentifiedLayerName);
	} else {
		Layer->MakeActive();
		UpdateLayerInAllViews(EoDb::kLayerSafe, Layer);
	}
}

void AeSysDoc::OnTracingCurrent() {
	auto Layer {GetLayerAt(m_IdentifiedLayerName)};
	Layer->MakeCurrent();
	UpdateLayerInAllViews(EoDb::kLayerSafe, Layer);
}

void AeSysDoc::OnTracingFuse() {
	TracingFuse(m_IdentifiedLayerName);
	theApp.AddStringToMessageList(IDS_MSG_TRACING_CONVERTED_TO_LAYER, m_IdentifiedLayerName);
}

void AeSysDoc::OnTracingLock() {
	auto Layer {GetLayerAt(m_IdentifiedLayerName)};
	if (Layer->IsCurrent()) {
		theApp.WarningMessageBox(IDS_MSG_CLOSE_TRACING_FIRST, m_IdentifiedLayerName);
	} else {
		Layer->SetIsLocked(true);
		UpdateLayerInAllViews(EoDb::kLayerSafe, Layer);
	}
}

void AeSysDoc::OnTracingOff() {
	auto Layer {GetLayerAt(m_IdentifiedLayerName)};
	if (Layer->IsCurrent()) {
		CFile File(m_IdentifiedLayerName, CFile::modeWrite | CFile::modeCreate);
		if (File != CFile::hFileNull) {
			EoDbJobFile JobFile;
			JobFile.WriteHeader(File);
			JobFile.WriteLayer(File, Layer);
			UpdateLayerInAllViews(EoDb::kLayerErase, Layer);
			Layer->SetIsOff(true);
			// <tas="If a single layer tracing if being set off no layers will be visible. Need to correct."</tas>
			Layer = GetLayerAt(0);
			SetCurrentLayer(Layer->TableRecord());
			m_SaveAsType_ = EoDb::kUnknown;
		} else {
			theApp.WarningMessageBox(IDS_MSG_TRACING_WRITE_FAILURE, m_IdentifiedLayerName);
		}
	}
}

void AeSysDoc::OnLayersSetAllActive() {
	for (auto LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		auto Layer {GetLayerAt(LayerIndex)};
		if (!Layer->IsCurrent()) { Layer->MakeActive(); }
	}
	UpdateAllViews(nullptr);
}

void AeSysDoc::OnLayersSetAllLocked() {
	for (auto LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		auto Layer {GetLayerAt(LayerIndex)};
		if (!Layer->IsCurrent()) { Layer->SetIsLocked(true); }
	}
	UpdateAllViews(nullptr);
}

void AeSysDoc::OnPurgeUnusedLayers() {
	RemoveEmptyLayers();
}

void AeSysDoc::OnToolsGroupRestore() {
	DeletedGroupsRestore();
}

void AeSysDoc::OnPensRemoveUnusedLinetypes() {
	OdDbLinetypeTablePtr Linetypes {m_DatabasePtr->getLinetypeTableId().safeOpenObject(OdDb::kForWrite)};
	auto Iterator {Linetypes->newIterator()};
	for (Iterator->start(); !Iterator->done(); Iterator->step()) {
		OdDbLinetypeTableRecordPtr Linetype {Iterator->getRecordId().safeOpenObject(OdDb::kForWrite)};
		auto Name {Linetype->getName()};
		if (LinetypeIndexReferenceCount(static_cast<short>(EoDbLinetypeTable::LegacyLinetypeIndex(Name))) == 0) {
			const auto Result {Linetype->erase(true)};
			if (Result) {
				auto ErrorDescription {m_DatabasePtr->appServices()->getErrorDescription(Result)};
				ErrorDescription += L" <%s> linetype can not be deleted";
				theApp.AddStringToMessageList(ErrorDescription, Name);
			} else {
				theApp.AddStringToMessageList(IDS_MSG_UNUSED_LINETYPE_REMOVED, Name);
			}
		}
	}
}

void AeSysDoc::OnPurgeUnreferencedBlocks() {
	PurgeUnreferencedBlocks();
}

void AeSysDoc::OnEditImageToClipboard() {
	const auto MetaFile {CreateEnhMetaFileW(nullptr, nullptr, nullptr, nullptr)};
	DisplayAllLayers(AeSysView::GetActiveView(), CDC::FromHandle(MetaFile));
	const auto MetaFileHandle {CloseEnhMetaFile(MetaFile)};
	OpenClipboard(nullptr);
	EmptyClipboard();
	SetClipboardData(CF_ENHMETAFILE, MetaFileHandle);
	CloseClipboard();
}

void AeSysDoc::OnEditTrace() {
	if (OpenClipboard(nullptr)) {
		wchar_t sBuf[16] {0};
		unsigned ClipboardFormat;
		unsigned Format {0};
		while ((ClipboardFormat = EnumClipboardFormats(Format)) != 0) {
			GetClipboardFormatNameW(ClipboardFormat, sBuf, 16);
			if (wcscmp(sBuf, L"EoGroups") == 0) {
				const auto ClipboardDataHandle {GetClipboardData(ClipboardFormat)};
				if (ClipboardDataHandle != nullptr) {

					const auto ClipboardData {static_cast<const char*>(GlobalLock(ClipboardDataHandle))};
					if (ClipboardData != nullptr) {
						const auto ClipboardDataLength {*(unsigned long*)ClipboardData};
						CMemFile MemFile;
						MemFile.Write(ClipboardData, gsl::narrow_cast<unsigned>(ClipboardDataLength));
						GlobalUnlock(ClipboardDataHandle);
						MemFile.Seek(96, CFile::begin);
						EoDbJobFile JobFile;
						JobFile.ReadMemFile(m_DatabasePtr->getModelSpaceId().safeOpenObject(OdDb::kForWrite), MemFile);
					}
					break;
				}
			}
			Format = ClipboardFormat;
		}
		CloseClipboard();
	} else {
		theApp.WarningMessageBox(IDS_MSG_CLIPBOARD_LOCKED);
	}
}

void AeSysDoc::OnEditTrapDelete() {
	DeleteAllTrappedGroups();
	UpdateAllViews(nullptr);
	OnEditTrapQuit();
}

void AeSysDoc::OnEditTrapQuit() {
	RemoveAllTrappedGroups();
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::kTrapCount);
}

void AeSysDoc::OnEditTrapCopy() {
	CopyTrappedGroupsToClipboard(AeSysView::GetActiveView());
}

void AeSysDoc::OnEditTrapCut() {
	CopyTrappedGroupsToClipboard(AeSysView::GetActiveView());
	DeleteAllTrappedGroups();
	UpdateAllViews(nullptr);
}

void AeSysDoc::OnEditTrapPaste() {
	if (OpenClipboard(nullptr)) {
		const auto ClipboardFormat {theApp.ClipboardFormatIdentifierForEoGroups()};
		if (IsClipboardFormatAvailable(ClipboardFormat)) {
			EoDlgSetPastePosition Dialog;
			if (Dialog.DoModal() == IDOK) {
				const auto ClipboardDataHandle {GetClipboardData(ClipboardFormat)};
				if (ClipboardDataHandle != nullptr) {
					OdGePoint3d LowerLeftExtent;
					const auto InsertionPoint {theApp.GetCursorPosition()};
					SetTrapPivotPoint(InsertionPoint);
					const auto ClipboardData {static_cast<const char*>(GlobalLock(ClipboardDataHandle))};
					const auto ClipboardDataLength {*(unsigned long*)ClipboardData};
					CMemFile MemoryFile;
					MemoryFile.Write(ClipboardData, gsl::narrow_cast<unsigned>(ClipboardDataLength));
					MemoryFile.Seek(sizeof(unsigned long), CFile::begin);
					MemoryFile.Read(&LowerLeftExtent.x, sizeof(double));
					MemoryFile.Read(&LowerLeftExtent.y, sizeof(double));
					MemoryFile.Read(&LowerLeftExtent.z, sizeof(double));
					GlobalUnlock(ClipboardDataHandle);
					MemoryFile.Seek(96, CFile::begin);
					EoDbJobFile JobFile;
					JobFile.ReadMemFile(m_DatabasePtr->getModelSpaceId().safeOpenObject(OdDb::kForWrite), MemoryFile);
					EoGeMatrix3d TransformMatrix;
					TransformMatrix.setToTranslation(InsertionPoint - LowerLeftExtent);
					TransformTrappedGroups(TransformMatrix);
				}
			}
		} else if (IsClipboardFormatAvailable(CF_TEXT)) {
			const auto ClipboardDataHandle {GetClipboardData(CF_TEXT)};
			if (ClipboardDataHandle != nullptr) {
				const auto ClipboardData {static_cast<char*>(GlobalLock(ClipboardDataHandle))};
				if (ClipboardData != nullptr) {
					const unsigned ClipboardDataSize {GlobalSize(ClipboardDataHandle)};
					const auto Text {new wchar_t[ClipboardDataSize]};
					for (unsigned i = 0; i < ClipboardDataSize; i++) {
						Text[i] = static_cast<wchar_t>(ClipboardData[i]);
					}
					GlobalUnlock(ClipboardDataHandle);
					AddTextBlock(Text);
					delete[] Text;
				}
			}
		} else if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
			const auto ClipboardDataHandle {GetClipboardData(CF_UNICODETEXT)};
			const auto ClipboardData {static_cast<wchar_t*>(GlobalLock(ClipboardDataHandle))};
			const unsigned ClipboardDataSize {GlobalSize(ClipboardDataHandle)};
			const auto Text {new wchar_t[ClipboardDataSize]};
			for (unsigned i = 0; i < ClipboardDataSize; i++) {
				Text[i] = ClipboardData[i];
			}
			GlobalUnlock(ClipboardDataHandle);
			AddTextBlock(Text);
			delete[] Text;
		}
		CloseClipboard();
	} else {
		theApp.WarningMessageBox(IDS_MSG_CLIPBOARD_LOCKED);
	}
}

void AeSysDoc::OnEditTrapWork() {
	RemoveAllTrappedGroups();
	AddGroupsToTrap(GetWorkLayer());
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::kTrapCount);
}

void AeSysDoc::OnEditTrapWorkAndActive() {
	RemoveAllTrappedGroups();
	for (auto LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		const auto Layer {GetLayerAt(LayerIndex)};
		if (Layer->IsCurrent() || Layer->IsActive()) { AddGroupsToTrap(Layer); }

	}
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::kTrapCount);
}

void AeSysDoc::OnTrapCommandsCompress() {
	CompressTrappedGroups();
}

void AeSysDoc::OnTrapCommandsExpand() {
	ExpandTrappedGroups();
}

void AeSysDoc::OnTrapCommandsInvert() {
	const auto NumberOfLayers {GetLayerTableSize()};
	for (auto LayerIndex = 0; LayerIndex < NumberOfLayers; LayerIndex++) {
		auto Layer {GetLayerAt(LayerIndex)};
		if (Layer->IsCurrent() || Layer->IsActive()) {
			auto LayerPosition {Layer->GetHeadPosition()};
			while (LayerPosition != nullptr) {
				const auto Group {Layer->GetNext(LayerPosition)};
				const auto GroupPosition {FindTrappedGroup(Group)};
				if (GroupPosition != nullptr) {
					m_TrappedGroupList.RemoveAt(GroupPosition);
				} else {
					AddGroupToTrap(Group);
				}
			}
		}
	}
	UpdateAllViews(nullptr);
}

void AeSysDoc::OnTrapCommandsSquare() {
	SquareTrappedGroups(AeSysView::GetActiveView());
}

void AeSysDoc::OnTrapCommandsQuery() {
	EoDlgEditTrapCommandsQuery Dialog;
	if (Dialog.DoModal() == IDOK) {
	}
}

void AeSysDoc::OnTrapCommandsFilter() {
	EoDlgTrapFilter Dialog(this, m_DatabasePtr);
	if (Dialog.DoModal() == IDOK) {
	}
}

void AeSysDoc::OnTrapCommandsBlock() {
	if (m_TrappedGroupList.GetCount() == 0) { return; }
	EoDbBlock* Block {nullptr};
	auto w {BlockTableSize()};
	wchar_t BlockName[16];
	do {
		swprintf_s(BlockName, 16, L"_%.3i", ++w);
	} while (LookupBlock(BlockName, Block));
	Block = new EoDbBlock;
	auto Position {GetFirstTrappedGroupPosition()};
	while (Position != nullptr) {
		const auto Group {GetNextTrappedGroup(Position)};
		auto pSeg2 {new EoDbGroup(*Group)};
		Block->AddTail(pSeg2);
		pSeg2->RemoveAll();
		delete pSeg2;
	}
	Block->SetBasePoint(m_TrapPivotPoint);
	InsertBlock(BlockName, Block);
}

void AeSysDoc::OnTrapCommandsUnblock() {
	m_TrappedGroupList.BreakSegRefs();
}

void AeSysDoc::OnSetupPenColor() {
	EoDlgSetupColor Dialog;
	Dialog.m_ColorIndex = static_cast<unsigned>(g_PrimitiveState.ColorIndex());
	if (Dialog.DoModal() == IDOK) {
		g_PrimitiveState.SetColorIndex(nullptr, static_cast<short>(Dialog.m_ColorIndex));
		AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::kPen);
	}
}

void AeSysDoc::OnSetupLinetype() {
	const OdDbLinetypeTablePtr Linetypes {m_DatabasePtr->getLinetypeTableId().safeOpenObject(OdDb::kForRead)};
	EoDlgSetupLinetype Dialog(Linetypes);
	if (Dialog.DoModal() == IDOK) {
		const auto Name {Dialog.m_Linetype->getName()};
		const auto LinetypeIndex {static_cast<short>(EoDbLinetypeTable::LegacyLinetypeIndex(Name))};
		g_PrimitiveState.SetLinetypeIndexPs(nullptr, LinetypeIndex);
		AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::kLine);
	}
}

void AeSysDoc::OnSetupFillHollow() noexcept {
	g_PrimitiveState.SetHatchInteriorStyle(EoDbHatch::kHollow);
}

void AeSysDoc::OnSetupFillSolid() noexcept {
	g_PrimitiveState.SetHatchInteriorStyle(EoDbHatch::kSolid);
}

void AeSysDoc::OnSetupFillPattern() noexcept {
}

void AeSysDoc::OnSetupFillHatch() {
	EoDlgSetupHatch Dialog;
	Dialog.m_HatchXScaleFactor = EoDbHatch::sm_PatternScaleX;
	Dialog.m_HatchYScaleFactor = EoDbHatch::sm_PatternScaleY;
	Dialog.m_HatchRotationAngle = EoToDegree(EoDbHatch::sm_PatternAngle);
	if (Dialog.DoModal() == IDOK) {
		g_PrimitiveState.SetHatchInteriorStyle(EoDbHatch::kHatch);
		EoDbHatch::sm_PatternScaleX = EoMax(.01, Dialog.m_HatchXScaleFactor);
		EoDbHatch::sm_PatternScaleY = EoMax(.01, Dialog.m_HatchYScaleFactor);
		EoDbHatch::sm_PatternAngle = EoArcLength(Dialog.m_HatchRotationAngle);
	}
}

void AeSysDoc::OnSetupNote() {
	auto FontDefinition {g_PrimitiveState.FontDefinition()};
	EoDlgSetupNote Dialog(&FontDefinition);
	auto CharacterCellDefinition {g_PrimitiveState.CharacterCellDefinition()};
	Dialog.m_Height = CharacterCellDefinition.Height();
	Dialog.m_RotationAngle = EoToDegree(CharacterCellDefinition.RotationAngle());
	Dialog.m_WidthFactor = CharacterCellDefinition.WidthFactor();
	Dialog.m_ObliqueAngle = EoToDegree(CharacterCellDefinition.ObliqueAngle());
	if (Dialog.DoModal() == IDOK) {
		CharacterCellDefinition.SetHeight(Dialog.m_Height);
		CharacterCellDefinition.SetRotationAngle(EoToRadian(Dialog.m_RotationAngle));
		CharacterCellDefinition.SetWidthFactor(Dialog.m_WidthFactor);
		CharacterCellDefinition.SetObliqueAngle(EoToRadian(Dialog.m_ObliqueAngle));
		g_PrimitiveState.SetCharacterCellDefinition(CharacterCellDefinition);
		auto ActiveView {AeSysView::GetActiveView()};
		const auto DeviceContext {ActiveView ? ActiveView->GetDC() : nullptr};
		g_PrimitiveState.SetFontDefinition(DeviceContext, FontDefinition);
	}
}

void AeSysDoc::OnToolsGroupBreak() {
	auto ActiveView {AeSysView::GetActiveView()};
	ActiveView->BreakAllPolylines();
	ActiveView->BreakAllSegRefs();
}

void AeSysDoc::OnToolsGroupDelete() {
	auto ActiveView {AeSysView::GetActiveView()};
	const auto CurrentPnt {ActiveView->GetCursorPosition()};
	const auto Group {ActiveView->SelectGroupAndPrimitive(CurrentPnt)};
	if (Group != nullptr) {
		AnyLayerRemove(Group);
		RemoveGroupFromAllViews(Group);
		if (RemoveTrappedGroup(Group) != nullptr) {
			ActiveView->UpdateStateInformation(AeSysView::kTrapCount);
		}
		UpdateGroupInAllViews(EoDb::kGroupEraseSafe, Group);
		DeletedGroupsAddTail(Group);
		theApp.AddStringToMessageList(IDS_MSG_GROUP_ADDED_TO_DEL_GROUPS);
	}
}

void AeSysDoc::OnToolsGroupDeleteLast() {
	AeSysView::GetActiveView()->DeleteLastGroup();
}

void AeSysDoc::OnToolsGroupExchange() {
	if (m_DeletedGroupList.GetSize() > 1) {
		const auto TailGroup {DeletedGroupsRemoveTail()};
		const auto HeadGroup {DeletedGroupsRemoveHead()};
		DeletedGroupsAddTail(HeadGroup);
		DeletedGroupsAddHead(TailGroup);
	}
}

void AeSysDoc::OnToolsPrimitiveSnapToEndPoint() {
	auto ActiveView {AeSysView::GetActiveView()};
	EoGePoint4d View(ActiveView->GetCursorPosition(), 1.0);
	ActiveView->ModelViewTransformPoint(View);
	if (ActiveView->GroupIsEngaged()) {
		auto Primitive {ActiveView->EngagedPrimitive()};
		if (Primitive->PivotOnGripPoint(ActiveView, View)) {
			const auto ptEng {ActiveView->DetPt()};
			Primitive->AddReportToMessageList(ptEng);
			ActiveView->SetCursorPosition(ptEng);
			return;
		}
		// Did not pivot on engaged primitive
		if (Primitive->IsPointOnControlPoint(ActiveView, View)) {
			EoDbGroup::SetPrimitiveToIgnore(Primitive);
		}
	}
	if (ActiveView->SelSegAndPrimAtCtrlPt(View) != nullptr) {
		const auto ptEng {ActiveView->DetPt()};
		ActiveView->EngagedPrimitive()->AddReportToMessageList(ptEng);
		ActiveView->SetCursorPosition(ptEng);
	}
	EoDbGroup::SetPrimitiveToIgnore(static_cast<EoDbPrimitive*>(nullptr));
}

void AeSysDoc::OnPrimGotoCenterPoint() {
	auto ActiveView {AeSysView::GetActiveView()};
	if (ActiveView->GroupIsEngaged()) {
		const auto pt {ActiveView->EngagedPrimitive()->GetCtrlPt()};
		ActiveView->SetCursorPosition(pt);
	}
}

void AeSysDoc::OnToolsPrimitiveDelete() {
	const auto CurrentPnt {theApp.GetCursorPosition()};
	auto ActiveView {AeSysView::GetActiveView()};
	auto Group {ActiveView->SelectGroupAndPrimitive(CurrentPnt)};
	if (Group != nullptr) {
		const auto Position {FindTrappedGroup(Group)};
		LPARAM lHint = Position != nullptr ? EoDb::kGroupEraseSafeTrap : EoDb::kGroupEraseSafe;
		// erase entire group even if group has more than one primitive
		UpdateGroupInAllViews(lHint, Group);
		if (Group->GetCount() > 1) { // remove primitive from group
			const auto Primitive {ActiveView->EngagedPrimitive()};
			Group->FindAndRemovePrimitive(Primitive);
			lHint = Position != nullptr ? EoDb::kGroupSafeTrap : EoDb::kGroupSafe;
			// display the group with the primitive removed
			UpdateGroupInAllViews(lHint, Group);
			// new group required to allow primitive to be placed into deleted group list
			Group = new EoDbGroup;
			Group->AddTail(Primitive);
		} else { // deleting an entire group
			AnyLayerRemove(Group);
			RemoveGroupFromAllViews(Group);
			if (RemoveTrappedGroup(Group) != nullptr) {
				ActiveView->UpdateStateInformation(AeSysView::kTrapCount);
			}
		}
		DeletedGroupsAddTail(Group);
		theApp.AddStringToMessageList(IDS_MSG_PRIM_ADDED_TO_DEL_GROUPS);
	}
}

void AeSysDoc::OnPrimModifyAttributes() {
	auto ActiveView {AeSysView::GetActiveView()};
	const auto CurrentPnt {ActiveView->GetCursorPosition()};
	const auto Group {ActiveView->SelectGroupAndPrimitive(CurrentPnt)};
	if (Group != nullptr) {
		ActiveView->EngagedPrimitive()->ModifyState();
		UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, ActiveView->EngagedPrimitive());
	}
}

void AeSysDoc::OnSetupSavePoint() {
	EoDlgSetHomePoint Dialog(AeSysView::GetActiveView());
	if (Dialog.DoModal() == IDOK) {
	}
}

void AeSysDoc::OnSetupGotoPoint() {
	EoDlgSelectGotoHomePoint Dialog(AeSysView::GetActiveView());
	if (Dialog.DoModal() == IDOK) {
	}
}

void AeSysDoc::OnSetupOptionsDraw() {
	EoDlgDrawOptions Dialog;
	if (Dialog.DoModal() == IDOK) {
		AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::kAll);
	}
}

void AeSysDoc::OnFileManage() {
	EoDlgFileManage dlg(this, m_DatabasePtr);
	if (dlg.DoModal() == IDOK) {
	}
}

void AeSysDoc::OnFileTracing() {
	static unsigned long FilterIndex {1};
	OPENFILENAME OpenFileName;
	::ZeroMemory(&OpenFileName, sizeof(OPENFILENAME));
	OpenFileName.lStructSize = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner = nullptr;
	OpenFileName.hInstance = theApp.GetInstance();
	OpenFileName.lpstrFilter = L"Tracing Files\0*.tra;*.jb1\0\0";
	OpenFileName.nFilterIndex = FilterIndex;
	OpenFileName.lpstrFile = new wchar_t[MAX_PATH];
	OpenFileName.lpstrFile[0] = 0;
	OpenFileName.nMaxFile = MAX_PATH;
	OpenFileName.lpstrTitle = L"Load Tracing";
	OpenFileName.Flags = OFN_EXPLORER | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
	OpenFileName.lpstrDefExt = L"tra";
	OpenFileName.lpfnHook = OfnHookProcFileTracing;
	OpenFileName.lpTemplateName = MAKEINTRESOURCEW(IDD_TRACING_EX);
	if (GetOpenFileNameW(&OpenFileName)) {
		FilterIndex = OpenFileName.nFilterIndex;
		TracingOpen(OpenFileName.lpstrFile);
	}
	delete[] OpenFileName.lpstrFile;
}

void AeSysDoc::OnPurgeDuplicateObjects() {
	PurgeDuplicateObjects();
}

void AeSysDoc::OnPurgeEmptyNotes() {
	const auto NumberOfEmptyNotes {RemoveEmptyNotesAndDelete()};
	const auto NumberOfEmptyGroups {RemoveEmptyGroups()};
	OdString Message;
	Message.format(L"%d notes were removed resulting in %d empty groups which were also removed.", NumberOfEmptyNotes, NumberOfEmptyGroups);
	theApp.AddStringToMessageList(Message);
}

void AeSysDoc::OnPurgeEmptyGroups() {
	const auto NumberOfEmptyGroups = RemoveEmptyGroups();
	OdString Message;
	Message.format(L"%d were removed.", NumberOfEmptyGroups);
	theApp.AddStringToMessageList(Message);
}

void AeSysDoc::OnPensEditColors() {
	theApp.EditColorPalette();
}

void AeSysDoc::OnPensLoadColors() {
	const auto InitialDirectory {theApp.ResourceFolderPath() + L"Pens\\Colors\\"};
	OPENFILENAME OpenFileName;
	::ZeroMemory(&OpenFileName, sizeof(OPENFILENAME));
	OpenFileName.lStructSize = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner = nullptr;
	OpenFileName.hInstance = theApp.GetInstance();
	OpenFileName.lpstrFilter = L"Pen Color Files\0*.txt\0\0";
	OpenFileName.lpstrFile = new wchar_t[MAX_PATH];
	OpenFileName.lpstrFile[0] = 0;
	OpenFileName.nMaxFile = MAX_PATH;
	OpenFileName.lpstrTitle = L"Load Pen Colors";
	OpenFileName.lpstrInitialDir = InitialDirectory;
	OpenFileName.Flags = OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	OpenFileName.lpstrDefExt = L"txt";
	if (GetOpenFileNameW(&OpenFileName)) {
		if ((OpenFileName.Flags & OFN_EXTENSIONDIFFERENT) == 0) {
			theApp.LoadColorPalletFromFile(OpenFileName.lpstrFile);
			UpdateAllViews(nullptr);
		} else theApp.WarningMessageBox(IDS_MSG_FILE_TYPE_ERROR);
	}
	delete[] OpenFileName.lpstrFile;
}

// <tas="OnPensTranslate would be more useful if the file name could be selected. Currently fixed as xlate.txt"</tas>
void AeSysDoc::OnPensTranslate() {
	CStdioFile StreamFile;
	if (StreamFile.Open(AeSys::ResourceFolderPath() + L"\\Pens\\xlate.txt", CFile::modeRead | CFile::typeText)) {
		wchar_t pBuf[128];
		std::vector<int> ColorIndex;
		std::vector<int> NewColorIndex;
		wchar_t* NextToken {nullptr};
		while (StreamFile.ReadString(pBuf, sizeof pBuf / sizeof(wchar_t) - 1) != nullptr) {
			NextToken = nullptr;
			ColorIndex.push_back(_wtoi(wcstok_s(pBuf, L",", &NextToken)));
			NewColorIndex.push_back(_wtoi(wcstok_s(nullptr, L"\n", &NextToken)));
		}
		PenTranslation(NewColorIndex.size(), NewColorIndex, ColorIndex);
		UpdateAllViews(nullptr);
	}
}

void AeSysDoc::OnFile() {
	CPoint Position(8, 8);
	theApp.GetMainWnd()->ClientToScreen(&Position);
	auto FileSubMenu {CMenu::FromHandle(theApp.GetAeSysSubMenu(0))};
	FileSubMenu->TrackPopupMenuEx(TPM_LEFTALIGN, Position.x, Position.y, AfxGetMainWnd(), nullptr);
}

void AeSysDoc::OnPrimExtractNum() {
	auto ActiveView {AeSysView::GetActiveView()};
	const auto CurrentPnt {ActiveView->GetCursorPosition()};
	if (ActiveView->SelectGroupAndPrimitive(CurrentPnt)) {
		const auto Primitive {ActiveView->EngagedPrimitive()};
		CString Number;
		if (Primitive->IsKindOf(RUNTIME_CLASS(EoDbText))) {
			Number = dynamic_cast<EoDbText*>(Primitive)->Text();
		} else if (Primitive->IsKindOf(RUNTIME_CLASS(EoDbDimension))) {
			Number = dynamic_cast<EoDbDimension*>(Primitive)->Text();
		} else {
			return;
		}
		double dVal[32];
		auto iTyp {0};
		long DataDefinition {0};
		auto iTokId = 0;
		lex::Parse(Number);
		lex::EvalTokenStream(&iTokId, &DataDefinition, &iTyp, static_cast<void*>(dVal));
		if (iTyp != lex::TOK_LENGTH_OPERAND) {
			lex::ConvertValTyp(iTyp, lex::TOK_REAL, &DataDefinition, dVal);
		}
		wchar_t Message[64];
		swprintf_s(Message, 64, L"%10.4f ", dVal[0]);
		wcscat_s(Message, 64, L"was extracted from drawing");
		theApp.AddStringToMessageList(Message);
	}
}

void AeSysDoc::OnPrimExtractStr() {
	auto ActiveView {AeSysView::GetActiveView()};
	const auto CurrentPnt {ActiveView->GetCursorPosition()};
	if (ActiveView->SelectGroupAndPrimitive(CurrentPnt)) {
		const auto Primitive {ActiveView->EngagedPrimitive()};
		CString String;
		if (Primitive->IsKindOf(RUNTIME_CLASS(EoDbText))) {
			String = dynamic_cast<EoDbText*>(Primitive)->Text();
		} else if (Primitive->IsKindOf(RUNTIME_CLASS(EoDbDimension))) {
			String = dynamic_cast<EoDbDimension*>(Primitive)->Text();
		} else {
			return;
		}
		String += L" was extracted from drawing";
		theApp.AddStringToMessageList(String);
	}
}

// Returns a pointer to the currently active document.
AeSysDoc* AeSysDoc::GetDoc() {
	const CMDIFrameWndEx* Frame {dynamic_cast<CMDIFrameWndEx*>(AfxGetMainWnd())};
	if (Frame == nullptr) { return nullptr; }
	auto Child {dynamic_cast<CMDIChildWndEx*>(Frame->MDIGetActive())};
	return Child == nullptr ? nullptr : dynamic_cast<AeSysDoc*>(Child->GetActiveDocument());
}

void AeSysDoc::AddGroupToAllViews(EoDbGroup* group) {
	auto ViewPosition {GetFirstViewPosition()};
	while (ViewPosition != nullptr) {
		auto View {dynamic_cast<AeSysView*>(GetNextView(ViewPosition))};
		View->AddVisibleGroup(group);
	}
}

void AeSysDoc::AddGroupsToAllViews(EoDbGroupList* groups) {
	auto ViewPosition {GetFirstViewPosition()};
	while (ViewPosition != nullptr) {
		auto View {dynamic_cast<AeSysView*>(GetNextView(ViewPosition))};
		View->AddVisibleGroups(groups);
	}
}

void AeSysDoc::RemoveAllGroupsFromAllViews() {
	auto ViewPosition {GetFirstViewPosition()};
	while (ViewPosition != nullptr) {
		auto View {dynamic_cast<AeSysView*>(GetNextView(ViewPosition))};
		View->RemoveAllVisibleGroups();
	}
}

void AeSysDoc::RemoveGroupFromAllViews(EoDbGroup* group) {
	auto ViewPosition {GetFirstViewPosition()};
	while (ViewPosition != nullptr) {
		auto View {dynamic_cast<AeSysView*>(GetNextView(ViewPosition))};
		View->RemoveVisibleGroup(group);
	}
}

void AeSysDoc::ResetAllViews() {
	auto ViewPosition {GetFirstViewPosition()};
	while (ViewPosition != nullptr) {
		auto View {dynamic_cast<AeSysView*>(GetNextView(ViewPosition))};
		View->ResetView();
	}
}

void AeSysDoc::OnHelpKey() {
	switch (theApp.CurrentMode()) {
		case ID_MODE_DRAW:
			HtmlHelpW(AfxGetMainWnd()->GetSafeHwnd(), L"..\\AeSys\\hlp\\AeSys.chm::/hid_mode_draw.htm", HH_DISPLAY_TOPIC, NULL);
			break;
		case ID_MODE_EDIT: {
			HtmlHelpW(AfxGetMainWnd()->GetSafeHwnd(), L"..\\AeSys\\hlp\\AeSys.chm::/hid_mode_edit.htm", HH_DISPLAY_TOPIC, NULL);
			break;
		}
		case ID_MODE_TRAP: case ID_MODE_TRAPR: {
			HtmlHelpW(AfxGetMainWnd()->GetSafeHwnd(), L"..\\AeSys\\hlp\\AeSys.chm::/hid_mode_trap.htm", HH_DISPLAY_TOPIC, NULL);
			break;
		}
		default: ;
	}
}

void AeSysDoc::DeleteNodalResources() {
	auto UniquePointPosition {m_UniquePoints.GetHeadPosition()};
	while (UniquePointPosition != nullptr) {
		delete GetNextUniquePoint(UniquePointPosition);
	}
	RemoveAllUniquePoints();
	auto MaskedPrimitivePosition {GetFirstMaskedPrimitivePosition()};
	while (MaskedPrimitivePosition != nullptr) {
		delete GetNextMaskedPrimitive(MaskedPrimitivePosition);
	}
	RemoveAllMaskedPrimitives();
	RemoveAllNodalGroups();
}

void AeSysDoc::UpdateNodalList(EoDbGroup* group, EoDbPrimitive* primitive, const unsigned long mask, const int bit, const OdGePoint3d point) {
	if (theApp.m_NodalModeAddGroups) {
		if (!btest(mask, bit)) {
			if (!FindNodalGroup(group)) {
				AddNodalGroup(group);
			}
			AddPrimitiveBit(primitive, bit);
			if (AddUniquePoint(point) == 1) {
				EoDbPoint PointPrimitive(point);
				PointPrimitive.SetColorIndex2(252);
				PointPrimitive.SetPointDisplayMode(8);
				UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, &PointPrimitive);
			}
		}
	} else {
		if (btest(mask, bit)) {
			RemovePrimitiveBit(primitive, bit);
			if (RemoveUniquePoint(point) == 0) {
				EoDbPoint PointPrimitive(point);
				PointPrimitive.SetColorIndex2(252);
				PointPrimitive.SetPointDisplayMode(8);
				UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, &PointPrimitive);
			}
		}
	}
}

POSITION AeSysDoc::AddNodalGroup(EoDbGroup* group) {
	return m_NodalGroupList.AddTail(group);
}

POSITION AeSysDoc::FindNodalGroup(EoDbGroup* group) {
	return m_NodalGroupList.Find(group);
}

POSITION AeSysDoc::GetFirstNodalGroupPosition() const {
	return m_NodalGroupList.GetHeadPosition();
}

EoDbGroup* AeSysDoc::GetNextNodalGroup(POSITION& position) {
	return m_NodalGroupList.GetNext(position);
}

void AeSysDoc::RemoveAllNodalGroups() {
	m_NodalGroupList.RemoveAll();
}

POSITION AeSysDoc::AddMaskedPrimitive(EoDbMaskedPrimitive* maskedPrimitive) {
	return m_MaskedPrimitives.AddTail(static_cast<CObject*>(maskedPrimitive));
}

POSITION AeSysDoc::GetFirstMaskedPrimitivePosition() const {
	return m_MaskedPrimitives.GetHeadPosition();
}

EoDbMaskedPrimitive* AeSysDoc::GetNextMaskedPrimitive(POSITION& position) {
	return static_cast<EoDbMaskedPrimitive*>(m_MaskedPrimitives.GetNext(position));
}

void AeSysDoc::RemoveAllMaskedPrimitives() {
	m_MaskedPrimitives.RemoveAll();
}

int AeSysDoc::AddUniquePoint(const OdGePoint3d& point) {
	auto UniquePointPosition {m_UniquePoints.GetHeadPosition()};
	while (UniquePointPosition != nullptr) {
		const auto UniquePoint {GetNextUniquePoint(UniquePointPosition)};
		if ((point - UniquePoint->m_Point).length() <= OdGeContext::gTol.equalPoint()) {

			//		if (point == UniquePoint->m_Point) {
			UniquePoint->m_References++;
			return UniquePoint->m_References;
		}
	}
	m_UniquePoints.AddTail(new EoGeUniquePoint(1, point));
	return 1;
}

EoGeUniquePoint* AeSysDoc::GetNextUniquePoint(POSITION& position) {
	return static_cast<EoGeUniquePoint*>(m_UniquePoints.GetNext(position));
}

void AeSysDoc::RemoveUniquePointAt(const POSITION position) {
	m_UniquePoints.RemoveAt(position);
}

void AeSysDoc::RemoveAllUniquePoints() {
	m_UniquePoints.RemoveAll();
}

void AeSysDoc::DisplayUniquePoints() {
	if (m_UniquePoints.IsEmpty()) { return; }
	EoDbGroup Group;
	auto UniquePointPosition {m_UniquePoints.GetHeadPosition()};
	while (UniquePointPosition != nullptr) {
		const auto UniquePoint {GetNextUniquePoint(UniquePointPosition)};
		auto PointPrimitive {new EoDbPoint(UniquePoint->m_Point)};
		PointPrimitive->SetColorIndex2(252);
		PointPrimitive->SetPointDisplayMode(8);
		Group.AddTail(PointPrimitive);
	}
	UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &Group);
	Group.DeletePrimitivesAndRemoveAll();
}

int AeSysDoc::RemoveUniquePoint(const OdGePoint3d& point) {
	auto References {0};
	auto UniquePointPosition {m_UniquePoints.GetHeadPosition()};
	while (UniquePointPosition != nullptr) {
		const auto Position {UniquePointPosition};
		auto UniquePoint {GetNextUniquePoint(UniquePointPosition)};
		if (point == UniquePoint->m_Point) {
			References = --UniquePoint->m_References;
			if (References == 0) {
				RemoveUniquePointAt(Position);
				delete UniquePoint;
			}
			break;
		}
	}
	return References;
}

void AeSysDoc::AddPrimitiveBit(EoDbPrimitive* primitive, const int bit) {
	EoDbMaskedPrimitive* MaskedPrimitive {nullptr};
	auto MaskedPrimitivePosition {GetFirstMaskedPrimitivePosition()};
	while (MaskedPrimitivePosition != nullptr) {
		const auto posCur = MaskedPrimitivePosition;
		MaskedPrimitive = GetNextMaskedPrimitive(MaskedPrimitivePosition);
		if (MaskedPrimitive->GetPrimitive() == primitive) {
			MaskedPrimitivePosition = posCur;
			break;
		}
	}
	if (MaskedPrimitivePosition == nullptr) {
		MaskedPrimitive = new EoDbMaskedPrimitive(primitive, 0);
		AddMaskedPrimitive(MaskedPrimitive);
	}
	MaskedPrimitive->SetMaskBit(bit);
}

void AeSysDoc::RemovePrimitiveBit(EoDbPrimitive* primitive, const int bit) {
	EoDbMaskedPrimitive* MaskedPrimitive = nullptr;
	auto MaskedPrimitivePosition {GetFirstMaskedPrimitivePosition()};
	while (MaskedPrimitivePosition != nullptr) {
		const auto posCur {MaskedPrimitivePosition};
		MaskedPrimitive = GetNextMaskedPrimitive(MaskedPrimitivePosition);
		if (MaskedPrimitive->GetPrimitive() == primitive) {
			MaskedPrimitivePosition = posCur;
			break;
		}
	}
	if (MaskedPrimitivePosition != nullptr) {
		MaskedPrimitive->ClearMaskBit(bit);
	}
}

unsigned long AeSysDoc::GetPrimitiveMask(EoDbPrimitive* primitive) {
	EoDbMaskedPrimitive* MaskedPrimitive {nullptr};
	auto MaskedPrimitivePosition {GetFirstMaskedPrimitivePosition()};
	while (MaskedPrimitivePosition != nullptr) {
		const auto posCur {MaskedPrimitivePosition};
		MaskedPrimitive = GetNextMaskedPrimitive(MaskedPrimitivePosition);
		if (MaskedPrimitive->GetPrimitive() == primitive) {
			MaskedPrimitivePosition = posCur;
			break;
		}
	}
	return MaskedPrimitivePosition != nullptr ? MaskedPrimitive->GetMask() : 0UL;
}

void AeSysDoc::OnSetupLayerProperties() {
	EoDlgLayerPropertiesManager LayerPropertiesManager(m_DatabasePtr);
	if (IDOK == LayerPropertiesManager.DoModal()) {
		UpdateAllViews(nullptr);
	}
}

void AeSysDoc::OnInsertTracing() {
	static unsigned long FilterIndex {1};
	OPENFILENAME OpenFileName;
	::ZeroMemory(&OpenFileName, sizeof(OPENFILENAME));
	OpenFileName.lStructSize = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner = nullptr;
	OpenFileName.hInstance = theApp.GetInstance();
	OpenFileName.lpstrFilter = L"Tracing Files\0*.tra;*.jb1\0\0";
	OpenFileName.nFilterIndex = FilterIndex;
	OpenFileName.lpstrFile = new wchar_t[MAX_PATH];
	OpenFileName.lpstrFile[0] = 0;
	OpenFileName.nMaxFile = MAX_PATH;
	OpenFileName.lpstrTitle = L"Insert Tracing";
	OpenFileName.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
	OpenFileName.lpstrDefExt = L"tra";
	if (GetOpenFileNameW(&OpenFileName)) {
		FilterIndex = OpenFileName.nFilterIndex;
		OdString Name {OpenFileName.lpstrFile};
		auto strPath {Name.left(OpenFileName.nFileOffset)};
		Name = Name.mid(OpenFileName.nFileOffset);
		const auto FileType {AeSys::GetFileType(Name)};
		if (FileType != EoDb::kTracing && FileType != EoDb::kJob) { return; }
		auto Layers {LayerTable(OdDb::kForWrite)};
		if (Layers->getAt(Name).isNull()) {
			auto LayerTableRecord {OdDbLayerTableRecord::createObject()};
			LayerTableRecord->setName(Name);
			auto Layer {new EoDbLayer(LayerTableRecord)};
			if (TracingLoadLayer(Name, Layer)) {
				m_LayerTable.Add(Layer);
				Layer->SetIsOff(false);
				Layer->SetIsLocked(true);
				Layer->SetLinetype(m_DatabasePtr->getLinetypeContinuousId());
				Layer->SetColorIndex(1);
			} else {
				delete Layer;
			}
		}
	}
	delete[] OpenFileName.lpstrFile;
}

void AeSysDoc::OnFilePageSetup() {
	OdSmartPtr<OdDbUserIO> pIO; // = pDbCmdCtx->userIO();
	const auto idLayout {OdDbBlockTableRecordPtr(m_DatabasePtr->getActiveLayoutBTRId().safeOpenObject())->getLayoutId()};
	OdSmartPtr<OdDbLayout> Layout {idLayout.safeOpenObject(OdDb::kForWrite)};
	OdDbPlotSettings* PlotSettings = Layout.get();
	EoDlgPageSetup PageSetupDialog(*PlotSettings, pIO);
	if (PageSetupDialog.DoModal() == IDOK) {
	}
}

// <command_console>
AeSysDoc::DataSource::DataSource() {
}

void AeSysDoc::DataSource::Create(AeSysDoc* document, const OdGePoint3d& point) {
	Empty();
	const auto ObjectIds {document->SelectionSet()->objectIdArray()};
	auto Database {document->m_DatabasePtr->wblock(ObjectIds, OdGePoint3d::kOrigin)};
	wchar_t TemporaryPath[MAX_PATH];
	::GetTempPath(MAX_PATH, TemporaryPath);
	wchar_t TemporaryName[MAX_PATH];
	::GetTempFileName(TemporaryPath, L"", 0, TemporaryName);
	m_tmpPath = TemporaryName;
	m_tmpPath.makeLower();
	m_tmpPath.replace(L".tmp", L".dwg");
	auto StreamBuffer {theApp.createFile(m_tmpPath, Oda::kFileWrite, Oda::kShareDenyWrite, Oda::kCreateNew)};

	//Database->writeFile(StreamBuffer,OdDb::kDwg,OdDb::vAC21);
	//auto hGlobal {GlobalAlloc(GMEM_FIXED, sizeof(AcadClipDataR15))};
	//new (hGlobal) AcadClipDataR15(m_tmpPath, OdString(document->GetPathName()), point);
	//CacheGlobalData(ClipboardData::m_FormatR16, hGlobal);
	Database->writeFile(StreamBuffer, OdDb::kDwg, OdDb::vAC21);
	auto hGlobalR21 {GlobalAlloc(GMEM_FIXED, sizeof(AcadClipDataR21))};
	new(hGlobalR21)AcadClipDataR21(m_tmpPath, OdString(document->GetPathName()), point);
	CacheGlobalData(ClipboardData::m_FormatR17, hGlobalR21);
}
// </command_console>
bool AeSysDoc::DataSource::DoDragDrop() {
	return COleDataSource::DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_MOVE) != DROPEFFECT_NONE;
}

void AeSysDoc::DataSource::Empty() {
	COleDataSource::Empty();
	if (!m_tmpPath.isEmpty()) {
		DeleteFile(m_tmpPath);
	}
}

AeSysDoc::DataSource::~DataSource() {
	Empty();
}

void AeSysDoc::OnDrawingUtilitiesAudit() {
	const auto FixErrors {AfxMessageBox(L"Fix any errors detected?", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES};
	ODA_ASSERT(!theApp.m_pAuditDlg);
	theApp.m_pAuditDlg = new EoDlgAudit();
	if (!theApp.m_pAuditDlg) {
		AfxMessageBox(L"Error Creating Audit Dialog Object");
		return;
	}
	if (!theApp.m_pAuditDlg->Create(IDD_AUDITINFO)) {
		AfxMessageBox(L"Error Creating Audit Dialog Window");
		return;
	}
	try {
		EoAppAuditInfo aiAppAudit;
		aiAppAudit.setFixErrors(FixErrors);
		aiAppAudit.setPrintDest(OdDbAuditInfo::kBoth);
		aiAppAudit.setHostAppServices(&theApp);
		m_DatabasePtr->auditDatabase(&aiAppAudit);
	} catch (const OdError& Error) {
		delete theApp.m_pAuditDlg;
		theApp.m_pAuditDlg = nullptr;
		theApp.reportError(L"Error Auditing Database...", Error);
		AfxThrowUserException();
	} catch (const UserBreak&) {
		delete theApp.m_pAuditDlg;
		theApp.m_pAuditDlg = nullptr;
	}
	if (!theApp.m_pAuditDlg) {
		return;
	}
	const auto Title(L"Audit info - " + GetTitle());
	theApp.m_pAuditDlg->SetWindowTextW(Title);
	theApp.m_pAuditDlg->ShowWindow(SW_SHOW);
	theApp.m_pAuditDlg = nullptr;
}

BOOL AeSysDoc::DoPromptFileName(CString& fileName, unsigned nIDSTitle, unsigned long flags, BOOL openFileDialog, CDocTemplate* documentTemplate) {
	const auto DwgVersion {m_DatabasePtr->originalFileVersion()};
	auto Extension {fileName.Right(3)};
	const auto IsDwg {Extension.CompareNoCase(L"dxf") != 0};
	CString Title;
	VERIFY(Title.LoadString(nIDSTitle));
	CFileDialog FileDialog(openFileDialog);
	FileDialog.m_ofn.Flags |= flags;
	CString Filter;
	Filter = L"AutoCAD 2018 Compatible Drawing |*.dwg|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (IsDwg && DwgVersion == OdDb::vAC32) FileDialog.m_ofn.nFilterIndex = 1;
	Filter += L"AutoCAD 2013 Compatible Drawing |*.dwg|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (IsDwg && DwgVersion == OdDb::vAC27) FileDialog.m_ofn.nFilterIndex = 2;
	Filter += "AutoCAD 2010 Compatible Drawing |*.dwg|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (IsDwg && DwgVersion == OdDb::vAC24) FileDialog.m_ofn.nFilterIndex = 3;
	Filter += "AutoCAD 2007 Compatible Drawing |*.dwg|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (IsDwg && DwgVersion == OdDb::vAC21) FileDialog.m_ofn.nFilterIndex = 4;
	Filter += L"AutoCAD 2004 Compatible Drawing |*.dwg|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (IsDwg && (DwgVersion == OdDb::kDHL_1800a || DwgVersion == OdDb::kDHL_1800)) FileDialog.m_ofn.nFilterIndex = 5;
	Filter += L"AutoCAD 2000 Compatible Drawing |*.dwg|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (IsDwg && DwgVersion == OdDb::vAC15) FileDialog.m_ofn.nFilterIndex = 6;
	Filter += L"AutoCAD R14 Compatible Drawing |*.dwg|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (IsDwg && DwgVersion == OdDb::vAC14) FileDialog.m_ofn.nFilterIndex = 7;
	Filter += L"AutoCAD R13 Compatible Drawing |*.dwg|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (IsDwg && DwgVersion == OdDb::vAC13) FileDialog.m_ofn.nFilterIndex = 8;
	Filter += L"AutoCAD R12 Compatible Drawing |*.dwg|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (IsDwg && DwgVersion <= OdDb::vAC12) FileDialog.m_ofn.nFilterIndex = 9;
	Filter += L"AutoCAD 2018 Compatible DXF |*.dxf|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (!IsDwg && DwgVersion == OdDb::vAC32) FileDialog.m_ofn.nFilterIndex = 10;
	Filter += L"AutoCAD 2013 Compatible DXF |*.dxf|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (!IsDwg && DwgVersion == OdDb::kDHL_1027) FileDialog.m_ofn.nFilterIndex = 11;
	Filter += L"AutoCAD 2010 Compatible DXF |*.dxf|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (!IsDwg && DwgVersion == OdDb::kDHL_1024) FileDialog.m_ofn.nFilterIndex = 12;
	Filter += L"AutoCAD 2007 Compatible DXF |*.dxf|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (!IsDwg && DwgVersion == OdDb::kDHL_1021) FileDialog.m_ofn.nFilterIndex = 13;
	Filter += L"AutoCAD 2004 Compatible DXF |*.dxf|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (!IsDwg && (DwgVersion == OdDb::kDHL_1800a || DwgVersion == OdDb::kDHL_1800)) FileDialog.m_ofn.nFilterIndex = 14;
	Filter += L"AutoCAD 2000 Compatible DXF |*.dxf|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (!IsDwg && DwgVersion == OdDb::vAC15) FileDialog.m_ofn.nFilterIndex = 15;
	Filter += L"AutoCAD R14 Compatible DXF |*.dxf|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (!IsDwg && DwgVersion == OdDb::vAC14) FileDialog.m_ofn.nFilterIndex = 16;
	Filter += L"AutoCAD R13 Compatible DXF |*.dxf|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (!IsDwg && DwgVersion == OdDb::vAC13) FileDialog.m_ofn.nFilterIndex = 17;
	Filter += L"AutoCAD R12 Compatible DXF |*.dxf|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (!IsDwg && DwgVersion == OdDb::vAC12) FileDialog.m_ofn.nFilterIndex = 18;
	Filter += L"AutoCAD R10 Compatible DXF |*.dxf|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (!IsDwg && DwgVersion == OdDb::vAC10) FileDialog.m_ofn.nFilterIndex = 19;
	Filter += L"AutoCAD R9 Compatible DXF |*.dxf|";
	FileDialog.m_ofn.nMaxCustFilter++;
	if (!IsDwg && DwgVersion == OdDb::vAC09) FileDialog.m_ofn.nFilterIndex = 20;
	Filter += L"|";
	Filter.Replace('|', '\0');
	if (fileName.Find('.') != -1) {
		fileName = fileName.Left(fileName.Find('.'));
	}
	FileDialog.m_ofn.lpstrFilter = Filter;
	FileDialog.m_ofn.lpstrTitle = Title;
	FileDialog.m_ofn.lpstrFile = fileName.GetBuffer(MAX_PATH);
	const LPARAM nResult = FileDialog.DoModal();
	fileName.ReleaseBuffer();
	if (fileName.Find('.') == -1) {
		if (FileDialog.m_ofn.nFilterIndex < 10) {
			fileName += L".dwg";
		} else {
			fileName += L".dxf";
		}
	}
	if (FileDialog.m_ofn.nFilterIndex < 10) {
		m_SaveAsType = OdDb::kDwg;
	} else {
		m_SaveAsType = OdDb::kDxf;
	}
	switch (FileDialog.m_ofn.nFilterIndex) {
		case 1: case 10:
			m_SaveAsVer = OdDb::vAC32; // R32 (2018) release
		case 2: case 11:
			m_SaveAsVer = OdDb::vAC27; // R27 (2013) release
			break;
		case 3: case 12:
			m_SaveAsVer = OdDb::vAC24; // R24 (2010) release
			break;
		case 4: case 13:
			m_SaveAsVer = OdDb::vAC21;  // R21 (2007) release
			break;
		case 5: case 14:
			m_SaveAsVer = OdDb::vAC18; // R18 (2004) release
			break;
		case 6: case 15:
			m_SaveAsVer = OdDb::vAC15; // R15 (2000) release
			break;
		case 7: case 16:
			m_SaveAsVer = OdDb::vAC14; // R14 (release date 1997)
			break;
		case 8: case 17:
			m_SaveAsVer = OdDb::vAC13; // R13 (release date 1994)
			break;
		case 9: case 18:
			m_SaveAsVer = OdDb::vAC12; // R11 & R12 (release date 1990)
			break;
		case 19:
			m_SaveAsVer = OdDb::vAC10; // R10 (release date 1988)
			break;
		case 20:
			m_SaveAsVer = OdDb::vAC09; // R9 (release date 1987) 
			break;
		default:
			m_SaveAsVer = m_DatabasePtr->originalFileVersion();
	}
	return nResult == IDOK;
}

void AeSysDoc::OnEditClearSelection() {
	if (m_DisableClearSelection) { return; }
	auto Cleared {false};
	auto ViewPosition {GetFirstViewPosition()};
	while (ViewPosition != nullptr) {
		const auto View {GetNextView(ViewPosition)};
		if (OdString(View->GetRuntimeClass()->m_lpszClassName).compare(L"AeSysView") == 0 && View->GetDocument() == this) {
			dynamic_cast<AeSysView*>(View)->editorObject().Unselect();
			Cleared = true;
		}
	}
	if (!Cleared) { // No view found
		SelectionSet()->clear();
	}
}

void AeSysDoc::OnEditExplode() {
	ExecuteCommand(L"explode");
}

void AeSysDoc::OnEditEntget() {
	OdDbSelectionSetIteratorPtr SelectionSetIterator {SelectionSet()->newIterator()};
	if (!SelectionSetIterator->done()) {
		auto selId {SelectionSetIterator->objectId()};
		EoDlgEditProperties EditPropertiesDialog(selId, theApp.GetMainWnd());
		m_DatabasePtr->startUndoRecord();
		EditPropertiesDialog.DoModal();
	}
}

void AeSysDoc::OnViewNamedViews() {
	ExecuteCommand(L"VIEW");
}

void AeSysDoc::OnEditUndo() {
	m_LayoutSwitchable = true;
	m_DatabasePtr->undo();
	m_LayoutSwitchable = false;
	UpdateAllViews(nullptr);
}

void AeSysDoc::OnUpdateEditUndo(CCmdUI* commandUserInterface) {
	theApp.RefreshCommandMenu();
	commandUserInterface->Enable(m_DatabasePtr->hasUndo() ? TRUE : FALSE);
}

void AeSysDoc::OnEditRedo() {
	m_LayoutSwitchable = true;
	m_DatabasePtr->redo();
	m_LayoutSwitchable = false;
	UpdateAllViews(nullptr);
}

void AeSysDoc::OnUpdateEditRedo(CCmdUI* commandUserInterface) {
	commandUserInterface->Enable(m_DatabasePtr->hasRedo() ? TRUE : FALSE);
}

void AeSysDoc::OnEditSelectAll() {
	OnEditClearSelection();
	m_DisableClearSelection = true;
	ExecuteCommand(L"select single all");
	m_DisableClearSelection = false;
	auto ViewPosition {GetFirstViewPosition()};
	while (ViewPosition != nullptr) {
		const auto View {GetNextView(ViewPosition)};
		if (CString(View->GetRuntimeClass()->m_lpszClassName).Compare(L"AeSysView") == 0 && View->GetDocument() == this) {
			dynamic_cast<AeSysView*>(View)->editorObject().SelectionSetChanged();
		}
	}
}
