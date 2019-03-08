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
	Section(double width, double depth, long properties);
	~Section(void);

	bool operator==(const Section& other);
	bool operator!=(const Section& other);
	void operator()(double width, double depth, long properties);
	void SetWidth(double width);
	void SetDepth(double depth);
	double Width(void) const;
	double Depth(void) const;
	bool Identical(const Section& section);
	bool IsRectangular();
	bool IsRound();
};
