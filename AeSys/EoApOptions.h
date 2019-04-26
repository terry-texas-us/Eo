#pragma once

class EoApOptions {
private:

public: // Constructors and destructor
	EoApOptions();
	~EoApOptions();

	enum TabsStyle { None, Standard, Grouped };

    TabsStyle m_nTabsStyle;

	CMDITabInfo m_MdiTabInfo;

	BOOL m_bTabsContextMenu;
	BOOL m_bDisableSetRedraw;

public: // Methods
	void Load();
	void Save();
};