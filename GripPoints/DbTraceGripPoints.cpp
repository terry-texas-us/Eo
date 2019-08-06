#include <OdaCommon.h>
#include <DbTrace.h>
#include <DbSolid.h>
#include "DbTraceGripPoints.h"

class _DbTraceGPRedir {
public:
	virtual void getPointAt(int nPt, OdGePoint3d& pPt) const = 0;

	virtual void setPointAt(int nPt, const OdGePoint3d& pPt) = 0;

	virtual double thickness() const = 0;

	virtual OdGeVector3d normal() const = 0;
};

template <class classPtr>
class _DbTraceGPRedirImpl : public _DbTraceGPRedir {
	classPtr pTrace;
public:
	_DbTraceGPRedirImpl(const OdDbEntity* entity)
		: pTrace(entity) { }

	_DbTraceGPRedirImpl(OdDbEntity* entity)
		: pTrace(entity) { }

	void getPointAt(int nPt, OdGePoint3d& pPt) const override {
		pTrace->getPointAt(nPt, pPt);
	}

	void setPointAt(int nPt, const OdGePoint3d& pPt) override {
		pTrace->setPointAt(nPt, pPt);
	}

	double thickness() const override {
		return pTrace->thickness();
	}

	OdGeVector3d normal() const override {
		return pTrace->normal();
	}
};

OdResult OdDbTraceGripPointsPE_Base::getGripPoints(const _DbTraceGPRedir* trace, OdGePoint3dArray& gripPoints) const {
	gripPoints.resize(4);
	int nPt;
	for (nPt = 0; nPt < 4; nPt++) {
		trace->getPointAt(nPt, gripPoints[nPt]);
	}
	if (OdNonZero(trace->thickness())) {
		const auto thkPath(trace->normal().normal() * trace->thickness());
		gripPoints.resize(8);
		for (nPt = 4; nPt < 8; nPt++) {
			gripPoints[nPt] = gripPoints[nPt - 4] + thkPath;
		}
	}
	return eOk;
}

OdResult OdDbTraceGripPointsPE_Base::moveGripPointsAt(_DbTraceGPRedir* trace, const OdIntArray& indices, const OdGeVector3d& offset) {
	OdGeVector3d planarBasis[3];
	double planarCoefs[3];
	int nPt;
	planarBasis[2] = trace->normal();
	planarBasis[0] = planarBasis[2].perpVector().normal();
	planarBasis[1] = planarBasis[2].crossProduct(planarBasis[0]).normal();
	for (nPt = 0; nPt < 3; nPt++) {
		planarCoefs[nPt] = planarBasis[nPt].dotProduct(offset);
	}
	if (OdNonZero(planarCoefs[0]) || OdNonZero(planarCoefs[1])) {
		bool bMarkIds[4] = {false, false, false, false};
		const auto delta {planarBasis[0] * planarCoefs[0] + planarBasis[1] * planarCoefs[1]};
		for (nPt = 0; nPt < (int)indices.size(); nPt++) {
			bMarkIds[indices[nPt] % 4] = true;
		}
		for (nPt = 0; nPt < 4; nPt++) {
			if (bMarkIds[nPt]) {
				OdGePoint3d movePt;
				trace->getPointAt(nPt, movePt);
				movePt += delta;
				trace->setPointAt(nPt, movePt);
			}
		}
	}
	if (OdNonZero(planarCoefs[2])) {
		const auto delta {planarBasis[2] * planarCoefs[2]};
		for (nPt = 0; nPt < 4; nPt++) {
			OdGePoint3d movePt;
			trace->getPointAt(nPt, movePt);
			movePt += delta;
			trace->setPointAt(nPt, movePt);
		}
	}
	return eOk;
}

OdResult OdDbTraceGripPointsPE_Base::getStretchPoints(const _DbTraceGPRedir* trace, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(trace, stretchPoints);
}

OdResult OdDbTraceGripPointsPE_Base::moveStretchPointsAt(_DbTraceGPRedir* trace, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(trace, indices, offset);
}

OdResult OdDbTraceGripPointsPE_Base::getOsnapPoints(const _DbTraceGPRedir* trace, OdDb::OsnapMode objectSnapMode, OdGsMarker /*gsSelectionMark*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*xWorldToEye*/, OdGePoint3dArray& snapPoints) const {
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd: {
			const OdUInt32 nSnaps {snapPoints.size()};
			snapPoints.resize(nSnaps + 4);
			for (auto nPt = 0; nPt < 4; nPt++) {
				trace->getPointAt(nPt, snapPoints[nSnaps + nPt]);
			}
		}
		break;
		default:
			break;
	}
	return eOk;
} // TracePE
OdResult OdDbTraceGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	_DbTraceGPRedirImpl<OdDbTracePtr> redir(entity);
	return OdDbTraceGripPointsPE_Base::getGripPoints(&redir, gripPoints);
}

OdResult OdDbTraceGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	_DbTraceGPRedirImpl<OdDbTracePtr> redir(entity);
	return OdDbTraceGripPointsPE_Base::moveGripPointsAt(&redir, indices, offset);
}

OdResult OdDbTraceGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	_DbTraceGPRedirImpl<OdDbTracePtr> redir(entity);
	return OdDbTraceGripPointsPE_Base::getStretchPoints(&redir, stretchPoints);
}

OdResult OdDbTraceGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	_DbTraceGPRedirImpl<OdDbTracePtr> redir(entity);
	return OdDbTraceGripPointsPE_Base::moveStretchPointsAt(&redir, indices, offset);
}

OdResult OdDbTraceGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker gsSelectionMark, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& viewXform, OdGePoint3dArray& snapPoints) const {
	_DbTraceGPRedirImpl<OdDbTracePtr> redir(entity);
	return OdDbTraceGripPointsPE_Base::getOsnapPoints(&redir, objectSnapMode, gsSelectionMark, pickPoint, lastPoint, viewXform, snapPoints);
} // SolidPE
OdResult OdDbSolidGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	_DbTraceGPRedirImpl<OdDbSolidPtr> redir(entity);
	return OdDbTraceGripPointsPE_Base::getGripPoints(&redir, gripPoints);
}

OdResult OdDbSolidGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	_DbTraceGPRedirImpl<OdDbSolidPtr> redir(entity);
	return OdDbTraceGripPointsPE_Base::moveGripPointsAt(&redir, indices, offset);
}

OdResult OdDbSolidGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	_DbTraceGPRedirImpl<OdDbSolidPtr> redir(entity);
	return OdDbTraceGripPointsPE_Base::getStretchPoints(&redir, stretchPoints);
}

OdResult OdDbSolidGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	_DbTraceGPRedirImpl<OdDbSolidPtr> redir(entity);
	return OdDbTraceGripPointsPE_Base::moveStretchPointsAt(&redir, indices, offset);
}

OdResult OdDbSolidGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker gsSelectionMark, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& viewXform, OdGePoint3dArray& snapPoints) const {
	_DbTraceGPRedirImpl<OdDbSolidPtr> redir(entity);
	return OdDbTraceGripPointsPE_Base::getOsnapPoints(&redir, objectSnapMode, gsSelectionMark, pickPoint, lastPoint, viewXform, snapPoints);
}
