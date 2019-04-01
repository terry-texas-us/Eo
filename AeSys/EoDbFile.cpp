#include "stdafx.h"
#include "AeSysApp.h"

EoDbFile::EoDbFile() {
}
EoDbFile::EoDbFile(const OdString& fileName, UINT openFlags)
	: CFile(fileName, openFlags) {
}
EoDbFile::~EoDbFile() {
}
void EoDbFile::ConstructBlockReferencePrimitiveFromInsertPrimitive(EoDbPrimitive*& primitive) {
}
void EoDbFile::ConstructPointPrimitiveFromTagPrimitive(EoDbPrimitive *&primitive) {
	const EoInt16 ColorIndex = ReadInt16();
	const EoInt16 PointDisplayMode = ReadInt16();
	const OdGePoint3d Point(ReadPoint3d());

	EoDbPoint* PointPrimitive = new EoDbPoint(Point);
	PointPrimitive->SetColorIndex(ColorIndex);
	PointPrimitive->SetPointDisplayMode(PointDisplayMode);
	primitive = PointPrimitive;
}
void EoDbFile::ConstructPolylinePrimitiveFromCSplinePrimitive(EoDbPrimitive*& primitive) {
	const EoInt16 ColorIndex = ReadInt16();
	const EoInt16 LinetypeIndex = ReadInt16();

	Seek(sizeof(EoUInt16), CFile::current);
	const EoUInt16 NumberOfPoints = ReadUInt16();
	Seek(sizeof(EoUInt16), CFile::current);
	Seek(3 * sizeof(double), CFile::current);
	Seek(3 * sizeof(double), CFile::current);
	OdGePoint3dArray Points;
	Points.setLogicalLength(NumberOfPoints);
	for (EoUInt16 n = 0; n < NumberOfPoints; n++) {
		Points[n] = ReadPoint3d();
	}
	EoDbPolyline* Polyline = new EoDbPolyline();
	// <tas="No vertices appended to Polyline"</tas>

	Polyline->SetColorIndex(ColorIndex);
	Polyline->SetLinetypeIndex(LinetypeIndex);
	primitive = Polyline;
}

EoDbPrimitive* EoDbFile::ReadPrimitive() {
	EoDbPrimitive* Primitive = 0;
	switch (ReadUInt16()) {
	case EoDb::kPointPrimitive:
		Primitive = EoDbPoint::ConstructFrom(*this);
		break;
	case EoDb::kInsertPrimitive:
		ConstructBlockReferencePrimitiveFromInsertPrimitive(Primitive);
		break;
	case EoDb::kGroupReferencePrimitive:
		Primitive = EoDbBlockReference::ConstructFrom(*this);
		break;
	case EoDb::kLinePrimitive:
		Primitive = EoDbLine::ConstructFrom(*this);
		break;
	case EoDb::kHatchPrimitive:
		Primitive = EoDbHatch::ConstructFrom(*this);
		break;
	case EoDb::kEllipsePrimitive:
		Primitive = EoDbEllipse::ConstructFrom(*this);
		break;
	case EoDb::kSplinePrimitive:
		Primitive = EoDbSpline::ConstructFrom(*this);
		break;
	case EoDb::kCSplinePrimitive:
		ConstructPolylinePrimitiveFromCSplinePrimitive(Primitive);
		break;
	case EoDb::kPolylinePrimitive:
		Primitive = EoDbPolyline::ConstructFrom(*this);
		break;
	case EoDb::kTextPrimitive:
		Primitive = EoDbText::ConstructFrom(*this);
		break;
	case EoDb::kDimensionPrimitive:
		Primitive = EoDbDimension::ConstructFrom(*this);
		break;
	case EoDb::kTagPrimitive:
		ConstructPointPrimitiveFromTagPrimitive(Primitive);
		break;

	default:
		theApp.WarningMessageBox(IDS_MSG_BAD_PRIM_TYPE);
	}
	return Primitive;
}
void EoDbFile::ReadString(CString& string) {
	string.Empty();
	char c;
	while (Read(&c, 1) == 1) {
		if (c == '\t') return;
		string += c;
	}
}
void EoDbFile::ReadString(OdString& string) {
	string.empty();
	char c;
	while (Read(&c, 1) == 1) {
		if (c == '\t') return;
		string += c;
	}
}
double EoDbFile::ReadDouble() {
	double number;
	Read(&number, sizeof(double));
	return number;
}
EoInt16 EoDbFile::ReadInt16() {
	EoInt16 number;
	Read(&number, sizeof(EoInt16));
	return number;
}
OdGePoint3d EoDbFile::ReadPoint3d() {
	OdGePoint3d Point;
	Read(&Point.x, sizeof(double));
	Read(&Point.y, sizeof(double));
	Read(&Point.z, sizeof(double));
	return Point;
}
EoUInt16 EoDbFile::ReadUInt16() {
	EoUInt16 number;
	Read(&number, sizeof(EoUInt16));
	return number;
}
OdGeVector3d EoDbFile::ReadVector3d() {
	OdGeVector3d Vector;
	Read(&Vector.x, sizeof(double));
	Read(&Vector.y, sizeof(double));
	Read(&Vector.z, sizeof(double));
	return Vector;
}
void EoDbFile::WriteDouble(double number) {
	Write(&number, sizeof(double));
}
void EoDbFile::WriteInt16(EoInt16 number) {
	Write(&number, sizeof(EoInt16));
}
void EoDbFile::WritePoint3d(const OdGePoint3d& point) {
	Write(&point.x, sizeof(double));
	Write(&point.y, sizeof(double));
	Write(&point.z, sizeof(double));
}

void EoDbFile::WriteString(const CString& string) {
	const int NumberOfCharacters = string.GetLength();
	for (int n = 0; n < NumberOfCharacters; n++) {
		const char c = EoByte(string.GetAt(n));
		Write(&c, 1);
	}
	Write("\t", 1);
}
void EoDbFile::WriteString(const OdString& string) {
	const int NumberOfCharacters = string.getLength();
	for (int n = 0; n < NumberOfCharacters; n++) {
		const char c = EoByte(string.getAt(n));
		Write(&c, 1);
	}
	Write("\t", 1);
}
void EoDbFile::WriteUInt16(EoUInt16 number) {
	Write(&number, sizeof(EoUInt16));
}
void EoDbFile::WriteVector3d(const OdGeVector3d& vector) {
	Write(&vector.x, sizeof(double));
	Write(&vector.y, sizeof(double));
	Write(&vector.z, sizeof(double));
}
