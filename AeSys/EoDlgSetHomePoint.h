#pragma once
class EoDlgSetHomePoint final : public CDialog {
DECLARE_DYNAMIC(EoDlgSetHomePoint)

	EoDlgSetHomePoint(CWnd* parent = nullptr);

	EoDlgSetHomePoint(AeSysView* activeView, CWnd* parent = nullptr);

	virtual ~EoDlgSetHomePoint();

	enum { IDD = IDD_HOME_POINT_EDIT };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;

	BOOL OnInitDialog() final;

	void OnOK() final;

	AeSysView* m_ActiveView {nullptr};
	static OdGePoint3d m_CursorPosition;
public:
	CComboBox homePointNames;
	CEdit x;
	CEdit y;
	CEdit z;

	void OnCbnEditUpdateList();

DECLARE_MESSAGE_MAP()
};
