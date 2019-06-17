#include "stdafx.h"
#include "MainFrm.h"
#include "AeSysDoc.h"
#include "EoDlgEditTrapCommandsQuery.h"

// EoDlgEditTrapCommandsQuery dialog

IMPLEMENT_DYNAMIC(EoDlgEditTrapCommandsQuery, CDialog)

BEGIN_MESSAGE_MAP(EoDlgEditTrapCommandsQuery, CDialog)
	ON_NOTIFY(TVN_SELCHANGED, IDC_GROUP_TREE, &EoDlgEditTrapCommandsQuery::OnTvnSelchangedGroupTree)
END_MESSAGE_MAP()

EoDlgEditTrapCommandsQuery::EoDlgEditTrapCommandsQuery(CWnd* parent)
	: CDialog(EoDlgEditTrapCommandsQuery::IDD, parent) {
}

EoDlgEditTrapCommandsQuery::~EoDlgEditTrapCommandsQuery() {
}

void EoDlgEditTrapCommandsQuery::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GROUP_TREE, m_GroupTreeViewControl);
	DDX_Control(pDX, IDC_GEOMETRY_LIST, m_GeometryListViewControl);
	DDX_Control(pDX, IDC_EXTRA_LIST_CTRL, m_ExtraListViewControl);
}

BOOL EoDlgEditTrapCommandsQuery::OnInitDialog() {
	CDialog::OnInitDialog();

	auto GroupTreeWindowHandle {::GetDlgItem(this->GetSafeHwnd(), IDC_GROUP_TREE)};
	auto GroupsInTrap {AeSysDoc::GetDoc()->GroupsInTrap()};
	auto GroupListTreeItemHandle {CMainFrame::InsertTreeViewControlItem(GroupTreeWindowHandle, TVI_ROOT, L"<Groups>", GroupsInTrap)};

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

void EoDlgEditTrapCommandsQuery::OnTvnSelchangedGroupTree(NMHDR* notifyStructure, LRESULT* result) {
	LPNMTREEVIEWW pNMTreeView {reinterpret_cast<LPNMTREEVIEWW>(notifyStructure)};

	wchar_t szText[256] {L"\0"};

	TV_ITEMW item;
	::ZeroMemory(&item, sizeof(item));
	item.hItem = pNMTreeView->itemNew.hItem;
	item.mask = TVIF_TEXT | TVIF_PARAM;
	item.pszText = szText;
	item.cchTextMax = sizeof(szText) / sizeof(wchar_t);

	m_GroupTreeViewControl.GetItem(&item);
	m_ExtraListViewControl.DeleteAllItems();
	m_GeometryListViewControl.DeleteAllItems();

	if (wcscmp(item.pszText, L"<Groups>") == 0) {
	} else if (wcscmp(item.pszText, L"<Group>") == 0) {
	} else {
		auto Primitive {( EoDbPrimitive*) item.lParam};
		FillExtraList(Primitive);
		FillGeometryList(Primitive);
	}
	*result = 0;
}

void EoDlgEditTrapCommandsQuery::FillExtraList(EoDbPrimitive* primitive) {
	wchar_t szBuf[64] {L""};

	int iItem {0};

	CString Extra;
	primitive->FormatExtra(Extra);

	unsigned nOff {0};
	for (auto nDel = Extra.Mid(nOff).Find(';'); nDel != -1;) {
		wcscpy_s(szBuf, Extra.Mid(nOff, nDel));

		m_ExtraListViewControl.InsertItem(iItem, szBuf);

		nOff += nDel + 1;
		nDel = Extra.Mid(nOff).Find('\t');
		auto nLen {static_cast<int>(min(nDel, sizeof(szBuf) / sizeof(wchar_t) - 1))};
		wcscpy_s(szBuf, 64, Extra.Mid(nOff, nLen));

		m_ExtraListViewControl.SetItemText(iItem++, 1, szBuf);

		nOff += nDel + 1;
		nDel = Extra.Mid(nOff).Find(';');
	}
}

void EoDlgEditTrapCommandsQuery::FillGeometryList(EoDbPrimitive * primitive) {
	wchar_t szBuf[64] {L""};
	int iItem {0};

	CString strBuf;
	primitive->FormatGeometry(strBuf);

	unsigned nOff {0};
	for (auto nDel = strBuf.Mid(nOff).Find(';'); nDel != -1;) {
		wcscpy_s(szBuf, 64, strBuf.Mid(nOff, nDel));
		m_GeometryListViewControl.InsertItem(iItem, szBuf);
		nOff += nDel + 1;
		nDel = strBuf.Mid(nOff).Find(';');
		wcscpy_s(szBuf, 64, strBuf.Mid(nOff, nDel));
		m_GeometryListViewControl.SetItemText(iItem, 1, szBuf);
		nOff += nDel + 1;
		nDel = strBuf.Mid(nOff).Find(';');
		wcscpy_s(szBuf, 64, strBuf.Mid(nOff, nDel));
		m_GeometryListViewControl.SetItemText(iItem, 2, szBuf);
		nOff += nDel + 1;
		nDel = strBuf.Mid(nOff).Find('\t');
		wcscpy_s(szBuf, 64, strBuf.Mid(nOff, nDel));
		m_GeometryListViewControl.SetItemText(iItem++, 3, szBuf);
		nOff += nDel + 1;
		nDel = strBuf.Mid(nOff).Find(';');
	}
}
