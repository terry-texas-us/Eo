#pragma once
class EoApOptions {
public: // Constructors and destructor
	EoApOptions();
	~EoApOptions() = default;

	enum TabsStyle { None, Standard, Grouped };

	TabsStyle m_nTabsStyle;
	CMDITabInfo m_MdiTabInfo;
	bool m_TabsContextMenu;
	bool m_DisableSetRedraw;

	// Methods
	void Load();
	void Save();
};
