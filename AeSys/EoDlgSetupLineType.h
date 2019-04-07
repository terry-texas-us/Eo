#pragma once

// EoDlgSetupLinetype dialog

class EoDlgSetupLinetype : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetupLinetype)

public:
	EoDlgSetupLinetype(CWnd* parent = NULL);
	EoDlgSetupLinetype(OdDbLinetypeTablePtr linetypeTable, CWnd* parent = NULL);
	virtual ~EoDlgSetupLinetype();

// Dialog Data
	enum { IDD = IDD_SETUP_LINETYPE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override;

	enum LinetypesListColumnLabels {
		Name,
		Appearance,
		Description
	};
	OdDbLinetypeTablePtr m_LinetypeTable;
	CListCtrl m_LinetypesListControl;

public:
	OdDbLinetypeTableRecordPtr m_Linetype;
	
	afx_msg void OnDrawItem(int controlIdentifier, LPDRAWITEMSTRUCT drawItemStruct);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBylayerButton();
	afx_msg void OnBnClickedByblockButton();
};