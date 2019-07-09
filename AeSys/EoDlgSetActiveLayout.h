#pragma once
class EoDlgSetActiveLayout final : public CDialog {
	OdDbDatabase* m_Database;
	int m_OldActiveLayout {0};
public:
	int m_NewActiveLayout {0};
	bool m_CreateNewLayout {false};
	CString m_NewLayoutName;

	EoDlgSetActiveLayout(OdDbDatabase* database, CWnd* parent = nullptr);

	void FillListBox();

	enum { IDD = IDD_SET_ACTIVE_LAYOUT };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;

	BOOL OnInitDialog() final;

	void OnLayoutDlgClose();

	void OnSelectionChangeLayoutlist();

	void OnDoubleClickLayoutlist();

	void OnRename();

	void OnDelete();

	void OnCopy();

	void OnNew();

	void OnFromTemplate();

DECLARE_MESSAGE_MAP()
};
