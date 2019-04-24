#include "stdafx.h"

#include "AeSysApp.h"

#include "Gi/GiDrawableImpl.h"
#include "Gi/GiWorldDraw.h"
#include "Gs/Gs.h"
#include "Gs/GsBaseVectorizer.h"
#include "EoExEditorObject.h"
#include "GiContextForDbDatabase.h"
#include "DbLayout.h"
#include "DbCommandContext.h"
#include "DbAbstractViewportData.h"
#include "DbHostAppServices.h"
#include "OdDToStr.h"
#include "SaveState.h"

static const int SNAP_SIZE = 10;

void setWorkingSelectionSet(OdDbCommandContext* commandContext, OdDbSelectionSet* selectionSet) {
    commandContext->setArbitraryData(L"OdaMfcApp Working Selection Set", selectionSet);
}
OdDbSelectionSetPtr workingSelectionSet(OdDbCommandContext* commandContext) {
    OdDbSelectionSetPtr pRes;
    if (commandContext) {
        pRes = commandContext->arbitraryData(L"OdaMfcApp Working Selection Set");
        if (pRes.isNull()) {
            pRes = OdDbSelectionSet::createObject(commandContext->database());
            setWorkingSelectionSet(commandContext, pRes);
        }
    }
    return pRes;
}
class XFormDrawable : public OdGiDrawableImpl<OdGiDrawable> {
    OdGiDrawablePtr m_pDrawable;
    const OdGeMatrix3d* m_pXForm;
protected:
    XFormDrawable() :
        m_pXForm(0) {}
public:
    static OdGiDrawablePtr createObject(OdGiDrawable* drawable, const OdGeMatrix3d& xForm) {
        OdSmartPtr<XFormDrawable> pRes = OdRxObjectImpl<XFormDrawable>::createObject();
        pRes->m_pDrawable = drawable;
        pRes->m_pXForm = &xForm;
        return pRes;
    }
    OdUInt32 subSetAttributes(OdGiDrawableTraits* traits) const noexcept override {
        return kDrawableUsesNesting;
    }
    bool subWorldDraw(OdGiWorldDraw* worldDraw) const override {
        OdGiModelTransformSaver mt(worldDraw->geometry(), *m_pXForm);
        worldDraw->geometry().draw(m_pDrawable);
        return true;
    }
    void subViewportDraw(OdGiViewportDraw* viewportDraw) const noexcept override {}
};

EoExEditorObject::EoExEditorObject() :
    m_bSnapOn(true), m_pCmdCtx(0) {}
void EoExEditorObject::initialize(OdGsDevice* device, OdDbCommandContext* commandContext) {
    m_pDevice = device;
    m_pCmdCtx = commandContext;

    m_p2dModel = device->createModel();

    if (!m_p2dModel.isNull()) {
        m_p2dModel->setRenderType(OdGsModel::kDirect);
    }
    m_gripManager.init(device, m_p2dModel, commandContext, workingSelectionSet);
}
void EoExEditorObject::uninitialize() {
    OdDbSelectionSetPtr SelectionSet = workingSSet();
    if (SelectionSet.get()) {
        SelectionSet->clear();
        m_gripManager.selectionSetChanged(SelectionSet);
    }
    m_gripManager.uninit();

    m_pDevice.release();
    m_pCmdCtx = 0;
}
void EoExEditorObject::initSnapping(OdGsView* view) {
    view->add(&m_osnapManager, m_p2dModel);
}
void EoExEditorObject::uninitSnapping(OdGsView* view) {
    view->erase(&m_osnapManager);
}
OdDbSelectionSetPtr EoExEditorObject::workingSSet() const {
    return workingSelectionSet(m_pCmdCtx);
}
void EoExEditorObject::setWorkingSSet(OdDbSelectionSet* selectionSet) {
    setWorkingSelectionSet(m_pCmdCtx, selectionSet);
}
void EoExEditorObject::selectionSetChanged() {
    m_gripManager.selectionSetChanged(workingSSet());
}
const OdGsView* EoExEditorObject::activeView() const {
    return m_pDevice->activeView();
}
OdGsView* EoExEditorObject::activeView() {
    return m_pDevice->activeView();
}
const OdGsView* EoExEditorObject::activeTopView() const {
    const OdGsView* ActiveView(activeView());
    if (!m_pCmdCtx->database()->getTILEMODE()) {
        OdDbObjectPtr ActiveViewportObject = m_pCmdCtx->database()->activeViewportId().safeOpenObject();
        OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewportObject);
        if (!AbstractViewportData.isNull() && AbstractViewportData->gsView(ActiveViewportObject)) {
            ActiveView = AbstractViewportData->gsView(ActiveViewportObject);
        }
    }
    return (ActiveView);
}
OdGsView* EoExEditorObject::activeTopView() {
    OdGsView* ActiveView = activeView();
    if (!m_pCmdCtx->database()->getTILEMODE()) {
        OdDbObjectPtr ActiveViewportObject = m_pCmdCtx->database()->activeViewportId().safeOpenObject();
        OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewportObject);
        if (!AbstractViewportData.isNull() && AbstractViewportData->gsView(ActiveViewportObject)) {
            ActiveView = AbstractViewportData->gsView(ActiveViewportObject);
        }
    }
    return (ActiveView);
}
OdDbObjectId EoExEditorObject::activeVpId() const {
    OdGsClientViewInfo ClientViewInfo;
    ((OdGsView*)activeView())->clientViewInfo(ClientViewInfo);
    return OdDbObjectId(ClientViewInfo.viewportObjectId);
}
void EoExEditorObject::ucsPlane(OdGePlane& plane) const {
    OdDbObjectPtr ActiveViewportObject = activeVpId().safeOpenObject();
    OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewportObject);
    OdGePoint3d ucsOrigin;
    OdGeVector3d ucsXAxis;
    OdGeVector3d ucsYAxis;
    AbstractViewportData->getUcs(ActiveViewportObject, ucsOrigin, ucsXAxis, ucsYAxis);
    plane.set(ucsOrigin, ucsXAxis, ucsYAxis);
}
OdGePoint3d EoExEditorObject::toEyeToWorld(int x, int y) const {
    OdGePoint3d wcsPt(x, y, 0.0);
    const OdGsView* ActiveView = activeView();
    if (ActiveView->isPerspective()) {
        wcsPt.z = ActiveView->projectionMatrix()(2, 3);
    }
    wcsPt.transformBy((ActiveView->screenMatrix() * ActiveView->projectionMatrix()).inverse());
    wcsPt.z = 0.;
    // eye CS at this point.

    wcsPt.transformBy(OdAbstractViewPEPtr(ActiveView)->eyeToWorld(ActiveView));
    return wcsPt;
}
bool EoExEditorObject::toUcsToWorld(OdGePoint3d& wcsPt) const {
    const OdGsView* ActiveView = activeView();
    OdGeLine3d Line(wcsPt, OdAbstractViewPEPtr(ActiveView)->direction(ActiveView));
    OdGePlane Plane;
    ucsPlane(Plane);
    return Plane.intersectWith(Line, wcsPt);
}
OdGePoint3d EoExEditorObject::toScreenCoord(int x, int y) const {
    OdGePoint3d scrPt(x, y, 0.0);
    const OdGsView* ActiveView = activeView();
    scrPt.transformBy((ActiveView->screenMatrix() * ActiveView->projectionMatrix()).inverse());
    scrPt.z = 0.;
    return scrPt;
}
OdGePoint3d EoExEditorObject::toScreenCoord(const OdGePoint3d& wcsPt) const {
    // To DCS
    OdGePoint3d scrPt(wcsPt);
    const OdGsView* ActiveView = activeView();
    OdGsClientViewInfo ClientViewInfo;
    ActiveView->clientViewInfo(ClientViewInfo);
    OdRxObjectPtr pObj = OdDbObjectId(ClientViewInfo.viewportObjectId).openObject();
    OdAbstractViewPEPtr pVp(pObj);
    const OdGeVector3d vecY = pVp->upVector(pObj);
    const OdGeVector3d vecZ = pVp->direction(pObj);
    const OdGeVector3d vecX = vecY.crossProduct(vecZ).normal();
    const OdGeVector2d offset = pVp->viewOffset(pObj);
    const OdGePoint3d prTarg = pVp->target(pObj) - vecX * offset.x - vecY * offset.y;
    scrPt.x = vecX.dotProduct(wcsPt - prTarg);
    scrPt.y = vecY.dotProduct(wcsPt - prTarg);
    scrPt.z = 0.;
    return scrPt;
}
unsigned EoExEditorObject::getSnapModes() const noexcept {
    return m_osnapManager.snapModes();
}
void EoExEditorObject::resetSnapManager() {
    m_osnapManager.reset();
}
void EoExEditorObject::setSnapModes(bool snapOn, unsigned snapModes) noexcept {
    m_bSnapOn = snapOn;
    m_osnapManager.setSnapModes(snapModes);
}
OdGiDrawablePtr EoExEditorObject::snapDrawable() const {
    return &m_osnapManager;
}
OdEdCommandPtr EoExEditorObject::command(const OdString & commandName) {
    if (commandName == m_cmd_ZOOM.globalName()) {
        return &m_cmd_ZOOM;
    }
    if (commandName == m_cmd_3DORBIT.globalName()) {
        return &m_cmd_3DORBIT;
    }
    if (commandName == m_cmd_DOLLY.globalName()) {
        return &m_cmd_DOLLY;
    }
    return OdEdCommandPtr();
}
void EoExEditorObject::set3DView(_3DViewType type) {
    OdGePoint3d Position;
    OdGePoint3d Target(OdGePoint3d::kOrigin);
    OdGeVector3d Axis;
    switch (type) {
    case k3DViewTop:
        Position = OdGePoint3d::kOrigin + OdGeVector3d::kZAxis;
        Axis = OdGeVector3d::kYAxis;
        break;
    case k3DViewBottom:
        Position = OdGePoint3d::kOrigin - OdGeVector3d::kZAxis;
        Axis = OdGeVector3d::kYAxis;
        break;
    case k3DViewLeft:
        Position = OdGePoint3d::kOrigin - OdGeVector3d::kXAxis;
        Axis = OdGeVector3d::kZAxis;
        break;
    case k3DViewRight:
        Position = OdGePoint3d::kOrigin + OdGeVector3d::kXAxis;
        Axis = OdGeVector3d::kZAxis;
        break;
    case k3DViewFront:
        Position = OdGePoint3d::kOrigin - OdGeVector3d::kYAxis;
        Axis = OdGeVector3d::kZAxis;
        break;
    case k3DViewBack:
        Position = OdGePoint3d::kOrigin + OdGeVector3d::kYAxis;
        Axis = OdGeVector3d::kZAxis;
        break;
    };
    unselect();

    OdGsView* ActiveView = activeView();

    OdGsClientViewInfo ClientViewInfo;
    ActiveView->clientViewInfo(ClientViewInfo);
    OdDbObjectPtr pObject = OdDbObjectId(ClientViewInfo.viewportObjectId).safeOpenObject(OdDb::kForWrite);
    OdAbstractViewPEPtr(pObject)->setUcs(pObject, Target, Axis.crossProduct(Position.asVector()), Axis);

    const OdGsView::Projection ProjectionType((ActiveView->isPerspective()) ? OdGsView::kPerspective : OdGsView::kParallel);
    ActiveView->setView(Position, Target, Axis, ActiveView->fieldWidth(), ActiveView->fieldHeight(), ProjectionType);
}
bool EoExEditorObject::snap(OdGePoint3d & point, const OdGePoint3d * lastPoint) {
    if (m_bSnapOn) {
        if (m_osnapManager.snap(activeView(), point, lastPoint)) {
            if (!m_p2dModel.isNull()) {
                m_p2dModel->onModified(&m_osnapManager, (OdGiDrawable*)0);
            }
            return true;
        }
    }
    return false;
}
bool EoExEditorObject::unselect() {
    bool bRes = false;
    OdDbSelectionSetPtr SelectionSet = workingSSet();
    OdDbSelectionSetIteratorPtr SelectionSetIterator = SelectionSet->newIterator();
    while (!SelectionSetIterator->done()) {
        OdDbEntityPtr Entity = OdDbEntity::cast(SelectionSetIterator->objectId().openObject());
        if (Entity.get()) {
            Entity->highlight(false);
            bRes = true;
        }
        SelectionSetIterator->next();
    }
    SelectionSet = OdDbSelectionSet::createObject(SelectionSet->database());
    setWorkingSelectionSet(m_pCmdCtx, SelectionSet);
    m_gripManager.selectionSetChanged(SelectionSet);
    return bRes;
}
bool EoExEditorObject::OnCtrlClick() {
    return m_gripManager.onControlClick();
}
bool EoExEditorObject::OnMouseLeftButtonClick(unsigned int flags, int x, int y, OleDragCallback * dragCallback) {
    const bool ShiftIsDown = (OdEdBaseIO::kShiftIsDown & flags) != 0;
    const bool ControlIsDown = (OdEdBaseIO::kControlIsDown & flags) != 0;
    const OdGePoint3d pt = toEyeToWorld(x, y);

    if (m_gripManager.onMouseDown(x, y, ShiftIsDown)) {
        return true;
    }
    try {
        // Should be here I guess.
        if (dragCallback && !ShiftIsDown) {
            OdDbSelectionSetPtr pWorkSet = workingSSet();
            OdDbSelectionSetPtr pAtPointSet = OdDbSelectionSet::select(activeVpId(), 1, &pt, OdDbVisualSelection::kPoint, ControlIsDown ? OdDbVisualSelection::kEnableSubents : OdDbVisualSelection::kDisableSubents);
            OdDbSelectionSetIteratorPtr pIter = pAtPointSet->newIterator();
            while (!pIter->done()) {
                if (pWorkSet->isMember(pIter->objectId()) && !ControlIsDown) {
                    pIter.release();
                    break;
                }
                pIter->next();
            }
            if (pIter.isNull()) {
                if (dragCallback->beginDragCallback(pt)) {
                    // Not good idea to clear selection set if already selected object has been selected, but if selection set is being cleared - items must be unhighlighted too.
                    //workingSSet()->clear();
                    //selectionSetChanged();
                    unselect();
                    return(true);
                }
            }
        }
    } catch (const OdError & Error) {
        theApp.reportError(L"", Error);
        return (false);
    }
    OdDbUserIO* UserIO = m_pCmdCtx->dbUserIO();
    UserIO->setLASTPOINT(pt);
    UserIO->setPickfirst(0);

    int Options(OdEd::kSelPickLastPoint | OdEd::kSelSinglePass | OdEd::kSelLeaveHighlighted | OdEd::kSelAllowInactSpaces);
    if (ShiftIsDown) {
        if (m_pCmdCtx->database()->appServices()->getPICKADD() > 0) {
            Options |= OdEd::kSelRemove;
        }
    } else {
        if (m_pCmdCtx->database()->appServices()->getPICKADD() == 0) {
            unselect();
        }
    }
    if (ControlIsDown) {
        Options |= OdEd::kSelAllowSubents;
    }

    OdDbSelectionSetPtr SelectionSet;
    try {
        OdSaveState<bool> ss_m_bSnapOn(m_bSnapOn, false);
        SelectionSet = UserIO->select(OdString::kEmpty, Options, workingSSet());
        setWorkingSSet(SelectionSet);
    } catch (const OdError&) {
        return(false);
    }
    selectionSetChanged();

    return true;
}
bool EoExEditorObject::OnMouseLeftButtonDoubleClick(unsigned int flags, int x, int y) {
    const OdGsView* ActiveView = activeView();
    m_pDevice->setActiveViewport(OdGePoint2d(x, y));
    const bool bChanged = ActiveView != activeView();
    if (bChanged) {
        // @@@ probably move this code to GsLayoutHelper's?
        OdDbObjectPtr pObj = activeVpId().safeOpenObject();
        const OdDbDatabase* pDb = pObj->database();
        if (pDb->getTILEMODE()) {
            OdDbViewportTable::cast(pDb->getViewportTableId().safeOpenObject(OdDb::kForWrite))->SetActiveViewport(activeVpId());
        } else {
            OdDbLayout::cast(OdDbBlockTableRecord::cast(pDb->getPaperSpaceId().safeOpenObject())->getLayoutId().safeOpenObject(OdDb::kForWrite))->setActiveViewportId(activeVpId());
        }
        unselect();
    }
    return bChanged;
}
bool EoExEditorObject::OnMouseRightButtonDoubleClick(unsigned int nFlags, int x, int y) {
    unselect();

    OdGsView* pView = activeView();

    // set target to center of the scene, keep view direction:
    const OdGePoint3d targ = pView->target();

    pView->setView(targ + OdGeVector3d::kZAxis, targ, OdGeVector3d::kYAxis, pView->fieldWidth(), pView->fieldHeight());

    return true;
}
bool EoExEditorObject::OnMouseMove(unsigned int flags, int x, int y) {
    return m_gripManager.onMouseMove(x, y);
    //  if (hasSelection() && m_pGripPoints.get()) {
    //    OdGsView* pView = activeView();
    //    bool bRes = m_pGripPoints->onMouseMove(OdGePoint2d(x, y), pView->worldToDeviceMatrix());
    //    if (bRes) {
    //#pragma MARKMESSAGE(Rect of active grip point should be passed in OdGsView::invalidate())
    //      pView->invalidate();
    //    }
    //    return bRes;
    //  }
    //  return false;
}

void EoExEditorObject::dolly(int x, int y) {
    OdGsView* ActiveView = activeView();
    OdGeVector3d DollyVector(-x, -y, 0.);
    DollyVector.transformBy((ActiveView->screenMatrix() * ActiveView->projectionMatrix()).inverse());
    ActiveView->dolly(DollyVector);
}

bool EoExEditorObject::OnMouseWheel(unsigned int nFlags, int x, int y, short zDelta) {
    OdGsView* ActiveView = activeView();

    OdGePoint3d Position(ActiveView->position());
    Position.transformBy(ActiveView->worldToDeviceMatrix());
    // In 2d mode perspective zoom change lens length instead of fieldWidth/fieldHeight. This is non-standard mode. Practically 2d mode can't be perspective.
    if (ActiveView->isPerspective() && ActiveView->mode() == OdGsView::k2DOptimized) {
        Position = OdGePoint3d(0.5, 0.5, 0.0).transformBy(ActiveView->screenMatrix());
    }
    int vx((int)OdRound(Position.x));
    int vy((int)OdRound(Position.y));
    vx = x - vx;
    vy = y - vy;
    dolly(-vx, -vy);
    ActiveView->zoom(zDelta > 0 ? 1. / .9 : .9);
    dolly(vx, vy);

    if (!m_p2dModel.isNull()) {
        m_p2dModel->invalidate(activeTopView());
    }
    return true;
}

void Zoom_Window(OdGePoint3d & pt1, OdGePoint3d & pt2, OdGsView * view) {
    const OdGeMatrix3d WorldToEye = OdAbstractViewPEPtr(view)->worldToEye(view);
    pt1.transformBy(WorldToEye);
    pt2.transformBy(WorldToEye);
    OdGeVector3d eyeVec = pt2 - pt1;
    if (OdNonZero(eyeVec.x) && OdNonZero(eyeVec.y)) {
        OdGePoint3d newPos = pt1 + eyeVec / 2.;

        eyeVec.x = fabs(eyeVec.x);
        eyeVec.y = fabs(eyeVec.y);

        view->dolly(newPos.asVector());

        const double FieldWidth = view->fieldWidth() / eyeVec.x;
        const double FieldHeight = view->fieldHeight() / eyeVec.y;

        view->zoom(odmin(FieldWidth, FieldHeight));
    }
}
void zoom_scale(double factor) noexcept {}
static bool getLayoutExtents(const OdDbObjectId & spaceId, const OdGsView * view, OdGeBoundBlock3d & boundingBox) {
    OdDbBlockTableRecordPtr BlockTableRecord = spaceId.safeOpenObject();
    OdDbLayoutPtr Layout = BlockTableRecord->getLayoutId().safeOpenObject();
    OdGeExtents3d Extents;
    if (Layout->getGeomExtents(Extents) == eOk) {
        Extents.transformBy(view->viewingMatrix());
        boundingBox.set(Extents.minPoint(), Extents.maxPoint());
        return (Extents.minPoint() != Extents.maxPoint());
    }
    return false;
}
void zoom_extents(OdGsView * view, OdDbObject * activeViewportObject) {
    const OdDbDatabase* Database = activeViewportObject->database();
    OdAbstractViewPEPtr pVpPE(view);
    OdGeBoundBlock3d BoundBlock;
    bool bBboxValid = pVpPE->viewExtents(view, BoundBlock);

    // paper space overall view
    OdDbViewportPtr pVp = OdDbViewport::cast(activeViewportObject);
    if (pVp.get() && pVp->number() == 1) {
        if (!bBboxValid || !(BoundBlock.minPoint().x < BoundBlock.maxPoint().x && BoundBlock.minPoint().y < BoundBlock.maxPoint().y)) {
            bBboxValid = ::getLayoutExtents(Database->getPaperSpaceId(), view, BoundBlock);
        }
    } else if (!bBboxValid) { // model space viewport
        bBboxValid = ::getLayoutExtents(Database->getPaperSpaceId(), view, BoundBlock);
    }
    if (!bBboxValid) { // set to somewhat reasonable (e.g. paper size)
        if (Database->getMEASUREMENT() == OdDb::kMetric) {
            BoundBlock.set(OdGePoint3d::kOrigin, OdGePoint3d(297., 210., 0.)); // set to papersize ISO A4 (portrait)
        } else {
            BoundBlock.set(OdGePoint3d::kOrigin, OdGePoint3d(11., 8.5, 0.)); // ANSI A (8.50 x 11.00) (landscape)
        }
        BoundBlock.transformBy(view->viewingMatrix());
    }
    pVpPE->zoomExtents(view, &BoundBlock);
}
void zoom_scaleXP(double factor) noexcept {}

class RTZoomTracker : public OdEdPointTracker {
    OdGsView* m_pView;
    double  m_base;
    double  m_fw;
    double  m_fh;
public:
    void init(OdGsView* view, const OdGePoint3d& base) {
        m_pView = view;
        m_fw = view->fieldWidth();
        m_fh = view->fieldHeight();
        m_base = (m_pView->projectionMatrix() * m_pView->viewingMatrix() * base).y;
    }
    void setValue(const OdGePoint3d& value) override {
        const OdGeMatrix3d xWorldToNDC = m_pView->projectionMatrix() * m_pView->viewingMatrix();
        const OdGePoint3d pt2 = xWorldToNDC * value;
        double fac = 1. + fabs(pt2.y - m_base) * 1.5;
        if (pt2.y > m_base) {
            fac = 1. / fac;
        }
        const OdGsView::Projection ProjectionType(m_pView->isPerspective() ? OdGsView::kPerspective : OdGsView::kParallel);
        m_pView->setView(m_pView->position(), m_pView->target(), m_pView->upVector(), m_fw * fac, m_fh * fac, ProjectionType);
    }
    int addDrawables(OdGsView * view) noexcept override {
        return 1;
    }
    void removeDrawables(OdGsView * view) noexcept override {}
};
const OdString OdExZoomCmd::groupName() const {
    return globalName();
}
const OdString OdExZoomCmd::globalName() const {
    return (L"ZOOM");
}
void OdExZoomCmd::execute(OdEdCommandContext* commandContext) {
    OdDbCommandContextPtr CommandContext(commandContext);
    OdDbDatabasePtr Database = CommandContext->database();
    OdSmartPtr<OdDbUserIO> UserIO = CommandContext->userIO();

    OdDbObjectPtr ActiveViewportObject = Database->activeViewportId().safeOpenObject(OdDb::kForWrite);
    OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewportObject);

    OdGsView* pView = AbstractViewportData->gsView(ActiveViewportObject);

    OdGePoint3d pt1;
    OdGePoint3d pt2;

    try {
        const OdChar* szKeywords = OD_T("All Center Dynamic Extents Previous Scale Window Object");
        const OdString FirstCornerPrompt(L"Specify corner of window, enter a scale factor (nX or nXP), or\n[All/Center/Dynamic/Extents/Previous/Scale/Window/Object] <real time>:");
        const OdString OppositeCornerPrompt(L"Specify opposite corner:");
        pt1 = UserIO->getPoint(FirstCornerPrompt, OdEd::kInpThrowEmpty | OdEd::kInpThrowOther | OdEd::kGptNoOSnap, 0, szKeywords);
        pt2 = UserIO->getPoint(OppositeCornerPrompt, OdEd::kGptNoUCS | OdEd::kGptRectFrame | OdEd::kGptNoOSnap);
        Zoom_Window(pt1, pt2, pView);
    } catch (const OdEdEmptyInput) { // real time
        OdStaticRxObject<RTZoomTracker> tracker;
        for (;;) {
            try {
                tracker.init(pView, UserIO->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptBeginDrag | OdEd::kGptNoOSnap));
                UserIO->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptEndDrag | OdEd::kGptNoOSnap, 0, OdString::kEmpty, &tracker);
            } catch (const OdEdCancel) {
                break;
            }
        }
    } catch (const OdEdOtherInput & otherInput) { // nX or nXP
        OdChar* pEnd;
        const double scale = odStrToD(otherInput.string(), &pEnd);
        if (pEnd > otherInput.string().c_str()) {
            OdString sEnd(pEnd);
            if (sEnd.iCompare(L"X") == 0) {
                pView->zoom(scale);
            } else if (sEnd.iCompare(L"XP") == 0) {
                zoom_scaleXP(scale);
            } else if (!*pEnd) {
                pView->zoom(scale);
            }
        }
        UserIO->putString(L"Requires a distance, numberX, or option keyword.");
    } catch (const OdEdKeyword & kw) {
        switch (kw.keywordIndex()) {
        case 0: // All
            break;
        case 1: // Center
            break;
        case 2: // Dynamic
            break;
        case 3: // Extents
            ::zoom_extents(pView, ActiveViewportObject);
            break;
        case 4: // Previous
            break;
        case 5: // Scale
            break;
        case 6: // Window
            pt1 = UserIO->getPoint(L"Specify first corner:", OdEd::kGptNoUCS | OdEd::kGptNoOSnap);
            pt2 = UserIO->getPoint(L"Specify opposite corner:", OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptRectFrame);
            ::Zoom_Window(pt1, pt2, pView);
            break;
        case 7: // Object
            break;
        }
    }
    AbstractViewportData->setView(ActiveViewportObject, pView);
}

class OrbitCtrl : public OdGiDrawableImpl<> {
public:
    OdUInt32 subSetAttributes(OdGiDrawableTraits* traits) const noexcept override {
        return kDrawableIsAnEntity | kDrawableRegenDraw;
    }
    bool subWorldDraw(OdGiWorldDraw* worldDraw) const noexcept override {
        return false;
    }
    void subViewportDraw(OdGiViewportDraw* viewportDraw) const override {
        const OdGiViewport& ViewPort = viewportDraw->viewport();
        OdGiGeometry& Geometry = viewportDraw->geometry();
        viewportDraw->subEntityTraits().setColor(OdCmEntityColor::kACIGreen);
        viewportDraw->subEntityTraits().setFillType(kOdGiFillNever);

        OdGiModelTransformSaver mt(Geometry, ViewPort.getEyeToModelTransform());
        OdGiDrawFlagsHelper _dfh(viewportDraw->subEntityTraits(), OdGiSubEntityTraits::kDrawNoPlotstyle);

        OdGePoint3d pt1;
        OdGePoint2d pt2;
        ViewPort.getViewportDcCorners((OdGePoint2d&)pt1, pt2);
        pt2.x -= pt1.x;
        pt2.y -= pt1.y;
        const double Radius = odmin(pt2.x, pt2.y) / 9. * 7. / 2.;
        ((OdGePoint2d&)pt1) += (pt2.asVector() / 2.);
        Geometry.circle(pt1, Radius, OdGeVector3d::kZAxis);

        Geometry.circle(pt1 + OdGeVector3d(0., Radius, 0.), Radius / 20., OdGeVector3d::kZAxis);
        Geometry.circle(pt1 + OdGeVector3d(0., -Radius, 0.), Radius / 20., OdGeVector3d::kZAxis);
        Geometry.circle(pt1 + OdGeVector3d(Radius, 0., 0.), Radius / 20., OdGeVector3d::kZAxis);
        Geometry.circle(pt1 + OdGeVector3d(-Radius, 0., 0.), Radius / 20., OdGeVector3d::kZAxis);
    }
};
class RTOrbitTracker : public OdEdPointTracker {
    OdGsView* m_pView;
    OdGePoint3d m_pt;
    OdGiDrawablePtr m_pDrw;
    OdGePoint3d m_pos;
    OdGePoint3d m_trg;
    OdGeVector3d    m_up;
    OdGeVector3d    m_x;
    OdGePoint3d m_viewCenter;
    OdGeMatrix3d    m_initViewingMatrixInv;
    double  m_D; // diameter of orbit control in projected coordinates
    OdGsModelPtr    m_pModel;

    enum Axis {
        kHorizontal,
        kVertical,
        kPerpDir, // orbit around perpendicular to mouse direction
        kEye,
    }
    m_axis;

    void viewportDcCorners(OdGePoint2d& lowerLeft, OdGePoint2d& upperRight) const {
        const OdGePoint3d Target = m_pView->viewingMatrix() * m_pView->target();
        const double HalfFieldWidth = m_pView->fieldWidth() / 2.0;
        const double HalfFieldHeight = m_pView->fieldHeight() / 2.0;
        lowerLeft.x = Target.x - HalfFieldWidth;
        lowerLeft.y = Target.y - HalfFieldHeight;
        upperRight.x = Target.x + HalfFieldWidth;
        upperRight.y = Target.y + HalfFieldHeight;
    }
public:
    RTOrbitTracker() :
        m_pView(0), m_D(0) {}
    void reset() noexcept {
        m_pView = 0;
    }
    void init(OdGsView* view, const OdGePoint3d& point) {
        m_pView = view;
        m_pos = view->position();
        m_trg = view->target();
        m_up = view->upVector();
        m_x = m_up.crossProduct(view->target() - m_pos).normal();

        m_initViewingMatrixInv = m_pView->viewingMatrix();
        m_pt = m_initViewingMatrixInv * point;
        m_pt.z = 0.;
        m_initViewingMatrixInv.invert();

        OdGePoint3d pt1;
        OdGePoint2d pt2;
        viewportDcCorners((OdGePoint2d&)pt1, pt2);
        pt2.x -= pt1.x;
        pt2.y -= pt1.y;
        const double Radius = odmin(pt2.x, pt2.y) / 9. * 7. / 2.;
        m_D = 2.0 * Radius;
        ((OdGePoint2d&)pt1) += (pt2.asVector() / 2.);
        const double r2sqrd = Radius * Radius / 400.;

        pt1.y += Radius;
        if ((pt1 - m_pt).lengthSqrd() <= r2sqrd) {
            m_axis = kHorizontal;
        } else {
            pt1.y -= Radius;
            pt1.y -= Radius;
            if ((pt1 - m_pt).lengthSqrd() <= r2sqrd) {
                m_axis = kHorizontal;
            } else {
                pt1.y += Radius;
                pt1.x += Radius;
                if ((pt1 - m_pt).lengthSqrd() <= r2sqrd) {
                    m_axis = kVertical;
                } else {
                    pt1.x -= Radius;
                    pt1.x -= Radius;
                    if ((pt1 - m_pt).lengthSqrd() <= r2sqrd) {
                        m_axis = kVertical;
                    } else {
                        pt1.x += Radius;
                        if ((pt1 - m_pt).lengthSqrd() <= Radius * Radius) {
                            m_axis = kPerpDir;
                        } else {
                            m_axis = kEye;
                        }
                    }
                }
            }
        }
        OdAbstractViewPEPtr pAView = view;
        OdGeBoundBlock3d BoundBlock;
        pAView->viewExtents(view, BoundBlock);
        m_viewCenter = BoundBlock.center();
        m_viewCenter.transformBy(m_initViewingMatrixInv);
    }
    double angle(const OdGePoint3d& value) const {
        const OdGePoint3d pt2 = m_pView->viewingMatrix() * value;
        double Distance(0.);
        if (m_axis == kHorizontal) {
            Distance = pt2.y - m_pt.y;
        } else if (m_axis == kVertical) {
            Distance = pt2.x - m_pt.x;
        }
        return Distance * OdaPI / m_D;
    }
    double angleZ(const OdGePoint3d& value) const {
        OdGePoint3d pt2 = m_pView->viewingMatrix() * value;
        OdGePoint3d Target = m_pView->viewingMatrix() * m_viewCenter;
        pt2.z = Target.z = m_pt.z;
        return (pt2 - Target).angleTo((m_pt - Target), OdGeVector3d::kZAxis);
    }
    double anglePerp(const OdGePoint3d & value) const {
        OdGePoint3d pt2 = m_pView->viewingMatrix() * value;
        pt2.z = 0.0;
        return pt2.distanceTo(m_pt)* OdaPI / m_D;
    }
    void setValue(const OdGePoint3d & value) override {
        if (m_pView) {
            OdGeMatrix3d x;
            switch (m_axis) {
            case kHorizontal:
                x.setToRotation(-angle(value), m_x, m_viewCenter);
                break;
            case kVertical:
                x.setToRotation(-angle(value), m_up, m_viewCenter);
                break;
            case kEye:
                x.setToRotation(-angleZ(value), m_trg - m_pos, m_viewCenter);
                break;
            case kPerpDir:
            {
                OdGePoint3d value1(value);
                value1.transformBy(m_pView->viewingMatrix());
                value1.z = 0.0;
                const OdGeVector2d dir = (value1 - m_pt).convert2d();
                const OdGeVector2d perp = dir.perpVector();
                OdGeVector3d perp3d(perp.x, perp.y, 0.0);
                perp3d.normalizeGetLength();
                perp3d.transformBy(m_initViewingMatrixInv);
                x.setToRotation(-anglePerp(value), perp3d, m_viewCenter);
                break;
            }
            }
            OdGePoint3d newPos = x * m_pos;
            const OdGePoint3d newTarget = x * m_trg;
            OdGeVector3d newPosDir = newPos - newTarget;
            newPosDir.normalizeGetLength();
            newPosDir *= m_pos.distanceTo(m_trg);
            newPos = newTarget + newPosDir;
            const OdGsView::Projection ProjectionType(m_pView->isPerspective() ? OdGsView::kPerspective : OdGsView::kParallel);
            m_pView->setView(newPos, newTarget, x * m_up, m_pView->fieldWidth(), m_pView->fieldHeight(), ProjectionType);
        }
    }
    int addDrawables(OdGsView * view) override {
        m_pDrw = OdRxObjectImpl<OrbitCtrl>::createObject();
        if (m_pModel.isNull()) {
            m_pModel = view->device()->createModel();
            if (!m_pModel.isNull()) {
                m_pModel->setRenderType(OdGsModel::kDirect);
            }
        }
        view->add(m_pDrw, m_pModel.get());
        return 1;
    }
    void removeDrawables(OdGsView * view) override {
        view->erase(m_pDrw);
    }
};
const OdString OdEx3dOrbitCmd::groupName() const {
    return globalName();
}
const OdString OdEx3dOrbitCmd::globalName() const {
    return (L"3DORBIT");
}
void OdEx3dOrbitCmd::execute(OdEdCommandContext * commandContext) {
    OdDbCommandContextPtr CommandContext(commandContext);
    OdDbDatabasePtr Database = CommandContext->database();
    OdSmartPtr<OdDbUserIO> UserIO = CommandContext->userIO();

    OdDbObjectPtr ActiveViewportObject = Database->activeViewportId().safeOpenObject(OdDb::kForWrite);
    OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewportObject);

    OdGsView* pView = AbstractViewportData->gsView(ActiveViewportObject);

    // There is one special case: layout with enabled 'draw viewports first' mode
    {
        if (!Database->getTILEMODE()) {
            OdDbLayoutPtr Layout = Database->currentLayoutId().openObject();
            if (Layout->drawViewportsFirst()) {
                if (pView->device()->viewAt(pView->device()->numViews() - 1) == pView) {
                    pView = pView->device()->viewAt(0);
                }
            }
        }
    }
    //
    OdStaticRxObject<RTOrbitTracker> tracker;
    for (;;) {
        try {
            const int BeginDragOptions(OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptBeginDrag);
            tracker.init(pView, UserIO->getPoint(L"Press ESC or ENTER to exit.", BeginDragOptions, 0, OdString::kEmpty, &tracker));
            const int EndDragOptions(OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptEndDrag);
            UserIO->getPoint(L"Press ESC or ENTER to exit.", EndDragOptions, 0, OdString::kEmpty, &tracker);
            tracker.reset();
        } catch (const OdEdCancel) {
            break;
        }
    }
}

class RTDollyTracker : public OdEdPointTracker {
    OdGsView* m_pView;
    OdGePoint3d m_pt;
    OdGePoint3d m_pos;
public:
    RTDollyTracker() :
        m_pView(0) {}
    void reset() noexcept {
        m_pView = 0;
    }
    void init(OdGsView* view, const OdGePoint3d& point) {
        m_pView = view;
        m_pos = view->position();
        m_pt = point - m_pos.asVector();
    }
    void setValue(const OdGePoint3d& value) override {
        if (m_pView) {
            OdGeVector3d delta = (m_pt - (value - m_pos)).asVector();
            m_pt = value - m_pos.asVector();
            delta.transformBy(m_pView->viewingMatrix());
            m_pView->dolly(delta.x, delta.y, delta.z);
            m_pos = m_pView->position();
        }
    }
    int addDrawables(OdGsView * view) noexcept override {
        return 0;
    }
    void removeDrawables(OdGsView * view) noexcept override {}
};
const OdString OdExDollyCmd::groupName() const {
    return globalName();
}
const OdString OdExDollyCmd::globalName() const {
    return (L"DOLLY");
}
void OdExDollyCmd::execute(OdEdCommandContext * commandContext) {
    OdDbCommandContextPtr CommandContext(commandContext);
    OdDbDatabasePtr Database = CommandContext->database();
    OdSmartPtr<OdDbUserIO> UserIO = CommandContext->userIO();

    OdDbObjectPtr ActiveViewportObject = Database->activeViewportId().safeOpenObject(OdDb::kForWrite);
    OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewportObject);

    OdGsView* pView = AbstractViewportData->gsView(ActiveViewportObject);

    // @@@ There is one special case: layout with enabled 'draw viewports first' mode
    {
        if (!Database->getTILEMODE()) {
            OdDbLayoutPtr Layout = Database->currentLayoutId().openObject();
            if (Layout->drawViewportsFirst()) {
                if (pView->device()->viewAt(pView->device()->numViews() - 1) == pView)
                    pView = pView->device()->viewAt(0);
            }
        }
    }
    //
    OdStaticRxObject<RTDollyTracker> tracker;
    for (;;) {
        try {
            const int BeginDragOptions(OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptBeginDrag);
            tracker.init(pView, UserIO->getPoint(L"Press ESC or ENTER to exit.", BeginDragOptions, 0, OdString::kEmpty, &tracker));
            const int EndDragOptions(OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptEndDrag);
            UserIO->getPoint(L"Press ESC or ENTER to exit.", EndDragOptions, 0, OdString::kEmpty, &tracker);
            tracker.reset();
        } catch (const OdEdCancel) {
            break;
        }
    }
}
