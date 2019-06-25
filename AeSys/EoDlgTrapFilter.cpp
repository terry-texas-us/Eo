#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include <DbLinetypeTable.h>
#include <DbLinetypeTableRecord.h>
#include "EoDlgTrapFilter.h"
#include "EoDbHatch.h"
#include "EoDbPolyline.h"

IMPLEMENT_DYNAMIC(EoDlgTrapFilter, CDialog)

BEGIN_MESSAGE_MAP(EoDlgTrapFilter, CDialog)
END_MESSAGE_MAP()

EoDlgTrapFilter::EoDlgTrapFilter(CWnd* parent)
	: CDialog(IDD, parent) {
}

EoDlgTrapFilter::EoDlgTrapFilter(AeSysDoc* document, OdDbDatabasePtr database, CWnd* parent)
	: CDialog(IDD, parent)
	, EoDlgTrapFilter::document {document}
	, EoDlgTrapFilter::database {database} {
}

EoDlgTrapFilter::~EoDlgTrapFilter() = default;

void EoDlgTrapFilter::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_TRAP_FILTER_LINE_LIST, m_FilterLineComboBoxControl);
	DDX_Control(dataExchange, IDC_TRAP_FILTER_ELEMENT_LIST, m_FilterPrimitiveTypeListBoxControl);
}

BOOL EoDlgTrapFilter::OnInitDialog() {
	CDialog::OnInitDialog();
	SetDlgItemInt(IDC_TRAP_FILTER_PEN_ID, 1, FALSE);
	OdDbLinetypeTablePtr Linetypes {database->getLinetypeTableId().safeOpenObject(OdDb::kForRead)};
	auto Iterator {Linetypes->newIterator()};
	for (Iterator->start(); !Iterator->done(); Iterator->step()) {
		OdDbLinetypeTableRecordPtr Linetype = Iterator->getRecordId().safeOpenObject(OdDb::kForRead);
		m_FilterLineComboBoxControl.AddString(Linetype->getName());
	}
	m_FilterLineComboBoxControl.SetCurSel(0);
	const auto PrimitiveTypes {AeSys::LoadStringResource(IDS_PRIMITIVE_FILTER_LIST)};
	auto TypesPosition {0};
	while (TypesPosition < PrimitiveTypes.GetLength()) {
		m_FilterPrimitiveTypeListBoxControl.AddString(PrimitiveTypes.Tokenize(L"\n", TypesPosition));
	}
	m_FilterPrimitiveTypeListBoxControl.SetCurSel(0);
	return TRUE;
}

void EoDlgTrapFilter::OnOK() {
	if (IsDlgButtonChecked(IDC_TRAP_FILTER_PEN)) {
		const auto ColorIndex {short(GetDlgItemInt(IDC_TRAP_FILTER_PEN_ID, 0, FALSE))};
		FilterByColor(ColorIndex);
	}
	if (IsDlgButtonChecked(IDC_TRAP_FILTER_LINE)) {
		wchar_t Name[32];
		if (GetDlgItemTextW(IDC_TRAP_FILTER_LINE_LIST, Name, sizeof Name / sizeof(wchar_t))) {
			OdDbLinetypeTablePtr Linetypes {database->getLinetypeTableId().safeOpenObject(OdDb::kForRead)};
			if (!Linetypes->getAt(Name).isNull()) {
				const auto LinetypeIndex {gsl::narrow_cast<short>(EoDbLinetypeTable::LegacyLinetypeIndex(Name))};
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
			default: ;
		}
	}
	CDialog::OnOK();
}

void EoDlgTrapFilter::FilterByColor(const short colorIndex) {
	auto GroupPosition {document->GetFirstTrappedGroupPosition()};
	while (GroupPosition != nullptr) {
		const auto Group {document->GetNextTrappedGroup(GroupPosition)};
		auto PrimitivePosition {Group->GetHeadPosition()};
		while (PrimitivePosition != nullptr) {
			const auto Primitive {Group->GetNext(PrimitivePosition)};
			if (Primitive->ColorIndex() == colorIndex) {
				document->RemoveTrappedGroup(Group);
				document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
				break;
			}
		}
	}
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::kTrapCount);
}

void EoDlgTrapFilter::FilterByLinetype(const short linetypeIndex) {
	auto GroupPosition {document->GetFirstTrappedGroupPosition()};
	while (GroupPosition != nullptr) {
		const auto Group {document->GetNextTrappedGroup(GroupPosition)};
		auto PrimitivePosition {Group->GetHeadPosition()};
		while (PrimitivePosition != nullptr) {
			const auto Primitive {Group->GetNext(PrimitivePosition)};
			if (Primitive->LinetypeIndex() == linetypeIndex) {
				document->RemoveTrappedGroup(Group);
				document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
				break;
			}
		}
	}
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::kTrapCount);
}

void EoDlgTrapFilter::FilterByPrimitiveType(const EoDb::PrimitiveTypes primitiveType) {
	auto GroupPosition {document->GetFirstTrappedGroupPosition()};
	while (GroupPosition != nullptr) {
		auto Filter {false};
		const auto Group {document->GetNextTrappedGroup(GroupPosition)};
		auto PrimitivePosition {Group->GetHeadPosition()};
		while (PrimitivePosition != nullptr) {
			const auto Primitive {Group->GetNext(PrimitivePosition)};
			switch (primitiveType) {
				case EoDb::kLinePrimitive:
					Filter = Primitive->IsKindOf(RUNTIME_CLASS(EoDbLine));
					break;
				case EoDb::kEllipsePrimitive:
					Filter = Primitive->IsKindOf(RUNTIME_CLASS(EoDbEllipse));
					break;
				case EoDb::kGroupReferencePrimitive:
					Filter = Primitive->IsKindOf(RUNTIME_CLASS(EoDbBlockReference));
					break;
				case EoDb::kTextPrimitive:
					Filter = Primitive->IsKindOf(RUNTIME_CLASS(EoDbText));
					break;
				case EoDb::kHatchPrimitive:
					Filter = Primitive->IsKindOf(RUNTIME_CLASS(EoDbHatch));
					break;
				case EoDb::kPolylinePrimitive:
					Filter = Primitive->IsKindOf(RUNTIME_CLASS(EoDbPolyline));
					break;
				case EoDb::kPointPrimitive:
					Filter = Primitive->IsKindOf(RUNTIME_CLASS(EoDbPoint));
					break;
				case EoDb::kInsertPrimitive: case EoDb::kSplinePrimitive: case EoDb::kCSplinePrimitive: case EoDb::kTagPrimitive: case EoDb::kDimensionPrimitive: default:
					break;
			}
			if (Filter) {
				document->RemoveTrappedGroup(Group);
				document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
				break;
			}
		}
	}
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::kTrapCount);
}
