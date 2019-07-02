#include "stdafx.h"
#include "MainFrm.h"
#include "AeSysDoc.h"
#include "EoDlgEditTrapCommandsQuery.h"
IMPLEMENT_DYNAMIC(EoDlgEditTrapCommandsQuery, CDialog)

BEGIN_MESSAGE_MAP(EoDlgEditTrapCommandsQuery, CDialog)
		ON_NOTIFY(TVN_SELCHANGED, IDC_GROUP_TREE, &EoDlgEditTrapCommandsQuery::OnSelectionChangedGroupTree)
END_MESSAGE_MAP()

EoDlgEditTrapCommandsQuery::EoDlgEditTrapCommandsQuery(CWnd* parent)
	: CDialog(IDD, parent) {
}

EoDlgEditTrapCommandsQuery::~EoDlgEditTrapCommandsQuery() = default;

void EoDlgEditTrapCommandsQuery::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_GROUP_TREE, m_GroupTreeViewControl);
	DDX_Control(dataExchange, IDC_GEOMETRY_LIST, m_GeometryListViewControl);
	DDX_Control(dataExchange, IDC_EXTRA_LIST_CTRL, m_ExtraListViewControl);
}

BOOL EoDlgEditTrapCommandsQuery::OnInitDialog() {
	CDialog::OnInitDialog();
	const auto GroupTreeWindowHandle {::GetDlgItem(this->GetSafeHwnd(), IDC_GROUP_TREE)};
	auto GroupsInTrap {AeSysDoc::GetDoc()->GroupsInTrap()};
	const auto GroupListTreeItemHandle {CMainFrame::InsertTreeViewControlItem(GroupTreeWindowHandle, TVI_ROOT, L"<Groups>", GroupsInTrap)};
	GroupsInTrap->AddToTreeViewControl(GroupTreeWindowHandle, GroupListTreeItemHandle);
	m_ExtraListViewControl.InsertColumn(0, L"Property", LVCFMT_LEFT, 128);
	m_ExtraListViewControl.InsertColumn(1, L"Value", LVCFMT_LEFT, 192);
	m_GeometryListViewControl.InsertColumn(0, L"Property", LVCFMT_LEFT, 128);
	m_GeometryListViewControl.InsertColumn(1, L"X-Axis", LVCFMT_LEFT, 96);
	m_GeometryListViewControl.InsertColumn(2, L"Y-Axis", LVCFMT_LEFT, 96);
	m_GeometryListViewControl.InsertColumn(3, L"Z-Axis", LVCFMT_LEFT, 96);
	TreeView_Expand(GroupTreeWindowHandle, GroupListTreeItemHandle, TVE_EXPAND);
	return TRUE;
}

void EoDlgEditTrapCommandsQuery::OnSelectionChangedGroupTree(NMHDR* notifyStructure, LRESULT* result) {
	const auto TreeViewNotificationMessage {reinterpret_cast<tagNMTREEVIEWW*>(notifyStructure)};
	wchar_t Text[256] {L""};
	TV_ITEMW Item;
	::ZeroMemory(&Item, sizeof Item);
	Item.hItem = TreeViewNotificationMessage->itemNew.hItem;
	Item.mask = TVIF_TEXT | TVIF_PARAM;
	Item.pszText = Text;
	Item.cchTextMax = sizeof Text / sizeof(wchar_t);
	m_GroupTreeViewControl.GetItem(&Item);
	m_ExtraListViewControl.DeleteAllItems();
	m_GeometryListViewControl.DeleteAllItems();
	if (wcscmp(Item.pszText, L"<Groups>") == 0) {
	} else if (wcscmp(Item.pszText, L"<Group>") == 0) {
	} else {
		const auto Primitive {reinterpret_cast<EoDbPrimitive*>(Item.lParam)};
		FillExtraList(Primitive);
		FillGeometryList(Primitive);
	}
	*result = 0;
}

void EoDlgEditTrapCommandsQuery::FillExtraList(EoDbPrimitive* primitive) {
	wchar_t Token[64] {L""};
	auto Item {0};
	CString Extra;
	primitive->FormatExtra(Extra);
	auto Offset {0};
	for (auto Delimiter = Extra.Mid(Offset).Find(';'); Delimiter != -1;) {
		wcscpy_s(Token, Extra.Mid(Offset, Delimiter));
		m_ExtraListViewControl.InsertItem(Item, Token);
		Offset += Delimiter + 1;
		Delimiter = Extra.Mid(Offset).Find('\t');
		const auto Length {min(Delimiter, static_cast<int>(sizeof(Token) / sizeof(wchar_t) - 1))};
		wcscpy_s(Token, 64, Extra.Mid(Offset, Length));
		m_ExtraListViewControl.SetItemText(Item++, 1, Token);
		Offset += Delimiter + 1;
		Delimiter = Extra.Mid(Offset).Find(';');
	}
}

void EoDlgEditTrapCommandsQuery::FillGeometryList(EoDbPrimitive* primitive) {
	wchar_t Token[64] {L""};
	auto Item {0};
	CString Geometry;
	primitive->FormatGeometry(Geometry);
	auto Offset {0};
	for (auto Delimiter = Geometry.Mid(Offset).Find(';'); Delimiter != -1;) {
		wcscpy_s(Token, 64, Geometry.Mid(Offset, Delimiter));
		m_GeometryListViewControl.InsertItem(Item, Token);
		Offset += Delimiter + 1;
		Delimiter = Geometry.Mid(Offset).Find(';');
		wcscpy_s(Token, 64, Geometry.Mid(Offset, Delimiter));
		m_GeometryListViewControl.SetItemText(Item, 1, Token);
		Offset += Delimiter + 1;
		Delimiter = Geometry.Mid(Offset).Find(';');
		wcscpy_s(Token, 64, Geometry.Mid(Offset, Delimiter));
		m_GeometryListViewControl.SetItemText(Item, 2, Token);
		Offset += Delimiter + 1;
		Delimiter = Geometry.Mid(Offset).Find('\t');
		wcscpy_s(Token, 64, Geometry.Mid(Offset, Delimiter));
		m_GeometryListViewControl.SetItemText(Item++, 3, Token);
		Offset += Delimiter + 1;
		Delimiter = Geometry.Mid(Offset).Find(';');
	}
}
