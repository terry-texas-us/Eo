#include "stdafx.h"

#include "EoDbBlock.h"

EoDbBlock::EoDbBlock() {
	m_wBlkTypFlgs = 0;
}
EoDbBlock::EoDbBlock(OdUInt16 flags, const OdGePoint3d& basePoint) {
	m_wBlkTypFlgs = flags;
	m_BasePoint = basePoint;
}
EoDbBlock::EoDbBlock(OdUInt16 flags, const OdGePoint3d&  basePoint, const OdString& pathName) {
	m_wBlkTypFlgs = flags;
	m_BasePoint = basePoint;
	m_strXRefPathName = pathName;
}

OdGePoint3d	EoDbBlock::BasePoint() const noexcept {
	return m_BasePoint;
}
OdUInt16 EoDbBlock::GetBlkTypFlgs() noexcept {
	return m_wBlkTypFlgs;
}
bool EoDbBlock::HasAttributes() noexcept {
	return (m_wBlkTypFlgs & 2) == 2;
}
bool EoDbBlock::IsAnonymous() noexcept {
	return (m_wBlkTypFlgs & 1) == 1;
}
bool EoDbBlock::IsFromExternalReference() noexcept {
	return (m_wBlkTypFlgs & 4) == 4;
}
void EoDbBlock::SetBlkTypFlgs(OdUInt16 flags) noexcept {
	m_wBlkTypFlgs = flags;
}
void EoDbBlock::SetBasePoint(const OdGePoint3d& basePoint) noexcept {
	m_BasePoint = basePoint;
}
