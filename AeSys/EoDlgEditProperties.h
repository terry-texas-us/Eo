#pragma once

class EoDlgEditProperties : public CDialog {
	DECLARE_DYNAMIC(EoDlgEditProperties)

	EoDlgEditProperties(OdDbObjectId& id, CWnd* parent = nullptr);
	virtual ~EoDlgEditProperties();

	enum { IDD = IDD_PROPERTIES };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;

	OdDbObjectId& m_pObjectId;
	OdResBufPtr m_ResourceBuffer;
	int m_nCurItem;
	CButton m_doset;
	CListCtrl m_propList;
	CString m_sValue;

	void OnSetfocusValue();
	void OnButton();
	void OnClickProplist(NMHDR* notifyStructure, LRESULT* result);
	void OnKeydownProplist(NMHDR* notifyStructure, LRESULT* result);

	DECLARE_MESSAGE_MAP()
};