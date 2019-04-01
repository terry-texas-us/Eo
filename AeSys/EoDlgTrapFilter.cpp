#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDlgTrapFilter.h"

// EoDlgTrapFilter dialog

IMPLEMENT_DYNAMIC(EoDlgTrapFilter, CDialog)

BEGIN_MESSAGE_MAP(EoDlgTrapFilter, CDialog)
END_MESSAGE_MAP()

EoDlgTrapFilter::EoDlgTrapFilter(CWnd* parent) 
    : CDialog(EoDlgTrapFilter::IDD, parent)
    , m_Document(0) {
}

EoDlgTrapFilter::EoDlgTrapFilter(AeSysDoc* document, OdDbDatabasePtr database, CWnd* parent)
    : CDialog(EoDlgTrapFilter::IDD, parent)
    , m_Document(document)
    , m_Database(database) {
}

EoDlgTrapFilter::~EoDlgTrapFilter() {
}

void EoDlgTrapFilter::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TRAP_FILTER_LINE_LIST, m_FilterLineComboBoxControl);
	DDX_Control(pDX, IDC_TRAP_FILTER_ELEMENT_LIST, m_FilterPrimitiveTypeListBoxControl);
}
BOOL EoDlgTrapFilter::OnInitDialog() {
	CDialog::OnInitDialog();

	SetDlgItemInt(IDC_TRAP_FILTER_PEN_ID, 1, FALSE);

	OdDbLinetypeTablePtr Linetypes = m_Database->getLinetypeTableId().safeOpenObject(OdDb::kForRead);
	OdDbSymbolTableIteratorPtr Iterator = Linetypes->newIterator();

	for (Iterator->start(); !Iterator->done(); Iterator->step()) {
		OdDbLinetypeTableRecordPtr Linetype = Iterator->getRecordId().safeOpenObject(OdDb::kForRead);
		m_FilterLineComboBoxControl.AddString(Linetype->getName());
	}
	m_FilterLineComboBoxControl.SetCurSel(0);

	CString PrimitiveTypes = theApp.LoadStringResource(IDS_PRIMITIVE_FILTER_LIST);

	int TypesPosition = 0;
	while (TypesPosition < PrimitiveTypes.GetLength()) {
		m_FilterPrimitiveTypeListBoxControl.AddString(PrimitiveTypes.Tokenize(L"\n", TypesPosition));
	}
	m_FilterPrimitiveTypeListBoxControl.SetCurSel(0);

	return TRUE;
}
void EoDlgTrapFilter::OnOK() {
	if (IsDlgButtonChecked(IDC_TRAP_FILTER_PEN)) {
		const EoInt16 ColorIndex = EoInt16(GetDlgItemInt(IDC_TRAP_FILTER_PEN_ID, 0, FALSE));
		FilterByColor(ColorIndex);
	}
	if (IsDlgButtonChecked(IDC_TRAP_FILTER_LINE)) {
		wchar_t Name[32];

		if (GetDlgItemTextW(IDC_TRAP_FILTER_LINE_LIST, (LPWSTR)Name, sizeof(Name) / sizeof(wchar_t))) {
			OdDbLinetypeTablePtr Linetypes = m_Database->getLinetypeTableId().safeOpenObject(OdDb::kForRead);
			if (!Linetypes->getAt(Name).isNull()) {
				EoInt16 LinetypeIndex = EoDbLinetypeTable::LegacyLinetypeIndex(Name);
				FilterByLinetype(LinetypeIndex);
			}
		}
	}
	if (IsDlgButtonChecked(IDC_TRAP_FILTER_ELEMENT)) {
		switch (m_FilterPrimitiveTypeListBoxControl.GetCurSel()) {
		case 0:
			FilterByPrimitiveType(EoDb::kEllipsePrimitive);
			break;
		case 1:
			FilterByPrimitiveType(EoDb::kGroupReferencePrimitive);
			break;
		case 2:
			FilterByPrimitiveType(EoDb::kLinePrimitive);
			break;
		case 3:
			FilterByPrimitiveType(EoDb::kPointPrimitive);
			break;
		case 4:
			FilterByPrimitiveType(EoDb::kTextPrimitive);
			break;
		case 5:
			FilterByPrimitiveType(EoDb::kHatchPrimitive);
			break;
		case 6:
			FilterByPrimitiveType(EoDb::kPolylinePrimitive);
		}
	}
	CDialog::OnOK();
}
void EoDlgTrapFilter::FilterByColor(EoInt16 colorIndex) {
	POSITION GroupPosition = m_Document->GetFirstTrappedGroupPosition();
	while (GroupPosition != 0) {
		EoDbGroup* Group = m_Document->GetNextTrappedGroup(GroupPosition);

		POSITION PrimitivePosition = Group->GetHeadPosition();
		while (PrimitivePosition != 0) {
			const EoDbPrimitive* Primitive = Group->GetNext(PrimitivePosition);
			if (Primitive->ColorIndex() == colorIndex) {
				m_Document->RemoveTrappedGroup(Group);
				m_Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
				break;
			}
		}
	}
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::TrapCount);
}
void EoDlgTrapFilter::FilterByLinetype(EoInt16 linetypeIndex) {
	POSITION GroupPosition = m_Document->GetFirstTrappedGroupPosition();
	while (GroupPosition != 0) {
		EoDbGroup* Group = m_Document->GetNextTrappedGroup(GroupPosition);

		POSITION PrimitivePosition = Group->GetHeadPosition();
		while (PrimitivePosition != 0) {
			const EoDbPrimitive* Primitive = Group->GetNext(PrimitivePosition);
			if (Primitive->LinetypeIndex() == linetypeIndex) {
				m_Document->RemoveTrappedGroup(Group);
				m_Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
				break;
			}
		}
	}
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::TrapCount);
}
void EoDlgTrapFilter::FilterByPrimitiveType(const EoDb::PrimitiveTypes primitiveType) {
	POSITION GroupPosition = m_Document->GetFirstTrappedGroupPosition();
	while (GroupPosition != 0) {
		bool bFilter = FALSE;

		EoDbGroup* Group = m_Document->GetNextTrappedGroup(GroupPosition);

		POSITION PrimitivePosition = Group->GetHeadPosition();
		while (PrimitivePosition != 0) {
			const EoDbPrimitive* Primitive = Group->GetNext(PrimitivePosition);

			switch (primitiveType) {
			case EoDb::kLinePrimitive:
				bFilter = Primitive->Is(EoDb::kLinePrimitive);
				break;
			case EoDb::kEllipsePrimitive:
				bFilter = Primitive->Is(EoDb::kEllipsePrimitive);
				break;
			case EoDb::kGroupReferencePrimitive:
				bFilter = Primitive->Is(EoDb::kGroupReferencePrimitive);
				break;
			case EoDb::kTextPrimitive:
				bFilter = Primitive->Is(EoDb::kTextPrimitive);
				break;
			case EoDb::kHatchPrimitive:
				bFilter = Primitive->Is(EoDb::kHatchPrimitive);
				break;
			case EoDb::kPolylinePrimitive:
				bFilter = Primitive->Is(EoDb::kPolylinePrimitive);
				break;
			case EoDb::kPointPrimitive:
				bFilter = Primitive->Is(EoDb::kPointPrimitive);
				break;
			}
			if (bFilter) {
				m_Document->RemoveTrappedGroup(Group);
				m_Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
				break;
			}
		}
	}
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::TrapCount);
}
