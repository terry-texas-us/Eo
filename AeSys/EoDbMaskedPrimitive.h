#pragma once

class EoDbMaskedPrimitive : public CObject {
private:
	EoDbPrimitive* m_Primitive;
	DWORD m_Mask;

public:
	EoDbMaskedPrimitive() {
		m_Primitive = (EoDbPrimitive*) 0;
		m_Mask = 0;
	}
	EoDbMaskedPrimitive(EoDbPrimitive* primitive, DWORD mask) {
		m_Primitive = primitive;
		m_Mask = mask;
	}
	void ClearMaskBit(int bit) noexcept {
		m_Mask &= ~(1UL << bit);
	}
	DWORD GetMask() noexcept {
		return m_Mask;
	}
	EoDbPrimitive*& GetPrimitive() noexcept {
		return m_Primitive;
	}
	void SetMaskBit(int bit) noexcept {
		m_Mask |= (1UL << bit);
	}
};
