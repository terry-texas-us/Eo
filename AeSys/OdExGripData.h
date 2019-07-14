// Extracted class from Examples\Editor\ExGripManager.h (last compare 20.5)
#pragma once
#include "ExGripManager.h"
class OdBaseGripManager;
class OdExGripData;
using OdExGripDataPtr = OdSmartPtr<class OdExGripData>;

class OdExGripData : public OdGiDrawableImpl<> {
public:
	OdExGripData() = default;

	~OdExGripData();

	static OdExGripDataPtr CreateObject(OdDbStub* id, const OdDbGripDataPtr& gripData, const OdGePoint3d& point, OdBaseGripManager* gripManager);

	static OdExGripDataPtr CreateObject(const OdDbBaseFullSubentPath& entityPath, const OdDbGripDataPtr& gripData, const OdGePoint3d& point, OdBaseGripManager* gripManager);

	unsigned long subSetAttributes(OdGiDrawableTraits* drawableTraits) const override;

	bool subWorldDraw(OdGiWorldDraw* worldDraw) const override;

	void subViewportDraw(OdGiViewportDraw* viewportDraw) const override;

	[[nodiscard]] OdDbGripOperations::DrawType Status() const noexcept {
		return m_Status;
	}

	[[nodiscard]] bool IsInvisible() const noexcept {
		return m_Invisible;
	}

	[[nodiscard]] bool IsShared() const noexcept {
		return m_Shared;
	}

	[[nodiscard]] OdGePoint3d Point() const noexcept {
		return m_Point;
	}

	[[nodiscard]] OdDbGripDataPtr GripData() const {
		return m_GripData;
	}

	[[nodiscard]] OdDbStub* EntityId() const {
		return m_SubentPath.objectIds().last();
	}

	bool EntityPath(OdDbBaseFullSubentPath* path = nullptr) const {
		if (path != nullptr) {
			*path = m_SubentPath;
		}
		return m_SubentPath.subentId() != OdDbSubentId();
	}

	void SetStatus(const OdDbGripOperations::DrawType status) noexcept {
		m_Status = status;
	}

	void SetInvisible(const bool invisible) noexcept {
		m_Invisible = invisible;
	}

	void SetShared(const bool shared) noexcept {
		m_Shared = shared;
	}

private:
	bool ComputeDragPoint(OdGePoint3d& computedPoint) const;

	OdDbGripOperations::DrawType m_Status {OdDbGripOperations::kWarmGrip};
	bool m_Invisible {false};
	bool m_Shared {false};
	OdGePoint3d m_Point {OdGePoint3d::kOrigin};
	OdDbGripDataPtr m_GripData;
	OdDbBaseFullSubentPath m_SubentPath;
	OdBaseGripManager* m_GripManager {nullptr};
};
