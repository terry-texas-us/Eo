#include "stdafx.h"
#include <DbDatabase.h>
#include <DbFiler.h>
#include <DbObject.h>
#include <DbUnitsFormatter.h>
#include "EoDlgEditProperties.h"
IMPLEMENT_DYNAMIC(EoDlgEditProperties, CDialog)

EoDlgEditProperties::EoDlgEditProperties(OdDbObjectId& id, CWnd* parent)
	: CDialog(IDD, parent)
	, m_pObjectId(id)
	, m_CurrentItem(-1) {
}

EoDlgEditProperties::~EoDlgEditProperties() = default;

void EoDlgEditProperties::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_BUTTON1, m_SetValue);
	DDX_Control(dataExchange, IDC_PROPLIST, m_propList);
	DDX_Text(dataExchange, IDC_VALUE, m_sValue);
}
#pragma warning(push)
#pragma warning(disable : 4191) // (level 3) 'operator': unsafe conversion from 'type_of_expression' to 'type_required'
BEGIN_MESSAGE_MAP(EoDlgEditProperties, CDialog)
		ON_EN_SETFOCUS(IDC_VALUE, OnSetFocusValue)
		ON_BN_CLICKED(IDC_BUTTON1, OnButton)
		ON_NOTIFY(NM_CLICK, IDC_PROPLIST, OnClickProplist)
		ON_NOTIFY(LVN_KEYDOWN, IDC_PROPLIST, OnKeydownProplist)
END_MESSAGE_MAP()
#pragma warning (pop)
static OdString FormatValue(const OdResBuf* resourceBuffer) {
	if (resourceBuffer->restype() == OdResBuf::kRtEntName || resourceBuffer->restype() == OdResBuf::kDxfEnd) {
		const auto ObjectId {resourceBuffer->getObjectId(nullptr)};
		return ObjectId.getHandle().ascii();
	}
	OdString FormattedValue;
	switch (OdDxfCode::_getType(resourceBuffer->restype())) {
		case OdDxfCode::Unknown: // to use RT codes
			if (resourceBuffer->restype() == OdResBuf::kRtColor) {
				FormattedValue = OdDbUnitsFormatter::formatColor(resourceBuffer->getColor());
			}
			break;
		case OdDxfCode::Name: case OdDxfCode::String: case OdDxfCode::Handle: case OdDxfCode::LayerName:
			FormattedValue = resourceBuffer->getString();
			break;
		case OdDxfCode::Bool:
			FormattedValue.format(L"%d", static_cast<int>(resourceBuffer->getBool()));
			break;
		case OdDxfCode::Integer8:
			FormattedValue.format(L"%d", resourceBuffer->getInt8());
			break;
		case OdDxfCode::Integer16:
			FormattedValue.format(L"%d", resourceBuffer->getInt16());
			break;
		case OdDxfCode::Integer32:
			FormattedValue.format(L"%d", resourceBuffer->getInt32());
			break;
		case OdDxfCode::Integer64:
			FormattedValue.format(L"%I64d", resourceBuffer->getInt64());
			break;
		case OdDxfCode::Double: case OdDxfCode::Angle:
			FormattedValue.format(L"%g", resourceBuffer->getDouble());
			break;
		case OdDxfCode::Point:
			FormattedValue.format(L"%g %g %g", resourceBuffer->getPoint3d().x, resourceBuffer->getPoint3d().y, resourceBuffer->getPoint3d().z);
			break;
		case OdDxfCode::ObjectId: case OdDxfCode::SoftPointerId: case OdDxfCode::HardPointerId: case OdDxfCode::SoftOwnershipId: case OdDxfCode::HardOwnershipId:
			FormattedValue = resourceBuffer->getHandle().ascii();
			break;
		case OdDxfCode::BinaryChunk: default:
			break;
	}
	return FormattedValue;
}

static OdString FormatCode(const OdResBuf* resourceBuffer) {
	OdString FormattedCode;
	FormattedCode.format(L"%d", resourceBuffer->restype());
	return FormattedCode;
}

BOOL EoDlgEditProperties::OnInitDialog() {
	CDialog::OnInitDialog();
	m_propList.InsertColumn(0, L"DXF code", LVCFMT_LEFT, 180);
	m_propList.InsertColumn(1, L"Value", LVCFMT_LEFT, 120);
	m_ResourceBuffer = oddbEntGet(m_pObjectId, L"*");
	auto i {0};
	for (auto ResourceBuffer = m_ResourceBuffer; !ResourceBuffer.isNull(); ++i, ResourceBuffer = ResourceBuffer->next()) {
		m_propList.InsertItem(i, FormatCode(ResourceBuffer));
		m_propList.SetItemText(i, 1, FormatValue(ResourceBuffer));
	}
	return TRUE;
}

void EoDlgEditProperties::OnButton() {
	UpdateData();
	if (m_CurrentItem == -1) { return; }
	auto ResourceBuffer {m_ResourceBuffer};
	auto i {0};
	while (!ResourceBuffer.isNull() && i < m_CurrentItem) {
		++i;
		ResourceBuffer = ResourceBuffer->next();
	}
	if (ResourceBuffer.isNull()) { return; }
	switch (ResourceBuffer->restype()) {
		case OdResBuf::kRtColor:
			ResourceBuffer->setColor(OdDbUnitsFormatter::unformatColor(static_cast<const wchar_t*>(m_sValue)));
			break;
		default:
			switch (OdDxfCode::_getType(ResourceBuffer->restype())) {
				case OdDxfCode::Name: case OdDxfCode::String: case OdDxfCode::Handle: case OdDxfCode::LayerName:
					ResourceBuffer->setString(static_cast<const wchar_t*>(m_sValue));
					break;
				case OdDxfCode::Bool:
					ResourceBuffer->setBool(_wtoi(m_sValue) != 0);
					break;
				case OdDxfCode::Integer8:
					ResourceBuffer->setInt8(signed char(_wtoi(m_sValue)));
					break;
				case OdDxfCode::Integer16:
					ResourceBuffer->setInt16(short(_wtoi(m_sValue)));
					break;
				case OdDxfCode::Integer32:
					ResourceBuffer->setInt32(_wtoi(m_sValue));
					break;
				case OdDxfCode::Double: case OdDxfCode::Angle:
					ResourceBuffer->setDouble(wcstod(m_sValue, nullptr));
					break;
				case OdDxfCode::Point: {
					const auto sp1 {m_sValue.Find(' ')};
					const auto sp2 {m_sValue.Find(' ', sp1 + 1)};
					const auto x {wcstod(m_sValue.Left(sp1), nullptr)};
					const auto y {wcstod(m_sValue.Mid(sp1 + 1, sp2 - sp1 - 1), nullptr)};
					const auto z {wcstod(m_sValue.Mid(sp2 + 1), nullptr)};
					ResourceBuffer->setPoint3d(OdGePoint3d(x, y, z));
					break;
				}
				case OdDxfCode::ObjectId: case OdDxfCode::SoftPointerId: case OdDxfCode::HardPointerId: case OdDxfCode::SoftOwnershipId: case OdDxfCode::HardOwnershipId:
					ResourceBuffer->setHandle(OdDbHandle(m_sValue.GetString()));
					break;
				case OdDxfCode::Unknown: case OdDxfCode::BinaryChunk: case OdDxfCode::Integer64: default:
					break;
			}
	}
	m_propList.SetItemText(m_CurrentItem, 1, m_sValue);
	try {
		oddbEntMod(m_pObjectId, m_ResourceBuffer);
	} catch (const OdError& Error) {
		AfxMessageBox(Error.description());
	}
}

void EoDlgEditProperties::OnClickProplist(NMHDR* /*notifyStructure*/, LRESULT* result) {
	OnSetFocusValue();
	*result = 0;
}

void EoDlgEditProperties::OnSetFocusValue() {
	m_CurrentItem = m_propList.GetSelectionMark();
	m_SetValue.EnableWindow(static_cast<BOOL>(m_CurrentItem != -1));
	if (m_CurrentItem != -1) {
		m_sValue = m_propList.GetItemText(m_CurrentItem, 1);
		UpdateData(FALSE);
	}
}

void EoDlgEditProperties::OnKeydownProplist(NMHDR* /*notifyStructure*/, LRESULT* result) {
	OnSetFocusValue();
	*result = 0;
}
