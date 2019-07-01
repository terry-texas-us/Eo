#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "EoVaxFloat.h"
#include "EoDbJobFile.h"
#include "EoDbDimension.h"
#include "EoDbEllipse.h"
#include "EoDbLine.h"
#include "EoDbHatch.h"
#include "EoDbSpline.h"

EoDbJobFile::EoDbJobFile() {
	m_Version = 3;
	m_PrimBuf = new unsigned char[EoDbPrimitive::mc_BufferSize];
}

EoDbJobFile::~EoDbJobFile() {
	delete[] m_PrimBuf;
}

void EoDbJobFile::ConstructPrimitive(const OdDbBlockTableRecordPtr& blockTableRecord, EoDbPrimitive*& primitive, const short primitiveType) {
	switch (primitiveType) {
		case EoDb::kTagPrimitive: case EoDb::kPointPrimitive: {
			if (primitiveType == EoDb::kTagPrimitive) {
				*reinterpret_cast<unsigned short*>(& m_PrimBuf[4]) = EoDb::kPointPrimitive;
				::ZeroMemory(&m_PrimBuf[20], 12);
			}
			auto Point {EoDbPoint::Create(blockTableRecord, m_PrimBuf, 3)};
			primitive = EoDbPoint::Create(Point);
			break;
		}
		case EoDb::kLinePrimitive: {
			const auto Line {EoDbLine::Create(blockTableRecord, m_PrimBuf, 3)};
			primitive = EoDbLine::Create(Line);
			break;
		}
		case EoDb::kHatchPrimitive: {
			const auto Hatch {EoDbHatch::Create(blockTableRecord, m_PrimBuf, 3)};
			primitive = EoDbHatch::Create(Hatch);
			break;
		}
		case EoDb::kEllipsePrimitive: {
			auto Ellipse {EoDbEllipse::Create(blockTableRecord, m_PrimBuf, 3)};
			primitive = EoDbEllipse::Create(Ellipse);
			break;
		}
		case EoDb::kCSplinePrimitive: case EoDb::kSplinePrimitive: {
			if (primitiveType == EoDb::kCSplinePrimitive) {
				const auto NumberOfControlPoints {*reinterpret_cast<unsigned short*>(&m_PrimBuf[10])};
				m_PrimBuf[3] = static_cast<unsigned char>((2 + NumberOfControlPoints * 3) / 8 + 1);
				*reinterpret_cast<unsigned short*>(&m_PrimBuf[4]) = static_cast<unsigned short>(EoDb::kSplinePrimitive);
				m_PrimBuf[8] = m_PrimBuf[10];
				m_PrimBuf[9] = m_PrimBuf[11];
				::MoveMemory(&m_PrimBuf[10], &m_PrimBuf[38], NumberOfControlPoints * 3 * sizeof(EoVaxFloat));
			}
			auto Spline {EoDbSpline::Create(blockTableRecord, m_PrimBuf, 3)};
			primitive = EoDbSpline::Create(Spline);
			break;
		}
		case EoDb::kTextPrimitive: {
			auto Text {EoDbText::Create(blockTableRecord, m_PrimBuf, 3)};
			primitive = EoDbText::Create(Text);
			break;
		}
		case EoDb::kDimensionPrimitive: {
			auto AlignedDimension {EoDbDimension::Create(blockTableRecord, m_PrimBuf, 3)};
			primitive = EoDbDimension::Create(AlignedDimension);
			break;
		}
		default:
			ConstructPrimitiveFromVersion1(blockTableRecord, primitive);
	}
}

void EoDbJobFile::ConstructPrimitiveFromVersion1(const OdDbBlockTableRecordPtr& blockTableRecord, EoDbPrimitive*& primitive) {
	switch (m_PrimBuf[5]) {
		case 17: {
			auto Text {EoDbText::Create(blockTableRecord, m_PrimBuf, 1)};
			primitive = EoDbText::Create(Text);
			break;
		}
		case 24: {
			auto Spline {EoDbSpline::Create(blockTableRecord, m_PrimBuf, 1)};
			primitive = EoDbSpline::Create(Spline);
			break;
		}
		case 33:
			break; // Conic primitive
		case 61: {
			auto Ellipse {EoDbEllipse::Create(blockTableRecord, m_PrimBuf, 1)};
			primitive = EoDbEllipse::Create(Ellipse);
			break;
		}
		case 67: {
			const auto Line {EoDbLine::Create(blockTableRecord, m_PrimBuf, 1)};
			primitive = EoDbLine::Create(Line);
			break;
		}
		case 70: {
			auto Point {EoDbPoint::Create(blockTableRecord, m_PrimBuf, 1)};
			primitive = EoDbPoint::Create(Point);
			break;
		}
		case 100: {
			const auto Hatch {EoDbHatch::Create(blockTableRecord, m_PrimBuf, 1)};
			primitive = EoDbHatch::Create(Hatch);
			break;
		}
		default:
			throw L"Exception.FileJob: Invalid primitive type.";
	}
}

bool EoDbJobFile::GetNextPrimitive(OdDbBlockTableRecordPtr blockTableRecord, CFile& file, EoDbPrimitive*& primitive) {
	short PrimitiveType = 0;
	do {
		if (!ReadNextPrimitive(file, m_PrimBuf, PrimitiveType)) {
			return false;
		}
	} while (PrimitiveType <= 0);
	ConstructPrimitive(blockTableRecord, primitive, PrimitiveType);
	return true;
}

bool EoDbJobFile::GetNextVisibleGroup(OdDbBlockTableRecordPtr blockTableRecord, CFile& file, EoDbGroup*& group) {
	auto Position {file.GetPosition()};
	group = nullptr;
	try {
		EoDbPrimitive* Primitive;
		if (!GetNextPrimitive(blockTableRecord, file, Primitive)) { return false; }
		group = new EoDbGroup;
		group->AddTail(Primitive);
		const auto NumberOfPrimitives {*reinterpret_cast<unsigned short*>(m_Version == 1 ? &m_PrimBuf[2] : &m_PrimBuf[1])};
		for (unsigned w = 1; w < NumberOfPrimitives; w++) {
			try {
				Position = file.GetPosition();
				if (!GetNextPrimitive(blockTableRecord, file, Primitive)) { throw L"Exception.FileJob: Unexpected end of file."; }
				group->AddTail(Primitive);
			} catch (const wchar_t* Message) {
				AeSys::AddStringToMessageList(Message);
				file.Seek(static_cast<long long>(Position + 32), CFile::begin);
			}
		}
	} catch (const wchar_t* Message) {

		if (Position >= 96) {

			if (MessageBoxW(nullptr, Message, nullptr, MB_ICONERROR | MB_RETRYCANCEL) == IDCANCEL) { return false; }
		}
		file.Seek(static_cast<long long>(Position + 32), CFile::begin);
	}
	return true;
}

void EoDbJobFile::ReadHeader(CFile& file) {
	if (file.Read(m_PrimBuf, 32) == 32) {
		m_Version = Version();
		if (m_Version == 1) {
			file.SeekToBegin();
		} else {
			if (file.GetLength() >= 96) {
				file.Seek(96, CFile::begin);
			}
		}
	}
}

void EoDbJobFile::ReadLayer(OdDbBlockTableRecordPtr blockTableRecord, CFile& file, EoDbLayer* layer) {
	EoDbGroup* Group;
	while (GetNextVisibleGroup(blockTableRecord, file, Group)) {
		if (Group != nullptr) { layer->AddTail(Group); }
	}
}

void EoDbJobFile::ReadMemFile(OdDbBlockTableRecordPtr blockTableRecord, CFile& file) {
	auto Document {AeSysDoc::GetDoc()};
	Document->RemoveAllTrappedGroups();
	EoDbGroup* Group;
	while (GetNextVisibleGroup(blockTableRecord, file, Group)) {
		Document->AddWorkLayerGroup(Group);
		Document->AddGroupToTrap(Group);
	}
}

bool EoDbJobFile::ReadNextPrimitive(CFile& file, unsigned char* buffer, short& primitiveType) {
	if (file.Read(buffer, 32) < 32) { return false; }
	primitiveType = *reinterpret_cast<short*>(& buffer[4]);
	if (!IsValidPrimitive(primitiveType)) { throw L"Exception.FileJob: Invalid primitive type."; }
	const unsigned LengthInChunks = m_Version == 1 ? buffer[6] : buffer[3];
	if (LengthInChunks > 1) {
		const auto BytesRemaining {(LengthInChunks - 1) * 32};
		if (BytesRemaining >= EoDbPrimitive::mc_BufferSize - 32) { throw L"Exception.FileJob: Primitive buffer overflow."; }
		if (file.Read(&buffer[32], BytesRemaining) < BytesRemaining) { throw L"Exception.FileJob: Unexpected end of file."; }
	}
	return true;
}

int EoDbJobFile::Version() noexcept {
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
	return m_Version;
}

bool EoDbJobFile::IsValidPrimitive(const short primitiveType) noexcept {
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

bool EoDbJobFile::IsValidVersion1Primitive(short primitiveType) noexcept {
	const unsigned char* PrimitiveType = reinterpret_cast<unsigned char*>(& primitiveType);
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
	*reinterpret_cast<unsigned short*>(& m_PrimBuf[1]) = static_cast<unsigned short>(group->GetCount());
	auto Position {group->GetHeadPosition()};
	while (Position != nullptr) {
		const auto Primitive {group->GetNext(Position)};
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
	auto Position {layer->GetHeadPosition()};
	while (Position != nullptr) {
		const auto Group {layer->GetNext(Position)};
		WriteGroup(file, Group);
	}
}

void EoDbJobFile::ConvertFormattingCharacters(OdString& textString) noexcept {
	for (auto i = 0; i < textString.getLength() - 1; i++) {
		if (textString[i] == '^') {
			if (textString[i + 1] == '/') { // Fractions
				const auto EndCaret {textString.find('^', i + 1)};
				if (EndCaret != -1) {
					const auto FractionBar {textString.find('/', i + 2)};
					if (FractionBar != -1 && FractionBar < EndCaret) {
						textString.setAt(i++, '\\');
						textString.setAt(i, 'S');
						textString.setAt(EndCaret, ';');
						i = EndCaret;
					}
				}
			}
		}
	}
}
