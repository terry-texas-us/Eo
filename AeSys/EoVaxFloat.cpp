#include "stdafx.h"
#include "EoVaxFloat.h"

void EoVaxFloat::Convert(const double& intelDouble) noexcept {
	auto IntelFloat {float(intelDouble)};
	auto VaxFloat {0.0F};
	if (IntelFloat != 0.0F) {
		const auto Intel {reinterpret_cast<unsigned char*>(&IntelFloat)};
		const auto Vax {reinterpret_cast<unsigned char*>(&VaxFloat)};
		const auto Sign {static_cast<unsigned char>(Intel[3] & 0x80)};
		auto Exponent {static_cast<unsigned char>(Intel[3] << 1 & 0xff)};
		Exponent |= Intel[2] >> 7U;
		if (Exponent > 0xfd) { Exponent = 0xfd; }

		// - 127 + 128 + 1 (to get hidden 1 to the right of the binary point)
		Exponent += 2;
		Vax[1] = static_cast<unsigned char>(Exponent >> 1U);
		Vax[1] |= Sign;
		Vax[0] = static_cast<unsigned char>(Exponent << 7U & 0xff);
		Vax[0] |= Intel[2] & 0x7f;
		Vax[3] = Intel[1];
		Vax[2] = Intel[0];
	}
	m_Float = VaxFloat;
}

double EoVaxFloat::Convert() {
	auto IntelFloat {0.0F};
	const auto Vax {reinterpret_cast<unsigned char*>(&m_Float)};
	const auto Intel {reinterpret_cast<unsigned char*>(&IntelFloat)};
	const auto Sign {static_cast<unsigned char>(Vax[1] & 0x80)};
	auto Exponent = static_cast<unsigned char>(Vax[1] << 1 & 0xff);
	Exponent |= Vax[0] >> 7U;
	if (Exponent == 0) {
		if (Sign != 0) { throw L"EoVaxFloat: Conversion to Intel - Reserve operand fault"; }
	} else if (Exponent == 1) { // this is a valid vax exponent but because the vax places the hidden leading 1 to the right of the binary point we have a problem .. the possible values are 2.94e-39 to 5.88e-39 .. just call it 0.
	} else { // - 128 + 127 - 1 (to get hidden 1 to the left of the binary point)
		Exponent -= 2;
		Intel[3] = static_cast<unsigned char>(Exponent >> 1U);
		Intel[3] |= Sign;
		Intel[2] = static_cast<unsigned char>(Exponent << 7U & 0xff);
		Intel[2] |= Vax[0] & 0x7f;
		Intel[1] = Vax[3];
		Intel[0] = Vax[2];
	}
	return double(IntelFloat);
}
