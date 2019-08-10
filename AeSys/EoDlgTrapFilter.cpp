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
	: CDialog(IDD, parent) {}

EoDlgTrapFilter::EoDlgTrapFilter(AeSysDoc* document, OdDbDatabasePtr database, CWnd* parent)
	: CDialog(IDD, parent)
	, m_Document {document}
	, m_Database {database} {}

EoDlgTrapFilter::~EoDlgTrapFilter() = default;

void EoDlgTrapFilter::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_TRAP_FILTER_LINE_LIST, filterLineComboBoxControl);
	DDX_Control(dataExchange, IDC_TRAP_FILTER_ELEMENT_LIST, filterPrimitiveTypeListBoxControl);
}

BOOL EoDlgTrapFilter::OnInitDialog() {
	CDialog::OnInitDialog();
	SetDlgItemInt(IDC_TRAP_FILTER_PEN_ID, 1, FALSE);
	OdDbLinetypeTablePtr Linetypes {m_Database->getLinetypeTableId().safeOpenObject(OdDb::kForRead)};
	auto Iterator {Linetypes->newIterator()};
	for (Iterator->start(); !Iterator->done(); Iterator->step()) {
		OdDbLinetypeTableRecordPtr Linetype {Iterator->getRecordId().safeOpenObject(OdDb::kForRead)};
		filterLineComboBoxControl.AddString(Linetype->getName());
	}
	filterLineComboBoxControl.SetCurSel(0);
	const auto PrimitiveTypes {AeSys::LoadStringResource(IDS_PRIMITIVE_FILTER_LIST)};
	auto TypesPosition {0};
	while (TypesPosition < PrimitiveTypes.GetLength()) {
		filterPrimitiveTypeListBoxControl.AddString(PrimitiveTypes.Tokenize(L"\n", TypesPosition));
	}
	filterPrimitiveTypeListBoxControl.SetCurSel(0);
	return TRUE;
}

void EoDlgTrapFilter::OnOK() {
	if (IsDlgButtonChecked(IDC_TRAP_FILTER_PEN) != 0U) {
		const auto ColorIndex {short(GetDlgItemInt(IDC_TRAP_FILTER_PEN_ID, nullptr, FALSE))};
		FilterByColor(ColorIndex);
	}
	if (IsDlgButtonChecked(IDC_TRAP_FILTER_LINE) != 0U) {
		wchar_t Name[32];
		if (GetDlgItemTextW(IDC_TRAP_FILTER_LINE_LIST, Name, sizeof Name / sizeof(wchar_t)) != 0) {
			OdDbLinetypeTablePtr Linetypes {m_Database->getLinetypeTableId().safeOpenObject(OdDb::kForRead)};
			if (!Linetypes->getAt(Name).isNull()) {
				const auto LinetypeIndex {gsl::narrow_cast<short>(EoDbLinetypeTable::LegacyLinetypeIndex(Name))};
				FilterByLinetype(LinetypeIndex);
			}
		}
	}
	if (IsDlgButtonChecked(IDC_TRAP_FILTER_ELEMENT) != 0U) {
		switch (filterPrimitiveTypeListBoxControl.GetCurSel()) {
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

void EoDlgTrapFilter::FilterByColor(const short colorIndex) const {
	auto GroupPosition {m_Document->GetFirstTrappedGroupPosition()};
	while (GroupPosition != nullptr) {
		const auto Group {m_Document->GetNextTrappedGroup(GroupPosition)};
		auto PrimitivePosition {Group->GetHeadPosition()};
		while (PrimitivePosition != nullptr) {
			const auto Primitive {Group->GetNext(PrimitivePosition)};
			if (Primitive->ColorIndex() == colorIndex) {
				m_Document->RemoveTrappedGroup(Group);
				m_Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
				break;
			}
		}
	}
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::kTrapCount);
}

void EoDlgTrapFilter::FilterByLinetype(const short linetypeIndex) const {
	auto GroupPosition {m_Document->GetFirstTrappedGroupPosition()};
	while (GroupPosition != nullptr) {
		const auto Group {m_Document->GetNextTrappedGroup(GroupPosition)};
		auto PrimitivePosition {Group->GetHeadPosition()};
		while (PrimitivePosition != nullptr) {
			const auto Primitive {Group->GetNext(PrimitivePosition)};
			if (Primitive->LinetypeIndex() == linetypeIndex) {
				m_Document->RemoveTrappedGroup(Group);
				m_Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
				break;
			}
		}
	}
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::kTrapCount);
}

void EoDlgTrapFilter::FilterByPrimitiveType(const EoDb::PrimitiveTypes primitiveType) const {
	auto GroupPosition {m_Document->GetFirstTrappedGroupPosition()};
	while (GroupPosition != nullptr) {
		auto Filter {false};
		const auto Group {m_Document->GetNextTrappedGroup(GroupPosition)};
		auto PrimitivePosition {Group->GetHeadPosition()};
		while (PrimitivePosition != nullptr) {
			const auto Primitive {Group->GetNext(PrimitivePosition)};
			switch (primitiveType) {
				case EoDb::kLinePrimitive:
					Filter = Primitive->IsKindOf(RUNTIME_CLASS(EoDbLine)) != 0;
					break;
				case EoDb::kEllipsePrimitive:
					Filter = Primitive->IsKindOf(RUNTIME_CLASS(EoDbEllipse)) != 0;
					break;
				case EoDb::kGroupReferencePrimitive:
					Filter = Primitive->IsKindOf(RUNTIME_CLASS(EoDbBlockReference)) != 0;
					break;
				case EoDb::kTextPrimitive:
					Filter = Primitive->IsKindOf(RUNTIME_CLASS(EoDbText)) != 0;
					break;
				case EoDb::kHatchPrimitive:
					Filter = Primitive->IsKindOf(RUNTIME_CLASS(EoDbHatch)) != 0;
					break;
				case EoDb::kPolylinePrimitive:
					Filter = Primitive->IsKindOf(RUNTIME_CLASS(EoDbPolyline)) != 0;
					break;
				case EoDb::kPointPrimitive:
					Filter = Primitive->IsKindOf(RUNTIME_CLASS(EoDbPoint)) != 0;
					break;
				case EoDb::kInsertPrimitive: case EoDb::kSplinePrimitive: case EoDb::kCSplinePrimitive: case EoDb::kTagPrimitive: case EoDb::kDimensionPrimitive: default:
					break;
			}
			if (Filter) {
				m_Document->RemoveTrappedGroup(Group);
				m_Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
				break;
			}
		}
	}
	AeSysView::GetActiveView()->UpdateStateInformation(AeSysView::kTrapCount);
}
