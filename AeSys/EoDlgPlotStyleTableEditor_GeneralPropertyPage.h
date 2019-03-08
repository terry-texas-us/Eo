#pragma once

#include "Ps/PlotStyles.h"

#define  PS_EDIT_MAX_SCALEFACTOR    10

class EoDlgPlotStyleEditor_GeneralPropertyPage : public CPropertyPage {
	DECLARE_DYNCREATE(EoDlgPlotStyleEditor_GeneralPropertyPage)


	OdPsPlotStyleTable* m_pPlotStyleTable;
	OdString m_sFileBufPath;

public:
	EoDlgPlotStyleEditor_GeneralPropertyPage();
	~EoDlgPlotStyleEditor_GeneralPropertyPage();

	enum { IDD = IDD_PLOTSTYLE_GENERAL_PROPERTY_PAGE };

	CEdit	  m_editDescription;
	CButton m_checkScalefactor;
	CEdit   m_editScalefactor;
	CStatic   	m_staticFilepath;
	CStatic   	m_staticFilename;
	CStatic m_staticBitmap;
	CStatic m_staticRegular;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

public:
	const bool SetPlotStyleTable(OdPsPlotStyleTable* pPlotStyleTable);
	void SetFileBufPath(const OdString sFilePath);

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditDescription();
	afx_msg void OnCheckScalefactor();
	afx_msg void OnEditScalefactor();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
};