#pragma once

class Section {
public:
	static const long Round = 0x0001;
	static const long Oval = 0x0002;
	static const long Rectangular = 0x0004;
	static const long Fixed = 0x0010;

private:
	double m_Width;
	double m_Depth;
	int m_Properties;

public:
	Section(void);
	Section(double width, double depth, long properties) noexcept;
	~Section(void);

	bool operator==(const Section& other) noexcept;
	bool operator!=(const Section& other);
	void operator()(double width, double depth, long properties) noexcept;
	void SetWidth(double width) noexcept;
	void SetDepth(double depth) noexcept;
	double Width(void) const noexcept;
	double Depth(void) const noexcept;
	bool Identical(const Section& section) noexcept;
	bool IsRectangular() noexcept;
	bool IsRound() noexcept;
};
