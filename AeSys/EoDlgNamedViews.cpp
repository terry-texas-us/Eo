#include "stdafx.h"
#include <DbLayout.h>
#include <DbViewTable.h>
#include <DbViewTableRecord.h>
#include <DbLayerState.h>
#include <DbUCSTableRecord.h>
#include "AeSys.h"
#include "AeSysDoc.h"
#include "EoDlgNamedViews.h"
#include "EoDlgNewView.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
OdDbObjectId CNamedViewListCtrl::viewId(const int item) const {
	return OdDbObjectId(reinterpret_cast<OdDbStub*>(GetItemData(item)));
}

void CNamedViewListCtrl::setViewId(const int item, const OdDbObjectId& id) {
	SetItemData(item, reinterpret_cast<unsigned long>(static_cast<OdDbStub*>(id)));
}

OdDbViewTableRecordPtr CNamedViewListCtrl::view(const int item) {
	return viewId(item).safeOpenObject(OdDb::kForWrite);
}

void CNamedViewListCtrl::setView(const int item, const OdDbViewTableRecord* view) {
	setViewId(item, view->objectId());
}

OdString ucsString(const OdDbObject* viewport) {
	OdString Result;
	OdDbAbstractViewportDataPtr AbstractViewportData(viewport);
	switch (AbstractViewportData->orthoUcs(viewport)) {
		case OdDb::kTopView:
			Result = L"Top";
			break;
		case OdDb::kBottomView:
			Result = L"Bottom";
			break;
		case OdDb::kFrontView:
			Result = L"Front";
			break;
		case OdDb::kBackView:
			Result = L"Back";
			break;
		case OdDb::kLeftView:
			Result = L"Left";
			break;
		case OdDb::kRightView:
			Result = L"Right";
			break;
		case OdDb::kNonOrthoView: default: {
			OdDbUCSTableRecordPtr pUCS {OdDbObjectId(AbstractViewportData->ucsName(viewport)).openObject()};
			if (pUCS.get()) {
				Result = pUCS->getName();
			} else {
				OdGePoint3d Origin;
				OdGeVector3d XAxis;
				OdGeVector3d YAxis;
				AbstractViewportData->getUcs(viewport, Origin, XAxis, YAxis);
				if (Origin == OdGePoint3d::kOrigin && XAxis == OdGeVector3d::kXAxis && YAxis == OdGeVector3d::kYAxis) {
					Result = L"World";
				} else {
					Result = L"Unnamed";
				}
			}
		}
	}
	return Result;
}

void CNamedViewListCtrl::InsertItem(const int i, const OdDbViewTableRecord* pView) {
	CListCtrl::InsertItem(i, pView->getName());
	setView(i, pView);
	SetItemText(i, 1, pView->getCategoryName());
	SetItemText(i, 2, pView->isPaperspaceView() ? L"Layout" : L"Model");
	SetItemText(i, 3, pView->isViewAssociatedToViewport() ? L"True" : L"");
	SetItemText(i, 4, pView->getLayerState().isEmpty() ? L"" : L"Saved");
	if (pView->isUcsAssociatedToView()) {
		SetItemText(i, 5, ucsString(pView));
	}
	SetItemText(i, 6, pView->perspectiveEnabled() ? L"On" : L"Off");
}

OdDbViewTableRecordPtr CNamedViewListCtrl::selectedView() {
	const auto nSelectionMark {GetSelectionMark()};
	if (nSelectionMark > - 1) {
		return view(nSelectionMark);
	}
	return OdDbViewTableRecordPtr();
}

EoDlgNamedViews::EoDlgNamedViews(AeSysDoc* pDoc, CWnd* parent)
	: CDialog(IDD, parent) {
	m_pDoc = pDoc;
}

OdDbDatabase* EoDlgNamedViews::database() {
	return document()->m_DatabasePtr;
}

void EoDlgNamedViews::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_NAMEDVIEWS, m_views);
}

BEGIN_MESSAGE_MAP(EoDlgNamedViews, CDialog)
		ON_BN_CLICKED(IDC_SETCURRENT_BUTTON, OnSetcurrentButton)
		ON_NOTIFY(NM_DBLCLK, IDC_NAMEDVIEWS, OnDblclkNamedviews)
		ON_BN_CLICKED(IDC_NEW_BUTTON, OnNewButton)
		ON_BN_CLICKED(IDC_UPDATE_LAYERS_BUTTON, OnUpdateLayersButton)
		ON_BN_CLICKED(IDC_DELETE_BUTTON, OnDeleteButton)
END_MESSAGE_MAP()

BOOL EoDlgNamedViews::OnInitDialog() {
	CDialog::OnInitDialog();
	m_views.InsertColumn(0, L"Name", LVCFMT_LEFT, 100);
	m_views.InsertColumn(1, L"Category", LVCFMT_LEFT, 60);
	m_views.InsertColumn(2, L"Location", LVCFMT_LEFT, 50);
	m_views.InsertColumn(3, L"VP", LVCFMT_LEFT, 40);
	m_views.InsertColumn(4, L"Layers", LVCFMT_LEFT, 50);
	m_views.InsertColumn(5, L"UCS", LVCFMT_LEFT, 60);
	m_views.InsertColumn(6, L"Perspective", LVCFMT_LEFT, 30);
	try {
		const OdDbDatabase* Database = m_pDoc->m_DatabasePtr;
		OdDbViewTablePtr ViewTable = Database->getViewTableId().safeOpenObject();
		auto Index {0};
		for (auto ViewTableIterator = ViewTable->newIterator(); !ViewTableIterator->done(); ViewTableIterator->step()) {
			OdDbViewTableRecordPtr ViewTableRecord = ViewTableIterator->getRecordId().openObject();
			m_views.InsertItem(Index++, ViewTableRecord);
		}
	} catch (const OdError& Error) {
		theApp.reportError(L"Error creating Named Views dialog", Error);
		EndDialog(IDCANCEL);
		return FALSE;
	}
	return TRUE;
}

void EoDlgNamedViews::OnSetcurrentButton() {
	auto NamedView {m_views.selectedView()};
	if (NamedView.get()) {
		OdDbDatabase* pDb = m_pDoc->m_DatabasePtr;
		auto ActiveViewportObject {pDb->activeViewportId().safeOpenObject(OdDb::kForWrite)};
		OdDbAbstractViewportDataPtr pVpPE(ActiveViewportObject);
		pVpPE->setView(ActiveViewportObject, NamedView);
		pVpPE->setUcs(ActiveViewportObject, NamedView);
		pVpPE->setProps(ActiveViewportObject, NamedView);
		const auto sLSName {NamedView->getLayerState()};
		if (!sLSName.isEmpty()) {
			OdDbLayerState::restore(pDb, sLSName, OdDbLayerState::kUndefDoNothing, OdDbLayerState::kOn | OdDbLayerState::kFrozen);
		}
	}
}

void EoDlgNamedViews::OnDblclkNamedviews(NMHDR* notifyStructure, LRESULT* /*pResult*/) {
	OnSetcurrentButton();
}

void deleteLayerState(OdDbViewTableRecord* pNamedView) {
	const auto sLSName {pNamedView->getLayerState()};
	if (!sLSName.isEmpty()) {
		OdDbLayerState::remove(pNamedView->database(), sLSName);
		pNamedView->setLayerState(L"");
	}
}

void updateLayerState(OdDbViewTableRecord* pNamedView) {
	auto sLSName {pNamedView->getLayerState()};
	const auto pDb {pNamedView->database()};
	if (sLSName.isEmpty()) {
		OdString name;
		name.format(L"ACAD_VIEWS_%s", pNamedView->getName().c_str());
		sLSName = name;
		auto i {1};
		while (OdDbLayerState::has(pDb, sLSName)) {
			sLSName.format(L"%s(%d)", name.c_str(), ++i);
		}
		pNamedView->setLayerState(sLSName);
	}
	OdDbLayerState::save(pDb, sLSName, OdDbLayerState::kHidden | OdDbLayerState::kCurrentViewport);
}

void EoDlgNamedViews::OnNewButton() {
	EoDlgNewView newDlg(this);
	OdDbViewTableRecordPtr pNamedView;
	const OdDbDatabase* pDb = m_pDoc->m_DatabasePtr;
	while (newDlg.DoModal() == IDOK) {
		LVFINDINFO lvfi = {LVFI_STRING, newDlg.m_sViewName, 0, {0, 0}, 0};
		auto i {m_views.FindItem(&lvfi)};
		if (i >= 0) {
			if (AfxMessageBox(newDlg.m_sViewName + L" already exists.\nDo you want to replace it?", MB_YESNOCANCEL) != IDYES) continue;
			pNamedView = m_views.view(i);
			m_views.DeleteItem(i);
		} else {
			OdDbViewTablePtr pViewTable = pDb->getViewTableId().safeOpenObject(OdDb::kForWrite);
			pNamedView = OdDbViewTableRecord::createObject();
			pNamedView->setName(OdString(newDlg.m_sViewName));
			pViewTable->add(pNamedView);
			i = m_views.GetItemCount();
		}
		auto ActiveViewportObject {pDb->activeViewportId().safeOpenObject()};
		OdDbAbstractViewportDataPtr pViewPE(pNamedView);
		pViewPE->setView(pNamedView, ActiveViewportObject);
		if (newDlg.m_bSaveUCS) {
			if (newDlg.m_sUcsName == L"Unnamed") pViewPE->setUcs(pNamedView, ActiveViewportObject);
			else if (newDlg.m_sUcsName == L"World") pNamedView->setUcsToWorld();
			else pNamedView->setUcs(OdDbSymUtil::getUCSId(OdString(newDlg.m_sUcsName), pDb));
		} else pNamedView->disassociateUcsFromView();
		pViewPE->setProps(pNamedView, ActiveViewportObject);
		pNamedView->setCategoryName(OdString(newDlg.m_sViewCategory));
		if (newDlg.m_bStoreLS) updateLayerState(pNamedView);
		else deleteLayerState(pNamedView);
		m_views.InsertItem(i, pNamedView);
		break;
	}
}

void EoDlgNamedViews::OnUpdateLayersButton() {
	const auto SelectionMark {m_views.GetSelectionMark()};
	if (SelectionMark > - 1) {
		updateLayerState(m_views.selectedView());
		m_views.SetItemText(SelectionMark, 4, L"Saved");
	}
}

void EoDlgNamedViews::OnDeleteButton() {
	const auto SelectionMark {m_views.GetSelectionMark()};
	if (SelectionMark > - 1) {
		m_views.view(SelectionMark)->erase();
		m_views.DeleteItem(SelectionMark);
	}
}
