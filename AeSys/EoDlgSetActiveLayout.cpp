#include "stdafx.h"
#include <OdArray.h>
#include <DbDictionary.h>
#include <DbLayout.h>
#include "AeSys.h"
#include "EoDlgSetActiveLayout.h"

EoDlgSetActiveLayout::EoDlgSetActiveLayout(OdDbDatabase* database, CWnd* parent)
	: CDialog(IDD, parent)
	, m_Database(database) {}

void EoDlgSetActiveLayout::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
}

BEGIN_MESSAGE_MAP(EoDlgSetActiveLayout, CDialog)
		ON_COMMAND(IDC_ALAYOUT_CLOSE, &EoDlgSetActiveLayout::OnLayoutDlgClose)
		ON_LBN_SELCHANGE(IDC_LAYOUTLIST, &EoDlgSetActiveLayout::OnSelectionChangeLayoutlist)
		ON_LBN_DBLCLK(IDC_LAYOUTLIST, &EoDlgSetActiveLayout::OnDoubleClickLayoutlist)
		ON_BN_CLICKED(IDC_RENAME, &EoDlgSetActiveLayout::OnRename)
		ON_BN_CLICKED(IDC_DELETE, &EoDlgSetActiveLayout::OnDelete)
		ON_BN_CLICKED(IDC_COPY, &EoDlgSetActiveLayout::OnCopy)
		ON_BN_CLICKED(IDC_NEW, &EoDlgSetActiveLayout::OnNew)
		ON_BN_CLICKED(IDC_FROM_TEMPLATE, &EoDlgSetActiveLayout::OnFromTemplate)
END_MESSAGE_MAP()

BOOL EoDlgSetActiveLayout::OnInitDialog() {
	CDialog::OnInitDialog();
	FillListBox();
	return TRUE;
}

void EoDlgSetActiveLayout::FillListBox() {
	try {
		OdArray<OdString> Items;
		const auto Database {m_Database};
		OdDbDictionaryPtr LayoutDictionary {Database->getLayoutDictionaryId().safeOpenObject()};
		auto LayoutIterator {LayoutDictionary->newIterator()};
		const auto ActiveLayoutBlockTableRecord {Database->getActiveLayoutBTRId()};
		m_OldActiveLayout = -1;
		while (!LayoutIterator->done()) {
			OdSmartPtr<OdDbLayout> Layout {LayoutIterator->objectId().safeOpenObject()};
			const auto ItemIndex {static_cast<unsigned>(Layout->getTabOrder())};
			if (ItemIndex >= Items.size()) {
				Items.resize(ItemIndex + 1);
			}
			Items[ItemIndex] = LayoutIterator->name();
			if (Layout->getBlockTableRecordId() == ActiveLayoutBlockTableRecord) {
				m_OldActiveLayout = static_cast<int>(ItemIndex);
			}
			LayoutIterator->next();
		}
		auto Layouts {static_cast<CListBox*>(GetDlgItem(IDC_LAYOUTLIST))};
		Layouts->ResetContent();
		for (unsigned ItemIndex = 0; ItemIndex < Items.size(); ++ItemIndex) {
			Layouts->InsertString(static_cast<int>(ItemIndex), Items[ItemIndex]);
		}
		Layouts->SetSel(m_OldActiveLayout);
		m_NewActiveLayout = m_OldActiveLayout;
		GetDlgItem(IDC_NEWNAME)->SetWindowTextW(Items[static_cast<unsigned>(m_OldActiveLayout)]);
	} catch (const OdError& Error) {
		theApp.ErrorMessageBox(L"Error Selecting Layout", Error);
	}
}

void EoDlgSetActiveLayout::OnLayoutDlgClose() {
	EndDialog(m_OldActiveLayout != m_NewActiveLayout ? IDOK : IDCANCEL);
}

void EoDlgSetActiveLayout::OnSelectionChangeLayoutlist() {
	const auto Layouts {static_cast<CListBox*>(GetDlgItem(IDC_LAYOUTLIST))};
	m_NewActiveLayout = Layouts->GetCurSel();
	Layouts->GetText(m_NewActiveLayout, m_NewLayoutName);
	/*
	CWnd* pNewName = GetDlgItem(IDC_NEWNAME);
	if (!m_CreateNewLayout)
	{
		GetDlgItem(IDC_RENAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_DELETE)->EnableWindow(TRUE);
		pNewName->EnableWindow(TRUE);
		pNewName->SetWindowTextW(m_NewLayoutName);
	} else {
		GetDlgItem(IDC_RENAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
		pNewName->EnableWindow(FALSE);
		pNewName->SetWindowTextW(L"");
	}
	*/
}

void EoDlgSetActiveLayout::OnDoubleClickLayoutlist() {
	EndDialog(m_OldActiveLayout != m_NewActiveLayout ? IDOK : IDCANCEL);
}

void EoDlgSetActiveLayout::OnRename() {
	auto Layouts {static_cast<CListBox*>(GetDlgItem(IDC_LAYOUTLIST))};
	CString OldName;
	CString NewName;
	Layouts->GetText(m_NewActiveLayout, OldName);
	GetDlgItem(IDC_NEWNAME)->GetWindowText(NewName);
	if (NewName != OldName) {
		try {
			m_Database->renameLayout(OdString(OldName), OdString(NewName));
		} catch (const OdError& Error) {
			theApp.ErrorMessageBox(L"Error Renaming Layout", Error);
			return;
		}
		Layouts->DeleteString(static_cast<unsigned>(m_NewActiveLayout));
		Layouts->InsertString(m_NewActiveLayout, NewName);
		Layouts->SetSel(m_NewActiveLayout);
	}
}

void EoDlgSetActiveLayout::OnDelete() {
	const auto Layouts {static_cast<CListBox*>(GetDlgItem(IDC_LAYOUTLIST))};
	CString CurrentName;
	Layouts->GetText(m_NewActiveLayout, CurrentName);
	try {
		m_Database->startUndoRecord();
		m_Database->deleteLayout(OdString(CurrentName));
	} catch (const OdError& Error) {
		theApp.ErrorMessageBox(L"Error Deleting Layout", Error);
		m_Database->disableUndoRecording(true);
		m_Database->undo();
		m_Database->disableUndoRecording(false);
		return;
	}
	FillListBox();
}

void EoDlgSetActiveLayout::OnCopy() {
	const auto Layouts {static_cast<CListBox*>(GetDlgItem(IDC_LAYOUTLIST))};
	CString SourceName;
	CString NewName;
	Layouts->GetText(m_NewActiveLayout, SourceName);
	GetDlgItem(IDC_NEWNAME)->GetWindowText(NewName);
	const OdString Name {SourceName};
	auto LayoutManager {m_Database->appServices()->layoutManager()};
	try {
		OdSmartPtr<OdDbLayout> Layout {LayoutManager->findLayoutNamed(m_Database, Name).safeOpenObject()};
		LayoutManager->cloneLayout(m_Database, Layout, OdString(NewName));
	} catch (const OdError& Error) {
		theApp.ErrorMessageBox(L"Error Cloning Layout", Error);
		return;
	}
	FillListBox();
}

void EoDlgSetActiveLayout::OnNew() {
	CString LayoutName;
	GetDlgItem(IDC_NEWNAME)->GetWindowText(LayoutName);
	try {
		m_Database->createLayout(OdString(LayoutName));
	} catch (const OdError& Error) {
		theApp.ErrorMessageBox(L"Error Creating Layout", Error);
		return;
	}
	FillListBox();
}

void EoDlgSetActiveLayout::OnFromTemplate() {
	const OdString Filter {L"DWG files (*.dwg)|*.dwg|DXF files (*.dxf)|*.dxf|All Files (*.*)|*.*||"};
	const auto FileName {AeSys::BrowseWithPreview(GetSafeHwnd(), Filter)};
	if (FileName.GetLength() == 0) {
		return;
	}
	auto Database {theApp.readFile(OdString(FileName))};
	if (Database.isNull()) {
		return;
	}
	auto LayoutManager {m_Database->appServices()->layoutManager()};
	auto Layout {OdDbLayout::cast(LayoutManager->findLayoutNamed(Database, L"Layout1").openObject())};
	if (Layout.isNull()) {
		return;
	}
	CString NewName;
	GetDlgItem(IDC_NEWNAME)->GetWindowText(NewName);
	try {
		LayoutManager->cloneLayout(m_Database, Layout, OdString(NewName));
	} catch (const OdError& Error) {
		theApp.ErrorMessageBox(L"Error Cloning Layout", Error);
		return;
	}
	FillListBox();
}
