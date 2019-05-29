#pragma once

// EoDlgSetupLinetype dialog

class EoDlgSetupLinetype : public CDialog {
    DECLARE_DYNAMIC(EoDlgSetupLinetype)

public:
    EoDlgSetupLinetype(CWnd* parent = nullptr);
    EoDlgSetupLinetype(OdDbLinetypeTablePtr linetypeTable, CWnd* parent = nullptr);
    virtual ~EoDlgSetupLinetype();

    // Dialog Data
    enum { IDD = IDD_SETUP_LINETYPE };

protected:
    void DoDataExchange(CDataExchange* pDX) final;
    BOOL OnInitDialog() final;
    void OnOK() final;

    enum LinetypesListColumnLabels {
        Name,
        Appearance,
        Description
    };
    OdDbLinetypeTablePtr m_LinetypeTable;
    CListCtrl m_LinetypesListControl;

public:
    OdDbLinetypeTableRecordPtr m_Linetype;

    void OnDrawItem(int controlIdentifier, LPDRAWITEMSTRUCT drawItemStruct);
    void OnBnClickedBylayerButton();
    void OnBnClickedByblockButton();

protected:
    DECLARE_MESSAGE_MAP()
};