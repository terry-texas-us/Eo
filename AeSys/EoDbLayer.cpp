#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"

EoDbLayer::EoDbLayer(OdDbLayerTableRecordPtr layer) :
	m_Layer(layer) {
	m_TracingFlags = 0; 
	m_StateFlags = kIsResident | kIsInternal | kIsActive;
	const OdDbObjectId LinetypeObjectId = layer->linetypeObjectId();
}
EoDbLayer::EoDbLayer(const OdString& name, OdUInt16 stateFlags) {
	// <tas="need to check this .. no defaults"></tas>
	m_TracingFlags = 0;
	m_StateFlags = stateFlags;
}
COLORREF EoDbLayer::Color() const {
	return ColorPalette[m_Layer->colorIndex()];
}
OdInt16 EoDbLayer::ColorIndex() const {
	return m_Layer->colorIndex();
}
void EoDbLayer::Display(AeSysView* view, CDC* deviceContext) {
	EoDbPrimitive::SetLayerColorIndex(ColorIndex());
	EoDbPrimitive::SetLayerLinetypeIndex(LinetypeIndex());

	COLORREF* pCurColTbl = pColTbl;

	pColTbl = (IsCurrent() || IsActive()) ? ColorPalette : GreyPalette;

	EoDbGroupList::Display(view, deviceContext);
	pColTbl = pCurColTbl;
}
void EoDbLayer::Display_(AeSysView* view, CDC* deviceContext, bool identifyTrap) {
	ATLTRACE2(atlTraceGeneral, 1, L"EoDbLayer<%08.8lx>::Display(%08.8lx, %08.8lx, %i) + Name: %s\n", this, view, deviceContext, identifyTrap, (LPCWSTR) this->Name());

	AeSysDoc* Document = AeSysDoc::GetDoc();

	try {
		if (!IsOff()) {
			EoDbPrimitive::SetLayerColorIndex(ColorIndex());
			EoDbPrimitive::SetLayerLinetypeIndex(LinetypeIndex());

			COLORREF* pCurColTbl = pColTbl;

			const bool LayerIsDetectable = IsCurrent() || IsActive();

			pColTbl = LayerIsDetectable ? ColorPalette : GreyPalette;

            auto Position = GetHeadPosition();
			while (Position != 0) {
				EoDbGroup* Group = GetNext(Position);

				if (Group->IsInView(view)) {
					if (LayerIsDetectable) {
						Document->AddGroupToAllViews(Group);
					}
					if (identifyTrap && Document->FindTrappedGroup(Group) != 0) {
						EoDbPrimitive::SetHighlightColorIndex(theApp.TrapHighlightColor());
						Group->Display(view, deviceContext);
						EoDbPrimitive::SetHighlightColorIndex(0);
					}
					else {
						Group->Display(view, deviceContext);
					}
				}
			}
			pColTbl = pCurColTbl;
		}
	}
	catch (CException* e) {
		e->Delete();
	}
}
bool EoDbLayer::IsActive() const noexcept {
	return ((m_StateFlags & kIsActive) == kIsActive);
}
bool EoDbLayer::IsInternal() const noexcept {
	return ((m_StateFlags & kIsInternal) == kIsInternal);
}
bool EoDbLayer::IsOff() const noexcept {
	return ((m_StateFlags & kIsOff) == kIsOff);
}
bool EoDbLayer::IsResident() const noexcept {
	return ((m_StateFlags & kIsResident) == kIsResident);
}
bool EoDbLayer::IsLocked() const noexcept {
	return ((m_StateFlags & kIsLocked) == kIsLocked);
}
bool EoDbLayer::IsCurrent() const {
	const bool IsCurrent = m_Layer->objectId() == m_Layer->database()->getCLAYER();
	VERIFY(((m_StateFlags & kIsCurrent) == kIsCurrent) == IsCurrent);

	return ((m_StateFlags & kIsCurrent) == kIsCurrent);
}
OdInt16 EoDbLayer::LinetypeIndex() {
	OdDbLinetypeTableRecordPtr Linetype = m_Layer->linetypeObjectId().safeOpenObject();
	return EoDbLinetypeTable::LegacyLinetypeIndex(Linetype->getName());
}
OdString EoDbLayer::LinetypeName() {
	OdDbLinetypeTableRecordPtr Linetype = m_Layer->linetypeObjectId().safeOpenObject();
	return Linetype->getName();
}
void EoDbLayer::MakeInternal(bool isInternal) noexcept {
	if (isInternal) {
		m_StateFlags |= kIsInternal;
	}
	else {
		m_StateFlags &= ~kIsInternal;
	}
}
void EoDbLayer::MakeResident(bool isResident) noexcept {
	if (isResident) {
		m_StateFlags |= kIsResident;
	}
	else {
		m_StateFlags &= ~kIsResident;
	}
}
void EoDbLayer::MakeActive() {
	m_StateFlags &= ~(kIsCurrent | kIsLocked | kIsOff);
	m_StateFlags |= kIsActive;

	m_Layer->upgradeOpen();
	m_Layer->setIsOff(false);
	m_Layer->setIsFrozen(false);
	m_Layer->setIsLocked(false);
	// <tas="Make Active not considering CLAYER setting?"</tas>
	m_Layer->downgradeOpen();
}
OdDbLayerTableRecordPtr EoDbLayer::TableRecord() const {
	return (m_Layer);
}
void EoDbLayer::SetIsOff(bool isOff) {
	// <tas="
	// Legacy convention visibility state is exclusive. Never was a SetIsOff(false), always changed state buy MakeCurrent, MakeActive, MakeStatic.
	// This conflicts with Teigha where off layers can retain state as current, frozen or locked.
	// If called with state false - setting layer state to Active for now.
	// </tas>
	if (isOff) {
		m_StateFlags &= ~(kIsCurrent | kIsActive | kIsLocked);
		m_StateFlags |= kIsOff;
	}
	else {
		MakeActive();
	}
	m_Layer->upgradeOpen();
	m_Layer->setIsOff(IsOff());
	m_Layer->downgradeOpen();
}
void EoDbLayer::SetIsFrozen(bool isFrozen) {
	m_Layer->upgradeOpen();
	m_Layer->setIsFrozen(isFrozen);
	m_Layer->downgradeOpen();
}
void EoDbLayer::SetIsLocked(bool isLocked) {
	OdCmTransparency Transparency;
	if (isLocked) {
		m_StateFlags &= ~(kIsCurrent | kIsActive | kIsOff);
		m_StateFlags |= kIsLocked;
		Transparency.setAlpha(OdUInt8(96));
	}
	else {
		MakeActive();
		Transparency.setAlpha(OdUInt8(255));
	}
	m_Layer->upgradeOpen();
	m_Layer->setTransparency(Transparency);
	m_Layer->setIsLocked(IsLocked());
	m_Layer->downgradeOpen();
}
void EoDbLayer::MakeCurrent() noexcept {
	m_StateFlags &= ~(kIsActive | kIsLocked | kIsOff);
	m_StateFlags |= kIsCurrent;
}
OdString EoDbLayer::Name() const {
	return m_Layer->getName();
}
void EoDbLayer::PenTranslation(OdUInt16 wCols, OdInt16* pColNew, OdInt16* pCol) {
	for (int i = 0; i < wCols; i++) {
		if (m_Layer->colorIndex() == pCol[i]) {
			m_Layer->setColorIndex(pColNew[i]);
			break;
		}
	}
	EoDbGroupList::PenTranslation(wCols, pColNew, pCol);
}
void EoDbLayer::SetColorIndex(OdInt16 colorIndex) {
	m_Layer->upgradeOpen();
	m_Layer->setColorIndex(colorIndex);
	m_Layer->downgradeOpen();
}
void EoDbLayer::SetLinetype(OdDbObjectId linetype) {
	m_Layer->upgradeOpen();
	m_Layer->setLinetypeObjectId(linetype);
	m_Layer->downgradeOpen();
}
void EoDbLayer::SetName(const OdString& name) {
	m_Layer->upgradeOpen();
	m_Layer->setName(name);
	m_Layer->downgradeOpen();
}
void EoDbLayer::SetStateFlags(OdUInt16 flags) noexcept {
	m_StateFlags = flags;
}
void EoDbLayer::SetTransparency(const OdCmTransparency& transparency) {
	m_Layer->upgradeOpen();
	m_Layer->setTransparency(transparency);
	m_Layer->downgradeOpen();
}
OdUInt16 EoDbLayer::StateFlags() const noexcept {
	return m_StateFlags;
}

