#pragma once
#include <DbLayerTableRecord.h>
#include <DbSymUtl.h>
#include <DbDatabase.h>
#include <DbCommandContext.h>
#include <Ed/EdCommandStack.h>
#include <ExDbCommandContext.h>
#include <DbLayoutManager.h>
#include <ExEdBaseIO.h>
#include "EoGeUniquePoint.h"
#include "EoDbText.h"
#include "EoDbMaskedPrimitive.h"
#include "EoDbBlock.h"
#include "EoDbLayer.h"
#include "EoDbLinetypeTable.h"
# include "OdApplicationImpl.h"
class ExStringIO;
class EoDlgUserIOConsole;

class Cmd_VIEW : public OdEdCommand {
public:
	static const OdString name();
	[[nodiscard]] const OdString groupName() const final;
	[[nodiscard]] const OdString globalName() const final;
	void execute(OdEdCommandContext* commandContext) final;
	[[nodiscard]] const OdRxModule* commandApp() const;
	void commandUndef(bool undefIt);
	[[nodiscard]] long commandFlags() const;
};

class Cmd_SELECT : public OdEdCommand {
public:
	static const OdString name();
	[[nodiscard]] const OdString groupName() const final;
	[[nodiscard]] const OdString globalName() const final;
	void execute(OdEdCommandContext* commandContext) final;
	[[nodiscard]] const OdRxModule* commandApp() const;
	void commandUndef(bool undefIt);
	[[nodiscard]] long commandFlags() const;
};

class OdDbDatabaseDoc : public OdDbDatabase {
	static AeSysDoc* g_pDoc;
	mutable AeSysDoc* m_pDoc;
public:
ODRX_DECLARE_MEMBERS(OdDbDatabaseDoc);
	OdDbDatabaseDoc() noexcept;
	AeSysDoc* document() const noexcept;
	static void setDocToAssign(AeSysDoc* document) noexcept;
};

using OdDbDatabaseDocPtr = OdSmartPtr<OdDbDatabaseDoc>;

class AeSysAppDocStaticRxObjects : public OdDbLayoutManagerReactor, public OdEdBaseIO {
	ODRX_NO_HEAP_OPERATORS()
};

class AeSysDoc : public COleDocument, protected OdStaticRxObject<AeSysAppDocStaticRxObjects> {
protected:
	using COleDocument::operator new;
	using COleDocument::operator delete;
	AeSysView* m_pViewer {nullptr};
	EoDlgUserIOConsole* UserIOConsole();
	bool m_bConsole {false};
	bool m_ConsoleResponded {false};
	int m_nCmdActive {0};

	class DataSource : COleDataSource {
		friend class AeSysDoc;
		OdString m_tmpPath;
	public:
		DataSource();
		void Create(AeSysDoc* document, const OdGePoint3d& point = OdGePoint3d::kOrigin);
		bool DoDragDrop(); // hides non-virtual function of parent
		void Empty(); // hides non-virtual function of parent
		~DataSource();
	};

	template <class T>
	struct AcadClipData {
		void init() noexcept {
			memset(this, 0, sizeof(AcadClipData<T>));
		}

		void read(CFile* file) {
			file->Read(this, sizeof(AcadClipData<T>));
		}

		T _tempFileName[0x104]; // name of the temp dwg file, where dragged entities are
		T _origFileName[0x104]; // original file name
		T _version[4]; // version of the original file, e.g. 'R15'
		int _one1; // seem to be always 1
		double _x; // pick point
		double _y;
		double _z;
		int _zero1; // seem to be always zero
		int _one2; // seem to be always 1
		int _unk[4];
		int _zero2[4]; // seem to be always zero
	};

	template <class T>
	struct AcadClipDataConstr : AcadClipData<T> {
		AcadClipDataConstr(const OdString& tempFileName, const OdString& origFileName, const OdGePoint3d& pickPoint) {
			AcadClipData<wchar_t>::init();
			AcadClipData<wchar_t>::_one1 = 1;
			AcadClipData<wchar_t>::_one2 = 1;
			AcadClipData<wchar_t>::_version[0] = 'R';
			AcadClipData<wchar_t>::_version[1] = '1';
			AcadClipData<wchar_t>::_version[2] = '5';
			AcadClipData<wchar_t>::_version[3] = 0;
			AcadClipData<wchar_t>::_x = pickPoint.x;
			AcadClipData<wchar_t>::_y = pickPoint.y;
			AcadClipData<wchar_t>::_z = pickPoint.z;
			memcpy(AcadClipData<wchar_t>::_tempFileName, static_cast<const T*>(tempFileName), odmin((0x100 * sizeof(T)), ((tempFileName.getLength() + 1) * sizeof(T))));
			memcpy(AcadClipData<wchar_t>::_origFileName, static_cast<const T*>(origFileName), odmin((0x100 * sizeof(T)), ((origFileName.getLength() + 1) * sizeof(T))));
		}

		AcadClipDataConstr() {
			AcadClipData<wchar_t>::init();
		}
	};

	using AcadClipDataR15 = AcadClipDataConstr<char>;
	using AcadClipDataR21 = AcadClipDataConstr<wchar_t>;
public:
	class ClipboardData {
	public:
		static unsigned short m_FormatR15;
		static unsigned short m_FormatR16;
		static unsigned short m_FormatR17;
		static unsigned short m_FormatR18;
		static unsigned short m_FormatR19;

		static bool isAcadDataAvailable(COleDataObject* dataObject, bool attach = false) {
			if (attach && !dataObject->AttachClipboard()) { return false; }
			return dataObject->IsDataAvailable(m_FormatR15) || dataObject->IsDataAvailable(m_FormatR16) || dataObject->IsDataAvailable(m_FormatR17) || dataObject->IsDataAvailable(m_FormatR18) ||
				dataObject->IsDataAvailable(m_FormatR19);
		}

		static OdSharedPtr<ClipboardData> get(COleDataObject* dataObject, bool attach = false) {
			if (attach && !dataObject->AttachClipboard()) { return nullptr; }
			OdSharedPtr<ClipboardData> Data {new ClipboardData()};
			if (Data->read(dataObject)) { return Data; }
			return nullptr;
		}

		ClipboardData() noexcept = default;

		bool read(COleDataObject* dataObject) {
			OdSharedPtr<CFile> File {nullptr};
			if ((File = dataObject->GetFileData(m_FormatR15)).get() || (File = dataObject->GetFileData(m_FormatR16)).get()) {
				_isR15format = true;
				_data._r15.read(File);
				return true;
			}
			if ((File = dataObject->GetFileData(m_FormatR17)).get() || (File = dataObject->GetFileData(m_FormatR18)).get() || (File = dataObject->GetFileData(m_FormatR19)).get()) {
				_isR15format = false;
				_data._r21.read(File);
				return true;
			}
			return false;
		}

		OdString tempFileName() {
			return _isR15format ? OdString(_data._r15._tempFileName) : OdString(_data._r21._tempFileName);
		}

		OdGePoint3d pickPoint() {
			return _isR15format ? OdGePoint3d(_data._r15._x, _data._r15._y, _data._r15._z) : OdGePoint3d(_data._r21._x, _data._r21._y, _data._r21._z);
		}

	private:
		union Data {
			AcadClipData<char> _r15;
			AcadClipData<wchar_t> _r21;

			Data() noexcept {
				_r21.init();
			}
		} _data;

		bool _isR15format {false};
	};

protected:
	AeSysDoc() noexcept;
DECLARE_DYNCREATE(AeSysDoc)
	BOOL DoPromptFileName(CString& fileName, unsigned nIDSTitle, unsigned long flags, BOOL openFileDialog, CDocTemplate* documentTemplate);
	OdDbCommandContextPtr m_CommandContext;

	// <command_console>
	OdSmartPtr<EoDlgUserIOConsole> m_UserIOConsole;
	OdSmartPtr<ExStringIO> m_pMacro;
	OdDbCommandContextPtr CommandContext();
	OdEdBaseIO* BaseIO() noexcept;
	OdString CommandPrompt();
	OdString RecentCommand();
	OdString RecentCommandName();
	unsigned long getKeyState() noexcept override;
	OdGePoint3d getPoint(const OdString& prompt, int options, OdEdPointTracker* tracker) override;
	OdString getString(const OdString& prompt, int options, OdEdStringTracker* tracker) override;
	void putString(const OdString& string) override;
	// </command_console>
	// OdDbLayoutManagerReactor
	bool m_LayoutSwitchable {false};
	void layoutSwitched(const OdString& newLayoutName, const OdDbObjectId& newLayout) override;
	bool m_DisableClearSelection {false};
	bool m_bPartial {false};
	OdDb::DwgVersion m_SaveAsVer {OdDb::kDHL_CURRENT};
	OdDb::SaveType m_SaveAsType {OdDb::kDwg};
	EoDb::FileTypes m_SaveAsType_ {EoDb::kUnknown};
	[[nodiscard]] OdDbSelectionSetPtr SelectionSet() const;
	AeSysView* getViewer() noexcept;
	void OnCloseVectorizer(AeSysView* view);
	void setVectorizer(AeSysView* view);
	void ExecuteCommand(const OdString& command, bool echo = true);
	OdDbDatabasePtr m_DatabasePtr;
	void DeleteSelection(bool force);
	void startDrag(const OdGePoint3d& point);
	BOOL OnSaveDocument(const wchar_t* pathName) override;
	BOOL OnCmdMsg(unsigned commandId, int messageCategory, void* commandObject, AFX_CMDHANDLERINFO* handlerInfo) override;
	BOOL OnNewDocument() override;
	BOOL OnOpenDocument(const wchar_t* file) override;
	void DeleteContents() override;
	BOOL CanCloseFrame(CFrameWnd* frame) override;
	~AeSysDoc();
	BOOL DoSave(const wchar_t* pathName, BOOL replace = TRUE) override;
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif
	OdSmartPtr<OdApplicationDocumentImpl> m_pRefDocument;
protected:
	void OnVectorize(const OdString& vectorizerPath);
	void AddRegisteredApp(const OdString& name);
private:
	OdString m_IdentifiedLayerName;
	EoDbLinetypeTable m_LinetypeTable;
	EoDbBlockTable m_BlockTable;
	EoDbLayerTable m_LayerTable;
	EoDbLayer* m_WorkLayer {nullptr};
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

	/// <summary>Constructs 0 to many seperate text primitives for each "\r\n" delimited text block</summary>
	void AddTextBlock(wchar_t* text);

	// Text Style Table interface
	/// <summary>Add a new text style to the text style table.</summary>
	OdDbTextStyleTableRecordPtr AddNewTextStyle(OdString name, OdDbTextStyleTablePtr& textStyles);
	OdDbTextStyleTableRecordPtr AddStandardTextStyle();
	OdDbDimStyleTableRecordPtr AddStandardDimensionStyle();

	// Block Table interface
	EoDbBlockTable* BlockTable() noexcept;
	bool BlockTableIsEmpty();
	unsigned short BlockTableSize();
	int GetBlockReferenceCount(const CString& name);
	POSITION GetFirstBlockPosition();
	void GetNextBlock(POSITION& position, CString& name, EoDbBlock*& block);
	bool LookupBlock(const wchar_t* name, EoDbBlock*& block);
	/// <summary>Removes all blocks and defining primitives.</summary>
	void RemoveAllBlocks();
	void PurgeUnreferencedBlocks();
	void InsertBlock(const wchar_t* name, EoDbBlock* block);
	/// <summary>A layer is converted to a tracing or a job file</summary>
	bool LayerMelt(OdString& name);
	int LinetypeIndexReferenceCount(short linetypeIndex);
	void GetExtents___(AeSysView* view, OdGeExtents3d& extents);
	int NumberOfGroupsInWorkLayer();
	int NumberOfGroupsInActiveLayers();
	void BuildVisibleGroupList(AeSysView* view);

	/// <summary>Displays drawing and determines which groups are detectable.</summary>
	void DisplayAllLayers(AeSysView* view, CDC* deviceContext);

	// Layer Table interface
	void AddLayer(EoDbLayer* layer);
	OdDbObjectId AddLayerTo(OdDbLayerTablePtr layers, EoDbLayer* layer);
	EoDbLayer* AnyLayerRemove(EoDbGroup* group);
	EoDbLayer* GetLayerAt(const OdString& name);
	EoDbLayer* GetLayerAt(int layerIndex);
	[[nodiscard]] int GetLayerTableSize() const;
	[[nodiscard]] int FindLayerAt(const OdString& name) const;
	OdDbLayerTablePtr LayerTable(OdDb::OpenMode openMode = OdDb::kForRead);
	void RemoveAllLayers();
	void RemoveLayerAt(int layerIndex);
	void RemoveEmptyLayers();
	EoDbLayer* SelectLayerBy(const OdGePoint3d& point);
	void PenTranslation(unsigned numberOfColors, std::vector<int>& newColors, std::vector<int>& pCol);
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
	[[nodiscard]] POSITION GetFirstWorkLayerGroupPosition() const;
	[[nodiscard]] EoDbGroup* GetLastWorkLayerGroup() const;
	[[nodiscard]] POSITION GetLastWorkLayerGroupPosition() const;
	EoDbGroup* GetNextWorkLayerGroup(POSITION& position) const;
	EoDbGroup* GetPreviousWorkLayerGroup(POSITION& position) const;
	[[nodiscard]] EoDbLayer* GetWorkLayer() const noexcept;
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
	
	// trap interface
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
	[[nodiscard]] POSITION GetFirstTrappedGroupPosition() const;
	EoDbGroup* GetNextTrappedGroup(POSITION& position);
	EoDbGroupList* GroupsInTrap() noexcept;
	[[nodiscard]] bool IsTrapEmpty() const;
	void ModifyTrappedGroupsColorIndex(short colorIndex);
	void ModifyTrappedGroupsLinetypeIndex(short linetypeIndex);
	void ModifyTrappedGroupsNoteAttributes(EoDbFontDefinition& fontDef, EoDbCharacterCellDefinition& cellDef, int attributes);
	void RemoveAllTrappedGroups();
	EoDbGroup* RemoveLastTrappedGroup();
	POSITION RemoveTrappedGroup(EoDbGroup* group);
	void RemoveTrappedGroupAt(POSITION position);
	void SetTrapPivotPoint(const OdGePoint3d& pivotPoint) noexcept;
	void SquareTrappedGroups(AeSysView* view);
	void TracingFuse(OdString& nameAndLocation);
	bool TracingLoadLayer(const OdString& file, EoDbLayer* layer);
	bool TracingOpen(const OdString& fileName);
	void TransformTrappedGroups(const EoGeMatrix3d& transformMatrix);
	[[nodiscard]] int TrapGroupCount() const;
	[[nodiscard]] OdGePoint3d TrapPivotPoint() const noexcept;
	
	// Nodal list interface (includes list of groups, primitives and unique points)
	void DeleteNodalResources();
	/// <summary>Maintains a list of the primatives with at least one identified node.</summary>
	void UpdateNodalList(EoDbGroup* group, EoDbPrimitive* primitive, unsigned long mask, int bit, OdGePoint3d point);
	POSITION AddNodalGroup(EoDbGroup* group);
	POSITION FindNodalGroup(EoDbGroup* group);
	[[nodiscard]] POSITION GetFirstNodalGroupPosition() const;
	EoDbGroup* GetNextNodalGroup(POSITION& position);
	void RemoveAllNodalGroups();
	POSITION AddMaskedPrimitive(EoDbMaskedPrimitive* maskedPrimitive);
	[[nodiscard]] POSITION GetFirstMaskedPrimitivePosition() const;
	EoDbMaskedPrimitive* GetNextMaskedPrimitive(POSITION& position);
	void RemoveAllMaskedPrimitives();
	unsigned long GetPrimitiveMask(EoDbPrimitive* primitive);
	void AddPrimitiveBit(EoDbPrimitive* primitive, int bit);
	void RemovePrimitiveBit(EoDbPrimitive* primitive, int bit);
	int AddUniquePoint(const OdGePoint3d& point);
	void DisplayUniquePoints();

	[[nodiscard]] POSITION GetFirstUniquePointPosition() const { return m_UniquePoints.GetHeadPosition(); }

	EoGeUniquePoint* GetNextUniquePoint(POSITION& position);
	void RemoveUniquePointAt(POSITION position);
	void RemoveAllUniquePoints();
	int RemoveUniquePoint(const OdGePoint3d& point);
	
	// Generated message map functions
	void OnBlocksLoad();
	void OnPurgeUnreferencedBlocks();
	void OnClearActiveLayers();
	void OnClearAllLayers();
	void OnClearAllTracings();
	void OnClearMappedTracings();
	void OnClearViewedTracings();
	void OnClearWorkingLayer();
	/// <summary>The current view is copied to the clipboard as an enhanced metafile.</summary>
	void OnEditImageToClipboard();
	void OnEditSegToWork();
	/// <summary>Pastes clipboard to drawing. Only EoGroups format handled and no translation is performed.</summary>
	void OnEditTrace();
	void OnEditTrapCopy();
	void OnEditTrapCut();
	void OnEditTrapDelete();
	/// <summary>Initializes current trap and all trap component lists.</summary>
	void OnEditTrapQuit();
	/// <summary>Pastes clipboard to drawing. If the clipboard has the EoGroups format, all other formats are ignored.</summary>
	void OnEditTrapPaste();
	/// <summary>Adds all groups in the work layer to the trap.</summary>
	void OnEditTrapWork();
	/// <summary>Add all groups in all work and active layers to the trap.</summary>
	void OnEditTrapWorkAndActive();
	void OnFile();
	void OnFileManage();
	void OnFilePagesetup();
	void OnFileQuery();
	void OnFileTracing();
	void OnHelpKey();
#ifdef OD_OLE_SUPPORT
	void OnInsertOleobject();
#endif // OD_OLE_SUPPORT
	void OnInsertTracing();
	void OnLayerActive();
	void OnLayerCurrent();
	void OnLayerLock();
	void OnLayerMelt();
	void OnLayerOff();
	void OnLayersSetAllActive();
	void OnLayersSetAllLocked();
	void OnPurgeUnusedLayers();
	void OnPurgeDuplicateObjects();
	void OnPurgeEmptyNotes();
	void OnPurgeEmptyGroups();
	void OnPensEditColors();
	void OnPensLoadColors();
	void OnPensRemoveUnusedLinetypes();
	void OnPensTranslate();
	/// <summary>Breaks a primitive into a simpler set of primitives.</summary>
	void OnPrimBreak();
	/// <summary>Searches for closest detectible primitive. If found, primitive is lifted from its group, inserted into a new group which is added to deleted group list. The primitive resources are not freed.</summary>
	void OnToolsPrimitiveDelete();
	void OnPrimExtractNum();
	void OnPrimExtractStr();
	/// <summary>Positions the cursor at a "control" point on the current engaged group.</summary>
	void OnPrimGotoCenterPoint();
	/// <summary>Picks a primative and modifies its attributes to current settings.</summary>
	void OnPrimModifyAttributes();
	void OnToolsPrimitiveSnaptoendpoint();
	/// <summary>Reduces complex primitives and group references to a simpler form</summary>
	void OnToolsGroupBreak();
	/// <summary>
	/// Searches for closest detectible group.  If found, group is removed
	/// from all general group lists and added to deleted group list.
	/// Notes: The group resources are not freed.
	/// </summary>
	void OnToolsGroupDelete();
	void OnToolsGroupDeletelast();
	/// <summary>Exchanges the first and last groups on the deleted group list.</summary>
	void OnToolsGroupExchange();
	void OnToolsGroupUndelete();
	void OnSetupFillHatch();
	void OnSetupFillHollow() noexcept;
	void OnSetupFillPattern() noexcept;
	void OnSetupFillSolid() noexcept;
	void OnSetupGotoPoint();
	void OnSetupNote();
	void OnSetupOptionsDraw();
	void OnSetupPenColor();
	void OnSetupLayerproperties();
	void OnSetupLinetype();
	void OnSetupSavePoint();
	void OnTracingActive();
	void OnTracingCurrent();
	void OnTracingFuse();
	void OnTracingLock();
	void OnTracingOff();
	void OnTrapCommandsBlock();
	void OnTrapCommandsCompress();
	void OnTrapCommandsExpand();
	void OnTrapCommandsFilter();
	void OnTrapCommandsInvert();
	void OnTrapCommandsQuery();
	void OnTrapCommandsSquare();
	void OnTrapCommandsUnblock();
	// Returns a pointer to the currently active document.
	static AeSysDoc* GetDoc();
protected:
DECLARE_MESSAGE_MAP()
public:
	void OnViewSetactivelayout();
	void OnDrawingutilitiesAudit();
	// <command_console>
	void OnEditClearselection();
	void OnEditSelectall();
	void OnEditConsole();
	void OnEditExplode();
	void OnEditEntget();
	void OnViewNamedViews();
	void OnEditUndo();
	void OnUpdateEditUndo(CCmdUI* commandUserInterface);
	void OnUpdateEditRedo(CCmdUI* commandUserInterface);
	void OnEditRedo();
	// </command_console>
	void OnVectorize(); // <tas="This is the vectorize menu and toolbar button handler in Oda"</tas>
	// <tas="Will not use"> void OnRemoteGeomViewer();"</tas>
	void OnUpdateVectorize(CCmdUI* commandUserInterface);
};
