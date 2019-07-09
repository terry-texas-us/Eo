#pragma once

/// <remarks>
/// Vax: the excess 128 exponent .. range is -128 (0x00 - 0x80) to 127 (0xff - 0x80)
/// Intel: the excess 127 exponent .. range is -127 (0x00 - 0x7f) to 128 (0xff - 0x7f)
/// </remarks>
class EoVaxFloat {
	float m_Float {0.0F};
public:
	EoVaxFloat() = default;

	void Convert(const double& intelDouble) noexcept;

	double Convert();
};

class EoVaxPoint3d {
	EoVaxFloat m_X;
	EoVaxFloat m_Y;
	EoVaxFloat m_Z;
public:
	EoVaxPoint3d() = default;

	void Convert(const OdGePoint3d& point) noexcept {
		m_X.Convert(point.x);
		m_Y.Convert(point.y);
		m_Z.Convert(point.z);
	}

	OdGePoint3d Convert() {
		return {m_X.Convert(), m_Y.Convert(), m_Z.Convert()};
	}
};

class EoVaxVector3d {
	EoVaxFloat m_X;
	EoVaxFloat m_Y;
	EoVaxFloat m_Z;
public:
	EoVaxVector3d() = default;

	void Convert(const OdGeVector3d& vector) noexcept {
		m_X.Convert(vector.x);
		m_Y.Convert(vector.y);
		m_Z.Convert(vector.z);
	}

	OdGeVector3d Convert() {
		return {m_X.Convert(), m_Y.Convert(), m_Z.Convert()};
	}
};
