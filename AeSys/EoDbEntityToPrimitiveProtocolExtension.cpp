#include "stdafx.h"
#include "AeSysDoc.h"
#include <DbSpline.h>
#include <DbViewport.h>
#include <DbBlockTableRecord.h>
#include <Db2dPolyline.h>
#include <Db3dPolyline.h>
#include <Db3dPolylineVertex.h>
#include <Db3dSolid.h>
#include <DbAlignedDimension.h>
#include <DbArc.h>
#include <DbAttribute.h>
#include <DbAttributeDefinition.h>
#include <DbCircle.h>
#include <DbEllipse.h>
#include <DbFace.h>
#include <DbLeader.h>
#include <DbMInsertBlock.h>
#include <DbOle2Frame.h>
#include <DbProxyEntity.h>
#include <DbRasterImage.h>
#include <DbRotatedDimension.h>
#include <DbSolid.h>
#include <DbTable.h>
#include <DbTrace.h>
#include <DbUCSTableRecord.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeCurve2d.h>
#include <Ge/GeEllipArc2d.h>
#include <Ge/GeNurbCurve2d.h>
#include <GeometryFromProxy.h>
#include "EoDbCharacterCellDefinition.h"
#include "EoDbBlockReference.h"
#include "EoDbDimension.h"
#include "EoDbEllipse.h"
#include "EoDbLine.h"
#include "EoDbHatch.h"
#include "EoDbPolyline.h"
#include "EoDbSpline.h"
#include "EoDbEntityToPrimitiveProtocolExtension.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/// <remarks>
/// Creates the default implementation of RTTI methods for (EoDbConvertEntityToPrimitive, OdRxObject),
/// empty pseudo-constructor, zero version numbers, zero proxy flags, empty dxf and application names,
/// dwg class name coinciding with the class name, and uses the ODRX_DEFINE_MEMBERS2 macro.
/// To implement the empty pseudo-constructor, it uses the EMPTY_CONSTR substituting macro. 
/// </remarks>
ODRX_NO_CONS_DEFINE_MEMBERS(EoDbConvertEntityToPrimitive, OdRxObject)

void ConvertEntityData(OdDbEntity* entity, EoDbPrimitive* primitive) {
	OdDbDatabasePtr DatabasePtr = entity->database();
	primitive->SetEntityObjectId(entity->objectId());
	const auto Color {entity->color()};
	if (Color.isByBlock()) {
		primitive->SetColorIndex(7); // 7 is used when entity is not in a block. Primitives are never in blocks so use 7.
	} else if (Color.isByLayer()) {
		primitive->SetColorIndex(EoDbPrimitive::COLORINDEX_BYLAYER);
	} else {
		primitive->SetColorIndex(static_cast<short>(Color.colorIndex()));
	}
	const auto Linetype {entity->linetypeId()};
	if (Linetype == DatabasePtr->getLinetypeByBlockId()) {
		primitive->SetLinetypeIndex(EoDbPrimitive::LINETYPE_BYBLOCK);
	} else if (Linetype == DatabasePtr->getLinetypeByLayerId()) {
		primitive->SetLinetypeIndex(EoDbPrimitive::LINETYPE_BYLAYER);
	} else {
		const auto Name {entity->linetype()};
		primitive->SetLinetypeIndex(static_cast<short>(EoDbLinetypeTable::LegacyLinetypeIndex(Name)));
	}
	OdGeExtents3d extents;
	if (eOk == entity->getGeomExtents(extents)) {
	}
	OdGePlane plane;
	auto planarity {OdDb::kNonPlanar};
	entity->getPlane(plane, planarity);
	if (entity->isPlanar()) {
		OdGePoint3d origin;
		OdGeVector3d uAxis;
		OdGeVector3d vAxis;
		plane.get(origin, uAxis, vAxis);
	}
}

void ConvertTextData(OdDbText* text, EoDbGroup* group) {
	const auto TextStyleObjectId {text->textStyle()};
	OdDbTextStyleTableRecordPtr TextStyleTableRecordPtr = TextStyleObjectId.safeOpenObject(OdDb::kForRead);
	OdString FileName;
	if (TextStyleTableRecordPtr->isShapeFile()) {
		FileName = L"Standard";
	} else {
		FileName = TextStyleTableRecordPtr->fileName();
		const auto nExt {FileName.reverseFind('.')};
		if (nExt != -1) {
			if (FileName.mid(nExt).compare(L".shx") == 0) {
				FileName = FileName.left(nExt);
				for (auto n = nExt; n < 8; n++) {
					FileName += '_';
				}
				FileName += L".ttf";
			}
		}
	}
	const auto VerticalAlignment {EoDbText::ConvertVerticalAlignment(text->verticalMode())};
	const auto HorizontalAlignment {EoDbText::ConvertHorizontalAlignment(text->horizontalMode())};
	auto AlignmentPoint {text->position()};
	if (HorizontalAlignment != EoDb::kAlignLeft || VerticalAlignment != EoDb::kAlignBottom) { AlignmentPoint = text->alignmentPoint(); }
	EoDbFontDefinition FontDefinition;
	FontDefinition.SetPrecision(EoDb::kTrueType);
	FontDefinition.SetFontName(static_cast<const wchar_t*>(FileName));
	FontDefinition.SetHorizontalAlignment(HorizontalAlignment);
	FontDefinition.SetVerticalAlignment(VerticalAlignment);
	EoDbCharacterCellDefinition CharacterCellDefinition;
	CharacterCellDefinition.SetHeight(text->height());
	CharacterCellDefinition.SetRotationAngle(text->rotation());
	CharacterCellDefinition.SetWidthFactor(text->widthFactor());
	CharacterCellDefinition.SetObliqueAngle(text->oblique());
	const EoGeReferenceSystem ReferenceSystem(AlignmentPoint, text->normal(), CharacterCellDefinition);
	auto TextPrimitive {new EoDbText()};
	TextPrimitive->SetFontDefinition(FontDefinition);
	TextPrimitive->SetReferenceSystem(ReferenceSystem);
	TextPrimitive->SetText(static_cast<const wchar_t*>(text->textString()));
	ConvertEntityData(text, TextPrimitive);
	group->AddTail(TextPrimitive);
}

void ConvertDimensionData(OdDbDimension* dimension) {
	OdDbBlockTableRecordPtr Block = dimension->dimBlockId().safeOpenObject(OdDb::kForRead);
	if (dimension->getMeasurement() >= 0.0) {
		OdString formattedMeasurement;
		dimension->formatMeasurement(formattedMeasurement, dimension->getMeasurement(), dimension->dimensionText());
	}
	//OdCmColor bgrndTxtColor;
	//unsigned short bgrndTxtFlags = dimension->getBgrndTxtColor(bgrndTxtColor));
}

void ConvertCurveData(OdDbEntity* entity, EoDbPrimitive* primitive) {
	OdDbCurvePtr Curve = entity;
	OdGePoint3d StartPoint;
	if (eOk == Curve->getStartPoint(StartPoint)) {
	}
	OdGePoint3d EndPoint;
	if (eOk == Curve->getEndPoint(EndPoint)) {
	}
	static_cast<EoDbPolyline*>(primitive)->SetClosed(Curve->isClosed());
	double Area;
	if (eOk == Curve->getArea(Area)) {
	}
	ConvertEntityData(entity, primitive);
}

//<summary>This is the default implementation to be attached to OdDbEntity as a catch-all. This guarantees that this protocol extension will be found for any entity, so the search up the OdRxClass tree will not fail and abort.</summary>
void EoDbConvertEntityToPrimitive::Convert(OdDbEntity* entity, EoDbGroup*) {
	TRACE1("Entity %s was not converted ...\n", static_cast<const wchar_t*>(entity->isA()->name()));
}

class EoDb2dPolyline_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDb2dPolylinePtr PolylineEntity = entity;
		auto PolylinePrimitive {new EoDbPolyline()};
		auto Iterator {PolylineEntity->vertexIterator()};
		for (auto i = 0; !Iterator->done(); i++, Iterator->step()) {
			OdDb2dVertexPtr Vertex = Iterator->entity();
			if (Vertex.get()) {
				auto Point {Vertex->position()};
				Point.z = PolylineEntity->elevation();
				PolylinePrimitive->AppendVertex(Vertex->position().convert2d(), Vertex->bulge(), Vertex->startWidth(), Vertex->endWidth());
			}
		}
		PolylinePrimitive->SetClosed(PolylineEntity->isClosed());
		PolylinePrimitive->SetNormal(PolylineEntity->normal());
		PolylinePrimitive->SetElevation(PolylineEntity->elevation());
		ConvertCurveData(PolylineEntity, PolylinePrimitive);
		if (PolylineEntity->polyType() == OdDb::k2dCubicSplinePoly) {
		} else if (PolylineEntity->polyType() == OdDb::k2dQuadSplinePoly) {
		}
		group->AddTail(PolylinePrimitive);
	}
};

class EoDb3dPolyline_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDb3dPolylinePtr PolylineEntity = entity;

		// <tas="No vertices appended to polyline"</tas>
		auto Iterator {PolylineEntity->vertexIterator()};
		for (auto i = 0; !Iterator->done(); i++, Iterator->step()) {
			OdDb3dPolylineVertexPtr Vertex = Iterator->entity();
		}
		auto PolylinePrimitive {new EoDbPolyline()};
		if (PolylineEntity->polyType() == OdDb::k3dCubicSplinePoly) {
		} else if (PolylineEntity->polyType() == OdDb::k3dQuadSplinePoly) {
		}
		PolylinePrimitive->SetClosed(PolylineEntity->isClosed());
		ConvertCurveData(PolylineEntity, PolylinePrimitive);
		group->AddTail(PolylinePrimitive);
	}
};

class EoDbAlignedDimension_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbAlignedDimensionPtr AlignedDimension = entity;
		group->AddTail(EoDbDimension::Create(AlignedDimension));
	}
};

class EoDbArc_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbArcPtr ArcEntity = entity;
		const auto Normal(ArcEntity->normal());
		const auto Center(ArcEntity->center());
		auto StartAngle {ArcEntity->startAngle()};
		auto EndAngle {ArcEntity->endAngle()};
		if (StartAngle >= Oda2PI) { // need to rationalize angles to first period angles in range on (0 to twopi)
			StartAngle -= Oda2PI;
			EndAngle -= Oda2PI;
		}
		auto SweepAngle {EndAngle - StartAngle};
		if (SweepAngle <= FLT_EPSILON) {
			SweepAngle += Oda2PI;
		}
		OdGePoint3d StartPoint;
		ArcEntity->getStartPoint(StartPoint);
		const auto MajorAxis(StartPoint - Center);
		const auto MinorAxis {Normal.crossProduct(MajorAxis)};
		auto ArcPrimitive {new EoDbEllipse()};
		// <tas="Encountered Circular Arc entity with zero radius. Is this valid for dwg files?"</tas>
		if (!MajorAxis.isZeroLength() && !MinorAxis.isZeroLength()) {
			ArcPrimitive->SetTo2(Center, MajorAxis, MinorAxis, SweepAngle);
		}
		ConvertEntityData(ArcEntity, ArcPrimitive);
		group->AddTail(ArcPrimitive);
	}
};

class EoDbAttributeDefinition_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbAttributeDefinitionPtr AttributeDefinitionEntity = entity;
		if (AttributeDefinitionEntity->isConstant() && !AttributeDefinitionEntity->isInvisible()) {
			ConvertTextData(static_cast<OdDbText*>(entity), group);
		}
	}
};

class EoDbBlockReference_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbBlockReferencePtr BlockReferenceEntity = entity;
		OdDbBlockTableRecordPtr BlockTableRecordPtr = BlockReferenceEntity->blockTableRecord().safeOpenObject(OdDb::kForRead);
		auto BlockReferencePrimitive {new EoDbBlockReference()};
		BlockReferencePrimitive->SetName(BlockTableRecordPtr->getName());
		BlockReferencePrimitive->SetPosition(BlockReferenceEntity->position());
		BlockReferencePrimitive->SetNormal(BlockReferenceEntity->normal());
		BlockReferencePrimitive->SetScaleFactors(BlockReferenceEntity->scaleFactors());
		BlockReferencePrimitive->SetRotation(BlockReferenceEntity->rotation());
		ConvertEntityData(BlockReferenceEntity, BlockReferencePrimitive);
		group->AddTail(BlockReferencePrimitive);

		// <tas="Block reference - attributes"</tas>
		auto ObjectIterator {BlockReferenceEntity->attributeIterator()};
		for (auto i = 0; !ObjectIterator->done(); i++, ObjectIterator->step()) {
			OdDbAttributePtr AttributePtr = ObjectIterator->entity();
			if (!AttributePtr.isNull()) {
				if (!AttributePtr->isConstant() && !AttributePtr->isInvisible()) {
					ConvertTextData(static_cast<OdDbText*>(AttributePtr), group);
				}
			}
		}
	}
};

class EoDbCircle_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbCirclePtr CircleEntity = entity;
		const auto CirclePrimitive {new EoDbEllipse(CircleEntity->center(), CircleEntity->normal(), CircleEntity->radius())};
		ConvertEntityData(CircleEntity, CirclePrimitive);
		group->AddTail(CirclePrimitive);
	}
};

class EoDbEllipse_Converter : public EoDbConvertEntityToPrimitive {
public:
	/// <remarks>
	/// Can only properly convert ellipse which is radial (trivial) or non radials which have a start parameter of 0.
	/// </remarks>
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbEllipsePtr Ellipse = entity;
		group->AddTail(EoDbEllipse::Create(Ellipse));
	}
};

class EoDbFace_Converter : public EoDbConvertEntityToPrimitive {
public:
	/// <remarks>Four sided, not necessarily planar, surface. It hides other objects and fills with solid color. No support for individual edge visibility.</remarks>
	/// <tas="Convert Face entity to 2 triangular polygons to ensure planar surface"</tas>
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbFacePtr FaceEntity = entity;
		auto HatchPrimitive {new EoDbHatch};
		OdGePoint3d Vertex;
		for (unsigned short VertexIndex = 0; VertexIndex < 4; VertexIndex++) {
			FaceEntity->getVertexAt(VertexIndex, Vertex);
			HatchPrimitive->Append(Vertex);
		}
		HatchPrimitive->SetInteriorStyle(EoDbHatch::kSolid);
		HatchPrimitive->SetInteriorStyleIndex(0);
		ConvertEntityData(FaceEntity, HatchPrimitive);
		group->AddTail(HatchPrimitive);
	}
};

class EoDbHatch_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		const OdDbHatchPtr Hatch {entity};
		group->AddTail(EoDbHatch::Create(Hatch));
	}
};

class EoDbLeader_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbLeaderPtr LeaderEntity = entity;
		OdRxObjectPtrArray EntitySet;
		LeaderEntity->explode(EntitySet);
		const auto NumberOfEntities {EntitySet.size()};
		for (unsigned i = 0; i < NumberOfEntities; i++) {
			auto Entity {static_cast<OdDbEntityPtr>(EntitySet[i])};
			OdSmartPtr<EoDbConvertEntityToPrimitive> EntityConverter = Entity;
			EntityConverter->Convert(Entity, group);
		}
	}
};

class EoDbLine_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		const OdDbLinePtr Line {entity};
		group->AddTail(EoDbLine::Create(Line));
	}
};

class EoDbMInsertBlock_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbMInsertBlockPtr MInsertBlockEntity = entity;
		OdDbBlockTableRecordPtr BlockTableRecordPtr = MInsertBlockEntity->blockTableRecord().safeOpenObject(OdDb::kForRead);
		auto BlockReferencePrimitive {new EoDbBlockReference()};
		BlockReferencePrimitive->SetName(BlockTableRecordPtr->getName());
		BlockReferencePrimitive->SetPosition(MInsertBlockEntity->position());
		BlockReferencePrimitive->SetNormal(MInsertBlockEntity->normal());
		BlockReferencePrimitive->SetScaleFactors(MInsertBlockEntity->scaleFactors());
		BlockReferencePrimitive->SetRotation(MInsertBlockEntity->rotation());
		BlockReferencePrimitive->SetRows(MInsertBlockEntity->rows());
		BlockReferencePrimitive->SetRowSpacing(MInsertBlockEntity->rowSpacing());
		BlockReferencePrimitive->SetColumns(MInsertBlockEntity->columns());
		BlockReferencePrimitive->SetColumnSpacing(MInsertBlockEntity->columnSpacing());
		ConvertEntityData(MInsertBlockEntity, BlockReferencePrimitive);
		group->AddTail(BlockReferencePrimitive);
	}
};

class EoDbMText_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbMTextPtr MTextEntity = entity;
		group->AddTail(EoDbText::Create(MTextEntity));
	}
};

class EoDbPoint_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbPointPtr PointEntity = entity;
		auto PointPrimitive {new EoDbPoint(PointEntity->position())};
		PointPrimitive->SetPointDisplayMode(PointEntity->database()->getPDMODE());
		ConvertEntityData(PointEntity, PointPrimitive);
		group->AddTail(PointPrimitive);
	}
};

class EoDbPolyline_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		const OdDbPolylinePtr Polyline {entity};
		group->AddTail(EoDbPolyline::Create(Polyline));
	}
};

class EoDbProxyEntity_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbProxyEntityPtr ProxyEntityEntity = entity;
		if (ProxyEntityEntity->graphicsMetafileType() == ProxyEntityEntity->kNoMetafile) {
		} else {
			if (ProxyEntityEntity->graphicsMetafileType() == ProxyEntityEntity->kBoundingBox) {
			} else if (ProxyEntityEntity->graphicsMetafileType() == ProxyEntityEntity->kFullGraphics) {
			}
			OdRxObjectPtrArray EntitySet;
			ProxyEntityEntity->explodeGeometry(EntitySet);
			const auto NumberOfEntities {EntitySet.size()};
			for (unsigned n = 0; n < NumberOfEntities; n++) {
				auto Entity {static_cast<OdDbEntityPtr>(EntitySet[n])};
				OdSmartPtr<EoDbConvertEntityToPrimitive> EntityConverter = Entity;
				EntityConverter->Convert(Entity, group);
			}
		}
		OdAnsiString satString;
	}
};

class EoDbRotatedDimension_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbRotatedDimensionPtr RotatedDimensionEntity = entity;
		OdDbBlockTableRecordPtr Block = RotatedDimensionEntity->dimBlockId().safeOpenObject(OdDb::kForRead);
		ConvertDimensionData(RotatedDimensionEntity);

		// <tas="Improper conversion - entity is used a lot"/>
		auto BlockReferencePrimitive {new EoDbBlockReference()};
		BlockReferencePrimitive->SetName(Block->getName());
		BlockReferencePrimitive->SetPosition(OdGePoint3d::kOrigin);
		BlockReferencePrimitive->SetNormal(OdGeVector3d::kZAxis);
		BlockReferencePrimitive->SetScaleFactors(OdGeScale3d(1.0, 1.0, 1.0));
		BlockReferencePrimitive->SetRotation(0.0);
		ConvertEntityData(RotatedDimensionEntity, BlockReferencePrimitive);
		group->AddTail(BlockReferencePrimitive);
	}
};

class EoDbSolid_Converter : public EoDbConvertEntityToPrimitive {
public:
	/// <remarks>
	/// The first two points define one edge of the polygon. The third point is diagonally opposite the second
	/// If the fourth point coincides with third result is a filled triangle.
	/// else fourth point creates a quadrilateral area.
	/// </remarks>
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbSolidPtr SolidEntity = entity;
		auto HatchPrimitive {new EoDbHatch};
		OdGePoint3d Point;
		SolidEntity->getPointAt(0, Point);
		HatchPrimitive->Append(Point);
		SolidEntity->getPointAt(1, Point);
		HatchPrimitive->Append(Point);
		SolidEntity->getPointAt(2, Point);
		OdGePoint3d EndPoint;
		SolidEntity->getPointAt(3, EndPoint);
		HatchPrimitive->Append(Point == EndPoint ? Point : EndPoint);
		HatchPrimitive->Append(Point);
		HatchPrimitive->SetInteriorStyle(EoDbHatch::kSolid);
		HatchPrimitive->SetInteriorStyleIndex(0);
		ConvertEntityData(SolidEntity, HatchPrimitive);
		group->AddTail(HatchPrimitive);
	}
};

class EoDbSpline_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbSplinePtr SplineEntity = entity;
		int Degree;
		bool Rational;
		bool Closed;
		bool Periodic;
		OdGePoint3dArray ControlPoints;
		OdGeDoubleArray Weights;
		OdGeKnotVector Knots;
		double Tolerance;
		SplineEntity->getNurbsData(Degree, Rational, Closed, Periodic, ControlPoints, Knots, Weights, Tolerance);
		for (unsigned n = 0; n < ControlPoints.size(); n++) {
		}
		for (auto n = 0; n < Knots.length(); n++) {
		}
		if (Rational) {

			for (unsigned n = 0; n < Weights.size(); n++) {
			}
		}
		if (Periodic) {
			// <tas="Only creating non-periodic splines."</tas>
		} else {
			auto SplinePrimitive {new EoDbSpline()};
			SplinePrimitive->Set(Degree, Knots, ControlPoints, Weights, Periodic);
			ConvertCurveData(entity, SplinePrimitive);
			group->AddTail(SplinePrimitive);
		}
	}
};

class EoDbText_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbTextPtr Text = entity;
		group->AddTail(EoDbText::Create(Text));
	}
};

class EoDbTrace_Converter : public EoDbConvertEntityToPrimitive {
public:
	/// <remarks>
	/// A Trace entity is the exact same thing as a Solid entity
	/// </remarks>
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbTracePtr TraceEntity = entity;
		auto HatchPrimitive {new EoDbHatch};
		OdGePoint3d Point;
		TraceEntity->getPointAt(0, Point);
		HatchPrimitive->Append(Point);
		TraceEntity->getPointAt(1, Point);
		HatchPrimitive->Append(Point);
		TraceEntity->getPointAt(2, Point);
		OdGePoint3d EndPoint;
		TraceEntity->getPointAt(3, EndPoint);
		HatchPrimitive->Append(Point == EndPoint ? Point : EndPoint);
		HatchPrimitive->Append(Point);
		HatchPrimitive->SetInteriorStyle(EoDbHatch::kSolid);
		HatchPrimitive->SetInteriorStyleIndex(0);
		ConvertEntityData(TraceEntity, HatchPrimitive);
		group->AddTail(HatchPrimitive);
	}
};

class EoDbViewport_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbViewportPtr ViewportEntity = entity;
		TRACE1("%s was not converted ...\n", static_cast<const wchar_t*>(ViewportEntity->desc()->name()));
		OdDbObjectIdArray layerIds;
		ViewportEntity->getFrozenLayerList(layerIds);
		if (layerIds.length()) {
			for (auto i = 0; i < static_cast<int>(layerIds.length()); i++) {
			}
		} else {
		}
		OdGePoint3d origin;
		OdGeVector3d xAxis;
		OdGeVector3d yAxis;
		ViewportEntity->getUcs(origin, xAxis, yAxis);
		if (!ViewportEntity->nonRectClipEntityId().isNull()) {
		}
		if (!ViewportEntity->ucsName().isNull()) {
			OdDbUCSTableRecordPtr UCS = ViewportEntity->ucsName().safeOpenObject(OdDb::kForRead);
		} else {
		}
		//ConvertEntityData(ViewportEntity, );
	}
};

class Converters {
	OdStaticRxObject<EoDbConvertEntityToPrimitive> m_EntityConverter;
	OdStaticRxObject<EoDb2dPolyline_Converter> m_2dPolylineConverter;
	OdStaticRxObject<EoDb3dPolyline_Converter> m_3dPolylineConverter;
	OdStaticRxObject<EoDbAlignedDimension_Converter> m_AlignedDimensionConverter;
	OdStaticRxObject<EoDbArc_Converter> m_ArcConverter;
	OdStaticRxObject<EoDbAttributeDefinition_Converter> m_AttributeDefinitionConverter;
	OdStaticRxObject<EoDbBlockReference_Converter> m_BlockReference;
	OdStaticRxObject<EoDbCircle_Converter> m_CircleConverter;
	OdStaticRxObject<EoDbEllipse_Converter> m_EllipseConverter;
	OdStaticRxObject<EoDbFace_Converter> m_FaceConverter;
	OdStaticRxObject<EoDbHatch_Converter> m_HatchConverter;
	OdStaticRxObject<EoDbLeader_Converter> m_LeaderConverter;
	OdStaticRxObject<EoDbLine_Converter> m_LineConverter;
	OdStaticRxObject<EoDbMInsertBlock_Converter> m_MInsertBlock;
	OdStaticRxObject<EoDbMText_Converter> m_MTextConverter;
	OdStaticRxObject<EoDbPoint_Converter> m_PointConverter;
	OdStaticRxObject<EoDbPolyline_Converter> m_PolylineConverter;
	OdStaticRxObject<EoDbProxyEntity_Converter> m_ProxyEntityConverter;
	OdStaticRxObject<EoDbRotatedDimension_Converter> m_RotatedDimensionConverter;
	OdStaticRxObject<EoDbSolid_Converter> m_SolidConverter;
	OdStaticRxObject<EoDbSpline_Converter> m_SplineConverter;
	OdStaticRxObject<EoDbText_Converter> m_TextConverter;
	OdStaticRxObject<EoDbTrace_Converter> m_TraceConverter;
	OdStaticRxObject<EoDbViewport_Converter> m_ViewportConverter;
public:
	void AddExtensions() {
		OdDbEntity::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_EntityConverter);
		OdDb2dPolyline::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_2dPolylineConverter);
		OdDb3dPolyline::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_3dPolylineConverter);
		OdDbAlignedDimension::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_AlignedDimensionConverter);
		OdDbArc::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_ArcConverter);
		OdDbAttributeDefinition::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_AttributeDefinitionConverter);
		OdDbBlockReference::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_BlockReference);
		OdDbCircle::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_CircleConverter);
		OdDbEllipse::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_EllipseConverter);
		OdDbFace::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_FaceConverter);
		OdDbHatch::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_HatchConverter);
		OdDbLeader::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_LeaderConverter);
		OdDbLine::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_LineConverter);
		OdDbMInsertBlock::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_MInsertBlock);
		OdDbMText::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_MTextConverter);
		OdDbPoint::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_PointConverter);
		OdDbPolyline::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_PolylineConverter);
		OdDbProxyEntity::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_ProxyEntityConverter);
		OdDbRotatedDimension::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_RotatedDimensionConverter);
		OdDbSolid::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_SolidConverter);
		OdDbSpline::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_SplineConverter);
		OdDbText::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_TextConverter);
		OdDbTrace::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_TraceConverter);
		OdDbViewport::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_ViewportConverter);
	}

	void DeleteExtensions() {
		OdDbEntity::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDb2dPolyline::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDb3dPolyline::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbAlignedDimension::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbArc::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbAttributeDefinition::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbBlockReference::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbCircle::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbEllipse::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbFace::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbHatch::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbLeader::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbLine::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbMInsertBlock::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbMText::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbPoint::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbPolyline::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbProxyEntity::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbRotatedDimension::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbSolid::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbSpline::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbText::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbTrace::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbViewport::desc()->delX(EoDbConvertEntityToPrimitive::desc());
	}
};

AeSysDoc* ConvertEntityToPrimitiveProtocolExtension::m_Document = nullptr;

ConvertEntityToPrimitiveProtocolExtension::ConvertEntityToPrimitiveProtocolExtension(AeSysDoc* document) noexcept
	: m_Converters(nullptr) {
	m_Document = document;
}

ConvertEntityToPrimitiveProtocolExtension::~ConvertEntityToPrimitiveProtocolExtension() {
	if (m_Converters) {
		Uninitialize();
	}
}

void ConvertEntityToPrimitiveProtocolExtension::Initialize() {
	EoDbConvertEntityToPrimitive::rxInit();
	m_Converters = new Converters;
	m_Converters->AddExtensions();
}

void ConvertEntityToPrimitiveProtocolExtension::Uninitialize() {
	m_Converters->DeleteExtensions();
	EoDbConvertEntityToPrimitive::rxUninit();
	delete m_Converters;
	m_Converters = nullptr;
}
