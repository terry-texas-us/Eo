#pragma once
#include <DbLayerTableRecord.h>
#include "EoDbGroupList.h"

class EoDbLayer final : public EoDbGroupList {
	OdDbLayerTableRecordPtr m_Layer;
	unsigned short m_StateFlags {kIsResident | kIsInternal | kIsActive};
	unsigned short m_TracingFlags {0U};
public:
	enum StateFlags : unsigned {
		kIsResident = 0x0001U, // entry in table list is saved
		kIsInternal = 0x0002U, // group list saved within drawing
		kIsCurrent = 0x0004U, // may have groups added (0 or 1), displayed using hot color set
		kIsActive = 0x0008U, // may have groups modified (0 or more), displayed using warm color set
		kIsLocked = 0x0010U, // tracing which is viewed or layer which is static (no additions or modifications), displayed using warm color set
		kIsOff = 0x0020U
	};

	EoDbLayer(OdDbLayerTableRecordPtr layer);

	EoDbLayer(const OdString& name, unsigned short stateFlags);

	[[nodiscard]] COLORREF Color() const;

	[[nodiscard]] short ColorIndex() const;

	void BuildVisibleGroupList(AeSysView* view);

	void Display(AeSysView* view, CDC* deviceContext); // hides non-virtual function of parent
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

	void PenTranslation(unsigned numberOfColors, std::vector<int>& newColors, std::vector<int>& pCol); // hides non-virtual function of parent
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
