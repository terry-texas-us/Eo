#include "stdafx.h"
#include "EoVaxFloat.h"

void EoVaxFloat::Convert(const double& dMS) noexcept {
	auto fMS {float(dMS)};
	auto fVax {0.f};
	if (fMS != 0.f) {
		auto pMS = reinterpret_cast<unsigned char*>(& fMS);
		auto pVax = reinterpret_cast<unsigned char*>(& fVax);
		const auto bSign {static_cast<unsigned char>(pMS[3] & 0x80)};
		auto bExp {static_cast<unsigned char>(pMS[3] << 1 & 0xff)};
		bExp |= pMS[2] >> 7;
		if (bExp > 0xfd) { bExp = 0xfd; }

		// - 127 + 128 + 1 (to get hidden 1 to the right of the binary point)
		bExp += 2;
		pVax[1] = static_cast<unsigned char>(bExp >> 1);
		pVax[1] |= bSign;
		pVax[0] = static_cast<unsigned char>(bExp << 7 & 0xff);
		pVax[0] |= pMS[2] & 0x7f;
		pVax[3] = pMS[1];
		pVax[2] = pMS[0];
	}
	m_f = fVax;
}

double EoVaxFloat::Convert() {
	auto fMS {0.f};
	auto pvax = reinterpret_cast<unsigned char*>(&m_f);
	auto pms = reinterpret_cast<unsigned char*>(&fMS);
	const auto bSign {static_cast<unsigned char>(pvax[1] & 0x80)};
	auto bExp = static_cast<unsigned char>(pvax[1] << 1 & 0xff);
	bExp |= pvax[0] >> 7;
	if (bExp == 0) {
		if (bSign != 0) { throw L"EoVaxFloat: Conversion to MS - Reserve operand fault"; }

	} else if (bExp == 1) { // this is a valid vax exponent but because the vax places the hidden
		// leading 1 to the right of the binary point we have a problem ..
		// the possible values are 2.94e-39 to 5.88e-39 .. just call it 0.
	} else { // - 128 + 127 - 1 (to get hidden 1 to the left of the binary point)
		bExp -= 2;
		pms[3] = static_cast<unsigned char>(bExp >> 1);
		pms[3] |= bSign;
		pms[2] = static_cast<unsigned char>(bExp << 7 & 0xff);
		pms[2] |= pvax[0] & 0x7f;
		pms[1] = pvax[3];
		pms[0] = pvax[2];
	}
	return double(fMS);
}
