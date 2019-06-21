#pragma once

/// <remarks>
/// Vax: the excess 128 exponent .. range is -128 (0x00 - 0x80) to 127 (0xff - 0x80)
/// MS: the excess 127 exponent .. range is -127 (0x00 - 0x7f) to 128 (0xff - 0x7f)
/// </remarks>
class EoVaxFloat {
	float m_f {0.0f};
public:
	EoVaxFloat() = default;
	void Convert(const double& dMS) noexcept;
	double Convert();
};

class EoVaxPoint3d {
	EoVaxFloat x;
	EoVaxFloat y;
	EoVaxFloat z;
public:
	EoVaxPoint3d() = default;

	void Convert(const OdGePoint3d& point) noexcept {
		x.Convert(point.x);
		y.Convert(point.y);
		z.Convert(point.z);
	}

	OdGePoint3d Convert() {
		return {x.Convert(), y.Convert(), z.Convert()};
	}
};

class EoVaxVector3d {
	EoVaxFloat x;
	EoVaxFloat y;
	EoVaxFloat z;
public:
	EoVaxVector3d() = default;

	void Convert(const OdGeVector3d& vector) noexcept {
		x.Convert(vector.x);
		y.Convert(vector.y);
		z.Convert(vector.z);
	}

	OdGeVector3d Convert() {
		return {x.Convert(), y.Convert(), z.Convert()};
	}
};
