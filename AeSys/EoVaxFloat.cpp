#include "stdafx.h"

EoVaxFloat::EoVaxFloat() {
	m_f = 0.f;
}
void EoVaxFloat::Convert(const double& dMS) {
	float fMS = float(dMS);
	float fVax = 0.f;

	if (fMS != 0.f) {
		EoByte* pMS = (EoByte*) &fMS;
		EoByte* pVax = (EoByte*) &fVax;

		EoByte bSign = EoByte(pMS[3] & 0x80);
		EoByte bExp = EoByte((pMS[3] << 1) & 0xff);
		bExp |= pMS[2] >> 7;

		if (bExp > 0xfd)
			bExp = 0xfd;

		// - 127 + 128 + 1 (to get hidden 1 to the right of the binary point)
		bExp += 2;

		pVax[1] = EoByte(bExp >> 1);
		pVax[1] |= bSign;

		pVax[0] = EoByte((bExp << 7) & 0xff);
		pVax[0] |= pMS[2] & 0x7f;

		pVax[3] = pMS[1];
		pVax[2] = pMS[0];
	}
	m_f = fVax;
}
double EoVaxFloat::Convert() {
	float fMS = 0.f;

	EoByte* pvax = (EoByte*) &m_f;
	EoByte* pms = (EoByte*) &fMS;

	EoByte bSign = EoByte(pvax[1] & 0x80);
	EoByte bExp = EoByte((pvax[1] << 1) & 0xff);
	bExp |= pvax[0] >> 7;

	if (bExp == 0) {
		if (bSign != 0) {
			throw L"EoVaxFloat: Conversion to MS - Reserve operand fault";
		}
	}
	else if (bExp == 1) { // this is a valid vax exponent but because the vax places the hidden
		// leading 1 to the right of the binary point we have a problem ..
		// the possible values are 2.94e-39 to 5.88e-39 .. just call it 0.
	}
	else { // - 128 + 127 - 1 (to get hidden 1 to the left of the binary point)
		bExp -= 2;

		pms[3] = EoByte(bExp >> 1);
		pms[3] |= bSign;

		pms[2] = EoByte((bExp << 7) & 0xff);
		pms[2] |= pvax[0] & 0x7f;

		pms[1] = pvax[3];
		pms[0] = pvax[2];
	}
	return (double(fMS));
}
