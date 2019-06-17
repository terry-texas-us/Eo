#pragma once

#include "Ge/GeScale3d.h"

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

#include "EoDbBlockReference.h"
#include "EoDbEllipse.h"
#include "EoDbHatch.h"
#include "EoDbLine.h"
#include "EoDbPoint.h"
#include "EoDbPolyline.h"
#include "EoDbGroup.h"
#include "EoDbGroupList.h"
#include "PrimState.h"
#include "Section.h"

class AeSysDoc;
class EoDbText;

class AeSysView
	: public CView
	, public OdGiContextForDbDatabase
	, OdEdBaseIO
	, OdExEditorObject::OleDragCallback {

	friend class SaveViewParams;

	void destroyDevice();
	COleDropTarget m_dropTarget;

	OdString m_sPrompt;
	ExEdInputParser m_inpars;

	static unsigned g_nRedrawMSG;
	OdExEditorObject m_editor;
	mutable bool m_bRegenAbort {false};
	mutable bool m_bInRegen {false}; // flag to avoid reentrancy in regen, if new redraw message is received while regen is incomplete (e.g. when assert pops up)

	enum PaintMode { PaintMode_Redraw, PaintMode_Regen };

	PaintMode m_paintMode {PaintMode_Regen};

	CPoint m_oldPoint;
	HCURSOR m_hCursor {nullptr};

	enum Mode { kQuiescent, kGetPoint, kGetString, kDragDrop };

	Mode m_mode {kQuiescent};

	struct Response {
		enum Type { kNone, kPoint, kString, kCancel };
		OdGePoint3d m_Point;
		OdString m_string;
		Type m_type;
	};
	Response m_response;
	int m_inpOptions {0};

	void exeCmd(const OdString& szCmdStr);
	bool beginDragCallback(const OdGePoint3d& point) override;

protected:
	using CView::operator new;
	using CView::operator delete;
private:
	OdDbObjectId m_layoutId;
	bool m_PsOverall {false};

	bool m_bPlotPlotstyle {false};
	bool m_bShowPlotstyle {false};
	bool m_bPlotGrayscale {false};

	OdGiContext::PStyleType plotStyleType() const override;
	void plotStyle(OdDbStub* psNameId, OdPsPlotStyleData& plotStyleData) const override;

	void plotStyle(int penNumber, OdPsPlotStyleData& plotStyleData) const noexcept override {} // OdGiContextForDbDatabase (to suppress C4266 warning)

protected:

	friend OdGsLayoutHelperPtr odGetDocDevice(CDocument* document);

	OdGsLayoutHelperPtr m_LayoutHelper;
	OdGsLayoutHelperPtr m_pPrinterDevice;
	HDC m_hWindowDC {nullptr};
	int m_pagingCounter {0};

	CRect viewportRect() const;
	static CRect viewRect(OdGsView*);

	AeSysView() noexcept; // protected constructor used by dynamic creation

	void preparePlotstyles(const OdDbLayout* layout = nullptr, bool forceReload = false);

	unsigned long glyphSize(GlyphType glyphType) const override;
	void fillContextualColors(OdGiContextualColorsImpl* pCtxColors) override;

	DECLARE_DYNCREATE(AeSysView)

public:
	OdGsView* getActiveView();
	const OdGsView* getActiveView() const;
	OdGsView* getActiveTopView();
	const OdGsView* getActiveTopView() const;
	OdGsLayoutHelper* getDevice() { return m_LayoutHelper; }
	void propagateActiveViewChanges(bool forceAutoRegen = false) const;
	void recreateDevice() { createDevice(true); }

	void track(OdEdInputTracker* inputTracker);
	void setCursor(HCURSOR cursor) noexcept;
	HCURSOR cursor() const noexcept;

	void setViewportBorderProperties();
	// <command_view>
	bool canClose() const;
	bool isGettingString() const noexcept { return m_mode != kQuiescent; }
	OdString prompt() const { return m_sPrompt; }
	int inpOptions() const noexcept { return m_inpOptions; }
	void respond(const OdString& s);
	OdEdCommandPtr command(const OdString& commandName);
	OdExEditorObject& editorObject() noexcept;
	const OdExEditorObject& editorObject() const noexcept;
	bool isModelSpaceView() const;

	OdIntPtr drawableFilterFunctionId(OdDbStub* viewportId) const override; // OdGiContextForDbDatabase
	unsigned long drawableFilterFunction(OdIntPtr functionId, const OdGiDrawable* drawable, unsigned long flags) override; // OdGiContextForDbDatabase

	// </command_view>

public:
	void OnInitialUpdate() override;

protected:
	void OnDraw(CDC* deviceContext) override;
	void OnPrint(CDC* deviceContext, CPrintInfo* printInformation) override;
	void OnEndPrinting(CDC* deviceContext, CPrintInfo* printInformation) override;
	void OnBeginPrinting(CDC* deviceContext, CPrintInfo* printInformation) override;
	BOOL OnPreparePrinting(CPrintInfo* printInformation) override;

	void OnPrepareDC(CDC* deviceContext, CPrintInfo* printInformation) override;

	void OnActivateFrame(unsigned state, CFrameWnd* deactivateFrame) override;
	void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;
	BOOL PreCreateWindow(CREATESTRUCT& createStructure) override;
	void OnUpdate(CView* sender, LPARAM hint, CObject* hintObject) override;

protected:
	void addRef() noexcept override {}
	void release() noexcept override {}

	AeSysDoc* GetDocument() const; // hides non-virtual function of parent

protected:
	~AeSysView();
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

//	void adjustDevice(OdGsDevice* device);
	void createDevice(bool recreate = false);
	bool regenAbort() const noexcept override;

public: // Methods - virtuals 

	unsigned long getKeyState() noexcept override;
	OdGePoint3d getPoint(const OdString& prompt, int options, OdEdPointTracker* tracker) override;
	OdString getString(const OdString& prompt, int options, OdEdStringTracker* tracker) override;
	void putString(const OdString& string) override;

	bool UpdateStringTrackerCursor();

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
	EoDbPrimitive* m_EngagedPrimitive {nullptr};
	EoDbGroup* m_EngagedGroup {nullptr};
	EoGsViewTransform m_OverviewViewTransform;
	unsigned short m_OpHighlighted {0};
	bool m_Plot {false};
	float m_PlotScaleFactor {1.0f};
	EoDbGroup m_PreviewGroup;
	EoGsViewTransform m_PreviousViewTransform;
	unsigned short m_PreviousOp {0};
	OdGePoint3d m_PreviousPnt;
	double m_SelectApertureSize {0.005};
	bool m_ViewBackgroundImage {false};
	bool m_ViewOdometer {true};
	bool m_ViewPenWidths {false};
	CViewports m_Viewports;
	EoGsViewTransforms m_ViewTransforms;
	bool m_ViewTrueTypeFonts {true};
	EoDbGroupList m_VisibleGroupList;
	double m_WorldScale {1.0};
	bool m_LeftButton {false};
	bool m_MiddleButton {false};
	bool m_RightButton {false};

	ERubs m_RubberbandType {None};
	OdGePoint3d m_RubberbandBeginPoint;
	CPoint m_RubberbandLogicalBeginPoint;
	CPoint m_RubberbandLogicalEndPoint;

	OdGePoint3d m_ptCursorPosDev;
	OdGePoint3d	m_ptCursorPosWorld;

	CPoint m_MouseClick;
	CPoint m_MousePosition {0};
	bool m_ZoomWindow {false};
	OdGePoint3dArray m_Points;

	ODCOLORREF m_Background;

	OdGePoint3d m_ptDet;
	OdGeVector3d m_vRelPos;
	OdGePoint3dArray m_DrawModePoints;
	OdGePoint3dArray m_NodalModePoints;
	OdGePoint3dArray m_PipeModePoints;
	OdGePoint3dArray m_PowerModePoints;

private: // grid and axis constraints
	OdGePoint3d m_GridOrigin;
	int m_MaximumDotsPerLine {64};

	double m_XGridLineSpacing {1.0};
	double m_YGridLineSpacing {1.0};
	double m_ZGridLineSpacing {1.0};

	double m_XGridSnapSpacing {12.0};
	double m_YGridSnapSpacing {12.0};
	double m_ZGridSnapSpacing {12.0};

	double m_XGridPointSpacing {3.0};
	double m_YGridPointSpacing {3.0};
	double m_ZGridPointSpacing {0.0};

	double m_AxisConstraintInfluenceAngle {5.0};
	double m_AxisConstraintOffsetAngle {0.0};
	bool m_DisplayGridWithLines {false};
	bool m_DisplayGridWithPoints {false};
	bool m_GridSnap {false};

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
	void SetRenderMode(OdGsView::RenderMode renderMode);
	OdGsView::RenderMode RenderMode() const noexcept {
		return m_ViewTransform.RenderMode();
	}
	const ODCOLORREF* CurrentPalette() const;
public:
	OdDbDatabasePtr Database() const;

protected: // Windows messages
	void OnContextMenu(CWnd*, CPoint point); // hides non-virtual function of parent

public: // Input message handler member functions

	void OnChar(unsigned characterCodeValue, unsigned RepeatCount, unsigned flags); // hides non-virtual function of parent
	void OnKeyDown(unsigned nChar, unsigned repeatCount, unsigned flags); // hides non-virtual function of parent
	void OnLButtonDown(unsigned flags, CPoint point); // hides non-virtual function of parent
	void OnLButtonUp(unsigned flags, CPoint point); // hides non-virtual function of parent
	void OnMButtonDown(unsigned flags, CPoint point); // hides non-virtual function of parent
	void OnMButtonUp(unsigned flags, CPoint point); // hides non-virtual function of parent
	void OnMouseMove(unsigned flags, CPoint point); // hides non-virtual function of parent
	BOOL OnMouseWheel(unsigned flags, short zDelta, CPoint point); // hides non-virtual function of parent
	void OnRButtonDown(unsigned flags, CPoint point); // hides non-virtual function of parent
	void OnRButtonUp(unsigned flags, CPoint point); // hides non-virtual function of parent

	int OnCreate(LPCREATESTRUCT createStructure); // hides non-virtual function of parent
	void OnDestroy(); // hides non-virtual function of parent
	void OnDrag();
	void OnUpdateDrag(CCmdUI* pCmdUI);
	BOOL OnEraseBkgnd(CDC* deviceContext); // hides non-virtual function of parent
	void OnKillFocus(CWnd* newWindow); // hides non-virtual function of parent
	void OnPaint(); // hides non-virtual function of parent
	void OnSetFocus(CWnd* oldWindow); // hides non-virtual function of parent
	void OnSize(unsigned type, int cx, int cy); // hides non-virtual function of parent

	void OnViewStateInformation();
	void OnUpdateViewStateinformation(CCmdUI* pCmdUI);

public:
	static AeSysView* GetActiveView();

	void VerifyFindString(CMFCToolBarComboBoxButton* findCombo, OdString& findText);

	bool m_ViewStateInformation {true}; // Legacy state info within the view
	void UpdateStateInformation(EStateInformationItem item);

	void RubberBandingDisable();
	void RubberBandingStartAtEnable(const OdGePoint3d& point, ERubs type);

	OdGePoint3d GetCursorPosition();
	OdGePoint3d GetWorldCoordinates(CPoint point);
	/// <summary> Positions cursor at targeted position.</summary>
	void SetCursorPosition(const OdGePoint3d& point);
	void SetModeCursor(unsigned mode);

	std::pair<EoDbGroup*, EoDbEllipse*> SelectCircleUsingPoint(const OdGePoint3d& point, double tolerance);
	std::pair<EoDbGroup*, EoDbLine*> SelectLineUsingPoint(const OdGePoint3d& point);
	std::pair<EoDbGroup*, EoDbPoint*> SelectPointUsingPoint(const OdGePoint3d& point, double tolerance, short pointColor);
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

	void ResetView() noexcept;

	/// <summary> Deletes last group detectable in the this view.</summary>
	void DeleteLastGroup();

/// <Section="Visible group interface">
	POSITION AddVisibleGroup(EoDbGroup* group) { return m_VisibleGroupList.AddTail(group); }
	void AddVisibleGroups(EoDbGroupList* groups) { return m_VisibleGroupList.AddTail(groups); }
	POSITION RemoveVisibleGroup(EoDbGroup* group) { return m_VisibleGroupList.Remove(group); }
	void RemoveAllVisibleGroups() { m_VisibleGroupList.RemoveAll(); }
	EoDbGroup* RemoveLastVisibleGroup();

	POSITION GetFirstVisibleGroupPosition() const { return m_VisibleGroupList.GetHeadPosition(); }
	POSITION GetLastGroupPosition() const { return m_VisibleGroupList.GetTailPosition(); }
	EoDbGroup* GetNextVisibleGroup(POSITION& position) { return m_VisibleGroupList.GetNext(position); }
	EoDbGroup* GetPreviousGroup(POSITION& position) { return m_VisibleGroupList.GetPrev(position); }
/// </Section>

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
	unsigned NumPages(CDC* deviceContext, double scaleFactor, unsigned& horizontalPages, unsigned& verticalPages);

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

	EoDbGroup* m_SubModeEditGroup {nullptr};
	EoDbPrimitive* m_SubModeEditPrimitive {nullptr};
	OdGePoint3d m_SubModeEditBeginPoint;
	OdGePoint3d m_SubModeEditEndPoint;
	EoGeMatrix3d m_tmEditSeg;

	void InitializeGroupAndPrimitiveEdit();
	void DoEditGroupCopy();
	void DoEditGroupEscape();
	void DoEditGroupTransform(unsigned short operation);
	void DoEditPrimitiveTransform(unsigned short operation);
	void DoEditPrimitiveCopy();
	void DoEditPrimitiveEscape();
	void PreviewPrimitiveEdit();
	void PreviewGroupEdit();

	OdGePoint3d m_MendPrimitiveBegin;
	unsigned long m_MendPrimitiveVertexIndex {0};
	EoDbPrimitive* m_PrimitiveToMend {nullptr};
	EoDbPrimitive* m_PrimitiveToMendCopy {nullptr};

	void PreviewMendPrimitive();
	void MendPrimitiveEscape();
	void MendPrimitiveReturn();

private: // Annotate and Dimension interface
	double m_GapSpaceFactor {0.5}; // Edge space factor 50 percent of character height
	double m_CircleRadius {0.03125};
	int m_EndItemType {1}; // Arrow type
	double m_EndItemSize {0.1}; // Arrow size
	double m_BubbleRadius {0.125};
	int m_NumberOfSides {0}; // Number of sides on bubble (0 indicating circle)
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
	double m_CenterLineEccentricity {0.5}; // Center line eccentricity for parallel lines
	bool m_ContinueCorner {false};
	EoGeLineSeg3d m_CurrentLeftLine;
	EoGeLineSeg3d m_CurrentRightLine;
	EoGeLineSeg3d m_PreviousReferenceLine;
	EoGeLineSeg3d m_CurrentReferenceLine;
	double m_DistanceBetweenLines {0.0625};
	EoDbGroup* m_AssemblyGroup {nullptr};
	EoDbGroup* m_EndSectionGroup {nullptr};
	EoDbGroup* m_BeginSectionGroup {nullptr};
	EoDbLine* m_BeginSectionLine {nullptr};
	EoDbLine* m_EndSectionLine {nullptr};

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

public: // Fixup mode interface

	enum CornerFlags {
		kTrimPreviousToIntersection = 0x001,
		kTrimCurrentToIntersection = 0x002,
		kTrimPreviousToSize = 0x004,
		kTrimCurrentToSize = 0x008,
		kCorner = 0x100,
		kChamfer = 0x200,
		kFillet = 0x400,
		kCircle = 0x800,
		kTrimBothToIntersection = kTrimPreviousToIntersection | kTrimCurrentToIntersection,
		kTrimPrevious = kTrimPreviousToIntersection | kTrimPreviousToSize,
		kTrimCurrent = kTrimCurrentToIntersection | kTrimCurrentToSize,
	};

	double m_AxisTolerance {2.0};
	double m_CornerSize {0.25};

	void OnFixupModeOptions();
	void OnFixupModeReference();
	void OnFixupModeMend();
	void OnFixupModeChamfer();
	void OnFixupModeFillet();
	void OnFixupModeSquare();
	void OnFixupModeParallel();
	void OnFixupModeReturn();
	void OnFixupModeEscape();

	void GenerateCorner(OdGePoint3d intersection, SelectionPair previousSelection, SelectionPair currentSelection, int cornerType = kCorner | kTrimBothToIntersection);

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
	void SetEditModeMirrorScaleFactors(double sx, double sy, double sz) noexcept;

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
	double m_InsideRadiusFactor {1.5};
	double m_DuctSeamSize {0.03125};
	double m_DuctTapSize {0.03125};
	bool m_GenerateTurningVanes {true};
	EElbow m_ElbowType {Mittered};
	EJust m_DuctJustification {Center};
	double m_TransitionSlope {4.0};
	bool m_BeginWithTransition {false};
	bool m_ContinueSection {false};
	int m_EndCapLocation {0};
	EoDbPoint* m_EndCapPoint {nullptr};
	EoDbGroup* m_EndCapGroup {nullptr};
	bool m_OriginalPreviousGroupDisplayed {true};
	EoDbGroup* m_OriginalPreviousGroup {nullptr};

	Section m_PreviousSection {0.125, 0.0625, Section::Rectangular};
	Section m_CurrentSection {0.125, 0.0625, Section::Rectangular};

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
	void GenerateRiseDrop(unsigned short riseDropIndicator, Section section, EoGeLineSeg3d& referenceLine, EoDbGroup* group);
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
	int m_CurrentPipeSymbolIndex {0};
	double m_PipeTicSize {0.03125};
	double m_PipeRiseDropRadius {0.03125};

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
	bool m_PowerArrow {false};
	bool m_PowerConductor {false};
	double m_PowerConductorSpacing {0.04};
	OdGePoint3d m_CircuitEndPoint;
	double m_PreviousRadius {0.0};

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

	void GeneratePowerConductorSymbol(unsigned short conductorType, const OdGePoint3d& pointOnCircuit, const OdGePoint3d& endPoint);
	void GenerateHomeRunArrow(const OdGePoint3d& pointOnCircuit, const OdGePoint3d& endPoint);
	void DoPowerModeConductor(unsigned short conductorType);

public: // Status & Mode Line
	void ModeLineDisplay();
	unsigned short ModeLineHighlightOp(unsigned short op);
	void ModeLineUnhighlightOp(unsigned short& op);

	CMFCStatusBar& GetStatusBar() const;

public:
	void OnBackgroundImageLoad();
	void OnBackgroundImageRemove();
	void OnFilePlotHalf();
	void OnFilePlotFull();
	void OnFilePlotQuarter();
	void OnFilePrint(); // hides non-virtual function of parent
	void OnFind();
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
	void OnViewRendermode(unsigned commandId);
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
	void OnEditFind() noexcept;
	LRESULT OnRedraw(WPARAM wParam, LPARAM lParam);
	void OnRefresh();
	void OnViewerRegen();

protected:
	void OnViewerVpregen();
	void OnUpdateViewerRegen(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()
public:
	BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) override;
	DROPEFFECT OnDragOver(COleDataObject* dataObject, unsigned long keyState, CPoint point) override;
	BOOL OnIdle(long count);
};
#ifndef _DEBUG  // debug version in PegView.cpp
inline AeSysDoc* AeSysView::GetDocument() const {
	return reinterpret_cast<AeSysDoc*>(m_pDocument);
}
#endif
