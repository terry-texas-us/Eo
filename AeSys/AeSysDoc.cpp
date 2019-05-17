#include "stdafx.h"

#include <atlbase.h>
#include <strsafe.h>

#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

#include "DbRegAppTable.h"
#include "DbRegAppTableRecord.h"

#include "DbDimStyleTable.h"
#include "DbDimStyleTableRecord.h"
#include "DbTextStyleTable.h"
#include "DbTextStyleTableRecord.h"

#include "Ed/EdLispEngine.h"
#include "DbObjectContextCollection.h"
#include "DbObjectContextManager.h"
#include "SaveState.h"

#include "EoDbHatch.h"
#include "EoDbPolyline.h"
#include "EoDbPegFile.h"
#include "EoDbTracingFile.h"

#include "ColorMapping.h"
#include "EoAppAuditInfo.h"
#include "ExPageController.h"

#include "ExStringIO.h"

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
#include "EoDlgSetLength.h"
#include "EoDlgSetPastePosition.h"
#include "EoDlgSetupColor.h"
#include "EoDlgSetupHatch.h"
#include "EoDlgSetupNote.h"
#include "EoDlgSetupLinetype.h"
#include "EoDlgTrapFilter.h"
#include "lex.h"

#include "OdValue.h"

UINT CALLBACK OFNHookProcFileTracing(HWND, UINT, WPARAM, LPARAM);

UINT AFXAPI HashKey(CString& str) noexcept {
	LPCWSTR pStr = (LPCWSTR) str;
	UINT nHash = 0;
	while (*pStr) {
		nHash = (nHash << 5) + nHash + *pStr++;
	}
	return nHash;
}

#ifdef _DEBUG
// #define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// AeSysDoc

IMPLEMENT_DYNCREATE(AeSysDoc, CDocument)

#define NEW_CONSTR(CLASS) OdSmartPtr<CLASS>(new CLASS, kOdRxObjAttach)

ODRX_CONS_DEFINE_MEMBERS(OdDbDatabaseDoc, OdDbDatabase, NEW_CONSTR);

AeSysDoc* OdDbDatabaseDoc::g_pDoc = 0;

OdDbDatabaseDoc::OdDbDatabaseDoc() noexcept
	: m_pDoc(g_pDoc) {
	g_pDoc = 0;
}
AeSysDoc* OdDbDatabaseDoc::document() const noexcept {
	return m_pDoc;
}
void OdDbDatabaseDoc::setDocToAssign(AeSysDoc * document) noexcept {
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
	ON_COMMAND(ID_TOOLS_PRIMITIVE_SNAPTOENDPOINT, OnToolsPrimitiveSnaptoendpoint)
	ON_COMMAND(ID_PRIM_GOTOCENTERPOINT, OnPrimGotoCenterPoint)
	ON_COMMAND(ID_TOOLS_PRIMITVE_DELETE, OnToolsPrimitiveDelete)
	ON_COMMAND(ID_PRIM_MODIFY_ATTRIBUTES, OnPrimModifyAttributes)
	ON_COMMAND(ID_TOOLS_GROUP_BREAK, OnToolsGroupBreak)
	ON_COMMAND(ID_TOOLS_GROUP_DELETE, OnToolsGroupDelete)
	ON_COMMAND(ID_TOOLS_GROUP_DELETELAST, OnToolsGroupDeletelast)
	ON_COMMAND(ID_TOOLS_GROUP_EXCHANGE, OnToolsGroupExchange)
	ON_COMMAND(ID_TOOLS_GROUP_UNDELETE, OnToolsGroupUndelete)
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
	ON_COMMAND(ID_SETUP_LAYERPROPERTIES, &AeSysDoc::OnSetupLayerproperties)
	ON_COMMAND(ID_INSERT_TRACING, &AeSysDoc::OnInsertTracing)
	ON_COMMAND(ID_FILE_PAGESETUP, &AeSysDoc::OnFilePagesetup)
	ON_COMMAND(ID_VIEW_SETACTIVELAYOUT, &AeSysDoc::OnViewSetactivelayout)
	ON_COMMAND(ID_DRAWINGUTILITIES_AUDIT, &AeSysDoc::OnDrawingutilitiesAudit)

	ON_COMMAND(ID_SELECTIONSETCOMMANDS_CLEAR, &AeSysDoc::OnEditClearselection)
	ON_COMMAND(ID_EDIT_CONSOLE, &AeSysDoc::OnEditConsole)
	ON_COMMAND(ID_VIEW_NAMEDVIEWS, &AeSysDoc::OnViewNamedViews)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_COMMAND(ID_SELECTIONSETCOMMANDS_SELECTALL, &AeSysDoc::OnEditSelectall)
	ON_COMMAND(ID_SELECTIONSETCOMMANDS_ENTGET, &AeSysDoc::OnEditEntget)

	ON_COMMAND(ID_VECTORIZE, &AeSysDoc::OnVectorize)
	ON_UPDATE_COMMAND_UI(ID_VECTORIZE, &AeSysDoc::OnUpdateVectorize)
END_MESSAGE_MAP()

unsigned short AeSysDoc::ClipboardData::m_FormatR15 = (CLIPFORMAT)::RegisterClipboardFormat(L"AutoCAD.r15");
unsigned short AeSysDoc::ClipboardData::m_FormatR16 = (CLIPFORMAT)::RegisterClipboardFormat(L"AutoCAD.r16");
unsigned short AeSysDoc::ClipboardData::m_FormatR17 = (CLIPFORMAT)::RegisterClipboardFormat(L"AutoCAD.r17");
unsigned short AeSysDoc::ClipboardData::m_FormatR18 = (CLIPFORMAT)::RegisterClipboardFormat(L"AutoCAD.r18");
unsigned short AeSysDoc::ClipboardData::m_FormatR19 = (CLIPFORMAT)::RegisterClipboardFormat(L"AutoCAD.r19");

AeSysDoc* g_pDoc = 0;

AeSysDoc::AeSysDoc() noexcept
	: m_bPartial(false)
	, m_pViewer(nullptr)
	, m_SaveAsType(OdDb::kDwg)
	, m_SaveAsType_(EoDb::kUnknown)
	, m_SaveAsVer(OdDb::kDHL_CURRENT)
	, m_bConsole(false)
	, m_bConsoleResponded(false)
	, m_nCmdActive(0)
	, m_bLayoutSwitchable(false)
	, m_bDisableClearSel(false) {
	m_WorkLayer = NULL;
	g_pDoc = this;

	m_pRefDocument = OdApDocumentImpl::createObject(this);
}
// <tas="crash with smart pointer m_DatabasePtr release"\>
AeSysDoc::~AeSysDoc() {
	m_pRefDocument->m_pImp->SetNull();
}

BOOL AeSysDoc::DoSave(LPCWSTR pathName, BOOL replace) {
	m_SaveAsVer = m_DatabasePtr->originalFileVersion();

	CString PathName(pathName);
	if (PathName.IsEmpty()) { // Save As
		CDocTemplate* DocTemplate = GetDocTemplate();

		PathName = m_strPathName;
		if (replace && PathName.IsEmpty()) {
			PathName = m_strTitle;
			const int BadCharacterPosition = PathName.FindOneOf(L" #%;/\\");
			if (BadCharacterPosition != -1) {
				PathName.ReleaseBuffer(BadCharacterPosition);
			}
			CString Extension;
			if (DocTemplate->GetDocString(Extension, CDocTemplate::filterExt) && !Extension.IsEmpty()) {
				ASSERT(Extension[0] == '.');
				PathName += Extension;
			}
		}
		if (!DoPromptFileName(PathName, replace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY, OFN_HIDEREADONLY | OFN_EXPLORER | OFN_PATHMUSTEXIST, FALSE, DocTemplate)) {
			return FALSE; // don't even attempt to save
		}
	}
	CWaitCursor wait;

	if (!OnSaveDocument(PathName)) {
		if (pathName == NULL) {
			TRY {
				CFile::Remove(PathName);
			}
				CATCH_ALL(Errors) {
				theApp.AddStringToMessageList(L"Warning: Failed to delete file <%s> after failed/aborted SaveAs", PathName);
				do {
					Errors->Delete();
				} while (0);
			}
			END_CATCH_ALL
		}
		return FALSE;
	}
	if (replace) {
		SetPathName(PathName);
	}
	return TRUE;
}
void AeSysDoc::DeleteContents() {
	RemoveAllBlocks();
	RemoveAllLayers();
	m_WorkLayer = NULL;
	DeletedGroupsRemoveGroups();

	RemoveAllTrappedGroups();
	RemoveAllGroupsFromAllViews();

	DeleteNodalResources();

	ResetAllViews();

	const size_t NumberOfReactors = theApp.m_aAppReactors.size();
	
	for (size_t ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
		theApp.m_aAppReactors[ReactorIndex]->documentToBeDestroyed(this);
	}
	if (!m_DatabasePtr.isNull()) {
		m_DatabasePtr->appServices()->layoutManager()->removeReactor(this);
	}
	// <tas="crash with smart pointer m_DatabasePtr release"> m_DatabasePtr.release();"</tas>
	m_DatabasePtr.release();

	COleDocument::DeleteContents();

	for (size_t ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
		theApp.m_aAppReactors[ReactorIndex]->documentDestroyed((const wchar_t*) GetPathName());
	}
}

// <command_view>
BOOL AeSysDoc::CanCloseFrame(CFrameWnd * frame) {
	CView* ActiveView = frame->GetActiveView();
	if (ActiveView->IsKindOf(&AeSysView::classAeSysView)) {
		if (!static_cast<AeSysView*>(ActiveView)->canClose()) {
			return FALSE;
		}
	}
	return CDocument::CanCloseFrame(frame);
}
// </command_view>

AeSysView* AeSysDoc::getViewer() noexcept {
	return m_pViewer;
}

void AeSysDoc::OnViewSetactivelayout() {
	EoDlgSetActiveLayout ActiveLayoutDialog(m_DatabasePtr, theApp.GetMainWnd());
	m_bLayoutSwitchable = true;
	if (ActiveLayoutDialog.DoModal() == IDOK) {
		try {
			m_DatabasePtr->startUndoRecord();
			m_DatabasePtr->setCurrentLayout(OdString(ActiveLayoutDialog.m_sNewLayoutName));
		} catch (const OdError & Error) {
			theApp.reportError(L"Error Setting Layout...", Error);
			m_DatabasePtr->disableUndoRecording(true);
			m_DatabasePtr->undo();
			m_DatabasePtr->disableUndoRecording(false);
		}
	}
	m_bLayoutSwitchable = false;
}
void AeSysDoc::layoutSwitched(const OdString & newLayoutName, const OdDbObjectId & newLayout) {
	// AMark : Prevent Zoom/Rotate crashes
	// AMark : Prevent Undo/Redo crashes
	if (m_bLayoutSwitchable) {
		// This test can be exchanged by remove/add reactor in layout manager, but this operations must be added
		// into all functions which can call setCurrentLayout (but where vectorization no need to be changed).
		POSITION pos = GetFirstViewPosition();
		while (pos != NULL) {
			const CView* view = GetNextView(pos);
			if (CString(view->GetRuntimeClass()->m_lpszClassName).Compare(L"AeSysView") == 0) {

				if (view->GetDocument() == this) {
					const CWnd* pParent = view->GetParent();
					// Get prev params
					const bool bIconic = pParent->IsIconic() != FALSE;
					const bool bZoomed = pParent->IsZoomed() != FALSE;
					CRect wRect;
					pParent->GetWindowRect(&wRect);
					POINT point1, point2;
					point1.x = wRect.left;
					point1.y = wRect.top;
					point2.x = wRect.right;
					point2.y = wRect.bottom;
					pParent->GetParent()->ScreenToClient(&point1);
					pParent->GetParent()->ScreenToClient(&point2);
					wRect.left = point1.x;
					wRect.top = point1.y;
					wRect.right = point2.x;
					wRect.bottom = point2.y;
					//
					view->GetParent()->SendMessage(WM_CLOSE);
					OnVectorize();
					// Search again for new view
					POSITION pos = GetFirstViewPosition();

					while (pos != NULL) {
						const CView* view = GetNextView(pos);
						if (CString(view->GetRuntimeClass()->m_lpszClassName).Compare(L"AeSysView") == 0) {
							if (view->GetDocument() == this) {
								CWnd* pParent = view->GetParent();
								if (bZoomed) {
									if (!pParent->IsZoomed()) {
										reinterpret_cast<CMDIChildWnd*>(pParent)->MDIMaximize();
									}
								} else {
									reinterpret_cast<CMDIChildWnd*>(pParent)->MDIRestore();
									if (!bIconic) {
										pParent->SetWindowPos(NULL, wRect.left, wRect.top, wRect.right - wRect.left, wRect.bottom - wRect.top, SWP_NOZORDER);
									}
								}
								break;
							}
						}
					}
				}
			}
		}
	}
}

const OdString Cmd_VIEW::groupName() const { return L"AeSysApp"; }
const OdString Cmd_VIEW::name() { return L"VIEW"; }
const OdString Cmd_VIEW::globalName() const { return name(); }

void Cmd_VIEW::execute(OdEdCommandContext * commandContext) {
	OdDbCommandContextPtr CommandContext(commandContext);
	OdDbDatabaseDocPtr Database = CommandContext->database();

	EoDlgNamedViews NamedViewsDialog(Database->document(), theApp.GetMainWnd());

	if (NamedViewsDialog.DoModal() != IDOK) {
		throw OdEdCancel();
	}
}

const OdString Cmd_SELECT::groupName() const { return L"AeSysApp"; }
const OdString Cmd_SELECT::name() { return L"SELECT"; }
const OdString Cmd_SELECT::globalName() const { return name(); }

void Cmd_SELECT::execute(OdEdCommandContext * commandContext) {
	OdDbCommandContextPtr CommandContext(commandContext);
	OdDbDatabaseDocPtr Database = CommandContext->database();

	auto Document {Database->document()};
	auto pView {Document->getViewer()};

	if (pView == NULL) {
		throw OdEdCancel();
	}
	Document->OnEditClearselection();
	Document->UpdateAllViews(nullptr);
	OdDbUserIO* pIO = CommandContext->dbUserIO();
	pIO->setPickfirst(0);
	int iOpt = OdEd::kSelLeaveHighlighted | OdEd::kSelAllowEmpty;

	OdDbSelectionSetPtr SelectionSet;
	try {
		SelectionSet = pIO->select(OdString::kEmpty, iOpt, pView->editorObject().workingSSet());
		pView->editorObject().setWorkingSSet(SelectionSet);
	} catch (const OdError&) {
		throw OdEdCancel();
	}
	pView->editorObject().selectionSetChanged();

	Database->pageObjects();
}

struct CDocTemplateEx : CDocTemplate {
	void SetViewToCreate(CRuntimeClass* viewClass) noexcept {
		m_pViewClass = viewClass;
	}
};
void AeSysDoc::OnVectorize(const OdString & vectorizerPath) {
	// <tas="likely misused in AeSys environment"</tas>
	theApp.setRecentGsDevicePath(vectorizerPath);

	CDocTemplateEx* DocTemplate = (CDocTemplateEx*) GetDocTemplate();
	ASSERT_VALID(DocTemplate);

	DocTemplate->SetViewToCreate(RUNTIME_CLASS(AeSysView));
	CFrameWnd* NewFrame = DocTemplate->CreateNewFrame(this, NULL);

	DocTemplate->InitialUpdateFrame(NewFrame, this);

	m_pViewer = dynamic_cast<AeSysView*>(NewFrame->GetActiveView());
}

void AeSysDoc::OnCloseVectorizer(AeSysView * view) {
	if (view != m_pViewer) {
		ATLTRACE2(atlTraceGeneral, 0, L"Vectorizer does not match expected viewer\n");
	}
	m_pViewer = nullptr;
}
void AeSysDoc::setVectorizer(AeSysView * view) {
	// <tas="limits the command context to a single view per document. So New window crashes."/>"
	ODA_ASSERT(m_pViewer == nullptr);
	m_pViewer = view;
}
void AeSysDoc::OnVectorize() {
	OnVectorize(theApp.recentGsDevicePath());
}
void AeSysDoc::OnUpdateVectorize(CCmdUI * pCmdUI) {
	pCmdUI->Enable(m_pViewer == nullptr && !theApp.recentGsDevicePath().isEmpty());
}

OdDbCommandContextPtr AeSysDoc::cmdCtx() {
	if (m_pCmdCtx.isNull()) {
		m_pCmdCtx = ExDbCommandContext::createObject(cmdIO(), m_DatabasePtr);
	}
	return m_pCmdCtx;
}

OdDbSelectionSetPtr AeSysDoc::selectionSet() const {
	OdDbCommandContext* CommandContext = const_cast<AeSysDoc*>(this)->cmdCtx();
	OdDbSelectionSetPtr SelectionSet {CommandContext->arbitraryData(L"OdaMfcApp Working Selection Set")};

	if (SelectionSet.isNull()) {
		SelectionSet = OdDbSelectionSet::createObject(m_DatabasePtr);
		CommandContext->setArbitraryData(L"OdaMfcApp Working Selection Set", SelectionSet);
	}
	ATLTRACE2(atlTraceGeneral, 0, L"Working Selection set contains %d items\n", SelectionSet->numEntities());
	return SelectionSet;
}

OdEdBaseIO* AeSysDoc::cmdIO() noexcept {
	return (this);
}

EoDlgUserIOConsole* AeSysDoc::console() {
	
	if (m_pConsole.isNull()) {
		m_pConsole = EoDlgUserIOConsole::create(theApp.GetMainWnd());
	}
	return m_pConsole;
}

OdUInt32 AeSysDoc::getKeyState() noexcept {
	OdUInt32 KeyState(0);
	if (::GetKeyState(VK_CONTROL) != 0) { KeyState |= MK_CONTROL; }

	if (::GetKeyState(VK_SHIFT) != 0) { KeyState |= MK_SHIFT; }

	return (KeyState);
}

OdGePoint3d AeSysDoc::getPoint(const OdString & prompt, int options, OdEdPointTracker * tracker) {
	if (m_pMacro.get() && !m_pMacro->isEof()) {
		OdString strRes = getString(prompt, options, 0);
		throw OdEdOtherInput(strRes);
	}
	if (m_bConsole) { return m_pConsole->getPoint(prompt, options, tracker); }

	if (m_pViewer) {
		console()->putString(prompt);
		return m_pViewer->getPoint(prompt, options, tracker);
	}
	return console()->getPoint(prompt, options, tracker);
}

OdString AeSysDoc::getString(const OdString & prompt, int options, OdEdStringTracker * tracker) {
	OdString sRes;
	if (m_pMacro.get() && !m_pMacro->isEof()) {
		sRes = m_pMacro->getString(prompt, options, tracker);
		putString(prompt + L" " + sRes);
		return sRes;
	}
	if (m_bConsole) { return console()->getString(prompt, options, tracker); }

	if (m_pViewer) {
		m_bConsoleResponded = prompt.isEmpty();
		sRes = m_pViewer->getString(prompt, options, tracker);

		if (!m_bConsoleResponded) { putString(OdString(prompt) + L" " + sRes); }

		return sRes;
	}
	return console()->getString(prompt, options, tracker);
}

void AeSysDoc::putString(const OdString & string) {
	if (m_pViewer) { m_pViewer->putString(string); }

	console()->putString(string);
}

OdString AeSysDoc::recentCmd() {
	return theApp.getRecentCmd();
}

OdString AeSysDoc::recentCmdName() {
	return theApp.getRecentCmd().spanExcluding(L" \n");
}

OdString AeSysDoc::commandPrompt() {
	return L"Command:";
}

void AeSysDoc::OnEditConsole() {
	auto CommandStack {::odedRegCmds()};
	OdDbCommandContextPtr CommandContext(cmdCtx());
	OdSaveState<bool> saveConsoleMode(m_bConsole, true);

	try {
		if (m_pViewer && m_pViewer->isGettingString()) {

			m_pViewer->respond(console()->getString(m_pViewer->prompt(), m_pViewer->inpOptions(), 0));
			m_bConsoleResponded = true;

		} else {
			for (;;) {
				OdString CommandName = CommandContext->userIO()->getString(commandPrompt(), 0, L"");
				if (CommandName.isEmpty()) {
					CommandName = recentCmdName();

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

OdString commandMessageCaption(const OdString & command) {
	OdString Caption;
	Caption.format(L"Command: %ls", command.c_str());
	return Caption;
}

class CmdReactor
	: public OdStaticRxObject<OdEdCommandStackReactor>
	, public OdStaticRxObject<OdDbDatabaseReactor> {
	ODRX_NO_HEAP_OPERATORS();
	OdDbCommandContext* m_pCmdCtx;
	bool m_bModified;
	OdString m_sLastInput;

	void setModified() {
		m_bModified = true;
		m_pCmdCtx->database()->removeReactor(this);
	}

public:

	CmdReactor(OdDbCommandContext* dbCommandContext)
		: m_pCmdCtx(dbCommandContext)
		, m_bModified(false) {
		ODA_ASSERT(m_pCmdCtx);
		::odedRegCmds()->addReactor(this);
		m_pCmdCtx->database()->addReactor(this);
	}

	~CmdReactor() {
		::odedRegCmds()->removeReactor(this);

		if (!m_bModified) {
			m_pCmdCtx->database()->removeReactor(this);
		}
	}

	void setLastInput(const OdString& sLastInput) {
		m_sLastInput = sLastInput;
	}
	
	const OdString& lastInput() const noexcept {
		return m_sLastInput;
	}
	
	bool isDatabaseModified() const noexcept {
		return m_bModified;
	}

	void objectOpenedForModify(const OdDbDatabase*, const OdDbObject*) override {
		setModified();
	}

	void headerSysVarWillChange(const OdDbDatabase*, const char*) {
		setModified();
	}

	OdEdCommandPtr unknownCommand(const OdString& commandName, OdEdCommandContext* commandContext) override {
		AeSysView* pViewer = OdDbDatabaseDocPtr(m_pCmdCtx->database())->document()->getViewer();
		if (pViewer) {
			OdEdCommandPtr pRes = pViewer->command(commandName);
			if (pRes.get()) {
				return pRes;
			}
		}
		OdString String;
		String.format(L"Unknown command \"%ls\".", commandName.c_str());
		m_pCmdCtx->userIO()->putString(String);
		return OdEdCommandPtr();
	}

	void commandWillStart(OdEdCommand* pCmd, OdEdCommandContext* /*edCommandContext*/) override {
		m_sLastInput.makeUpper();

		if (!GETBIT(pCmd->flags(), OdEdCommand::kNoHistory)) {
			theApp.setRecentCmd(m_sLastInput);
		}
		if (!GETBIT(pCmd->flags(), OdEdCommand::kNoUndoMarker)) {
			m_pCmdCtx->database()->startUndoRecord();
		}
	}

	void commandCancelled(OdEdCommand*, OdEdCommandContext*) override {
		undoCmd();
	}

	void commandFailed(OdEdCommand*, OdEdCommandContext*) override {
		undoCmd();
	}
private:
	void undoCmd() {
		auto pDb {m_pCmdCtx->database()};
		try {
			pDb->disableUndoRecording(true);
			pDb->undo();
			pDb->disableUndoRecording(false);
		} catch (const OdError & Error) {
			theApp.reportError(L"Can't repair database", Error);
		}
#ifndef _DEBUG
		catch (...) {
			::MessageBoxW(NULL, L"Unknown error occurred...", L"Can't repair database", MB_OK | MB_ICONERROR);
		}
#endif //_DEBUG
	}
};

void AeSysDoc::ExecuteCommand(const OdString& command, bool echo) {
	OdSaveState<int> save_m_nCmdActive(m_nCmdActive);
	++m_nCmdActive;

	OdDbCommandContextPtr CommandContext(cmdCtx());

	CmdReactor CommandReactor(CommandContext);

	try {
		auto CommandStack {::odedRegCmds()};

		ExDbCommandContext* pExCmdCtx = dynamic_cast<ExDbCommandContext*>(CommandContext.get());
		
		if (m_DatabasePtr->appServices()->getPICKFIRST()) {
			pExCmdCtx->setPickfirst(selectionSet());
		}
		if (command[0] == '(') {
			OdEdLispModulePtr lspMod = odrxDynamicLinker()->loadApp(OdLspModuleName);
			if (!lspMod.isNull())
				lspMod->createLispEngine()->execute(pExCmdCtx, command);
		} else {
			OdString s = command.spanExcluding(L" \t\r\n");
			if (s.getLength() == command.getLength()) {
				if (echo) {
					CommandContext->userIO()->putString(commandPrompt() + L" " + s);
				}
				s.makeUpper();
				CommandReactor.setLastInput(s);
				CommandStack->executeCommand(s, CommandContext);
			} else {
				m_pMacro = ExStringIO::create(command);
				while (!m_pMacro->isEof()) {
					try {
						s = CommandContext->userIO()->getString(commandPrompt());
						s.makeUpper();
						CommandReactor.setLastInput(s);
					} catch (const OdEdEmptyInput) {
						s = recentCmdName();
					}
					CommandStack->executeCommand(s, CommandContext);
				}
			}
		}
		if (getViewer()) {
			getViewer()->propagateActiveViewChanges();
		}
	} catch (const OdEdEmptyInput) {
	} catch (const OdEdCancel) {
	} catch (const OdError & err) {

		if (!m_bConsole) {
			theApp.reportError(commandMessageCaption(command), err);
		}
		cmdIO()->putString(err.description());
	}
	if ((CommandReactor.isDatabaseModified() || selectionSet()->numEntities())) {

		if (0 != CommandReactor.lastInput().iCompare(L"SELECT") || CommandContext->database()->appServices()->getPICKADD() != 2) {
			OnEditClearselection();
		}
		UpdateAllViews(nullptr);
	}
}

BOOL AeSysDoc::OnCmdMsg(UINT commandId, int messageCategory, void* commandObject, AFX_CMDHANDLERINFO * handlerInfo) {
	if (handlerInfo == NULL) {
		CMenu* TopMenu = CMenu::FromHandle(theApp.GetAeSysMenu());
		if (TopMenu) { // Check if it is a theApp's dynamic menu item
			MENUITEMINFO MenuItemInfo;
			MenuItemInfo.cbSize = sizeof(MenuItemInfo);
			MenuItemInfo.fMask = MIIM_DATA;
			if (TopMenu->GetMenuItemInfoW(commandId, &MenuItemInfo, FALSE)) {
				// <tas="Will not use. Need to decide if/how to select a vectorizer. Possible OpenGL is a desired option to Directx">
				if (MenuItemInfo.dwItemData == theApp.getGSMenuItemMarker()) {
					CString Vectorizer;
					TopMenu->GetSubMenu(3)->GetMenuStringW(commandId, Vectorizer, MF_BYCOMMAND);
					if (messageCategory == CN_COMMAND) {
						OnVectorize((LPCWSTR) Vectorizer);
					} else if (messageCategory == CN_UPDATE_COMMAND_UI) {
						((CCmdUI*) commandObject)->Enable(m_pViewer == nullptr);
						((CCmdUI*) commandObject)->SetCheck(Vectorizer == (LPCWSTR) theApp.recentGsDevicePath());
					}
					return TRUE;
				}
				// </tas>
				if (commandId >= _APS_NEXT_COMMAND_VALUE + 100 && commandId <= _APS_NEXT_COMMAND_VALUE + theApp.numCustomCommands() + 100) { // custom commands
					OdRxObjectPtr ItemData(reinterpret_cast<OdRxObject*>(MenuItemInfo.dwItemData));
					if (ItemData.get()) {
						if (messageCategory == CN_COMMAND) {
							OdEdCommandPtr EdCommand = OdEdCommand::cast(ItemData);
							if (EdCommand.get()) {
								ExecuteCommand(EdCommand->globalName());
								return TRUE;
							}
						} else if (messageCategory == CN_UPDATE_COMMAND_UI) {
							((CCmdUI*) commandObject)->Enable(TRUE);
						}
						return TRUE;
					}
				} else if (commandId >= _APS_NEXT_COMMAND_VALUE && commandId < _APS_NEXT_COMMAND_VALUE + 100) { // annotation scales
					if (messageCategory == CN_COMMAND) {
						const int SelectedScale = commandId - _APS_NEXT_COMMAND_VALUE - 1;
						OdDbObjectContextCollectionIteratorPtr ScalesCollectionIterator = m_DatabasePtr->objectContextManager()->contextCollection(ODDB_ANNOTATIONSCALES_COLLECTION)->newIterator();
						for (int ScaleIndex = 0; !ScalesCollectionIterator->done(); ScalesCollectionIterator->next()) {
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
						((CCmdUI*) commandObject)->Enable(TRUE);
					}
					return TRUE;
				}
			}
		}
	}
	return COleDocument::OnCmdMsg(commandId, messageCategory, commandObject, handlerInfo);
}

void AeSysDoc::DeleteSelection(bool force) {

	if (m_DatabasePtr->appServices()->getPICKFIRST() && selectionSet()->numEntities()) {

		if (force) {
			ExecuteCommand(L"ForceErase");
		} else {
			ExecuteCommand(L"erase");
		}
		if (m_pViewer) {
			m_pViewer->editorObject().Set_Entity_centers();
		}
	}
}

void AeSysDoc::startDrag(const OdGePoint3d & point) {
	DataSource ds;
	ds.Create(this, point);
	if (ds.DoDragDrop()) {
		UpdateAllViews(nullptr);
	}
}

OdDbTextStyleTableRecordPtr AeSysDoc::AddNewTextStyle(OdString name, OdDbTextStyleTablePtr & textStyles) {
	OdDbTextStyleTableRecordPtr TextStyle = OdDbTextStyleTableRecord::createObject();

	try {
		TextStyle->setName(name);
		textStyles->add(TextStyle);
	} catch (const OdError & Error) {
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
	OdDbDimStyleTableRecordPtr DimStyle = OdDbDimStyleTableRecord::createObject();
	DimStyle->setName(L"EoStandard");
	OdDbObjectId dimStyleId = DimStyleTable->add(DimStyle);

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
		auto TextStyle {TextStyles->getAt(L"EoStandard")};
		DimStyle->setDimtxsty(TextStyle);
	}

	DimStyle->setDimasz(0.);

	//	DimStyle->setDimblk(L"_None");
	
	return DimStyle;
}

void AeSysDoc::AddRegisteredApp(const OdString & name) {
	OdDbRegAppTablePtr RegisteredApps = m_DatabasePtr->getRegAppTableId().safeOpenObject(OdDb::kForWrite);
	if (!RegisteredApps->has(name)) {
		OdDbRegAppTableRecordPtr RegisteredApp = OdDbRegAppTableRecord::createObject();
		try {
			RegisteredApp->setName(name);
			RegisteredApps->add(RegisteredApp);
		} catch (const OdError & Error) {
			RegisteredApp->erase();
			theApp.reportError(L"ODA Error - AeSysDoc::AddRegisteredApp", Error);
		}
	}
}

BOOL AeSysDoc::OnNewDocument() {
	const size_t NumberOfReactors = theApp.m_aAppReactors.size();

	for (size_t ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
		theApp.m_aAppReactors[ReactorIndex]->documentCreateStarted(this);
	}
	if (COleDocument::OnNewDocument()) {
		OdDbDatabaseDoc::setDocToAssign(this);
		try { // create *database* populated with the default set of objects(all tables, ModelSpace and PaperSpace blocks etc.)
			m_DatabasePtr = theApp.createDatabase(true, OdDb::kEnglish);
		} catch (const OdError & Error) {
			m_DatabasePtr = nullptr;
			theApp.reportError(L"Database Creating Error...", Error);
			return FALSE;
		}
		OdDbTextStyleTableRecordPtr TextStyle = AddStandardTextStyle();
		m_DatabasePtr->setTEXTSTYLE(TextStyle->objectId());

		AddStandardDimensionStyle();

		AddRegisteredApp(L"AeSys");

		m_DatabasePtr->startUndoRecord();

		const ODCOLORREF* DarkPalette = odcmAcadDarkPalette();
		ODCOLORREF LocalDarkPalette[256];
		memcpy(LocalDarkPalette, DarkPalette, 256 * sizeof(ODCOLORREF));

		EoDbDwgToPegFile DwgToPegFile(m_DatabasePtr);

		DwgToPegFile.ConvertToPeg(this);

		// <tas="Continuous Linetype initialization ??"</tas>
		m_LinetypeTable.LoadLinetypesFromTxtFile(m_DatabasePtr, AeSysApp::ResourceFolderPath() + L"Pens\\Linetypes.txt");

		m_SaveAsType_ = EoDb::kPeg;
		SetCurrentLayer(m_DatabasePtr->getCLAYER().safeOpenObject());

		InitializeGroupAndPrimitiveEdit();


		if (!m_DatabasePtr.isNull()) {
			m_DatabasePtr->appServices()->layoutManager()->addReactor(this);
		}
		for (size_t ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
			theApp.m_aAppReactors[ReactorIndex]->documentCreated(this);
		}
		return TRUE;
	}
	for (size_t ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
		theApp.m_aAppReactors[ReactorIndex]->documentCreateCanceled(this);
	}
	return FALSE;
}

BOOL AeSysDoc::OnOpenDocument(LPCWSTR file) {
	OdDbDatabaseDoc::setDocToAssign(this);
	EoDb::FileTypes FileType = AeSysApp::GetFileType(file);

	switch (FileType) {
		case EoDb::kDwg:
		case EoDb::kDxf:
		{
			m_DatabasePtr = theApp.readFile(file, false, false);

			//<tas="disable lineweight display until lineweight by default is properly defined"</tas>
			if (m_DatabasePtr->getLWDISPLAY()) m_DatabasePtr->setLWDISPLAY(false);

			OdDbTextStyleTableRecordPtr TextStyle = AddStandardTextStyle();

			AddStandardDimensionStyle();

			m_DatabasePtr->startUndoRecord();

			CString FileAndVersion;
			FileAndVersion.Format(L"Opened <%s> (Version: %d)\n", (LPCWSTR) m_DatabasePtr->getFilename(), m_DatabasePtr->originalFileVersion());
			theApp.AddStringToMessageList(FileAndVersion);

			EoDbDwgToPegFile DwgToPegFile(m_DatabasePtr);

			DwgToPegFile.ConvertToPeg(this);
			m_SaveAsType_ = FileType;
			SetCurrentLayer(m_DatabasePtr->getCLAYER().safeOpenObject());

			break;
		}
		case EoDb::kPeg:
		{
			m_DatabasePtr = theApp.createDatabase(true, OdDb::kEnglish);

			OdDbTextStyleTableRecordPtr TextStyle = AddStandardTextStyle();

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
		case EoDb::kTracing:
		case EoDb::kJob:
			TracingOpen(file);
			break;
		default:
			return CDocument::OnOpenDocument(file);
	}
	return TRUE;
}

BOOL AeSysDoc::OnSaveDocument(LPCWSTR pathName) {
	BOOL ReturnStatus = FALSE;

	switch (m_SaveAsType_) {
		case EoDb::kPeg:
		{
			// <tas="shadow files disabled"/> WriteShadowFile();
			EoDbPegFile DwgToPegFile(m_DatabasePtr);
			CFileException e;
			if (DwgToPegFile.Open(pathName, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive, &e)) {
				DwgToPegFile.Unload(this);
				ReturnStatus = TRUE;
			}
			break;
		}
		case EoDb::kJob:
		{
			EoDbLayer* Layer = GetLayerAt(pathName);
			if (Layer != 0) {

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
		case EoDb::kTracing:
		{
			EoDbLayer* Layer = GetLayerAt(pathName);
			if (Layer != 0) {
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
		{
			m_DatabasePtr->writeFile(pathName, OdDb::kDxf, OdDb::kDHL_PRECURR);
			ReturnStatus = TRUE;
			break;
		}
		case EoDb::kDwg:
		{
			m_DatabasePtr->writeFile(pathName, OdDb::kDwg, OdDb::kDHL_PRECURR);
			ReturnStatus = TRUE;
			break;
		}
		default:
			theApp.WarningMessageBox(IDS_MSG_NOTHING_TO_SAVE);
	}
	return ReturnStatus;
}
// AeSysDoc diagnostics

#ifdef _DEBUG
void AeSysDoc::AssertValid() const {
	COleDocument::AssertValid();
}
void AeSysDoc::Dump(CDumpContext & dc) const {
	COleDocument::Dump(dc);
}
#endif //_DEBUG

void AeSysDoc::UpdateGroupInAllViews(LPARAM hint, EoDbGroup * group) {
	CDocument::UpdateAllViews(nullptr, hint, static_cast<CObject*>(group));
}
void AeSysDoc::UpdateGroupsInAllViews(LPARAM hint, EoDbGroupList * groups) {
	CDocument::UpdateAllViews(nullptr, hint, static_cast<CObject*>(groups));
}
void AeSysDoc::UpdateLayerInAllViews(LPARAM hint, EoDbLayer * layer) {
	CDocument::UpdateAllViews(nullptr, hint, static_cast<CObject*>(layer));
}
void AeSysDoc::UpdatePrimitiveInAllViews(LPARAM hint, EoDbPrimitive * primitive) {
	CDocument::UpdateAllViews(nullptr, hint, static_cast<CObject*>(primitive));
}

void AeSysDoc::AddTextBlock(LPWSTR text) {
	const OdGePoint3d ptPvt = theApp.GetCursorPosition();

	EoDbFontDefinition FontDefinition = pstate.FontDefinition();
	const EoDbCharacterCellDefinition CharacterCellDefinition = pstate.CharacterCellDefinition();
	EoGeReferenceSystem ReferenceSystem(ptPvt, AeSysView::GetActiveView(), CharacterCellDefinition);
	OdGeVector3d PlaneNormal;
	ReferenceSystem.GetUnitNormal(PlaneNormal);

	OdDbBlockTableRecordPtr BlockTableRecord = m_DatabasePtr->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

	LPWSTR NextToken {nullptr};
	LPWSTR pText = wcstok_s(text, L"\r", &NextToken);
	while (pText != 0) {
		if (wcslen(pText) > 0) {
			EoDbGroup* Group = new EoDbGroup;

			OdDbTextPtr Text = EoDbText::Create(BlockTableRecord, ReferenceSystem.Origin(), (LPCWSTR) pText);

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
		ReferenceSystem.SetOrigin(text_GetNewLinePos(FontDefinition, ReferenceSystem, 1., 0));
		pText = wcstok_s(nullptr, L"\r", &NextToken);
		if (pText == 0)
			break;
		pText++;
	}
}

POSITION AeSysDoc::DeletedGroupsAddHead(EoDbGroup * group) {
	m_DatabasePtr->disableUndoRecording(false);
	group->Erase();
	m_DatabasePtr->disableUndoRecording(true);

	return (m_DeletedGroupList.AddHead(group));
}

POSITION AeSysDoc::DeletedGroupsAddTail(EoDbGroup * group) {
	m_DatabasePtr->disableUndoRecording(false);
	group->Erase();
	m_DatabasePtr->disableUndoRecording(true);

	return (m_DeletedGroupList.AddTail(group));
}

EoDbGroup* AeSysDoc::DeletedGroupsRemoveHead() {
	EoDbGroup* Group = NULL;
	if (!m_DeletedGroupList.IsEmpty()) {
		Group = m_DeletedGroupList.RemoveHead();
		Group->UndoErase();
	}
	return (Group);
}
void AeSysDoc::DeletedGroupsRemoveGroups() {
	m_DeletedGroupList.DeleteGroupsAndRemoveAll();
}
EoDbGroup* AeSysDoc::DeletedGroupsRemoveTail() {
	EoDbGroup* Group = NULL;
	if (!m_DeletedGroupList.IsEmpty()) {
		Group = m_DeletedGroupList.RemoveTail();
		Group->UndoErase();
	}
	return (Group);
}
void AeSysDoc::DeletedGroupsRestore() {
	// <tas="UndoErase group is restored to original layer. If this is desired behavior need to revise AddWorkLayerGroup call."</tas>
	if (!m_DeletedGroupList.IsEmpty()) {
		EoDbGroup* Group = DeletedGroupsRemoveTail();
		AddWorkLayerGroup(Group);
		UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
	}
}

int AeSysDoc::LinetypeIndexReferenceCount(OdInt16 linetypeIndex) {
	int Count = 0;

	for (int LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);
		Count += Layer->GetLinetypeIndexRefCount(linetypeIndex);
	}
	CString Key;
	EoDbBlock* Block;

	POSITION Position = m_BlockTable.GetStartPosition();
	while (Position != NULL) {
		m_BlockTable.GetNextAssoc(Position, Key, Block);
		Count += Block->GetLinetypeIndexRefCount(linetypeIndex);
	}
	return (Count);
}
void AeSysDoc::GetExtents___(AeSysView * view, OdGeExtents3d & extents) {

	for (int LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);
		if (!Layer->IsOff()) {
			Layer->GetExtents__(view, extents);
		}
	}
}
int AeSysDoc::NumberOfGroupsInWorkLayer() {
	int iCount = 0;

	for (int LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		const EoDbLayer* Layer = GetLayerAt(LayerIndex);
		if (Layer->IsCurrent()) {
			iCount += Layer->GetCount();
		}
	}
	return iCount;
}
int AeSysDoc::NumberOfGroupsInActiveLayers() {
	int iCount = 0;

	for (int LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		const EoDbLayer* Layer = GetLayerAt(LayerIndex);
		if (Layer->IsActive()) {
			iCount += Layer->GetCount();
		}
	}
	return iCount;
}

void AeSysDoc::BuildVisibleGroupList(AeSysView* view) {
	RemoveAllGroupsFromAllViews();

	for (int LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		auto Layer {GetLayerAt(LayerIndex)};
		Layer->BuildVisibleGroupList(view);
	}
}


void AeSysDoc::DisplayAllLayers(AeSysView* view, CDC* deviceContext) {
	try {
		const auto IdentifyTrap {theApp.IsTrapHighlighted() && !IsTrapEmpty()};

		RemoveAllGroupsFromAllViews();

		const auto BackgroundColor {deviceContext->GetBkColor()};

		deviceContext->SetBkColor(ViewBackgroundColor);

		const int PrimitiveState = pstate.Save();

		for (int LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
			EoDbLayer* Layer = GetLayerAt(LayerIndex);
			Layer->Display_(view, deviceContext, IdentifyTrap);
		}
		pstate.Restore(deviceContext, PrimitiveState);

		deviceContext->SetBkColor(BackgroundColor);
	} catch (CException * Exception) {
		Exception->Delete();
	}
}

OdDbObjectId AeSysDoc::AddLayerTo(OdDbLayerTablePtr layers, EoDbLayer * layer) {
	m_LayerTable.Add(layer);

	return (layers->add(layer->TableRecord()));
}
void AeSysDoc::AddLayer(EoDbLayer * layer) {
	m_LayerTable.Add(layer);
}
int AeSysDoc::GetLayerTableSize() const {
	return m_LayerTable.GetSize();
}
EoDbLayer* AeSysDoc::GetLayerAt(const OdString & name) {
	const int i = FindLayerAt(name);
	return (i < 0 ? (EoDbLayer*) 0 : m_LayerTable.GetAt(i));
}
EoDbLayer* AeSysDoc::GetLayerAt(int layerIndex) {
	return (layerIndex >= (int) m_LayerTable.GetSize() ? (EoDbLayer*) NULL : m_LayerTable.GetAt(layerIndex));
}
int AeSysDoc::FindLayerAt(const OdString & name) const {
	for (OdUInt16 LayerIndex = 0; LayerIndex < m_LayerTable.GetSize(); LayerIndex++) {
		const EoDbLayer* Layer = m_LayerTable.GetAt(LayerIndex);
		if (name.iCompare(Layer->Name()) == 0) {
			return (LayerIndex);
		}
	}
	return (-1);
}
OdDbLayerTablePtr AeSysDoc::LayerTable(OdDb::OpenMode openMode) {
	return (m_DatabasePtr->getLayerTableId().safeOpenObject(openMode));
}
void AeSysDoc::RemoveAllLayers() {
	for (OdUInt16 LayerIndex = 0; LayerIndex < m_LayerTable.GetSize(); LayerIndex++) {
		EoDbLayer* Layer = m_LayerTable.GetAt(LayerIndex);
		if (Layer) {
			Layer->DeleteGroupsAndRemoveAll();
			delete Layer;
		}
	}
	m_LayerTable.RemoveAll();
}
void AeSysDoc::RemoveLayerAt(int layerIndex) {
	EoDbLayer* Layer = GetLayerAt(layerIndex);

	Layer->DeleteGroupsAndRemoveAll();
	delete Layer;

	m_LayerTable.RemoveAt(layerIndex);
}
void AeSysDoc::RemoveEmptyLayers() {
	for (int LayerIndex = GetLayerTableSize() - 1; LayerIndex > 0; LayerIndex--) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);

		if (Layer && Layer->IsEmpty()) {
			Layer->DeleteGroupsAndRemoveAll();
			delete Layer;
			m_LayerTable.RemoveAt(LayerIndex);
		}
	}
}
bool AeSysDoc::LayerMelt(OdString & name) {
	EoDbLayer* Layer = GetLayerAt(name);
	if (Layer == 0)
		return false;

	bool bRetVal = false;

	OPENFILENAME of;
	::ZeroMemory(&of, sizeof(OPENFILENAME));
	of.lStructSize = sizeof(OPENFILENAME);
	of.hwndOwner = 0;
	of.hInstance = theApp.GetInstance();
	of.lpstrFilter = L"Tracing Files\0*.tra;*.jb1\0\0";
	of.lpstrFile = new wchar_t[MAX_PATH];
	wcscpy_s(of.lpstrFile, MAX_PATH, name);
	of.nMaxFile = MAX_PATH;
	of.lpstrTitle = L"Melt As";
	of.Flags = OFN_OVERWRITEPROMPT;
	of.lpstrDefExt = L"tra";

	if (GetSaveFileNameW(&of)) {
		name = of.lpstrFile;

		const EoDb::FileTypes FileType = AeSysApp::GetFileType(name);
		if (FileType == EoDb::kTracing || FileType == EoDb::kJob) {
			if (FileType == EoDb::kJob) {
				CFile File(name, CFile::modeWrite | CFile::modeCreate);
				if (File == CFile::hFileNull) {
					theApp.WarningMessageBox(IDS_MSG_TRACING_WRITE_FAILURE, name);
					return (false);
				}
				EoDbJobFile JobFile;
				JobFile.WriteHeader(File);
				JobFile.WriteLayer(File, Layer);
			} else {
				EoDbTracingFile TracingFile(name, CFile::modeWrite | CFile::modeCreate);
				if (TracingFile == CFile::hFileNull) {
					theApp.WarningMessageBox(IDS_MSG_TRACING_WRITE_FAILURE, name);
					return (false);
				}
				TracingFile.WriteHeader();
				TracingFile.WriteLayer(Layer);
			}
			name = name.mid(of.nFileOffset);

			Layer->MakeResident(true);
			Layer->MakeInternal(false);
			Layer->SetIsLocked(true);
			Layer->SetName(name);
			bRetVal = true;
		}
	}
	delete[] of.lpstrFile;
	return (bRetVal);
}
void AeSysDoc::PenTranslation(OdUInt16 wCols, OdInt16 * pColNew, OdInt16 * pCol) {
	for (int LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);
		Layer->PenTranslation(wCols, pColNew, pCol);
	}
}
EoDbLayer* AeSysDoc::SelectLayerBy(const OdGePoint3d & point) {
	AeSysView* ActiveView = AeSysView::GetActiveView();

	auto Group {ActiveView->SelectGroupAndPrimitive(point)};

	if (Group != nullptr) {
		for (int LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
			EoDbLayer* Layer = GetLayerAt(LayerIndex);
			if (Layer->Find(Group)) {
				return (Layer);
			}
		}
	}
	for (int LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);

		if (Layer->SelectGroupBy(point) != 0) {
			return (Layer);
		}
	}
	return 0;
}
void AeSysDoc::PurgeDuplicateObjects() {
	for (int LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);
		Layer->RemoveDuplicatePrimitives();
	}
}
int AeSysDoc::RemoveEmptyNotesAndDelete() {
	int iCount = 0;

	for (int LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);
		iCount += Layer->RemoveEmptyNotesAndDelete();
	}

	//Note: remove empty notes from blocks

	CString Key;
	EoDbBlock* Block;

	POSITION Position = m_BlockTable.GetStartPosition();
	while (Position != NULL) {
		m_BlockTable.GetNextAssoc(Position, Key, Block);
	}
	return (iCount);
}
int AeSysDoc::RemoveEmptyGroups() {
	int iCount = 0;

	for (OdUInt16 LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);
		iCount += Layer->RemoveEmptyGroups();
	}

	//Note: remove empty groups from blocks

	CString Key;
	EoDbBlock* Block;

	POSITION Position = m_BlockTable.GetStartPosition();
	while (Position != NULL) {
		m_BlockTable.GetNextAssoc(Position, Key, Block);
	}
	return (iCount);
}
// Work Layer interface
void AeSysDoc::AddWorkLayerGroup(EoDbGroup * group) {
	m_WorkLayer->AddTail(group);
	AddGroupToAllViews(group);
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::WorkCount);
	SetModifiedFlag(TRUE);
}
void AeSysDoc::AddWorkLayerGroups(EoDbGroupList * groups) {
	m_WorkLayer->AddTail(groups);
	AddGroupsToAllViews(groups);
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::WorkCount);
	SetModifiedFlag(TRUE);
}
POSITION AeSysDoc::FindWorkLayerGroup(EoDbGroup * group) const {
	return (m_WorkLayer->Find(group));
}
POSITION AeSysDoc::GetFirstWorkLayerGroupPosition() const {
	return m_WorkLayer->GetHeadPosition();
}
EoDbGroup* AeSysDoc::GetLastWorkLayerGroup() const {
	POSITION Position = m_WorkLayer->GetTailPosition();
	return ((EoDbGroup*) (Position != 0 ? m_WorkLayer->GetPrev(Position) : 0));
}
POSITION AeSysDoc::GetLastWorkLayerGroupPosition() const {
	return m_WorkLayer->GetTailPosition();
}
EoDbGroup* AeSysDoc::GetNextWorkLayerGroup(POSITION & position) const {
	return m_WorkLayer->GetNext(position);
}
EoDbGroup* AeSysDoc::GetPreviousWorkLayerGroup(POSITION & position) const {
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
	OdDbObjectId PreviousLayer = m_DatabasePtr->getCLAYER();
	m_DatabasePtr->setCLAYER(layerTableRecord->objectId());

	m_WorkLayer = GetLayerAt(layerTableRecord->getName());
	m_WorkLayer->MakeCurrent();

	return PreviousLayer;
}

// Locates the layer containing a group and removes it.
// The group itself is not deleted.
EoDbLayer* AeSysDoc::AnyLayerRemove(EoDbGroup * group) {
	for (int LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);
		if (Layer->IsCurrent() || Layer->IsActive()) {
			if (Layer->Remove(group) != 0) {
				AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::WorkCount);
				SetModifiedFlag(TRUE);

				return Layer;
			}
		}
	}
	return 0;
}
void AeSysDoc::TracingFuse(OdString & nameAndLocation) {
	EoDbLayer* Layer = GetLayerAt(nameAndLocation);
	if (Layer != 0) {
		LPWSTR Title = new wchar_t[MAX_PATH];
		GetFileTitle(nameAndLocation, Title, MAX_PATH);
		LPWSTR NextToken = NULL;
		wcstok_s(Title, L".", &NextToken);
		nameAndLocation = Title;
		delete[] Title;

		Layer->MakeResident(true);
		Layer->MakeInternal(true);
		Layer->SetIsLocked(true);

		Layer->SetName(nameAndLocation);
	}
}

bool AeSysDoc::TracingLoadLayer(const OdString & file, EoDbLayer * layer) {
	if (!layer)
		return false;

	const EoDb::FileTypes FileType = AeSysApp::GetFileType(file);
	if (FileType != EoDb::kTracing && FileType != EoDb::kJob) {
		return false;
	}
	const bool bFileOpen = false;

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
		if (File != 0) {
			EoDbJobFile JobFile;
			JobFile.ReadHeader(File);
			JobFile.ReadLayer(m_DatabasePtr->getModelSpaceId().safeOpenObject(OdDb::kForWrite), File, layer);
			return true;
		}
		theApp.WarningMessageBox(IDS_MSG_TRACING_OPEN_FAILURE, file);
	}
	return (bFileOpen);
}

bool AeSysDoc::TracingOpen(const OdString & fileName) {
	m_DatabasePtr = theApp.createDatabase(true, OdDb::kEnglish);

	OdDbTextStyleTableRecordPtr TextStyle = AddStandardTextStyle();
	m_DatabasePtr->setTEXTSTYLE(TextStyle->objectId());

	AddRegisteredApp(L"AeSys");

	m_DatabasePtr->startUndoRecord();

	EoDbDwgToPegFile DwgToPegFile(m_DatabasePtr);
	DwgToPegFile.ConvertToPeg(this);

	OdDbLayerTableRecordPtr LayerTableRecord = m_DatabasePtr->getCLAYER().safeOpenObject(OdDb::kForWrite);
	SetCurrentLayer(LayerTableRecord);
	LayerTableRecord->setIsReconciled(true);

	EoDbLayer* Layer = GetLayerAt(0);
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
		CString ShadowFilePath(theApp.ShadowFolderPath());
		ShadowFilePath += GetTitle();
		const int nExt = ShadowFilePath.Find('.');
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
	for (int LayerIndex = GetLayerTableSize() - 1; LayerIndex > 0; LayerIndex--) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);

		if (Layer->IsActive()) {
			UpdateLayerInAllViews(EoDb::kLayerErase, Layer);
			Layer->DeleteGroupsAndRemoveAll();
		}
	}
}
void AeSysDoc::OnClearAllLayers() {
	InitializeGroupAndPrimitiveEdit();

	for (int LayerIndex = GetLayerTableSize() - 1; LayerIndex > 0; LayerIndex--) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);

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

	for (int LayerIndex = GetLayerTableSize() - 1; LayerIndex > 0; LayerIndex--) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);

		if (!Layer->IsInternal()) {
			UpdateLayerInAllViews(EoDb::kLayerErase, Layer);
			Layer->DeleteGroupsAndRemoveAll();
		}
	}
	UpdateAllViews(nullptr);
}
void AeSysDoc::OnClearMappedTracings() {
	InitializeGroupAndPrimitiveEdit();
	for (int LayerIndex = GetLayerTableSize() - 1; LayerIndex > 0; LayerIndex--) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);

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
	for (int LayerIndex = GetLayerTableSize() - 1; LayerIndex > 0; LayerIndex--) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);

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
	OdDbBlockTableRecordPtr BlockTableRecord = Database->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

	auto Group {ActiveView->SelectGroupAndPrimitive(ActiveView->GetCursorPosition())};
	if (Group != nullptr && ActiveView->EngagedPrimitive() != 0) {
		EoDbPrimitive* Primitive = ActiveView->EngagedPrimitive();

		if (Primitive->Is(EoDb::kPolylinePrimitive)) {
			const EoDbPolyline* PolylinePrimitive = dynamic_cast<EoDbPolyline*>(Primitive);
			Group->FindAndRemovePrimitive(Primitive);

			OdGePoint3dArray Points;
			PolylinePrimitive->GetAllPoints(Points);
			OdDbLinePtr Line;
			for (OdUInt16 w = 0; w < Points.size() - 1; w++) {
				Line = EoDbLine::Create(BlockTableRecord, Points[w], Points[w + 1]);
				Line->setColorIndex(Primitive->ColorIndex());
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(Primitive->LinetypeIndex()));
				Group->AddTail(EoDbLine::Create(Line));
			}
			if (PolylinePrimitive->IsClosed()) {
				Line = EoDbLine::Create(BlockTableRecord, Points[Points.size() - 1], Points[0]);
				Line->setColorIndex(Primitive->ColorIndex());
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(Primitive->LinetypeIndex()));
				Group->AddTail(EoDbLine::Create(Line));
			}
			delete Primitive;
			ResetAllViews();
		} else if (Primitive->Is(EoDb::kGroupReferencePrimitive)) {
			const EoDbBlockReference* BlockReference = dynamic_cast<EoDbBlockReference*>(Primitive);

			EoDbBlock* Block;

			if (LookupBlock(BlockReference->Name(), Block) != 0) {
				Group->FindAndRemovePrimitive(Primitive);

				EoGeMatrix3d tm = BlockReference->BlockTransformMatrix(Block->BasePoint());

				EoDbGroup* pSegT = new EoDbGroup(*Block);
				pSegT->TransformBy(tm);
				Group->AddTail(pSegT);

				delete Primitive;
				ResetAllViews();
			}
		}
	}
}
void AeSysDoc::OnEditSegToWork() {
	const OdGePoint3d Point = theApp.GetCursorPosition();

	EoDbLayer* Layer = SelectLayerBy(Point);

	if (Layer != 0) {
		if (Layer->IsInternal()) {
			EoDbGroup* Group = Layer->SelectGroupBy(Point);

			if (Group != 0) {
				Layer->Remove(Group);
				UpdateGroupInAllViews(EoDb::kGroupEraseSafe, Group);
				AddWorkLayerGroup(Group);
				UpdateGroupInAllViews(EoDb::kGroup, Group);
			}
		}
	}
}
void AeSysDoc::OnFileQuery() {
	const OdGePoint3d pt = theApp.GetCursorPosition();

	const EoDbLayer* Layer = SelectLayerBy(pt);

	if (Layer != 0) {
		CPoint CurrentPosition;
		::GetCursorPos(&CurrentPosition);

		m_IdentifiedLayerName = Layer->Name();

		const int MenuResource = (Layer->IsInternal()) ? IDR_LAYER : IDR_TRACING;

		HMENU LayerTracingMenu = ::LoadMenu(theApp.GetInstance(), MAKEINTRESOURCE(MenuResource));
		CMenu* SubMenu = CMenu::FromHandle(::GetSubMenu(LayerTracingMenu, 0));

		SubMenu->ModifyMenu(0, MF_BYPOSITION | MF_STRING, 0, m_IdentifiedLayerName);

		if (MenuResource == IDR_LAYER) {
			SubMenu->CheckMenuItem(ID_LAYER_CURRENT, (MF_BYCOMMAND | Layer->IsCurrent()) ? MF_CHECKED : MF_UNCHECKED);
			SubMenu->CheckMenuItem(ID_LAYER_ACTIVE, (MF_BYCOMMAND | Layer->IsActive()) ? MF_CHECKED : MF_UNCHECKED);
			SubMenu->CheckMenuItem(ID_LAYER_LOCK, (MF_BYCOMMAND | Layer->IsLocked()) ? MF_CHECKED : MF_UNCHECKED);
			SubMenu->CheckMenuItem(ID_LAYER_OFF, (MF_BYCOMMAND | Layer->IsOff()) ? MF_CHECKED : MF_UNCHECKED);
		} else {
			SubMenu->CheckMenuItem(ID_TRACING_CURRENT, (MF_BYCOMMAND | Layer->IsCurrent()) ? MF_CHECKED : MF_UNCHECKED);
			SubMenu->CheckMenuItem(ID_TRACING_ACTIVE, (MF_BYCOMMAND | Layer->IsActive()) ? MF_CHECKED : MF_UNCHECKED);
			SubMenu->CheckMenuItem(ID_TRACING_LOCK, (MF_BYCOMMAND | Layer->IsLocked()) ? MF_CHECKED : MF_UNCHECKED);
			SubMenu->CheckMenuItem(ID_TRACING_OFF, (MF_BYCOMMAND | Layer->IsOff()) ? MF_CHECKED : MF_UNCHECKED);
		}
		SubMenu->TrackPopupMenuEx(0, CurrentPosition.x, CurrentPosition.y, AfxGetMainWnd(), 0);
		::DestroyMenu(LayerTracingMenu);
	}
}
void AeSysDoc::OnLayerActive() {
	EoDbLayer* Layer = GetLayerAt(m_IdentifiedLayerName);

	if (Layer == 0) {
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
	EoDbLayer* Layer = GetLayerAt(m_IdentifiedLayerName);

	if (Layer != 0) {
		if (Layer->IsCurrent()) {
			theApp.WarningMessageBox(IDS_MSG_LAYER_NO_STATIC, m_IdentifiedLayerName);
		} else {
			Layer->SetIsLocked(true);
			UpdateLayerInAllViews(EoDb::kLayerSafe, Layer);
		}
	}
}
void AeSysDoc::OnLayerOff() {
	EoDbLayer* Layer = GetLayerAt(m_IdentifiedLayerName);

	if (Layer != 0) {
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
	OdDbLayerTablePtr Layers = LayerTable(OdDb::kForRead);
	SetCurrentLayer(Layers->getAt(m_IdentifiedLayerName).safeOpenObject(OdDb::kForRead));
}
void AeSysDoc::OnTracingActive() {
	EoDbLayer* Layer = GetLayerAt(m_IdentifiedLayerName);
	if (Layer->IsCurrent()) {
		theApp.WarningMessageBox(IDS_MSG_CLOSE_TRACING_FIRST, m_IdentifiedLayerName);
	} else {
		Layer->MakeActive();
		UpdateLayerInAllViews(EoDb::kLayerSafe, Layer);
	}
}
void AeSysDoc::OnTracingCurrent() {
	EoDbLayer* Layer = GetLayerAt(m_IdentifiedLayerName);
	Layer->MakeCurrent();
	UpdateLayerInAllViews(EoDb::kLayerSafe, Layer);
}
void AeSysDoc::OnTracingFuse() {
	TracingFuse(m_IdentifiedLayerName);
	theApp.AddStringToMessageList(IDS_MSG_TRACING_CONVERTED_TO_LAYER, m_IdentifiedLayerName);
}
void AeSysDoc::OnTracingLock() {
	EoDbLayer* Layer = GetLayerAt(m_IdentifiedLayerName);
	if (Layer->IsCurrent()) {
		theApp.WarningMessageBox(IDS_MSG_CLOSE_TRACING_FIRST, m_IdentifiedLayerName);
	} else {
		Layer->SetIsLocked(true);
		UpdateLayerInAllViews(EoDb::kLayerSafe, Layer);
	}
}
void AeSysDoc::OnTracingOff() {
	EoDbLayer* Layer = GetLayerAt(m_IdentifiedLayerName);

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
	for (int LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);
		if (!Layer->IsCurrent()) {
			Layer->MakeActive();
		}
	}
	UpdateAllViews(nullptr);
}
void AeSysDoc::OnLayersSetAllLocked() {
	for (int LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);
		if (!Layer->IsCurrent()) {
			Layer->SetIsLocked(true);
		}
	}
	UpdateAllViews(nullptr);
}
void AeSysDoc::OnPurgeUnusedLayers() {
	RemoveEmptyLayers();
}
void AeSysDoc::OnToolsGroupUndelete() {
	DeletedGroupsRestore();
}
void AeSysDoc::OnPensRemoveUnusedLinetypes() {
	OdDbLinetypeTablePtr Linetypes = m_DatabasePtr->getLinetypeTableId().safeOpenObject(OdDb::kForWrite);
	OdDbSymbolTableIteratorPtr Iterator = Linetypes->newIterator();

	for (Iterator->start(); !Iterator->done(); Iterator->step()) {
		OdDbLinetypeTableRecordPtr Linetype = Iterator->getRecordId().safeOpenObject(OdDb::kForWrite);

		OdString Name = Linetype->getName();
		if (LinetypeIndexReferenceCount(EoDbLinetypeTable::LegacyLinetypeIndex(Name)) == 0) {
			const OdResult Result = Linetype->erase(true);
			if (Result) {
				CString ErrorDescription = m_DatabasePtr->appServices()->getErrorDescription(Result);
				ErrorDescription += L" <%s> linetype can not be deleted";
				theApp.AddStringToMessageList(ErrorDescription, (LPCWSTR) Name);
			} else {
				theApp.AddStringToMessageList(IDS_MSG_UNUSED_LINETYPE_REMOVED, (LPCWSTR) Name);
			}
		}
	}
}
void AeSysDoc::OnPurgeUnreferencedBlocks() {
	PurgeUnreferencedBlocks();
}
void AeSysDoc::OnEditImageToClipboard() {
	AeSysView* ActiveView = AeSysView::GetActiveView();

	HDC hdcEMF = ::CreateEnhMetaFile(0, 0, 0, 0);
	DisplayAllLayers(ActiveView, CDC::FromHandle(hdcEMF));
	HENHMETAFILE hemf = ::CloseEnhMetaFile(hdcEMF);

	::OpenClipboard(NULL);
	::EmptyClipboard();
	::SetClipboardData(CF_ENHMETAFILE, hemf);
	::CloseClipboard();
}
void AeSysDoc::OnEditTrace() {
	if (::OpenClipboard(NULL)) {
		wchar_t sBuf[16];

		UINT ClipboardFormat;
		UINT Format = 0;

		while ((ClipboardFormat = EnumClipboardFormats(Format)) != 0) {
			GetClipboardFormatName(ClipboardFormat, sBuf, 16);

			if (wcscmp(sBuf, L"EoGroups") == 0) {
				HGLOBAL ClipboardDataHandle = GetClipboardData(ClipboardFormat);
				if (ClipboardDataHandle != 0) {

					LPCSTR ClipboardData = (LPCSTR) GlobalLock(ClipboardDataHandle);
					if (ClipboardData != NULL) {
						const DWORD ClipboardDataLength = *((DWORD*) ClipboardData);
						CMemFile MemFile;
						MemFile.Write(ClipboardData, UINT(ClipboardDataLength));
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
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::TrapCount);
}
void AeSysDoc::OnEditTrapCopy() {
	AeSysView* ActiveView = AeSysView::GetActiveView();
	CopyTrappedGroupsToClipboard(ActiveView);
}
void AeSysDoc::OnEditTrapCut() {
	AeSysView* ActiveView = AeSysView::GetActiveView();
	CopyTrappedGroupsToClipboard(ActiveView);
	DeleteAllTrappedGroups();
	UpdateAllViews(nullptr);
}
void AeSysDoc::OnEditTrapPaste() {
	if (::OpenClipboard(NULL)) {
		const UINT nClipboardFormat = theApp.ClipboardFormatIdentifierForEoGroups();

		if (IsClipboardFormatAvailable(nClipboardFormat)) {
			EoDlgSetPastePosition Dialog;
			if (Dialog.DoModal() == IDOK) {
				HGLOBAL ClipboardDataHandle = GetClipboardData(nClipboardFormat);
				if (ClipboardDataHandle != 0) {
					OdGePoint3d LowerLeftExtent;
					const OdGePoint3d InsertionPoint(theApp.GetCursorPosition());
					SetTrapPivotPoint(InsertionPoint);

					LPCSTR ClipboardData = (LPCSTR) GlobalLock(ClipboardDataHandle);
					const DWORD ClipboardDataLength = *((DWORD*) ClipboardData);
					CMemFile MemoryFile;
					MemoryFile.Write(ClipboardData, UINT(ClipboardDataLength));

					MemoryFile.Seek(sizeof(DWORD), CFile::begin);
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
			HGLOBAL ClipboardDataHandle = GetClipboardData(CF_TEXT);
			if (ClipboardDataHandle != NULL) {
				const char* ClipboardData = (char*) GlobalLock(ClipboardDataHandle);
				if (ClipboardData != NULL) {
					const size_t ClipboardDataSize = GlobalSize(ClipboardDataHandle);
					wchar_t* Text = new wchar_t[ClipboardDataSize];
					for (size_t i = 0; i < ClipboardDataSize; i++) {
						Text[i] = (wchar_t) ClipboardData[i];
					}
					GlobalUnlock(ClipboardDataHandle);
					AddTextBlock(Text);
					delete[] Text;
				}
			}
		} else if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
			HGLOBAL ClipboardDataHandle = GetClipboardData(CF_UNICODETEXT);

			const wchar_t* ClipboardData = (wchar_t*) GlobalLock(ClipboardDataHandle);
			const size_t ClipboardDataSize = GlobalSize(ClipboardDataHandle);
			wchar_t* Text = new wchar_t[ClipboardDataSize];
			for (size_t i = 0; i < ClipboardDataSize; i++) {
				Text[i] = ClipboardData[i];
			}
			GlobalUnlock(ClipboardDataHandle);
			AddTextBlock(Text);
			delete[] Text;
		}
		CloseClipboard();
	} else
		theApp.WarningMessageBox(IDS_MSG_CLIPBOARD_LOCKED);
}
void AeSysDoc::OnEditTrapWork() {
	RemoveAllTrappedGroups();
	AddGroupsToTrap(GetWorkLayer());
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::TrapCount);
}
void AeSysDoc::OnEditTrapWorkAndActive() {
	RemoveAllTrappedGroups();

	for (int LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);
		if (Layer->IsCurrent() || Layer->IsActive()) {
			AddGroupsToTrap(Layer);
		}
	}
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::TrapCount);
}
void AeSysDoc::OnTrapCommandsCompress() {
	CompressTrappedGroups();
}
void AeSysDoc::OnTrapCommandsExpand() {
	ExpandTrappedGroups();
}
void AeSysDoc::OnTrapCommandsInvert() {
	const int NumberOfLayers = GetLayerTableSize();
	for (int LayerIndex = 0; LayerIndex < NumberOfLayers; LayerIndex++) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);
		if (Layer->IsCurrent() || Layer->IsActive()) {
			POSITION LayerPosition = Layer->GetHeadPosition();
			while (LayerPosition != 0) {
				EoDbGroup* Group = Layer->GetNext(LayerPosition);
				POSITION GroupPosition = FindTrappedGroup(Group);
				if (GroupPosition != 0) {
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
	AeSysView* ActiveView = AeSysView::GetActiveView();
	SquareTrappedGroups(ActiveView);
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
	if (m_TrappedGroupList.GetCount() == 0)
		return;

	EoDbBlock * Block;
	OdUInt16 w = BlockTableSize();
	wchar_t szBlkNam[16];

	do {
		swprintf_s(szBlkNam, 16, L"_%.3i", ++w);
	} while (LookupBlock(szBlkNam, Block));

	Block = new EoDbBlock;

	POSITION Position = GetFirstTrappedGroupPosition();
	while (Position != 0) {
		const EoDbGroup* Group = GetNextTrappedGroup(Position);

		EoDbGroup* pSeg2 = new EoDbGroup(*Group);

		Block->AddTail(pSeg2);

		pSeg2->RemoveAll();

		delete pSeg2;
	}
	Block->SetBasePoint(m_TrapPivotPoint);
	InsertBlock(szBlkNam, Block);
}
void AeSysDoc::OnTrapCommandsUnblock() {
	m_TrappedGroupList.BreakSegRefs();
}
void AeSysDoc::OnSetupPenColor() {
	EoDlgSetupColor Dialog;
	Dialog.m_ColorIndex = pstate.ColorIndex();

	if (Dialog.DoModal() == IDOK) {
		pstate.SetColorIndex(NULL, Dialog.m_ColorIndex);

		AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::Pen);
	}
}
void AeSysDoc::OnSetupLinetype() {
	OdDbLinetypeTablePtr Linetypes = m_DatabasePtr->getLinetypeTableId().safeOpenObject(OdDb::kForRead);
	EoDlgSetupLinetype Dialog(Linetypes);

	if (Dialog.DoModal() == IDOK) {
		OdString Name = Dialog.m_Linetype->getName();
		const OdInt16 LinetypeIndex = EoDbLinetypeTable::LegacyLinetypeIndex(Name);
		pstate.SetLinetypeIndexPs(NULL, LinetypeIndex);
		AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::Line);
	}
}
void AeSysDoc::OnSetupFillHollow() noexcept {
	pstate.SetHatchInteriorStyle(EoDbHatch::kHollow);
}
void AeSysDoc::OnSetupFillSolid() noexcept {
	pstate.SetHatchInteriorStyle(EoDbHatch::kSolid);
}
void AeSysDoc::OnSetupFillPattern() noexcept {}
void AeSysDoc::OnSetupFillHatch() {
	EoDlgSetupHatch Dialog;
	Dialog.m_HatchXScaleFactor = EoDbHatch::sm_PatternScaleX;
	Dialog.m_HatchYScaleFactor = EoDbHatch::sm_PatternScaleY;
	Dialog.m_HatchRotationAngle = EoToDegree(EoDbHatch::sm_PatternAngle);

	if (Dialog.DoModal() == IDOK) {
		pstate.SetHatchInteriorStyle(EoDbHatch::kHatch);
		EoDbHatch::sm_PatternScaleX = EoMax(.01, Dialog.m_HatchXScaleFactor);
		EoDbHatch::sm_PatternScaleY = EoMax(.01, Dialog.m_HatchYScaleFactor);
		EoDbHatch::sm_PatternAngle = EoArcLength(Dialog.m_HatchRotationAngle);
	}
}
void AeSysDoc::OnSetupNote() {
	EoDbFontDefinition FontDefinition = pstate.FontDefinition();

	EoDlgSetupNote Dialog(&FontDefinition);

	EoDbCharacterCellDefinition CharacterCellDefinition = pstate.CharacterCellDefinition();

	Dialog.m_Height = CharacterCellDefinition.Height();
	Dialog.m_RotationAngle = EoToDegree(CharacterCellDefinition.RotationAngle());
	Dialog.m_WidthFactor = CharacterCellDefinition.WidthFactor();
	Dialog.m_ObliqueAngle = EoToDegree(CharacterCellDefinition.ObliqueAngle());

	if (Dialog.DoModal() == IDOK) {
		CharacterCellDefinition.SetHeight(Dialog.m_Height);
		CharacterCellDefinition.SetRotationAngle(EoToRadian(Dialog.m_RotationAngle));
		CharacterCellDefinition.SetWidthFactor(Dialog.m_WidthFactor);
		CharacterCellDefinition.SetObliqueAngle(EoToRadian(Dialog.m_ObliqueAngle));
		pstate.SetCharacterCellDefinition(CharacterCellDefinition);

		AeSysView* ActiveView = AeSysView::GetActiveView();
		CDC* DeviceContext = (ActiveView) ? ActiveView->GetDC() : NULL;

		pstate.SetFontDefinition(DeviceContext, FontDefinition);
	}
}
void AeSysDoc::OnToolsGroupBreak() {
	AeSysView* ActiveView = AeSysView::GetActiveView();

	ActiveView->BreakAllPolylines();
	ActiveView->BreakAllSegRefs();
}
void AeSysDoc::OnToolsGroupDelete() {
	auto ActiveView {AeSysView::GetActiveView()};
	const auto CurrentPnt {ActiveView->GetCursorPosition()};
	auto Group {ActiveView->SelectGroupAndPrimitive(CurrentPnt)};

	if (Group != nullptr) {
		AnyLayerRemove(Group);
		RemoveGroupFromAllViews(Group);

		if (RemoveTrappedGroup(Group) != NULL) {
			ActiveView->UpdateStateInformation(AeSysView::TrapCount);
		}
		UpdateGroupInAllViews(EoDb::kGroupEraseSafe, Group);
		DeletedGroupsAddTail(Group);
		theApp.AddStringToMessageList(IDS_MSG_GROUP_ADDED_TO_DEL_GROUPS);
	}
}
void AeSysDoc::OnToolsGroupDeletelast() {
	AeSysView* ActiveView = AeSysView::GetActiveView();

	ActiveView->DeleteLastGroup();
}
void AeSysDoc::OnToolsGroupExchange() {
	if (m_DeletedGroupList.GetSize() > 1) {
		EoDbGroup* TailGroup = DeletedGroupsRemoveTail();
		EoDbGroup* HeadGroup = DeletedGroupsRemoveHead();
		DeletedGroupsAddTail(HeadGroup);
		DeletedGroupsAddHead(TailGroup);
	}
}
void AeSysDoc::OnToolsPrimitiveSnaptoendpoint() {
	AeSysView* ActiveView = AeSysView::GetActiveView();

	EoGePoint4d ptView(ActiveView->GetCursorPosition(), 1.);
	ActiveView->ModelViewTransformPoint(ptView);

	if (ActiveView->GroupIsEngaged()) {
		EoDbPrimitive* Primitive = ActiveView->EngagedPrimitive();

		if (Primitive->PivotOnGripPoint(ActiveView, ptView)) {
			const OdGePoint3d ptEng = ActiveView->DetPt();
			Primitive->AddReportToMessageList(ptEng);
			ActiveView->SetCursorPosition(ptEng);
			return;
		}
		// Did not pivot on engaged primitive
		if (Primitive->IsPointOnControlPoint(ActiveView, ptView)) {
			EoDbGroup::SetPrimitiveToIgnore(Primitive);
		}
	}
	if (ActiveView->SelSegAndPrimAtCtrlPt(ptView) != 0) {
		const OdGePoint3d ptEng = ActiveView->DetPt();
		ActiveView->EngagedPrimitive()->AddReportToMessageList(ptEng);
		ActiveView->SetCursorPosition(ptEng);
	}
	EoDbGroup::SetPrimitiveToIgnore(static_cast<EoDbPrimitive*>(NULL));
}
void AeSysDoc::OnPrimGotoCenterPoint() {
	AeSysView* ActiveView = AeSysView::GetActiveView();
	if (ActiveView->GroupIsEngaged()) {
		const OdGePoint3d pt = ActiveView->EngagedPrimitive()->GetCtrlPt();
		ActiveView->SetCursorPosition(pt);
	}
}
void AeSysDoc::OnToolsPrimitiveDelete() {
	const OdGePoint3d pt = theApp.GetCursorPosition();

	AeSysView* ActiveView = AeSysView::GetActiveView();

	auto Group {ActiveView->SelectGroupAndPrimitive(pt)};

	if (Group != nullptr) {
		const POSITION Position = FindTrappedGroup(Group);

		LPARAM lHint = (Position != 0) ? EoDb::kGroupEraseSafeTrap : EoDb::kGroupEraseSafe;
		// erase entire group even if group has more than one primitive
		UpdateGroupInAllViews(lHint, Group);

		if (Group->GetCount() > 1) { // remove primitive from group
			EoDbPrimitive* Primitive = ActiveView->EngagedPrimitive();
			Group->FindAndRemovePrimitive(Primitive);
			lHint = (Position != 0) ? EoDb::kGroupSafeTrap : EoDb::kGroupSafe;
			// display the group with the primitive removed
			UpdateGroupInAllViews(lHint, Group);
			// new group required to allow primitive to be placed into deleted group list
			Group = new EoDbGroup;
			Group->AddTail(Primitive);
		} else { // deleting an entire group
			AnyLayerRemove(Group);
			RemoveGroupFromAllViews(Group);

			if (RemoveTrappedGroup(Group) != 0) {
				ActiveView->UpdateStateInformation(AeSysView::TrapCount);
			}
		}
		DeletedGroupsAddTail(Group);
		theApp.AddStringToMessageList(IDS_MSG_PRIM_ADDED_TO_DEL_GROUPS);
	}
}
void AeSysDoc::OnPrimModifyAttributes() {
	AeSysView* ActiveView = AeSysView::GetActiveView();

	const OdGePoint3d pt = ActiveView->GetCursorPosition();

	const auto Group {ActiveView->SelectGroupAndPrimitive(pt)};

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
		AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::All);
	}
}
void AeSysDoc::OnFileManage() {
	EoDlgFileManage dlg(this, m_DatabasePtr);

	if (dlg.DoModal() == IDOK) {
	}
}
void AeSysDoc::OnFileTracing() {
	static DWORD FilterIndex = 1;

	OPENFILENAME of;
	::ZeroMemory(&of, sizeof(OPENFILENAME));
	of.lStructSize = sizeof(OPENFILENAME);
	of.hwndOwner = 0;
	of.hInstance = theApp.GetInstance();
	of.lpstrFilter = L"Tracing Files\0*.tra;*.jb1\0\0";
	of.nFilterIndex = FilterIndex;
	of.lpstrFile = new wchar_t[MAX_PATH];
	of.lpstrFile[0] = 0;
	of.nMaxFile = MAX_PATH;
	of.lpstrTitle = L"Load Tracing";
	of.Flags = OFN_EXPLORER | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
	of.lpstrDefExt = L"tra";
	of.lpfnHook = OFNHookProcFileTracing;
	of.lpTemplateName = MAKEINTRESOURCE(IDD_TRACING_EX);

	if (GetOpenFileNameW(&of)) {
		FilterIndex = of.nFilterIndex;

		TracingOpen(of.lpstrFile);
	}

	delete[] of.lpstrFile;
}
void AeSysDoc::OnPurgeDuplicateObjects() {
	PurgeDuplicateObjects();
}
void AeSysDoc::OnPurgeEmptyNotes() {
	const int NumberOfEmptyNotes = RemoveEmptyNotesAndDelete();
	const int NumberOfEmptyGroups = RemoveEmptyGroups();
	CString str;
	str.Format(L"%d notes were removed resulting in %d empty groups which were also removed.", NumberOfEmptyNotes, NumberOfEmptyGroups);
	theApp.AddStringToMessageList(str);
}
void AeSysDoc::OnPurgeEmptyGroups() {
	const int NumberOfEmptyGroups = RemoveEmptyGroups();
	CString str;
	str.Format(L"%d were removed.", NumberOfEmptyGroups);
	theApp.AddStringToMessageList(str);
}
void AeSysDoc::OnPensEditColors() {
	theApp.EditColorPalette();
}
void AeSysDoc::OnPensLoadColors() {
	CString InitialDirectory = theApp.ResourceFolderPath() + L"Pens\\Colors\\";

	OPENFILENAME of;
	::ZeroMemory(&of, sizeof(OPENFILENAME));
	of.lStructSize = sizeof(OPENFILENAME);
	of.hwndOwner = 0;
	of.hInstance = theApp.GetInstance();
	of.lpstrFilter = L"Pen Color Files\0*.txt\0\0";
	of.lpstrFile = new wchar_t[MAX_PATH];
	of.lpstrFile[0] = 0;
	of.nMaxFile = MAX_PATH;
	of.lpstrTitle = L"Load Pen Colors";
	of.lpstrInitialDir = InitialDirectory;
	of.Flags = OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	of.lpstrDefExt = L"txt";

	if (GetOpenFileNameW(&of)) {
		if ((of.Flags & OFN_EXTENSIONDIFFERENT) == 0) {
			theApp.LoadColorPalletFromFile(of.lpstrFile);

			UpdateAllViews(nullptr);
		} else
			theApp.WarningMessageBox(IDS_MSG_FILE_TYPE_ERROR);
	}
	delete[] of.lpstrFile;
}
void AeSysDoc::OnPensTranslate() {
	CStdioFile fl;

	// <tas="OnPensTranslate would be more useful if the file name could be selected. Currently fixed as xlate.txt"</tas>
	if (fl.Open(AeSysApp::ResourceFolderPath() + L"\\Pens\\xlate.txt", CFile::modeRead | CFile::typeText)) {
		wchar_t pBuf[128];
		OdUInt16 wCols = 0;

		while (fl.ReadString(pBuf, sizeof(pBuf) / sizeof(wchar_t) - 1) != 0)
			wCols++;

		if (wCols > 0) {
			OdInt16* pColNew = new OdInt16[wCols];
			OdInt16* pCol = new OdInt16[wCols];

			OdUInt16 w = 0;

			fl.SeekToBegin();

			LPWSTR NextToken;
			while (fl.ReadString(pBuf, sizeof(pBuf) / sizeof(wchar_t) - 1) != 0) {
				NextToken = NULL;
				pCol[w] = OdInt16(_wtoi(wcstok_s(pBuf, L",", &NextToken)));
				pColNew[w++] = OdInt16(_wtoi(wcstok_s(nullptr, L"\n", &NextToken)));
			}
			PenTranslation(wCols, pColNew, pCol);

			delete[] pColNew;
			delete[] pCol;
		}
	}
	UpdateAllViews(nullptr);
}

void AeSysDoc::OnFile() {
	CPoint Position(8, 8);

	theApp.GetMainWnd()->ClientToScreen(&Position);
	CMenu* FileSubMenu = CMenu::FromHandle(theApp.GetAeSysSubMenu(0));
	FileSubMenu->TrackPopupMenuEx(TPM_LEFTALIGN, Position.x, Position.y, AfxGetMainWnd(), 0);
}

void AeSysDoc::OnPrimExtractNum() {
	AeSysView* ActiveView = AeSysView::GetActiveView();

	const OdGePoint3d pt = ActiveView->GetCursorPosition();

	if (ActiveView->SelectGroupAndPrimitive(pt)) {
		EoDbPrimitive* Primitive = ActiveView->EngagedPrimitive();

		CString Number;

		if (Primitive->Is(EoDb::kTextPrimitive)) {
			Number = dynamic_cast<EoDbText*>(Primitive)->Text();
		} else if (Primitive->Is(EoDb::kDimensionPrimitive)) {
			Number = dynamic_cast<EoDbDimension*>(Primitive)->Text();
		} else {
			return;
		}
		double dVal[32];
		int iTyp;
		long lDef;
		int iTokId = 0;

		lex::Parse(Number);
		lex::EvalTokenStream(&iTokId, &lDef, &iTyp, (void*) dVal);

		if (iTyp != lex::TOK_LENGTH_OPERAND) {
			lex::ConvertValTyp(iTyp, lex::TOK_REAL, &lDef, dVal);
		}
		wchar_t Message[64];
		swprintf_s(Message, 64, L"%10.4f ", dVal[0]);
		wcscat_s(Message, 64, L"was extracted from drawing");
		theApp.AddStringToMessageList(Message);
	}
}
void AeSysDoc::OnPrimExtractStr() {
	AeSysView* ActiveView = AeSysView::GetActiveView();

	const OdGePoint3d pt = ActiveView->GetCursorPosition();

	if (ActiveView->SelectGroupAndPrimitive(pt)) {
		EoDbPrimitive* Primitive = ActiveView->EngagedPrimitive();

		CString String;

		if (Primitive->Is(EoDb::kTextPrimitive)) {
			String = dynamic_cast<EoDbText*>(Primitive)->Text();
		} else if (Primitive->Is(EoDb::kDimensionPrimitive)) {
			String = dynamic_cast<EoDbDimension*>(Primitive)->Text();
		} else {
			return;
		}
		String += L" was extracted from drawing";
		theApp.AddStringToMessageList(String);
	}
	return;
}
// Returns a pointer to the currently active document.
AeSysDoc* AeSysDoc::GetDoc() {
	const CMDIFrameWndEx* Frame = (CMDIFrameWndEx*) AfxGetMainWnd();
	if (Frame == NULL) {
		return NULL;
	}
	CMDIChildWndEx* Child = (CMDIChildWndEx*) Frame->MDIGetActive();

	return (Child == NULL) ? NULL : (AeSysDoc*) Child->GetActiveDocument();
}

void AeSysDoc::AddGroupToAllViews(EoDbGroup * group) {
	auto ViewPosition {GetFirstViewPosition()};

	while (ViewPosition != 0) {
		auto View {(AeSysView*) GetNextView(ViewPosition)};
		View->AddVisibleGroup(group);
	}
}

void AeSysDoc::AddGroupsToAllViews(EoDbGroupList * groups) {
	auto ViewPosition {GetFirstViewPosition()};

	while (ViewPosition != 0) {
		auto View {(AeSysView*) GetNextView(ViewPosition)};
		View->AddVisibleGroups(groups);
	}
}

void AeSysDoc::RemoveAllGroupsFromAllViews() {
	auto ViewPosition {GetFirstViewPosition()};

	while (ViewPosition != 0) {
		auto View {(AeSysView*) GetNextView(ViewPosition)};
		View->RemoveAllVisibleGroups();
	}
}

void AeSysDoc::RemoveGroupFromAllViews(EoDbGroup * group) {
	auto ViewPosition {GetFirstViewPosition()};

	while (ViewPosition != 0) {
		auto View {(AeSysView*) GetNextView(ViewPosition)};
		View->RemoveVisibleGroup(group);
	}
}

void AeSysDoc::ResetAllViews() {
	auto ViewPosition {GetFirstViewPosition()};

	while (ViewPosition != 0) {
		auto View {(AeSysView*) GetNextView(ViewPosition)};
		View->ResetView();
	}
}

void AeSysDoc::OnHelpKey() {
	switch (theApp.CurrentMode()) {
		case ID_MODE_DRAW:
			HtmlHelpW(AfxGetMainWnd()->GetSafeHwnd(), L"..\\AeSys\\hlp\\AeSys.chm::/hid_mode_draw.htm", HH_DISPLAY_TOPIC, NULL);
			break;

		case ID_MODE_EDIT:
		{
			HtmlHelpW(AfxGetMainWnd()->GetSafeHwnd(), L"..\\AeSys\\hlp\\AeSys.chm::/hid_mode_edit.htm", HH_DISPLAY_TOPIC, NULL);
			break;
		}
		case ID_MODE_TRAP:
		case ID_MODE_TRAPR:
		{
			HtmlHelpW(AfxGetMainWnd()->GetSafeHwnd(), L"..\\AeSys\\hlp\\AeSys.chm::/hid_mode_trap.htm", HH_DISPLAY_TOPIC, NULL);
			break;
		}
	}
}

void AeSysDoc::DeleteNodalResources() {
	POSITION UniquePointPosition = m_UniquePoints.GetHeadPosition();
	while (UniquePointPosition != 0) {
		delete GetNextUniquePoint(UniquePointPosition);
	}
	RemoveAllUniquePoints();
	POSITION MaskedPrimitivePosition = GetFirstMaskedPrimitivePosition();
	while (MaskedPrimitivePosition != 0) {
		delete GetNextMaskedPrimitive(MaskedPrimitivePosition);
	}
	RemoveAllMaskedPrimitives();
	RemoveAllNodalGroups();
}
void AeSysDoc::UpdateNodalList(EoDbGroup * group, EoDbPrimitive * primitive, DWORD mask, int bit, OdGePoint3d point) {
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
POSITION AeSysDoc::AddNodalGroup(EoDbGroup * group) {
	return m_NodalGroupList.AddTail(group);
}
POSITION AeSysDoc::FindNodalGroup(EoDbGroup * group) {
	return m_NodalGroupList.Find(group);
}
POSITION AeSysDoc::GetFirstNodalGroupPosition() const {
	return m_NodalGroupList.GetHeadPosition();
}
EoDbGroup* AeSysDoc::GetNextNodalGroup(POSITION & position) {
	return m_NodalGroupList.GetNext(position);
}
void AeSysDoc::RemoveAllNodalGroups() {
	m_NodalGroupList.RemoveAll();
}
POSITION AeSysDoc::AddMaskedPrimitive(EoDbMaskedPrimitive * maskedPrimitive) {
	return m_MaskedPrimitives.AddTail((CObject*) maskedPrimitive);
}
POSITION AeSysDoc::GetFirstMaskedPrimitivePosition() const {
	return m_MaskedPrimitives.GetHeadPosition();
}
EoDbMaskedPrimitive* AeSysDoc::GetNextMaskedPrimitive(POSITION & position) {
	return (EoDbMaskedPrimitive*) m_MaskedPrimitives.GetNext(position);
}
void AeSysDoc::RemoveAllMaskedPrimitives() {
	m_MaskedPrimitives.RemoveAll();
}

int AeSysDoc::AddUniquePoint(const OdGePoint3d & point) {
	auto UniquePointPosition {m_UniquePoints.GetHeadPosition()};

	while (UniquePointPosition != 0) {
		auto UniquePoint {GetNextUniquePoint(UniquePointPosition)};
		
		if ((point - UniquePoint->m_Point).length() <= OdGeContext::gTol.equalPoint()) {

//		if (point == UniquePoint->m_Point) {
			(UniquePoint->m_References)++;
			return (UniquePoint->m_References);
		}
	}
	m_UniquePoints.AddTail(new EoGeUniquePoint(1, point));
	return (1);
}

EoGeUniquePoint* AeSysDoc::GetNextUniquePoint(POSITION & position) {
	return (EoGeUniquePoint*) m_UniquePoints.GetNext(position);
}

void AeSysDoc::RemoveUniquePointAt(POSITION position) {
	m_UniquePoints.RemoveAt(position);
}

void AeSysDoc::RemoveAllUniquePoints() {
	m_UniquePoints.RemoveAll();
}

void AeSysDoc::DisplayUniquePoints() {
	if (m_UniquePoints.IsEmpty()) { return; }

	EoDbGroup Group;
	POSITION UniquePointPosition {m_UniquePoints.GetHeadPosition()};
	
	while (UniquePointPosition != 0) {
		const auto UniquePoint {GetNextUniquePoint(UniquePointPosition)};
		auto PointPrimitive {new EoDbPoint(UniquePoint->m_Point)};
		PointPrimitive->SetColorIndex2(252);
		PointPrimitive->SetPointDisplayMode(8);
		Group.AddTail(PointPrimitive);
	}
	UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &Group);
	Group.DeletePrimitivesAndRemoveAll();
}

int AeSysDoc::RemoveUniquePoint(const OdGePoint3d & point) {
	int References = 0;

	POSITION UniquePointPosition = m_UniquePoints.GetHeadPosition();
	while (UniquePointPosition != 0) {
		POSITION Position = UniquePointPosition;
		EoGeUniquePoint* UniquePoint = GetNextUniquePoint(UniquePointPosition);
		if (point == UniquePoint->m_Point) {
			References = --(UniquePoint->m_References);

			if (References == 0) {
				RemoveUniquePointAt(Position);
				delete UniquePoint;
			}
			break;
		}
	}
	return References;
}

void AeSysDoc::AddPrimitiveBit(EoDbPrimitive * primitive, int bit) {
	EoDbMaskedPrimitive* MaskedPrimitive = 0;

	POSITION MaskedPrimitivePosition = GetFirstMaskedPrimitivePosition();
	while (MaskedPrimitivePosition != 0) {
		POSITION posCur = MaskedPrimitivePosition;
		MaskedPrimitive = GetNextMaskedPrimitive(MaskedPrimitivePosition);
		if (MaskedPrimitive->GetPrimitive() == primitive) {
			MaskedPrimitivePosition = posCur;
			break;
		}
	}
	if (MaskedPrimitivePosition == 0) {
		MaskedPrimitive = new EoDbMaskedPrimitive(primitive, 0);
		AddMaskedPrimitive(MaskedPrimitive);
	}
	MaskedPrimitive->SetMaskBit(bit);
}
void AeSysDoc::RemovePrimitiveBit(EoDbPrimitive * primitive, int bit) {
	EoDbMaskedPrimitive* MaskedPrimitive = 0;

	POSITION MaskedPrimitivePosition = GetFirstMaskedPrimitivePosition();
	while (MaskedPrimitivePosition != 0) {
		POSITION posCur = MaskedPrimitivePosition;
		MaskedPrimitive = GetNextMaskedPrimitive(MaskedPrimitivePosition);
		if (MaskedPrimitive->GetPrimitive() == primitive) {
			MaskedPrimitivePosition = posCur;
			break;
		}
	}
	if (MaskedPrimitivePosition != 0) {
		MaskedPrimitive->ClearMaskBit(bit);
	}
}
DWORD AeSysDoc::GetPrimitiveMask(EoDbPrimitive * primitive) {
	EoDbMaskedPrimitive* MaskedPrimitive = 0;

	POSITION MaskedPrimitivePosition = GetFirstMaskedPrimitivePosition();
	while (MaskedPrimitivePosition != 0) {
		POSITION posCur = MaskedPrimitivePosition;
		MaskedPrimitive = GetNextMaskedPrimitive(MaskedPrimitivePosition);
		if (MaskedPrimitive->GetPrimitive() == primitive) {
			MaskedPrimitivePosition = posCur;
			break;
		}
	}
	return ((MaskedPrimitivePosition != 0) ? MaskedPrimitive->GetMask() : 0UL);
}

void AeSysDoc::OnSetupLayerproperties() {
	EoDlgLayerPropertiesManager LayerPropertiesManager(m_DatabasePtr);

	if (IDOK == LayerPropertiesManager.DoModal()) {
		UpdateAllViews(nullptr);
	}
}
void AeSysDoc::OnInsertTracing() {
	static DWORD FilterIndex = 1;

	OPENFILENAME of;
	::ZeroMemory(&of, sizeof(OPENFILENAME));
	of.lStructSize = sizeof(OPENFILENAME);
	of.hwndOwner = 0;
	of.hInstance = theApp.GetInstance();
	of.lpstrFilter = L"Tracing Files\0*.tra;*.jb1\0\0";
	of.nFilterIndex = FilterIndex;
	of.lpstrFile = new wchar_t[MAX_PATH];
	of.lpstrFile[0] = 0;
	of.nMaxFile = MAX_PATH;
	of.lpstrTitle = L"Insert Tracing";
	of.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
	of.lpstrDefExt = L"tra";

	if (GetOpenFileNameW(&of)) {
		FilterIndex = of.nFilterIndex;

		OdString Name = of.lpstrFile;
		CString strPath = Name.left(of.nFileOffset);

		Name = Name.mid(of.nFileOffset);

		const EoDb::FileTypes FileType = AeSysApp::GetFileType(Name);
		if (FileType != EoDb::kTracing && FileType != EoDb::kJob) {
			return;
		}
		OdDbLayerTablePtr Layers = LayerTable(OdDb::kForWrite);
		if (Layers->getAt(Name).isNull()) {
			OdDbLayerTableRecordPtr LayerTableRecord = OdDbLayerTableRecord::createObject();
			LayerTableRecord->setName(Name);
			EoDbLayer* Layer = new EoDbLayer(LayerTableRecord);

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
	delete[] of.lpstrFile;
}
void AeSysDoc::OnFilePagesetup() {
	OdSmartPtr<OdDbUserIO> pIO; // = pDbCmdCtx->userIO();

	const OdDbObjectId idLayout = OdDbBlockTableRecordPtr(m_DatabasePtr->getActiveLayoutBTRId().safeOpenObject())->getLayoutId();
	OdDbLayoutPtr Layout = idLayout.safeOpenObject(OdDb::kForWrite);

	OdDbPlotSettings* PlotSettings = Layout.get();
	EoDlgPageSetup PageSetupDialog(*PlotSettings, pIO);
	if (PageSetupDialog.DoModal() == IDOK) {
	}
}

// <command_console>
AeSysDoc::DataSource::DataSource() {}

void AeSysDoc::DataSource::Create(AeSysDoc * document, const OdGePoint3d & point) {
	Empty();

	OdDbObjectIdArray objs = document->selectionSet()->objectIdArray();
	OdDbDatabasePtr pDb = document->m_DatabasePtr->wblock(objs, OdGePoint3d::kOrigin);

	wchar_t tempdir[MAX_PATH];
	::GetTempPath(MAX_PATH, tempdir);
	wchar_t tempname[MAX_PATH];
	::GetTempFileName(tempdir, L"", 0, tempname);
	m_tmpPath = tempname;
	m_tmpPath.makeLower();
	m_tmpPath.replace(L".tmp", L".dwg");
	OdStreamBufPtr sbuf = theApp.createFile(m_tmpPath, Oda::kFileWrite, Oda::kShareDenyWrite, Oda::kCreateNew);

	//pDb->writeFile(sbuf,OdDb::kDwg,OdDb::vAC21);
	//HGLOBAL hGlobal = GlobalAlloc(GMEM_FIXED, sizeof(AcadClipDataR15));
	//new (hGlobal) AcadClipDataR15(m_tmpPath, OdString(pDoc->GetPathName()), p2 );
	//CacheGlobalData(ClipboardData::m_FormatR16, hGlobal);

	pDb->writeFile(sbuf, OdDb::kDwg, OdDb::vAC21);
	HGLOBAL hGlobalR21 = GlobalAlloc(GMEM_FIXED, sizeof(AcadClipDataR21));
	new (hGlobalR21)AcadClipDataR21(m_tmpPath, OdString(document->GetPathName()), point);
	CacheGlobalData(ClipboardData::m_FormatR17, hGlobalR21);
}
// </command_console>

bool AeSysDoc::DataSource::DoDragDrop() {
	return (COleDataSource::DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_MOVE) != DROPEFFECT_NONE);
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
void AeSysDoc::OnDrawingutilitiesAudit() {
	const bool bFixErrors = AfxMessageBox(L"Fix any errors detected?", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES;

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
		aiAppAudit.setFixErrors(bFixErrors);
		aiAppAudit.setPrintDest(OdDbAuditInfo::kBoth);
		aiAppAudit.setHostAppServices(&theApp);

		m_DatabasePtr->auditDatabase(&aiAppAudit);
	} catch (const OdError & Error) {
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
	CString Title(L"Audit info - " + GetTitle());
	theApp.m_pAuditDlg->SetWindowText(Title);
	theApp.m_pAuditDlg->ShowWindow(SW_SHOW);

	theApp.m_pAuditDlg = nullptr;
}

BOOL AeSysDoc::DoPromptFileName(CString & fileName, UINT nIDSTitle, DWORD lFlags, BOOL isOpenFileDialog, CDocTemplate * docTemplate) {
	const auto dwgver {m_DatabasePtr->originalFileVersion()};
	auto Extension {fileName.Right(3)};

	const bool isDwg {Extension.CompareNoCase(L"dxf") != 0};

	CString title;
	VERIFY(title.LoadString(nIDSTitle));

	CFileDialog dlgFile(isOpenFileDialog);
	dlgFile.m_ofn.Flags |= lFlags;

	CString Filter;

	Filter = L"AutoCAD 2018 Compatible Drawing |*.dwg|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (isDwg && dwgver == OdDb::vAC32)
		dlgFile.m_ofn.nFilterIndex = 1;

	Filter += L"AutoCAD 2013 Compatible Drawing |*.dwg|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (isDwg && dwgver == OdDb::vAC27)
		dlgFile.m_ofn.nFilterIndex = 2;

	Filter += "AutoCAD 2010 Compatible Drawing |*.dwg|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (isDwg && dwgver == OdDb::vAC24)
		dlgFile.m_ofn.nFilterIndex = 3;

	Filter += "AutoCAD 2007 Compatible Drawing |*.dwg|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (isDwg && dwgver == OdDb::vAC21)
		dlgFile.m_ofn.nFilterIndex = 4;

	Filter += L"AutoCAD 2004 Compatible Drawing |*.dwg|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (isDwg && (dwgver == OdDb::kDHL_1800a || dwgver == OdDb::kDHL_1800))
		dlgFile.m_ofn.nFilterIndex = 5;

	Filter += L"AutoCAD 2000 Compatible Drawing |*.dwg|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (isDwg && dwgver == OdDb::vAC15)
		dlgFile.m_ofn.nFilterIndex = 6;

	Filter += L"AutoCAD R14 Compatible Drawing |*.dwg|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (isDwg && dwgver == OdDb::vAC14)
		dlgFile.m_ofn.nFilterIndex = 7;

	Filter += L"AutoCAD R13 Compatible Drawing |*.dwg|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (isDwg && dwgver == OdDb::vAC13)
		dlgFile.m_ofn.nFilterIndex = 8;

	Filter += L"AutoCAD R12 Compatible Drawing |*.dwg|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (isDwg && dwgver <= OdDb::vAC12)
		dlgFile.m_ofn.nFilterIndex = 9;

	Filter += L"AutoCAD 2018 Compatible DXF |*.dxf|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (!isDwg && (dwgver == OdDb::vAC32))
		dlgFile.m_ofn.nFilterIndex = 10;

	Filter += L"AutoCAD 2013 Compatible DXF |*.dxf|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (!isDwg && (dwgver == OdDb::kDHL_1027))
		dlgFile.m_ofn.nFilterIndex = 11;

	Filter += L"AutoCAD 2010 Compatible DXF |*.dxf|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (!isDwg && (dwgver == OdDb::kDHL_1024))
		dlgFile.m_ofn.nFilterIndex = 12;

	Filter += L"AutoCAD 2007 Compatible DXF |*.dxf|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (!isDwg && (dwgver == OdDb::kDHL_1021))
		dlgFile.m_ofn.nFilterIndex = 13;

	Filter += L"AutoCAD 2004 Compatible DXF |*.dxf|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (!isDwg && (dwgver == OdDb::kDHL_1800a || dwgver == OdDb::kDHL_1800))
		dlgFile.m_ofn.nFilterIndex = 14;

	Filter += L"AutoCAD 2000 Compatible DXF |*.dxf|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (!isDwg && dwgver == OdDb::vAC15)
		dlgFile.m_ofn.nFilterIndex = 15;

	Filter += L"AutoCAD R14 Compatible DXF |*.dxf|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (!isDwg && dwgver == OdDb::vAC14)
		dlgFile.m_ofn.nFilterIndex = 16;

	Filter += L"AutoCAD R13 Compatible DXF |*.dxf|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (!isDwg && dwgver == OdDb::vAC13)
		dlgFile.m_ofn.nFilterIndex = 17;

	Filter += L"AutoCAD R12 Compatible DXF |*.dxf|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (!isDwg && dwgver == OdDb::vAC12)
		dlgFile.m_ofn.nFilterIndex = 18;

	Filter += L"AutoCAD R10 Compatible DXF |*.dxf|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (!isDwg && dwgver == OdDb::vAC10)
		dlgFile.m_ofn.nFilterIndex = 19;

	Filter += L"AutoCAD R9 Compatible DXF |*.dxf|";
	dlgFile.m_ofn.nMaxCustFilter++;
	if (!isDwg && dwgver == OdDb::vAC09)
		dlgFile.m_ofn.nFilterIndex = 20;

	Filter += L"|";
	Filter.Replace('|', '\0');

	if (fileName.Find('.') != -1) {
		fileName = fileName.Left(fileName.Find('.'));
	}
	dlgFile.m_ofn.lpstrFilter = Filter;
	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(MAX_PATH);

	const LPARAM nResult = dlgFile.DoModal();
	fileName.ReleaseBuffer();

	if (fileName.Find('.') == -1) {
		if (dlgFile.m_ofn.nFilterIndex < 10) {
			fileName += L".dwg";
		} else {
			fileName += L".dxf";
		}
	}
	if (dlgFile.m_ofn.nFilterIndex < 10) {
		m_SaveAsType = OdDb::kDwg;
	} else {
		m_SaveAsType = OdDb::kDxf;
	}
	switch (dlgFile.m_ofn.nFilterIndex) {
		case 1:
		case 10:
			m_SaveAsVer = OdDb::vAC32; // R32 (2018) release
		case 2:
		case 11:
			m_SaveAsVer = OdDb::vAC27; // R27 (2013) release
			break;
		case 3:
		case 12:
			m_SaveAsVer = OdDb::vAC24; // R24 (2010) release
			break;
		case 4:
		case 13:
			m_SaveAsVer = OdDb::vAC21;  // R21 (2007) release
			break;
		case 5:
		case 14:
			m_SaveAsVer = OdDb::vAC18; // R18 (2004) release
			break;
		case 6:
		case 15:
			m_SaveAsVer = OdDb::vAC15; // R15 (2000) release
			break;
		case 7:
		case 16:
			m_SaveAsVer = OdDb::vAC14; // R14 (release date 1997)
			break;
		case 8:
		case 17:
			m_SaveAsVer = OdDb::vAC13; // R13 (release date 1994)
			break;
		case 9:
		case 18:
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
	};

	return nResult == IDOK;
}

void AeSysDoc::OnEditClearselection() {
	
	if (m_bDisableClearSel) { return; }
	
	auto cleared {false};
	auto ViewPosition {GetFirstViewPosition()};
	
	while (ViewPosition != NULL) {
		auto View {GetNextView(ViewPosition)};

		if (OdString(View->GetRuntimeClass()->m_lpszClassName).compare(L"AeSysView") == 0 && View->GetDocument() == this) {
			dynamic_cast<AeSysView*>(View)->editorObject().unselect();
			cleared = true;
		}
	}
	if (!cleared) { // No view found
		selectionSet()->clear();
	}
}

void AeSysDoc::OnEditExplode() {
	ExecuteCommand(L"explode");
}

void AeSysDoc::OnEditEntget() {
	OdDbSelectionSetIteratorPtr SelectionSetIterator {selectionSet()->newIterator()};

	if (!SelectionSetIterator->done()) {
		OdDbObjectId selId = SelectionSetIterator->objectId();
		EoDlgEditProperties EditPropertiesDialog(selId, theApp.GetMainWnd());
		m_DatabasePtr->startUndoRecord();
		EditPropertiesDialog.DoModal();
	}
}

void AeSysDoc::OnViewNamedViews() {
	ExecuteCommand(L"VIEW");
}

void AeSysDoc::OnEditUndo() {
	m_bLayoutSwitchable = true;
	m_DatabasePtr->undo();
	m_bLayoutSwitchable = false;
	UpdateAllViews(nullptr);
}

void AeSysDoc::OnUpdateEditUndo(CCmdUI* pCmdUI) {
	theApp.RefreshCommandMenu();
	pCmdUI->Enable(m_DatabasePtr->hasUndo() ? TRUE : FALSE);
}

void AeSysDoc::OnEditRedo() {
	m_bLayoutSwitchable = true;
	m_DatabasePtr->redo();
	m_bLayoutSwitchable = false;
	UpdateAllViews(nullptr);
}

void AeSysDoc::OnUpdateEditRedo(CCmdUI * pCmdUI) {
	pCmdUI->Enable(m_DatabasePtr->hasRedo() ? TRUE : FALSE);
}

void AeSysDoc::OnEditSelectall() {
	OnEditClearselection();
	m_bDisableClearSel = true;
	ExecuteCommand(L"select single all");
	m_bDisableClearSel = false;
	auto ViewPosition{ GetFirstViewPosition() };

	while (ViewPosition != NULL) {
		CView* View = GetNextView(ViewPosition);

		if (CString(View->GetRuntimeClass()->m_lpszClassName).Compare(L"AeSysView") == 0 && View->GetDocument() == this) {
			dynamic_cast<AeSysView*>(View)->editorObject().selectionSetChanged();
		}
	}
}
