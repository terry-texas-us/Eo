#pragma once

#include "StaticRxObject.h"

#include "DbGsManager.h"
#include "GiContextForDbDatabase.h"
#include "ExEdBaseIO.h"
#include "Gs/Gs.h"
#include "atltypes.h"
#include "EditorObject.h"
#include "ExEdInputParser.h"

#include "EoGsViewport.h"
#include "EoGsModelTransform.h"
#include "EoGsViewTransform.h"

#include "EoDbEllipse.h"
#include "EoDbLine.h"
#include "EoDbPoint.h"
#include "EoDbGroup.h"
#include "EoDbGroupList.h"
#include "PrimState.h"
#include "Section.h"

class AeSysDoc;
class EoDbText;

class AeSysView
    : public CView
    , public OdGiContextForDbDatabase
    // <command_console>
    , OdEdBaseIO
    // </command_console>
    , OdExEditorObject::OleDragCallback {
    // <command_view>
    friend class SaveViewParams;
    // </command_view>

    void destroyDevice();
    COleDropTarget m_dropTarget;

    OdString m_sPrompt;
    ExEdInputParser m_inpars;

    static UINT g_nRedrawMSG;
    // <command_view>
    OdExEditorObject m_editor;
    // </command_view>
    mutable bool m_bRegenAbort;
    mutable bool m_bInRegen; // flag to avoid reentrancy in regen, if new redraw message is received while regen is incomplete (e.g. when assert pops up)

    enum PaintMode { PaintMode_Redraw, PaintMode_Regen };

    PaintMode m_paintMode;

    CPoint m_oldPoint;
    HCURSOR m_hCursor;

    enum Mode { kQuiescent, kGetPoint, kGetString, kDragDrop };

    Mode m_mode;

    struct Response {
        enum Type { kNone, kPoint, kString, kCancel };
        Type m_type;
        OdGePoint3d m_point;
        OdString m_string;
    };
    Response m_response;
    // <command_view>
    int m_inpOptions;
    // </command_view>

    OdEdInputTracker* m_pTracker;
    bool m_bTrackerHasDrawables;
    OdGePoint3d m_basePt;
    const OdGePoint3d* m_pBasePt;

    void exeCmd(const OdString& szCmdStr);
    // <OleDragCallback virtual>
    bool beginDragCallback(const OdGePoint3d& point) override;
    // </OleDragCallback virtual>
protected:
    using CView::operator new;
    using CView::operator delete;
private:
    BOOL m_bPsOverall;
    OdDbObjectId m_layoutId;

    bool m_bPlotPlotstyle;
    bool m_bShowPlotstyle;
    bool m_bPlotGrayscale;

    OdGiContext::PStyleType plotStyleType() const override;
    void plotStyle(OdDbStub* psNameId, OdPsPlotStyleData& plotStyleData) const override;
protected:
#ifdef ODAMFC_EXPORT_SYMBOL
    friend OdGsLayoutHelperPtr odGetDocDevice(CDocument* document);
#endif // ODAMFC_EXPORT_SYMBOL

    OdGsLayoutHelperPtr m_pDevice;
    OdGsLayoutHelperPtr m_pPrinterDevice;
    HDC m_hWindowDC;
    int m_pagingCounter;

    CRect viewportRect() const;
    static CRect viewRect(OdGsView*);

    AeSysView() noexcept; // protected constructor used by dynamic creation

    void preparePlotstyles(const OdDbLayout* pLayout = NULL, bool bForceReload = false);

    OdUInt32 glyphSize(GlyphType glyphType) const override;
    void fillContextualColors(OdGiContextualColorsImpl* pCtxColors) override;

    DECLARE_DYNCREATE(AeSysView)

public:
    OdGsView* getActiveView();
    const OdGsView* getActiveView() const;
    OdGsView* getActiveTopView();
    const OdGsView* getActiveTopView() const;
    void propagateActiveViewChanges() const;

    void track(OdEdInputTracker* tracker);
    void setCursor(HCURSOR cursor) noexcept;
    HCURSOR cursor() const noexcept;

    void setViewportBorderProperties();
    // <command_view>
    bool canClose() const;
    bool isGettingString() const noexcept;
    OdString prompt() const;
    int inpOptions() const noexcept;
    void respond(const OdString& s);
    OdEdCommandPtr command(const OdString& commandName);
    OdExEditorObject& editorObject() noexcept;
    const OdExEditorObject& editorObject() const noexcept;
    bool isModelSpaceView() const;
    bool drawableVectorizationCallback(const OdGiDrawable* drawable);
    // </command_view>

public:
    void OnInitialUpdate() override;

protected:
    void OnDraw(CDC* deviceContext) override;
    BOOL OnPreparePrinting(CPrintInfo* printInformation) override;
    void OnBeginPrinting(CDC* deviceContext, CPrintInfo* printInformation) override;
    void OnPrepareDC(CDC* deviceContext, CPrintInfo* printInformation) override;
    void OnPrint(CDC* deviceContext, CPrintInfo* printInformation) override;
    void OnEndPrinting(CDC* deviceContext, CPrintInfo* printInformation) override;

    void OnActivateFrame(UINT nState, CFrameWnd* pDeactivateFrame) override;
    void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;

    BOOL PreCreateWindow(CREATESTRUCT& createStructure) override;
    void OnUpdate(CView* sender, LPARAM hint, CObject* hintObject) override;

protected:
    void addRef() noexcept override {}
    void release() noexcept override {}

    AeSysDoc* GetDocument() const;

protected:
    ~AeSysView();
#ifdef _DEBUG
    void AssertValid() const override;
    void Dump(CDumpContext& dc) const override;
#endif

    // <tas="Not implemented in example"</tas> void adjustDevice(OdGsDevice* pDevice);
    void createDevice();
    bool regenAbort() const noexcept override;

public: // Methods - virtuals 
    // <command_console>
    OdUInt32 getKeyState() noexcept override;
    OdGePoint3d getPoint(const OdString& prompt, int options, OdEdPointTracker* tracker) override;
    OdString getString(const OdString& prompt, int options, OdEdStringTracker* tracker) override;
    void putString(const OdString& string) override;
    // </command_console>

    bool UpdateStringTrackerCursor(void);

public:
    enum EStateInformationItem {
        WorkCount = 0x0001,
        TrapCount = 0x0002,
        BothCounts = WorkCount | TrapCount,
        Pen = 0x0004,
        Line = 0x0008,
        TextHeight = 0x0010,
        WndRatio = 0x0020,
        Scale = 0x0040,
        DimLen = 0x0080,
        DimAng = 0x0100,
        All = BothCounts | Pen | Line | TextHeight | WndRatio | Scale | DimLen | DimAng
    };
    enum ERubs { None, Lines, Rectangles };

private:
    static const double sm_MaximumWindowRatio;
    static const double sm_MinimumWindowRatio;

    EoGsModelTransform m_ModelTransform;
    EoGsViewport m_Viewport;
    EoGsViewTransform m_ViewTransform;

    CBitmap m_BackgroundImageBitmap;
    CPalette m_BackgroundImagePalette;
    EoDbPrimitive* m_EngagedPrimitive;
    EoDbGroup* m_EngagedGroup;
    OdUInt16 m_OpHighlighted;
    EoGsViewTransform m_OverviewViewTransform;
    bool m_Plot;
    float m_PlotScaleFactor;
    EoDbGroup m_PreviewGroup;
    EoGsViewTransform m_PreviousViewTransform;
    OdUInt16 m_PreviousOp;
    OdGePoint3d m_PreviousPnt;
    double m_SelectApertureSize;
    bool m_ViewBackgroundImage;
    bool m_ViewOdometer;
    bool m_ViewPenWidths;
    CViewports m_Viewports;
    EoGsViewTransforms m_ViewTransforms;
    bool m_ViewTrueTypeFonts;
    EoDbGroupList m_VisibleGroupList;
    double m_WorldScale;
    bool m_LeftButton;
    bool m_MiddleButton;
    bool m_RightButton;

    ERubs m_RubberbandType;
    OdGePoint3d m_RubberbandBeginPoint;
    CPoint m_RubberbandLogicalBeginPoint;
    CPoint m_RubberbandLogicalEndPoint;

    OdGePoint3d m_ptCursorPosDev;
    OdGePoint3d	m_ptCursorPosWorld;

    CPoint m_MouseClick;
    CPoint m_MousePosition;
    bool m_ZoomWindow;
    OdGePoint3dArray m_Points;

    bool m_ModelTabIsActive;
    ODCOLORREF m_Background;

    OdGePoint3d m_ptDet;
    OdGeVector3d m_vRelPos;
    OdGePoint3dArray m_DrawModePoints;
    OdGePoint3dArray m_NodalModePoints;
    OdGePoint3dArray m_PipeModePoints;
    OdGePoint3dArray m_PowerModePoints;

private: // grid and axis constraints
    OdGePoint3d m_GridOrigin;
    int m_MaximumDotsPerLine;

    double m_XGridLineSpacing;
    double m_YGridLineSpacing;
    double m_ZGridLineSpacing;

    double m_XGridSnapSpacing;
    double m_YGridSnapSpacing;
    double m_ZGridSnapSpacing;

    double m_XGridPointSpacing;
    double m_YGridPointSpacing;
    double m_ZGridPointSpacing;

    double m_AxisConstraintInfluenceAngle;
    double m_AxisConstraintOffsetAngle;
    bool m_DisplayGridWithLines;
    bool m_DisplayGridWithPoints;
    bool m_GridSnap;

public:
    double AxisConstraintInfluenceAngle() const noexcept;
    void SetAxisConstraintInfluenceAngle(const double angle) noexcept;
    double AxisConstraintOffsetAngle() const noexcept;
    void SetAxisConstraintOffsetAngle(const double angle) noexcept;
    void InitializeConstraints() noexcept;
    /// <summary>Generates a point display centered about the user origin in one or more of the three orthogonal planes for the current user grid.</summary>
    void DisplayGrid(CDC* deviceContext);
    OdGePoint3d GridOrigin() const noexcept;
    void SetGridOrigin(const OdGePoint3d& origin) noexcept;
    void GetGridLineSpacing(double& x, double& y, double& z) noexcept;
    void SetGridLineSpacing(double x, double y, double z) noexcept;
    void GetGridPointSpacing(double& x, double& y, double& z) noexcept;
    void SetGridPointSpacing(double x, double y, double z) noexcept;
    void GetGridSnapSpacing(double& x, double& y, double& z) noexcept;
    void SetGridSnapSpacing(double x, double y, double z) noexcept;
    /// <summary>Determines the nearest point on system constraining grid.</summary>
    OdGePoint3d	SnapPointToGrid(const OdGePoint3d& point) noexcept;
    /// <summary>Set Axis constraint tolerance angle and offset axis constraint offset angle. Constrains a line to nearest axis pivoting on first endpoint.</summary>
    /// <remarks>Offset angle only support about z-axis</remarks>
    /// <returns>Point after snap</returns>
    OdGePoint3d SnapPointToAxis(const OdGePoint3d& startPoint, const OdGePoint3d& endPoint);

    bool DisplayGridWithLines() const noexcept;
    void EnableDisplayGridWithLines(bool display) noexcept;
    void EnableDisplayGridWithPoints(bool display) noexcept;
    bool DisplayGridWithPoints() const noexcept;
    bool GridSnap() const noexcept;
    void EnableGridSnap(bool snap) noexcept;
    void ZoomWindow(OdGePoint3d point1, OdGePoint3d point2);

public:
    void ResetDevice(bool zoomExtents = FALSE);
    void SetRenderMode(OdGsView::RenderMode renderMode);
    OdGsView::RenderMode RenderMode() const noexcept {
        return m_ViewTransform.RenderMode();
    }
    void SetViewportBorderProperties(OdGsDevice* device, bool model);

    const ODCOLORREF* CurrentPalette() const;
public:
    OdDbDatabasePtr Database() const;

protected: // Windows messages
    void OnContextMenu(CWnd*, CPoint point);

public: // Input message handler member functions

    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnLButtonUp(UINT nFlags, CPoint point);
    void OnMButtonDown(UINT nFlags, CPoint point);
    void OnMButtonUp(UINT nFlags, CPoint point);
    void OnMouseMove(UINT nFlags, CPoint point);
    BOOL OnMouseWheel(UINT nFlags, OdInt16 zDelta, CPoint point);
    void OnRButtonDown(UINT nFlags, CPoint point);
    void OnRButtonUp(UINT nFlags, CPoint point);

    int OnCreate(LPCREATESTRUCT createStructure);
    void OnDestroy();
    void OnDrag();
    void OnUpdateDrag(CCmdUI* pCmdUI);
    BOOL OnEraseBkgnd(CDC* deviceContext);
    void OnKillFocus(CWnd* newWindow);
    void OnPaint();
    void OnSetFocus(CWnd* oldWindow);
    void OnSize(UINT type, int cx, int cy);

    void OnViewStateInformation();
    void OnUpdateViewStateinformation(CCmdUI* pCmdUI);

public:
    static AeSysView* GetActiveView(void);

    void VerifyFindString(CMFCToolBarComboBoxButton* findCombo, CString& findText);

    bool m_ViewStateInformation;
    void UpdateStateInformation(EStateInformationItem item);

    void RubberBandingDisable();
    void RubberBandingStartAtEnable(const OdGePoint3d& point, ERubs type);

    OdGePoint3d GetCursorPosition();
    OdGePoint3d GetWorldCoordinates(CPoint point);
    /// <summary> Positions cursor at targeted position.</summary>
    void SetCursorPosition(const OdGePoint3d& point);
    void SetModeCursor(int mode);

    std::pair<EoDbGroup*, EoDbEllipse*> SelectCircleUsingPoint(const OdGePoint3d& point, double tolerance);
    std::pair<EoDbGroup*, EoDbLine*> SelectLineUsingPoint(const OdGePoint3d& point);
    std::pair<EoDbGroup*, EoDbPoint*> AeSysView::SelectPointUsingPoint(const OdGePoint3d& point, double tolerance, OdInt16 pointColor);
    EoDbGroup* SelSegAndPrimAtCtrlPt(const EoGePoint4d& pt);
    EoDbText* SelectTextUsingPoint(const OdGePoint3d& point);
    EoDbGroup* SelectGroupAndPrimitive(const OdGePoint3d& point);
    OdGePoint3d& DetPt() noexcept;
    EoDbPrimitive*& EngagedPrimitive() noexcept;
    EoDbGroup*& EngagedGroup() noexcept;
    /// <summary>Set a pixel.</summary>
    void DisplayPixel(CDC* deviceContext, COLORREF colorReference, const OdGePoint3d& point);

    bool GroupIsEngaged() noexcept;
    double SelectApertureSize() const noexcept;
    void BreakAllPolylines();
    void BreakAllSegRefs();

    bool PenWidthsOn() noexcept;
    double WorldScale() const noexcept;
    void SetWorldScale(const double scale);
    POSITION AddGroup(EoDbGroup* group);
    void AddGroups(EoDbGroupList* groups);
    POSITION RemoveGroup(EoDbGroup* group);
    void RemoveAllGroups();
    void ResetView() noexcept;
    /// <summary> Deletes last group detectable in the this view.</summary>
    void DeleteLastGroup();
    POSITION GetFirstVisibleGroupPosition() const;
    POSITION GetLastGroupPosition() const;
    EoDbGroup* GetNextVisibleGroup(POSITION& position);
    EoDbGroup* GetPreviousGroup(POSITION& position);
    void BackgroundImageDisplay(CDC* deviceContext);
    bool ViewTrueTypeFonts() noexcept;
    void DisplayOdometer();
    /// <summary> Streams a sequence of characters as WM_KEYDOWN or WM_CHAR window messages.</summary>
    /// <remarks> This is a legacy feature.</remarks>
    void DoCustomMouseClick(const CString& characters);
    void Orbit(double x, double y);
    void Dolly();
    void DollyAndZoom(double zoomFactor);

    void CopyActiveModelViewToPreviousModelView() noexcept;
    EoGsViewTransform PreviousModelView();
    void ExchangeActiveAndPreviousModelViews();

    EoGeMatrix3d ModelToWorldTransform() const noexcept;
    void PushModelTransform(const EoGeMatrix3d& transformation);
    void PopModelTransform();
    void ModelTransformPoint(OdGePoint3d& point);

    void ModelViewGetViewport(EoGsViewport& viewport) noexcept;
    OdGeVector3d CameraDirection() const;
    EoGeMatrix3d ModelViewMatrix() const noexcept;
    OdGePoint3d	CameraTarget() const noexcept;
    double ZoomFactor() const noexcept;
    OdGeVector3d ViewUp() const noexcept;
    void ModelViewInitialize();

    void PopViewTransform();
    void PushViewTransform();
    void ModelViewTransformPoint(EoGePoint4d& point);
    void ModelViewTransformPoints(EoGePoint4dArray& pointsArray);
    void ModelViewTransformPoints(int numberOfPoints, EoGePoint4d* points);
    void ModelViewTransformVector(OdGeVector3d& vector);
    void SetProjectionPlaneField(double fieldWidth, double fieldHeight);
    void SetViewTransform(EoGsViewTransform& viewTransform);
    void SetCameraPosition(const OdGeVector3d& direction);
    void SetCameraTarget(const OdGePoint3d& target);
    void SetView(const OdGePoint3d& position, const OdGePoint3d& target, const OdGeVector3d& upVector, double fieldWidth, double fieldHeight);
    void SetViewWindow(const double uMin, const double vMin, const double uMax, const double vMax);

    /// <summary>Determines the number of pages for 1 to 1 print</summary>
    UINT NumPages(CDC* deviceContext, double dScaleFactor, UINT& nHorzPages, UINT& nVertPages);

    double OverviewUExt() noexcept;
    double OverviewUMin() noexcept;
    double OverviewVExt() noexcept;
    double OverviewVMin() noexcept;
    CPoint DoViewportProjection(const EoGePoint4d& point) const noexcept;
    void DoViewportProjection(CPoint* pnt, int iPts, EoGePoint4d* pt) const noexcept;
    void DoViewportProjection(CPoint* pnt, EoGePoint4dArray& points) const;
    OdGePoint3d DoViewportProjectionInverse(const OdGePoint3d& point) const noexcept;
    double ViewportHeightInInches() const noexcept;
    double ViewportWidthInInches() const noexcept;
    void ViewportPopActive();
    void ViewportPushActive();
    void SetViewportSize(const int width, const int height) noexcept;
    void SetDeviceHeightInInches(double height) noexcept;
    void SetDeviceWidthInInches(double width) noexcept;
public: // Group and Primitive operations

    EoDbGroup* m_SubModeEditGroup;
    EoDbPrimitive* m_SubModeEditPrimitive;
    OdGePoint3d m_SubModeEditBeginPoint;
    OdGePoint3d m_SubModeEditEndPoint;
    EoGeMatrix3d m_tmEditSeg;

    void InitializeGroupAndPrimitiveEdit();
    void DoEditGroupCopy();
    void DoEditGroupEscape();
    void DoEditGroupTransform(OdUInt16 operation);
    void DoEditPrimitiveTransform(OdUInt16 operation);
    void DoEditPrimitiveCopy();
    void DoEditPrimitiveEscape();
    void PreviewPrimitiveEdit();
    void PreviewGroupEdit();

    OdGePoint3d m_MendPrimitiveBegin;
    DWORD m_MendPrimitiveVertexIndex;
    EoDbPrimitive* m_PrimitiveToMend;
    EoDbPrimitive* m_PrimitiveToMendCopy;

    void PreviewMendPrimitive();
    void MendPrimitiveEscape();
    void MendPrimitiveReturn();

private: // Annotate and Dimension interface
    double m_GapSpaceFactor; // Edge space factor 25 percent of character height
    double m_CircleRadius;
    int m_EndItemType;
    double m_EndItemSize;
    double m_BubbleRadius;
    int m_NumberOfSides; // Number of sides on bubble (0 indicating circle)
    CString m_DefaultText;

public:
    double BubbleRadius() const noexcept;
    void SetBubbleRadius(double radius) noexcept;
    double CircleRadius() const noexcept;
    void SetCircleRadius(double radius) noexcept;
    CString DefaultText() const;
    void SetDefaultText(const CString& text);
    double EndItemSize() const noexcept;
    void SetEndItemSize(double size) noexcept;
    int EndItemType() noexcept;
    void SetEndItemType(int type) noexcept;
    double GapSpaceFactor() const noexcept;
    void SetGapSpaceFactor(double factor) noexcept;
    int NumberOfSides() const noexcept;
    void SetNumberOfSides(int number) noexcept;

public: // Annotate mode interface
    void DoAnnotateModeMouseMove();

    void OnAnnotateModeOptions();
    void OnAnnotateModeLine();
    void OnAnnotateModeArrow();
    void OnAnnotateModeBubble();
    void OnAnnotateModeHook();
    void OnAnnotateModeUnderline();
    void OnAnnotateModeBox();
    void OnAnnotateModeCutIn();
    void OnAnnotateModeConstructionLine();
    void OnAnnotateModeReturn() noexcept;
    void OnAnnotateModeEscape();

    /// <summary>Generates arrow heads for annotation mode.</summary>
    /// <param name="type">type of end item</param>
    /// <param name="size">size of end item</param>
    /// <param name="startPoint">tail of line segment defining arrow head</param>
    /// <param name="endPoint">head of line segment defining arrow head</param>
    /// <param name="group">group where primitives are placed</param>
    void GenerateLineEndItem(int type, double size, const OdGePoint3d& startPoint, const OdGePoint3d& endPoint, EoDbGroup* group);
    bool CorrectLeaderEndpoints(int beginType, int endType, OdGePoint3d& startPoint, OdGePoint3d& endPoint) const;

public: // Draw mode interface
    void DoDrawModeMouseMove();

    void OnDrawModeOptions();
    void OnDrawModePoint();
    void OnDrawModeLine();
    void OnDrawModePolygon();
    void OnDrawModeQuad();
    void OnDrawModeArc();
    void OnDrawModeBspline();
    void OnDrawModeCircle();
    void OnDrawModeEllipse();
    void OnDrawModeInsert();
    void OnDrawModeReturn();
    void OnDrawModeEscape();

private: // Draw2 mode interface
    double m_CenterLineEccentricity;	// Center line eccentricity for parallel lines
    bool m_ContinueCorner;
    EoGeLineSeg3d m_CurrentLeftLine;
    EoGeLineSeg3d m_CurrentRightLine;
    EoGeLineSeg3d m_PreviousReferenceLine;
    EoGeLineSeg3d m_CurrentReferenceLine;
    double m_DistanceBetweenLines;
    EoDbGroup* m_AssemblyGroup;
    EoDbGroup* m_EndSectionGroup;
    EoDbGroup* m_BeginSectionGroup;
    EoDbLine* m_BeginSectionLine;
    EoDbLine* m_EndSectionLine;

public:
    void DoDraw2ModeMouseMove();

    void OnDraw2ModeOptions();
    /// <summary>Searches for an existing wall side or endcap</summary>
    void OnDraw2ModeJoin();
    void OnDraw2ModeWall();
    void OnDraw2ModeReturn();
    void OnDraw2ModeEscape();

    bool CleanPreviousLines();
    bool StartAssemblyFromLine();

    enum EJust { Left = -1, Center, Right };
    
    enum EElbow { Mittered, Radial };

public:
    void OnDimensionModeOptions();
    void OnDimensionModeArrow();
    void OnDimensionModeLine();
    void OnDimensionModeDLine();
    void OnDimensionModeDLine2();
    void OnDimensionModeExten();
    void OnDimensionModeRadius();
    void OnDimensionModeDiameter();
    void OnDimensionModeAngle();
    void OnDimensionModeConvert();
    void OnDimensionModeReturn();
    void OnDimensionModeEscape();

    double m_AxisTolerance;
    double m_CornerSize;
	EoGeLineSeg3d m_CurrentLineSeg;
	EoGeLineSeg3d m_PreviousLineSeg;
	EoGeLineSeg3d m_ReferenceLineSeg;

    void OnFixupModeOptions();
    void OnFixupModeReference();
    void OnFixupModeMend();
    void OnFixupModeChamfer();
    void OnFixupModeFillet();
    void OnFixupModeSquare();
    void OnFixupModeParallel();
    void OnFixupModeReturn();
    void OnFixupModeEscape();

/// <summary>Finds center point of a circle given radius and two tangent vectors.</summary>
/// <Notes>A radius and two lines define four center points. The center point selected is on the concave side of the angle formed by the two vectors defined by the line endpoints. These two vectors are oriented with the tail of the second vector at the head of the first.</notes>
/// <Returns>
/// true    center point determined
/// false   endpoints of first line coincide or endpoints of second line coincide or two lines are parallel or four points are not coplanar
/// </Returns>
	bool FindCenterPointGivenRadiusAndTwoLineSegments(double radius, OdGeLineSeg3d firstLineSeg, OdGeLineSeg3d secondLineSeg, OdGePoint3d& intersection);

public: // Nodal mode interface

    void DoNodalModeMouseMove();

    void OnNodalModeAddRemove();
    void OnNodalModePoint();
    void OnNodalModeLine();
    void OnNodalModeArea();
    /// <summary>Translate all control points identified</summary>
    void OnNodalModeMove();
    void OnNodalModeCopy();
    void OnNodalModeToLine();
    void OnNodalModeToPolygon();
    void OnNodalModeEmpty();
    void OnNodalModeEngage();
    void OnNodalModeReturn();
    void OnNodalModeEscape();

    void ConstructPreviewGroup();
    void ConstructPreviewGroupForNodalGroups();

public: // Cut mode interface
    void OnCutModeOptions() noexcept;
    /// <summary>Cuts a primative at cursor position.</summary>
    void OnCutModeTorch();
    /// <summary>Cuts all primatives which intersect with line defined by two points.</summary>
    // Notes: Colinear fill area edges are not considered to intersect.
    void OnCutModeSlice();
    void OnCutModeField();
    /// <summary>Cuts a primative at two pnts and puts non-null middle piece in trap.</summary>
    // Notes:	Accuracy of arc section cuts diminishes with high
    //			eccentricities. if two cut points are coincident
    //			nothing happens.
    void OnCutModeClip();
    void OnCutModeDivide() noexcept;
    void OnCutModeReturn();
    void OnCutModeEscape();

public: // Edit mode interface
    OdGeScale3d m_MirrorScaleFactors;
    OdGeVector3d m_EditModeRotationAngles;
    OdGeScale3d m_ScaleFactors;

    OdGeVector3d EditModeRotationAngles() const noexcept;
    EoGeMatrix3d EditModeInvertedRotationMatrix() const;
    OdGeScale3d EditModeMirrorScaleFactors() const noexcept;
    EoGeMatrix3d EditModeRotationMatrix() const;
    OdGeScale3d EditModeScaleFactors() const noexcept;
    void SetEditModeScaleFactors(const double sx, const double sy, const double sz) noexcept;
    void SetEditModeRotationAngles(double x, double y, double z) noexcept;
    void SetEditModeMirrorScaleFactors(double sx, double sy, double sz);

    void OnEditModeOptions();
    void OnEditModePivot();
    void OnEditModeRotccw();
    void OnEditModeRotcw();
    void OnEditModeMove();
    void OnEditModeCopy();
    void OnEditModeFlip();
    void OnEditModeReduce();
    void OnEditModeEnlarge();
    void OnEditModeReturn() noexcept;
    void OnEditModeEscape();

    void OnInsertBlockreference();

    void OnTrapModeRemoveAdd();
    void OnTrapModePoint();
    /// <summary>Identifies groups which intersect with a line and adds them to the trap.</summary>
    void OnTrapModeStitch();
    /// <summary>Identifies groups which lie wholly or partially within a rectangular area.</summary>
    /// <remarks>Needs to be generalized to quad.</remarks>
    void OnTrapModeField();
    /// <summary>Adds last detectable group which is not already in trap to trap</summary>
    void OnTrapModeLast();
    void OnTrapModeEngage();
    void OnTrapModeMenu();
    void OnTrapModeModify();
    void OnTrapModeEscape();

    void OnTraprModeRemoveAdd();
    void OnTraprModePoint();
    /// <summary>Identifies groups which intersect with a line and removes them from the trap.</summary>
    void OnTraprModeStitch();
    /// <summary>Identifies groups which lie wholly or partially within a orthoganal rectangular area.</summary>
    // Notes: This routine fails in all but top view. !!
    // Parameters:	pt1 	one corner of the area
    //				pt2 	other corner of the area
    void OnTraprModeField();
    void OnTraprModeLast();
    void OnTraprModeEngage() noexcept;
    void OnTraprModeMenu();
    void OnTraprModeModify();
    void OnTraprModeEscape();

private: // Low Pressure Duct (retangular) interface
    double m_InsideRadiusFactor;
    double m_DuctSeamSize;
    double m_DuctTapSize;
    bool m_GenerateTurningVanes;
    EElbow m_ElbowType;
    EJust m_DuctJustification;
    double m_TransitionSlope;
    bool m_BeginWithTransition;
    bool m_ContinueSection;
    int m_EndCapLocation;
    EoDbPoint* m_EndCapPoint;
    EoDbGroup* m_EndCapGroup;
    bool m_OriginalPreviousGroupDisplayed;
    EoDbGroup* m_OriginalPreviousGroup;

    Section m_PreviousSection;
    Section m_CurrentSection;

public:
    void DoDuctModeMouseMove();

    void OnLpdModeOptions();
    void OnLpdModeJoin();
    void OnLpdModeDuct();
    void OnLpdModeTransition();
    void OnLpdModeTap();
    void OnLpdModeEll();
    void OnLpdModeTee();
    void OnLpdModeUpDown();
    void OnLpdModeSize();
    void OnLpdModeReturn();
    void OnLpdModeEscape();

    /// <summary>
    ///Locates and returns the first two lines that have an endpoint which coincides with
    ///the endpoints of the specified line.
    /// </summary>
    /// <remarks>
    ///	The lines are oriented so direction vectors defined by each point to the specified line.
    ///	No check is made to see if lines are colinear.
    /// Lines are normal to to test line (and therefore parallel to each other) if acceptance angle is 0.
    /// </remarks>
    /// <returns>true if qualifying lines located else false</returns>
    /// <param name="testLinePrimitive">test line</param>
    /// <param name="angularTolerance">angle tolerance for qualifying line (radians)</param>
    /// <param name="leftLine">endpoints of left line</param>
    /// <param name="rightLine">endpoints of right line</param>
    bool Find2LinesUsingLineEndpoints(EoDbLine* testLinePrimitive, double angularTolerance, EoGeLineSeg3d& leftLine, EoGeLineSeg3d& rightLine);
    /// <summary>Generates an end-cap.</summary>
    /// <remarks>
    ///End-caps are groups containing a line and a point.  The line defines the orientation of the end-cap.
    ///The point contains information about the cross-section (width, depth)
    ///and optionally a number which might be used for something like cfm.
    /// </remarks>
    /// <param name="startPoint">start point of the line</param>
    /// <param name="endPoint">end point of the line</param>
    /// <param name="section">width and depth data</param>
    /// <param name="group"></param>
    void GenerateEndCap(const OdGePoint3d& startPoint, const OdGePoint3d& endPoint, Section section, EoDbGroup* group);
    /// <summary>Generates rise or drop fitting.</summary>
    /// <param name="riseDropIndicator">	rise or drop indicator; 1 rise, 2 drop</param>
    /// <param name="section">horizontal section width and depth</param>
    void GenerateRiseDrop(OdUInt16 riseDropIndicator, Section section, EoGeLineSeg3d& referenceLine, EoDbGroup* group);
    /// <summary>Generates rectangular section using a set of parallel lines.</summary>
    /// <param name="section">width and depth of section</param>
    /// <param name="group"></param>
    void GenerateRectangularSection(EoGeLineSeg3d& referenceLine, double eccentricity, Section section, EoDbGroup* group);
    /// <summary> Generates text segment representing width and depth of a piece of duct. </summary>
    void GenSizeNote(const OdGePoint3d& position, double angle, Section section);
    /// <param name="previousReferenceLine"></param>
    /// <param name="previousSection"></param>
    /// <param name="currentReferenceLine">on exit the start point is the same as the point on the endcap</param>
    /// <param name="currentSection"></param>
    /// <param name="group"></param>
    void GenerateRectangularElbow(EoGeLineSeg3d& previousReferenceLine, Section previousSection, EoGeLineSeg3d& currentReferenceLine, Section currentSection, EoDbGroup* group, bool generateEndCaps = true);
    /// <summary>Generates rectangular tap fitting.</summary>
    /// <param name="justification"></param>
    /// <param name="section"></param>
    bool GenerateRectangularTap(EJust justification, Section section);
    /// <summary>Generates a mitered bullhead tee fitting.</summary>
    /// <remarks>
    ///Requires current operation to be a regular rectangular section. The selection based on the current cursor location
    ///identifies the second section, and the direction from the point to the cursor location defines the direction for the two elbow
    /// turns.
    /// </remarks>
    /// <returns>Center point of end cap of exit transition.</returns>
    /// <param name="existingGroup">group containing rectangular section to join</param>
    /// <param name="existingSectionReferenceLine"></param>
    /// <param name="existingSectionWidth"></param>
    /// <param name="existingSectionDepth"></param>
    /// <param name="group"></param>
    OdGePoint3d GenerateBullheadTee(EoDbGroup* existingGroup, EoGeLineSeg3d& existingSectionReferenceLine, double existingSectionWidth, double existingSectionDepth, EoDbGroup* group);
    /// <summary>Generates a full elbow takeoff fitting.</summary>
    void GenerateFullElbowTakeoff(EoDbGroup* existingGroup, EoGeLineSeg3d& existingSectionReferenceLine, Section existingSection, EoDbGroup* group);
    /// <summary>Generates section which transitions from one rectangle to another</summary>
    /// <param name="referenceLine">line defining the start point and direction of the transition</param>
    /// <param name="eccentricity"></param>
    /// <param name="justification"></param>
    /// <param name="slope">slope of the transition</param>
    /// <param name="previousSection">width and depth at start of the transition</param>
    /// <param name="currentSection">width and depth at end of the transition</param>
    /// <param name="group">group receiving the primitives</param>
    void GenerateTransition(EoGeLineSeg3d& referenceLine, double eccentricity, EJust justification, double slope, Section previousSection, Section currentSection, EoDbGroup* group);
    /// <summary>Sets the width and depth of ductwork.</summary>
    void SetDuctOptions(Section& section);
    /// <summary>Determines the total length required to transition duct from one size to another</summary>
    /// <param name="justification">justification: 0 centered, %gt 0 taper to right, %lt 0 taper to left</param>
    /// <param name="slope">slope of the section sides</param>
    /// <param name="previousSection">width and depth of start section</param>
    /// <param name="currentSection">width and depth of end section</param>
    /// <returns>length of the transition</returns>
    double LengthOfTransition(EJust justification, double slope, Section previousSection, Section currentSection) noexcept;

private: // Pipe mode interface
    int m_CurrentPipeSymbolIndex;
    double m_PipeTicSize;
    double m_PipeRiseDropRadius;

    /// <summary>Adds a fitting indication to horizontal pipe section as required by previous fitting type.</summary>
    void GenerateLineWithFittings(int beginType, OdGePoint3d& startPoint, int endType, OdGePoint3d& endPoint, EoDbGroup* group);
    /// <summary>Draws tic mark at a point distance from start point on the line defined by begin and end points.</summary>
    bool GenerateTicMark(const OdGePoint3d& startPoint, const OdGePoint3d& endPoint, double distance, EoDbGroup* group);
    void DropFromOrRiseIntoHorizontalSection(const OdGePoint3d& point, EoDbGroup* group, EoDbLine* section);
    void DropIntoOrRiseFromHorizontalSection(const OdGePoint3d& point, EoDbGroup* group, EoDbLine* section);
public:
    void DoPipeModeMouseMove();

    void OnPipeModeOptions();
    void OnPipeModeLine();
    void OnPipeModeFitting();
    void OnPipeModeRise();
    void OnPipeModeDrop();
    /// <summary>Generates a piping symbol at point specified if pipe section located.</summary>
    void OnPipeModeSymbol();
    void OnPipeModeWye();
    void OnPipeModeReturn();
    void OnPipeModeEscape();

private: // Power mode interface
    bool m_PowerArrow;
    bool m_PowerConductor;
    double m_PowerConductorSpacing;
    OdGePoint3d m_CircuitEndPoint;
    double m_PreviousRadius;

public:
    void DoPowerModeMouseMove();

    void OnPowerModeOptions() noexcept;
    void OnPowerModeCircuit();
    void OnPowerModeGround();
    void OnPowerModeHot();
    void OnPowerModeSwitch();
    void OnPowerModeNeutral();
    void OnPowerModeHome();
    void OnPowerModeReturn();
    void OnPowerModeEscape();

    void GeneratePowerConductorSymbol(OdUInt16 conductorType, const OdGePoint3d& pointOnCircuit, const OdGePoint3d& endPoint);
    void GenerateHomeRunArrow(const OdGePoint3d& pointOnCircuit, const OdGePoint3d& endPoint);
    void DoPowerModeConductor(OdUInt16 conductorType);

public: // Status & Mode Line
    void ModeLineDisplay();
    OdUInt16 ModeLineHighlightOp(OdUInt16 op);
    void ModeLineUnhighlightOp(OdUInt16& op);

    CMFCStatusBar& GetStatusBar(void) const;

public:
    void OnBackgroundImageLoad();
    void OnBackgroundImageRemove();
    void OnFilePlotHalf();
    void OnFilePlotFull();
    void OnFilePlotQuarter();
    void OnFilePrint();
    void OnFind(void);
    void On3dViewsBack();
    void On3dViewsBottom();
    void On3dViewsFront();
    void On3dViewsIsometric();
    void On3dViewsLeft();
    void On3dViewsRight();
    void On3dViewsTop();
    void OnRelativeMovesEngDown();
    void OnRelativeMovesEngDownRotate();
    void OnRelativeMovesEngIn();
    void OnRelativeMovesEngOut();
    void OnRelativeMovesEngLeft();
    void OnRelativeMovesEngLeftRotate();
    void OnRelativeMovesEngRight();
    void OnRelativeMovesEngRightRotate();
    void OnRelativeMovesEngUp();
    void OnRelativeMovesEngUpRotate();
    void OnRelativeMovesRight();
    void OnRelativeMovesUp();
    void OnRelativeMovesLeft();
    void OnRelativeMovesDown();
    void OnRelativeMovesIn();
    void OnRelativeMovesOut();
    void OnRelativeMovesRightRotate();
    void OnRelativeMovesUpRotate();
    void OnRelativeMovesLeftRotate();
    void OnRelativeMovesDownRotate();
    void OnSetupScale();
    void OnToolsPrimitiveSnapto();
    void OnUpdateViewOdometer(CCmdUI* pCmdUI);
    void OnUpdateViewTrueTypeFonts(CCmdUI* pCmdUI);
    void OnUpdateViewBackgroundImage(CCmdUI* pCmdUI);
    void OnUpdateBackgroundimageLoad(CCmdUI* pCmdUI);
    void OnUpdateBackgroundimageRemove(CCmdUI* pCmdUI);
    void OnUpdateViewPenwidths(CCmdUI* pCmdUI);
    void OnUpdateWindowZoomWindow(CCmdUI* pCmdUI);
    void OnViewBackgroundImage();
    void OnViewTrueTypeFonts();
    void OnViewPenWidths();
    void OnViewOdometer();
    void OnViewRefresh();
    void OnViewParameters();
    void OnUpdateViewRendermode2doptimized(CCmdUI* pCmdUI);
    void OnUpdateViewRendermodeWireframe(CCmdUI* pCmdUI);
    void OnUpdateViewRendermodeHiddenline(CCmdUI* pCmdUI);
    void OnUpdateViewRendermodeFlatshaded(CCmdUI* pCmdUI);
    void OnUpdateViewRendermodeSmoothshaded(CCmdUI* pCmdUI);
    void OnViewRendermode(UINT commandId);
    void OnWindowZoomSpecial();
    void OnWindowNormal();
    void OnWindowBest();
    void OnWindowLast();
    void OnWindowSheet();
    void OnWindowZoomIn();
    void OnWindowZoomOut();
    void OnWindowZoomWindow();
    void OnWindowPan();
    void OnWindowPanLeft();
    void OnWindowPanRight();
    void OnWindowPanUp();
    void OnWindowPanDown();
    void OnCameraRotateLeft();
    void OnCameraRotateRight();
    void OnCameraRotateUp();
    void OnCameraRotateDown();
    void OnViewWindow();
    void OnSetupDimLength();
    void OnSetupDimAngle();
    void OnSetupUnits();
    void OnSetupConstraints();
    void OnSetupMouseButtons();
    void OnModePrimitiveEdit();
    void OnModeGroupEdit();
    void OnModePrimitiveMend();
    void OnPrimPerpJump();
    void OnHelpKey();
    void OnOp0();
    void OnOp2();
    void OnOp3();
    void OnOp4();
    void OnOp5();
    void OnOp6();
    void OnOp7();
    void OnOp8();
    void OnReturn();
    void OnEscape();
    void OnEditFind();
    LRESULT OnRedraw(WPARAM wParam, LPARAM lParam);
    void OnRefresh();
    void OnViewerRegen();

protected:
    void OnViewerVpregen();
    void OnUpdateViewerRegen(CCmdUI* pCmdUI);

    DECLARE_MESSAGE_MAP()
public:
    BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) override;
    DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) override;
    BOOL OnIdle(long count);
};
#ifndef _DEBUG  // debug version in PegView.cpp
inline AeSysDoc* AeSysView::GetDocument() const {
    return reinterpret_cast<AeSysDoc*>(m_pDocument);
}
#endif
