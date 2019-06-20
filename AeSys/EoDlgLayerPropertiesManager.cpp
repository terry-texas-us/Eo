#include "stdafx.h"

#include "DbSymUtl.h"

#include "EoDlgLayerPropertiesManager.h"

// EoDlgLayerPropertiesManager dialog

IMPLEMENT_DYNAMIC(EoDlgLayerPropertiesManager, CDialog)

BEGIN_MESSAGE_MAP(EoDlgLayerPropertiesManager, CDialog)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_NOTIFY(NM_DBLCLK, IDC_LAYER_FILTER_TREE, &EoDlgLayerPropertiesManager::OnNMDblclkLayerFilterTree)
	ON_NOTIFY(TVN_KEYDOWN, IDC_LAYER_FILTER_TREE, &EoDlgLayerPropertiesManager::OnTvnKeydownLayerFilterTree)
END_MESSAGE_MAP()

EoDlgLayerPropertiesManager::EoDlgLayerPropertiesManager(CWnd* parent)
	: CDialog(IDD, parent)
	, m_DeltaHeight(0)
	, m_DeltaWidth(0)
	, m_InititialHeight(0)
	, m_InititialWidth(0) {
}

EoDlgLayerPropertiesManager::EoDlgLayerPropertiesManager(OdDbDatabasePtr database, CWnd* parent)
	: CDialog(IDD, parent)
	, m_Database(database)
	, m_DeltaHeight(0)
	, m_DeltaWidth(0)
	, m_InititialHeight(0)
	, m_InititialWidth(0) {
}

EoDlgLayerPropertiesManager::~EoDlgLayerPropertiesManager() {
}

void EoDlgLayerPropertiesManager::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LAYER_FILTER_TREE, m_TreeFilters);
}
int EoDlgLayerPropertiesManager::OnCreate(LPCREATESTRUCT createStructure) {
	if (CDialog::OnCreate(createStructure) == -1) {
		return -1;
	}
	m_InititialWidth = createStructure->cx;
	m_InititialHeight = createStructure->cy;

	return 0;
}
void EoDlgLayerPropertiesManager::OnNMDblclkLayerFilterTree(NMHDR* notifyStructure, LRESULT* result) {
	if (auto h = m_TreeFilters.GetSelectedItem()) {
		const OdLyLayerFilter* lf = static_cast<OdLyLayerFilter*>(reinterpret_cast<void*>(m_TreeFilters.GetItemData(h)));
		if (!lf->dynamicallyGenerated() && !lf->isIdFilter()) {
			//OdaLayerFilterPropDlg(lf, this).DoModal();
		}
	}
	*result = 0;
}

void EoDlgLayerPropertiesManager::OnTvnKeydownLayerFilterTree(NMHDR* notifyStructure, LRESULT* result) {
	const auto pTVKeyDown {reinterpret_cast<tagTVKEYDOWN*>(notifyStructure)};

	if (pTVKeyDown->wVKey == VK_DELETE) {
		if (auto SelectedItem = m_TreeFilters.GetSelectedItem()) {
			OdLyLayerFilter* Filter = static_cast<OdLyLayerFilter*>(reinterpret_cast<void*>(m_TreeFilters.GetItemData(SelectedItem)));
			if (Filter->dynamicallyGenerated()) return;
			if (AfxMessageBox(L"Delete this filter?", MB_YESNO) != IDYES) return;
			Filter->parent()->removeNested(Filter);
			m_TreeFilters.DeleteItem(SelectedItem);

			const OdLyLayerFilter * Root = static_cast<OdLyLayerFilter*>(reinterpret_cast<void*>(m_TreeFilters.GetItemData(m_TreeFilters.GetRootItem())));
			odlyGetLayerFilterManager(m_Database)->setFilters(Root, Root);
		}
	}
	*result = 0;
}
BOOL EoDlgLayerPropertiesManager::OnInitDialog() {
	CDialog::OnInitDialog();

	CBitmap Bitmap;
	Bitmap.LoadBitmapW(IDB_LAYER_FILTERS);
	m_TreeImages.Create(16, 16, ILC_COLOR32, 0, 1);
	m_TreeImages.Add(&Bitmap, RGB(0, 0, 0));
	m_TreeFilters.SetImageList(&m_TreeImages, TVSIL_NORMAL);
	Bitmap.DeleteObject();

	UpdateFiltersTree();

// <tas="testing main dictionary interface">
	OdDbDictionaryPtr MainDictionary = m_Database->getNamedObjectsDictionaryId().safeOpenObject(OdDb::kForRead);
	TRACE1("Main dictionary contains %i entries\n", MainDictionary->numEntries());
// </tas>
	auto MainDictionaryIterator {MainDictionary->newIterator()};
	while (!MainDictionaryIterator->done()) {
		const auto MainDictionaryEntryObjectId {MainDictionaryIterator->objectId()};
		TRACE2("<%4s> \"%s\"\n", MainDictionaryEntryObjectId.getHandle().ascii().c_str(), MainDictionaryIterator->name().c_str());
		if (MainDictionaryIterator->objectId() == m_Database->getLayoutDictionaryId()) {
			OdDbDictionaryPtr LayoutDictionary = m_Database->getLayoutDictionaryId().safeOpenObject(OdDb::kForRead);
			auto LayoutDictionaryIterator {LayoutDictionary->newIterator()};
			while (!LayoutDictionaryIterator->done()) {
				const auto LayoutDictionaryEntryObjectId {LayoutDictionaryIterator->objectId()};
				TRACE2("    <%4s> \"%s\"\n", LayoutDictionaryEntryObjectId.getHandle().ascii().c_str(), LayoutDictionaryIterator->name().c_str());
				LayoutDictionaryIterator->next();
			}
		} else if (MainDictionaryIterator->objectId() == m_Database->getScaleListDictionaryId()) {
			OdDbDictionaryPtr ScaleListDictionary = m_Database->getScaleListDictionaryId().safeOpenObject(OdDb::kForRead);
			auto ScaleListDictionaryIterator {ScaleListDictionary->newIterator()};
			while (!ScaleListDictionaryIterator->done()) {
				const auto ScaleListDictionaryEntryObjectId {ScaleListDictionaryIterator->objectId()};
				TRACE2("    <%4s> \"%s\"\n", ScaleListDictionaryEntryObjectId.getHandle().ascii().c_str(), ScaleListDictionaryIterator->name().c_str());

				ScaleListDictionaryIterator->next();
			}
		}
		MainDictionaryIterator->next();
	}
	return 0;
}
///<summary>Recursive filter tree filling helper</summary>
static void UpdateFilterTree(CTreeCtrl& tree, HTREEITEM parent, const OdLyLayerFilter* root, const OdLyLayerFilter* current) {
	if (root) {
		const auto TreeItem {tree.InsertItem(root->name(), parent)};
		tree.SetItemData(TreeItem, reinterpret_cast<unsigned long>((void*)root));
		const auto Image {root->isIdFilter() ? 2 : 1};
		tree.SetItemImage(TreeItem, Image, Image);
		
		for (const auto& Filter : root->getNestedFilters()) {
			UpdateFilterTree(tree, TreeItem, Filter, current);
		}
		if (current == root) { tree.SelectItem(TreeItem); }
	}
}

void EoDlgLayerPropertiesManager::UpdateFiltersTree() {
	m_TreeFilters.DeleteAllItems();
	auto FilterManager {odlyGetLayerFilterManager(m_Database)};
	OdLyLayerFilterPtr pCurrent;
	
	if (FilterManager->getFilters(m_RootFilter, pCurrent) != eOk) { return; }

	UpdateFilterTree(m_TreeFilters, TVI_ROOT, m_RootFilter, pCurrent);
	m_TreeFilters.SetItemImage(m_TreeFilters.GetRootItem(), 0, 0);
}
void EoDlgLayerPropertiesManager::OnSize(unsigned type, int newWidth, int newHeight) {
	CDialog::OnSize(type, newWidth, newHeight);

	CRect itemRect;
	CRect dlgRect;
	if (GetDlgItem(IDC_STATIC_CURRENT_LAYER)) {
		GetDlgItem(IDC_STATIC_CURRENT_LAYER)->GetWindowRect(&itemRect);
		ScreenToClient(itemRect);
		GetWindowRect(&dlgRect);
		itemRect.right += dlgRect.Width() - m_DeltaWidth;
		GetDlgItem(IDC_STATIC_CURRENT_LAYER)->MoveWindow(itemRect);
	}
	if (GetDlgItem(IDC_STATIC_LAYER_STATISTIC)) {
		GetDlgItem(IDC_STATIC_LAYER_STATISTIC)->GetWindowRect(&itemRect);
		ScreenToClient(itemRect);
		GetWindowRect(&dlgRect);
		GetDlgItem(IDC_STATIC_LAYER_STATISTIC)->MoveWindow(
			itemRect.left, itemRect.top + (dlgRect.Height() - m_DeltaHeight),
			itemRect.Width() + (dlgRect.Width() - m_DeltaWidth), itemRect.Height());
	}
	if (GetDlgItem(IDCANCEL)) {
		GetDlgItem(IDCANCEL)->GetWindowRect(&itemRect);
		ScreenToClient(itemRect);
		GetWindowRect(&dlgRect);
		GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDCANCEL)->MoveWindow(
			itemRect.left + (dlgRect.Width() - m_DeltaWidth), itemRect.top + (dlgRect.Height() - m_DeltaHeight),
			itemRect.Width(), itemRect.Height());
		GetDlgItem(IDCANCEL)->ShowWindow(SW_SHOW);
	}
	if (GetDlgItem(IDOK)) {
		GetDlgItem(IDOK)->GetWindowRect(&itemRect);
		ScreenToClient(itemRect);
		GetWindowRect(&dlgRect);
		GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDOK)->MoveWindow(itemRect.left + (dlgRect.Width() - m_DeltaWidth),
			itemRect.top + (dlgRect.Height() - m_DeltaHeight), itemRect.Width(), itemRect.Height());
		GetDlgItem(IDOK)->ShowWindow(SW_SHOW);
	}
}
void EoDlgLayerPropertiesManager::OnSizing(unsigned side, LPRECT rectangle) {
	CDialog::OnSizing(side, rectangle);

	const CRect rct(*rectangle);

	CRect dlgRect;
	GetWindowRect(&dlgRect);
	m_DeltaWidth = dlgRect.Width();
	m_DeltaHeight = dlgRect.Height();

	if (rct.Width() < m_InititialWidth) {
		switch (side) {
			case WMSZ_LEFT:
			case WMSZ_BOTTOMLEFT:
			case WMSZ_TOPLEFT:
				rectangle->left = rectangle->right - m_InititialWidth;
				break;
			case WMSZ_RIGHT:
			case WMSZ_BOTTOMRIGHT:
			case WMSZ_TOPRIGHT:
				rectangle->right = rectangle->left + m_InititialWidth;
				break;
		}
	}
	if (rct.Height() < m_InititialHeight) {
		switch (side) {
			case WMSZ_BOTTOM:
			case WMSZ_BOTTOMLEFT:
			case WMSZ_BOTTOMRIGHT:
				rectangle->bottom = rectangle->top + m_InititialHeight;
				break;
			case WMSZ_TOP:
			case WMSZ_TOPLEFT:
			case WMSZ_TOPRIGHT:
				rectangle->top = rectangle->bottom - m_InititialHeight;
				break;
		}
	}
}
