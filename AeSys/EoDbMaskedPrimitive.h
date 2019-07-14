#pragma once
class EoDbMaskedPrimitive final : public CObject {
	EoDbPrimitive* m_Primitive {nullptr};
	unsigned m_Mask {0};
public:
	EoDbMaskedPrimitive() = default;

	EoDbMaskedPrimitive(EoDbPrimitive* primitive, const unsigned mask)
		: m_Primitive(primitive)
		, m_Mask(mask) { }

	void ClearMaskBit(const unsigned bit) noexcept {
		m_Mask &= ~(1U << bit);
	}

	[[nodiscard]] unsigned GetMask() const noexcept {
		return m_Mask;
	}

	EoDbPrimitive*& GetPrimitive() noexcept {
		return m_Primitive;
	}

	void SetMaskBit(const unsigned bit) noexcept {
		m_Mask |= 1U << bit;
	}
};
