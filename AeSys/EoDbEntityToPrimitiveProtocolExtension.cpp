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
#include "DbLine.h"
#include "DbMInsertBlock.h"
#include "DbMText.h"
#include "DbMline.h"
#include "DbOle2Frame.h"
#include "DbOrdinateDimension.h"
#include "DbPolyFaceMesh.h"
#include "DbPolyFaceMeshVertex.h"
#include "DbPolygonMesh.h"
#include "DbPolygonMeshVertex.h"
#include "DbPolyline.h"
#include "DbProxyEntity.h"
#include "DbRadialDimension.h"
#include "DbRasterImage.h"
#include "DbRay.h"
#include "DbRegion.h"
#include "DbRotatedDimension.h"
#include "DbShape.h"
#include "DbSolid.h"
#include "DbSpline.h"
#include "DbTable.h"
#include "DbTextStyleTableRecord.h"
#include "DbTrace.h"
#include "DbUCSTableRecord.h"
#include "DbWipeout.h"
#include "DbXline.h"
#include "Ge/GeCircArc2d.h"
#include "Ge/GeCircArc3d.h"
#include "Ge/GeCurve2d.h"
#include "Ge/GeEllipArc2d.h"
#include "Ge/GeKnotVector.h"
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
	OdDbObjectId Linetype = entity->linetypeId();

	if (Linetype == DatabasePtr->getLinetypeByBlockId()) {
		primitive->SetLinetypeIndex(EoDbPrimitive::LINETYPE_BYBLOCK);
	}
	else if (Linetype == DatabasePtr->getLinetypeByLayerId()) {
		primitive->SetLinetypeIndex(EoDbPrimitive::LINETYPE_BYLAYER);
	}
	else {
		OdString Name = entity->linetype();
		primitive->SetLinetypeIndex(EoDbLinetypeTable::LegacyLinetypeIndex(Name));
	}

	OdGeExtents3d extents;
	if (eOk == entity->getGeomExtents(extents)) {
		ATLTRACE2(atlTraceGeneral, 2, L"Min Extents: %f, %f, %f\n", extents.minPoint());
		ATLTRACE2(atlTraceGeneral, 2, L"Max Extents: %f, %f, %f\n", extents.maxPoint());
	}
	ATLTRACE2(atlTraceGeneral, 2, L"Layer: %s\n", (PCTSTR) entity->layer());
	ATLTRACE2(atlTraceGeneral, 2, L"Color Index: %i\n", entity->colorIndex());
	ATLTRACE2(atlTraceGeneral, 2, L"Color: %i\n", entity->color());
	ATLTRACE2(atlTraceGeneral, 2, L"Linetype: %i\n", entity->linetype());
	ATLTRACE2(atlTraceGeneral, 2, L"LTscale: %f\n", entity->linetypeScale());
	ATLTRACE2(atlTraceGeneral, 2, L"Lineweight: %i\n", entity->lineWeight());
	ATLTRACE2(atlTraceGeneral, 2, L"Plot Style: %i\n", entity->plotStyleName());
	ATLTRACE2(atlTraceGeneral, 2, L"Transparency Method: %i\n", entity->transparency().method());
	ATLTRACE2(atlTraceGeneral, 2, L"Visibility: %i\n", entity->visibility());
	ATLTRACE2(atlTraceGeneral, 2, L"Planar: %i\n", entity->isPlanar());

	OdGePlane plane;
	OdDb::Planarity planarity = OdDb::kNonPlanar;
	entity->getPlane(plane, planarity);
	ATLTRACE2(atlTraceGeneral, 2, L"Planarity: %i\n", planarity);
	if (entity->isPlanar()) {
		OdGePoint3d origin;
		OdGeVector3d uAxis;
		OdGeVector3d vAxis;
		plane.get(origin, uAxis, vAxis);
		ATLTRACE2(atlTraceGeneral, 2, L"Origin: %f, %f, %f\n", origin);
		ATLTRACE2(atlTraceGeneral, 2, L"u-Axis: %f, %f, %f\n", uAxis);
		ATLTRACE2(atlTraceGeneral, 2, L"v-Axis: %f, %f, %f\n", vAxis);
	}
}

void ConvertTextData(OdDbText* text, EoDbGroup* group) {
	ATLTRACE2(atlTraceGeneral, 1, L"Converting %s to EoDbText ...\n", (PCTSTR) text->desc()->name());

	OdDbObjectId TextStyleObjectId = text->textStyle();
	OdDbTextStyleTableRecordPtr TextStyleTableRecordPtr = TextStyleObjectId.safeOpenObject(OdDb::kForRead);

	OdString FileName;
	if (TextStyleTableRecordPtr->isShapeFile()) {
		FileName = L"Standard";
		ATLTRACE2(atlTraceGeneral, 2, L"TextStyle references shape library %s.\n", (PCTSTR) TextStyleTableRecordPtr->desc()->name());
	}
	else {
		FileName = TextStyleTableRecordPtr->fileName();
		int nExt = FileName.reverseFind('.');
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
	EoDb::VerticalAlignment VerticalAlignment;
	switch (text->verticalMode()) {
	case OdDb::kTextVertMid:
		VerticalAlignment = EoDb::kAlignMiddle;
		break;

	case OdDb::kTextTop:
		VerticalAlignment = EoDb::kAlignTop;
		break;

	default: // OdDb::kTextBottom & OdDb::kTextBase
		VerticalAlignment = EoDb::kAlignBottom;
	}
	EoDb::HorizontalAlignment HorizontalAlignment;
	switch (text->horizontalMode()) {
	case OdDb::kTextMid:
	case OdDb::kTextCenter:
		HorizontalAlignment = EoDb::kAlignCenter;
		break;

	case OdDb::kTextRight:
	case OdDb::kTextAlign:
	case OdDb::kTextFit:
		HorizontalAlignment = EoDb::kAlignRight;
		break;

	default: // OdDb::kTextLeft
		HorizontalAlignment = EoDb::kAlignLeft;
	}
	OdGePoint3d AlignmentPoint = text->position();
	if (HorizontalAlignment != EoDb::kAlignLeft || VerticalAlignment != EoDb::kAlignBottom)
		AlignmentPoint = text->alignmentPoint();

	EoDbFontDefinition FontDefinition;
	FontDefinition.SetPrecision(EoDb::kEoTrueType);
	FontDefinition.SetFontName((PCTSTR) FileName);
	FontDefinition.SetHorizontalAlignment(HorizontalAlignment);
	FontDefinition.SetVerticalAlignment(VerticalAlignment);

	EoDbCharacterCellDefinition CharacterCellDefinition;
	CharacterCellDefinition.SetHeight(text->height());
	CharacterCellDefinition.SetRotationAngle(text->rotation());
	CharacterCellDefinition.SetWidthFactor(text->widthFactor());
	CharacterCellDefinition.SetObliqueAngle(text->oblique());

	OdGeVector3d XDirection;
	OdGeVector3d YDirection;
	CharCellDef_EncdRefSys(text->normal(), CharacterCellDefinition, XDirection, YDirection);

	EoGeReferenceSystem ReferenceSystem(AlignmentPoint, XDirection, YDirection);
	EoDbText* TextPrimitive = new EoDbText();
	TextPrimitive->SetFontDefinition(FontDefinition);
	TextPrimitive->SetReferenceSystem(ReferenceSystem);
	TextPrimitive->SetText((PCTSTR) text->textString());

	ConvertEntityData(text, TextPrimitive);
	group->AddTail(TextPrimitive);

	ATLTRACE2(atlTraceGeneral, 2, L"Default Alignment: %i\n", text->isDefaultAlignment());
	ATLTRACE2(atlTraceGeneral, 2, L"Mirrored in X: %i\n", text->isMirroredInX());
	ATLTRACE2(atlTraceGeneral, 2, L"Mirrored in Y: %i\n", text->isMirroredInY());
	OdGePoint3dArray points;
	text->getBoundingPoints(points);
	ATLTRACE2(atlTraceGeneral, 2, L"TL Bounding Point: %f, %f, %f\n", points[0]);
	ATLTRACE2(atlTraceGeneral, 2, L"TR Bounding Point: %f, %f, %f\n", points[1]);
	ATLTRACE2(atlTraceGeneral, 2, L"BL Bounding Point: %f, %f, %f\n", points[2]);
	ATLTRACE2(atlTraceGeneral, 2, L"BR Bounding Point: %f, %f, %f\n", points[3]);
	ATLTRACE2(atlTraceGeneral, 2, L"Normal: %f, %f, %f\n", text->normal());
	ATLTRACE2(atlTraceGeneral, 2, L"Thickness: %f\n", text->thickness());
};

void ConvertAttributeData(OdDbAttribute* attribute) {
	ATLTRACE2(atlTraceGeneral, 2, L"Tag: %s\n", (PCTSTR) attribute->tag());
	ATLTRACE2(atlTraceGeneral, 2, L"Field Length: %s\n", (PCTSTR) attribute->fieldLength());
	ATLTRACE2(atlTraceGeneral, 2, L"Invisible: %i\n", (PCTSTR) attribute->isInvisible());
	ATLTRACE2(atlTraceGeneral, 2, L"Preset: %i\n", (PCTSTR) attribute->isPreset());
	ATLTRACE2(atlTraceGeneral, 2, L"Verifiable: %i\n", (PCTSTR) attribute->isVerifiable());
	ATLTRACE2(atlTraceGeneral, 2, L"Locked in Position: %i\n", (PCTSTR) attribute->lockPositionInBlock());
	ATLTRACE2(atlTraceGeneral, 2, L"Constant: %i\n", (PCTSTR) attribute->isConstant());
};

void ConvertDimensionData(OdDbDimension* dimension) {
	OdDbBlockTableRecordPtr Block = dimension->dimBlockId().safeOpenObject(OdDb::kForRead);
	ATLTRACE2(atlTraceGeneral, 2, L"Measurement: %f\n", dimension->getMeasurement());
	ATLTRACE2(atlTraceGeneral, 2, L"Dimension Text: %s\n", (PCTSTR) dimension->dimensionText());

	if (dimension->getMeasurement() >= 0.) {
		OdString formattedMeasurement;
		dimension->formatMeasurement(formattedMeasurement, dimension->getMeasurement(), dimension->dimensionText());
		ATLTRACE2(atlTraceGeneral, 2, L"Formatted Measurement: %s\n", (PCTSTR) formattedMeasurement);
	}
	ATLTRACE2(atlTraceGeneral, 2, L"Dimension Block Name: %s\n", (PCTSTR) Block->getName());
	ATLTRACE2(atlTraceGeneral, 2, L"Dimension Block Position: %f, %f, %f\n", dimension->dimBlockPosition().x, dimension->dimBlockPosition().y, dimension->dimBlockPosition().z);
	ATLTRACE2(atlTraceGeneral, 2, L"Dimension Block Rotation: %f\n", dimension->dimBlockRotation());
	ATLTRACE2(atlTraceGeneral, 2, L"Dimension Block Scale: %f, %f, %f\n", dimension->dimBlockScale().sx, dimension->dimBlockScale().sy, dimension->dimBlockScale().sz);
	ATLTRACE2(atlTraceGeneral, 2, L"Text Position: %f, %f, %f\n", dimension->textPosition().x, dimension->textPosition().y, dimension->textPosition().z);
	ATLTRACE2(atlTraceGeneral, 2, L"Text Rotation: %f\n", dimension->textRotation());
	ATLTRACE2(atlTraceGeneral, 2, L"Dimension Style: %i\n", dimension->dimensionStyle());
	//OdCmColor bgrndTxtColor;
	//OdUInt16 bgrndTxtFlags = dimension->getBgrndTxtColor(bgrndTxtColor));
	//ATLTRACE2(atlTraceGeneral, 2, L"Background Text Color: %i\n", bgrndTxtColor);
	//ATLTRACE2(atlTraceGeneral, 2, L"Background Text Flags: %i\n", bgrndTxtFlags);
	ATLTRACE2(atlTraceGeneral, 2, L"Extension Line 1 Linetype: %i\n", dimension->getDimExt1Linetype());
	ATLTRACE2(atlTraceGeneral, 2, L"Extension Line 2 Linetype: %i\n", dimension->getDimExt2Linetype());
	ATLTRACE2(atlTraceGeneral, 2, L"Dimension Line Linetype: %i\n", dimension->getDimLinetype());
	ATLTRACE2(atlTraceGeneral, 2, L"Horizontal Rotation: %f\n", dimension->horizontalRotation());
	ATLTRACE2(atlTraceGeneral, 2, L"Elevation: %f\n", dimension->elevation());
	ATLTRACE2(atlTraceGeneral, 2, L"Normal: %f, %f, %f\n", dimension->normal().x, dimension->normal().y, dimension->normal().z);
};

void ConvertCurveData(OdDbEntity* entity, EoDbPrimitive* primitive) {
	OdDbCurvePtr Curve = entity;
	OdGePoint3d StartPoint;
	if (eOk == Curve->getStartPoint(StartPoint)) {
		ATLTRACE2(atlTraceGeneral, 2, L"Start Point: %f, %f, %f\n", StartPoint);
	}
	OdGePoint3d EndPoint;
	if (eOk == Curve->getEndPoint(EndPoint)) {
		ATLTRACE2(atlTraceGeneral, 2, L"End Point: %f, %f, %f\n", StartPoint);
	}
	static_cast<EoDbPolyline*>(primitive)->SetClosed(Curve->isClosed());

	ATLTRACE2(atlTraceGeneral, 2, L"Periodic: %i\n", Curve->isPeriodic());

	double Area;
	if (eOk == Curve->getArea(Area)) {
		ATLTRACE2(atlTraceGeneral, 2, L"Area: %f\n", Area);
	}
	ConvertEntityData(entity, primitive);
}

void EoDbConvertEntityToPrimitive::Convert(OdDbEntity* entity, EoDbGroup*) {
	OdDbEntityPtr UnknownEntity = entity;
	ATLTRACE2(atlTraceGeneral, 0, L"%s is unknown entity ...\n", (PCTSTR) UnknownEntity->desc()->name());
}

class EoDb2LineAngularDimension_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDb2LineAngularDimensionPtr AngularDimensionEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR)AngularDimensionEntity->desc()->name());
	}
};

class EoDb2dPolyline_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
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
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
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

class EoDb3dSolid_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDb3dSolidPtr SolidEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) SolidEntity->desc()->name());
	}
};

class EoDb3PointAngularDimension_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDb3PointAngularDimensionPtr AngularDimensionEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) AngularDimensionEntity->desc()->name());
	}
};

class EoDbAlignedDimension_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbAlignedDimensionPtr AlignedDimensionEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) AlignedDimensionEntity->desc()->name());
	}
};

class EoDbArc_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
		OdDbArcPtr ArcEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"Converting %s to EoDbEllipse ...\n", (PCTSTR) ArcEntity->desc()->name());

		OdGeVector3d Normal(ArcEntity->normal());
		OdGePoint3d Center(ArcEntity->center());

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

		OdGeVector3d MajorAxis(StartPoint - Center);
		OdGeVector3d MinorAxis = Normal.crossProduct(MajorAxis);
		
		EoDbEllipse* ArcPrimitive = new EoDbEllipse();
		// <tas="Encountered Circular Arc entity with zero radius. Is this valid for dwg files?"</tas>
		if (!MajorAxis.isZeroLength() && !MinorAxis.isZeroLength()) {
			ArcPrimitive->SetTo(Center, MajorAxis, MinorAxis, SweepAngle);
		}
		ConvertEntityData(ArcEntity, ArcPrimitive);
		group->AddTail(ArcPrimitive);
	}
};

class EoDbArcAlignedText_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbArcAlignedTextPtr ArcAlignedTextEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) ArcAlignedTextEntity->desc()->name());
	}
};

class EoDbArcDimension_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbArcDimensionPtr ArcDimensionEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) ArcDimensionEntity->desc()->name());
	}
};
class EoDbAttributeDefinition_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
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
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
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
class EoDbBody_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbBodyPtr BodyEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) BodyEntity->desc()->name());
	}
};
class EoDbCircle_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
		OdDbCirclePtr CircleEntity = entity;
		ATLTRACE2(atlTraceGeneral, 2, L"Converting %s to EoDbEllipse ...\n", (PCTSTR) CircleEntity->desc()->name());

		EoDbEllipse* CirclePrimitive = new EoDbEllipse(CircleEntity->center(), CircleEntity->normal(), CircleEntity->radius());

		ConvertEntityData(CircleEntity, CirclePrimitive);
		group->AddTail(CirclePrimitive);
	}
};
class EoDbDiametricDimension_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbDiametricDimensionPtr DiametricDimensionEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) DiametricDimensionEntity->desc()->name());
	}
};
class EoDbEllipse_Converter : public EoDbConvertEntityToPrimitive {
public:
	/// <remarks>
	/// Can only properly convert ellipse which is radial (trival) or non radials which have a start parameter of 0.
	/// </remarks>
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
		OdDbEllipsePtr EllipseEntity = entity;
		ATLTRACE2(atlTraceGeneral, 1, L"Converting %s to EoDbEllipse ...\n", (PCTSTR) EllipseEntity->desc()->name());

		OdGeVector3d MajorAxis(EllipseEntity->majorAxis());
		OdGeVector3d MinorAxis(EllipseEntity->minorAxis());

		double StartAngle = EllipseEntity->startAngle();
		double EndAngle = EllipseEntity->endAngle();

		if (StartAngle >= TWOPI) { // need to rationalize angs to first period angles in range on (0 to twopi)
			StartAngle -= TWOPI;
			EndAngle -= TWOPI;
		}
		double SweepAngle = EndAngle - StartAngle;
		if (SweepAngle <= FLT_EPSILON)
			SweepAngle += TWOPI;

		if (StartAngle != 0.) {
			MajorAxis.rotateBy(StartAngle, EllipseEntity->normal());
			MinorAxis.rotateBy(StartAngle, EllipseEntity->normal());
			if (EllipseEntity->radiusRatio() != 1.) {
				ATLTRACE2(atlTraceGeneral, 0, L"Ellipse: Non radial with start parameter not 0.\n");
			}
		}
		EoDbEllipse* EllipsePrimitive = new EoDbEllipse(EllipseEntity->center(), MajorAxis, MinorAxis, SweepAngle);
		
		ConvertEntityData(EllipseEntity, EllipsePrimitive);
		group->AddTail(EllipsePrimitive);

	}
};
class EoDbFace_Converter : public EoDbConvertEntityToPrimitive {
public:
	/// <remarks>Four sided, not necessarily planar, surface. It hides other objects and fills with solid color. No support for individual edge visibilty.</remarks>
	/// <tas="Convert Face entity to 2 triangular polygons to ensure planar surface"</tas>
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
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
class EoDbFcf_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbFcfPtr FcfEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) FcfEntity->desc()->name());
	}
};
class EoDbHatch_Converter : public EoDbConvertEntityToPrimitive {
private:
	static void ConvertPolylineType(int loopIndex , OdDbHatchPtr &hatchEntity, EoDbHatch* hatchPrimitive) {
		hatchPrimitive->SetLoopAt(loopIndex, hatchEntity);
	}
	static void ConvertCircularArcEdge(OdGeCurve2d* pEdge) {
		OdGeCircArc2d* pCircArc = (OdGeCircArc2d*) pEdge;
		ATLTRACE2(atlTraceGeneral, 2, L"Center: %f, %f\n", pCircArc->center());
		ATLTRACE2(atlTraceGeneral, 2, L"Radius: %f\n", pCircArc->radius());
		ATLTRACE2(atlTraceGeneral, 2, L"Start Angle %f\n", pCircArc->startAng());
		ATLTRACE2(atlTraceGeneral, 2, L"End Angle: %f\n", pCircArc->endAng());
		ATLTRACE2(atlTraceGeneral, 2, L"Clockwise: %i\n", pCircArc->isClockWise());
	}
	static void ConvertEllipticalArcEdge(OdGeCurve2d* pEdge) {
		OdGeEllipArc2d* pEllipArc = (OdGeEllipArc2d*)pEdge;
		ATLTRACE2(atlTraceGeneral, 2, L"Center: %f, %f\n", pEllipArc->center()[0], pEllipArc->center()[1]);
		ATLTRACE2(atlTraceGeneral, 2, L"Major Radius: %f\n", pEllipArc->majorRadius());
		ATLTRACE2(atlTraceGeneral, 2, L"Minor Radius: %f\n", pEllipArc->minorRadius());
		ATLTRACE2(atlTraceGeneral, 2, L"Major Axis: %f\n", pEllipArc->majorAxis());
		ATLTRACE2(atlTraceGeneral, 2, L"Minor Axis: %f, %f\n", pEllipArc->minorAxis());
		ATLTRACE2(atlTraceGeneral, 2, L"Start Angle: %f\n", pEllipArc->startAng());
		ATLTRACE2(atlTraceGeneral, 2, L"End Angle: %f\n", pEllipArc->endAng());
		ATLTRACE2(atlTraceGeneral, 2, L"Clockwise:%i\n", pEllipArc->isClockWise());
	}
	static void ConvertNurbCurveEdge(OdGeCurve2d* pEdge) {
		OdGeNurbCurve2d* pNurbCurve = (OdGeNurbCurve2d*)pEdge;
		int Degree;
		bool Rational, Periodic;
		OdGePoint2dArray ControlPoints;
		OdGeDoubleArray Weights;
		OdGeKnotVector  Knots;

		pNurbCurve->getDefinitionData (Degree, Rational, Periodic, Knots, ControlPoints, Weights);
		ATLTRACE2(atlTraceGeneral, 2, L"Degree: %i\n", Degree);
		ATLTRACE2(atlTraceGeneral, 2, L"Rational: %i\n", Rational);
		ATLTRACE2(atlTraceGeneral, 2, L"Periodic: %i\n", Periodic);

		ATLTRACE2(atlTraceGeneral, 2, L"Number of Control Points: %i\n", (int) ControlPoints.size());
		int i;
		for (i = 0; i < (int) ControlPoints.size(); i++) {
			ATLTRACE2(atlTraceGeneral, 2, L"Control Point: %f, %f", ControlPoints[i][0], ControlPoints[i][1]);
		}
		ATLTRACE2(atlTraceGeneral, 2, L"Number of Knots: %i\n", Knots.length());
		for (i = 0; i < Knots.length(); i++) {
			ATLTRACE2(atlTraceGeneral, 2, L"Knot: %f\n", Knots[i]);
		}
		if (Rational) {
			ATLTRACE2(atlTraceGeneral, 2, L"Number of Weights: %i\n", (int) Weights.size());
			for (i = 0; i < (int) Weights.size(); i++) {
				ATLTRACE2(atlTraceGeneral, 2, L"Weight: %f\n", Weights[i]);
			}
		}
	}
	static void ConvertEdgesType(int loopIndex , OdDbHatchPtr &hatchEntity, EoDbHatch* hatchPrimitive) {
		EdgeArray Edges;
		hatchEntity->getLoopAt (loopIndex, Edges);
		
		double Lower;
		double Upper(1.);
		size_t NumberOfEdges = Edges.size();

		for (size_t EdgeIndex = 0; EdgeIndex < NumberOfEdges; EdgeIndex++) {
			OdGeCurve2d* Edge = Edges[EdgeIndex];
			switch (Edge->type ()) {
			case OdGe::kLineSeg2d:
				break;
			case OdGe::kCircArc2d:
				ConvertCircularArcEdge(Edge);
				break;
			case OdGe::kEllipArc2d:
				ConvertEllipticalArcEdge(Edge);
				break;
			case OdGe::kNurbCurve2d:
				ConvertNurbCurveEdge(Edge);
				break;
			}
			// Common Edge Properties
			OdGeInterval Interval;
			Edge->getInterval(Interval);
			Interval.getBounds(Lower, Upper);

			OdGePoint2d LowerPoint(Edge->evalPoint(Lower));

			hatchPrimitive->Append(OdGePoint3d(LowerPoint.x, LowerPoint.y, hatchEntity->elevation()));
		}
		OdGePoint2d UpperPoint(Edges[NumberOfEdges - 1]->evalPoint(Upper));
		hatchPrimitive->Append(OdGePoint3d(UpperPoint.x, UpperPoint.y, hatchEntity->elevation()));
		// <tas="Hatch edge conversion - not considering the effect of "Closed" edge property"</tas>
	}
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
		OdDbHatchPtr HatchEntity = entity;
		ATLTRACE2(atlTraceGeneral, 2, L"Converting %s to EoDbHatch ...\n", (PCTSTR) HatchEntity->desc()->name());

		EoDbHatch* HatchPrimitive = new EoDbHatch();

		ConvertEntityData(HatchEntity, HatchPrimitive);
		group->AddTail(HatchPrimitive);
		
		ATLTRACE2(atlTraceGeneral, 2, L"Hatch Style: %i\n", HatchEntity->hatchStyle()); // kNormal(0), kOuter(1), kIgnore(2)
		ATLTRACE2(atlTraceGeneral, 2, L"Hatch Object Type: %i\n", HatchEntity->hatchObjectType());
		if (HatchEntity->isHatch()) {
			switch (HatchEntity->patternType()) {
			case OdDbHatch::kPreDefined:
			case OdDbHatch::kCustomDefined:
				if (HatchEntity->isSolidFill()) {
					HatchPrimitive->SetInteriorStyle(EoDbHatch::kSolid);
				}
				else {
					HatchPrimitive->SetInteriorStyle(EoDbHatch::kHatch);
					HatchPrimitive->SetInteriorStyleIndex(EoDbHatch::LegacyInteriorStyleIndex(HatchEntity->patternName()));
					OdGePoint3d Origin = OdGePoint3d::kOrigin + HatchEntity->elevation() * HatchEntity->normal();
					// <tas="Pattern scaling model to world issues. Resulting hatch is very large without the world scale division"</tas>
					HatchPrimitive->SetPatternReferenceSystem(Origin, HatchEntity->normal(), HatchEntity->patternAngle(), HatchEntity->patternScale());
				}
				break;
			case OdDbHatch::kUserDefined:
				HatchPrimitive->SetInteriorStyle(EoDbHatch::kHatch);
				HatchPrimitive->SetInteriorStyleIndex(EoDbHatch::LegacyInteriorStyleIndex(HatchEntity->patternName()));
				OdGePoint3d Origin = OdGePoint3d::kOrigin + HatchEntity->elevation() * HatchEntity->normal();
				// <tas="Pattern scaling model to world issues. Resulting hatch is very large without the world scale division"</tas>
				HatchPrimitive->SetPatternReferenceSystem(Origin, HatchEntity->normal(), HatchEntity->patternAngle(), HatchEntity->patternScale());
				break;
			}
		}
		if (HatchEntity->isGradient()) {
			ATLTRACE2(atlTraceGeneral, 0, L"Gradient Type: %i\n", HatchEntity->gradientType());
			ATLTRACE2(atlTraceGeneral, 2, L"Gradient Name: %s\n", HatchEntity->gradientName());
			ATLTRACE2(atlTraceGeneral, 2, L"Gradient Angle: %f\n", HatchEntity->gradientAngle());
			ATLTRACE2(atlTraceGeneral, 2, L"Gradient Shift: %f\n", HatchEntity->gradientShift());
			ATLTRACE2(atlTraceGeneral, 2, L"Gradient One-Color Mode: %i\n", HatchEntity->getGradientOneColorMode());
			if (HatchEntity->getGradientOneColorMode()) {
				ATLTRACE2(atlTraceGeneral, 2, L"ShadeTintValue: %f\n", HatchEntity->getShadeTintValue());
			}
			OdCmColorArray colors;
			OdGeDoubleArray values;
			HatchEntity->getGradientColors(colors, values);
			for (int i = 0; i < (int) colors.size(); i++) {
				ATLTRACE2(atlTraceGeneral, 2, L"Color: %f", colors[i]);
				ATLTRACE2(atlTraceGeneral, 2, L"Interpolation: %f", values[i]);
			}
		}
		ATLTRACE2(atlTraceGeneral, 2, L"Associated objects: %i\n", HatchEntity->associative());
		OdDbObjectIdArray AssociatedObjectIds;
		HatchEntity->getAssocObjIds(AssociatedObjectIds);
		int i;
		for (i = 0; i < (int) AssociatedObjectIds.size(); i++) {
			OdDbEntityPtr pAssoc = AssociatedObjectIds[i].safeOpenObject(OdDb::kForRead);
		}
		ATLTRACE2(atlTraceGeneral, 2, L"Seed points: %i\n", HatchEntity->numSeedPoints());
		for (i = 0; i < HatchEntity->numSeedPoints(); i++) {
			ATLTRACE2(atlTraceGeneral, 2, L"Seed point %f, %f\n", HatchEntity->getSeedPointAt(i)[0], HatchEntity->getSeedPointAt(i)[1]);
		}
		int NumberOfLoops = HatchEntity->numLoops();
		if (NumberOfLoops > 1) {
			ATLTRACE2(atlTraceGeneral, 0, L"Hatch defined using more than one loop, %i skipped\n", NumberOfLoops - 1);
		}
		for (i = 0; i < HatchEntity->numLoops(); i++) {
			ATLTRACE2(atlTraceGeneral, 2, L"Loop %i\n", HatchEntity->loopTypeAt(i));

			if (HatchEntity->loopTypeAt(i) & OdDbHatch::kPolyline) {
				if (i == 0) {
					// <tas="Only handling the first loop"</tas>
					ConvertPolylineType(i, HatchEntity, HatchPrimitive);
				}
			}
			else {
				ConvertEdgesType(i, HatchEntity, HatchPrimitive);
			}
			if (HatchEntity->associative()) {
				AssociatedObjectIds.clear();
				HatchEntity->getAssocObjIdsAt(i, AssociatedObjectIds);
				for (int j = 0; j < (int) AssociatedObjectIds.size(); j++) {
					OdDbEntityPtr pAssoc = AssociatedObjectIds[j].safeOpenObject(OdDb::kForRead);
					// pAssoc->isA(), pAssoc->getDbHandle();
				}
			}
		}
		ATLTRACE2(atlTraceGeneral, 2, L"Elevation: %f\n", HatchEntity->elevation());
		ATLTRACE2(atlTraceGeneral, 2, L"Normal: %f, %f, %f\n", HatchEntity->normal()[0], HatchEntity->normal()[1], HatchEntity->normal()[2]);
	}
};
class EoDbLeader_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
		OdDbLeaderPtr LeaderEntity = entity;
		ATLTRACE2(atlTraceGeneral, 2, L"Converting %s to primitive set ...\n", (PCTSTR) LeaderEntity->desc()->name());

		OdRxObjectPtrArray EntitySet;
		LeaderEntity->explode(EntitySet);
		int NumberOfEntities = EntitySet.size();
		for (int i = 0; i < NumberOfEntities; i++) {
			OdDbEntityPtr Entity = static_cast<OdDbEntityPtr>(EntitySet[i]);
			OdSmartPtr<EoDbConvertEntityToPrimitive> EntityConverter = Entity;
			EntityConverter->Convert(Entity, group);
		}
	}
};
class EoDbLine_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
		OdDbLinePtr Entity = entity;
		ATLTRACE2(atlTraceGeneral, 1, L"Converting %s to EoDbLine ...\n", (LPCWSTR) Entity->desc()->name());

		group->AddTail(EoDbLine::Create(Entity));
	}
};
class EoDbMInsertBlock_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
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
class EoDbMline_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbMlinePtr MlineEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) MlineEntity->desc()->name());
	}
};
class EoDbMText_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
		OdDbMTextPtr MTextEntity = entity;
		ATLTRACE2(atlTraceGeneral, 2, L"Converting %s to EoDbText ...\n", (PCTSTR) MTextEntity->desc()->name());

		OdDbObjectId TextStyleObjectId = MTextEntity->textStyle();
		OdDbTextStyleTableRecordPtr TextStyleTableRecordPtr = TextStyleObjectId.safeOpenObject(OdDb::kForRead);
		OdString FileName("Standard");
		if (TextStyleTableRecordPtr->isShapeFile()) {
			ATLTRACE2(atlTraceGeneral, 2, L"TextStyle references shape library.\n", (PCTSTR) TextStyleTableRecordPtr->desc()->name());
		}
		else {
			FileName = TextStyleTableRecordPtr->fileName();
			int nExt = FileName.reverseFind('.');
			if (nExt != - 1) {
				if (FileName.mid(nExt).compare(".shx") == 0) {
					FileName = FileName.left(nExt);
					for (int n = nExt; n < 8; n++) {
						FileName += '_';
					}
					FileName += L".ttf";
				}
			}
		}
		OdGePoint3d Location = MTextEntity->location();
		//double Width = MTextEntity->width();

		EoDb::HorizontalAlignment HorizontalAlignment;
		EoDb::VerticalAlignment VerticalAlignment;
		switch (MTextEntity->attachment()) {
		case OdDb::kTopLeft:
			HorizontalAlignment = EoDb::kAlignLeft; 
			VerticalAlignment = EoDb::kAlignTop; 
			break;
		case OdDb::kTopCenter:
			HorizontalAlignment = EoDb::kAlignCenter; 
			VerticalAlignment = EoDb::kAlignTop; 
			break;
		case OdDb::kTopRight:
			HorizontalAlignment = EoDb::kAlignRight; 
			VerticalAlignment = EoDb::kAlignTop; 
			break;
		case OdDb::kMiddleLeft:
			HorizontalAlignment = EoDb::kAlignLeft; 
			VerticalAlignment = EoDb::kAlignMiddle; 
			break;
		case OdDb::kMiddleCenter:
			HorizontalAlignment = EoDb::kAlignCenter; 
			VerticalAlignment = EoDb::kAlignMiddle; 
			break;
		case OdDb::kMiddleRight:
			HorizontalAlignment = EoDb::kAlignRight; 
			VerticalAlignment = EoDb::kAlignMiddle; 
			break;
		case OdDb::kBottomCenter:
			HorizontalAlignment = EoDb::kAlignCenter; 
			VerticalAlignment = EoDb::kAlignBottom; 
			break;
		case OdDb::kBottomRight:
			HorizontalAlignment = EoDb::kAlignRight; 
			VerticalAlignment = EoDb::kAlignBottom; 
			break;
		default:
			HorizontalAlignment = EoDb::kAlignLeft; 
			VerticalAlignment = EoDb::kAlignBottom;
		}
		OdString Contents = MTextEntity->contents();

		EoDbFontDefinition FontDefinition;
		FontDefinition.SetPrecision(EoDb::kEoTrueType);
		FontDefinition.SetFontName((PCTSTR) FileName);
		FontDefinition.SetHorizontalAlignment(HorizontalAlignment);
		FontDefinition.SetVerticalAlignment(VerticalAlignment);

		EoDbCharacterCellDefinition CharacterCellDefinition;
		CharacterCellDefinition.SetHeight(MTextEntity->textHeight());
		CharacterCellDefinition.SetRotationAngle(MTextEntity->rotation());

		OdGeVector3d XDirection;
		OdGeVector3d YDirection;
		CharCellDef_EncdRefSys(MTextEntity->normal(), CharacterCellDefinition, XDirection, YDirection);

		EoGeReferenceSystem ReferenceSystem(MTextEntity->location(), XDirection, YDirection);
		EoDbText* TextPrimitive = new EoDbText();
		TextPrimitive->SetFontDefinition(FontDefinition);
		TextPrimitive->SetReferenceSystem(ReferenceSystem);
		TextPrimitive->SetText((PCTSTR) Contents);

		ConvertEntityData(MTextEntity, TextPrimitive);
		group->AddTail(TextPrimitive);
	}
};
class EoDbOrdinateDimension_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbOrdinateDimensionPtr OrdinateDimensionEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) OrdinateDimensionEntity->desc()->name());
	}
};
class EoDbPolyFaceMesh_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbPolyFaceMeshPtr PolyFaceMeshEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) PolyFaceMeshEntity->desc()->name());
	}
};
class EoDbOle2Frame_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbOle2FramePtr Ole2FrameEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) Ole2FrameEntity->desc()->name());
	}
};
class EoDbPoint_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
		OdDbPointPtr PointEntity = entity;
		ATLTRACE2(atlTraceGeneral, 2, L"Converting %s to EoDbPoint ...\n", (PCTSTR) PointEntity->desc()->name());

		EoDbPoint* PointPrimitive = new EoDbPoint(PointEntity->position());
		PointPrimitive->SetPointDisplayMode(PointEntity->database()->getPDMODE());

		ConvertEntityData(PointEntity, PointPrimitive);
		group->AddTail(PointPrimitive);
	}
};
class EoDbPolygonMesh_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbPolygonMeshPtr PolygonMeshEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) PolygonMeshEntity->desc()->name());
	}
};
class EoDbPolyline_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
		OdDbPolylinePtr PolylineEntity = entity;

		ATLTRACE2(atlTraceGeneral, 2, L"Converting %s to EoDbPolyline ...\n", (PCTSTR) PolylineEntity->desc()->name());
		
		size_t NumberOfVertices = PolylineEntity->numVerts();

		EoDbPolyline* PolylinePrimitive = new EoDbPolyline();
		OdGePoint2d Vertex;
		for (size_t VertexIndex = 0; VertexIndex < NumberOfVertices; VertexIndex++) {
			PolylineEntity->getPointAt(VertexIndex, Vertex);
			double StartWidth;
			double EndWidth;
			PolylineEntity->getWidthsAt(VertexIndex, StartWidth, EndWidth);
			PolylinePrimitive->AppendVertex(Vertex, PolylineEntity->getBulgeAt(VertexIndex), StartWidth, EndWidth);
		}
		PolylinePrimitive->SetClosed(PolylineEntity->isClosed());
		PolylinePrimitive->SetConstantWidth(PolylineEntity->getConstantWidth());
		PolylinePrimitive->SetNormal(PolylineEntity->normal());
		PolylinePrimitive->SetElevation(PolylineEntity->elevation());

		ConvertEntityData(PolylineEntity, PolylinePrimitive);
		group->AddTail(PolylinePrimitive);
	}
};
class EoDbProxyEntity_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
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
			int NumberOfEntities = EntitySet.size();
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
class EoDbRadialDimension_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbRadialDimensionPtr RadialDimensionEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) RadialDimensionEntity->desc()->name());
	}
};
class EoDbRasterImage_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbRasterImagePtr RasterImageEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) RasterImageEntity->desc()->name());
	}
};
class EoDbRay_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbRayPtr RayEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) RayEntity->desc()->name());
	}
};
class EoDbRegion_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbRegionPtr RegionEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) RegionEntity->desc()->name());
	}
};
class EoDbRotatedDimension_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
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
class EoDbShape_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbShapePtr ShapeEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) ShapeEntity->desc()->name());
	}
};
class EoDbSolid_Converter : public EoDbConvertEntityToPrimitive {
public:
	/// <remarks>
	/// The first two points define one edge of the polygon. The third point is diagonally opposite the second
	/// If the fourth point coincides with third result is a filled triangle.
	/// else fourth point creates a quadrilateral area.
	/// </remarks>
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
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
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
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
		for (EoUInt16 n = 0; n < ControlPoints.size(); n++) {
			ATLTRACE2(atlTraceGeneral, 2, L"Control Point: %f, %f, %f\n",  ControlPoints[n]);
		}
		ATLTRACE2(atlTraceGeneral, 0, L"Number of Knots: %i\n", Knots.length());
		for (EoUInt16 n = 0; n < Knots.length(); n++) {
			ATLTRACE2(atlTraceGeneral, 0, L"Knot: %f\n", Knots[n]);
		}
		if (Rational) {
			ATLTRACE2(atlTraceGeneral, 0, L"Number of Weights: %i\n", Weights.size());
			for (EoUInt16 n = 0; n < Weights.size(); n++) {
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
class EoDbTable_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbTablePtr TableEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) TableEntity->desc()->name());
	}
};
class EoDbText_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
		OdDbTextPtr TextEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"Converting %s to EoDbText ...\n", (PCTSTR) TextEntity->desc()->name());

		OdDbObjectId TextStyleObjectId = TextEntity->textStyle();
		OdDbTextStyleTableRecordPtr TextStyleTableRecordPtr = TextStyleObjectId.safeOpenObject(OdDb::kForRead);
		OdString FileName("Standard");
		if (TextStyleTableRecordPtr->isShapeFile()) {
			ATLTRACE2(atlTraceGeneral, 2, L"TextStyle references shape library %s.\n", (PCTSTR) TextStyleTableRecordPtr->desc()->name());
		}
		else {
			FileName = TextStyleTableRecordPtr->fileName();
			int nExt = FileName.reverseFind('.');
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
		EoDb::HorizontalAlignment HorizontalAlignment;
		EoDb::VerticalAlignment VerticalAlignment;
		switch (TextEntity->verticalMode()) {
			case OdDb::kTextVertMid:
				VerticalAlignment = EoDb::kAlignMiddle;
				break;

			case OdDb::kTextTop:
				VerticalAlignment = EoDb::kAlignTop;
				break;

			default: // OdDb::kTextBottom & OdDb::kTextBase
				VerticalAlignment = EoDb::kAlignBottom;
		}
		switch (TextEntity->horizontalMode()) {
			case OdDb::kTextMid:
			case OdDb::kTextCenter:
				HorizontalAlignment = EoDb::kAlignCenter;
				break;

			case OdDb::kTextRight:
			case OdDb::kTextAlign:
			case OdDb::kTextFit:
				HorizontalAlignment = EoDb::kAlignRight;
				break;

			default: // OdDb::kTextLeft
				HorizontalAlignment = EoDb::kAlignLeft;
		}
		OdGePoint3d AlignmentPoint = TextEntity->position();
		if (HorizontalAlignment != EoDb::kAlignLeft || VerticalAlignment != EoDb::kAlignBottom)
			AlignmentPoint = TextEntity->alignmentPoint();

		EoDbFontDefinition FontDefinition;
		FontDefinition.SetPrecision(EoDb::kEoTrueType);
		FontDefinition.SetFontName((PCTSTR) FileName);
		FontDefinition.SetHorizontalAlignment(HorizontalAlignment);
		FontDefinition.SetVerticalAlignment(VerticalAlignment);

		EoDbCharacterCellDefinition CharacterCellDefinition;
		CharacterCellDefinition.SetHeight(TextEntity->height());
		CharacterCellDefinition.SetWidthFactor(TextEntity->widthFactor());
		CharacterCellDefinition.SetRotationAngle(TextEntity->rotation());
		CharacterCellDefinition.SetObliqueAngle(TextEntity->oblique());

		OdGeVector3d XDirection;
		OdGeVector3d YDirection;
		CharCellDef_EncdRefSys(TextEntity->normal(), CharacterCellDefinition, XDirection, YDirection);

		EoGeReferenceSystem ReferenceSystem(AlignmentPoint, XDirection, YDirection);
		EoDbText* TextPrimitive = new EoDbText();
		TextPrimitive->SetFontDefinition(FontDefinition);
		TextPrimitive->SetReferenceSystem(ReferenceSystem);
		TextPrimitive->SetText((LPCWSTR) TextEntity->textString());

		ConvertEntityData(TextEntity, TextPrimitive);
		group->AddTail(TextPrimitive);
	}
};
class EoDbTrace_Converter : public EoDbConvertEntityToPrimitive {
public:
	/// <remarks>
	/// A Trace entity is the exact same thing as a Solid entity
	/// </remarks>
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
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
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
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
class EoDbWipeout_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup*) {
		OdDbWipeoutPtr WipeoutEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) WipeoutEntity->desc()->name());
	}
};
class EoDbXline_Converter : public EoDbConvertEntityToPrimitive {
public:
	void Convert(OdDbEntity* entity, EoDbGroup* group) {
		OdDbXlinePtr XlineEntity = entity;
		ATLTRACE2(atlTraceGeneral, 0, L"%s was not converted ...\n", (PCTSTR) XlineEntity->desc()->name());
	}
};

class Converters {
	OdStaticRxObject<EoDb2LineAngularDimension_Converter> m_2LineAngularDimensionConverter;
	OdStaticRxObject<EoDb2dPolyline_Converter> m_2dPolylineConverter;
	OdStaticRxObject<EoDb3PointAngularDimension_Converter> m_3PointAngularDimensionConverter;
	OdStaticRxObject<EoDb3dPolyline_Converter> m_3dPolylineConverter;
	OdStaticRxObject<EoDb3dSolid_Converter> m_3dSolidConverter;
	OdStaticRxObject<EoDbAlignedDimension_Converter> m_AlignedDimensionConverter;
	OdStaticRxObject<EoDbArcAlignedText_Converter> m_ArcAlignedTextConverter;
	OdStaticRxObject<EoDbArcDimension_Converter> m_ArcDimensionConverter;
	OdStaticRxObject<EoDbArc_Converter> m_ArcConverter;
	OdStaticRxObject<EoDbAttributeDefinition_Converter> m_AttributeDefinitionConverter;
	OdStaticRxObject<EoDbBlockReference_Converter> m_BlockReference;
	OdStaticRxObject<EoDbBody_Converter> m_BodyConverter;
	OdStaticRxObject<EoDbCircle_Converter> m_CircleConverter;
	OdStaticRxObject<EoDbDiametricDimension_Converter> m_DiametricDimensionConverter;
	OdStaticRxObject<EoDbEllipse_Converter> m_EllipseConverter;
	OdStaticRxObject<EoDbConvertEntityToPrimitive> m_EntityConverter;
	OdStaticRxObject<EoDbFace_Converter> m_FaceConverter;
	OdStaticRxObject<EoDbFcf_Converter> m_FcfConverter;
	OdStaticRxObject<EoDbHatch_Converter> m_HatchConverter;
	OdStaticRxObject<EoDbLeader_Converter> m_LeaderConverter;
	OdStaticRxObject<EoDbLine_Converter> m_LineConverter;
	OdStaticRxObject<EoDbMInsertBlock_Converter> m_MInsertBlock;
	OdStaticRxObject<EoDbMText_Converter> m_MTextConverter;
	OdStaticRxObject<EoDbMline_Converter> m_MlineConverter;
	OdStaticRxObject<EoDbOle2Frame_Converter> m_Ole2FrameConverter;
	OdStaticRxObject<EoDbOrdinateDimension_Converter> m_OrdinateDimensionConverter;
	OdStaticRxObject<EoDbPoint_Converter> m_PointConverter;
	OdStaticRxObject<EoDbPolyFaceMesh_Converter> m_PolyFaceMeshConverter;
	OdStaticRxObject<EoDbPolygonMesh_Converter> m_PolygonMesh;
	OdStaticRxObject<EoDbPolyline_Converter> m_PolylineConverter;
	OdStaticRxObject<EoDbProxyEntity_Converter> m_ProxyEntityConverter;
	OdStaticRxObject<EoDbRadialDimension_Converter> m_RadialDimensionConverter;
	OdStaticRxObject<EoDbRasterImage_Converter> m_RasterImageConverter;
	OdStaticRxObject<EoDbRay_Converter> m_RayConverter;
	OdStaticRxObject<EoDbRegion_Converter> m_RegionConverter;
	OdStaticRxObject<EoDbRotatedDimension_Converter> m_RotatedDimensionConverter;
	OdStaticRxObject<EoDbShape_Converter> m_ShapeConverter;
	OdStaticRxObject<EoDbSolid_Converter> m_SolidConverter;
	OdStaticRxObject<EoDbSpline_Converter> m_SplineConverter;
	OdStaticRxObject<EoDbTable_Converter> m_TableConverter;
	OdStaticRxObject<EoDbText_Converter> m_TextConverter;
	OdStaticRxObject<EoDbTrace_Converter> m_TraceConverter;
	OdStaticRxObject<EoDbViewport_Converter> m_ViewportConverter;
	OdStaticRxObject<EoDbWipeout_Converter> m_WipeoutConverter;
	OdStaticRxObject<EoDbXline_Converter> m_XlineConverter;

public:
	void AddExtensions() {
		OdDb2LineAngularDimension::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_2LineAngularDimensionConverter);
		OdDb2dPolyline::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_2dPolylineConverter);
		OdDb3PointAngularDimension::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_3PointAngularDimensionConverter);
		OdDb3dPolyline::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_3dPolylineConverter);
		OdDb3dSolid::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_3dSolidConverter);
		OdDbAlignedDimension::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_AlignedDimensionConverter);
		OdDbArc::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_ArcConverter);
		OdDbArcAlignedText::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_ArcAlignedTextConverter);
		OdDbArcDimension::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_ArcDimensionConverter);
		OdDbAttributeDefinition::desc()->addX(EoDbConvertEntityToPrimitive::desc(),	 &m_AttributeDefinitionConverter);
		OdDbBlockReference::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_BlockReference);
		OdDbBody::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_BodyConverter);
		OdDbCircle::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_CircleConverter);
		OdDbDiametricDimension::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_DiametricDimensionConverter);
		OdDbEllipse::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_EllipseConverter);
		OdDbEntity::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_EntityConverter);
		OdDbFace::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_FaceConverter);
		OdDbFcf::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_FcfConverter);
		OdDbHatch::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_HatchConverter);
		OdDbLeader::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_LeaderConverter);
		OdDbLine::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_LineConverter);
		OdDbMInsertBlock::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_MInsertBlock);
		OdDbMText::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_MTextConverter);
		OdDbMline::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_MlineConverter);
		OdDbOle2Frame::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_Ole2FrameConverter);
		OdDbOrdinateDimension::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_OrdinateDimensionConverter);
		OdDbPoint::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_PointConverter);
		OdDbPolyFaceMesh::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_PolyFaceMeshConverter);
		OdDbPolygonMesh::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_PolygonMesh);
		OdDbPolyline::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_PolylineConverter);
		OdDbProxyEntity::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_ProxyEntityConverter);
		OdDbRadialDimension::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_RadialDimensionConverter);
		OdDbRasterImage::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_RasterImageConverter);
		OdDbRay::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_RayConverter);
		OdDbRegion::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_RegionConverter);
		OdDbRotatedDimension::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_RotatedDimensionConverter);
		OdDbShape::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_ShapeConverter);
		OdDbSolid::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_SolidConverter);
		OdDbSpline::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_SplineConverter);
		OdDbTable::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_TableConverter);
		OdDbText::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_TextConverter);
		OdDbTrace::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_TraceConverter);
		OdDbViewport::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_ViewportConverter);
		OdDbWipeout::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_WipeoutConverter);
		OdDbXline::desc()->addX(EoDbConvertEntityToPrimitive::desc(), &m_XlineConverter);
	}
	void DeleteExtensions() {
		OdDb2LineAngularDimension::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDb2dPolyline::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDb3PointAngularDimension::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDb3dPolyline::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDb3dSolid::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbAlignedDimension::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbArc::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbArcAlignedText::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbArcDimension::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbAttributeDefinition	::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbBlockReference::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbBody::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbCircle::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbDiametricDimension::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbEllipse::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbEntity::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbFace::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbFcf::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbHatch::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbLeader::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbLine::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbMInsertBlock::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbMText::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbMline::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbOle2Frame::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbOrdinateDimension::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbPoint::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbPolyFaceMesh::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbPolygonMesh::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbPolyline::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbProxyEntity::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbRadialDimension::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbRasterImage::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbRay::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbRegion::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbRotatedDimension::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbShape::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbSolid::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbSpline::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbTable::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbText::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbTrace::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbViewport::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbWipeout::desc()->delX(EoDbConvertEntityToPrimitive::desc());
		OdDbXline::desc()->delX(EoDbConvertEntityToPrimitive::desc());
	}
};
AeSysDoc* ConvertEntityToPrimitiveProtocolExtension::m_Document = NULL;

ConvertEntityToPrimitiveProtocolExtension::ConvertEntityToPrimitiveProtocolExtension(AeSysDoc* document) {
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
