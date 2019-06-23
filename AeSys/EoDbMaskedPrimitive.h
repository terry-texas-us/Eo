#pragma once
class EoDbMaskedPrimitive : public CObject {
	EoDbPrimitive* m_Primitive;
	unsigned long m_Mask;
public:
	EoDbMaskedPrimitive() noexcept {
		m_Primitive = static_cast<EoDbPrimitive*>(nullptr);
		m_Mask = 0;
	}

	EoDbMaskedPrimitive(EoDbPrimitive* primitive, const unsigned long mask) {
		m_Primitive = primitive;
		m_Mask = mask;
	}

	void ClearMaskBit(const int bit) noexcept {
		m_Mask &= ~(1UL << bit);
	}

	unsigned long GetMask() noexcept {
		return m_Mask;
	}

	EoDbPrimitive*& GetPrimitive() noexcept {
		return m_Primitive;
	}

	void SetMaskBit(const int bit) noexcept {
		m_Mask |= 1UL << bit;
	}
};
