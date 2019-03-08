#include "StdAfx.h"

Section::Section(void) {
}
Section::Section(double width, double depth, long properties) {
	m_Width = width;
	m_Depth = depth;
	m_Properties = properties;
}
Section::~Section(void) {
}
bool Section::operator==(const Section& other) {
	return Identical(other);
}
bool Section::operator!=(const Section& other) {
	return !Identical(other);
}
void Section::operator()(double width, double depth, long properties) {
	m_Width = width;
	m_Depth = depth;
	m_Properties = properties;
}
void Section::SetWidth(double width) {
	m_Width = width;
}
void Section::SetDepth(double depth) {
	m_Depth = depth;
}
double Section::Width(void) const {
	return m_Width;
}
double Section::Depth(void) const {
	return m_Depth;
}
bool Section::Identical(const Section& section) {
	return (m_Width == section.m_Width && m_Depth == section.m_Depth && m_Properties == section.m_Properties) ? true : false;
}
bool Section::IsRectangular() {
	return (m_Properties & Rectangular) == Rectangular;
}
bool Section::IsRound() {
	return (m_Properties & Round) == Round;
}
