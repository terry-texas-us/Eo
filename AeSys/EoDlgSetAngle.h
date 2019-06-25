#pragma once

class EoDlgSetAngle final : public CDialog {
DECLARE_DYNAMIC(EoDlgSetAngle)
	EoDlgSetAngle(CWnd* parent = nullptr);
	virtual ~EoDlgSetAngle();

	enum { IDD = IDD_SET_ANGLE };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
public:
	double angle {0.0};
	CString title;
protected:
DECLARE_MESSAGE_MAP()
};
