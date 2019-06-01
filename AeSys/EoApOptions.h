#pragma once

class EoApOptions {
private:

public: // Constructors and destructor
	EoApOptions();
	~EoApOptions();

	enum TabsStyle { None, Standard, Grouped };

	TabsStyle m_nTabsStyle;

	CMDITabInfo m_MdiTabInfo;

	bool m_TabsContextMenu;
	bool m_DisableSetRedraw;

public: // Methods
	void Load();
	void Save();
};