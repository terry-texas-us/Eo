#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "EoDbJobFile.h"

EoDbJobFile::EoDbJobFile() {
	m_Version = 3;
	m_PrimBuf = new EoByte[EoDbPrimitive::BUFFER_SIZE];
}
EoDbJobFile::~EoDbJobFile() {
	delete [] m_PrimBuf;
}

void EoDbJobFile::ConstructPrimitive(EoDbPrimitive *&primitive, EoInt16 PrimitiveType) {
	switch(PrimitiveType) {
	case EoDb::kTagPrimitive:
	case EoDb::kPointPrimitive:
		if (PrimitiveType == kTagPrimitive) {
			*((EoUInt16*) &m_PrimBuf[4]) = EoDb::kPointPrimitive;
			::ZeroMemory(&m_PrimBuf[20], 12);
		}
		primitive = EoDbPoint::ConstructFrom(m_PrimBuf, 3);
		break;
	case EoDb::kLinePrimitive:
		primitive = EoDbLine::ConstructFrom(m_PrimBuf, 3);
		break;
	case EoDb::kHatchPrimitive:
		primitive = EoDbHatch::ConstructFrom(m_PrimBuf, 3);
		break;
	case EoDb::kEllipsePrimitive:
		primitive = EoDbEllipse::ConstructFrom(m_PrimBuf, 3);
		break;
	case EoDb::kCSplinePrimitive:
	case EoDb::kSplinePrimitive:
		if (PrimitiveType == kCSplinePrimitive) {
			EoUInt16 NumberOfControlPoints = *((EoUInt16*) &m_PrimBuf[10]);
			m_PrimBuf[3] = EoSbyte((2 + NumberOfControlPoints * 3) / 8 + 1);
			*((EoUInt16*) &m_PrimBuf[4]) = EoUInt16(EoDb::kSplinePrimitive);
			m_PrimBuf[8] = m_PrimBuf[10];
			m_PrimBuf[9] = m_PrimBuf[11];
			::MoveMemory(&m_PrimBuf[10], &m_PrimBuf[38], NumberOfControlPoints * 3 * sizeof(EoVaxFloat));
		}
		primitive = EoDbSpline::ConstructFrom(m_PrimBuf, 3);
		break;
	case EoDb::kTextPrimitive:
		primitive = EoDbText::ConstructFrom(m_PrimBuf, 3);
		static_cast<EoDbText*>(primitive)->ConvertFormattingCharacters();
		break;
	case EoDb::kDimensionPrimitive:
		primitive = EoDbDimension::ConstructFrom(m_PrimBuf, 3);
		break;

	default:
		ConstructPrimitiveFromVersion1(primitive);
	}
}
void EoDbJobFile::ConstructPrimitiveFromVersion1(EoDbPrimitive *&primitive) {
	switch (m_PrimBuf[5]) {
	case 17:
		primitive = EoDbText::ConstructFrom(m_PrimBuf, 1);
		static_cast<EoDbText*>(primitive)->ConvertFormattingCharacters();
		break;
	case 24:
		primitive = EoDbSpline::ConstructFrom(m_PrimBuf, 1);
		break;
	case 33:
		break;
	case 61:
		primitive = EoDbEllipse::ConstructFrom(m_PrimBuf, 1);
		break;
	case 67:
		primitive = EoDbLine::ConstructFrom(m_PrimBuf, 1);
		break;
	case 70:
		primitive = EoDbPoint::ConstructFrom(m_PrimBuf, 1);
		break;
	case 100:
		primitive = EoDbHatch::ConstructFrom(m_PrimBuf, 1);
		break;

	default:
		throw L"Exception.FileJob: Invalid primitive type.";
	}
}
bool EoDbJobFile::GetNextPrimitive(CFile& file, EoDbPrimitive*& primitive) {
	EoInt16 PrimitiveType = 0;
	do {
		if (!ReadNextPrimitive(file, m_PrimBuf, PrimitiveType)) {
			return false;
		}
	}
	while (PrimitiveType <= 0);
	ConstructPrimitive(primitive, PrimitiveType);
	return true;
}
bool EoDbJobFile::GetNextVisibleGroup(CFile& file, EoDbGroup*& group) {
	ULONGLONG Position = file.GetPosition();

	group = 0;
	try {
		EoDbPrimitive*	Primitive;
		if (!GetNextPrimitive(file, Primitive)) {
			return false;
		}
		group = new EoDbGroup;
		group->AddTail(Primitive);
		EoUInt16 wPrims = *((EoUInt16*) ((m_Version == 1) ? &m_PrimBuf[2] : &m_PrimBuf[1]));
		for (EoUInt16 w = 1; w < wPrims; w++) {
			try {
				Position = file.GetPosition();
				if (!GetNextPrimitive(file, Primitive))
					throw L"Exception.FileJob: Unexpected end of file.";
				group->AddTail(Primitive);
			}
			catch (LPWSTR szMessage) {
				theApp.AddStringToMessageList(szMessage);
				file.Seek(Position + 32, CFile::begin);
			}
		}
	}
	catch (LPWSTR szMessage) {
		if (Position >= 96) {
			if (::MessageBoxW(0, szMessage, 0, MB_ICONERROR | MB_RETRYCANCEL) == IDCANCEL)
				return false;
		}
		file.Seek(Position + 32, CFile::begin);
	}
	return true;
}
void EoDbJobFile::ReadHeader(CFile& file) {
	if (file.Read(m_PrimBuf, 32) == 32) {
		m_Version = Version();

		if (m_Version == 1) {
			file.SeekToBegin();
		}
		else {
			if (file.GetLength() >= 96) {
				file.Seek(96, CFile::begin);
			}
		}
	}
}
void EoDbJobFile::ReadLayer(CFile& file, EoDbLayer* layer) {
	EoDbGroup* Group;

	while (GetNextVisibleGroup(file, Group)) {
		if (Group != 0) {
			layer->AddTail(Group);
		}
	}
}
void EoDbJobFile::ReadMemFile(CFile& file) {
	AeSysDoc* Document = AeSysDoc::GetDoc();

	Document->RemoveAllTrappedGroups();

	EoDbGroup* Group;
	while (GetNextVisibleGroup(file, Group)) {
		Document->AddWorkLayerGroup(Group);
		Document->AddGroupToTrap(Group);
	}
}
bool EoDbJobFile::ReadNextPrimitive(CFile &file, EoByte *buffer, EoInt16& primitiveType) {
	if (file.Read(buffer, 32) < 32) {
		return false;
	}
	primitiveType = *((EoInt16*) &buffer[4]);

	if (!IsValidPrimitive(primitiveType)) {
		throw L"Exception.FileJob: Invalid primitive type.";
	}
	int LengthInChunks = (m_Version == 1) ? buffer[6] : buffer[3];
	if (LengthInChunks > 1) {
		UINT BytesRemaining = (LengthInChunks - 1) * 32;

		if (BytesRemaining >= EoDbPrimitive::BUFFER_SIZE - 32) {
			throw L"Exception.FileJob: Primitive buffer overflow.";
		}
		if (file.Read(&buffer[32], BytesRemaining) < BytesRemaining) {
			throw L"Exception.FileJob: Unexpected end of file.";
		}
	}
	return true;
}
int EoDbJobFile::Version() {
	switch (m_PrimBuf[5]) {
	case 17: // 0x11 text
	case 24: // 0x18 bspline
	case 33: // 0x21 conic
	case 61: // 0x3D arc
	case 67: // 0x43 line
	case 70: // 0x46 point
	case 100:// 0x64 polygon
		m_Version = 1;
		break;

	default:
		m_Version = 3;
	}
	return (m_Version);
}
bool EoDbJobFile::IsValidPrimitive(EoInt16 primitiveType) {
	switch (primitiveType) {
	case EoDb::kPointPrimitive: // 0x0100
	case EoDb::kLinePrimitive: // 0x0200
	case EoDb::kHatchPrimitive: // 0x0400
	case EoDb::kEllipsePrimitive: // 0x1003
	case EoDb::kSplinePrimitive: // 0x2000
	case EoDb::kCSplinePrimitive: // 0x2001
	case EoDb::kTextPrimitive: // 0x4000
	case EoDb::kTagPrimitive: // 0x4100
	case EoDb::kDimensionPrimitive: // 0x4200
		return true;

	default:
		return IsValidVersion1Primitive(primitiveType);
	}
}
bool EoDbJobFile::IsValidVersion1Primitive(EoInt16 primitiveType) {
	EoByte* PrimitiveType = (EoByte*) &primitiveType;
	switch (PrimitiveType[1]) {
	case 17: // 0x11 text
	case 24: // 0x18 bspline
	case 33: // 0x21 conic
	case 61: // 0x3d arc
	case 67: // 0x43 line
	case 70: // 0x46 point
	case 100:// 0x64 polygon
		return true;

	default:
		return false;
	}
}
void EoDbJobFile::WriteGroup(CFile& file, EoDbGroup* group) {
	m_PrimBuf[0] = 0;
	*((EoUInt16*) &m_PrimBuf[1]) = EoUInt16(group->GetCount());

	POSITION Position = group->GetHeadPosition();
	while (Position != 0) {
		EoDbPrimitive* Primitive = group->GetNext(Position);
		Primitive->Write(file, m_PrimBuf);
	}
}
void EoDbJobFile::WriteHeader(CFile& file) {
	::ZeroMemory(m_PrimBuf, 96);
	m_PrimBuf[4] = 'T';
	m_PrimBuf[5] = 'c';
	file.Write(m_PrimBuf, 96);
}
void EoDbJobFile::WriteLayer(CFile& file, EoDbLayer* layer) {
	layer->BreakSegRefs();
	layer->BreakPolylines();

	POSITION Position = layer->GetHeadPosition();
	while (Position != 0) {
		EoDbGroup* Group = layer->GetNext(Position);
		WriteGroup(file, Group);
	}
}
