#include "stdafx.h"
#include "AeSysDoc.h"

#include "RxObjectImpl.h"
#include "Db2LineAngularDimension.h"
#include "Db2dPolyline.h"
#include "Db3dPolyline.h"
#include "Db3dPolylineVertex.h"
#include "Db3dSolid.h"
#include "Db3PointAngularDimension.h"
#include "DbAlignedDimension.h"
#include "DbArc.h"
#include "DbArcAlignedText.h"
#include "DbArcDimension.h"
#include "DbAttribute.h"
#include "DbAttributeDefinition.h"
#include "DbBody.h"
#include "DbCircle.h"
#include "DbDiametricDimension.h"
#include "DbEllipse.h"
#include "DbFace.h"
#include "DbFaceRecord.h"
#include "DbFcf.h"
#include "DbLeader.h"
#include "DbMInsertBlock.h"
#include "DbMline.h"
#include "DbOle2Frame.h"
#include "DbOrdinateDimension.h"
#include "DbPolyFaceMesh.h"
#include "DbPolyFaceMeshVertex.h"
#include "DbPolygonMesh.h"
#include "DbPolygonMeshVertex.h"
#include "DbProxyEntity.h"
#include "DbRadialDimension.h"
#include "DbRasterImage.h"
#include "DbRay.h"
#include "DbRegion.h"
#include "DbRotatedDimension.h"
#include "DbShape.h"
#include "DbSolid.h"
#include "DbTable.h"
#include "DbTrace.h"
#include "DbUCSTableRecord.h"
#include "DbWipeout.h"
#include "DbXline.h"
#include "Ge/GeCircArc2d.h"
#include "Ge/GeCircArc3d.h"
#include "Ge/GeCurve2d.h"
#include "Ge/GeEllipArc2d.h"
#include "Ge/GeNurbCurve2d.h"
#include "GeometryFromProxy.h"
#include "Gs/Gs.h"

#include "StaticRxObject.h"

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

	OdCmColor Color = entity->color();

	if (Color.isByBlock()) {
		primitive->SetColorIndex_(7); // 7 is used when entity is not in a block. Primitives are never in blocks so use 7.
	}
	else if (Color.isByLayer()) {
		primitive->SetColorIndex_(EoDbPrimitive::COLORINDEX_BYLAYER);
	}
	else {
		primitive->SetColorIndex_(Color.colorIndex());
	}
	const OdDbObjectId Linetype = entity->linetypeId();

	if (Linetype == DatabasePtr->getLinetypeByBlockId()) {
		primitive->SetLinetypeIndex_(EoDbPrimitive::LINETYPE_BYBLOCK);
	}
	else if (Linetype == DatabasePtr->getLinetypeByLayerId()) {
		primitive->SetLinetypeIndex_(EoDbPrimitive::LINETYPE_BYLAYER);
	}
	else {
		OdString Name = entity->linetype();
		primitive->SetLinetypeIndex_(EoDbLinetypeTable::LegacyLinetypeIndex(Name));
	}

	OdGeExtents3d extents;
	if (eOk == entity->getGeomExtents(extents)) {
	}

	OdGePlane plane;
	OdDb::Planarity planarity = OdDb::kNonPlanar;
	entity->getPlane(plane, planarity);
	if (entity->isPlanar()) {
		OdGePoint3d origin;
		OdGeVector3d uAxis;
		OdGeVector3d vAxis;
		plane.get(origin, uAxis, vAxis);
	}
}

void ConvertTextData(OdDbText* text, EoDbGroup* group) {
	ATLTRACE2(atlTraceGeneral, 1, L"Converting %s to EoDbText ...\n", (PCTSTR) text->desc()->name());

	const OdDbObjectId TextStyleObjectId = text->textStyle();
	OdDbTextStyleTableRecordPtr TextStyleTableRecordPtr = TextStyleObjectId.safeOpenObject(OdDb::kForRead);

	OdString FileName;
	if (TextStyleTableRecordPtr->isShapeFile()) {
		FileName = L"Standard";
	}
	else {
		FileName = TextStyleTableRecordPtr->fileName();
		const int nExt = FileName.reverseFind('.');
		if (nExt != - 1) {
			if (FileName.mid(nExt).compare(L".shx") == 0) {
				FileName = FileName.left(nExt);
				for (int n = nExt; n < 8; n++) {
					FileName += '_';
				}
				FileName += L".ttf";
			}
		}
	}
	const VerticalAlignment VerticalAlignment = EoDbText::ConvertVerticalAlignment(text->verticalMode());
	const HorizontalAlignment HorizontalAlignment = EoDbText::ConvertHorizontalAlignment(text->horizontalMode());

	OdGePoint3d AlignmentPoint = text->position();
	if (HorizontalAlignment != kAlignLeft || VerticalAlignment != kAlignBottom)
		AlignmentPoint = text->alignmentPoint();

	EoDbFontDefinition FontDefinition;
	FontDefinition.SetPrecision(kTrueType);
	FontDefinition.SetFontName((PCTSTR) FileName);
	FontDefinition.SetHorizontalAlignment(HorizontalAlignment);
	FontDefinition.SetVerticalAlignment(VerticalAlignment);

	EoDbCharacterCellDefinition CharacterCellDefinition;
	CharacterCellDefinition.SetHeight(text->height());
	CharacterCellDefinition.SetRotationAngle(text->rotation());
	CharacterCellDefinition.SetWidthFactor(text->widthFactor());
	CharacterCellDefinition.SetObliqueAngle(text->oblique());

    EoGeReferenceSystem ReferenceSystem(AlignmentPoint, text->normal(), CharacterCellDefinition);
    
    EoDbText* TextPrimitive = new EoDbText();
	TextPrimitive->SetFontDefinition(FontDefinition);
	TextPrimitive->SetReferenceSystem(ReferenceSystem);
	TextPrimitive->SetText((PCTSTR) text->textString());

	ConvertEntityData(text, TextPrimitive);

	group->AddTail(TextPrimitive);
};

void ConvertDimensionData(OdDbDimension* dimension) {
	OdDbBlockTableRecordPtr Block = dimension->dimBlockId().safeOpenObject(OdDb::kForRead);
	
	if (dimension->getMeasurement() >= 0.) {
		OdString formattedMeasurement;
		dimension->formatMeasurement(formattedMeasurement, dimension->getMeasurement(), dimension->dimensionText());
	}
	//OdCmColor bgrndTxtColor;
	//OdUInt16 bgrndTxtFlags = dimension->getBgrndTxtColor(bgrndTxtColor));
};

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
    ATLTRACE2(atlTraceGeneral, 0, L"Entity %s was not converted ...\n", (LPCWSTR) entity->isA()->name());
}

class EoDb2dPolyline_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDb2dPolylinePtr PolylineEntity = entity;

		ATLTRACE2(atlTraceGeneral, 0, L"Converting %s to EoDbPolyline ...\n", (PCTSTR) PolylineEntity->desc()->name());

		EoDbPolyline* PolylinePrimitive = new EoDbPolyline();

		OdDbObjectIteratorPtr Iterator = PolylineEntity->vertexIterator();
		for (int i = 0; !Iterator->done(); i++, Iterator->step()) {
			OdDb2dVertexPtr Vertex = Iterator->entity();
			if (Vertex.get()) {
				OdGePoint3d Point(Vertex->position());
				Point.z = PolylineEntity->elevation();
				PolylinePrimitive->AppendVertex(Vertex->position().convert2d(), Vertex->bulge(), Vertex->startWidth(), Vertex->endWidth());
			}
		}
		PolylinePrimitive->SetClosed(PolylineEntity->isClosed());
		PolylinePrimitive->SetNormal(PolylineEntity->normal());
		PolylinePrimitive->SetElevation(PolylineEntity->elevation());

		ConvertCurveData(PolylineEntity, PolylinePrimitive);

		if (PolylineEntity->polyType() == OdDb::k2dCubicSplinePoly) {
			ATLTRACE2(atlTraceGeneral, 2, L"Cubic spline polyline converted to simple polyline\n");
		}
		else if (PolylineEntity->polyType() == OdDb::k2dQuadSplinePoly) {
			ATLTRACE2(atlTraceGeneral, 2, L"Quad spline polyline converted to simple polyline\n");
		}
		group->AddTail(PolylinePrimitive);
	}
};

class EoDb3dPolyline_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDb3dPolylinePtr PolylineEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"Converting %s to EoDbPolyline ...\n", (PCTSTR) PolylineEntity->desc()->name());
		// <tas="No vertices appended to polyline"</tas>
		OdDbObjectIteratorPtr Iterator = PolylineEntity->vertexIterator();
		for (int i = 0; !Iterator->done(); i++, Iterator->step()) {
			OdDb3dPolylineVertexPtr Vertex = Iterator->entity();
		}
		EoDbPolyline* PolylinePrimitive = new EoDbPolyline();
		if (PolylineEntity->polyType() == OdDb::k3dCubicSplinePoly) {
			ATLTRACE2(atlTraceGeneral, 2, L"Cubic spline polyline converted to simple polyline\n");
		}
		else if (PolylineEntity->polyType() == OdDb::k3dQuadSplinePoly) {
			ATLTRACE2(atlTraceGeneral, 2, L"Quad spline polyline converted to simple polyline\n");
		}
		PolylinePrimitive->SetClosed(PolylineEntity->isClosed());
		ConvertCurveData(PolylineEntity, PolylinePrimitive);

		group->AddTail(PolylinePrimitive);
	}
};

class EoDbArc_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbArcPtr ArcEntity = entity;
		ATLTRACE2(atlTraceGeneral, 1, L"Converting %s to EoDbEllipse ...\n", (PCTSTR) ArcEntity->desc()->name());

		const OdGeVector3d Normal(ArcEntity->normal());
		const OdGePoint3d Center(ArcEntity->center());

		double StartAngle = ArcEntity->startAngle();
		double EndAngle = ArcEntity->endAngle();

		if (StartAngle >= TWOPI) { // need to rationalize angs to first period angles in range on (0 to twopi)
			StartAngle -= TWOPI;
			EndAngle -= TWOPI;
		}
		double SweepAngle = EndAngle - StartAngle;

		if (SweepAngle <= FLT_EPSILON) {
			SweepAngle += TWOPI;
		}
		OdGePoint3d StartPoint;
		ArcEntity->getStartPoint(StartPoint);

		const OdGeVector3d MajorAxis(StartPoint - Center);
		const OdGeVector3d MinorAxis = Normal.crossProduct(MajorAxis);
		
		EoDbEllipse* ArcPrimitive = new EoDbEllipse();
		// <tas="Encountered Circular Arc entity with zero radius. Is this valid for dwg files?"</tas>
		if (!MajorAxis.isZeroLength() && !MinorAxis.isZeroLength()) {
			ArcPrimitive->SetTo(Center, MajorAxis, MinorAxis, SweepAngle);
		}
		ConvertEntityData(ArcEntity, ArcPrimitive);
		group->AddTail(ArcPrimitive);
	}
};

class EoDbAttributeDefinition_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbAttributeDefinitionPtr AttributeDefinitionEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) AttributeDefinitionEntity->desc()->name());
		ATLTRACE2(atlTraceGeneral, 2, L"%s with constant text converted to EoDbText ...\n", (PCTSTR) AttributeDefinitionEntity->desc()->name());
		ATLTRACE2(atlTraceGeneral, 2, L"Tag: %s\n", (PCTSTR) AttributeDefinitionEntity->tag());

		if (AttributeDefinitionEntity->isConstant() && !AttributeDefinitionEntity->isInvisible()) {
			ConvertTextData(static_cast<OdDbText*>(entity), group);
		}
		//ATLTRACE2(atlTraceGeneral, 2, L"Field Length: %s\n", (PCTSTR) attribute->fieldLength());
		//ATLTRACE2(atlTraceGeneral, 2, L"Preset: %i\n", (PCTSTR) attribute->isPreset());
		//ATLTRACE2(atlTraceGeneral, 2, L"Verifiable: %i\n", (PCTSTR) attribute->isVerifiable());
		//ATLTRACE2(atlTraceGeneral, 2, L"Locked in Position: %i\n", (PCTSTR) attribute->lockPositionInBlock());
	}
};

class EoDbBlockReference_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbBlockReferencePtr BlockReferenceEntity = entity;
		ATLTRACE2(atlTraceGeneral, 2, L"Converting %s to EoDbBlockReference ...\n", (PCTSTR) BlockReferenceEntity->desc()->name());

		OdDbBlockTableRecordPtr BlockTableRecordPtr = BlockReferenceEntity->blockTableRecord().safeOpenObject(OdDb::kForRead);

		EoDbBlockReference* BlockReferencePrimitive = new EoDbBlockReference();

		BlockReferencePrimitive->SetName((LPCWSTR) BlockTableRecordPtr->getName());
		BlockReferencePrimitive->SetPosition(BlockReferenceEntity->position());
		BlockReferencePrimitive->SetNormal(BlockReferenceEntity->normal());
		BlockReferencePrimitive->SetScaleFactors(BlockReferenceEntity->scaleFactors());
		BlockReferencePrimitive->SetRotation(BlockReferenceEntity->rotation());

		ConvertEntityData(BlockReferenceEntity, BlockReferencePrimitive);
		group->AddTail(BlockReferencePrimitive);

		// <tas="Block reference - attributes"</tas>
		OdDbObjectIteratorPtr ObjectIterator = BlockReferenceEntity->attributeIterator();
		for (int i = 0; !ObjectIterator->done(); i++, ObjectIterator->step()) {
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
		ATLTRACE2(atlTraceGeneral, 2, L"Converting %s to EoDbEllipse ...\n", (PCTSTR) CircleEntity->desc()->name());

		EoDbEllipse* CirclePrimitive = new EoDbEllipse(CircleEntity->center(), CircleEntity->normal(), CircleEntity->radius());

		ConvertEntityData(CircleEntity, CirclePrimitive);
		group->AddTail(CirclePrimitive);
	}
};

class EoDbEllipse_Converter : public EoDbConvertEntityToPrimitive {
public:
	/// <remarks>
	/// Can only properly convert ellipse which is radial (trival) or non radials which have a start parameter of 0.
	/// </remarks>
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbEllipsePtr Ellipse = entity;
		ATLTRACE2(atlTraceGeneral, 1, L"Converting %s to EoDbEllipse ...\n", (PCTSTR) Ellipse->desc()->name());

		group->AddTail(EoDbEllipse::Create(Ellipse));

	}
};

class EoDbFace_Converter : public EoDbConvertEntityToPrimitive {
public:
	/// <remarks>Four sided, not necessarily planar, surface. It hides other objects and fills with solid color. No support for individual edge visibilty.</remarks>
	/// <tas="Convert Face entity to 2 triangular polygons to ensure planar surface"</tas>
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbFacePtr FaceEntity = entity;
		ATLTRACE2(atlTraceGeneral, 1, L"Converting %s to EoDbHatch ...\n", (PCTSTR) FaceEntity->desc()->name());

		EoDbHatch* HatchPrimitive = new EoDbHatch();

		OdGePoint3d Vertex;
		for (OdUInt16 VertexIndex = 0; VertexIndex < 4; VertexIndex++) {
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
		OdDbHatchPtr Hatch = entity;
		
        ATLTRACE2(atlTraceGeneral, 1, L"Converting %s to EoDbHatch ...\n", (LPCWSTR) Hatch->desc()->name());

        group->AddTail(EoDbHatch::Create(Hatch));
	}
};

class EoDbLeader_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbLeaderPtr LeaderEntity = entity;
		ATLTRACE2(atlTraceGeneral, 2, L"Converting %s to primitive set ...\n", (PCTSTR) LeaderEntity->desc()->name());

		OdRxObjectPtrArray EntitySet;
		LeaderEntity->explode(EntitySet);
		const int NumberOfEntities = EntitySet.size();
		for (int i = 0; i < NumberOfEntities; i++) {
			OdDbEntityPtr Entity = static_cast<OdDbEntityPtr>(EntitySet[i]);
			OdSmartPtr<EoDbConvertEntityToPrimitive> EntityConverter = Entity;
			EntityConverter->Convert(Entity, group);
		}
	}
};

class EoDbLine_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbLinePtr Line = entity;
        
		ATLTRACE2(atlTraceGeneral, 1, L"Converting %s to EoDbLine ...\n", (LPCWSTR) Line->desc()->name());

		group->AddTail(EoDbLine::Create(Line));
	}
};

class EoDbMInsertBlock_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbMInsertBlockPtr MInsertBlockEntity = entity;
		ATLTRACE2(atlTraceGeneral, 2, L"Converting %s to EoDbBlockReference ...\n", (PCTSTR) MInsertBlockEntity->desc()->name());

		OdDbBlockTableRecordPtr BlockTableRecordPtr = MInsertBlockEntity->blockTableRecord().safeOpenObject(OdDb::kForRead);

		EoDbBlockReference* BlockReferencePrimitive = new EoDbBlockReference();
		BlockReferencePrimitive->SetName((LPCWSTR) BlockTableRecordPtr->getName());
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
		ATLTRACE2(atlTraceGeneral, 2, L"Converting %s to EoDbText ...\n", (PCTSTR) MTextEntity->desc()->name());

		group->AddTail(EoDbText::Create(MTextEntity));
	}
};

class EoDbPoint_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbPointPtr PointEntity = entity;
		ATLTRACE2(atlTraceGeneral, 2, L"Converting %s to EoDbPoint ...\n", (PCTSTR) PointEntity->desc()->name());

		EoDbPoint* PointPrimitive = new EoDbPoint(PointEntity->position());
		PointPrimitive->SetPointDisplayMode(PointEntity->database()->getPDMODE());

		ConvertEntityData(PointEntity, PointPrimitive);
		group->AddTail(PointPrimitive);
	}
};

class EoDbPolyline_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbPolylinePtr Polyline = entity;

		ATLTRACE2(atlTraceGeneral, 1, L"Converting %s to EoDbPolyline ...\n", (LPCWSTR) Polyline->desc()->name());
		
        group->AddTail(EoDbPolyline::Create(Polyline));
	}
};

class EoDbProxyEntity_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbProxyEntityPtr ProxyEntityEntity = entity;
		ATLTRACE2(atlTraceGeneral, 2, L"Converting %s to ", (PCTSTR) ProxyEntityEntity->desc()->name());

		ATLTRACE2(atlTraceGeneral, 2, L"Graphics Metafile type: ");
		if (ProxyEntityEntity->graphicsMetafileType() == ProxyEntityEntity->kNoMetafile) {
			ATLTRACE2(atlTraceGeneral, 2, L"No Metafile\n");
		}
		else {
			if (ProxyEntityEntity->graphicsMetafileType() == ProxyEntityEntity->kBoundingBox) {
				ATLTRACE2(atlTraceGeneral, 2, L"Bounding Box\n");
			}
			else if (ProxyEntityEntity->graphicsMetafileType() == ProxyEntityEntity->kFullGraphics) {
				ATLTRACE2(atlTraceGeneral, 2, L"Full Graphics\n");
			}
			OdRxObjectPtrArray EntitySet;
			ProxyEntityEntity->explodeGeometry(EntitySet);
			const int NumberOfEntities = EntitySet.size();
			for (int n = 0; n < NumberOfEntities; n++) {
				OdDbEntityPtr Entity = static_cast<OdDbEntityPtr>(EntitySet[n]);
				OdSmartPtr<EoDbConvertEntityToPrimitive> EntityConverter = Entity;
				EntityConverter->Convert(Entity, group);
			}
		}
		ATLTRACE2(atlTraceGeneral, 2, L"Application Description: %s\n", (PCTSTR) ProxyEntityEntity->applicationDescription());
		ATLTRACE2(atlTraceGeneral, 2, L"Original class name: %s\n", (PCTSTR) ProxyEntityEntity->originalClassName());

		OdAnsiString satString;
		ATLTRACE2(atlTraceGeneral, 2, L"Proxy Sat: %s\n", (PCTSTR) odGetSatFromProxy(ProxyEntityEntity, satString));

		ATLTRACE2(atlTraceGeneral, 2, L"Proxy Flags: %i\n", ProxyEntityEntity->proxyFlags());
		ATLTRACE2(atlTraceGeneral, 2, L"Erase Allowed: %i\n", ProxyEntityEntity->eraseAllowed());
		ATLTRACE2(atlTraceGeneral, 2, L"Transform Allowed: %i\n", ProxyEntityEntity->transformAllowed());
		ATLTRACE2(atlTraceGeneral, 2, L"Color Change Allowed: %i\n", ProxyEntityEntity->colorChangeAllowed());
		ATLTRACE2(atlTraceGeneral, 2, L"Layer Change Allowed: %i\n", ProxyEntityEntity->layerChangeAllowed());
		ATLTRACE2(atlTraceGeneral, 2, L"Linetype Change Allowed: %i\n", ProxyEntityEntity->linetypeChangeAllowed());
		ATLTRACE2(atlTraceGeneral, 2, L"Linetype Scale Change Allowed: %i\n", ProxyEntityEntity->linetypeScaleChangeAllowed());
		ATLTRACE2(atlTraceGeneral, 2, L"Visibility Change Allowed: %i\n", ProxyEntityEntity->visibilityChangeAllowed());
		ATLTRACE2(atlTraceGeneral, 2, L"Cloning Allowed: %i\n", ProxyEntityEntity->cloningAllowed());
		ATLTRACE2(atlTraceGeneral, 2, L"Line Weight Change Allowed: %i\n", ProxyEntityEntity->lineWeightChangeAllowed());
		ATLTRACE2(atlTraceGeneral, 2, L"Plot Style Name Change Allowed: %i\n", ProxyEntityEntity->plotStyleNameChangeAllowed());
	}
};

class EoDbRotatedDimension_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) override {
		OdDbRotatedDimensionPtr RotatedDimensionEntity = entity;
		OdDbBlockTableRecordPtr Block = RotatedDimensionEntity->dimBlockId().safeOpenObject(OdDb::kForRead);

		ATLTRACE2(atlTraceGeneral, 0, L"Converting %s to EoDbBlockReference of block %s  ...\n", (PCTSTR) RotatedDimensionEntity->desc()->name(), (PCTSTR) Block->getName());

		ATLTRACE2(atlTraceGeneral, 2, L"Dimension Line Point: %f, %f, %f\n", RotatedDimensionEntity->dimLinePoint());
		ATLTRACE2(atlTraceGeneral, 2, L"Oblique: %f\n", RotatedDimensionEntity->oblique());
		ATLTRACE2(atlTraceGeneral, 2, L"Rotation: %f\n", RotatedDimensionEntity->rotation());
		ATLTRACE2(atlTraceGeneral, 2, L"Extension Line 1 Point: %f, %f, %f\n", RotatedDimensionEntity->xLine1Point());
		ATLTRACE2(atlTraceGeneral, 2, L"Extension Line 2 Point: %f, %f, %f\n", RotatedDimensionEntity->xLine2Point());
		ConvertDimensionData(RotatedDimensionEntity);
		ATLTRACE2(atlTraceGeneral, 2, L"Dimension Block Name: %s\n", (PCTSTR) Block->getName());

		// <tas="Improper conversion - entity is used alot"</tas>
		EoDbBlockReference* BlockReferencePrimitive = new EoDbBlockReference();
		BlockReferencePrimitive->SetName((LPCWSTR) Block->getName());
		BlockReferencePrimitive->SetPosition(OdGePoint3d::kOrigin);
		BlockReferencePrimitive->SetNormal(OdGeVector3d::kZAxis);
		BlockReferencePrimitive->SetScaleFactors(OdGeScale3d(1., 1., 1.));
		BlockReferencePrimitive->SetRotation(0.);

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
		ATLTRACE2(atlTraceGeneral, 0, L"Converting %s to EoDbHatch ...\n", (PCTSTR) SolidEntity->desc()->name());

		EoDbHatch* HatchPrimitive = new EoDbHatch();
		OdGePoint3d Point;
		SolidEntity->getPointAt(0, Point);
		HatchPrimitive->Append(Point);
		SolidEntity->getPointAt(1, Point);
		HatchPrimitive->Append(Point);
		SolidEntity->getPointAt(2, Point);
		OdGePoint3d EndPoint;
		SolidEntity->getPointAt(3, EndPoint);
		HatchPrimitive->Append((Point == EndPoint) ? Point : EndPoint);
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
		ATLTRACE2(atlTraceGeneral, 2, L"Converting %s to EoDbSpline ...\n", (PCTSTR) SplineEntity->desc()->name());

		int Degree;
		bool Rational;
		bool Closed;
		bool Periodic;
		OdGePoint3dArray ControlPoints;
		OdGeDoubleArray Weights;
		OdGeKnotVector Knots;
		double Tolerance;

		SplineEntity->getNurbsData(Degree, Rational, Closed, Periodic, ControlPoints, Knots, Weights, Tolerance);

		ATLTRACE2(atlTraceGeneral, 0, L"Degree: %i\n", Degree);
		ATLTRACE2(atlTraceGeneral, 2, L"Rational: %i\n", Rational);
		ATLTRACE2(atlTraceGeneral, 2, L"Periodic: %i\n", Periodic);
		ATLTRACE2(atlTraceGeneral, 2, L"Control Point Tolerance: %f\n", Tolerance);

		ATLTRACE2(atlTraceGeneral, 0, L"Number of control points: %i\n", ControlPoints.size());
		for (OdUInt16 n = 0; n < ControlPoints.size(); n++) {
			ATLTRACE2(atlTraceGeneral, 2, L"Control Point: %f, %f, %f\n",  ControlPoints[n]);
		}
		ATLTRACE2(atlTraceGeneral, 0, L"Number of Knots: %i\n", Knots.length());
		for (OdUInt16 n = 0; n < Knots.length(); n++) {
			ATLTRACE2(atlTraceGeneral, 0, L"Knot: %f\n", Knots[n]);
		}
		if (Rational) {
			ATLTRACE2(atlTraceGeneral, 0, L"Number of Weights: %i\n", Weights.size());
			for (OdUInt16 n = 0; n < Weights.size(); n++) {
				ATLTRACE2(atlTraceGeneral, 0, L"Weight: %f\n", Weights[n]);
			}
		}
		// <tas="Only creating non-periodic splines."</tas>
		if (Periodic) {
			ATLTRACE2(atlTraceGeneral, 0, L"Periodic %s was not converted ...\n", (PCTSTR) SplineEntity->desc()->name());
		}
		else {
			EoDbSpline* SplinePrimitive = new EoDbSpline();
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
		ATLTRACE2(atlTraceGeneral, 1, L"Converting %s to EoDbText ...\n", (LPCWSTR) Text->desc()->name());

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
		ATLTRACE2(atlTraceGeneral, 2, L"Converting %s to EoDbHatch ...\n", (PCTSTR) TraceEntity->desc()->name());

		EoDbHatch* HatchPrimitive = new EoDbHatch();
		OdGePoint3d Point;
		TraceEntity->getPointAt(0, Point);
		HatchPrimitive->Append(Point);
		TraceEntity->getPointAt(1, Point);
		HatchPrimitive->Append(Point);
		TraceEntity->getPointAt(2, Point);
		OdGePoint3d EndPoint;
		TraceEntity->getPointAt(3, EndPoint);
		HatchPrimitive->Append((Point == EndPoint) ? Point : EndPoint);
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
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) ViewportEntity->desc()->name());

		ATLTRACE2(atlTraceGeneral, 2, L"Back Clip Distance: %f\n", ViewportEntity->backClipDistance());
		ATLTRACE2(atlTraceGeneral, 2, L"Back Clip On: %i\n", ViewportEntity->isBackClipOn());
		ATLTRACE2(atlTraceGeneral, 2, L"Center Point: %f, %f, %f\n", ViewportEntity->centerPoint());
		ATLTRACE2(atlTraceGeneral, 2, L"Circle sides: %i\n", ViewportEntity->circleSides());
		ATLTRACE2(atlTraceGeneral, 2, L"Custom Scale: %f\n", ViewportEntity->customScale());
		ATLTRACE2(atlTraceGeneral, 2, L"Elevation: %f\n", ViewportEntity->elevation());
		ATLTRACE2(atlTraceGeneral, 2, L"Front Clip at Eye: %i\n", ViewportEntity->isFrontClipAtEyeOn());
		ATLTRACE2(atlTraceGeneral, 2, L"Front Clip Distance: %f\n", ViewportEntity->frontClipDistance());
		ATLTRACE2(atlTraceGeneral, 2, L"Front Clip On: %i\n", ViewportEntity->isFrontClipOn());
		ATLTRACE2(atlTraceGeneral, 2, L"Plot style sheet: %i\n", ViewportEntity->effectivePlotStyleSheet());

		OdDbObjectIdArray layerIds;
		ViewportEntity->getFrozenLayerList(layerIds);
		if (layerIds.length()) {
			ATLTRACE2(atlTraceGeneral, 2, L"Frozen Layers:");
			for (int i = 0; i < (int) layerIds.length(); i++) {
				ATLTRACE2(atlTraceGeneral, 2, L"%i  ", layerIds[i]);
			}
		}
		else {
			ATLTRACE2(atlTraceGeneral, 2, L"Frozen Layers: None\n");
		}

		OdGePoint3d origin;
		OdGeVector3d xAxis;
		OdGeVector3d yAxis;
		ViewportEntity->getUcs(origin, xAxis, yAxis);
		ATLTRACE2(atlTraceGeneral, 2, L"UCS origin: %f, %f, %f\n", origin);
		ATLTRACE2(atlTraceGeneral, 2, L"UCS x-Axis: %f, %f, %f\n", xAxis);
		ATLTRACE2(atlTraceGeneral, 2, L"UCS y-Axis: %f, %f, %f\n", yAxis);
		ATLTRACE2(atlTraceGeneral, 2, L"Grid Increment: %f\n", ViewportEntity->gridIncrement());
		ATLTRACE2(atlTraceGeneral, 2, L"Grid On: %i\n", ViewportEntity->isGridOn());
		ATLTRACE2(atlTraceGeneral, 2, L"Height: %f\n", ViewportEntity->height());
		ATLTRACE2(atlTraceGeneral, 2, L"Lens Length: %f\n", ViewportEntity->lensLength());
		ATLTRACE2(atlTraceGeneral, 2, L"Locked: %i\n", ViewportEntity->isLocked());
		ATLTRACE2(atlTraceGeneral, 2, L"Non-Rectangular Clip: %i\n", ViewportEntity->isNonRectClipOn());

		if (!ViewportEntity->nonRectClipEntityId().isNull()) {
			ATLTRACE2(atlTraceGeneral, 2, L"Non-rectangular Clipper: \n", ViewportEntity->nonRectClipEntityId().getHandle());
		}
		ATLTRACE2(atlTraceGeneral, 2, L"Render Mode: %i\n", ViewportEntity->renderMode());
		ATLTRACE2(atlTraceGeneral, 2, L"Remove Hidden Lines: %i\n", ViewportEntity->hiddenLinesRemoved());
		ATLTRACE2(atlTraceGeneral, 2, L"Shade Plot: \n", ViewportEntity->shadePlot());
		ATLTRACE2(atlTraceGeneral, 2, L"Snap Isometric: %i\n", ViewportEntity->isSnapIsometric());
		ATLTRACE2(atlTraceGeneral, 2, L"Snap On: %i\n", ViewportEntity->isSnapOn());
		ATLTRACE2(atlTraceGeneral, 2, L"Transparent: %i\n", ViewportEntity->isTransparent());
		ATLTRACE2(atlTraceGeneral, 2, L"UCS Follow: %i\n", ViewportEntity->isUcsFollowModeOn());
		ATLTRACE2(atlTraceGeneral, 2, L"UCS Icon at Origin: %i\n", ViewportEntity->isUcsIconAtOrigin());

		OdDb::OrthographicView orthoUCS;
		ATLTRACE2(atlTraceGeneral, 2, L"UCS Orthographic: %i\n", ViewportEntity->isUcsOrthographic(orthoUCS));
		ATLTRACE2(atlTraceGeneral, 2, L"Orthographic UCS: %i\n", orthoUCS);
		ATLTRACE2(atlTraceGeneral, 2, L"UCS Saved with VP: %i\n", ViewportEntity->isUcsSavedWithViewport());

		if (!ViewportEntity->ucsName().isNull()) {
			OdDbUCSTableRecordPtr UCS = ViewportEntity->ucsName().safeOpenObject(OdDb::kForRead);
			ATLTRACE2(atlTraceGeneral, 2, L"UCS Name: \n", UCS->getName());
		}
		else {
			ATLTRACE2(atlTraceGeneral, 2, L"UCS Name: Null");
		}
		ATLTRACE2(atlTraceGeneral, 2, L"View Center: %f, %f\n", ViewportEntity->viewCenter());
		ATLTRACE2(atlTraceGeneral, 2, L"View Height: %f\n", ViewportEntity->viewHeight());
		ATLTRACE2(atlTraceGeneral, 2, L"View Target: %f, %f, %f\n", ViewportEntity->viewTarget());
		ATLTRACE2(atlTraceGeneral, 2, L"Width: %f\n", ViewportEntity->width());
		//ConvertEntityData(ViewportEntity, );
	}
};

class Converters {
    OdStaticRxObject<EoDbConvertEntityToPrimitive> m_EntityConverter;
    
    OdStaticRxObject<EoDb2dPolyline_Converter> m_2dPolylineConverter;
	OdStaticRxObject<EoDb3dPolyline_Converter> m_3dPolylineConverter;
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
		OdDbArc::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_ArcConverter);
		OdDbAttributeDefinition::desc()->addX(EoDbConvertEntityToPrimitive::desc(),	 &m_AttributeDefinitionConverter);
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
		OdDbArc::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbAttributeDefinition	::desc()->delX(EoDbConvertEntityToPrimitive::desc());
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
AeSysDoc* ConvertEntityToPrimitiveProtocolExtension::m_Document = NULL;

ConvertEntityToPrimitiveProtocolExtension::ConvertEntityToPrimitiveProtocolExtension(AeSysDoc* document) noexcept
    : m_Converters(0) {
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
	m_Converters = 0;
}
