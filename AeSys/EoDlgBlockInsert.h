#pragma once

class EoDlgBlockInsert : public CDialog {
DECLARE_DYNAMIC(EoDlgBlockInsert)
	EoDlgBlockInsert(CWnd* parent = nullptr);
	EoDlgBlockInsert(AeSysDoc* document, CWnd* parent = nullptr);
	~EoDlgBlockInsert();

	enum { IDD = IDD_INSERT_BLOCK };

protected:
	void DoDataExchange(CDataExchange* dataExchange) override;
	BOOL OnInitDialog() override;
	void OnOK() override;
	static OdGePoint3d InsertionPoint;
	AeSysDoc* m_Document;
public:
	CListBox m_BlocksListBoxControl;
	CEdit m_InsertionPointX;
	CEdit m_InsertionPointY;
	CEdit m_InsertionPointZ;
	CEdit m_ScaleX;
	CEdit m_ScaleY;
	CEdit m_ScaleZ;
	CEdit m_RotationAngle;
	CButton m_InsertionPointOnscreen;
	CButton m_ScaleOnscreen;
	CButton m_RotationOnscreen;
	CButton m_Explode;
	void OnLbnSelchangeBlocksList();
	void OnBnClickedPurge();
protected:
DECLARE_MESSAGE_MAP()
public:
	void OnBnClickedCancel();
};
