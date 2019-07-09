#include "stdafx.h"
#include "AeSys.h"
#include "EoDlgNewView.h"
#include "EoDlgNamedViews.h"
#include <DbSymbolTable.h>
#include <DbViewTableRecord.h>
#include <DbUCSTableRecord.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
EoDlgNewView::EoDlgNewView(CWnd* parent)
	: CDialog(IDD, parent) {
	m_sViewName = L"";
	m_sViewCategory = L"";
	m_bStoreLS = TRUE;
	m_bSaveUCS = TRUE;
	m_sUcsName = L"";
}

void EoDlgNewView::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_COMBO_UCSNAME, m_UCSs);
	DDX_Control(dataExchange, IDC_COMBO_VIEWCATEGORY, m_categories);
	DDX_Text(dataExchange, IDC_EDIT_VIEWNAME, m_sViewName);
	DDX_CBString(dataExchange, IDC_COMBO_VIEWCATEGORY, m_sViewCategory);
	DDX_Check(dataExchange, IDC_CHECK_STORE_LS, m_bStoreLS);
	DDX_Check(dataExchange, IDC_CHECK_SAVEUCS, m_bSaveUCS);
	DDX_CBString(dataExchange, IDC_COMBO_UCSNAME, m_sUcsName);
}

BEGIN_MESSAGE_MAP(EoDlgNewView, CDialog)
END_MESSAGE_MAP()

OdString UcsString(const OdDbObject* pViewObj);

BOOL EoDlgNewView::OnInitDialog() {
	CDialog::OnInitDialog();
	auto Parent {dynamic_cast<EoDlgNamedViews*>(GetParent())};
	const auto Database {Parent->database()};
	OdDbSymbolTablePtr ViewTable {Database->getViewTableId().safeOpenObject()};
	OdDbSymbolTableIteratorPtr ViewTableIterator;
	for (ViewTableIterator = ViewTable->newIterator(); !ViewTableIterator->done(); ViewTableIterator->step()) {
		OdDbViewTableRecordPtr ViewTableRecord = ViewTableIterator->getRecordId().openObject();
		auto CategoryName {ViewTableRecord->getCategoryName()};
		if (!CategoryName.isEmpty()) {
			if (m_categories.FindString(-1, CategoryName) == -1) {
				m_categories.AddString(CategoryName);
			}
		}
	}
	ViewTable = Database->getUCSTableId().safeOpenObject();
	m_UCSs.AddString(L"World");
	m_sUcsName = static_cast<const wchar_t*>(UcsString(Database->activeViewportId().safeOpenObject()));
	if (m_sUcsName == L"Unnamed") {
		m_UCSs.AddString(m_sUcsName);
	}
	for (ViewTableIterator = ViewTable->newIterator(); !ViewTableIterator->done(); ViewTableIterator->step()) {
		OdDbUCSTableRecordPtr pUCS = ViewTableIterator->getRecordId().openObject();
		m_UCSs.AddString(pUCS->getName());
	}
	UpdateData(FALSE);
	return TRUE;
}
