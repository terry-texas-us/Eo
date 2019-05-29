#pragma once

#include "EoDbGroupList.h"

class EoDbLayer : public EoDbGroupList {
private:
	OdDbLayerTableRecordPtr m_Layer;
	unsigned short m_StateFlags;	
	unsigned short m_TracingFlags;

public:
	enum StateFlags {
		kIsResident = 0x0001,	// entry in table list is saved
		kIsInternal = 0x0002,	// group list saved within drawing
		kIsCurrent = 0x0004,	// may have groups added (0 or 1), displayed using hot color set
		kIsActive = 0x0008,		// may have groups modified (0 or more), displayed using warm color set
		kIsLocked = 0x0010,		// tracing which is viewed or layer which is static (no additions or modifications), displayed using warm color set
		kIsOff = 0x0020
	};
	EoDbLayer(OdDbLayerTableRecordPtr layer);
	EoDbLayer(const OdString& name, unsigned short flags);

	~EoDbLayer() {}
	COLORREF Color() const;
	short ColorIndex() const;
	void BuildVisibleGroupList(AeSysView* view);
	void Display(AeSysView* view, CDC* deviceContext);
	void Display_(AeSysView* view, CDC* deviceContext, bool identifyTrap);
	bool IsActive() const noexcept;
	bool IsCurrent() const;
	bool IsInternal() const noexcept;
	bool IsLocked() const noexcept;
	bool IsOff() const noexcept;
	bool IsResident() const noexcept;
	short LinetypeIndex();
	OdString LinetypeName();
	void MakeActive();
	void MakeCurrent() noexcept;
	void MakeInternal(bool isInternal) noexcept;
	void MakeResident(bool isResident) noexcept;
	OdString Name() const;
	void PenTranslation(unsigned short, short*, short*);
	void SetColorIndex(short colorIndex);
	void SetIsFrozen(bool isFrozen);
	void SetIsLocked(bool isLocked);
	void SetIsOff(bool isOff);
	void SetLinetype(OdDbObjectId linetype);
	void SetName(const OdString& name);
	void SetStateFlags(unsigned short flags) noexcept;
	void SetTransparency(const OdCmTransparency& transparency);
	unsigned short StateFlags() const noexcept;
	OdDbLayerTableRecordPtr TableRecord() const;
};

typedef CTypedPtrArray<CObArray, EoDbLayer*> EoDbLayerTable;
