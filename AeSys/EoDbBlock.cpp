#include "stdafx.h"
#include "EoDbBlock.h"

EoDbBlock::EoDbBlock(const unsigned short flags, const OdGePoint3d& basePoint) {
	m_TypeFlags = flags;
	m_BasePoint = basePoint;
}

EoDbBlock::EoDbBlock(const unsigned short flags, const OdGePoint3d& basePoint, const OdString& pathName) {
	m_TypeFlags = flags;
	m_BasePoint = basePoint;
	m_strXRefPathName = pathName;
}

OdGePoint3d EoDbBlock::BasePoint() const noexcept {
	return m_BasePoint;
}

unsigned short EoDbBlock::GetTypeFlags() const noexcept {
	return m_TypeFlags;
}

bool EoDbBlock::HasAttributes() const noexcept {
	return (m_TypeFlags & 2) == 2;
}

bool EoDbBlock::IsAnonymous() const noexcept {
	return (m_TypeFlags & 1) == 1;
}

bool EoDbBlock::IsFromExternalReference() const noexcept {
	return (m_TypeFlags & 4) == 4;
}

void EoDbBlock::SetTypeFlags(const unsigned short flags) noexcept {
	m_TypeFlags = flags;
}

void EoDbBlock::SetBasePoint(const OdGePoint3d& basePoint) noexcept {
	m_BasePoint = basePoint;
}
