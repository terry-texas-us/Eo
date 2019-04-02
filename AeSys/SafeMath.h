#pragma once

inline double EoArcLength(const double angleInDegrees) noexcept {
	return angleInDegrees * (PI / 180.);
}
template<class T> T EoMin(T a, T b) noexcept {
	return (a < b) ? a : b;
}
template<class T> T EoMax(T a, T b) noexcept {
	return (a > b) ? a : b;
}
inline double EoSignTransfer(const double a, const double b) noexcept {
	return (b >= 0. ? fabs(a) : - fabs(a));
}
inline double EoToDegree(const double angleInRadians) noexcept {
	return (angleInRadians / PI * 180.);
}
inline double EoToRadian(const double angleInDegrees) noexcept {
	return (angleInDegrees * RADIAN);
}
inline int EoRound(const double number) noexcept { // closest integer
	return  (int) (number + 0.5);
}
inline double EoRound(const double number, int precision) {
	precision = (number >= 1.) ? precision - int(log10(number)) - 1 : precision;
	CString FormatSpecification;
	FormatSpecification.Format(L"%%16.%if", precision);
	CString NumberAsString;
	NumberAsString.Format(FormatSpecification, number);
	return _wtof(NumberAsString);
}
/// <summary>Extracts the sign of a real number.</summary>
inline int EoSignum(const double number) noexcept {
	return (number < 0. ? - 1 : (number > 0. ? 1 : 0));
}
