#include "stdafx.h"
#include "AeSysApp.h"

EoDbFile::EoDbFile() {
}

EoDbFile::EoDbFile(OdDbDatabasePtr database)
    : m_Database {database} {
}

EoDbFile::EoDbFile(const OdString& fileName, UINT openFlags)
	: CFile(fileName, openFlags) {
}

EoDbFile::~EoDbFile() {
}

void EoDbFile::ConstructBlockReferencePrimitiveFromInsertPrimitive(EoDbPrimitive*& primitive) noexcept {
}

void EoDbFile::ConstructPointPrimitiveFromTagPrimitive(EoDbPrimitive *&primitive) {
	const OdInt16 ColorIndex = ReadInt16();
	const OdInt16 PointDisplayMode = ReadInt16();
	const OdGePoint3d Point(ReadPoint3d());

	EoDbPoint* PointPrimitive = new EoDbPoint(Point);
	PointPrimitive->SetColorIndex(ColorIndex);
	PointPrimitive->SetPointDisplayMode(PointDisplayMode);
	primitive = PointPrimitive;
}

void EoDbFile::ConstructPolylinePrimitiveFromCSplinePrimitive(EoDbPrimitive*& primitive) {
	const OdInt16 ColorIndex = ReadInt16();
	const OdInt16 LinetypeIndex = ReadInt16();

	Seek(sizeof(OdUInt16), CFile::current);
	const OdUInt16 NumberOfPoints = ReadUInt16();
	Seek(sizeof(OdUInt16), CFile::current);
	Seek(3 * sizeof(double), CFile::current);
	Seek(3 * sizeof(double), CFile::current);
	OdGePoint3dArray Points;
	Points.setLogicalLength(NumberOfPoints);
	for (OdUInt16 n = 0; n < NumberOfPoints; n++) {
		Points[n] = ReadPoint3d();
	}
	EoDbPolyline* Polyline = new EoDbPolyline();
	// <tas="No vertices appended to Polyline"</tas>

	Polyline->SetColorIndex(ColorIndex);
	Polyline->SetLinetypeIndex(LinetypeIndex);
	primitive = Polyline;
}

EoDbPrimitive* EoDbFile::ReadPrimitive(OdDbBlockTableRecordPtr blockTable) {
    EoDbPrimitive* Primitive = nullptr;
    auto PrimitiveType {ReadUInt16()};
    switch (PrimitiveType) {
    case kPointPrimitive: {
        OdDbPointPtr Point = EoDbPoint::Create(blockTable, *this);
        Primitive = EoDbPoint::Create(Point);
        break;
    }
    case kInsertPrimitive:
        ConstructBlockReferencePrimitiveFromInsertPrimitive(Primitive);
        break;
    case kGroupReferencePrimitive: {
        OdDbBlockReferencePtr BlockReference = EoDbBlockReference::Create(blockTable, *this);
        Primitive = EoDbBlockReference::Create(BlockReference);
        break;
    }
    case kLinePrimitive: {
        OdDbLinePtr Line = EoDbLine::Create(blockTable, *this);
        Primitive = EoDbLine::Create(Line);
        break;
    }
    case kHatchPrimitive: {
        OdDbHatchPtr Hatch = EoDbHatch::Create(blockTable, *this);
        Primitive = EoDbHatch::Create(Hatch);
        break;
    }
    case kEllipsePrimitive: {
        OdDbEllipsePtr Ellipse = EoDbEllipse::Create(blockTable, *this);
        Primitive = EoDbEllipse::Create(Ellipse);
        break;
    }
    case kSplinePrimitive: {
        OdDbSplinePtr Spline = EoDbSpline::Create(blockTable, *this);
        Primitive = EoDbSpline::Create(Spline);
        break;
    }
    case kCSplinePrimitive:
        ConstructPolylinePrimitiveFromCSplinePrimitive(Primitive);
        break;
    case kPolylinePrimitive: {
        OdDbPolylinePtr Polyline = EoDbPolyline::Create(blockTable, *this);
        Primitive = EoDbPolyline::Create(Polyline);
        break;
    }
    case kTextPrimitive: {
        OdDbTextPtr Text = EoDbText::Create(blockTable, *this);
        Primitive = EoDbText::Create(Text);
        break;
    }
    case kTagPrimitive:
        ConstructPointPrimitiveFromTagPrimitive(Primitive);
        break;
    case kDimensionPrimitive:
        Primitive = EoDbDimension::ConstructFrom(*this);
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

OdInt16 EoDbFile::ReadInt16() {
	OdInt16 number;
	Read(&number, sizeof(OdInt16));
	return number;
}

OdGePoint2d EoDbFile::ReadPoint2d() {
    OdGePoint2d Point;
    Read(&Point.x, sizeof(double));
    Read(&Point.y, sizeof(double));
    return Point;
}

OdGePoint3d EoDbFile::ReadPoint3d() {
    OdGePoint3d Point;
    Read(&Point.x, sizeof(double));
    Read(&Point.y, sizeof(double));
    Read(&Point.z, sizeof(double));
    return Point;
}

OdUInt16 EoDbFile::ReadUInt16() {
	OdUInt16 number;
	Read(&number, sizeof(OdUInt16));
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

void EoDbFile::WriteInt16(OdInt16 number) {
	Write(&number, sizeof(OdInt16));
}

void EoDbFile::WritePoint2d(const OdGePoint2d& point) {
    Write(&point.x, sizeof(double));
    Write(&point.y, sizeof(double));
}

void EoDbFile::WritePoint3d(const OdGePoint3d& point) {
    Write(&point.x, sizeof(double));
    Write(&point.y, sizeof(double));
    Write(&point.z, sizeof(double));
}

void EoDbFile::WriteString(const CString& string) {
	const int NumberOfCharacters = string.GetLength();
	for (int n = 0; n < NumberOfCharacters; n++) {
		const char c = OdUInt8(string.GetAt(n));
		Write(&c, 1);
	}
	Write("\t", 1);
}

void EoDbFile::WriteString(const OdString& string) {
	const int NumberOfCharacters = string.getLength();
	for (int n = 0; n < NumberOfCharacters; n++) {
		const char c = OdUInt8(string.getAt(n));
		Write(&c, 1);
	}
	Write("\t", 1);
}

void EoDbFile::WriteUInt16(OdUInt16 number) {
	Write(&number, sizeof(OdUInt16));
}

void EoDbFile::WriteVector3d(const OdGeVector3d& vector) {
	Write(&vector.x, sizeof(double));
	Write(&vector.y, sizeof(double));
	Write(&vector.z, sizeof(double));
}
