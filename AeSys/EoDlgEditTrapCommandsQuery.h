#pragma once

// EoDlgEditTrapCommandsQuery dialog

class EoDlgEditTrapCommandsQuery : public CDialog {
    DECLARE_DYNAMIC(EoDlgEditTrapCommandsQuery)

public:
    EoDlgEditTrapCommandsQuery(CWnd* parent = nullptr);
    virtual ~EoDlgEditTrapCommandsQuery();

    // Dialog Data
    enum { IDD = IDD_EDIT_TRAPCOMMANDS_QUERY };

protected:
    void DoDataExchange(CDataExchange* pDX) final;
    BOOL OnInitDialog() final;

    CTreeCtrl m_GroupTreeViewControl;
    CListCtrl m_GeometryListViewControl;
    CListCtrl m_ExtraListViewControl;

public:
    void FillExtraList(EoDbPrimitive* primitive);
    void FillGeometryList(EoDbPrimitive* primitive);

    void OnTvnSelchangedGroupTree(NMHDR* pNMHDR, LRESULT* result);

protected:
    DECLARE_MESSAGE_MAP()
};
