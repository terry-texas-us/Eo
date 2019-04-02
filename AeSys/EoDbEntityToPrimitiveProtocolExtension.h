#pragma once

#include "RxObject.h"

class EoDbConvertEntityToPrimitive : public OdRxObject {
public:
  ODRX_DECLARE_MEMBERS(EoDbConvertEntityToPrimitive);

  virtual void Convert(OdDbEntity* entity, EoDbGroup* group);
};
class Converters;

/// <summary> This class is the protocol extension class for all entity Converters</summary>
class ConvertEntityToPrimitiveProtocolExtension {
	Converters* m_Converters;
public:
	static AeSysDoc* m_Document;

	ConvertEntityToPrimitiveProtocolExtension(AeSysDoc* document) noexcept;
	virtual ~ConvertEntityToPrimitiveProtocolExtension();
	void Initialize();
	void Uninitialize();
};
