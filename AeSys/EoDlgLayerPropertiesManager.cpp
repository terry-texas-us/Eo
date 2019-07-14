#include "stdafx.h"
#include <DbSymUtl.h>
#include "EoDlgLayerPropertiesManager.h"
IMPLEMENT_DYNAMIC(EoDlgLayerPropertiesManager, CDialog)
#pragma warning(push)
#pragma warning(disable : 4191) // (level 3) 'operator': unsafe conversion from 'type_of_expression' to 'type_required'
BEGIN_MESSAGE_MAP(EoDlgLayerPropertiesManager, CDialog)
		ON_WM_CREATE()
		ON_WM_SIZE()
		ON_WM_SIZING()
		ON_NOTIFY(NM_DBLCLK, IDC_LAYER_FILTER_TREE, &EoDlgLayerPropertiesManager::OnNmDoubleClickLayerFilterTree)
		ON_NOTIFY(TVN_KEYDOWN, IDC_LAYER_FILTER_TREE, &EoDlgLayerPropertiesManager::OnTvnKeydownLayerFilterTree)
END_MESSAGE_MAP()
#pragma warning (pop)
EoDlgLayerPropertiesManager::EoDlgLayerPropertiesManager(CWnd* parent)
	: CDialog(IDD, parent) {}

EoDlgLayerPropertiesManager::EoDlgLayerPropertiesManager(const OdDbDatabasePtr& database, CWnd* parent)
	: CDialog(IDD, parent)
	, m_Database(database) {}

void EoDlgLayerPropertiesManager::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_LAYER_FILTER_TREE, treeFilters);
}

int EoDlgLayerPropertiesManager::OnCreate(const LPCREATESTRUCTW createStructure) {
	if (CDialog::OnCreate(createStructure) == -1) {
		return -1;
	}
	initialWidth = createStructure->cx;
	initialHeight = createStructure->cy;
	return 0;
}

void EoDlgLayerPropertiesManager::OnNmDoubleClickLayerFilterTree(NMHDR* /*notifyStructure*/, LRESULT* result) {
	if (auto h = treeFilters.GetSelectedItem()) {
		const OdLyLayerFilter* lf = static_cast<OdLyLayerFilter*>(reinterpret_cast<void*>(treeFilters.GetItemData(h)));
		if (!lf->dynamicallyGenerated() && !lf->isIdFilter()) {
			//OdaLayerFilterPropDlg(lf, this).DoModal();
		}
	}
	*result = 0;
}

void EoDlgLayerPropertiesManager::OnTvnKeydownLayerFilterTree(NMHDR* notifyStructure, LRESULT* result) {
	const auto pTVKeyDown {reinterpret_cast<tagTVKEYDOWN*>(notifyStructure)};
	if (pTVKeyDown->wVKey == VK_DELETE) {
		if (auto SelectedItem = treeFilters.GetSelectedItem()) {
			const auto Filter {static_cast<OdLyLayerFilter*>(reinterpret_cast<void*>(treeFilters.GetItemData(SelectedItem)))};
			if (Filter->dynamicallyGenerated()) {
				return;
			}
			if (AfxMessageBox(L"Delete this filter?", MB_YESNO) != IDYES) {
				return;
			}
			Filter->parent()->removeNested(Filter);
			treeFilters.DeleteItem(SelectedItem);
			const OdLyLayerFilter* Root = static_cast<OdLyLayerFilter*>(reinterpret_cast<void*>(treeFilters.GetItemData(treeFilters.GetRootItem())));
			odlyGetLayerFilterManager(m_Database)->setFilters(Root, Root);
		}
	}
	*result = 0;
}

BOOL EoDlgLayerPropertiesManager::OnInitDialog() {
	CDialog::OnInitDialog();
	CBitmap Bitmap;
	Bitmap.LoadBitmapW(IDB_LAYER_FILTERS);
	treeImages.Create(16, 16, ILC_COLOR32, 0, 1);
	treeImages.Add(&Bitmap, RGB(0, 0, 0));
	treeFilters.SetImageList(&treeImages, TVSIL_NORMAL);
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
static void UpdateFilterTree(CTreeCtrl& tree, const HTREEITEM parent, const OdLyLayerFilter* root, const OdLyLayerFilter* current) {
	if (root != nullptr) {
		const auto TreeItem {tree.InsertItem(root->name(), parent)};
		tree.SetItemData(TreeItem, reinterpret_cast<unsigned long>((void*)root));
		const auto Image {root->isIdFilter() ? 2 : 1};
		tree.SetItemImage(TreeItem, Image, Image);
		for (const auto& Filter : root->getNestedFilters()) {
			UpdateFilterTree(tree, TreeItem, Filter, current);
		}
		if (current == root) {
			tree.SelectItem(TreeItem);
		}
	}
}

void EoDlgLayerPropertiesManager::UpdateFiltersTree() {
	treeFilters.DeleteAllItems();
	auto FilterManager {odlyGetLayerFilterManager(m_Database)};
	OdLyLayerFilterPtr pCurrent;
	if (FilterManager->getFilters(rootFilter, pCurrent) != eOk) {
		return;
	}
	UpdateFilterTree(treeFilters, TVI_ROOT, rootFilter, pCurrent);
	treeFilters.SetItemImage(treeFilters.GetRootItem(), 0, 0);
}

void EoDlgLayerPropertiesManager::OnSize(const unsigned type, const int newWidth, const int newHeight) {
	CDialog::OnSize(type, newWidth, newHeight);
	CRect itemRect;
	CRect dlgRect;
	if (GetDlgItem(IDC_STATIC_CURRENT_LAYER) != nullptr) {
		GetDlgItem(IDC_STATIC_CURRENT_LAYER)->GetWindowRect(&itemRect);
		ScreenToClient(itemRect);
		GetWindowRect(&dlgRect);
		itemRect.right += dlgRect.Width() - deltaWidth;
		GetDlgItem(IDC_STATIC_CURRENT_LAYER)->MoveWindow(itemRect);
	}
	if (GetDlgItem(IDC_STATIC_LAYER_STATISTIC) != nullptr) {
		GetDlgItem(IDC_STATIC_LAYER_STATISTIC)->GetWindowRect(&itemRect);
		ScreenToClient(itemRect);
		GetWindowRect(&dlgRect);
		GetDlgItem(IDC_STATIC_LAYER_STATISTIC)->MoveWindow(itemRect.left, itemRect.top + (dlgRect.Height() - deltaHeight), itemRect.Width() + (dlgRect.Width() - deltaWidth), itemRect.Height());
	}
	if (GetDlgItem(IDCANCEL) != nullptr) {
		GetDlgItem(IDCANCEL)->GetWindowRect(&itemRect);
		ScreenToClient(itemRect);
		GetWindowRect(&dlgRect);
		GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDCANCEL)->MoveWindow(itemRect.left + (dlgRect.Width() - deltaWidth), itemRect.top + (dlgRect.Height() - deltaHeight), itemRect.Width(), itemRect.Height());
		GetDlgItem(IDCANCEL)->ShowWindow(SW_SHOW);
	}
	if (GetDlgItem(IDOK) != nullptr) {
		GetDlgItem(IDOK)->GetWindowRect(&itemRect);
		ScreenToClient(itemRect);
		GetWindowRect(&dlgRect);
		GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDOK)->MoveWindow(itemRect.left + (dlgRect.Width() - deltaWidth), itemRect.top + (dlgRect.Height() - deltaHeight), itemRect.Width(), itemRect.Height());
		GetDlgItem(IDOK)->ShowWindow(SW_SHOW);
	}
}

void EoDlgLayerPropertiesManager::OnSizing(const unsigned side, const LPRECT rectangle) {
	CDialog::OnSizing(side, rectangle);
	const CRect rct(*rectangle);
	CRect dlgRect;
	GetWindowRect(&dlgRect);
	deltaWidth = dlgRect.Width();
	deltaHeight = dlgRect.Height();
	if (rct.Width() < initialWidth) {
		switch (side) {
			case WMSZ_LEFT: case WMSZ_BOTTOMLEFT: case WMSZ_TOPLEFT:
				rectangle->left = rectangle->right - initialWidth;
				break;
			case WMSZ_RIGHT: case WMSZ_BOTTOMRIGHT: case WMSZ_TOPRIGHT:
				rectangle->right = rectangle->left + initialWidth;
				break;
			default: ;
		}
	}
	if (rct.Height() < initialHeight) {
		switch (side) {
			case WMSZ_BOTTOM: case WMSZ_BOTTOMLEFT: case WMSZ_BOTTOMRIGHT:
				rectangle->bottom = rectangle->top + initialHeight;
				break;
			case WMSZ_TOP: case WMSZ_TOPLEFT: case WMSZ_TOPRIGHT:
				rectangle->top = rectangle->bottom - initialHeight;
				break;
			default: ;
		}
	}
}
