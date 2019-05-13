#include "stdafx.h"
#include "resource.h"
#include "DbFiler.h"
#include "DbObject.h"
#include "DbUnitsFormatter.h"

#include "EoDlgEditProperties.h"

IMPLEMENT_DYNAMIC(EoDlgEditProperties, CDialog)
EoDlgEditProperties::EoDlgEditProperties(OdDbObjectId& id, CWnd* parent)
	: CDialog(EoDlgEditProperties::IDD, parent)
	, m_pObjectId(id)
	, m_nCurItem(-1) {
}

EoDlgEditProperties::~EoDlgEditProperties() {
}

void EoDlgEditProperties::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, m_doset);
	DDX_Control(pDX, IDC_PROPLIST, m_propList);
	DDX_Text(pDX, IDC_VALUE, m_sValue);
}

BEGIN_MESSAGE_MAP(EoDlgEditProperties, CDialog)
	ON_EN_SETFOCUS(IDC_VALUE, OnSetfocusValue)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton)
	ON_NOTIFY(NM_CLICK, IDC_PROPLIST, OnClickProplist)
	ON_NOTIFY(LVN_KEYDOWN, IDC_PROPLIST, OnKeydownProplist)
END_MESSAGE_MAP()

static OdString FormatValue(const OdResBuf* rb) {
	OdString s;

	if (rb->restype() == OdResBuf::kRtEntName || rb->restype() == OdResBuf::kDxfEnd) {
		const OdDbObjectId id = rb->getObjectId(0);
		s = id.getHandle().ascii();
		return s;
	}
	switch (OdDxfCode::_getType(rb->restype())) {
		case OdDxfCode::Unknown: // to use RT codes
			if (rb->restype() == OdResBuf::kRtColor)
				s = OdDbUnitsFormatter::formatColor(rb->getColor());
			break;
		case OdDxfCode::Name:
		case OdDxfCode::String:
		case OdDxfCode::Handle:
		case OdDxfCode::LayerName:
			s = rb->getString();
			break;
		case OdDxfCode::Bool:
			s.format(L"%d", rb->getBool());
			break;
		case OdDxfCode::Integer8:
			s.format(L"%d", rb->getInt8());
			break;
		case OdDxfCode::Integer16:
			s.format(L"%d", rb->getInt16());
			break;
		case OdDxfCode::Integer32:
			s.format(L"%d", rb->getInt32());
			break;
		case OdDxfCode::Integer64:
			s.format(OD_T(PRId64), rb->getInt64());
			break;
		case OdDxfCode::Double:
		case OdDxfCode::Angle:
			s.format(L"%g", rb->getDouble());
			break;
		case OdDxfCode::Point:
			s.format(L"%g %g %g", rb->getPoint3d().x, rb->getPoint3d().y, rb->getPoint3d().z);
			break;
		case OdDxfCode::ObjectId:
		case OdDxfCode::SoftPointerId:
		case OdDxfCode::HardPointerId:
		case OdDxfCode::SoftOwnershipId:
		case OdDxfCode::HardOwnershipId:
			s = rb->getHandle().ascii();
			break;
		default:
			break;
	}
	return s;
}
static OdString FormatCode(const OdResBuf * rb) {
	OdString s;
	s.format(L"%d", rb->restype());
	return s;
}
BOOL EoDlgEditProperties::OnInitDialog() {
	CDialog::OnInitDialog();
	m_propList.InsertColumn(0, L"DXF code", LVCFMT_LEFT, 180);
	m_propList.InsertColumn(1, L"Value", LVCFMT_LEFT, 120);

	m_pResBuf = oddbEntGet(m_pObjectId, L"*");
	int i = 0;
	for (OdResBufPtr rb = m_pResBuf; !rb.isNull(); ++i, rb = rb->next()) {
		m_propList.InsertItem(i, FormatCode(rb));
		m_propList.SetItemText(i, 1, FormatValue(rb));
	}
	return TRUE;
}
void EoDlgEditProperties::OnButton() {
	UpdateData();
	if (m_nCurItem == -1) {
		return;
	}
	OdResBufPtr rb = m_pResBuf;
	int i = 0;
	while (!rb.isNull() && i < m_nCurItem) {
		++i;
		rb = rb->next();
	}
	if (rb.isNull()) {
		return;
	}
	switch (rb->restype()) {
		case OdResBuf::kRtColor:
			rb->setColor(OdDbUnitsFormatter::unformatColor((LPCWSTR) m_sValue));
			break;
		default:
			switch (OdDxfCode::_getType(rb->restype())) {
				case OdDxfCode::Name:
				case OdDxfCode::String:
				case OdDxfCode::Handle:
				case OdDxfCode::LayerName:
					rb->setString((LPCWSTR) m_sValue);
					break;
				case OdDxfCode::Bool:
					rb->setBool(_wtoi(m_sValue) != 0);
					break;
				case OdDxfCode::Integer8:
					rb->setInt8(OdInt8(_wtoi(m_sValue)));
					break;
				case OdDxfCode::Integer16:
					rb->setInt16(OdInt16(_wtoi(m_sValue)));
					break;
				case OdDxfCode::Integer32:
					rb->setInt32(_wtoi(m_sValue));
					break;
				case OdDxfCode::Double:
				case OdDxfCode::Angle:
					rb->setDouble(wcstod(m_sValue, 0));
					break;
				case OdDxfCode::Point:
				{
					const int sp1 = m_sValue.Find(' ');
					const int sp2 = m_sValue.Find(' ', sp1 + 1);
					double x = wcstod(m_sValue.Left(sp1), 0);
					double y = wcstod(m_sValue.Mid(sp1 + 1, sp2 - sp1 - 1), 0);
					double z = wcstod(m_sValue.Mid(sp2 + 1), 0);
					rb->setPoint3d(OdGePoint3d(x, y, z));
					break;
				}
				case OdDxfCode::ObjectId:
				case OdDxfCode::SoftPointerId:
				case OdDxfCode::HardPointerId:
				case OdDxfCode::SoftOwnershipId:
				case OdDxfCode::HardOwnershipId:
					rb->setHandle(OdDbHandle((LPCWSTR) m_sValue));
					break;
				default:
					break;
			}
	}
	m_propList.SetItemText(m_nCurItem, 1, m_sValue);
	try {
		oddbEntMod(m_pObjectId, m_pResBuf);
	} catch (const OdError & Error) {
		AfxMessageBox(Error.description());
	}
}
void EoDlgEditProperties::OnClickProplist(NMHDR* /*pNMHDR*/, LRESULT * result) {
	OnSetfocusValue();
	*result = 0;
}
void EoDlgEditProperties::OnSetfocusValue() {
	m_nCurItem = m_propList.GetSelectionMark();
	m_doset.EnableWindow(m_nCurItem != -1);
	if (m_nCurItem != -1) {
		m_sValue = m_propList.GetItemText(m_nCurItem, 1);
		UpdateData(FALSE);
	}
}
void EoDlgEditProperties::OnKeydownProplist(NMHDR* /*pNMHDR*/, LRESULT * result) {
	OnSetfocusValue();
	*result = 0;
}