#pragma once

class EoDlgEditTrapCommandsQuery final : public CDialog {
DECLARE_DYNAMIC(EoDlgEditTrapCommandsQuery)
	EoDlgEditTrapCommandsQuery(CWnd* parent = nullptr);
	virtual ~EoDlgEditTrapCommandsQuery();

	enum { IDD = IDD_EDIT_TRAPCOMMANDS_QUERY };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
	CTreeCtrl m_GroupTreeViewControl;
	CListCtrl m_GeometryListViewControl;
	CListCtrl m_ExtraListViewControl;
public:
	void FillExtraList(EoDbPrimitive* primitive);
	void FillGeometryList(EoDbPrimitive* primitive);
	void OnSelectionChangedGroupTree(NMHDR* notifyStructure, LRESULT* result);
DECLARE_MESSAGE_MAP()
};
