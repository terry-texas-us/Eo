#pragma once
class EoDlgSetupLinetype final : public CDialog {
DECLARE_DYNAMIC(EoDlgSetupLinetype)

	EoDlgSetupLinetype(CWnd* parent = nullptr);

	EoDlgSetupLinetype(OdDbLinetypeTablePtr linetypeTable, CWnd* parent = nullptr);

	virtual ~EoDlgSetupLinetype();

	enum { IDD = IDD_SETUP_LINETYPE };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;

	BOOL OnInitDialog() final;

	void OnOK() final;

	enum LinetypesListColumnLabels { Name, Appearance, Description };

	OdDbLinetypeTablePtr m_LinetypeTable;
	CListCtrl m_LinetypesListControl;
public:
	OdDbLinetypeTableRecordPtr linetype;

	void OnDrawItem(int controlIdentifier, LPDRAWITEMSTRUCT drawItemStruct); // hides non-virtual function of parent
	void OnBnClickedBylayerButton();

	void OnBnClickedByblockButton();

DECLARE_MESSAGE_MAP()
};
