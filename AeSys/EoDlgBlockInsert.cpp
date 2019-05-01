#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "Preview.h"

#include "EoDb.h"
#include "EoDbBlockReference.h"
#include "EoDlgBlockInsert.h"

// EoDlgBlockInsert dialog

IMPLEMENT_DYNAMIC(EoDlgBlockInsert, CDialog)

BEGIN_MESSAGE_MAP(EoDlgBlockInsert, CDialog)
	ON_LBN_SELCHANGE(IDC_BLOCKS_LIST, &EoDlgBlockInsert::OnLbnSelchangeBlocksList)
	ON_BN_CLICKED(IDC_PURGE, &EoDlgBlockInsert::OnBnClickedPurge)
    ON_BN_CLICKED(IDCANCEL, &EoDlgBlockInsert::OnBnClickedCancel)
END_MESSAGE_MAP()

OdGePoint3d EoDlgBlockInsert::InsertionPoint;

EoDlgBlockInsert::EoDlgBlockInsert(CWnd* parent) 
    : CDialog(EoDlgBlockInsert::IDD, parent)
    , m_Document(nullptr) {
}

EoDlgBlockInsert::EoDlgBlockInsert(AeSysDoc* document, CWnd* parent) 
    : CDialog(EoDlgBlockInsert::IDD, parent)
    , m_Document(document) {
}

EoDlgBlockInsert::~EoDlgBlockInsert() {
}

void EoDlgBlockInsert::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BLOCKS_LIST, m_BlocksListBoxControl);
    
    DDX_Control(pDX, IDC_INSERTION_POINT_ONSCREEN, m_InsertionPointOnscreen);
    DDX_Control(pDX, IDC_INSERTION_POINT_X, m_InsertionPointX);
    DDX_Control(pDX, IDC_INSERTION_POINT_Y, m_InsertionPointY);
    DDX_Control(pDX, IDC_INSERTION_POINT_Z, m_InsertionPointZ);
    DDX_Control(pDX, IDC_SCALE_ONSCREEN, m_ScaleOnscreen);
    DDX_Control(pDX, IDC_SCALE_X, m_ScaleX);
    DDX_Control(pDX, IDC_SCALE_Y, m_ScaleY);
    DDX_Control(pDX, IDC_SCALE_Z, m_ScaleZ);
    DDX_Control(pDX, IDC_ROTATION_ONSCREEN, m_RotationOnscreen);
    DDX_Control(pDX, IDC_ROTATION_ANGLE, m_RotationAngle);
    DDX_Control(pDX, IDC_EXPLODE, m_Explode);
}

BOOL EoDlgBlockInsert::OnInitDialog() {
	CDialog::OnInitDialog();

	InsertionPoint = theApp.GetCursorPosition();

	CString BlockName;
	EoDbBlock* Block;

	POSITION BlockPosition = m_Document->GetFirstBlockPosition();
	while (BlockPosition != NULL) {
		m_Document->GetNextBlock(BlockPosition, BlockName, Block);
		if (!Block->IsAnonymous()) {
			m_BlocksListBoxControl.AddString(BlockName);
		}
	}
	m_BlocksListBoxControl.SetCurSel(0);

	if (m_Document->BlockTableIsEmpty()) {
		WndProcPreviewClear(GetDlgItem(IDC_LAYER_PREVIEW)->GetSafeHwnd());
	}
	else {
		BlockPosition = m_Document->GetFirstBlockPosition();
		m_Document->GetNextBlock(BlockPosition, BlockName, Block);
		SetDlgItemInt(IDC_GROUPS, (UINT) Block->GetCount(), FALSE);
		SetDlgItemInt(IDC_REFERENCES, m_Document->GetBlockReferenceCount(BlockName), FALSE);
		WndProcPreviewUpdate(GetDlgItem(IDC_LAYER_PREVIEW)->GetSafeHwnd(), Block);
	}
	return TRUE;
}

void EoDlgBlockInsert::OnOK() {
	const int CurrentSelection = m_BlocksListBoxControl.GetCurSel();

	if (CurrentSelection != LB_ERR) {
		CString BlockName;
		m_BlocksListBoxControl.GetText(CurrentSelection, BlockName);

		EoDbBlockReference* BlockReference = new EoDbBlockReference();
		BlockReference->SetName(BlockName);
		BlockReference->SetPosition(InsertionPoint);
		EoDbGroup* Group = new EoDbGroup;
		Group->AddTail(BlockReference);
		m_Document->AddWorkLayerGroup(Group);
		m_Document->UpdateGroupInAllViews(EoDb::kGroup, Group);
	}
	CDialog::OnOK();
}

void EoDlgBlockInsert::OnLbnSelchangeBlocksList() {
	const int CurrentSelection = m_BlocksListBoxControl.GetCurSel();

	if (CurrentSelection != LB_ERR) {
		CString BlockName;
		m_BlocksListBoxControl.GetText(CurrentSelection, BlockName);

		EoDbBlock* Block;
		m_Document->LookupBlock(BlockName, Block);
		SetDlgItemInt(IDC_GROUPS, (UINT) Block->GetCount(), FALSE);
		SetDlgItemInt(IDC_REFERENCES, m_Document->GetBlockReferenceCount(BlockName), FALSE);
		WndProcPreviewUpdate(GetDlgItem(IDC_LAYER_PREVIEW)->GetSafeHwnd(), Block);
	}
}

void EoDlgBlockInsert::OnBnClickedPurge() {
	m_Document->PurgeUnreferencedBlocks();

	CDialog::OnOK();
}

void EoDlgBlockInsert::OnBnClickedCancel() {
    // TODO: Add your control notification handler code here
    CDialog::OnCancel();
}
