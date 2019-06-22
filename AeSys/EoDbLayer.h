#pragma once
#include "DbLayerTableRecord.h"
#include "EoDbGroupList.h"

class EoDbLayer : public EoDbGroupList {
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
	~EoDbLayer() = default;
	[[nodiscard]] COLORREF Color() const;
	[[nodiscard]] short ColorIndex() const;
	void BuildVisibleGroupList(AeSysView* view);
	void Display(AeSysView* view, CDC* deviceContext);
	void Display_(AeSysView* view, CDC* deviceContext, bool identifyTrap);
	[[nodiscard]] bool IsActive() const noexcept;
	[[nodiscard]] bool IsCurrent() const;
	[[nodiscard]] bool IsInternal() const noexcept;
	[[nodiscard]] bool IsLocked() const noexcept;
	[[nodiscard]] bool IsOff() const noexcept;
	[[nodiscard]] bool IsResident() const noexcept;
	short LinetypeIndex();
	OdString LinetypeName();
	void MakeActive();
	void MakeCurrent() noexcept;
	void MakeInternal(bool isInternal) noexcept;
	void MakeResident(bool isResident) noexcept;
	[[nodiscard]] OdString Name() const;
	void PenTranslation(unsigned numberOfColors, std::vector<int>& newColors, std::vector<int>& pCol);
	void SetColorIndex(short colorIndex);
	void SetIsFrozen(bool isFrozen);
	void SetIsLocked(bool isLocked);
	void SetIsOff(bool isOff);
	void SetLinetype(OdDbObjectId linetype);
	void SetName(const OdString& name);
	void SetStateFlags(unsigned short flags) noexcept;
	void SetTransparency(const OdCmTransparency& transparency);
	[[nodiscard]] unsigned short StateFlags() const noexcept;
	[[nodiscard]] OdDbLayerTableRecordPtr TableRecord() const;
};

using EoDbLayerTable = CTypedPtrArray<CObArray, EoDbLayer*>;
