#pragma once
#include <Gi/GiDrawableImpl.h>
#include "ExGripManager.h"
class OdBaseGripManager;
using OdExGripDragPtr = OdSmartPtr<class OdExGripDrag>;

class OdExGripDrag : public OdGiDrawableImpl<> {
public:
	OdExGripDrag() noexcept;

	~OdExGripDrag() = default;

	static OdExGripDragPtr CreateObject(OdDbStub* id, OdBaseGripManager* gripManager);

	static OdExGripDragPtr CreateObject(const OdDbBaseFullSubentPath& entityPath, OdBaseGripManager* gripManager);

	unsigned long subSetAttributes(OdGiDrawableTraits* drawableTraits) const override;

	bool subWorldDraw(OdGiWorldDraw* worldDraw) const override;

	void subViewportDraw(OdGiViewportDraw* viewportDraw) const override;

	void CloneEntity();

	void CloneEntity(const OdGePoint3d& ptMoveAt);

	void MoveEntity(const OdGePoint3d& ptMoveAt);

	void NotifyDragStarted() const;

	void NotifyDragEnded() const;

	void NotifyDragAborted() const;

	[[nodiscard]] OdDbStub* EntityId() const;

	bool EntityPath(OdDbBaseFullSubentPath* subentPath = nullptr) const;

protected:
	bool LocateActiveGrips(OdIntArray& indices) const;

	OdDbBaseFullSubentPath m_SubentPath;
	OdGiDrawablePtr m_Clone;
	OdBaseGripManager* m_GripManager;
};
