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
	Section();
	Section(double width, double depth, long properties) noexcept;
	~Section();

	bool operator==(const Section& other) noexcept;
	bool operator!=(const Section& other) noexcept;
	void operator()(double width, double depth, long properties) noexcept;
	void SetWidth(double width) noexcept;
	void SetDepth(double depth) noexcept;
	double Width() const noexcept;
	double Depth() const noexcept;
	bool Identical(const Section& section) noexcept;
	bool IsRectangular() noexcept;
	bool IsRound() noexcept;
};
