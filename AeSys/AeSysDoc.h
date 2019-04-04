#pragma once

#include "DbDatabase.h"
#include "DbCommandContext.h"
#include "Ed/EdCommandStack.h"
#include "ExDbCommandContext.h"
#include "StaticRxObject.h"
#include "SharedPtr.h"
#include "DbLayoutManager.h"

#include "ExEdBaseIO.h"

#include "EoDbLinetypeTable.h"

#ifdef ODAMFC_EXPORT_SYMBOL
# include "EoMfcExportImpl.h"
#endif // ODAMFC_EXPORT_SYMBOL

class AeSysDoc;
class AeSysView;
class ExStringIO;

#ifdef DEV_COMMAND_CONSOLE
class EoDlgUserIOConsole;

class Cmd_VIEW : public OdEdCommand {
public:
	static const OdString name();
	const OdString groupName() const;
	const OdString globalName() const;
	void execute(OdEdCommandContext* commandContext);
	const OdRxModule* commandApp() const;
	void commandUndef(bool undefIt);
	OdInt32 commandFlags() const;
};

class Cmd_SELECT : public OdEdCommand {
public:
	static const OdString name();
	const OdString groupName() const;
	const OdString globalName() const;
	void execute(OdEdCommandContext* commandContext);
	const OdRxModule* commandApp() const;
	void commandUndef(bool undefIt);
	OdInt32 commandFlags() const;
};

class Cmd_DISPLAY_DIFFS : public OdEdCommand {
public:
	static const OdString name();
	const OdString groupName() const;
	const OdString globalName() const;
	void execute(OdEdCommandContext* commandContext);
	const OdRxModule* commandApp() const;
	void commandUndef(bool undefIt);
	OdInt32 commandFlags() const;
};
#endif // DEV_COMMAND_CONSOLE

class OdDbDatabaseDoc : public OdDbDatabase {
	static  AeSysDoc* g_pDoc;
	mutable AeSysDoc* m_pDoc;
public:
	ODRX_DECLARE_MEMBERS(OdDbDatabaseDoc);

	OdDbDatabaseDoc();

	AeSysDoc* document() const noexcept;

	static void setDocToAssign(AeSysDoc* document) noexcept;
};

typedef OdSmartPtr<OdDbDatabaseDoc> OdDbDatabaseDocPtr;

class AeSysAppDocStaticRxObjects : public OdDbLayoutManagerReactor
#ifdef DEV_COMMAND_CONSOLE
	                               , public OdEdBaseIO
#endif // DEV_COMMAND_CONSOLE
{
	ODRX_NO_HEAP_OPERATORS();
};

class AeSysDoc : public COleDocument
	           , protected OdStaticRxObject<AeSysAppDocStaticRxObjects>
{
protected:
	using COleDocument::operator new;
	using COleDocument::operator delete;

	AeSysView* m_pViewer;

#ifdef DEV_COMMAND_CONSOLE
	bool m_bConsole;
	bool m_bConsoleResponded;
	int m_nCmdActive;
	EoDlgUserIOConsole* console();
#endif // DEV_COMMAND_CONSOLE

	class DataSource : COleDataSource {
		friend class AeSysDoc;
		OdString m_tmpPath;
	public:
		DataSource();
		void Create(AeSysDoc* document, const OdGePoint3d& point = OdGePoint3d::kOrigin) noexcept;
		bool DoDragDrop();
		void Empty();
		~DataSource();
	};
	template<class TChar>
	struct AcadClipData {
		void init() noexcept {
			memset(this, 0, sizeof(AcadClipData<TChar>));
		}
		void read(CFile* pFile) {
			pFile->Read(this, sizeof(AcadClipData<TChar>) );
		}
		TChar _tempFileName[0x104];   // name of the temp dwg file, where dragged entities are
		TChar _origFileName[0x104];   // original file name
		TChar _version[4];            // version of the original file, e.g. 'R15'
		int _one1;                    // seem to be always 1
		double _x, _y, _z;            // pick point
		int _zero1;                   // seem to be always zero
		int _one2;                    // seem to be always 1
		int _unk[4];                  //
		int _zero2[4];                // seem to be always zero
	};
	template<class TChar>
	struct AcadClipDataConstr : public AcadClipData<TChar> {
		AcadClipDataConstr(const OdString& tempFileName,const OdString& origFileName, const OdGePoint3d& pickPoint) {
			init();
			_one1 = 1;
			_one2 = 1;
			_version[0] = 'R';
			_version[1] = '1';
			_version[2] = '5';
			_version[3] = 0;
			_x = pickPoint.x;
			_y = pickPoint.y;
			_z = pickPoint.z;
			memcpy(_tempFileName, (const TChar*)tempFileName, odmin((0x100*sizeof(TChar)),((tempFileName.getLength()+1)*sizeof(TChar))) );
			memcpy(_origFileName, (const TChar*)origFileName, odmin((0x100*sizeof(TChar)),((origFileName.getLength()+1)*sizeof(TChar))) );
		}
		AcadClipDataConstr() {
			init();
		}
	};
	typedef AcadClipDataConstr<char> AcadClipDataR15;
	typedef AcadClipDataConstr<OdChar> AcadClipDataR21;

public:
	class ClipboardData {
	public:
		static unsigned short m_FormatR15;
		static unsigned short m_FormatR16;
		static unsigned short m_FormatR17;
		static unsigned short m_FormatR18;
		static unsigned short m_FormatR19;
		static bool isAcadDataAvailable(COleDataObject* pDataObject, bool bAttach = false) {
			if (bAttach && !pDataObject->AttachClipboard()) {
				return false;
			}
			return pDataObject->IsDataAvailable(m_FormatR15) || pDataObject->IsDataAvailable(m_FormatR16) || pDataObject->IsDataAvailable(m_FormatR17) || pDataObject->IsDataAvailable(m_FormatR18) || pDataObject->IsDataAvailable(m_FormatR19);
		}
		static OdSharedPtr<ClipboardData> get(COleDataObject* pDataObject, bool bAttach = false) {
			if (bAttach && !pDataObject->AttachClipboard())
				return 0;

			OdSharedPtr<ClipboardData> pData = new ClipboardData();
			if (pData->read(pDataObject))
				return pData;
			return 0;
		}
		ClipboardData() :
			_isR15format(false) {
		}
		bool read(COleDataObject* pDataObject) {
			OdSharedPtr<CFile> pFile = 0;
			if ((pFile = pDataObject->GetFileData(m_FormatR15)).get() || (pFile = pDataObject->GetFileData(m_FormatR16)).get() ) {
				_isR15format = true;
				_data._r15.read(pFile);
				return true;
			}
			else if ((pFile = pDataObject->GetFileData(m_FormatR17)).get() || (pFile = pDataObject->GetFileData(m_FormatR18)).get()  || (pFile = pDataObject->GetFileData(m_FormatR19)).get()) {
				_isR15format = false;
				_data._r21.read(pFile);
				return true;
			}
			else {
				return false;
			}
		}
		OdString tempFileName() { return _isR15format ? OdString(_data._r15._tempFileName) : OdString(_data._r21._tempFileName); }
		OdGePoint3d pickPoint() { return _isR15format ? OdGePoint3d(_data._r15._x, _data._r15._y, _data._r15._z) : OdGePoint3d(_data._r21._x, _data._r21._y, _data._r21._z); }

	private:
		union Data {
			AcadClipData<char>   _r15;
			AcadClipData<OdChar> _r21;
			Data() { _r21.init(); }
		} _data;
		bool _isR15format;
	};

protected:
	AeSysDoc();
	DECLARE_DYNCREATE(AeSysDoc)

	BOOL DoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* documentTemplate);

	OdDbCommandContextPtr m_pCmdCtx;

#ifdef DEV_COMMAND_CONSOLE
	OdSmartPtr<EoDlgUserIOConsole> m_pConsole;
	OdSmartPtr<ExStringIO> m_pMacro;
	OdDbCommandContextPtr cmdCtx();
	OdEdBaseIO* cmdIO();
	OdString commandPrompt();
	OdString recentCmd();
	OdString AeSysDoc::recentCmdName();
#endif // DEV_COMMAND_CONSOLE

#ifdef DEV_COMMAND_CONSOLE
	// <OdEdBaseIO virtuals>
	virtual OdUInt32 getKeyState();
	OdGePoint3d AeSysDoc::getPoint(const OdString& prompt, int options, OdEdPointTracker* tracker);
	OdString getString(const OdString& prompt, int options, OdEdStringTracker* tracker);
	void putString(const OdString& string);
    // </OdEdBaseIO virtuals>
#endif // DEV_COMMAND_CONSOLE

	// OdDbLayoutManagerReactor
	bool m_bLayoutSwitchable;
	void layoutSwitched(const OdString& newLayoutName, const OdDbObjectId& newLayout);
	bool m_bDisableClearSel;

public:
	bool m_bPartial;
	OdDb::DwgVersion m_SaveAsVer;
	OdDb::SaveType m_SaveAsType;
	EoDb::FileTypes m_SaveAsType_;

public:
	OdDbSelectionSetPtr selectionSet() const;
	AeSysView* AeSysDoc::getViewer() noexcept;
	void OnCloseVectorizer(AeSysView* view);
	void setVectorizer(AeSysView* view);
	void ExecuteCommand(const OdString& command, bool bEcho = true) noexcept;
	
	OdDbDatabasePtr m_DatabasePtr;

#ifdef DEV_COMMAND_CONSOLE
	void DeleteSelection(bool force);
#endif // DEV_COMMAND_CONSOLE

	void startDrag(const OdGePoint3d& point);

public:
	virtual BOOL OnSaveDocument(LPCWSTR pathName);
	virtual BOOL OnCmdMsg(UINT nID, int code, void* extra, AFX_CMDHANDLERINFO* handlerInfo);
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCWSTR pathName);
	virtual void DeleteContents();
	virtual BOOL CanCloseFrame(CFrameWnd* frame);

public:
	virtual ~AeSysDoc();
	virtual BOOL DoSave(LPCWSTR pathName, BOOL replace = TRUE);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

#ifdef ODAMFC_EXPORT_SYMBOL
	OdSmartPtr<EoApDocumentImpl> m_pRefDocument;
#endif // ODAMFC_EXPORT_SYMBOL

protected:
	void OnVectorize(const OdString& vectorizerPath);

private:
	OdString m_IdentifiedLayerName;

	EoDbLinetypeTable m_LinetypeTable;
	EoDbBlockTable m_BlockTable;
	EoDbLayerTable m_LayerTable;
	EoDbLayer* m_WorkLayer;
	EoDbGroupList m_DeletedGroupList;
	EoDbGroupList m_TrappedGroupList;
	OdGePoint3d m_TrapPivotPoint;

	EoDbGroupList m_NodalGroupList;
	CObList m_MaskedPrimitives;
	CObList m_UniquePoints;

public:
	void UpdateGroupInAllViews(LPARAM hint, EoDbGroup* group);
	void UpdateGroupsInAllViews(LPARAM hint, EoDbGroupList* groups);
	void UpdateLayerInAllViews(LPARAM hint, EoDbLayer* layer);
	void UpdatePrimitiveInAllViews(LPARAM hint, EoDbPrimitive* primitive);

	void InitializeGroupAndPrimitiveEdit();

	/// <summary>Constructs 0 to many seperate text primitives for each "\r\n" delimited substr.</summary>
	void AddTextBlock(LPWSTR pszText);

// Text Style Table interface
/// <summary>Add a new text style to the text style table.</summary>
	OdDbTextStyleTableRecordPtr AddNewTextStyle(OdString name, OdDbTextStyleTablePtr textStyles);
    OdDbTextStyleTableRecordPtr AddStandardTextStyle();

// Block Table interface
	EoDbBlockTable* BlockTable() noexcept;
	bool BlockTableIsEmpty();
	OdUInt16 BlockTableSize();
	int GetBlockReferenceCount(const CString& name);
	POSITION GetFirstBlockPosition();
	void GetNextBlock(POSITION& position, CString& name, EoDbBlock*& block);
	bool LookupBlock(CString name, EoDbBlock*& block);
	/// <summary>Removes all blocks and defining primitives.</summary>
	void RemoveAllBlocks();
	void PurgeUnreferencedBlocks();
	void InsertBlock(const OdString& name, EoDbBlock* block);
	/// <summary>A layer is converted to a tracing or a job file</summary>
	bool LayerMelt(OdString& name);
	int LinetypeIndexReferenceCount(OdInt16 linetypeIndex);
	void GetExtents___(AeSysView* view, OdGeExtents3d& extents);
	int NumberOfGroupsInWorkLayer();
	int NumberOfGroupsInActiveLayers();
	/// <summary>Displays drawing and determines which groups are detectable.</summary>
	void DisplayAllLayers(AeSysView* view, CDC* deviceContext);

// Layer Table interface
	void AddLayer(EoDbLayer* layer);
	OdDbObjectId AddLayerTo(OdDbLayerTablePtr layers, EoDbLayer* layer);
	EoDbLayer* AnyLayerRemove(EoDbGroup* group);
	EoDbLayer* GetLayerAt(const OdString& layerName);
	EoDbLayer* GetLayerAt(int layerIndex);
	int GetLayerTableSize() const;
	int FindLayerAt(const OdString& layerName) const;
	OdDbLayerTablePtr LayerTable(OdDb::OpenMode openMode = OdDb::kForRead);
	void RemoveAllLayers();
	void RemoveLayerAt(int layerIndex);
	void RemoveEmptyLayers();
	EoDbLayer* SelectLayerBy(const OdGePoint3d& point);
	void PenTranslation(OdUInt16, OdInt16*, OdInt16*);
	void PurgeDuplicateObjects();
	int RemoveEmptyNotesAndDelete();
	int RemoveEmptyGroups();
	void ResetAllViews();
	void AddGroupToAllViews(EoDbGroup* group);
	void AddGroupsToAllViews(EoDbGroupList* groups);
	void RemoveGroupFromAllViews(EoDbGroup* group);
	void RemoveAllGroupsFromAllViews();

// <Work Layer> interface
	void AddWorkLayerGroup(EoDbGroup* group);
	void AddWorkLayerGroups(EoDbGroupList* groups);
	POSITION FindWorkLayerGroup(EoDbGroup* group) const;
	POSITION GetFirstWorkLayerGroupPosition() const;
	EoDbGroup* GetLastWorkLayerGroup() const;
	POSITION GetLastWorkLayerGroupPosition() const;
	EoDbGroup* GetNextWorkLayerGroup(POSITION& position) const;
	EoDbGroup* GetPreviousWorkLayerGroup(POSITION& position) const;
	EoDbLayer* GetWorkLayer() const noexcept;
	void InitializeWorkLayer();
	OdDbObjectId SetCurrentLayer(OdDbLayerTableRecordPtr layerTableRecord);
// </Work Layer>
	void WriteShadowFile();

// Deleted groups interface
	POSITION DeletedGroupsAddHead(EoDbGroup* group);
	POSITION DeletedGroupsAddTail(EoDbGroup* group);
	EoDbGroup* DeletedGroupsRemoveHead();
	void DeletedGroupsRemoveGroups();
	EoDbGroup* DeletedGroupsRemoveTail();
	/// <summary>Restores the last group added to the deleted group list.</summary>
	void DeletedGroupsRestore();

public: // trap interface
	void AddGroupsToTrap(EoDbGroupList* groups);
	POSITION AddGroupToTrap(EoDbGroup* group);
	/// <summary>Builds a single group from two or more groups in trap.</summary>
	/// <remarks>The new group is added to the hot layer even if the trap contained groups from one or more warm layers.</remarks>
	void CompressTrappedGroups();
	void CopyTrappedGroups(const OdGeVector3d& translate);
	/// <summary>The current trap is copied to the clipboard. This is done with two independent clipboard formats. The standard enhanced metafile and the private EoDbGroupList which is read exclusively by Peg.</summary>
	void CopyTrappedGroupsToClipboard(AeSysView* view);
	void DeleteAllTrappedGroups();
	/// <summary>Expands compressed groups.</summary>
	/// <remarks>The new groups are added to the hot layer even if the trap contained groups from one or more warm layers.</remarks>
	void ExpandTrappedGroups();
	POSITION FindTrappedGroup(EoDbGroup* group);
	POSITION GetFirstTrappedGroupPosition() const;
	EoDbGroup* GetNextTrappedGroup(POSITION& position);
	EoDbGroupList* GroupsInTrap() noexcept;
	BOOL IsTrapEmpty() const;
	void ModifyTrappedGroupsColorIndex(OdInt16 colorIndex);
	void ModifyTrappedGroupsLinetypeIndex(OdInt16 linetypeIndex);
	void ModifyTrappedGroupsNoteAttributes(EoDbFontDefinition& fontDef, EoDbCharacterCellDefinition& cellDef, int attributes);
	void RemoveAllTrappedGroups();
	EoDbGroup* RemoveLastTrappedGroup();
	POSITION RemoveTrappedGroup(EoDbGroup* group);
	void RemoveTrappedGroupAt(POSITION position);
	void SetTrapPivotPoint(const OdGePoint3d& pivotPoint) noexcept;
	void SquareTrappedGroups(AeSysView* view);

public:
	void TracingFuse(OdString& nameAndLocation);
	bool TracingLoadLayer(const OdString& file, EoDbLayer* layer);
	bool TracingOpen(const OdString& pathName);

public:
	void TransformTrappedGroups(const EoGeMatrix3d& transformMatrix);
	int TrapGroupCount() const;
	OdGePoint3d TrapPivotPoint() const noexcept;

public: // Nodal list interface (includes list of groups, primitives and unique points)
	void DeleteNodalResources();
	/// <summary>Maintains a list of the primatives with at least one identified node.</summary>
	void UpdateNodalList(EoDbGroup* group, EoDbPrimitive* primitive, DWORD mask, int bit, OdGePoint3d point);
	POSITION AddNodalGroup(EoDbGroup* group);
	POSITION FindNodalGroup(EoDbGroup* group);
	POSITION GetFirstNodalGroupPosition() const;
	EoDbGroup* GetNextNodalGroup(POSITION& position);
	void RemoveAllNodalGroups();
	POSITION AddMaskedPrimitive(EoDbMaskedPrimitive* maskedPrimitive);
	POSITION GetFirstMaskedPrimitivePosition() const;
	EoDbMaskedPrimitive* GetNextMaskedPrimitive(POSITION& position);
	void RemoveAllMaskedPrimitives();
	DWORD GetPrimitiveMask(EoDbPrimitive* primitive);
	void AddPrimitiveBit(EoDbPrimitive* primitive, int bit);
	void RemovePrimitiveBit(EoDbPrimitive* primitive, int bit);
	int AddUniquePoint(const OdGePoint3d& point);
	POSITION AddUniquePoint(EoGeUniquePoint* uniquePoint);
	void DisplayUniquePoints();
	POSITION GetFirstUniquePointPosition() const;
	EoGeUniquePoint* GetNextUniquePoint(POSITION& position);
	void RemoveUniquePointAt(POSITION position);
	void RemoveAllUniquePoints();
	int RemoveUniquePoint(const OdGePoint3d& point);

public: // Generated message map functions
	afx_msg void OnBlocksLoad();
	afx_msg void OnPurgeUnreferencedBlocks();
	afx_msg void OnClearActiveLayers();
	afx_msg void OnClearAllLayers();
	afx_msg void OnClearAllTracings();
	afx_msg void OnClearMappedTracings();
	afx_msg void OnClearViewedTracings();
	afx_msg void OnClearWorkingLayer();
	/// <summary>The current view is copied to the clipboard as an enhanced metafile.</summary>
	afx_msg void OnEditImageToClipboard();
	afx_msg void OnEditSegToWork();
	/// <summary>Pastes clipboard to drawing. Only EoGroups format handled and no translation is performed.</summary>
	afx_msg void OnEditTrace();
	afx_msg void OnEditTrapCopy();
	afx_msg void OnEditTrapCut();
	afx_msg void OnEditTrapDelete();
	/// <summary>Initializes current trap and all trap component lists.</summary>
	afx_msg void OnEditTrapQuit();
	/// <summary>Pastes clipboard to drawing. If the clipboard has the EoGroups format, all other formats are ignored.</summary>
	afx_msg void OnEditTrapPaste();
	/// <summary>Adds all groups in the work layer to the trap.</summary>
	afx_msg void OnEditTrapWork();
	/// <summary>Add all groups in all work and active layers to the trap.</summary>
	afx_msg void OnEditTrapWorkAndActive();
	afx_msg void OnFile();
	afx_msg void OnFileManage();
   	afx_msg void OnFilePagesetup();
	afx_msg void OnFileQuery();
	afx_msg void OnFileTracing();
	afx_msg void OnHelpKey();
#ifdef OD_OLE_SUPPORT
	afx_msg void OnInsertOleobject();
#endif // OD_OLE_SUPPORT
	afx_msg void OnInsertTracing();
	afx_msg void OnLayerActive();
	afx_msg void OnLayerCurrent();
	afx_msg void OnLayerLock();
	afx_msg void OnLayerMelt();
	afx_msg void OnLayerOff();
	afx_msg void OnLayersSetAllActive();
	afx_msg void OnLayersSetAllLocked();
	afx_msg void OnPurgeUnusedLayers();
	afx_msg void OnPurgeDuplicateObjects();
	afx_msg void OnPurgeEmptyNotes();
	afx_msg void OnPurgeEmptyGroups();
	afx_msg void OnPensEditColors();
	afx_msg void OnPensLoadColors();
	afx_msg void OnPensRemoveUnusedLinetypes();
	afx_msg void OnPensTranslate();
	/// <summary>Breaks a primitive into a simpler set of primitives.</summary>
	afx_msg void OnPrimBreak();
	/// <summary>Searches for closest detectible primitive. If found, primitive is lifted from its group, inserted into a new group which is added to deleted group list. The primitive resources are not freed.</summary>
	afx_msg void OnToolsPrimitiveDelete();
	afx_msg void OnPrimExtractNum();
	afx_msg void OnPrimExtractStr();
	/// <summary>Positions the cursor at a "control" point on the current engaged group.</summary>
	afx_msg void OnPrimGotoCenterPoint();
	/// <summary>Picks a primative and modifies its attributes to current settings.</summary>
	afx_msg void OnPrimModifyAttributes();
	afx_msg void OnToolsPrimitiveSnaptoendpoint();
	/// <summary>Reduces complex primitives and group references to a simpler form</summary>
	afx_msg void OnToolsGroupBreak();
	/// <summary>
	/// Searches for closest detectible group.  If found, group is removed
	/// from all general group lists and added to deleted group list.
	/// Notes: The group resources are not freed.
	/// </summary>
	afx_msg void OnToolsGroupDelete();
	afx_msg void OnToolsGroupDeletelast();
	/// <summary>Exchanges the first and last groups on the deleted group list.</summary>
	afx_msg void OnToolsGroupExchange();
	afx_msg void OnToolsGroupUndelete();
	afx_msg void OnSetupFillHatch();
	afx_msg void OnSetupFillHollow() noexcept;
	afx_msg void OnSetupFillPattern() noexcept;
	afx_msg void OnSetupFillSolid() noexcept;
	afx_msg void OnSetupGotoPoint();
	afx_msg void OnSetupNote();
	afx_msg void OnSetupOptionsDraw();
	afx_msg void OnSetupPenColor();
	afx_msg void OnSetupLayerproperties();
	afx_msg void OnSetupLinetype();
	afx_msg void OnSetupSavePoint();
	afx_msg void OnTracingActive();
	afx_msg void OnTracingCurrent();
	afx_msg void OnTracingFuse();
	afx_msg void OnTracingLock();
	afx_msg void OnTracingOff();
	afx_msg void OnTrapCommandsBlock();
	afx_msg void OnTrapCommandsCompress();
	afx_msg void OnTrapCommandsExpand();
	afx_msg void OnTrapCommandsFilter();
	afx_msg void OnTrapCommandsInvert();
	afx_msg void OnTrapCommandsQuery();
	afx_msg void OnTrapCommandsSquare();
	afx_msg void OnTrapCommandsUnblock();
	// Returns a pointer to the currently active document.
	static AeSysDoc* GetDoc(void);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnViewSetactivelayout();
	afx_msg void OnDrawingutilitiesAudit();
#ifdef DEV_COMMAND_CONSOLE
	afx_msg void OnEditClearselection();
	afx_msg void OnEditSelectall();
	afx_msg void OnEditConsole();
	afx_msg void OnEditExplode();
	afx_msg void OnEditEntget();
	afx_msg void OnViewNamedViews();
#endif // DEV_COMMAND_CONSOLE
	afx_msg void OnVectorize(); // <tas="This is the vectorize menu and toolbar button handler in Oda"</tas>
	// <tas="Will not use"> afx_msg void OnRemoteGeomViewer();"</tas>
	afx_msg void OnUpdateVectorize(CCmdUI* pCmdUI);

};
