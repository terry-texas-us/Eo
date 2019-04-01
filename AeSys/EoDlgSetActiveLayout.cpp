#include "stdafx.h"

#include "AeSysApp.h"
#include "EoDlgSetActiveLayout.h"
#include "OdArray.h"
#include "DbDictionary.h"
#include "DbLayout.h"

EoDlgSetActiveLayout::EoDlgSetActiveLayout(OdDbDatabase* database, CWnd* parent)
	: CDialog(EoDlgSetActiveLayout::IDD, parent)
    , m_pDb(database)
    , m_nOldActiveLayout(0)
    , m_bCreateNewLayout(false)
    , m_nNewActiveLayout(0) {
}

void EoDlgSetActiveLayout::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(EoDlgSetActiveLayout, CDialog)
	ON_COMMAND(IDC_ALAYOUT_CLOSE, OnLayoutDlgClose)
	ON_LBN_SELCHANGE(IDC_LAYOUTLIST, OnSelchangeLayoutlist)
	ON_LBN_DBLCLK(IDC_LAYOUTLIST, OnDblclkLayoutlist)
	ON_BN_CLICKED(IDC_RENAME, OnRename)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	ON_BN_CLICKED(IDC_FROM_TEMPLATE, OnFromTemplate)
END_MESSAGE_MAP()

BOOL EoDlgSetActiveLayout::OnInitDialog() {
	CDialog::OnInitDialog();
	FillListBox();  
	return TRUE;
}
void EoDlgSetActiveLayout::FillListBox() {
	try {
		UINT n;
		OdArray<OdString> items;
		const OdDbDatabase* Database = m_pDb;
		OdDbDictionaryPtr LayoutDictionary = Database->getLayoutDictionaryId().safeOpenObject();
		OdDbDictionaryIteratorPtr LayoutIterator = LayoutDictionary->newIterator();
		const OdDbObjectId ActiveLayoutBlockTableRecord = Database->getActiveLayoutBTRId();

		m_nOldActiveLayout = - 1;
		while (!LayoutIterator->done()) {
			OdDbLayoutPtr Layout = LayoutIterator->objectId().safeOpenObject();
			n = Layout->getTabOrder();
			if (n >= items.size()) {
				items.resize(n + 1);
			}
			items[n] = LayoutIterator->name();
			if (Layout->getBlockTableRecordId() == ActiveLayoutBlockTableRecord) {
				m_nOldActiveLayout = n;
			}
			LayoutIterator->next();
		}
		CListBox* pLayouts = (CListBox*) GetDlgItem(IDC_LAYOUTLIST);
		pLayouts->ResetContent();
		for (n = 0; n < items.size(); ++n) {
			pLayouts->InsertString(n, items[n]);
		}
		pLayouts->SetSel(m_nOldActiveLayout);
		m_nNewActiveLayout = m_nOldActiveLayout;

		GetDlgItem(IDC_NEWNAME)->SetWindowText(items[m_nOldActiveLayout]);
	}
	catch (const OdError& Error) {
		theApp.reportError(L"Error Selecting Layout", Error);
	}
}
void EoDlgSetActiveLayout::OnLayoutDlgClose() {
	EndDialog(m_nOldActiveLayout != m_nNewActiveLayout ? IDOK : IDCANCEL);
}
void EoDlgSetActiveLayout::OnSelchangeLayoutlist() {
	const CListBox* pLayouts = (CListBox*) GetDlgItem(IDC_LAYOUTLIST);
	m_nNewActiveLayout = pLayouts->GetCurSel();
	pLayouts->GetText(m_nNewActiveLayout, m_sNewLayoutName);
	/*
	CWnd* pNewName = GetDlgItem(IDC_NEWNAME);
	if (!m_bCreateNewLayout)
	{
		GetDlgItem(IDC_RENAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_DELETE)->EnableWindow(TRUE);
		pNewName->EnableWindow(TRUE);
		pNewName->SetWindowText(m_sNewLayoutName);
	}
	else {
		GetDlgItem(IDC_RENAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
		pNewName->EnableWindow(FALSE);
		pNewName->SetWindowText(L"");
	}
	*/
}

void EoDlgSetActiveLayout::OnDblclkLayoutlist() {
	EndDialog(m_nOldActiveLayout != m_nNewActiveLayout ? IDOK : IDCANCEL);
}
void EoDlgSetActiveLayout::OnRename() {
	CListBox* pLayouts = (CListBox*) GetDlgItem(IDC_LAYOUTLIST);
	CString oldName;
	CString newName;
	pLayouts->GetText(m_nNewActiveLayout, oldName);
	GetDlgItem(IDC_NEWNAME)->GetWindowText(newName);
	if (newName != oldName) {
		try {
			m_pDb->renameLayout(OdString(oldName), OdString(newName));
		}
		catch (const OdError& Error) {
			theApp.reportError(L"Error Renaming Layout", Error);
			return;
		}
		pLayouts->DeleteString(m_nNewActiveLayout);
		pLayouts->InsertString(m_nNewActiveLayout, newName);
		pLayouts->SetSel(m_nNewActiveLayout);
	}
}
void EoDlgSetActiveLayout::OnDelete() {
	const CListBox* pLayouts = (CListBox*) GetDlgItem(IDC_LAYOUTLIST);
	CString currName;
	pLayouts->GetText(m_nNewActiveLayout, currName);
	try {
		m_pDb->startUndoRecord();
		m_pDb->deleteLayout(OdString(currName));
	}
	catch (const OdError& Error) {
		theApp.reportError(L"Error Deleting Layout", Error);
		m_pDb->disableUndoRecording(true);
		m_pDb->undo();
		m_pDb->disableUndoRecording(false);
		return;
	}
	FillListBox();
}
void EoDlgSetActiveLayout::OnCopy() {
	const CListBox* pLayouts = (CListBox*) GetDlgItem(IDC_LAYOUTLIST);
	CString strSourceName, strNewName;
	pLayouts->GetText(m_nNewActiveLayout, strSourceName);
	GetDlgItem(IDC_NEWNAME)->GetWindowText(strNewName);
	OdString strName(strSourceName);
	OdDbLayoutManagerPtr pLManager = m_pDb->appServices()->layoutManager();
	try {
		OdDbLayoutPtr pLayout = pLManager->findLayoutNamed(m_pDb, strName).safeOpenObject();
		pLManager->cloneLayout(m_pDb, pLayout, OdString(strNewName));
	}
	catch (const OdError& Error) {
		theApp.reportError(L"Error Cloning Layout", Error);
		return;
	}
	FillListBox();
}
void EoDlgSetActiveLayout::OnNew() {
	CString strNewName;
	GetDlgItem(IDC_NEWNAME)->GetWindowText(strNewName);

	try {
		m_pDb->createLayout(OdString(strNewName));
	}
	catch (const OdError& Error) {
		theApp.reportError(L"Error Creating Layout", Error);
		return;
	}
	FillListBox();
}
void EoDlgSetActiveLayout::OnFromTemplate() {
	CString Filter(L"DWG files (*.dwg)|*.dwg|DXF files (*.dxf)|*.dxf|All Files (*.*)|*.*||");
	CString FileName = theApp.BrowseWithPreview(/*GetMainWnd()->*/GetSafeHwnd(), Filter);
	if(FileName.GetLength() == 0)
		return;

	OdDbDatabasePtr Database = theApp.readFile(OdString(FileName));
	if (Database.isNull()) {
		return;
	}
	OdDbLayoutManagerPtr pLManager = m_pDb->appServices()->layoutManager();
	OdDbLayoutPtr pLayout = OdDbLayout::cast(pLManager->findLayoutNamed(Database, L"Layout1").openObject());
	if (pLayout.isNull()) {
		return;
	}
	CString strNewName;
	GetDlgItem(IDC_NEWNAME)->GetWindowText(strNewName);
	try {
		pLManager->cloneLayout(m_pDb, pLayout, OdString(strNewName));
	}
	catch (const OdError& Error) {
		theApp.reportError(L"Error Cloning Layout", Error);
		return;
	}
	FillListBox();
}
