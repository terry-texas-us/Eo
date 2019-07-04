#pragma once
class Section {
public:
	static const long mc_Round = 0x0001;
	static const long mc_Oval = 0x0002;
	static const long mc_Rectangular = 0x0004;
	static const long mc_Fixed = 0x0010;
private:
	double m_Width {0.0};
	double m_Depth {0.0};
	int m_Properties {0};
public:
	Section() = default;

	Section(double width, double depth, long properties) noexcept;

	bool operator==(const Section& other) noexcept;

	bool operator!=(const Section& other) noexcept;

	void operator()(double width, double depth, long properties) noexcept;

	void SetWidth(double width) noexcept;

	void SetDepth(double depth) noexcept;

	[[nodiscard]] double Width() const noexcept;

	[[nodiscard]] double Depth() const noexcept;

	[[nodiscard]] bool Identical(const Section& section) const noexcept;

	[[nodiscard]] bool IsRectangular() const noexcept;

	[[nodiscard]] bool IsRound() const noexcept;
};
