#pragma once

class EoDbMaskedPrimitive : public CObject {
private:
	EoDbPrimitive* m_Primitive;
	unsigned long m_Mask;

public:
	EoDbMaskedPrimitive() noexcept {
		m_Primitive = (EoDbPrimitive*) nullptr;
		m_Mask = 0;
	}

	EoDbMaskedPrimitive(EoDbPrimitive* primitive, unsigned long mask) {
		m_Primitive = primitive;
		m_Mask = mask;
	}

	void ClearMaskBit(int bit) noexcept {
		m_Mask &= ~(1UL << bit);
	}

	unsigned long GetMask() noexcept {
		return m_Mask;
	}

	EoDbPrimitive*& GetPrimitive() noexcept {
		return m_Primitive;
	}

	void SetMaskBit(int bit) noexcept {
		m_Mask |= (1UL << bit);
	}
};
