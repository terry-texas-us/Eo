#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

#include "AbstractViewPE.h"
#include "ColorMapping.h"
#include "DbAbstractViewportData.h"
#include "DbViewTable.h"
#include "DbViewTableRecord.h"
#include "DbGsManager.h"
#include "RxVariantValue.h"
#include "SaveState.h"
#include "DbPageController.h"
#include "DbIdMapping.h"

#include "EoDlgSetAngle.h"
#include "EoDlgSetUnitsAndPrecision.h"
#include "EoDlgSetupConstraints.h"
#include "EoDlgSetupCustomMouseCharacters.h"
#include "EoDlgSetLength.h"
#include "EoDlgViewZoom.h"
#include "EoDlgSetScale.h"
#include "EoDbBitmapFile.h"
#include "EoDlgBlockInsert.h"
#include "EoDlgSelectIsometricView.h"
#include "EoDlgViewParameters.h"

#include "EoDbHatch.h"
#include "EoDbPolyline.h"

#include <atltypes.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef OdArray<ODCOLORREF, OdMemoryAllocator<ODCOLORREF> > ODGSPALETTE;

const double AeSysView::sm_MaximumWindowRatio = 999.;
const double AeSysView::sm_MinimumWindowRatio = 0.001;

UINT AeSysView::g_nRedrawMSG = 0;

IMPLEMENT_DYNCREATE(AeSysView, CView)

BEGIN_MESSAGE_MAP(AeSysView, CView)
	ON_WM_CHAR()
	ON_WM_CONTEXTMENU()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()

	ON_COMMAND(ID_OP0, OnOp0)
	ON_COMMAND(ID_OP2, OnOp2)
	ON_COMMAND(ID_OP3, OnOp3)
	ON_COMMAND(ID_OP4, OnOp4)
	ON_COMMAND(ID_OP5, OnOp5)
	ON_COMMAND(ID_OP6, OnOp6)
	ON_COMMAND(ID_OP7, OnOp7)
	ON_COMMAND(ID_OP8, OnOp8)
	ON_COMMAND(IDM_RETURN, OnReturn)
	ON_COMMAND(IDM_ESCAPE, OnEscape)

	ON_COMMAND(ID_EDIT_FIND_COMBO, OnFind)
	ON_COMMAND(ID_3DVIEWS_BACK, On3dViewsBack)
	ON_COMMAND(ID_3DVIEWS_BOTTOM, On3dViewsBottom)
	ON_COMMAND(ID_3DVIEWS_FRONT, On3dViewsFront)
	ON_COMMAND(ID_3DVIEWS_ISOMETRIC, On3dViewsIsometric)
	ON_COMMAND(ID_3DVIEWS_LEFT, On3dViewsLeft)
	ON_COMMAND(ID_3DVIEWS_RIGHT, On3dViewsRight)
	ON_COMMAND(ID_3DVIEWS_TOP, On3dViewsTop)

	ON_COMMAND(ID_BACKGROUNDIMAGE_LOAD, OnBackgroundImageLoad)
	ON_COMMAND(ID_BACKGROUNDIMAGE_REMOVE, OnBackgroundImageRemove)
	ON_COMMAND(ID_CAMERA_ROTATELEFT, OnCameraRotateLeft)
	ON_COMMAND(ID_CAMERA_ROTATERIGHT, OnCameraRotateRight)
	ON_COMMAND(ID_CAMERA_ROTATEUP, OnCameraRotateUp)
	ON_COMMAND(ID_CAMERA_ROTATEDOWN, OnCameraRotateDown)
	ON_COMMAND(ID_EDIT_FIND, &AeSysView::OnEditFind)
	ON_COMMAND(ID_FILE_PLOT_QUARTER, OnFilePlotQuarter)
	ON_COMMAND(ID_FILE_PLOT_HALF, OnFilePlotHalf)
	ON_COMMAND(ID_FILE_PLOT_FULL, OnFilePlotFull)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_REGISTERED_MESSAGE(g_nRedrawMSG, OnRedraw)

	ON_COMMAND(ID_HELP_KEY, OnHelpKey)
	ON_COMMAND(ID_MODE_GROUP_EDIT, OnModeGroupEdit)
	ON_COMMAND(ID_MODE_PRIMITIVE_EDIT, OnModePrimitiveEdit)
	ON_COMMAND(ID_MODE_PRIMITIVE_MEND, OnModePrimitiveMend)
	ON_COMMAND(ID_PRIM_PERPJUMP, OnPrimPerpJump)
	ON_COMMAND(ID_RELATIVEMOVES_ENG_DOWN, OnRelativeMovesEngDown)
	ON_COMMAND(ID_RELATIVEMOVES_ENG_DOWNROTATE, OnRelativeMovesEngDownRotate)
	ON_COMMAND(ID_RELATIVEMOVES_ENG_IN, OnRelativeMovesEngIn)
	ON_COMMAND(ID_RELATIVEMOVES_ENG_LEFT, OnRelativeMovesEngLeft)
	ON_COMMAND(ID_RELATIVEMOVES_ENG_LEFTROTATE, OnRelativeMovesEngLeftRotate)
	ON_COMMAND(ID_RELATIVEMOVES_ENG_OUT, OnRelativeMovesEngOut)
	ON_COMMAND(ID_RELATIVEMOVES_ENG_RIGHT, OnRelativeMovesEngRight)
	ON_COMMAND(ID_RELATIVEMOVES_ENG_RIGHTROTATE, OnRelativeMovesEngRightRotate)
	ON_COMMAND(ID_RELATIVEMOVES_ENG_UP, OnRelativeMovesEngUp)
	ON_COMMAND(ID_RELATIVEMOVES_ENG_UPROTATE, OnRelativeMovesEngUpRotate)
	ON_COMMAND(ID_RELATIVEMOVES_DOWN, OnRelativeMovesDown)
	ON_COMMAND(ID_RELATIVEMOVES_DOWNROTATE, OnRelativeMovesDownRotate)
	ON_COMMAND(ID_RELATIVEMOVES_IN, OnRelativeMovesIn)
	ON_COMMAND(ID_RELATIVEMOVES_LEFT, OnRelativeMovesLeft)
	ON_COMMAND(ID_RELATIVEMOVES_LEFTROTATE, OnRelativeMovesLeftRotate)
	ON_COMMAND(ID_RELATIVEMOVES_OUT, OnRelativeMovesOut)
	ON_COMMAND(ID_RELATIVEMOVES_RIGHT, OnRelativeMovesRight)
	ON_COMMAND(ID_RELATIVEMOVES_RIGHTROTATE, OnRelativeMovesRightRotate)
	ON_COMMAND(ID_RELATIVEMOVES_UP, OnRelativeMovesUp)
	ON_COMMAND(ID_RELATIVEMOVES_UPROTATE, OnRelativeMovesUpRotate)
	ON_COMMAND(ID_SETUP_CONSTRAINTS, OnSetupConstraints)
	ON_COMMAND(ID_SETUP_DIMANGLE, OnSetupDimAngle)
	ON_COMMAND(ID_SETUP_DIMLENGTH, OnSetupDimLength)
	ON_COMMAND(ID_SETUP_MOUSEBUTTONS, OnSetupMouseButtons)
	ON_COMMAND(ID_SETUP_SCALE, OnSetupScale)
	ON_COMMAND(ID_SETUP_UNITS, OnSetupUnits)
	ON_COMMAND(ID_TOOLS_PRIMITIVE_SNAPTO, OnToolsPrimitiveSnapto)
	ON_COMMAND(ID_VIEW_BACKGROUNDIMAGE, OnViewBackgroundImage)
	ON_COMMAND(ID_VIEW_ODOMETER, OnViewOdometer)
	ON_COMMAND(ID_VIEW_PARAMETERS, OnViewParameters)
	ON_COMMAND(ID_VIEW_PENWIDTHS, OnViewPenWidths)
	ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
	ON_COMMAND(ID_VIEW_STATEINFORMATION, OnViewStateInformation)
	ON_COMMAND(ID_VIEW_TRUETYPEFONTS, OnViewTrueTypeFonts)
	ON_COMMAND(ID_VIEW_WINDOW, OnViewWindow)
	ON_COMMAND_RANGE(ID_VIEW_RENDERMODE_2DOPTIMIZED, ID_VIEW_RENDERMODE_SMOOTHSHADED, &AeSysView::OnViewRendermode)
	ON_COMMAND(ID_WINDOW_BEST, OnWindowBest)
	ON_COMMAND(ID_WINDOW_NORMAL, OnWindowNormal)
	ON_COMMAND(ID_WINDOW_LAST, OnWindowLast)
	ON_COMMAND(ID_WINDOW_PAN, OnWindowPan)
	ON_COMMAND(ID_WINDOW_PAN_LEFT, OnWindowPanLeft)
	ON_COMMAND(ID_WINDOW_PAN_RIGHT, OnWindowPanRight)
	ON_COMMAND(ID_WINDOW_PAN_UP, OnWindowPanUp)
	ON_COMMAND(ID_WINDOW_PAN_DOWN, OnWindowPanDown)
	ON_COMMAND(ID_WINDOW_SHEET, OnWindowSheet)
	ON_COMMAND(ID_WINDOW_ZOOMIN, OnWindowZoomIn)
	ON_COMMAND(ID_WINDOW_ZOOMOUT, OnWindowZoomOut)
	ON_COMMAND(ID_WINDOW_ZOOMWINDOW, &AeSysView::OnWindowZoomWindow)
	ON_COMMAND(ID_WINDOW_ZOOMSPECIAL, &AeSysView::OnWindowZoomSpecial)

	ON_UPDATE_COMMAND_UI(ID_BACKGROUNDIMAGE_LOAD, OnUpdateBackgroundimageLoad)
	ON_UPDATE_COMMAND_UI(ID_BACKGROUNDIMAGE_REMOVE, OnUpdateBackgroundimageRemove)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BACKGROUNDIMAGE, OnUpdateViewBackgroundImage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ODOMETER, OnUpdateViewOdometer)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PENWIDTHS, OnUpdateViewPenwidths)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STATEINFORMATION, OnUpdateViewStateinformation)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TRUETYPEFONTS, OnUpdateViewTrueTypeFonts)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RENDERMODE_2DOPTIMIZED, &AeSysView::OnUpdateViewRendermode2doptimized)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RENDERMODE_WIREFRAME, &AeSysView::OnUpdateViewRendermodeWireframe)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RENDERMODE_HIDDENLINE, &AeSysView::OnUpdateViewRendermodeHiddenline)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RENDERMODE_FLATSHADED, &AeSysView::OnUpdateViewRendermodeFlatshaded)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RENDERMODE_SMOOTHSHADED, &AeSysView::OnUpdateViewRendermodeSmoothshaded)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_ZOOMWINDOW, OnUpdateWindowZoomWindow)

	ON_COMMAND(ID_DRAW_MODE_OPTIONS, &AeSysView::OnDrawModeOptions)
	ON_COMMAND(ID_DRAW_MODE_POINT, &AeSysView::OnDrawModePoint)
	ON_COMMAND(ID_DRAW_MODE_LINE, &AeSysView::OnDrawModeLine)
	ON_COMMAND(ID_DRAW_MODE_POLYGON, &AeSysView::OnDrawModePolygon)
	ON_COMMAND(ID_DRAW_MODE_QUAD, &AeSysView::OnDrawModeQuad)
	ON_COMMAND(ID_DRAW_MODE_ARC, &AeSysView::OnDrawModeArc)
	ON_COMMAND(ID_DRAW_MODE_BSPLINE, &AeSysView::OnDrawModeBspline)
	ON_COMMAND(ID_DRAW_MODE_CIRCLE, &AeSysView::OnDrawModeCircle)
	ON_COMMAND(ID_DRAW_MODE_ELLIPSE, &AeSysView::OnDrawModeEllipse)
	ON_COMMAND(ID_DRAW_MODE_INSERT, &AeSysView::OnDrawModeInsert)
	ON_COMMAND(ID_DRAW_MODE_RETURN, &AeSysView::OnDrawModeReturn)
	ON_COMMAND(ID_DRAW_MODE_ESCAPE, &AeSysView::OnDrawModeEscape)

	ON_COMMAND(ID_ANNOTATE_MODE_OPTIONS, &AeSysView::OnAnnotateModeOptions)
	ON_COMMAND(ID_ANNOTATE_MODE_LINE, &AeSysView::OnAnnotateModeLine)
	ON_COMMAND(ID_ANNOTATE_MODE_ARROW, &AeSysView::OnAnnotateModeArrow)
	ON_COMMAND(ID_ANNOTATE_MODE_BUBBLE, &AeSysView::OnAnnotateModeBubble)
	ON_COMMAND(ID_ANNOTATE_MODE_HOOK, &AeSysView::OnAnnotateModeHook)
	ON_COMMAND(ID_ANNOTATE_MODE_UNDERLINE, &AeSysView::OnAnnotateModeUnderline)
	ON_COMMAND(ID_ANNOTATE_MODE_BOX, &AeSysView::OnAnnotateModeBox)
	ON_COMMAND(ID_ANNOTATE_MODE_CUT_IN, &AeSysView::OnAnnotateModeCutIn)
	ON_COMMAND(ID_ANNOTATE_MODE_CONSTRUCTION_LINE, &AeSysView::OnAnnotateModeConstructionLine)
	ON_COMMAND(ID_ANNOTATE_MODE_RETURN, &AeSysView::OnAnnotateModeReturn)
	ON_COMMAND(ID_ANNOTATE_MODE_ESCAPE, &AeSysView::OnAnnotateModeEscape)

	ON_COMMAND(ID_PIPE_MODE_OPTIONS, &AeSysView::OnPipeModeOptions)
	ON_COMMAND(ID_PIPE_MODE_LINE, &AeSysView::OnPipeModeLine)
	ON_COMMAND(ID_PIPE_MODE_FITTING, &AeSysView::OnPipeModeFitting)
	ON_COMMAND(ID_PIPE_MODE_RISE, &AeSysView::OnPipeModeRise)
	ON_COMMAND(ID_PIPE_MODE_DROP, &AeSysView::OnPipeModeDrop)
	ON_COMMAND(ID_PIPE_MODE_SYMBOL, &AeSysView::OnPipeModeSymbol)
	ON_COMMAND(ID_PIPE_MODE_WYE, &AeSysView::OnPipeModeWye)
	ON_COMMAND(ID_PIPE_MODE_RETURN, &AeSysView::OnPipeModeReturn)
	ON_COMMAND(ID_PIPE_MODE_ESCAPE, &AeSysView::OnPipeModeEscape)

	ON_COMMAND(ID_POWER_MODE_OPTIONS, &AeSysView::OnPowerModeOptions)
	ON_COMMAND(ID_POWER_MODE_CIRCUIT, &AeSysView::OnPowerModeCircuit)
	ON_COMMAND(ID_POWER_MODE_GROUND, &AeSysView::OnPowerModeGround)
	ON_COMMAND(ID_POWER_MODE_HOT, &AeSysView::OnPowerModeHot)
	ON_COMMAND(ID_POWER_MODE_SWITCH, &AeSysView::OnPowerModeSwitch)
	ON_COMMAND(ID_POWER_MODE_NEUTRAL, &AeSysView::OnPowerModeNeutral)
	ON_COMMAND(ID_POWER_MODE_HOME, &AeSysView::OnPowerModeHome)
	ON_COMMAND(ID_POWER_MODE_RETURN, &AeSysView::OnPowerModeReturn)
	ON_COMMAND(ID_POWER_MODE_ESCAPE, &AeSysView::OnPowerModeEscape)

	ON_COMMAND(ID_DRAW2_MODE_OPTIONS, &AeSysView::OnDraw2ModeOptions)
	ON_COMMAND(ID_DRAW2_MODE_JOIN, &AeSysView::OnDraw2ModeJoin)
	ON_COMMAND(ID_DRAW2_MODE_WALL, &AeSysView::OnDraw2ModeWall)
	ON_COMMAND(ID_DRAW2_MODE_RETURN, &AeSysView::OnDraw2ModeReturn)
	ON_COMMAND(ID_DRAW2_MODE_ESCAPE, &AeSysView::OnDraw2ModeEscape)

	ON_COMMAND(ID_LPD_MODE_OPTIONS, &AeSysView::OnLpdModeOptions)
	ON_COMMAND(ID_LPD_MODE_JOIN, &AeSysView::OnLpdModeJoin)
	ON_COMMAND(ID_LPD_MODE_DUCT, &AeSysView::OnLpdModeDuct)
	ON_COMMAND(ID_LPD_MODE_TRANSITION, &AeSysView::OnLpdModeTransition)
	ON_COMMAND(ID_LPD_MODE_TAP, &AeSysView::OnLpdModeTap)
	ON_COMMAND(ID_LPD_MODE_ELL, &AeSysView::OnLpdModeEll)
	ON_COMMAND(ID_LPD_MODE_TEE, &AeSysView::OnLpdModeTee)
	ON_COMMAND(ID_LPD_MODE_UP_DOWN, &AeSysView::OnLpdModeUpDown)
	ON_COMMAND(ID_LPD_MODE_SIZE, &AeSysView::OnLpdModeSize)
	ON_COMMAND(ID_LPD_MODE_RETURN, &AeSysView::OnLpdModeReturn)
	ON_COMMAND(ID_LPD_MODE_ESCAPE, &AeSysView::OnLpdModeEscape)

	ON_COMMAND(ID_EDIT_MODE_OPTIONS, &AeSysView::OnEditModeOptions)
	ON_COMMAND(ID_EDIT_MODE_PIVOT, &AeSysView::OnEditModePivot)
	ON_COMMAND(ID_EDIT_MODE_ROTCCW, &AeSysView::OnEditModeRotccw)
	ON_COMMAND(ID_EDIT_MODE_ROTCW, &AeSysView::OnEditModeRotcw)
	ON_COMMAND(ID_EDIT_MODE_MOVE, &AeSysView::OnEditModeMove)
	ON_COMMAND(ID_EDIT_MODE_COPY, &AeSysView::OnEditModeCopy)
	ON_COMMAND(ID_EDIT_MODE_FLIP, &AeSysView::OnEditModeFlip)
	ON_COMMAND(ID_EDIT_MODE_REDUCE, &AeSysView::OnEditModeReduce)
	ON_COMMAND(ID_EDIT_MODE_ENLARGE, &AeSysView::OnEditModeEnlarge)
	ON_COMMAND(ID_EDIT_MODE_RETURN, &AeSysView::OnEditModeReturn)
	ON_COMMAND(ID_EDIT_MODE_ESCAPE, &AeSysView::OnEditModeEscape)

	ON_COMMAND(ID_TRAP_MODE_REMOVE_ADD, &AeSysView::OnTrapModeRemoveAdd)
	ON_COMMAND(ID_TRAP_MODE_POINT, &AeSysView::OnTrapModePoint)
	ON_COMMAND(ID_TRAP_MODE_STITCH, &AeSysView::OnTrapModeStitch)
	ON_COMMAND(ID_TRAP_MODE_FIELD, &AeSysView::OnTrapModeField)
	ON_COMMAND(ID_TRAP_MODE_LAST, &AeSysView::OnTrapModeLast)
	ON_COMMAND(ID_TRAP_MODE_ENGAGE, &AeSysView::OnTrapModeEngage)
	ON_COMMAND(ID_TRAP_MODE_MODIFY, &AeSysView::OnTrapModeModify)
	ON_COMMAND(ID_TRAP_MODE_ESCAPE, &AeSysView::OnTrapModeEscape)
	ON_COMMAND(ID_TRAP_MODE_MENU, &AeSysView::OnTrapModeMenu)

	ON_COMMAND(ID_TRAPR_MODE_REMOVE_ADD, &AeSysView::OnTraprModeRemoveAdd)
	ON_COMMAND(ID_TRAPR_MODE_POINT, &AeSysView::OnTraprModePoint)
	ON_COMMAND(ID_TRAPR_MODE_STITCH, &AeSysView::OnTraprModeStitch)
	ON_COMMAND(ID_TRAPR_MODE_FIELD, &AeSysView::OnTraprModeField)
	ON_COMMAND(ID_TRAPR_MODE_LAST, &AeSysView::OnTraprModeLast)
	ON_COMMAND(ID_TRAPR_MODE_ENGAGE, &AeSysView::OnTraprModeEngage)
	ON_COMMAND(ID_TRAPR_MODE_MENU, &AeSysView::OnTraprModeMenu)
	ON_COMMAND(ID_TRAPR_MODE_MODIFY, &AeSysView::OnTraprModeModify)
	ON_COMMAND(ID_TRAPR_MODE_ESCAPE, &AeSysView::OnTraprModeEscape)

	ON_COMMAND(ID_DIMENSION_MODE_OPTIONS, &AeSysView::OnDimensionModeOptions)
	ON_COMMAND(ID_DIMENSION_MODE_ARROW, &AeSysView::OnDimensionModeArrow)
	ON_COMMAND(ID_DIMENSION_MODE_LINE, &AeSysView::OnDimensionModeLine)
	ON_COMMAND(ID_DIMENSION_MODE_DLINE, &AeSysView::OnDimensionModeDLine)
	ON_COMMAND(ID_DIMENSION_MODE_DLINE2, &AeSysView::OnDimensionModeDLine2)
	ON_COMMAND(ID_DIMENSION_MODE_EXTEN, &AeSysView::OnDimensionModeExten)
	ON_COMMAND(ID_DIMENSION_MODE_RADIUS, &AeSysView::OnDimensionModeRadius)
	ON_COMMAND(ID_DIMENSION_MODE_DIAMETER, &AeSysView::OnDimensionModeDiameter)
	ON_COMMAND(ID_DIMENSION_MODE_ANGLE, &AeSysView::OnDimensionModeAngle)
	ON_COMMAND(ID_DIMENSION_MODE_CONVERT, &AeSysView::OnDimensionModeConvert)
	ON_COMMAND(ID_DIMENSION_MODE_RETURN, &AeSysView::OnDimensionModeReturn)
	ON_COMMAND(ID_DIMENSION_MODE_ESCAPE, &AeSysView::OnDimensionModeEscape)

	ON_COMMAND(ID_CUT_MODE_OPTIONS, &AeSysView::OnCutModeOptions)
	ON_COMMAND(ID_CUT_MODE_TORCH, &AeSysView::OnCutModeTorch)
	ON_COMMAND(ID_CUT_MODE_SLICE, &AeSysView::OnCutModeSlice)
	ON_COMMAND(ID_CUT_MODE_FIELD, &AeSysView::OnCutModeField)
	ON_COMMAND(ID_CUT_MODE_CLIP, &AeSysView::OnCutModeClip)
	ON_COMMAND(ID_CUT_MODE_DIVIDE, &AeSysView::OnCutModeDivide)
	ON_COMMAND(ID_CUT_MODE_RETURN, &AeSysView::OnCutModeReturn)
	ON_COMMAND(ID_CUT_MODE_ESCAPE, &AeSysView::OnCutModeEscape)

	ON_COMMAND(ID_FIXUP_MODE_OPTIONS, &AeSysView::OnFixupModeOptions)
	ON_COMMAND(ID_FIXUP_MODE_REFERENCE, &AeSysView::OnFixupModeReference)
	ON_COMMAND(ID_FIXUP_MODE_MEND, &AeSysView::OnFixupModeMend)
	ON_COMMAND(ID_FIXUP_MODE_CHAMFER, &AeSysView::OnFixupModeChamfer)
	ON_COMMAND(ID_FIXUP_MODE_FILLET, &AeSysView::OnFixupModeFillet)
	ON_COMMAND(ID_FIXUP_MODE_SQUARE, &AeSysView::OnFixupModeSquare)
	ON_COMMAND(ID_FIXUP_MODE_PARALLEL, &AeSysView::OnFixupModeParallel)
	ON_COMMAND(ID_FIXUP_MODE_RETURN, &AeSysView::OnFixupModeReturn)
	ON_COMMAND(ID_FIXUP_MODE_ESCAPE, &AeSysView::OnFixupModeEscape)

	ON_COMMAND(ID_NODAL_MODE_ADDREMOVE, &AeSysView::OnNodalModeAddRemove)
	ON_COMMAND(ID_NODAL_MODE_POINT, &AeSysView::OnNodalModePoint)
	ON_COMMAND(ID_NODAL_MODE_LINE, &AeSysView::OnNodalModeLine)
	ON_COMMAND(ID_NODAL_MODE_AREA, &AeSysView::OnNodalModeArea)
	ON_COMMAND(ID_NODAL_MODE_MOVE, &AeSysView::OnNodalModeMove)
	ON_COMMAND(ID_NODAL_MODE_COPY, &AeSysView::OnNodalModeCopy)
	ON_COMMAND(ID_NODAL_MODE_TOLINE, &AeSysView::OnNodalModeToLine)
	ON_COMMAND(ID_NODAL_MODE_TOPOLYGON, &AeSysView::OnNodalModeToPolygon)
	ON_COMMAND(ID_NODAL_MODE_EMPTY, &AeSysView::OnNodalModeEmpty)
	ON_COMMAND(ID_NODAL_MODE_ENGAGE, &AeSysView::OnNodalModeEngage)
	ON_COMMAND(ID_NODAL_MODE_RETURN, &AeSysView::OnNodalModeReturn)
	ON_COMMAND(ID_NODAL_MODE_ESCAPE, &AeSysView::OnNodalModeEscape)
	ON_COMMAND(ID_INSERT_BLOCKREFERENCE, &AeSysView::OnInsertBlockreference)
END_MESSAGE_MAP()

AeSysView::AeSysView() noexcept 
	: m_hWindowDC(0)
	, m_bRegenAbort(false)
	, m_bPsOverall(false)
	, m_mode(kQuiescent)
	, m_hCursor(0)
	, m_bInRegen(false)
	, m_paintMode(PaintMode_Regen)
	, m_bPlotPlotstyle(false)
	, m_bShowPlotstyle(false)
	, m_bPlotGrayscale(false)
	, m_pagingCounter(0)
	, m_RubberbandType(None)
	, m_LeftButton(false)
	, m_MiddleButton(false)
	, m_RightButton(false)
	, m_MousePosition(0)
	, m_ZoomWindow(false)
	, m_Points(0)
	, m_inpOptions(0) 
{
	m_Background = ViewBackgroundColor;

	m_PreviousOp = 0;
	m_Plot = false;
	m_ViewStateInformation = true;		// View state info within the view
	m_ViewBackgroundImage = false;
	m_ViewOdometer = true;
	m_ViewPenWidths = false;
	m_WorldScale = 1.;
	m_ViewTrueTypeFonts = true;
	m_OpHighlighted = 0;
	m_SelectApertureSize = .005;
	m_PlotScaleFactor = 1.0f;
	m_GapSpaceFactor = .5;			// Edge space factor 50 percent of character height
	m_CircleRadius = .03125;		// Circle radius
	m_EndItemType = 1;				// Arrow type
	m_EndItemSize = .1;				// Arrow size
	m_BubbleRadius = .125;			// Bubble radius
	m_NumberOfSides = 0;			// Number of sides on bubble (0 indicating circle)
	m_EngagedPrimitive = 0;
	m_EngagedGroup = 0;

	m_CenterLineEccentricity = .5;	// Center line eccentricity for parallel lines
	m_ContinueCorner = false;
	m_AssemblyGroup = nullptr;
	m_BeginSectionGroup = nullptr;
	m_EndSectionGroup = nullptr;
	m_BeginSectionLine = nullptr;
	m_EndSectionLine = nullptr;
	m_DistanceBetweenLines = .0625;

	// Constraints - grid and axis
	m_MaximumDotsPerLine = 64;

	m_XGridLineSpacing = 1.;
	m_YGridLineSpacing = 1.;
	m_ZGridLineSpacing = 1.;

	m_XGridSnapSpacing = 12.;
	m_YGridSnapSpacing = 12.;
	m_ZGridSnapSpacing = 12.;

	m_XGridPointSpacing = 3.;
	m_YGridPointSpacing = 3.;
	m_ZGridPointSpacing = 0.;

	m_AxisConstraintInfluenceAngle = 5.;
	m_AxisConstraintOffsetAngle = 0.;

	m_DisplayGridWithLines = false;
	m_DisplayGridWithPoints = false;
	m_GridSnap = false;

	m_SubModeEditGroup = 0;
	m_SubModeEditPrimitive = 0;

	m_MendPrimitiveVertexIndex = 0;
	m_PrimitiveToMend = 0;
	m_PrimitiveToMendCopy = 0;

	SetEditModeMirrorScaleFactors(-1, 1., 1.);
	SetEditModeScaleFactors(2., 2., 2.);

	SetEditModeRotationAngles(0., 0., 45.);

	m_AxisTolerance = 2.;
	m_CornerSize = .25;

	m_GenerateTurningVanes = true;	// turning vanes generation flag
	m_InsideRadiusFactor = 1.5;		// inside radius elbow factor
	m_DuctSeamSize = .03125;
	m_DuctTapSize = .09375;			// tap size
	m_ContinueSection = false;
	m_BeginWithTransition = false;
	m_DuctJustification = Center;	// justification (Left, Center or Right)
	m_TransitionSlope = 4.;
	m_ElbowType = Mittered;			// elbow type (Mittered or Radial)
	m_EndCapGroup = nullptr;
	m_EndCapPoint = nullptr;
	m_EndCapLocation = 0;
	m_OriginalPreviousGroupDisplayed = true;
	m_OriginalPreviousGroup = 0;

	m_PreviousSection(.125, .0625, Section::Rectangular);
	m_CurrentSection(.125, .0625, Section::Rectangular);
	m_PipeTicSize = .03125;
	m_PipeRiseDropRadius = .03125;
	m_CurrentPipeSymbolIndex = 0;

	// Power mode
	m_PowerArrow = false;
	m_PowerConductor = false;
	m_PowerConductorSpacing = .04;
	m_PreviousRadius = 0.;

	m_Viewport.SetDeviceWidthInPixels(theApp.DeviceWidthInPixels());
	m_Viewport.SetDeviceHeightInPixels(theApp.DeviceHeightInPixels());
	m_Viewport.SetDeviceWidthInInches(theApp.DeviceWidthInMillimeters() / EoMmPerInch);
	m_Viewport.SetDeviceHeightInInches(theApp.DeviceHeightInMillimeters() / EoMmPerInch);
}

AeSysView::~AeSysView() {}


#ifdef _DEBUG
void AeSysView::AssertValid() const {
	CView::AssertValid();
}
void AeSysView::Dump(CDumpContext & dc) const {
	CView::Dump(dc);
}
AeSysDoc* AeSysView::GetDocument() const {
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(AeSysDoc)));
	return (AeSysDoc*) m_pDocument;
}
#endif //_DEBUG

void AeSysView::exeCmd(const OdString& commandName) {
	GetDocument()->ExecuteCommand(commandName);
	propagateActiveViewChanges(true);
}

void AeSysView::OnDraw(CDC* deviceContext) {
	try {
		auto Document {GetDocument()};
		ASSERT_VALID(Document);
		
		BackgroundImageDisplay(deviceContext);
		DisplayGrid(deviceContext);

		if (m_LayoutHelper.isNull()) {
			Document->DisplayAllLayers(this, deviceContext);
		} else {
			Document->BuildVisibleGroupList(this);
			// <tas="background and grid display are obscurred by this update."/>
			m_LayoutHelper->update();
		}
		Document->DisplayUniquePoints();
		UpdateStateInformation(All);
		ModeLineDisplay();
		ValidateRect(NULL);
	} catch (CException * Exception) {
		Exception->Delete();
	}
}

void AeSysView::OnInitialUpdate() {
	::SetClassLongPtr(GetSafeHwnd(), GCLP_HBRBACKGROUND, (LONG_PTR) ::CreateSolidBrush(ViewBackgroundColor));

	CView::OnInitialUpdate();

	AeSysDoc* Document {static_cast<AeSysDoc*>(GetDocument())};

	OdDbDatabase* Database {Document->m_DatabasePtr};
	OdGiContextForDbDatabase::setDatabase(Database);

	m_hWindowDC = ::GetDC(m_hWnd);

	if (!g_nRedrawMSG) {
		g_nRedrawMSG = ::RegisterWindowMessageW(L"AeSysApp::AeSysView::WM_REDRAW");
	}
	createDevice();
		if (m_LayoutHelper.isNull()) {
		GetParent()->PostMessage(WM_CLOSE);
		return;
	}
	Document->setVectorizer(this);

	m_editor.Initialize(m_LayoutHelper, Document->CommandContext());

	theApp.OnModeDraw();
}

bool AeSysView::regenAbort() const noexcept {
	return false;
}

LRESULT AeSysView::OnRedraw(WPARAM wParam, LPARAM lParam) {

	if (m_bInRegen) { return 1; }

	m_bInRegen = true;
	m_bRegenAbort = false;

	auto  MainFrame {(CMainFrame*)theApp.GetMainWnd()};

	if (!regenAbort()) {
		try {
			MainFrame->StartTimer();
			if (m_LayoutHelper.get()) {
				setViewportBorderProperties();
				m_LayoutHelper->update();
			}
			if (!regenAbort()) {
				MainFrame->StopTimer((m_paintMode == PaintMode_Regen) ? L"Regen" : L"Redraw");
			}
		} catch (const OdError& Error) {
			theApp.reportError(L"Rendering aborted", Error);
			GetParent()->PostMessage(WM_CLOSE);
		} catch (const UserBreak&) {
			theApp.reportError(L"Rendering aborted", OdError(eUserBreak));
			GetParent()->PostMessage(WM_CLOSE);
		}
#ifndef _DEBUG
		catch (...) {
			theApp.reportError(L"Rendering aborted", OdError("Unknown exception is caught..."));
			GetParent()->PostMessage(WM_CLOSE);
		}
#endif //#ifndef _DEBUG
	}
	m_bRegenAbort = false;
	m_bInRegen = false;
	m_paintMode = PaintMode_Redraw;
	return 1;
}

void AeSysView::OnPaint(void) {
	/* <tas="Code section to enable when custom redraw message processing added">
		m_bRegenAbort = true;

		PAINTSTRUCT PaintStruct;
		BeginPaint(&PaintStruct);
		EndPaint(&PaintStruct);

		MSG Message;
		while(::PeekMessageW(&Message, NULL, g_nRedrawMSG, g_nRedrawMSG, PM_REMOVE)) {
		;
		}
		PostMessage(g_nRedrawMSG);
		</tas> */
	CView::OnPaint();
}
BOOL AeSysView::OnEraseBkgnd(CDC * deviceContext) {
	// TODO: Add your message handler code here and/or call default

	return __super::OnEraseBkgnd(deviceContext);
}

void AeSysView::OnSize(UINT type, int cx, int cy) {
	if (cx && cy) {
		if (m_LayoutHelper.isNull()) {
			__super::OnSize(type, cx, cy);
		} else {
			SetViewportSize(cx, cy);
			m_LayoutHelper->onSize(OdGsDCRect(0, cx, cy, 0));

			const auto Target {OdGePoint3d(m_ViewTransform.FieldWidth() / 2., m_ViewTransform.FieldHeight() / 2., 0.)};
			const auto Position {Target + (OdGeVector3d::kZAxis * m_ViewTransform.LensLength())};
			OdGsViewPtr FirstView = m_LayoutHelper->viewAt(0);
			FirstView->setView(Position, Target, OdGeVector3d::kYAxis, m_ViewTransform.FieldWidth(), m_ViewTransform.FieldHeight());

			m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
			m_ViewTransform.BuildTransformMatrix();

			m_OverviewViewTransform = m_ViewTransform;
		}
	}
}

void AeSysView::OnDestroy() {
	AeSysDoc* Document = GetDocument();
	Document->OnCloseVectorizer(this);

	m_editor.Uninitialize();
	destroyDevice();

	m_pPrinterDevice.release();

	::ReleaseDC(m_hWnd, m_hWindowDC);
	m_hWindowDC = 0;
	CView::OnDestroy();
}
int AeSysView::OnCreate(LPCREATESTRUCT createStructure) {
	if (CView::OnCreate(createStructure) == -1) {
		return -1;
	}
	return 0;
}

OdGsView* AeSysView::getActiveView() {
	return m_LayoutHelper->activeView();
}

const OdGsView* AeSysView::getActiveView() const {
	return m_LayoutHelper->activeView();
}

OdGsView* AeSysView::getActiveTopView() {
	auto ActiveView {getActiveView()};

	if (!getDatabase()->getTILEMODE()) {
		auto ActiveViewport {getDatabase()->activeViewportId().safeOpenObject()};
		OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);

		if (!AbstractViewportData.isNull() && AbstractViewportData->gsView(ActiveViewport)) {
			ActiveView = AbstractViewportData->gsView(ActiveViewport);
		}
	}
	return (ActiveView);
}

const OdGsView* AeSysView::getActiveTopView() const {
	auto ActiveView {getActiveView()};

	if (!getDatabase()->getTILEMODE()) {
		auto ActiveViewport {getDatabase()->activeViewportId().safeOpenObject()};
		OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);

		if (!AbstractViewportData.isNull() && AbstractViewportData->gsView(ActiveViewport)) {
			ActiveView = AbstractViewportData->gsView(ActiveViewport);
		}
	}
	return (ActiveView);
}

inline bool requireAutoRegen(OdGsView* view) {
	OdGsDevice* Device = view->device();
	if (!Device) {
		return false;
	}
	OdRxDictionaryPtr DeviceProperties = Device->properties();
	if (!DeviceProperties.isNull()) {
		if (DeviceProperties->has(L"RegenCoef")) {
			return OdRxVariantValue(DeviceProperties->getAt(L"RegenCoef"))->getDouble() > 1.;
		}
	}
	return false;
}
void AeSysView::propagateActiveViewChanges(bool forceAutoRegen) const {
	// @@@ probably move this functionality into GsLayoutHelper's?
	OdGsViewPtr View {getActiveView()};
	OdGsClientViewInfo ClientViewInfo;
	View->clientViewInfo(ClientViewInfo);
	OdRxObjectPtr pObj = OdDbObjectId(ClientViewInfo.viewportObjectId).openObject(OdDb::kForWrite);
	OdAbstractViewPEPtr pVp(pObj);

	if (!pVp.isNull()) {
		const OdGePoint3d ptTarget(View->target());
		OdGeVector3d vecDir(View->position() - ptTarget);
		const OdGeVector3d vecUp(View->upVector());
		const auto dFieldWidth {View->fieldWidth()};
		const auto dFieldHeight {View->fieldHeight()};
		const auto bPersp {View->isPerspective()};
		const auto dLensLength {View->lensLength()};
		
		if (vecDir.isZeroLength()) {
			vecDir = View->viewingMatrix().inverse().getCsZAxis();
			if (vecDir.isZeroLength())
				vecDir = OdGeVector3d::kZAxis;
			else
				vecDir.normalize();
		}
		if (!pVp->target(pObj).isEqualTo(ptTarget) || !pVp->direction(pObj).isEqualTo(vecDir) || !pVp->upVector(pObj).isEqualTo(vecUp) || !OdEqual(pVp->fieldWidth(pObj), dFieldWidth) || !OdEqual(pVp->fieldHeight(pObj), dFieldHeight) || pVp->isPerspective(pObj) != bPersp || !OdEqual(pVp->lensLength(pObj), dLensLength)) {
			OdGeVector2d viewOffset;
			if (pVp->direction(pObj).isEqualTo(vecDir) && pVp->upVector(pObj).isEqualTo(vecUp) && !bPersp && !pVp->isPerspective(pObj)) {
				const OdGeVector3d vecX = vecUp.crossProduct(vecDir).normal();
				viewOffset = pVp->viewOffset(pObj);
				const OdGePoint3d prevTarg = pVp->target(pObj) - vecX * viewOffset.x - vecUp * viewOffset.y;
				viewOffset.x = vecX.dotProduct(ptTarget - prevTarg);
				viewOffset.y = vecUp.dotProduct(ptTarget - prevTarg);
			}
			pVp->setView(pObj, ptTarget, vecDir, vecUp, dFieldWidth, dFieldHeight, bPersp, viewOffset);
			pVp->setLensLength(pObj, dLensLength);
			// Auto regen
			if (!theApp.disableAutoRegen() && requireAutoRegen(View)) {
				const_cast<AeSysView*>(this)->OnViewerRegen();
			}
		}
		OdDb::RenderMode rm = OdDb::k2DOptimized;
		switch (View->mode()) {
			case OdGsView::kWireframe:
				rm = OdDb::kWireframe;
				break;
			case OdGsView::kHiddenLine:
				rm = OdDb::kHiddenLine;
				break;
			case OdGsView::kFlatShaded:
				rm = OdDb::kFlatShaded;
				break;
			case OdGsView::kGouraudShaded:
				rm = OdDb::kGouraudShaded;
				break;
			case OdGsView::kFlatShadedWithWireframe:
				rm = OdDb::kFlatShadedWithWireframe;
				break;
			case OdGsView::kGouraudShadedWithWireframe:
				rm = OdDb::kGouraudShadedWithWireframe;
				break;
		}
		if (pVp->renderMode(pObj) != rm) {
			pVp->setRenderMode(pObj, rm);
		}
		OdDbObjectId vs(View->visualStyle());

		if ((pVp->visualStyle(pObj) != vs) && !vs.isNull()) {
			pVp->setVisualStyle(pObj, View->visualStyle());
		}
	}
}

inline OdGsViewPtr overallView(OdGsDevice* device) {
	OdGsViewPtr OverallView;
	auto PaperLayoutHelper {OdGsPaperLayoutHelper::cast(device)};
	
	if (PaperLayoutHelper.get()) {
		OverallView = PaperLayoutHelper->overallView();
	}
	return OverallView;
}

inline OdGsViewPtr activeView(OdGsDevice * device) {
	OdGsViewPtr ActiveView;
	OdGsLayoutHelperPtr LayoutHelper = OdGsLayoutHelper::cast(device);
	if (LayoutHelper.get()) {
		ActiveView = LayoutHelper->activeView();
	}
	return ActiveView;
}

void AeSysView::setViewportBorderProperties() {
	auto OverallView {overallView(m_LayoutHelper)};
	auto ActiveView {activeView(m_LayoutHelper)};

	const auto NumberOfViews {m_LayoutHelper->numViews()};
	
	if (NumberOfViews > 1) {
		for (int ViewIndex = 0; ViewIndex < NumberOfViews; ++ViewIndex) {
			OdGsViewPtr View {m_LayoutHelper->viewAt(ViewIndex)};
			
			// If the model layout is active, and it has more then one viewport then make their borders visible.
			// If a paper layout is active, then make visible the borders of all but the overall viewport.

			if ((View == OverallView) || (OdGsPaperLayoutHelper::cast(m_LayoutHelper).get() && (View != ActiveView))) {
				View->setViewportBorderVisibility(false);
			} else if (View != ActiveView) {
				View->setViewportBorderVisibility(true);
				View->setViewportBorderProperties(theApp.curPalette()[7], 2);
			} else {
				View->setViewportBorderVisibility(true);
				View->setViewportBorderProperties(theApp.curPalette()[7], 2);
			}
		}
	}
}

OdGiContext::PStyleType AeSysView::plotStyleType() const {
	if (isPlotGeneration() ? !m_bPlotPlotstyle : !m_bShowPlotstyle) {
		return kPsNone;
	}
	return getDatabase()->getPSTYLEMODE() ? kPsByColor : kPsByName;
}
void AeSysView::plotStyle(OdDbStub * psNameId, OdPsPlotStyleData & plotStyleData) const {
	OdGiContextForDbDatabase::plotStyle(psNameId, plotStyleData);
	if (m_bPlotGrayscale) { // #4203 : make grayscale preview if printer doesn't support color mode
		plotStyleData.setColorPolicy(plotStyleData.colorPolicy() | 2);
	}
}
void AeSysView::preparePlotstyles(const OdDbLayout * pLayout, bool bForceReload) {
	if (m_pPlotStyleTable.get() && !bForceReload) {
		return;
	}
	const OdDbDatabase* Database = GetDocument()->m_DatabasePtr;
	OdDbLayoutPtr pCurrLayout;
	if (!pLayout) {
		OdDbBlockTableRecordPtr pLayoutBlock = Database->getActiveLayoutBTRId().safeOpenObject();
		pCurrLayout = pLayoutBlock->getLayoutId().safeOpenObject();
		pLayout = pCurrLayout;
	}
	m_bPlotPlotstyle = pLayout->plotPlotStyles();
	m_bShowPlotstyle = pLayout->showPlotStyles();

	if (isPlotGeneration() ? m_bPlotPlotstyle : m_bShowPlotstyle) {
		OdString pssFile(pLayout->getCurrentStyleSheet());
		if (!pssFile.isEmpty()) {
			OdString testpath = Database->appServices()->findFile(pssFile);
			if (!testpath.isEmpty()) {
				OdStreamBufPtr pFileBuf = odSystemServices()->createFile(testpath);

				if (pFileBuf.get()) {
					loadPlotStyleTable(pFileBuf);
				}
			}
		}
	}
}

OdString GetRegistryAcadProfilesKey(); // external defined in AeSysApp
static bool GetRegistryDWORD(HKEY key, LPCWSTR subkey, LPCWSTR name, DWORD & rval) noexcept {
	bool rv = false;
	HKEY hKey;
	if (RegOpenKeyExW(key, subkey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		DWORD dwSize = sizeof(DWORD);
		if (RegQueryValueExW(hKey, name, NULL, NULL, (LPBYTE) & rval, &dwSize) == ERROR_SUCCESS) {
			rv = true;
		}
		RegCloseKey(hKey);
	}
	return rv;
}
static bool GetAcadProfileRegistryDWORD(LPCWSTR pSubkey, LPCWSTR pName, DWORD & value) {
	OdString subkey = GetRegistryAcadProfilesKey();
	if (!subkey.isEmpty()) {
		if (pSubkey) {
			subkey += L"\\";
			subkey += pSubkey;
		}
		return GetRegistryDWORD(HKEY_CURRENT_USER, (LPCWSTR) subkey, pName, value);
	}
	return false;
}
OdUInt32 AeSysView::glyphSize(GlyphType glyphType) const {
	bool bProcessed = false;
	DWORD val(0);
	switch (glyphType) {
		case kLightGlyph:
			bProcessed = GetAcadProfileRegistryDWORD(L"Light", L"Glyph size", val);
			break;
		case kCameraGlyph:
			bProcessed = GetAcadProfileRegistryDWORD(L"Dialogs\\AcCamera", L"GlyphSize", val);
			break;
	}
	if (bProcessed)
		return (OdUInt32) val;
	return OdGiContextForDbDatabase::glyphSize(glyphType);
}
void AeSysView::fillContextualColors(OdGiContextualColorsImpl * pCtxColors) {
	OdGiContextForDbDatabase::fillContextualColors(pCtxColors); // Fill by defaults first
	DWORD val;
#define SET_CTXCLR_ISOK(entry, subkey, name) \
	if (GetAcadProfileRegistryDWORD(LPCWSTR(subkey), LPCWSTR(name), val)) \
	pCtxColors->setContextualColor(OdGiContextualColorsImpl::entry, (ODCOLORREF)val);
#define SET_CTXCLRTINT_ISOK(entry, subkey, name) \
	if (GetAcadProfileRegistryDWORD(LPCWSTR(subkey), LPCWSTR(name), val)) \
	pCtxColors->setContextualColorTint(OdGiContextualColorsImpl::entry, val != 0);

	switch (pCtxColors->visualType()) {
		case OdGiContextualColorsImpl::k2dModel:
			SET_CTXCLR_ISOK(kGridMajorLinesColor, L"Drawing Window", L"2D Model grid major lines color");
			SET_CTXCLR_ISOK(kGridMinorLinesColor, L"Drawing Window", L"2D Model grid minor lines color");
			SET_CTXCLR_ISOK(kGridAxisLinesColor, L"Drawing Window", L"2D Model grid axis lines color");
			SET_CTXCLRTINT_ISOK(kGridMajorLineTint, L"Drawing Window", L"2D Model grid major lines tint");
			SET_CTXCLRTINT_ISOK(kGridMinorLineTint, L"Drawing Window", L"2D Model grid minor lines tint");
			SET_CTXCLRTINT_ISOK(kGridAxisLineTint, L"Drawing Window", L"2D Model grid axis lines tint");
			SET_CTXCLR_ISOK(kLightGlyphsColor, L"Light", L"Model glyphs color");
			SET_CTXCLR_ISOK(kLightHotspotColor, L"Light", L"Model hotspot color");
			SET_CTXCLR_ISOK(kLightFalloffColor, L"Light", L"Model falloff color");
			SET_CTXCLR_ISOK(kLightStartLimitColor, L"Light", L"Model start color");
			SET_CTXCLR_ISOK(kLightEndLimitColor, L"Light", L"Model end color");
			SET_CTXCLR_ISOK(kLightShapeColor, L"Light", L"Model Web shape color");
			SET_CTXCLR_ISOK(kLightDistanceColor, L"Light", L"Model lux at distance color");
			SET_CTXCLR_ISOK(kWebMeshColor, L"Light", L"Model Web color");
			SET_CTXCLR_ISOK(kWebMeshMissingColor, L"Light", L"Model Web(missing file) color");
			SET_CTXCLR_ISOK(kCameraGlyphsColor, L"Camera", L"Model glyphs color");
			SET_CTXCLR_ISOK(kCameraFrustrumColor, L"Camera", L"Model frustrum color");
			SET_CTXCLR_ISOK(kCameraClippingColor, L"Camera", L"Model clipping color");
			break;
		case OdGiContextualColorsImpl::kLayout:
			SET_CTXCLR_ISOK(kGridMajorLinesColor, L"Drawing Window", L"Layout grid major lines color");
			SET_CTXCLR_ISOK(kGridMinorLinesColor, L"Drawing Window", L"Layout grid minor lines color");
			SET_CTXCLR_ISOK(kGridAxisLinesColor, L"Drawing Window", L"Layout grid axis lines color");
			SET_CTXCLRTINT_ISOK(kGridMajorLineTint, L"Drawing Window", L"Layout grid major lines tint");
			SET_CTXCLRTINT_ISOK(kGridMinorLineTint, L"Drawing Window", L"Layout grid minor lines tint");
			SET_CTXCLRTINT_ISOK(kGridAxisLineTint, L"Drawing Window", L"Layout grid axis lines tint");
			SET_CTXCLR_ISOK(kLightGlyphsColor, L"Light", L"Layout glyphs color");
			SET_CTXCLR_ISOK(kLightHotspotColor, L"Light", L"Layout hotspot color");
			SET_CTXCLR_ISOK(kLightFalloffColor, L"Light", L"Layout falloff color");
			SET_CTXCLR_ISOK(kLightStartLimitColor, L"Light", L"Layout start color");
			SET_CTXCLR_ISOK(kLightEndLimitColor, L"Light", L"Layout end color");
			SET_CTXCLR_ISOK(kLightShapeColor, L"Light", L"Layout Web shape color");
			SET_CTXCLR_ISOK(kLightDistanceColor, L"Light", L"Layout lux at distance color");
			SET_CTXCLR_ISOK(kWebMeshColor, L"Light", L"Layout Web color");
			SET_CTXCLR_ISOK(kWebMeshMissingColor, L"Light", L"Layout Web(missing file) color");
			SET_CTXCLR_ISOK(kCameraGlyphsColor, L"Camera", L"Layout glyphs color");
			SET_CTXCLR_ISOK(kCameraFrustrumColor, L"Camera", L"Layout frustrum color");
			SET_CTXCLR_ISOK(kCameraClippingColor, L"Camera", L"Layout clipping color");
			break;
		case OdGiContextualColorsImpl::k3dParallel:
			SET_CTXCLR_ISOK(kGridMajorLinesColor, L"Drawing Window", L"Parallel grid major lines color");
			SET_CTXCLR_ISOK(kGridMinorLinesColor, L"Drawing Window", L"Parallel grid minor lines color");
			SET_CTXCLR_ISOK(kGridAxisLinesColor, L"Drawing Window", L"Parallel grid axis lines color");
			SET_CTXCLRTINT_ISOK(kGridMajorLineTint, L"Drawing Window", L"Parallel grid major lines tint");
			SET_CTXCLRTINT_ISOK(kGridMinorLineTint, L"Drawing Window", L"Parallel grid minor lines tint");
			SET_CTXCLRTINT_ISOK(kGridAxisLineTint, L"Drawing Window", L"Parallel grid axis lines tint");
			SET_CTXCLR_ISOK(kLightGlyphsColor, L"Light", L"Parallel glyphs color");
			SET_CTXCLR_ISOK(kLightHotspotColor, L"Light", L"Parallel hotspot color");
			SET_CTXCLR_ISOK(kLightFalloffColor, L"Light", L"Parallel falloff color");
			SET_CTXCLR_ISOK(kLightStartLimitColor, L"Light", L"Parallel start color");
			SET_CTXCLR_ISOK(kLightEndLimitColor, L"Light", L"Parallel end color");
			SET_CTXCLR_ISOK(kLightShapeColor, L"Light", L"Parallel Web shape color");
			SET_CTXCLR_ISOK(kLightDistanceColor, L"Light", L"Parallel lux at distance color");
			SET_CTXCLR_ISOK(kWebMeshColor, L"Light", L"Parallel Web color");
			SET_CTXCLR_ISOK(kWebMeshMissingColor, L"Light", L"Parallel Web(missing file) color");
			SET_CTXCLR_ISOK(kCameraGlyphsColor, L"Camera", L"Parallel glyphs color");
			SET_CTXCLR_ISOK(kCameraFrustrumColor, L"Camera", L"Parallel frustrum color");
			SET_CTXCLR_ISOK(kCameraClippingColor, L"Camera", L"Parallel clipping color");
			break;
		case OdGiContextualColorsImpl::k3dPerspective:
			SET_CTXCLR_ISOK(kGridMajorLinesColor, L"Drawing Window", L"Perspective grid major lines color");
			SET_CTXCLR_ISOK(kGridMinorLinesColor, L"Drawing Window", L"Perspective grid minor lines color");
			SET_CTXCLR_ISOK(kGridAxisLinesColor, L"Drawing Window", L"Perspective grid axis lines color");
			SET_CTXCLRTINT_ISOK(kGridMajorLineTint, L"Drawing Window", L"Perspective grid major lines tint");
			SET_CTXCLRTINT_ISOK(kGridMinorLineTint, L"Drawing Window", L"Perspective grid minor lines tint");
			SET_CTXCLRTINT_ISOK(kGridAxisLineTint, L"Drawing Window", L"Perspective grid axis lines tint");
			SET_CTXCLR_ISOK(kLightGlyphsColor, L"Light", L"Perspective glyphs color");
			SET_CTXCLR_ISOK(kLightHotspotColor, L"Light", L"Perspective hotspot color");
			SET_CTXCLR_ISOK(kLightFalloffColor, L"Light", L"Perspective falloff color");
			SET_CTXCLR_ISOK(kLightStartLimitColor, L"Light", L"Perspective start color");
			SET_CTXCLR_ISOK(kLightEndLimitColor, L"Light", L"Perspective end color");
			SET_CTXCLR_ISOK(kLightShapeColor, L"Light", L"Perspective Web shape color");
			SET_CTXCLR_ISOK(kLightDistanceColor, L"Light", L"Perspective lux at distance color");
			SET_CTXCLR_ISOK(kWebMeshColor, L"Light", L"Perspective Web color");
			SET_CTXCLR_ISOK(kWebMeshMissingColor, L"Light", L"Perspective Web(missing file) color");
			SET_CTXCLR_ISOK(kCameraGlyphsColor, L"Camera", L"Perspective glyphs color");
			SET_CTXCLR_ISOK(kCameraFrustrumColor, L"Camera", L"Perspective frustrum color");
			SET_CTXCLR_ISOK(kCameraClippingColor, L"Camera", L"Perspective clipping color");
			break;
		case OdGiContextualColorsImpl::kBlock:
			SET_CTXCLR_ISOK(kGridMajorLinesColor, L"Drawing Window", L"BEdit grid major lines color");
			SET_CTXCLR_ISOK(kGridMinorLinesColor, L"Drawing Window", L"BEdit grid minor lines color");
			SET_CTXCLR_ISOK(kGridAxisLinesColor, L"Drawing Window", L"BEdit grid axis lines color");
			SET_CTXCLRTINT_ISOK(kGridMajorLineTint, L"Drawing Window", L"BEdit grid major lines tint");
			SET_CTXCLRTINT_ISOK(kGridMinorLineTint, L"Drawing Window", L"BEdit grid minor lines tint");
			SET_CTXCLRTINT_ISOK(kGridAxisLineTint, L"Drawing Window", L"BEdit grid axis lines tint");
			SET_CTXCLR_ISOK(kLightGlyphsColor, L"Light", L"BEdit glyphs color");
			SET_CTXCLR_ISOK(kLightHotspotColor, L"Light", L"BEdit hotspot color");
			SET_CTXCLR_ISOK(kLightFalloffColor, L"Light", L"BEdit falloff color");
			SET_CTXCLR_ISOK(kLightStartLimitColor, L"Light", L"BEdit start color");
			SET_CTXCLR_ISOK(kLightEndLimitColor, L"Light", L"BEdit end color");
			SET_CTXCLR_ISOK(kLightShapeColor, L"Light", L"BEdit Web shape color");
			SET_CTXCLR_ISOK(kLightDistanceColor, L"Light", L"BEdit lux at distance color");
			SET_CTXCLR_ISOK(kWebMeshColor, L"Light", L"BEdit Web color");
			SET_CTXCLR_ISOK(kWebMeshMissingColor, L"Light", L"BEdit Web(missing file) color");
			break;
		default: break;
	}
#undef SET_CTXCLRTINT_ISOK
#undef SET_CTXCLR_ISOK
}

/* <tas="MaterialsEditor required for additional GLES device settings">
	bool odExGLES2CompositeMfSetting(); // Defined in MaterialsEditor.cpp
	bool odExGLES2VisualStylesSetting(); // Defined in MaterialsEditor.cpp
	bool odExGLES2OverlaysSetting(); // Defined in MaterialsEditor.cpp
	bool odExGLES2OITSetting(); // Defined in MaterialsEditor.cpp
	bool odExGLES2SceneGraphSetting(); // Defined in MaterialsEditor.cpp
	bool odExGLES2ExtendedMaterialsSetting(); // Defined in MaterialsEditor.cpp
	OdIntPtr odExGLES2SceneGraphOptions(); // Defined in MaterialsEditor.cpp
	const OdString& odExLoadGsStateSetting(); // Defined in MaterialsEditor.cpp
   </tas> */

void AeSysView::createDevice(bool recreate) {
	CRect ClientRectangle;
	GetClientRect(&ClientRectangle);
	try {
		OdArray<OdGsViewPtr> m_prevViews;
		OdGsModelPtr m_pModel;

		if (!recreate) {
			OdGsModulePtr GsModule {::odrxDynamicLinker()->loadModule(theApp.recentGsDevicePath(), false)};
			auto GsDevice {GsModule->createDevice()};

			auto DeviceProperties {GsDevice->properties()};

			if (DeviceProperties.get()) {
				if (DeviceProperties->has(L"WindowHWND")) {
					DeviceProperties->putAt("WindowHWND", OdRxVariantValue((OdIntPtr)m_hWnd));
				}
				if (DeviceProperties->has(L"WindowHDC")) {
					DeviceProperties->putAt(L"WindowHDC", OdRxVariantValue((OdIntPtr)m_hWindowDC));
				}
				if (DeviceProperties->has(L"DoubleBufferEnabled")) {
					DeviceProperties->putAt(L"DoubleBufferEnabled", OdRxVariantValue(theApp.doubleBufferEnabled()));
				}
				if (DeviceProperties->has(L"EnableSoftwareHLR")) {
					DeviceProperties->putAt(L"EnableSoftwareHLR", OdRxVariantValue(theApp.useSoftwareHLR()));
				}
				if (DeviceProperties->has(L"DiscardBackFaces")) {
					DeviceProperties->putAt(L"DiscardBackFaces", OdRxVariantValue(theApp.discardBackFaces()));
				}
				if (DeviceProperties->has(L"BlocksCache")) {
					DeviceProperties->putAt(L"BlocksCache", OdRxVariantValue(theApp.blocksCacheEnabled()));
				}
				if (DeviceProperties->has(L"EnableMultithread")) {
					DeviceProperties->putAt(L"EnableMultithread", OdRxVariantValue(theApp.gsDeviceMultithreadEnabled()));
				}
				if (DeviceProperties->has(L"MaxRegenThreads")) {
					DeviceProperties->putAt(L"MaxRegenThreads", OdRxVariantValue((OdUInt16)theApp.mtRegenThreadsCount()));
				}
				if (DeviceProperties->has(L"UseTextOut")) {
					DeviceProperties->putAt(L"UseTextOut", OdRxVariantValue(theApp.enableTTFTextOut()));
				}
				if (DeviceProperties->has(L"UseTTFCache")) {
					DeviceProperties->putAt(L"UseTTFCache", OdRxVariantValue(theApp.enableTTFCache()));
				}
				if (DeviceProperties->has(L"DynamicSubEntHlt")) {
					DeviceProperties->putAt(L"DynamicSubEntHlt", OdRxVariantValue(theApp.enableDynamicSubEntHlt()));
				}
			/* <tas="MaterialsEditor required for additional GLES device settings">
				if (DeviceProperties->has(L"UseCompositeMetafiles")) {
					DeviceProperties->putAt(L"UseCompositeMetafiles", OdRxVariantValue(odExGLES2CompositeMfSetting()));
				}
				if (DeviceProperties->has(L"RenderSettingsDlg") && odExGLES2CompositeMfSetting()) {
					DeviceProperties->putAt(L"RenderSettingsDlg", OdRxVariantValue((OdIntPtr)AfxGetMainWnd()->GetSafeHwnd()));
				}
				if (DeviceProperties->has(L"UseVisualStyles")) {
					DeviceProperties->putAt(L"UseVisualStyles", OdRxVariantValue(odExGLES2VisualStylesSetting()));
				}
				if (DeviceProperties->has(L"UseOverlays")) {
					DeviceProperties->putAt(L"UseOverlays", OdRxVariantValue(odExGLES2OverlaysSetting()));
				}
				if (DeviceProperties->has(L"UseSceneGraph")) {
					DeviceProperties->putAt(L"UseSceneGraph", OdRxVariantValue(odExGLES2SceneGraphSetting()));
				}
				if (DeviceProperties->has(L"SceneGraphOptions")) {
					DeviceProperties->putAt(L"SceneGraphOptions", OdRxVariantValue(odExGLES2SceneGraphOptions()));
				}
				if (DeviceProperties->has(L"UseExtendedMaterials")) {
					DeviceProperties->putAt(L"UseExtendedMaterials", OdRxVariantValue(odExGLES2ExtendedMaterialsSetting()));
				}
				if (!odExGLES2OITSetting() && DeviceProperties->has(L"BlendingMode")) { // Disable Order Independent Transparency if this is required
					DeviceProperties->putAt(L"BlendingMode", OdRxVariantValue(OdUInt32(0)));
				}
				if (DeviceProperties->has(L"GradientsAsBitmap")) {
					DeviceProperties->putAt(L"GradientsAsBitmap", OdRxVariantValue(theApp.enableGDIGradientsAsBitmap()));
				}
			   </tas> */
			}
			enableKeepPSLayoutHelperView(true);
			enableContextualColorsManagement(theApp.enableContextualColors());
			setTtfPolyDrawMode(theApp.enableTTFPolyDraw());
			enableGsModel(theApp.useGsModel());

			m_LayoutHelper = OdDbGsManager::setupActiveLayoutViews(GsDevice, this);

		/* <tas="MaterialsEditor required for additional GLES device settings">
			if (!odExLoadGsStateSetting().isEmpty()) {
				if (m_LayoutHelper->supportLayoutGsStateSaving()) {
					auto File {::odrxSystemServices()->createFile(odExLoadGsStateSetting(), Oda::kFileRead, Oda::kShareDenyWrite, Oda::kOpenExisting)};

					if (!m_LayoutHelper->restoreLayoutGsState(File)) {
						MessageBox(L"Failed to load Gs State.", L"Gs Error", MB_ICONERROR);
					}
				}
			}
		   </tas> */
		} else { // Store current device views to keep cache alive, detach views from exist device, create new helper for exist device, and release existing helper device
			auto LayoutHelperIn {m_LayoutHelper};
			for (auto ViewIndex = 0; ViewIndex < LayoutHelperIn->numViews(); ViewIndex++) {
				m_prevViews.append(LayoutHelperIn->viewAt(ViewIndex));
			}
			m_pModel = LayoutHelperIn->gsModel();
			LayoutHelperIn->eraseAllViews();

			auto LayoutHelperOut {OdDbGsManager::setupActiveLayoutViews(LayoutHelperIn->underlyingDevice(), this)};

			m_LayoutHelper = LayoutHelperOut;
			LayoutHelperIn.release();
			m_editor.Initialize(m_LayoutHelper, static_cast<AeSysDoc*>(GetDocument())->CommandContext());
		}
		m_layoutId = m_LayoutHelper->layoutId();

		const ODCOLORREF* Palette = theApp.curPalette();
		ODGSPALETTE PaletteCopy;
		PaletteCopy.insert(PaletteCopy.begin(), Palette, Palette + 256);
		PaletteCopy[0] = theApp.activeBackground();
		m_LayoutHelper->setLogicalPalette(PaletteCopy.asArrayPtr(), 256);
		auto PaperLayoutHelper {OdGsPaperLayoutHelper::cast(m_LayoutHelper)};

		if (PaperLayoutHelper.isNull()) {
			m_bPsOverall = false;
			m_LayoutHelper->setBackgroundColor(PaletteCopy[0]); // for model space
		} else {
			m_bPsOverall = (PaperLayoutHelper->overallView().get() == PaperLayoutHelper->activeView().get());
			m_LayoutHelper->setBackgroundColor(ODRGB(173, 174, 173)); // ACAD's color for paper bg
		}
		setPaletteBackground(theApp.activeBackground());

		setViewportBorderProperties();

		if (ClientRectangle.Width() && ClientRectangle.Height()) {
			m_LayoutHelper->onSize(OdGsDCRect(ClientRectangle.left, ClientRectangle.right, ClientRectangle.bottom, ClientRectangle.top));

			OdGsViewPtr FirstView {m_LayoutHelper->viewAt(0)};

			SetViewportSize(ClientRectangle.Width(), ClientRectangle.Height());

			m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
			m_ViewTransform.BuildTransformMatrix();

			m_OverviewViewTransform = m_ViewTransform;
		}
		preparePlotstyles(NULL, recreate);

		if (recreate) {
			// Call update to share cache from exist views
			m_LayoutHelper->update();
			// Invalidate views for exist Gs model (i. e. remove unused drawables and mark view props as invalid)
			if (!m_pModel.isNull()) {
				auto Views {m_prevViews.asArrayPtr()};
				OdUInt32 nViews = m_prevViews.size();

				for (OdUInt32 nView = 0; nView < nViews; nView++) {
					m_pModel->invalidate(Views[nView]);
				}
			}
			// Release exist views to detach from Gs and keep released slots free.
			m_prevViews.clear();
		}
	}
	catch (const OdError & Error) {
		destroyDevice();
		theApp.reportError(L"Graphic System Initialization Error", Error);
	}
}

void AeSysView::destroyDevice() {
	m_LayoutHelper.release();
}

void AeSysView::OnBeginPrinting(CDC * deviceContext, CPrintInfo * printInformation) {
	ViewportPushActive();
	PushViewTransform();

	const int HorizontalPixelWidth = deviceContext->GetDeviceCaps(HORZRES);
	const int VerticalPixelWidth = deviceContext->GetDeviceCaps(VERTRES);

	SetViewportSize(HorizontalPixelWidth, VerticalPixelWidth);

	const double HorizontalSize = static_cast<double>(deviceContext->GetDeviceCaps(HORZSIZE));
	const double VerticalSize = static_cast<double>(deviceContext->GetDeviceCaps(VERTSIZE));

	SetDeviceWidthInInches(HorizontalSize / EoMmPerInch);
	SetDeviceHeightInInches(VerticalSize / EoMmPerInch);

	if (m_Plot) {
		UINT HorizontalPages;
		UINT VerticalPages;
		printInformation->SetMaxPage(NumPages(deviceContext, m_PlotScaleFactor, HorizontalPages, VerticalPages));
	} else {
		m_ViewTransform.AdjustWindow(static_cast<double>(VerticalPixelWidth) / static_cast<double>(HorizontalPixelWidth));
		m_ViewTransform.BuildTransformMatrix();
	}
}

#include "../ExRender/BmpTilesGen.h"

void generateTiles(HDC hdc, RECT & drawrc, OdGsDevice * pBmpDevice, OdUInt32 nTileWidth, OdUInt32 nTileHeight) {
	CRect destrc = drawrc;
	destrc.NormalizeRect();
	OdGsDCRect step(0, 0, 0, 0);
	OdGsDCRect rc(drawrc.left, drawrc.right, drawrc.bottom, drawrc.top);
	const OdUInt32 nWidth = abs(rc.m_max.x - rc.m_min.x);
	rc.m_max.x -= rc.m_min.x;
	if (rc.m_max.x < 0) {
		rc.m_min.x = -rc.m_max.x;
		rc.m_max.x = 0;
		step.m_min.x = nTileWidth;
	} else {
		rc.m_min.x = 0;
		step.m_max.x = nTileWidth;
	}
	const OdUInt32 nHeight = abs(rc.m_max.y - rc.m_min.y);
	rc.m_max.y -= rc.m_min.y;
	if (rc.m_max.y < 0) {
		rc.m_min.y = -rc.m_max.y;
		rc.m_max.y = 0;
		step.m_min.y = nTileHeight;
	} else {
		rc.m_min.y = 0;
		step.m_max.y = nTileHeight;
	}
	const OdInt32 m = nWidth / nTileWidth + (nWidth % nTileWidth ? 1 : 0);
	const OdInt32 n = nHeight / nTileHeight + (nHeight % nTileHeight ? 1 : 0);

	BmpTilesGen tilesGen(pBmpDevice, rc);
	pBmpDevice->onSize(rc);

	OdGiRasterImagePtr pImg;

	const int dx = (step.m_max.x - step.m_min.x);
	const int dy = (step.m_max.y - step.m_min.y);

	const int dx2 = m > 1 ? dx / abs(dx) * 8 : 0;
	const int dy2 = n > 1 ? dy / abs(dy) * 8 : 0;

	BITMAPINFO bmi;
	bmi.bmiHeader.biBitCount = (WORD) 24;
	bmi.bmiHeader.biWidth = nTileWidth + abs(dx2) * 2;
	bmi.bmiHeader.biHeight = nTileHeight;
	bmi.bmiHeader.biClrImportant = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biSizeImage = 0;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;

	HDC bmpDC = CreateCompatibleDC(hdc);
	if (bmpDC) {
		void* pBuf;
		HBITMAP hBmp = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &pBuf, NULL, 0);
		if (hBmp) {
			HBITMAP hOld = (HBITMAP) SelectObject(bmpDC, hBmp);
			for (OdInt32 i = 0; i < m; ++i) {
				for (OdInt32 j = 0; j < n; ++j) {
					const int minx = rc.m_min.x + i * dx;
					const int maxx = minx + dx;
					const int miny = rc.m_min.y + j * dy;
					const int maxy = miny + dy;

					// render wider then a tile area to reduce gaps in lines.
					pImg = tilesGen.regenTile(OdGsDCRect(minx - dx2, maxx + dx2, miny - dy2, maxy + dy2));

					pImg->scanLines((OdUInt8*) pBuf, 0, nTileHeight);
					BitBlt(hdc, destrc.left + odmin(minx, maxx), destrc.top + odmin(miny, maxy), nTileWidth, nTileHeight, bmpDC, abs(dx2), 0, SRCCOPY);
				}
			}
			SelectObject(bmpDC, hOld);
			DeleteObject(hBmp);
		}
		DeleteDC(bmpDC);
	}
}

void AeSysView::OnPrint(CDC* deviceContext, CPrintInfo* printInformation) {
	const auto Database {getDatabase()};

	auto ActiveViewport {Database->activeViewportId().safeOpenObject(OdDb::kForWrite)};
	OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);
	const auto View {getActiveView()};

	if (View) {
		AbstractViewportData->setView(ActiveViewport, View);
	}
	setPlotGeneration(true);
	struct KeepPrevGiContextParams {
		OdGiContextForDbDatabase* m_pGiCtx;
		bool m_bPrevGsModelState;
		ODCOLORREF m_crPrevBkgndColor;

		KeepPrevGiContextParams(OdGiContextForDbDatabase* pGiCtx) :
			m_pGiCtx(pGiCtx) {
			m_bPrevGsModelState = pGiCtx->useGsModel();
			m_crPrevBkgndColor = pGiCtx->paletteBackground();
		}
		~KeepPrevGiContextParams() {
			m_pGiCtx->enableGsModel(m_bPrevGsModelState);
			m_pGiCtx->setPaletteBackground(m_crPrevBkgndColor);
		}
	} prevGiContextParams(this);

	// To get paper size of selected printer and to get properties (scale, offset) from PlotSettings to set using them OverAll View;
	//  Note: if we want to get the same Plot View for Paper Layout as AutoCAD then pIter needs to create pseudo DC having the requisite settings & properties.
	//        Look at OnPreparePrinting() where we try to set required printer device.
	//        Otherwise CPreviewView uses settings and properties of current Printer/plotter (see CPreviewView::OnDraw()) to draw empty page on screen.

	bool IsPlotViaBitmap = AfxGetApp()->GetProfileIntW(L"options", L"Print/Preview via bitmap device", 1) != 0;

	if (m_pPrinterDevice.isNull()) {
		OdGsModulePtr GsModule = ::odrxDynamicLinker()->loadModule(theApp.recentGsDevicePath());
		if (!IsPlotViaBitmap && GsModule.isNull()) {
			GsModule = ::odrxDynamicLinker()->loadModule(OdWinOpenGLModuleName);
		}
		OdGsDevicePtr GsPrinterDevice;
		if (IsPlotViaBitmap && GsModule.get()) {
			GsPrinterDevice = GsModule->createBitmapDevice();
		} else {
			IsPlotViaBitmap = false;
			GsModule = ::odrxDynamicLinker()->loadModule(OdWinGDIModuleName);
			if (GsModule.get()) {
				GsPrinterDevice = GsModule->createDevice();
			}
		}
		if (GsPrinterDevice.get()) {
			enableGsModel(true);
			if (!GsPrinterDevice->properties().isNull() && GsPrinterDevice->properties()->has(L"EnableSoftwareHLR")) {
				GsPrinterDevice->properties()->putAt(L"EnableSoftwareHLR", OdRxVariantValue(theApp.useSoftwareHLR()));
			}
			if (/*IsPlotViaBitmap &&*/ GsPrinterDevice->properties()->has(L"DPI")) { // #9633 (1)
				const int MinimumLogicalPixels = odmin(deviceContext->GetDeviceCaps(LOGPIXELSX), deviceContext->GetDeviceCaps(LOGPIXELSY));
				GsPrinterDevice->properties()->putAt(L"DPI", OdRxVariantValue((OdUInt32) MinimumLogicalPixels));
			}
			m_pPrinterDevice = OdDbGsManager::setupActiveLayoutViews(GsPrinterDevice, this);
			preparePlotstyles();

			m_pPrinterDevice->setLogicalPalette(odcmAcadLightPalette(), 256);
			m_pPrinterDevice->setBackgroundColor(ODRGB(255, 255, 255));
			setPaletteBackground(m_pPrinterDevice->getBackgroundColor());
		}
	} else {
		IsPlotViaBitmap = m_pPrinterDevice->properties()->has(L"RasterImage");
		setPaletteBackground(m_pPrinterDevice->getBackgroundColor());
	}
	if (m_pPrinterDevice.get()) {
		bool IsPrint90Degrees(false);
		bool IsPrint0Degrees(false);
		bool IsPrint180Degrees(false);
		bool IsPrint270Degrees(false);

		double PrinterWidth = deviceContext->GetDeviceCaps(PHYSICALWIDTH);
		if (printInformation->m_bPreview) {
			PrinterWidth -= 2;
		}
		const double PrinterHeight = deviceContext->GetDeviceCaps(PHYSICALHEIGHT);
		const double PrinterLeftMargin = deviceContext->GetDeviceCaps(PHYSICALOFFSETX);
		const double PrinterTopMargin = deviceContext->GetDeviceCaps(PHYSICALOFFSETY);
		const double PrinterMarginWidth = deviceContext->GetDeviceCaps(HORZRES);
		const double PrinterMarginHeight = deviceContext->GetDeviceCaps(VERTRES);
		const double LogicalPixelsX = deviceContext->GetDeviceCaps(LOGPIXELSX);
		const double LogicalPixelsY = deviceContext->GetDeviceCaps(LOGPIXELSY);
		// const double PrinterRightMargin = PrinterWidth - PrinterMarginWidth - PrinterLeftMargin;
		const double PrinterBottomMargin = PrinterHeight - PrinterMarginHeight - PrinterTopMargin;
		const double koeffX = LogicalPixelsX / kMmPerInch;
		const double koeffY = LogicalPixelsY / kMmPerInch;

		const bool IsModelLayout = m_pPrinterDevice->isKindOf(OdGsModelLayoutHelper::desc());

		// Get Layout info
		OdDbLayoutPtr Layout = m_pPrinterDevice->layoutId().safeOpenObject();

		bool IsScaledToFit = Layout->useStandardScale() && (OdDbPlotSettings::kScaleToFit == Layout->stdScaleType());
		bool IsCentered = Layout->plotCentered();
		const bool IsMetric = (Layout->plotPaperUnits() != OdDbPlotSettings::kInches) ? true : false;
		const bool IsPrintLineweights = Layout->printLineweights() || Layout->showPlotStyles();

		double offsetX;
		double offsetY;
		Layout->getPlotOrigin(offsetX, offsetY); // in mm
		OdGePoint2d PaperImageOrigin = Layout->getPaperImageOrigin(); // in mm

		double LeftMargin = Layout->getLeftMargin(); // in mm
		double RightMargin = Layout->getRightMargin(); // in mm
		double TopMargin = Layout->getTopMargin(); // in mm
		double BottomMargin = Layout->getBottomMargin(); // in mm

		const OdDbPlotSettings::PlotType plotType = Layout->plotType();

		// set LineWeight scale factor for model space
		if (IsPrintLineweights && IsModelLayout) {
			OdGsView* pTo = m_pPrinterDevice->viewAt(0);
			pTo->setLineweightToDcScale(odmax(LogicalPixelsX, LogicalPixelsY) / kMmPerInch * 0.01);
		}
		// Apply paper rotation to paper parameters
		if (printInformation->m_bPreview) {
			double dTmp;
			switch (Layout->plotRotation()) {
				case OdDbPlotSettings::k90degrees:
					dTmp = TopMargin;
					TopMargin = RightMargin;
					RightMargin = BottomMargin;
					BottomMargin = LeftMargin;
					LeftMargin = dTmp;
					//std::swap(PaperImageOrigin.x, PaperImageOrigin.y);
					std::swap(offsetX, offsetY);
					break;
				case OdDbPlotSettings::k180degrees:
					std::swap(BottomMargin, TopMargin);
					std::swap(RightMargin, LeftMargin);
					break;
				case OdDbPlotSettings::k270degrees:
					dTmp = TopMargin;
					TopMargin = LeftMargin;
					LeftMargin = BottomMargin;
					BottomMargin = RightMargin;
					RightMargin = dTmp;
					//std::swap(PaperImageOrigin.x, PaperImageOrigin.y);
					std::swap(offsetX, offsetY);
					break;
			};
		} else { // printing
			OdDbPlotSettings::PlotRotation plotRotation = Layout->plotRotation();

			if (plotRotation == OdDbPlotSettings::k90degrees || plotRotation == OdDbPlotSettings::k270degrees) {
				// 3962 fixed
				DWORD landOrientation = DeviceCapabilities(printInformation->m_pPD->GetDeviceName(), printInformation->m_pPD->GetPortName(), DC_ORIENTATION, 0, 0);
				if (landOrientation == 270) {
					plotRotation = (plotRotation == OdDbPlotSettings::k90degrees) ? OdDbPlotSettings::k270degrees : OdDbPlotSettings::k90degrees;
				}
			}
			switch (plotRotation) {
				case OdDbPlotSettings::k0degrees:
					IsPrint0Degrees = true;
					break;
				case OdDbPlotSettings::k90degrees:
					IsPrint90Degrees = true;
					std::swap(TopMargin, RightMargin);
					std::swap(BottomMargin, LeftMargin);
					std::swap(BottomMargin, TopMargin);
					//std::swap(PaperImageOrigin.x, PaperImageOrigin.y);
					std::swap(offsetX, offsetY);
					offsetY = -offsetY;
					offsetX = -offsetX;
					break;
				case OdDbPlotSettings::k180degrees:
					IsPrint180Degrees = true;
					std::swap(RightMargin, LeftMargin);
					offsetY = -offsetY;
					offsetX = -offsetX;
					break;
				case OdDbPlotSettings::k270degrees:
					IsPrint270Degrees = true;
					std::swap(TopMargin, RightMargin);
					std::swap(BottomMargin, LeftMargin);
					std::swap(offsetX, offsetY);
					break;
			}
		}
		// Get scale factor
		double factor;
		if (Layout->useStandardScale()) {
			Layout->getStdScale(factor);
		} else {
			double numerator;
			double denominator;
			Layout->getCustomPrintScale(numerator, denominator);
			factor = numerator / denominator;
		}
		// if (LeftMargin < PrinterLeftMargin / koeffX)
		// LeftMargin = PrinterLeftMargin / koeffX;
		// if (TopMargin < PrinterTopMargin  / koeffY)
		// TopMargin = PrinterTopMargin  / koeffY;

		// Calculate paper drawable area using margins from layout (in mm).
		double drx1 = (-PrinterLeftMargin / koeffX + LeftMargin); // in mm
		double drx2 = (drx1 + PrinterWidth / koeffX - LeftMargin - RightMargin); // in mm
		double dry1 = (-PrinterTopMargin / koeffY + TopMargin); // in mm
		double dry2 = (dry1 + PrinterHeight / koeffY - TopMargin - BottomMargin); // in mm

		// Margin clip box calculation
		TopMargin *= koeffY; /// in printer units
		RightMargin *= koeffX;
		BottomMargin *= koeffY;
		LeftMargin *= koeffX;

		CRgn newClipRgn;
		newClipRgn.CreateRectRgn(0, 0, 1, 1);
		CRect MarginsClipBox;

		const int ret = GetClipRgn(deviceContext->m_hDC, newClipRgn);
		const bool bNullMarginsClipBox = !ret || ret && (GetLastError() != ERROR_SUCCESS);

		double dScreenFactorH;
		double dScreenFactorW;
		if (bNullMarginsClipBox) { // printing way
			const double x = LeftMargin - PrinterLeftMargin;
			const double y = TopMargin - PrinterTopMargin;
			MarginsClipBox.SetRect(int(x), int(y), int(x + PrinterWidth - LeftMargin - RightMargin), int(y + PrinterHeight - TopMargin - BottomMargin));

			dScreenFactorH = dScreenFactorW = 1.;
		} else {
			newClipRgn.GetRgnBox(&MarginsClipBox);
			dScreenFactorH = double(MarginsClipBox.Height()) / PrinterMarginHeight;
			dScreenFactorW = double(MarginsClipBox.Width()) / PrinterMarginWidth;

			MarginsClipBox.left += long((LeftMargin - PrinterLeftMargin) * dScreenFactorW);
			MarginsClipBox.bottom -= long((BottomMargin - PrinterBottomMargin) * dScreenFactorH);

			MarginsClipBox.top = MarginsClipBox.bottom - long((PrinterHeight - TopMargin - BottomMargin) * dScreenFactorH);
			MarginsClipBox.right = MarginsClipBox.left + long((PrinterWidth - LeftMargin - RightMargin) * dScreenFactorW);
		}
		// MarginsClipBox is calculated
		CRect ClipBox(MarginsClipBox);

		OdGePoint3d ViewTarget;

		OdAbstractViewPEPtr pAbstractViewPE;
		OdRxObjectPtr pVObject;

		OdGsViewPtr pOverallView = IsModelLayout ? OdGsModelLayoutHelperPtr(m_pPrinterDevice)->activeView() : OdGsPaperLayoutHelperPtr(m_pPrinterDevice)->overallView();

		if (plotType == OdDbPlotSettings::kView) {
			OdString sPlotViewName = Layout->getPlotViewName();
			OdDbViewTableRecordPtr pVtr = ((OdDbViewTablePtr) (Database->getViewTableId().safeOpenObject()))->getAt(sPlotViewName).safeOpenObject();

			ViewTarget = pVtr->target(); // in plotPaperUnits
			pAbstractViewPE = OdAbstractViewPEPtr(pVObject = pVtr);
		} else if (IsModelLayout) {
			OdDbViewportTablePtr pVPT = Database->getViewportTableId().safeOpenObject();
			OdDbViewportTableRecordPtr ActiveViewport = pVPT->getActiveViewportId().safeOpenObject();

			ViewTarget = ActiveViewport->target(); // in plotPaperUnits
			pAbstractViewPE = OdAbstractViewPEPtr(pVObject = ActiveViewport);
		} else {
			const OdDbObjectId overallVpId = Layout->overallVportId();
			OdDbViewportPtr pActiveVP = overallVpId.safeOpenObject();

			ViewTarget = pActiveVP->viewTarget(); // in plotPaperUnits
			pAbstractViewPE = OdAbstractViewPEPtr(pVObject = pActiveVP);
		}
		const OdGePoint3d ViewportCenter = pAbstractViewPE->target(pVObject); // in plotPaperUnits
		const bool IsPerspective = pAbstractViewPE->isPerspective(pVObject);
		const double ViewportHeight = pAbstractViewPE->fieldHeight(pVObject); // in plotPaperUnits
		const double ViewportWidth = pAbstractViewPE->fieldWidth(pVObject); // in plotPaperUnits
		const OdGeVector3d ViewDirection = pAbstractViewPE->direction(pVObject);
		const OdGeVector3d ViewUpVector = pAbstractViewPE->upVector(pVObject);
		const OdGeMatrix3d EyeToWorld = pAbstractViewPE->eyeToWorld(pVObject);
		const OdGeMatrix3d WorldToeye = pAbstractViewPE->worldToEye(pVObject);
		bool SkipClipping = false;

		const bool IsPlanView = /*ViewTarget.isEqualTo(OdGePoint3d(0, 0, 0)) &&*/ ViewDirection.normal().isEqualTo(OdGeVector3d::kZAxis);

		const OdGePoint3d OldTarget = ViewTarget;

		double FieldWidth(ViewportWidth);
		double FieldHeight(ViewportHeight);
		if (plotType == OdDbPlotSettings::kWindow || (plotType == OdDbPlotSettings::kLimits && IsPlanView)) {
			double xmin;
			double ymin;
			double xmax;
			double ymax;
			if (plotType == OdDbPlotSettings::kWindow) {
				Layout->getPlotWindowArea(xmin, ymin, xmax, ymax); // in plotPaperUnits
			} else {
				xmin = Database->getLIMMIN().x;
				ymin = Database->getLIMMIN().y;
				xmax = Database->getLIMMAX().x;
				ymax = Database->getLIMMAX().y;
			}
			FieldWidth = xmax - xmin;
			FieldHeight = ymax - ymin;

			const OdGeVector3d tmp = ViewportCenter - ViewTarget;
			ViewTarget.set((xmin + xmax) / 2., (ymin + ymax) / 2., 0);
			ViewTarget.transformBy(EyeToWorld);
			ViewTarget -= tmp;
		} else if (plotType == OdDbPlotSettings::kDisplay) {
			ViewTarget = ViewportCenter;
			FieldWidth = ViewportWidth;
			FieldHeight = ViewportHeight;
		} else if (plotType == OdDbPlotSettings::kExtents || (plotType == OdDbPlotSettings::kLimits && !IsPlanView)) {
			OdGeBoundBlock3d extents;
			if (pAbstractViewPE->plotExtents(pVObject, extents)) { // pIter also skip 'off layers'
				extents.transformBy(EyeToWorld);
				ViewTarget = (extents.minPoint() + extents.maxPoint().asVector()) / 2.;
				extents.transformBy(WorldToeye);

				FieldWidth = fabs(extents.maxPoint().x - extents.minPoint().x);
				FieldHeight = fabs(extents.maxPoint().y - extents.minPoint().y);
			}
		} else if (plotType == OdDbPlotSettings::kView) {
			ViewTarget = ViewportCenter;
			FieldWidth = ViewportWidth;
			FieldHeight = ViewportHeight;
		} else if (plotType == OdDbPlotSettings::kLayout) {
			SkipClipping = true; // used full paper drawing area.

			FieldWidth = (drx2 - drx1) / factor; // drx in mm -> fieldWidth in mm
			FieldHeight = (dry2 - dry1) / factor;

			ViewTarget.set(FieldWidth / 2. - PaperImageOrigin.x - offsetX / factor, FieldHeight / 2. - PaperImageOrigin.y - offsetY / factor, 0); // in mm
			if (!IsMetric) {
				ViewTarget /= kMmPerInch; // must be in plotpaper units
				FieldWidth /= kMmPerInch;
				FieldHeight /= kMmPerInch;
			}
			offsetX = 0.; // pIter was applied to viewTarget, reset pIter.
			offsetY = 0.;
			PaperImageOrigin.x = 0.;
			PaperImageOrigin.y = 0.;
			IsCentered = false;
			IsScaledToFit = false; // kLayout doesn't support pIter.
		}
		if (plotType != OdDbPlotSettings::kView) {// AlexR : 3952
			ViewTarget = ViewTarget.orthoProject(OdGePlane(OldTarget, ViewDirection));
		}
		// in plotpaper units
		pOverallView->setView(ViewTarget + ViewDirection, ViewTarget, ViewUpVector, FieldWidth, FieldHeight, IsPerspective ? OdGsView::kPerspective : OdGsView::kParallel);

		if (!IsMetric) {
			FieldWidth *= kMmPerInch;
			FieldHeight *= kMmPerInch;
		}
		if (IsScaledToFit) { // Scale factor can be stored in layout, but preview recalculate pIter if bScaledToFit = true.
			// Some times stored factor isn't correct, due to autocad isn't update pIter before saving.
			factor = odmin((drx2 - drx1) / FieldWidth, (dry2 - dry1) / FieldHeight);
		}
		if (IsCentered) { // Offset also can be incorectly saved.
			offsetX = ((drx2 - drx1) - FieldWidth * factor) / 2.;
			offsetY = ((dry2 - dry1) - FieldHeight * factor) / 2.;

			if (IsPrint90Degrees || IsPrint180Degrees) {
				offsetY = -offsetY;
				offsetX = -offsetX;
			}
		}
		if (IsPrint180Degrees || IsPrint90Degrees) {
			drx1 = drx2 - FieldWidth * factor;
			dry2 = dry1 + FieldHeight * factor;
		} else if (IsPrint0Degrees || IsPrint270Degrees) {
			drx2 = drx1 + FieldWidth * factor;
			dry1 = dry2 - FieldHeight * factor;
		} else { // preview
			drx2 = drx1 + FieldWidth * factor;
			dry1 = dry2 - FieldHeight * factor;
		}
		if (!SkipClipping) {
			if (IsPrint180Degrees || IsPrint90Degrees) {
				ClipBox.left = long(ClipBox.right - (drx2 - drx1) * koeffX * dScreenFactorW);
				ClipBox.bottom = long(ClipBox.top + (dry2 - dry1) * koeffY * dScreenFactorH);
			} else if (IsPrint0Degrees || IsPrint270Degrees) {
				ClipBox.right = long(ClipBox.left + (drx2 - drx1) * koeffX * dScreenFactorW);
				ClipBox.top = long(ClipBox.bottom - (dry2 - dry1) * koeffY * dScreenFactorH);
			} else { // preview
				ClipBox.right = long(ClipBox.left + (drx2 - drx1) * koeffX * dScreenFactorW);
				ClipBox.top = long(ClipBox.bottom - (dry2 - dry1) * koeffY * dScreenFactorH);
			}
			ClipBox.OffsetRect(int(offsetX * koeffX * dScreenFactorW), int(-offsetY * koeffY * dScreenFactorH));
		}
		pOverallView->setViewport(OdGePoint2d(0, 0), OdGePoint2d(1, 1));

		CRect ResultClipBox;
		ResultClipBox.IntersectRect(&MarginsClipBox, &ClipBox);

		// Apply clip region to screen
		CRgn newClip;
		newClip.CreateRectRgnIndirect(&ResultClipBox);
		deviceContext->SelectClipRgn(&newClip);

		// Calculate viewport rect in printer units
		const long x1 = long((offsetX + drx1) * koeffX);
		const long x2 = long((offsetX + drx2) * koeffX);
		const long y1 = long((-offsetY + dry1) * koeffY);
		const long y2 = long((-offsetY + dry2) * koeffY);

		OdGsDCRect ViewportRectangle;
		
		if (IsPrint180Degrees || IsPrint90Degrees) {
			ViewportRectangle = OdGsDCRect(x2, x1, y1, y2);
		} else if (IsPrint0Degrees || IsPrint270Degrees) {
			ViewportRectangle = OdGsDCRect(x1, x2, y2, y1);
		} else { // preview
			ViewportRectangle = OdGsDCRect(x1, x2, y2, y1);
		}
		if (!IsPlotViaBitmap) {
			m_pPrinterDevice->onSize(ViewportRectangle);
			m_pPrinterDevice->properties()->putAt(L"WindowHDC", OdRxVariantValue((OdIntPtr) deviceContext->m_hDC));
			m_pPrinterDevice->update(0);
		} else {
			CRect rc(ViewportRectangle.m_min.x, ViewportRectangle.m_max.y, ViewportRectangle.m_max.x, ViewportRectangle.m_min.y);
			generateTiles(deviceContext->m_hDC, rc, m_pPrinterDevice, 1000, 1000);
		}
	} else {
		AfxMessageBox(L"Can't initialize GS for printing...");
	}
	setPlotGeneration(false);
	CView::OnPrint(deviceContext, printInformation);
}

void AeSysView::OnEndPrinting(CDC * deviceContext, CPrintInfo * printInformation) {
	PopViewTransform();
	ViewportPopActive();
}
BOOL AeSysView::OnPreparePrinting(CPrintInfo * printInformation) {
	if (m_Plot) {
		CPrintInfo pi;
		
		if (theApp.GetPrinterDeviceDefaults(&pi.m_pPD->m_pd)) {
			HDC hDC = pi.m_pPD->m_pd.hDC;
			if (hDC == NULL) {
				hDC = pi.m_pPD->CreatePrinterDC();
			}
			if (hDC != NULL) {
				UINT nHorzPages;
				UINT nVertPages;
				CDC DeviceContext;
				DeviceContext.Attach(hDC);
				printInformation->SetMaxPage(NumPages(&DeviceContext, m_PlotScaleFactor, nHorzPages, nVertPages));
				::DeleteDC(DeviceContext.Detach());
			}
		}
	}
	return DoPreparePrinting(printInformation);
}

void AeSysView::OnDrag() {
	exeCmd(L"dolly ");
}

void AeSysView::OnUpdateDrag(CCmdUI * pCmdUI) {
	pCmdUI->Enable(m_mode == kQuiescent);
}

void AeSysView::OnViewerRegen() {
	m_LayoutHelper->invalidate();
	if (m_LayoutHelper->gsModel()) {
		m_LayoutHelper->gsModel()->invalidate(OdGsModel::kInvalidateAll);
	}
	m_paintMode = PaintMode_Regen;
	PostMessage(WM_PAINT);
}

void AeSysView::OnViewerVpregen() {
	m_LayoutHelper->invalidate();
	if (m_LayoutHelper->gsModel()) {
		m_LayoutHelper->gsModel()->invalidate(getActiveView());
	}
	m_paintMode = PaintMode_Regen;
	PostMessage(WM_PAINT);
}

void AeSysView::OnUpdateViewerRegen(CCmdUI * pCmdUI) {
	pCmdUI->Enable(m_LayoutHelper->gsModel() != 0);
}

// <command_view>
bool AeSysView::canClose() const {
	if (m_mode != kQuiescent) {
		AfxMessageBox(L"Can not exit while command is active.", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	return true;
}

class SaveViewParams {
protected:
	AeSysView* m_View;
	HCURSOR m_Cursor;

public:

	SaveViewParams(AeSysView* view, OdEdInputTracker* inputTracker, HCURSOR cursor, bool snap)
		: m_View(view)
		, m_Cursor(view->cursor()) {
		view->track(inputTracker);
		view->setCursor(cursor);
		
		if (snap) {
			view->m_editor.InitializeSnapping(view->getActiveTopView(), inputTracker);
		}
	}
	~SaveViewParams() {
		m_View->track(0);
		m_View->setCursor(m_Cursor);
		m_View->m_editor.UninitializeSnapping(m_View->getActiveTopView());
	}
};

#define BLINK_CURSOR_TIMER 888
#define BLINK_CURSOR_RATE  GetCaretBlinkTime()

void CALLBACK StringTrackerTimer(HWND hWnd, UINT  nMsg, UINT  nIDTimer, DWORD dwTime);

class SaveViewParams2 : public SaveViewParams {
	bool m_bTimerSet;

public:
	SaveViewParams2(AeSysView* view, OdEdStringTracker* tracker, HCURSOR cursor)
		: SaveViewParams(view, tracker, cursor, false) {
		if (tracker) {
			tracker->setCursor(true);
			::SetTimer(m_View->m_hWnd, BLINK_CURSOR_TIMER, BLINK_CURSOR_RATE, (TIMERPROC) StringTrackerTimer);
			m_bTimerSet = true;
		} else {
			m_bTimerSet = false;
		}
	}
	~SaveViewParams2() {
		if (m_bTimerSet) { ::KillTimer(m_View->m_hWnd, BLINK_CURSOR_TIMER); }
	}
};

// Blink cursor timer

bool AeSysView::UpdateStringTrackerCursor(void) {
	if (m_mode == kGetString && m_response.m_type != Response::kString) {
		if (m_editor.TrackString(m_inpars.result())) {
			getActiveTopView()->invalidate();
			PostMessage(WM_PAINT);
			return true;
		}
	}
	return false;
}

void CALLBACK StringTrackerTimer(HWND hWnd, UINT nMsg, UINT nIDTimer, DWORD dwTime) {
	try {
		auto View {(AeSysView*)CWnd::FromHandle(hWnd)};

		if (!View->UpdateStringTrackerCursor()) {
			KillTimer(hWnd, nIDTimer);
		}
	} catch (...) {
		KillTimer(hWnd, nIDTimer);
	}
}

// </command_view>

OdUInt32 AeSysView::getKeyState() noexcept {
	OdUInt32 KeyState(0);
	if (::GetKeyState(VK_CONTROL) != 0) { KeyState |= MK_CONTROL; }

	if (::GetKeyState(VK_SHIFT) != 0) { KeyState |= MK_SHIFT; }

	return (KeyState);
}

OdGePoint3d AeSysView::getPoint(const OdString & prompt, int options, OdEdPointTracker * tracker) {
	m_sPrompt.empty();
	OdSaveState<OdString> savePrompt(m_sPrompt);
	putString(prompt);

	OdSaveState<Mode> saved_m_mode(m_mode, kGetPoint);

	m_response.m_type = Response::kNone;
	m_inpOptions = options;

	SaveViewParams svp(this, tracker, ::LoadCursor(0, IDC_CROSS), !GETBIT(options, OdEd::kGptNoOSnap));

	while (theApp.PumpMessage()) {
		switch (m_response.m_type) {
			case Response::kPoint:
				if (GETBIT(m_inpOptions, OdEd::kGptBeginDrag)) { SetCapture(); }
				return m_response.m_Point;

			case Response::kString:
				throw OdEdOtherInput(m_response.m_string);

			case Response::kCancel:
				throw OdEdCancel();
		}
		long Idle = 0;
		while (theApp.OnIdle(Idle++));
	}
	throw OdEdCancel();
}

OdString AeSysView::getString(const OdString & prompt, int options, OdEdStringTracker * tracker) {
	m_sPrompt.empty();
	OdSaveState<OdString> savePrompt(m_sPrompt);
	putString(prompt);

	OdSaveState<Mode> saved_m_mode(m_mode, kGetString);

	m_response.m_type = Response::kNone;

	if (tracker) { m_inpars.reset(true); }

	m_inpOptions = options;

	SaveViewParams2 svp(this, tracker, ::LoadCursor(0, IDC_IBEAM));

	while (theApp.PumpMessage()) {
		switch (m_response.m_type) {
			case Response::kString:
				return m_response.m_string;

			case Response::kCancel:
				throw OdEdCancel();
				break;
		}
		long Idle = 0;
		while (theApp.OnIdle(Idle++));
	}
	throw OdEdCancel();
}

void AeSysView::putString(const OdString& string) {
	m_sPrompt = string;
	auto n {m_sPrompt.reverseFind('\n')};

	LPCWSTR Text {string};

	if (n >= 0) { Text = Text + n + 1; }

	theApp.AddStringToMessageList(Text);
	theApp.SetStatusPaneTextAt(nStatusInfo, Text);
}

void AeSysView::track(OdEdInputTracker* inputTracker) {
	m_editor.SetTracker(inputTracker);
}

HCURSOR AeSysView::cursor() const noexcept {
	return (m_hCursor);
}
void AeSysView::setCursor(HCURSOR cursor) noexcept {
	m_hCursor = cursor;
	::SetCursor(cursor);
}

void AeSysView::OnRefresh() {
	PostMessage(WM_PAINT);
}
bool AeSysView::beginDragCallback(const OdGePoint3d & point) {
	OdSaveState<Mode> saved_m_mode(m_mode, kDragDrop);
	GetDocument()->startDrag(point);
	return true;
}

struct ReactorSort : public std::binary_function<OdDbObjectId, OdDbObjectId, bool> {
	bool operator()(OdDbObjectId id1, OdDbObjectId id2) {
		OdDbObjectPtr o2 = id2.openObject();
		if (o2.isNull()) {
			return false;
		}
		OdDbObjectIdArray r2 = o2->getPersistentReactors();
		if (r2.contains(id1)) {
			return true;
		}
		return false;
	}
};

void transform_object_set(OdDbObjectIdArray & objs, OdGeMatrix3d & xform) {
	std::sort(objs.begin(), objs.end(), ReactorSort());
	for (unsigned int i = 0; i < objs.size(); ++i) {
		OdDbEntityPtr pEnt = objs[i].safeOpenObject(OdDb::kForWrite);
		pEnt->transformBy(xform);
	}
}

// <command_console>
BOOL AeSysView::OnDrop(COleDataObject * pDataObject, DROPEFFECT dropEffect, CPoint point) {
	OdSharedPtr<AeSysDoc::ClipboardData> pData = AeSysDoc::ClipboardData::get(pDataObject);
	if (pData) {
		auto Document {GetDocument()};
		OdDbDatabase* Database = Document->m_DatabasePtr;
		Database->startUndoRecord();

		OdGeMatrix3d xform = OdGeMatrix3d::translation(m_editor.ToEyeToWorld(point.x, point.y) - pData->pickPoint());

		if (m_mode == kDragDrop) {
			auto SelectionSet {Document->selectionSet()};
			OdDbEntityPtr pEnt;
			OdDbObjectIdArray objs = SelectionSet->objectIdArray();
			if (::GetKeyState(VK_CONTROL) & 0xff00) {
				OdDbIdMappingPtr pIdMapping = OdDbIdMapping::createObject();
				OdDbDatabase* pHostDb = Database;
				pHostDb->deepCloneObjects(objs, pHostDb->getActiveLayoutBTRId(), *pIdMapping);

				for (unsigned int i = 0; i < objs.size(); ++i) {
					OdDbIdPair idPair(objs[i]);
					pIdMapping->compute(idPair);
					objs[i] = idPair.value();
				}
			}
			transform_object_set(objs, xform);
		} else {
			try {
				OdDbDatabasePtr pTmpDb = theApp.readFile(pData->tempFileName(), true, false, Oda::kShareDenyNo);
				Database->insert(xform, pTmpDb);
			} catch (const OdError & ex) {
				AfxMessageBox(ex.description());
				return FALSE;
			}
		}
		return TRUE;
	}
	return __super::OnDrop(pDataObject, dropEffect, point);
}
// </command_console>

DROPEFFECT AeSysView::OnDragOver(COleDataObject * pDataObject, DWORD dwKeyState, CPoint point) {
	if (m_mode == kQuiescent || m_mode == kDragDrop) {
		if (AeSysDoc::ClipboardData::isAcadDataAvailable(pDataObject)) {
			return (::GetKeyState(VK_CONTROL) & 0xff00) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
		}
	}
	return __super::OnDragOver(pDataObject, dwKeyState, point);
}

BOOL AeSysView::PreCreateWindow(CREATESTRUCT & createStructure) {
	// <tas="Modify the Window class or styles here by modifying the CREATESTRUCT"/>
	
return CView::PreCreateWindow(createStructure);
}

void AeSysView::OnUpdate(CView * sender, LPARAM hint, CObject * hintObject) {
	CDC* DeviceContext = GetDC();
	const COLORREF BackgroundColor = DeviceContext->GetBkColor();
	DeviceContext->SetBkColor(ViewBackgroundColor);

	int PrimitiveState = 0;
	int iDrawMode = 0;

	if ((hint & EoDb::kSafe) == EoDb::kSafe) {
		PrimitiveState = pstate.Save();
	}
	if ((hint & EoDb::kErase) == EoDb::kErase) {
		iDrawMode = pstate.SetROP2(DeviceContext, R2_XORPEN);
	}
	if ((hint & EoDb::kTrap) == EoDb::kTrap) {
		EoDbPrimitive::SetHighlightColorIndex(theApp.TrapHighlightColor());
	}
	switch (hint) {
		case EoDb::kPrimitive:
		case EoDb::kPrimitiveSafe:
		case EoDb::kPrimitiveEraseSafe:
			((EoDbPrimitive*) hintObject)->Display(this, DeviceContext);
			break;

		case EoDb::kGroup:
		case EoDb::kGroupSafe:
		case EoDb::kGroupEraseSafe:
		case EoDb::kGroupSafeTrap:
		case EoDb::kGroupEraseSafeTrap:
			((EoDbGroup*) hintObject)->Display(this, DeviceContext);
			break;

		case EoDb::kGroups:
		case EoDb::kGroupsSafe:
		case EoDb::kGroupsSafeTrap:
		case EoDb::kGroupsEraseSafeTrap:
			((EoDbGroupList*) hintObject)->Display(this, DeviceContext);
			break;

		case EoDb::kLayer:
		case EoDb::kLayerErase:
			((EoDbLayer*) hintObject)->Display(this, DeviceContext);
			break;

		default:
			CView::OnUpdate(sender, hint, hintObject);
	}
	if ((hint & EoDb::kTrap) == EoDb::kTrap) {
		EoDbPrimitive::SetHighlightColorIndex(0);
	}
	if ((hint & EoDb::kErase) == EoDb::kErase) {
		pstate.SetROP2(DeviceContext, iDrawMode);
	}
	if ((hint & EoDb::kSafe) == EoDb::kSafe) {
		pstate.Restore(DeviceContext, PrimitiveState);
	}
	DeviceContext->SetBkColor(BackgroundColor);
	ReleaseDC(DeviceContext);
}

void AeSysView::respond(const OdString & s) {
	m_response.m_type = Response::kString;
	m_response.m_string = s;
}

CRect AeSysView::viewportRect() const
{
	CRect ClientRectangle;
	GetClientRect(&ClientRectangle);
	return ClientRectangle;
}

CRect AeSysView::viewRect(OdGsView* view)
{
	OdGePoint3d LowerLeftPoint;
	OdGePoint3d UpperRightPoint;
	view->getViewport((OdGePoint2d&)LowerLeftPoint, (OdGePoint2d&)UpperRightPoint);
	OdGeMatrix3d ScreenMatrix = view->screenMatrix();
	LowerLeftPoint.transformBy(ScreenMatrix);
	UpperRightPoint.transformBy(ScreenMatrix);
	return CRect(OdRoundToLong(LowerLeftPoint.x), OdRoundToLong(UpperRightPoint.y), OdRoundToLong(UpperRightPoint.x), OdRoundToLong(LowerLeftPoint.y));
}

void AeSysView::OnChar(UINT characterCodeValue, UINT repeatCount, UINT flags) {
	__super::OnChar(characterCodeValue, repeatCount, flags);

	m_response.m_string = m_inpars.result();
	switch (characterCodeValue) {
		case VK_BACK:
			while (repeatCount--) {
				m_inpars.eraseChar();
			}
			break;

		case VK_ESCAPE:
			m_response.m_type = Response::kCancel;
			m_inpars.reset(false);

			switch (m_mode) {
				case kQuiescent:
					if (m_editor.Unselect()) {
						PostMessage(WM_PAINT);
					}
					break;

				case kGetPoint:
					// no op
					break;
			}
			break;

		default:
			while (repeatCount--) {
				if (!m_inpars.addChar((OdChar) characterCodeValue)) {
					m_inpars.reset(false);
					switch (m_mode) {
						case kQuiescent:
							if (m_response.m_string.isEmpty()) {
								// <command_console>
								GetDocument()->ExecuteCommand(GetDocument()->RecentCommandName());
								// </command_console>
							} else {
								GetDocument()->ExecuteCommand(m_response.m_string);
							}
							break;

						case kGetPoint:
						case kGetString:
							m_response.m_type = Response::kString;
							break;
					}
				}
			}
			break;
	}
	if (m_mode == kGetString && m_response.m_type != Response::kString && m_inpars.result() != m_response.m_string) {
		if (m_editor.TrackString(m_inpars.result())) {
			getActiveTopView()->invalidate();
			PostMessage(WM_PAINT);
		}
	}
	if (m_sPrompt.isEmpty()) {
		m_sPrompt = L"command: ";
	} else if (m_inpars.result().isEmpty()) {
		theApp.SetStatusPaneTextAt(nStatusInfo, m_sPrompt);
	} else {
		theApp.SetStatusPaneTextAt(nStatusInfo, m_inpars.result());
	}
}

void AeSysView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	switch (nChar) {
		case VK_ESCAPE:
			break;

		case VK_F5:
			PostMessage(WM_PAINT);
			break;

		case VK_DELETE:
			GetDocument()->DeleteSelection(false);
			PostMessage(WM_PAINT);
			break;
	}
	__super::OnKeyDown(nChar, nRepCnt, nFlags);
}

void AeSysView::OnLButtonDown(UINT flags, CPoint point) {
	if (AeSysApp::CustomLButtonDownCharacters.IsEmpty()) {
		__super::OnLButtonDown(flags, point);

		switch (m_mode) {
			case kQuiescent:
				if (m_editor.OnMouseLeftButtonClick(flags, point.x, point.y, this)) {
					PostMessage(WM_PAINT);
				}
				break;
			case kGetPoint:
				m_response.m_Point = m_editor.ToEyeToWorld(point.x, point.y);
				
				if (!GETBIT(m_inpOptions, OdEd::kGptNoUCS)) {
					if (!m_editor.ToUcsToWorld(m_response.m_Point))
						break;
				}
				m_editor.Snap(m_response.m_Point);
				m_response.m_type = Response::kPoint;
				break;
//			default:
//				m_LeftButton = true;
//				m_MousePosition = point;
//				m_MouseClick = point;
//				
//				if (m_ZoomWindow == true) {
//					m_Points.clear();
//					m_Points.append(GetWorldCoordinates(point));
//				}
		}

	} else {
		DoCustomMouseClick(AeSysApp::CustomLButtonDownCharacters);
	}
}

void AeSysView::OnLButtonUp(UINT flags, CPoint point) {
	
	if (AeSysApp::CustomLButtonUpCharacters.IsEmpty()) {

		__super::OnLButtonUp(flags, point);

		if (m_mode == kGetPoint && GetCapture() == this) {
			m_response.m_Point = m_editor.ToEyeToWorld(point.x, point.y);
			
			if (!GETBIT(m_inpOptions, OdEd::kGptNoUCS)) {

				if (!m_editor.ToUcsToWorld(m_response.m_Point)) { return; }
			}
			m_response.m_type = Response::kPoint;
			ReleaseCapture();
		}
		m_editor.SetEntityCenters();

//		m_LeftButton = false;
//		if (m_ZoomWindow == true) {
//			m_Points.append(GetWorldCoordinates(point));
//			if (m_Points.length() == 2) // Zoom rectangle has been completely defined
//			{
//				ZoomWindow(m_Points[0], m_Points[1]);
//				m_ZoomWindow = false;
//				m_Points.clear();
//			}
//		}
	} else {
		DoCustomMouseClick(AeSysApp::CustomLButtonUpCharacters);
	}
}

void AeSysView::OnMButtonDown(UINT flags, CPoint point) {
	m_MiddleButton = true;
	m_MousePosition = point;
	__super::OnMButtonDown(flags, point);
}

void AeSysView::OnMButtonUp(UINT flags, CPoint point) {
	m_MiddleButton = false;
	__super::OnMButtonUp(flags, point);
}

void AeSysView::OnMouseMove(UINT flags, CPoint point) {
	DisplayOdometer();
	if (m_MousePosition != point) {

		switch (m_mode) {
			case kQuiescent:
				m_editor.OnMouseMove(flags, point.x, point.y);
				break;

			case kGetPoint:
			{
				auto Point {m_editor.ToEyeToWorld(point.x, point.y)};

				if (!GETBIT(m_inpOptions, OdEd::kGptNoUCS)) {
					if (!m_editor.ToUcsToWorld(Point)) { return; }
				}

				if (!GETBIT(m_inpOptions, OdEd::kGptNoOSnap)) {
					m_editor.Snap(Point);
				}
				m_editor.TrackPoint(Point);
				break;
			}
		}
		if (m_LeftButton == true) {
			CClientDC dc(this);
			CRect rcZoomOld;
			rcZoomOld.SetRect(m_MouseClick.x, m_MouseClick.y, m_MousePosition.x, m_MousePosition.y);
			rcZoomOld.NormalizeRect();
			rcZoomOld.InflateRect(1, 1);

			RedrawWindow(&rcZoomOld);

			CRect rcZoom;
			rcZoom.SetRect(m_MouseClick.x, m_MouseClick.y, point.x, point.y);
			rcZoom.NormalizeRect();

			dc.DrawFocusRect(&rcZoom);
		}
		else if (m_MiddleButton == true) {
			OdGsViewPtr FirstView {m_LayoutHelper->viewAt(0)};

			OdGeVector3d DollyVector(double(m_MousePosition.x) - double(point.x), double(m_MousePosition.y) - double(point.y), 0.);

			DollyVector.transformBy((FirstView->screenMatrix() * FirstView->projectionMatrix()).inverse());
			FirstView->dolly(DollyVector);

			m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
			m_ViewTransform.BuildTransformMatrix();

			PostMessageW(WM_PAINT);
		}
		else if (m_RightButton == true) {
			Orbit((double(m_MousePosition.y) - double(point.y)) / 100., (double(m_MousePosition.x) - double(point.x)) / 100.);
			PostMessageW(WM_PAINT);
		}
		m_MousePosition = point;
	}
	__super::OnMouseMove(flags, point);

	switch (theApp.CurrentMode()) {
		case ID_MODE_ANNOTATE:
			DoAnnotateModeMouseMove();
			break;

		case ID_MODE_DRAW:
			DoDrawModeMouseMove();
			break;

		case ID_MODE_DRAW2:
			DoDraw2ModeMouseMove();
			break;

		case ID_MODE_LPD:
			DoDuctModeMouseMove();
			break;

		case ID_MODE_NODAL:
			DoNodalModeMouseMove();
			break;

		case ID_MODE_PIPE:
			DoPipeModeMouseMove();
			break;

		case ID_MODE_POWER:
			DoPowerModeMouseMove();
			break;

		case ID_MODE_PRIMITIVE_EDIT:
			PreviewPrimitiveEdit();
			break;

		case ID_MODE_PRIMITIVE_MEND:
			PreviewMendPrimitive();
			break;

		case ID_MODE_GROUP_EDIT:
			PreviewGroupEdit();
			break;
	}
	if (m_RubberbandType != None) {
		auto DeviceContext {GetDC()};
		const int DrawMode {DeviceContext->SetROP2(R2_XORPEN)};
		CPen RubberbandPen(PS_SOLID, 0, RubberbandColor);
		CPen* Pen = DeviceContext->SelectObject(&RubberbandPen);

		if (m_RubberbandType == Lines) {
			DeviceContext->MoveTo(m_RubberbandLogicalBeginPoint);
			DeviceContext->LineTo(m_RubberbandLogicalEndPoint);

			m_RubberbandLogicalEndPoint = point;
			DeviceContext->MoveTo(m_RubberbandLogicalBeginPoint);
			DeviceContext->LineTo(m_RubberbandLogicalEndPoint);
		}
		else if (m_RubberbandType == Rectangles) {
			CBrush* Brush {(CBrush*)DeviceContext->SelectStockObject(NULL_BRUSH)};

			DeviceContext->Rectangle(m_RubberbandLogicalBeginPoint.x, m_RubberbandLogicalBeginPoint.y, m_RubberbandLogicalEndPoint.x, m_RubberbandLogicalEndPoint.y);

			m_RubberbandLogicalEndPoint = point;
			DeviceContext->Rectangle(m_RubberbandLogicalBeginPoint.x, m_RubberbandLogicalBeginPoint.y, m_RubberbandLogicalEndPoint.x, m_RubberbandLogicalEndPoint.y);
			DeviceContext->SelectObject(Brush);
		}
		DeviceContext->SelectObject(Pen);
		DeviceContext->SetROP2(DrawMode);
		ReleaseDC(DeviceContext);
	}
}

BOOL AeSysView::OnMouseWheel(UINT nFlags, OdInt16 zDelta, CPoint point) {
	//ScreenToClient(&point);

	//if (m_editor.OnMouseWheel(nFlags, point.x, point.y, zDelta)) {
	//    PostMessage(WM_PAINT);
	//    propagateActiveViewChanges();
	//}
	DollyAndZoom((zDelta > 0) ? 1. / 0.9 : 0.9);
	InvalidateRect(nullptr);

	return __super::OnMouseWheel(nFlags, zDelta, point);
}

void AeSysView::OnRButtonDown(UINT flags, CPoint point) {
	if (AeSysApp::CustomRButtonDownCharacters.IsEmpty()) {
		m_RightButton = true;
		m_MousePosition = point;
		__super::OnRButtonDown(flags, point);
	} else {
		DoCustomMouseClick(AeSysApp::CustomRButtonDownCharacters);
	}
}

void AeSysView::OnRButtonUp(UINT flags, CPoint point) {
	if (AeSysApp::CustomRButtonUpCharacters.IsEmpty()) {
		m_RightButton = false;
	
		// <tas="Context menus using right mouse button goes here."/>
		
		__super::OnRButtonUp(flags, point);
	} else {
		DoCustomMouseClick(AeSysApp::CustomRButtonUpCharacters);
	}
}

struct OdExRegenCmd : OdEdCommand {
	OdGsLayoutHelper* m_LayoutHelper;
	AeSysView* m_View;
	const OdString groupName() const override { return L"REGEN"; }
	const OdString globalName() const override { return L"REGEN"; }

	OdInt32 flags() const override {
		return OdEdCommand::flags() | OdEdCommand::kNoUndoMarker;
	}

	void execute(OdEdCommandContext* edCommandContext) noexcept override {
		// <tas="placeholder until implemented" m_View->OnViewerRegen();"</tas>
	}
};

OdEdCommandPtr AeSysView::command(const OdString& commandName) {
	if (commandName.iCompare(L"REGEN") == 0) {
		OdSmartPtr<OdExRegenCmd> c = OdRxObjectImpl<OdExRegenCmd>::createObject();
		c->m_View = this;
		c->m_LayoutHelper = m_LayoutHelper;
		return c;
	} else {
		return m_editor.Command(commandName);
	}
}

OdExEditorObject& AeSysView::editorObject() noexcept {
	return m_editor;
}

const OdExEditorObject& AeSysView::editorObject() const noexcept {
	return m_editor;
}

bool AeSysView::isModelSpaceView() const {
	return (getDatabase()->getTILEMODE());
	//return m_bPsOverall;
}

OdIntPtr AeSysView::drawableFilterFunctionId(OdDbStub* viewportId) const {

	if (theApp.pagingType() == OdDb::kPage || theApp.pagingType() == OdDb::kUnload) {
		return OdGiContextForDbDatabase::drawableFilterFunctionId(viewportId) | kDrawableFilterAppRangeStart;
	}
	return OdGiContextForDbDatabase::drawableFilterFunctionId(viewportId);
}

OdUInt32 AeSysView::drawableFilterFunction(OdIntPtr functionId, const OdGiDrawable* drawable, OdUInt32 flags) {
	
	if (theApp.pagingType() == OdDb::kPage || theApp.pagingType() == OdDb::kUnload) {
		getDatabase()->pageObjects();
	}
	return OdGiContextForDbDatabase::drawableFilterFunction(functionId & ~kDrawableFilterAppRangeMask, drawable, flags);
}

BOOL AeSysView::OnIdle(long count) {
	if (!m_LayoutHelper->isValid()) {
		PostMessage(WM_PAINT);
	}
	return TRUE;
}

// <tas=Transition - code above sourced from Teigha examples"</tas>

OdDbDatabasePtr AeSysView::Database() const {
	return GetDocument()->m_DatabasePtr;
}

void AeSysView::OnActivateFrame(UINT state, CFrameWnd * deactivateFrame) {
	ATLTRACE2(atlTraceGeneral, 1, L"AeSysView<%08.8lx>::OnActivateFrame(%i, %08.8lx)\n", this, state, deactivateFrame);

	CView::OnActivateFrame(state, deactivateFrame);
}
void AeSysView::OnActivateView(BOOL activate, CView * activateView, CView * deactiveView) {
	ATLTRACE2(atlTraceGeneral, 1, L"AeSysView<%08.8lx>::OnActivateView(%i, %08.8lx, %08.8lx))\n", this, activate, activateView, deactiveView);

	CMainFrame* MainFrame = (CMainFrame*) (AfxGetMainWnd());
	if (activate) {
		if (::CopyAcceleratorTable(MainFrame->m_hAccelTable, NULL, 0) == 0) { // Accelerator table was destroyed when keyboard focus was killed - reload resource
			theApp.BuildModeSpecificAcceleratorTable();
		}
	}
	CMFCPropertyGridProperty& ActiveViewScaleProperty = MainFrame->GetPropertiesPane().GetActiveViewScaleProperty();
	ActiveViewScaleProperty.SetValue(m_WorldScale);
	ActiveViewScaleProperty.Enable(activate);
	SetCursorPosition(OdGePoint3d::kOrigin);
	CView::OnActivateView(activate, activateView, deactiveView);
}
void AeSysView::OnSetFocus(CWnd * oldWindow) {
	CMainFrame* MainFrame = (CMainFrame*) (AfxGetMainWnd());
	if (::CopyAcceleratorTable(MainFrame->m_hAccelTable, NULL, 0) == 0) { // Accelerator table was destroyed when keyboard focus was killed - reload resource
		theApp.BuildModeSpecificAcceleratorTable();
	}
	CView::OnSetFocus(oldWindow);
}
void AeSysView::OnKillFocus(CWnd * newWindow) {
	HACCEL AcceleratorTableHandle = ((CMainFrame*) AfxGetMainWnd())->m_hAccelTable;

	::DestroyAcceleratorTable(AcceleratorTableHandle);

	CView::OnKillFocus(newWindow);
}

void AeSysView::OnPrepareDC(CDC * deviceContext, CPrintInfo * printInformation) {
	CView::OnPrepareDC(deviceContext, printInformation);

	if (deviceContext->IsPrinting()) {
		if (m_Plot) {
			const double HorizontalSizeInInches = static_cast<double>(deviceContext->GetDeviceCaps(HORZSIZE) / EoMmPerInch) / m_PlotScaleFactor;
			const double VerticalSizeInInches = static_cast<double>(deviceContext->GetDeviceCaps(VERTSIZE) / EoMmPerInch) / m_PlotScaleFactor;

			UINT nHorzPages;
			UINT nVertPages;

			NumPages(deviceContext, m_PlotScaleFactor, nHorzPages, nVertPages);

			const double dX = ((printInformation->m_nCurPage - 1) % nHorzPages) * HorizontalSizeInInches;
			const double dY = ((printInformation->m_nCurPage - 1) / nHorzPages) * VerticalSizeInInches;

			m_ViewTransform.SetProjectionPlaneField(0., 0., HorizontalSizeInInches, VerticalSizeInInches);
			const OdGePoint3d Target(OdGePoint3d(dX, dY, 0.));
			m_ViewTransform.SetTarget(Target);
			const OdGePoint3d Position(Target + OdGeVector3d::kZAxis);
			m_ViewTransform.SetPosition_(Position);
			m_ViewTransform.SetViewUp(OdGeVector3d::kYAxis);
			// <tas="Near Far clipping on Plot DC prepare?
			m_ViewTransform.SetNearClipDistance(-1000.);
			m_ViewTransform.SetFarClipDistance(1000.);
			//</tas>
			m_ViewTransform.EnablePerspective(false);
			m_ViewTransform.BuildTransformMatrix();
		} else {
		}
	}
}

void AeSysView::OnContextMenu(CWnd*, CPoint point) {
	theApp.ShowPopupMenu(IDR_CONTEXT_MENU, point, this);
}
void AeSysView::DoCustomMouseClick(const CString & characters) {
	int Position = 0;

	while (Position < characters.GetLength()) {
		if (characters.Find(L"{", Position) == Position) {
			Position++;
			CString VirtualKey = characters.Tokenize(L"}", Position);
			PostMessageW(WM_KEYDOWN, _wtoi(VirtualKey), 0L);
		} else {
			PostMessageW(WM_CHAR, characters[Position++], 0L);
		}
	}
}

CMFCStatusBar& AeSysView::GetStatusBar(void) const {
	return ((CMainFrame*) AfxGetMainWnd())->GetStatusBar();
}
void AeSysView::PopViewTransform() {
	if (!m_ViewTransforms.IsEmpty()) {
		m_ViewTransform = m_ViewTransforms.RemoveTail();
	}
	m_ViewTransform.BuildTransformMatrix();
}
void AeSysView::PushViewTransform() {
	m_ViewTransforms.AddTail(m_ViewTransform);
}
EoGeMatrix3d AeSysView::ModelToWorldTransform() const noexcept {
	return m_ModelTransform.ModelMatrix();
}
void AeSysView::PushModelTransform(const EoGeMatrix3d & transformation) {
	m_ModelTransform.PushModelTransform(transformation);
}
void AeSysView::PopModelTransform() {
	m_ModelTransform.PopModelTransform();
}
void AeSysView::BackgroundImageDisplay(CDC * deviceContext) {
	if (m_ViewBackgroundImage && ((HBITMAP) m_BackgroundImageBitmap != 0)) {
		const int iWidDst = int(m_Viewport.WidthInPixels());
		const int iHgtDst = int(m_Viewport.HeightInPixels());

		BITMAP bm;
		m_BackgroundImageBitmap.GetBitmap(&bm);
		CDC dcMem;
		dcMem.CreateCompatibleDC(NULL);
		CBitmap* pBitmap = dcMem.SelectObject(&m_BackgroundImageBitmap);
		CPalette* pPalette = deviceContext->SelectPalette(&m_BackgroundImagePalette, FALSE);
		deviceContext->RealizePalette();

		const OdGePoint3d Target = m_ViewTransform.Target();
		const OdGePoint3d ptTargetOver = m_OverviewViewTransform.Target();
		const double dU = Target.x - ptTargetOver.x;
		const double dV = Target.y - ptTargetOver.y;

		// Determine the region of the bitmap to tranfer to display
		CRect rcWnd;
		rcWnd.left = EoRound((m_ViewTransform.FieldWidthMinimum() - OverviewUMin() + dU) / OverviewUExt() * static_cast<double>(bm.bmWidth));
		rcWnd.top = EoRound((1. - (m_ViewTransform.FieldHeightMaximum() - OverviewVMin() + dV) / OverviewVExt()) * static_cast<double>(bm.bmHeight));
		rcWnd.right = EoRound((m_ViewTransform.FieldWidthMaximum() - OverviewUMin() + dU) / OverviewUExt() * static_cast<double>(bm.bmWidth));
		rcWnd.bottom = EoRound((1. - (m_ViewTransform.FieldHeightMinimum() - OverviewVMin() + dV) / OverviewVExt()) * static_cast<double>(bm.bmHeight));

		const int iWidSrc = rcWnd.Width();
		const int iHgtSrc = rcWnd.Height();

		deviceContext->StretchBlt(0, 0, iWidDst, iHgtDst, &dcMem, (int) rcWnd.left, (int) rcWnd.top, iWidSrc, iHgtSrc, SRCCOPY);

		dcMem.SelectObject(pBitmap);
		deviceContext->SelectPalette(pPalette, FALSE);
	}
}
double AeSysView::OverviewUExt() noexcept {
	return m_OverviewViewTransform.FieldWidth();
}
double AeSysView::OverviewUMin() noexcept {
	return m_OverviewViewTransform.FieldWidthMinimum();
}
double AeSysView::OverviewVExt() noexcept {
	return m_OverviewViewTransform.FieldHeight();
}
double AeSysView::OverviewVMin() noexcept {
	return m_OverviewViewTransform.FieldHeightMinimum();
}
CPoint AeSysView::DoViewportProjection(const EoGePoint4d & point) const noexcept {
	return m_Viewport.DoProjection(point);
}
void AeSysView::DoViewportProjection(CPoint * pnt, int iPts, EoGePoint4d * pt) const noexcept {
	m_Viewport.DoProjection(pnt, iPts, pt);
}
void AeSysView::DoViewportProjection(CPoint * pnt, EoGePoint4dArray & points) const {
	m_Viewport.DoProjection(pnt, points);
}
OdGePoint3d AeSysView::DoViewportProjectionInverse(const OdGePoint3d & point) const noexcept {
	OdGePoint3d Point = point;
	m_Viewport.DoProjectionInverse(Point);
	return (Point);
}
double AeSysView::ViewportHeightInInches() const noexcept {
	return m_Viewport.HeightInInches();
}
double AeSysView::ViewportWidthInInches() const noexcept {
	return m_Viewport.WidthInInches();
}
void AeSysView::ViewportPopActive() {
	if (!m_Viewports.IsEmpty()) {
		m_Viewport = m_Viewports.RemoveTail();
	}
}
void AeSysView::ViewportPushActive() {
	m_Viewports.AddTail(m_Viewport);
}
void AeSysView::SetViewportSize(const int width, const int height) noexcept {
	m_Viewport.SetSize(width, height);
}
void AeSysView::SetDeviceHeightInInches(double height) noexcept {
	m_Viewport.SetDeviceHeightInInches(height);
}
void AeSysView::SetDeviceWidthInInches(double width) noexcept {
	m_Viewport.SetDeviceWidthInInches(width);
}
// AeSysView printing
void AeSysView::OnFilePlotFull() {
	m_Plot = true;
	m_PlotScaleFactor = 1.0f;
	CView::OnFilePrint();
}
void AeSysView::OnFilePlotHalf() {
	m_Plot = true;
	m_PlotScaleFactor = 0.5f;
	CView::OnFilePrint();
}
void AeSysView::OnFilePlotQuarter() {
	m_Plot = true;
	m_PlotScaleFactor = 0.25f;
	CView::OnFilePrint();
}
void AeSysView::OnFilePrint() {
	m_Plot = false;
	m_PlotScaleFactor = 1.0f;
	CView::OnFilePrint();
}
UINT AeSysView::NumPages(CDC * deviceContext, double dScaleFactor, UINT & nHorzPages, UINT & nVertPages) {
	OdGeExtents3d Extents;
	GetDocument()->GetExtents___(this, Extents);
	const OdGePoint3d MinimumPoint = Extents.minPoint();
	const OdGePoint3d MaximumPoint = Extents.maxPoint();

	const double HorizontalSizeInInches = static_cast<double>(deviceContext->GetDeviceCaps(HORZSIZE)) / EoMmPerInch;
	const double VerticalSizeInInches = static_cast<double>(deviceContext->GetDeviceCaps(VERTSIZE)) / EoMmPerInch;

	nHorzPages = EoRound(((MaximumPoint.x - MinimumPoint.x) * dScaleFactor / HorizontalSizeInInches) + 0.5);
	nVertPages = EoRound(((MaximumPoint.y - MinimumPoint.y) * dScaleFactor / VerticalSizeInInches) + 0.5);

	return nHorzPages * nVertPages;
}
void AeSysView::DisplayPixel(CDC * deviceContext, COLORREF cr, const OdGePoint3d & point) {
	EoGePoint4d ptView(point, 1.);

	ModelViewTransformPoint(ptView);

	if (ptView.IsInView()) {
		deviceContext->SetPixel(DoViewportProjection(ptView), cr);
	}
}
void AeSysView::Orbit(double x, double y) {
	auto FirstView {m_LayoutHelper->viewAt(0)};

	FirstView->orbit(x, y);
	m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
}
void AeSysView::Dolly() {
	CPoint Point;
	::GetCursorPos(&Point);
	ScreenToClient(&Point);

	OdGsViewPtr FirstView = m_LayoutHelper->viewAt(0);
	OdGePoint3d Position(FirstView->position());
	Position.transformBy(FirstView->worldToDeviceMatrix());

	int x = int(OdRound(Position.x));
	int y = int(OdRound(Position.y));

	x = Point.x - x;
	y = Point.y - y;

	OdGeVector3d DollyVector(double(x), double(y), 0.);
	DollyVector.transformBy((FirstView->screenMatrix() * FirstView->projectionMatrix()).inverse());
	FirstView->dolly(DollyVector);

	m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);

	SetCursorPosition(FirstView->target());
}
void AeSysView::DollyAndZoom(double zoomFactor) {
	Dolly();
	auto FirstView {m_LayoutHelper->viewAt(0)};
	FirstView->zoom(zoomFactor);

	m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);

	SetCursorPosition(FirstView->target());
}
void AeSysView::OnSetupScale() {
	EoDlgSetScale dlg;
	dlg.m_Scale = WorldScale();
	if (dlg.DoModal() == IDOK) {
		SetWorldScale(dlg.m_Scale);
	}
}
void AeSysView::On3dViewsTop() {
	auto FirstView {m_LayoutHelper->viewAt(0)};

	m_ViewTransform.EnablePerspective(false);

	const auto Target(FirstView->target());
	const auto Position(Target + OdGeVector3d::kZAxis * FirstView->lensLength());
	auto UpVector(OdGeVector3d::kYAxis);

	FirstView->setView(Position, Target, UpVector, FirstView->fieldWidth(), FirstView->fieldHeight());

	m_ViewTransform.SetView(Position, Target, UpVector, FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
}

void AeSysView::On3dViewsBottom() {
	auto FirstView {m_LayoutHelper->viewAt(0)};

	m_ViewTransform.EnablePerspective(false);

	const auto Target(FirstView->target());
	const auto Position(Target - OdGeVector3d::kZAxis);
	auto UpVector(OdGeVector3d::kYAxis);

	FirstView->setView(Position, Target, UpVector, FirstView->fieldWidth(), FirstView->fieldHeight());

	m_ViewTransform.SetView(Position, Target, UpVector, FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
}
void AeSysView::On3dViewsLeft() {
	auto FirstView {m_LayoutHelper->viewAt(0)};

	m_ViewTransform.EnablePerspective(false);

	const auto Target(FirstView->target());
	const auto Position(Target - OdGeVector3d::kXAxis * FirstView->lensLength());
	auto UpVector(OdGeVector3d::kZAxis);

	FirstView->setView(Position, Target, UpVector, FirstView->fieldWidth(), FirstView->fieldHeight());

	m_ViewTransform.SetView(Position, Target, UpVector, FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
}
void AeSysView::On3dViewsRight() {
	auto FirstView {m_LayoutHelper->viewAt(0)};

	m_ViewTransform.EnablePerspective(false);

	const auto Target(FirstView->target());
	const auto Position(Target + OdGeVector3d::kXAxis * FirstView->lensLength());
	auto UpVector(OdGeVector3d::kZAxis);

	FirstView->setView(Position, Target, UpVector, FirstView->fieldWidth(), FirstView->fieldHeight());

	m_ViewTransform.SetView(Position, Target, UpVector, FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
}

void AeSysView::On3dViewsFront() {
	auto FirstView {m_LayoutHelper->viewAt(0)};

	m_ViewTransform.EnablePerspective(false);

	const auto Target(FirstView->target());
	const auto Position(Target - OdGeVector3d::kYAxis * FirstView->lensLength());
	auto UpVector(OdGeVector3d::kZAxis);

	FirstView->setView(Position, Target, UpVector, FirstView->fieldWidth(), FirstView->fieldHeight());

	m_ViewTransform.SetView(Position, Target, UpVector, FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
}

void AeSysView::On3dViewsBack() {
	auto FirstView {m_LayoutHelper->viewAt(0)};

	m_ViewTransform.EnablePerspective(false);

	const auto Target(FirstView->target());
	const auto Position(Target + OdGeVector3d::kYAxis * FirstView->lensLength());
	auto UpVector(OdGeVector3d::kZAxis);

	FirstView->setView(Position, Target, UpVector, FirstView->fieldWidth(), FirstView->fieldHeight());

	m_ViewTransform.SetView(Position, Target, UpVector, FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
}
void AeSysView::On3dViewsIsometric() {
	static int LeftRight = 0;
	static int FrontBack = 0;
	static int AboveUnder = 0;

	EoDlgSelectIsometricView Dialog;
	Dialog.m_LeftRight = LeftRight;
	Dialog.m_FrontBack = FrontBack;
	Dialog.m_AboveUnder = AboveUnder;
	if (Dialog.DoModal()) {
		LeftRight = Dialog.m_LeftRight;
		FrontBack = Dialog.m_FrontBack;
		AboveUnder = Dialog.m_AboveUnder;

		OdGeVector3d Direction;

		Direction.x = LeftRight == 0 ? .5773503 : -.5773503;
		Direction.y = FrontBack == 0 ? .5773503 : -.5773503;
		Direction.z = AboveUnder == 0 ? -.5773503 : .5773503;

		auto FirstView {m_LayoutHelper->viewAt(0)};

		m_ViewTransform.EnablePerspective(false);

		const auto Target(FirstView->target());
		const auto Position(Target - Direction * FirstView->lensLength());
		auto UpVector {Direction.crossProduct(OdGeVector3d::kZAxis)};
		UpVector = UpVector.crossProduct(Direction);
		UpVector.normalize();

		FirstView->setView(Position, Target, UpVector, FirstView->fieldWidth(), FirstView->fieldHeight());

		m_ViewTransform.SetView(Position, Target, UpVector, FirstView->fieldWidth(), FirstView->fieldHeight());
		m_ViewTransform.BuildTransformMatrix();
		InvalidateRect(nullptr);
	}
}
void AeSysView::OnCameraRotateLeft() {
	Orbit(0., EoToRadian(-10.));
}
void AeSysView::OnCameraRotateRight() {
	Orbit(0., EoToRadian(10.));
}
void AeSysView::OnCameraRotateUp() {
	Orbit(EoToRadian(-10.), 0.);
}
void AeSysView::OnCameraRotateDown() {
	Orbit(EoToRadian(10.), 0.);
}
void AeSysView::OnViewParameters() {
	EoDlgViewParameters Dialog;

	EoGsViewTransform ModelView(m_ViewTransform);

	Dialog.m_ModelView = DWORD(&ModelView);
	Dialog.m_PerspectiveProjection = m_ViewTransform.IsPerspectiveOn();

	if (Dialog.DoModal() == IDOK) {
		m_ViewTransform.EnablePerspective(Dialog.m_PerspectiveProjection == TRUE);
	}
}
void AeSysView::OnViewTrueTypeFonts() {
	m_ViewTrueTypeFonts = !m_ViewTrueTypeFonts;
	InvalidateRect(nullptr);
}
void AeSysView::OnViewPenWidths() {
	m_ViewPenWidths = !m_ViewPenWidths;
	InvalidateRect(nullptr);
}
void AeSysView::OnViewRendermode(UINT commandId) {
	const OdGsView::RenderMode RenderMode = OdGsView::RenderMode(commandId - ID_VIEW_RENDERMODE_2DOPTIMIZED);
	SetRenderMode(RenderMode);
}
void AeSysView::OnViewWindow() {
	CPoint CurrentPosition;
	::GetCursorPos(&CurrentPosition);
	HMENU WindowMenu = ::LoadMenu(theApp.GetInstance(), MAKEINTRESOURCE(IDR_WINDOW));
	CMenu* SubMenu = CMenu::FromHandle(::GetSubMenu(WindowMenu, 0));
	SubMenu->TrackPopupMenuEx(TPM_LEFTALIGN, CurrentPosition.x, CurrentPosition.y, AfxGetMainWnd(), 0);
	::DestroyMenu(WindowMenu);
}
void AeSysView::OnWindowZoomWindow() {
	m_Points.clear();
	m_ZoomWindow = !m_ZoomWindow;
}
void AeSysView::OnUpdateWindowZoomWindow(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_ZoomWindow);
}
void AeSysView::OnViewOdometer() {
	m_ViewOdometer = !m_ViewOdometer;
	InvalidateRect(nullptr);
}
void AeSysView::OnViewRefresh() {
	InvalidateRect(nullptr);
}
void AeSysView::OnUpdateViewRendermode2doptimized(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(RenderMode() == OdGsView::k2DOptimized ? MF_CHECKED : MF_UNCHECKED);
}
void AeSysView::OnUpdateViewRendermodeSmoothshaded(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(RenderMode() == OdGsView::kGouraudShaded ? MF_CHECKED : MF_UNCHECKED);
}
void AeSysView::OnUpdateViewRendermodeFlatshaded(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(RenderMode() == OdGsView::kFlatShaded ? MF_CHECKED : MF_UNCHECKED);
}
void AeSysView::OnUpdateViewRendermodeHiddenline(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(RenderMode() == OdGsView::kHiddenLine ? MF_CHECKED : MF_UNCHECKED);
}
void AeSysView::OnUpdateViewRendermodeWireframe(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(RenderMode() == OdGsView::kWireframe ? MF_CHECKED : MF_UNCHECKED);
}

void AeSysView::OnWindowNormal() {
	CopyActiveModelViewToPreviousModelView();
	DollyAndZoom(m_ViewTransform.FieldWidth() / m_Viewport.WidthInInches());
}
void AeSysView::OnWindowBest() {
	auto FirstView {m_LayoutHelper->viewAt(0)};
	auto AbstractView {OdAbstractViewPEPtr(FirstView)};

	AbstractView->zoomExtents(FirstView);

	m_PreviousViewTransform = m_ViewTransform;

	m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();

	SetCursorPosition(FirstView->target());
	InvalidateRect(nullptr);
}
void AeSysView::OnWindowLast() {
	ExchangeActiveAndPreviousModelViews();
	InvalidateRect(nullptr);
}
void AeSysView::OnWindowSheet() {
	ModelViewInitialize();
	InvalidateRect(nullptr);
}
void AeSysView::OnWindowZoomIn() {
	DollyAndZoom(1. / 0.9);
}
void AeSysView::OnWindowZoomOut() {
	DollyAndZoom(0.9);
}
void AeSysView::OnWindowPan() {
	Dolly();
	InvalidateRect(nullptr);
}
void AeSysView::OnWindowPanLeft() {
	auto FirstView {m_LayoutHelper->viewAt(0)};
	const double Delta = -1. / (m_Viewport.WidthInInches() / m_ViewTransform.FieldWidth());

	FirstView->dolly(OdGeVector3d(Delta, 0., 0.));

	m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
}
void AeSysView::OnWindowPanRight() {
	auto FirstView {m_LayoutHelper->viewAt(0)};
	const double Delta = 1. / (m_Viewport.WidthInInches() / m_ViewTransform.FieldWidth());

	FirstView->dolly(OdGeVector3d(Delta, 0., 0.));

	m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
}
void AeSysView::OnWindowPanUp() {
	auto FirstView {m_LayoutHelper->viewAt(0)};
	const double Delta = 1. / (m_Viewport.HeightInInches() / m_ViewTransform.FieldHeight());

	FirstView->dolly(OdGeVector3d(0., Delta, 0.));

	m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
}
void AeSysView::OnWindowPanDown() {
	auto FirstView {m_LayoutHelper->viewAt(0)};
	const double Delta = -1. / (m_Viewport.HeightInInches() / m_ViewTransform.FieldHeight());

	FirstView->dolly(OdGeVector3d(0., Delta, 0.));

	m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
}
void AeSysView::OnWindowZoomSpecial() {
	EoDlgViewZoom ViewZoomDialog(this);

	const double ZoomFactor(m_Viewport.WidthInInches() / m_ViewTransform.FieldWidth());
	ViewZoomDialog.m_ZoomFactor = ZoomFactor;

	if (ViewZoomDialog.DoModal() == IDOK) {
		CopyActiveModelViewToPreviousModelView();
		DollyAndZoom(ViewZoomDialog.m_ZoomFactor / ZoomFactor);
		InvalidateRect(nullptr);
	}
}
void AeSysView::OnSetupDimLength() {
	EoDlgSetLength SetLengthDialog;

	SetLengthDialog.m_Title = L"Set Dimension Length";
	SetLengthDialog.m_Length = theApp.DimensionLength();
	if (SetLengthDialog.DoModal() == IDOK) {
		theApp.SetDimensionLength(SetLengthDialog.m_Length);
		UpdateStateInformation(DimLen);
	}
}
void AeSysView::OnSetupDimAngle() {
	EoDlgSetAngle dlg;

	dlg.m_strTitle = L"Set Dimension Angle";
	dlg.m_dAngle = theApp.DimensionAngle();
	if (dlg.DoModal() == IDOK) {
		theApp.SetDimensionAngle(dlg.m_dAngle);
		UpdateStateInformation(DimAng);
	}
}
void AeSysView::OnSetupUnits() {
	EoDlgSetUnitsAndPrecision Dialog;
	Dialog.m_Units = theApp.GetUnits();
	Dialog.m_Precision = theApp.ArchitecturalUnitsFractionPrecision();

	if (Dialog.DoModal() == IDOK) {
		theApp.SetUnits(Dialog.m_Units);
		theApp.SetArchitecturalUnitsFractionPrecision(Dialog.m_Precision);
	}
}
void AeSysView::OnSetupConstraints() {
	EoDlgSetupConstraints Dialog(this);

	if (Dialog.DoModal() == IDOK) {
		UpdateStateInformation(All);
	}
}
void AeSysView::OnSetupMouseButtons() {
	EoDlgSetupCustomMouseCharacters Dialog;
	if (Dialog.DoModal() == IDOK) {
	}
}
void AeSysView::OnRelativeMovesEngDown() {
	const OdGePoint3d Origin = GetCursorPosition();
	OdGePoint3d ptSec = Origin;
	ptSec.y -= theApp.EngagedLength();
	ptSec.rotateBy(theApp.EngagedAngle(), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(ptSec);
}
void AeSysView::OnRelativeMovesEngDownRotate() {
	const OdGePoint3d Origin = GetCursorPosition();
	OdGePoint3d ptSec = Origin;
	ptSec.y -= theApp.EngagedLength();
	ptSec.rotateBy(theApp.EngagedAngle() + EoToRadian(theApp.DimensionAngle()), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(ptSec);
}
void AeSysView::OnRelativeMovesEngIn() {
	OdGePoint3d pt = GetCursorPosition();
	pt.z -= theApp.EngagedLength();
	SetCursorPosition(pt);
}
void AeSysView::OnRelativeMovesEngLeft() {
	const OdGePoint3d Origin = GetCursorPosition();
	OdGePoint3d ptSec = Origin;
	ptSec.x -= theApp.EngagedLength();
	ptSec.rotateBy(theApp.EngagedAngle(), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(ptSec);
}
void AeSysView::OnRelativeMovesEngLeftRotate() {
	const OdGePoint3d Origin = GetCursorPosition();
	OdGePoint3d ptSec = Origin;
	ptSec.x -= theApp.EngagedLength();
	ptSec.rotateBy(theApp.EngagedAngle() + EoToRadian(theApp.DimensionAngle()), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(ptSec);
}
void AeSysView::OnRelativeMovesEngOut() {
	OdGePoint3d pt = GetCursorPosition();
	pt.z += theApp.EngagedLength();
	SetCursorPosition(pt);
}
void AeSysView::OnRelativeMovesEngRight() {
	const OdGePoint3d Origin = GetCursorPosition();
	OdGePoint3d ptSec = Origin;
	ptSec.x += theApp.EngagedLength();
	ptSec.rotateBy(theApp.EngagedAngle(), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(ptSec);
}
void AeSysView::OnRelativeMovesEngRightRotate() {
	const OdGePoint3d Origin = GetCursorPosition();
	OdGePoint3d ptSec = Origin;
	ptSec.x += theApp.EngagedLength();
	ptSec.rotateBy(theApp.EngagedAngle() + EoToRadian(theApp.DimensionAngle()), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(ptSec);
}
void AeSysView::OnRelativeMovesEngUp() {
	const OdGePoint3d Origin = GetCursorPosition();
	OdGePoint3d ptSec = Origin;
	ptSec.y += theApp.EngagedLength();
	ptSec.rotateBy(theApp.EngagedAngle(), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(ptSec);
}
void AeSysView::OnRelativeMovesEngUpRotate() {
	const OdGePoint3d Origin = GetCursorPosition();
	OdGePoint3d ptSec = Origin;
	ptSec.y += theApp.EngagedLength();
	ptSec.rotateBy(theApp.EngagedAngle() + EoToRadian(theApp.DimensionAngle()), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(ptSec);
}
void AeSysView::OnRelativeMovesDown() {
	OdGePoint3d pt = GetCursorPosition();
	pt.y -= theApp.DimensionLength();
	SetCursorPosition(pt);
}
void AeSysView::OnRelativeMovesDownRotate() {
	const OdGePoint3d Origin = GetCursorPosition();
	OdGePoint3d ptSec = Origin;
	ptSec.y -= theApp.DimensionLength();
	ptSec.rotateBy(EoToRadian(theApp.DimensionAngle()), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(ptSec);
}
void AeSysView::OnRelativeMovesLeft() {
	OdGePoint3d pt = GetCursorPosition();
	pt.x -= theApp.DimensionLength();
	SetCursorPosition(pt);
}
void AeSysView::OnRelativeMovesLeftRotate() {
	const OdGePoint3d Origin = GetCursorPosition();
	OdGePoint3d ptSec = Origin;
	ptSec.x -= theApp.DimensionLength();
	ptSec.rotateBy(EoToRadian(theApp.DimensionAngle()), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(ptSec);
}
void AeSysView::OnRelativeMovesIn() {
	OdGePoint3d pt = GetCursorPosition();
	pt.z -= theApp.DimensionLength();
	SetCursorPosition(pt);
}
void AeSysView::OnRelativeMovesOut() {
	OdGePoint3d pt = GetCursorPosition();
	pt.z += theApp.DimensionLength();
	SetCursorPosition(pt);
}
void AeSysView::OnRelativeMovesRight() {
	OdGePoint3d pt = GetCursorPosition();
	pt.x += theApp.DimensionLength();
	SetCursorPosition(pt);
}
void AeSysView::OnRelativeMovesRightRotate() {
	const OdGePoint3d Origin = GetCursorPosition();
	OdGePoint3d ptSec = Origin;
	ptSec.x += theApp.DimensionLength();
	ptSec.rotateBy(EoToRadian(theApp.DimensionAngle()), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(ptSec);
}
void AeSysView::OnRelativeMovesUp() {
	OdGePoint3d pt = GetCursorPosition();
	pt.y += theApp.DimensionLength();
	SetCursorPosition(pt);
}
void AeSysView::OnRelativeMovesUpRotate() {
	const OdGePoint3d Origin = GetCursorPosition();
	OdGePoint3d ptSec = Origin;
	ptSec.y += theApp.DimensionLength();
	ptSec.rotateBy(EoToRadian(theApp.DimensionAngle()), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(ptSec);
}
void AeSysView::OnToolsPrimitiveSnapto() {
	const OdGePoint3d pt = GetCursorPosition();

	OdGePoint3d ptDet;

	if (GroupIsEngaged()) {
		const EoDbPrimitive* Primitive = m_EngagedPrimitive;

		EoGePoint4d ptView(pt, 1.);
		ModelViewTransformPoint(ptView);

		EoDbHatch::SetEdgeToEvaluate(EoDbHatch::Edge());
		EoDbPolyline::SetEdgeToEvaluate(EoDbPolyline::Edge());

		if (Primitive->SelectBy(ptView, this, ptDet)) {
			ptDet = Primitive->GoToNxtCtrlPt();
			m_ptDet = ptDet;

			Primitive->AddReportToMessageList(ptDet);
			SetCursorPosition(ptDet);
			return;
		}
	}
	if (SelectGroupAndPrimitive(pt) != nullptr) {
		ptDet = m_ptDet;
		m_EngagedPrimitive->AddReportToMessageList(ptDet);
		SetCursorPosition(ptDet);
	}
}
void AeSysView::OnPrimPerpJump() {
	OdGePoint3d CursorPosition = GetCursorPosition();

	if (SelectGroupAndPrimitive(CursorPosition) != nullptr) {
		if (m_EngagedPrimitive->Is(EoDb::kLinePrimitive)) {
			const EoDbLine* LinePrimLine = dynamic_cast<EoDbLine*>(m_EngagedPrimitive);
			CursorPosition = LinePrimLine->ProjPt_(m_ptCursorPosWorld);
			SetCursorPosition(CursorPosition);
		}
	}
}
void AeSysView::OnHelpKey() {
	::HtmlHelpW(AfxGetMainWnd()->GetSafeHwnd(), L"..\\AeSys\\hlp\\AeSys.chm::/menu_mode.htm", HH_DISPLAY_TOPIC, NULL);
}
AeSysView* AeSysView::GetActiveView(void) {
	const CMDIFrameWndEx* MDIFrameWnd = (CMDIFrameWndEx*) AfxGetMainWnd();

	if (MDIFrameWnd == NULL) {
		return NULL;
	}
	const CMDIChildWndEx* MDIChildWnd = DYNAMIC_DOWNCAST(CMDIChildWndEx, MDIFrameWnd->MDIGetActive());

	if (MDIChildWnd == NULL) {
		return NULL;
	}
	CView* View = MDIChildWnd->GetActiveView();

	if (!View->IsKindOf(RUNTIME_CLASS(AeSysView))) { // View is the wrong kind (this could occur with splitter windows, or additional views in a single document.
		return NULL;
	}
	return (AeSysView*) View;
}
void AeSysView::OnUpdateViewOdometer(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_ViewOdometer);
}
void AeSysView::DisplayOdometer() {
	const OdGePoint3d pt = GetCursorPosition();
	m_vRelPos = pt - GridOrigin();

	if (m_ViewOdometer) {
		const AeSysApp::Units Units = theApp.GetUnits();

		CString Position;
		Position += theApp.FormatLength(m_vRelPos.x, Units) + L", ";
		Position += theApp.FormatLength(m_vRelPos.y, Units) + L", ";
		Position += theApp.FormatLength(m_vRelPos.z, Units);

		if (m_RubberbandType == Lines) {
			EoGeLineSeg3d Line(m_RubberbandBeginPoint, pt);

			const double LineLength = Line.length();
			const double AngleInXYPlane = Line.AngleFromXAxis_xy();

			Position += L" [" + theApp.FormatLength(LineLength, Units) + L" @ " + theApp.FormatAngle(AngleInXYPlane) + L"]";
		}
		CMainFrame* MainFrame = (CMainFrame*) (AfxGetMainWnd());
		MainFrame->SetStatusPaneTextAt(nStatusInfo, Position);

#if	defined(LEGACY_ODOMETER)
		CDC* DeviceContext = GetDC();

		CFont* Font = (CFont*) DeviceContext->SelectStockObject(ANSI_VAR_FONT);
		UINT nTextAlign = DeviceContext->SetTextAlign(TA_LEFT | TA_TOP);
		COLORREF crText = DeviceContext->SetTextColor(AppGetTextCol());
		COLORREF crBk = DeviceContext->SetBkColor(~AppGetTextCol() & 0x00ffffff);

		CRect ClientRect;
		GetClientRect(&ClientRect);
		TEXTMETRIC tm;
		DeviceContext->GetTextMetrics(&tm);

		int iLeft = ClientRect.right - 16 * tm.tmAveCharWidth;

		CRect rc(iLeft, ClientRect.top, ClientRect.right, ClientRect.top + tm.tmHeight);
		DeviceContext->ExtTextOutW(rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE, &rc, theApp.FormatLength(m_vRelPos.x, Units), (UINT) Length.GetLength(), 0);

		rc.SetRect(iLeft, ClientRect.top + 1 * tm.tmHeight, ClientRect.right, ClientRect.top + 2 * tm.tmHeight);
		DeviceContext->ExtTextOutW(rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE, &rc, theApp.FormatLength(m_vRelPos.y, Units), (UINT) Length.GetLength(), 0);

		rc.SetRect(iLeft, ClientRect.top + 2 * tm.tmHeight, ClientRect.right, ClientRect.top + 3 * tm.tmHeight);
		DeviceContext->ExtTextOutW(rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE, &rc, theApp.FormatLength(m_vRelPos.z, Units), (UINT) Length.GetLength(), 0);

		DeviceContext->SetBkColor(crBk);
		DeviceContext->SetTextColor(crText);
		DeviceContext->SetTextAlign(nTextAlign);
		DeviceContext->SelectObject(Font);
		ReleaseDC(DeviceContext);
#endif // LEGACY_ODOMETER
	}
}
void AeSysView::OnUpdateViewTrueTypeFonts(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_ViewTrueTypeFonts);
}
void AeSysView::OnBackgroundImageLoad() {
	CFileDialog dlg(TRUE, L"bmp", L"*.bmp");
	dlg.m_ofn.lpstrTitle = L"Load Background Image";

	if (dlg.DoModal() == IDOK) {
		EoDbBitmapFile BitmapFile(dlg.GetPathName());

		BitmapFile.Load(dlg.GetPathName(), m_BackgroundImageBitmap, m_BackgroundImagePalette);
		m_ViewBackgroundImage = true;
		InvalidateRect(nullptr);
	}
}
void AeSysView::OnBackgroundImageRemove() {
	if ((HBITMAP) m_BackgroundImageBitmap != 0) {
		m_BackgroundImageBitmap.DeleteObject();
		m_BackgroundImagePalette.DeleteObject();
		m_ViewBackgroundImage = false;

		InvalidateRect(nullptr);
	}
}
void AeSysView::OnViewBackgroundImage() {
	m_ViewBackgroundImage = !m_ViewBackgroundImage;
	InvalidateRect(nullptr);
}
void AeSysView::OnUpdateViewBackgroundImage(CCmdUI * pCmdUI) {
	pCmdUI->Enable((HBITMAP) m_BackgroundImageBitmap != 0);
	pCmdUI->SetCheck(m_ViewBackgroundImage);
}
void AeSysView::OnUpdateBackgroundimageLoad(CCmdUI * pCmdUI) {
	pCmdUI->Enable((HBITMAP) m_BackgroundImageBitmap == 0);
}
void AeSysView::OnUpdateBackgroundimageRemove(CCmdUI * pCmdUI) {
	pCmdUI->Enable((HBITMAP) m_BackgroundImageBitmap != 0);
}
void AeSysView::OnUpdateViewPenwidths(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_ViewPenWidths);
}

EoDbGroup* AeSysView::RemoveLastVisibleGroup() {

	if (m_VisibleGroupList.IsEmpty()) {
		theApp.AddStringToMessageList(IDS_MSG_NO_DET_GROUPS_IN_VIEW);
		return nullptr;
	} else {
		return m_VisibleGroupList.RemoveTail();
	}
}

void AeSysView::DeleteLastGroup() {
	auto Group {RemoveLastVisibleGroup()};

	if (Group != nullptr) {
		auto Document {GetDocument()};

		Document->AnyLayerRemove(Group);

		if (Document->RemoveTrappedGroup(Group) != 0) { // Display it normal color so the erase xor will work
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
			UpdateStateInformation(TrapCount);
		}
		Document->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, Group);
		Document->DeletedGroupsAddTail(Group);
		theApp.AddStringToMessageList(IDS_MSG_GROUP_ADDED_TO_DEL_GROUPS);
	}
}

bool AeSysView::ViewTrueTypeFonts() noexcept {
	return m_ViewTrueTypeFonts;
}
void AeSysView::BreakAllPolylines() {
	POSITION Position = GetFirstVisibleGroupPosition();
	while (Position != 0) {
		EoDbGroup* Group = GetNextVisibleGroup(Position);
		Group->BreakPolylines();
	}
}
void AeSysView::BreakAllSegRefs() {
	POSITION Position = GetFirstVisibleGroupPosition();
	while (Position != 0) {
		EoDbGroup* Group = GetNextVisibleGroup(Position);
		Group->BreakSegRefs();
	}
}
bool AeSysView::PenWidthsOn() noexcept {
	return m_ViewPenWidths;
}
double AeSysView::WorldScale() const noexcept {
	return m_WorldScale;
}

void AeSysView::ResetView() noexcept {
	m_EngagedGroup = 0;
	m_EngagedPrimitive = 0;
}
EoDbGroup* AeSysView::SelSegAndPrimAtCtrlPt(const EoGePoint4d & point) {
	OdGePoint3d ControlPoint;

	m_EngagedGroup = 0;
	m_EngagedPrimitive = 0;

	EoGeMatrix3d TransformMatrix = ModelViewMatrix();
	TransformMatrix.invert();

	POSITION Position = GetFirstVisibleGroupPosition();
	while (Position != 0) {
		EoDbGroup* Group = GetNextVisibleGroup(Position);
		EoDbPrimitive* Primitive = Group->SelectControlPointBy(point, this, &ControlPoint);
		if (Primitive != 0) {
			m_ptDet = ControlPoint;
			m_ptDet.transformBy(TransformMatrix);
			m_EngagedGroup = Group;
			m_EngagedPrimitive = Primitive;
		}
	}
	return (m_EngagedGroup);
}
OdGePoint3d& AeSysView::DetPt() noexcept {
	return m_ptDet;
}
EoDbPrimitive*& AeSysView::EngagedPrimitive() noexcept {
	return m_EngagedPrimitive;
}
EoDbGroup*& AeSysView::EngagedGroup() noexcept {
	return m_EngagedGroup;
}

bool AeSysView::GroupIsEngaged() noexcept {
	return m_EngagedGroup != 0;
}
double AeSysView::SelectApertureSize() const noexcept {
	return m_SelectApertureSize;
}
EoDbGroup* AeSysView::SelectGroupAndPrimitive(const OdGePoint3d & point) {
	OdGePoint3d ptEng;

	m_EngagedGroup = 0;
	m_EngagedPrimitive = 0;

	EoGePoint4d ptView(point, 1.);
	ModelViewTransformPoint(ptView);

	EoGeMatrix3d TransformMatrix = ModelViewMatrix();
	TransformMatrix.invert();

	double dPicApert = m_SelectApertureSize;

	EoDbHatch::SetEdgeToEvaluate(0);
	EoDbPolyline::SetEdgeToEvaluate(0);

	POSITION Position = GetFirstVisibleGroupPosition();
	while (Position != 0) {
		EoDbGroup* Group = GetNextVisibleGroup(Position);
		EoDbPrimitive* Primitive = Group->SelPrimUsingPoint(ptView, this, dPicApert, ptEng);
		if (Primitive != 0) {
			m_ptDet = ptEng;
			m_ptDet.transformBy(TransformMatrix);
			m_EngagedGroup = Group;
			m_EngagedPrimitive = Primitive;
			return (Group);
		}
	}
	return nullptr;
}

pair<EoDbGroup*, EoDbEllipse*> AeSysView::SelectCircleUsingPoint(const OdGePoint3d & point, double tolerance) {
	auto GroupPosition {GetFirstVisibleGroupPosition()};
	while (GroupPosition != nullptr) {
		auto Group = GetNextVisibleGroup(GroupPosition);
		auto PrimitivePosition = Group->GetHeadPosition();
		while (PrimitivePosition != nullptr) {
			auto Primitive = Group->GetNext(PrimitivePosition);
			if (Primitive->Is(EoDb::kEllipsePrimitive)) {
				auto Arc = dynamic_cast<EoDbEllipse*>(Primitive);

				if (fabs(Arc->SweepAngle() - TWOPI) <= DBL_EPSILON && (Arc->MajorAxis().lengthSqrd() - Arc->MinorAxis().lengthSqrd()) <= DBL_EPSILON) {
					if (point.distanceTo(Arc->Center()) <= tolerance) {
						return {Group, Arc};
					}
				}
			}
		}
	}
	return {nullptr, nullptr};
}

pair<EoDbGroup*, EoDbLine*> AeSysView::SelectLineUsingPoint(const OdGePoint3d & point) {
	EoGePoint4d ptView(point, 1.);
	ModelViewTransformPoint(ptView);

	auto GroupPosition {GetFirstVisibleGroupPosition()};
	while (GroupPosition != nullptr) {
		auto Group = GetNextVisibleGroup(GroupPosition);
		auto PrimitivePosition = Group->GetHeadPosition();
		while (PrimitivePosition != nullptr) {
			auto Primitive = Group->GetNext(PrimitivePosition);
			if (Primitive->Is(EoDb::kLinePrimitive)) {
				OdGePoint3d PointOnLine;
				if (Primitive->SelectBy(ptView, this, PointOnLine)) {
					return {Group, dynamic_cast<EoDbLine*>(Primitive)};
				}
			}
		}
	}
	return {nullptr, nullptr};

}

EoDbText* AeSysView::SelectTextUsingPoint(const OdGePoint3d & pt) {
	EoGePoint4d ptView(pt, 1.);
	ModelViewTransformPoint(ptView);

	POSITION GroupPosition = GetFirstVisibleGroupPosition();
	while (GroupPosition != 0) {
		const EoDbGroup* Group = GetNextVisibleGroup(GroupPosition);
		POSITION PrimitivePosition = Group->GetHeadPosition();
		while (PrimitivePosition != 0) {
			EoDbPrimitive* Primitive = Group->GetNext(PrimitivePosition);
			if (Primitive->Is(EoDb::kTextPrimitive)) {
				OdGePoint3d ptProj;
				if (dynamic_cast<EoDbText*>(Primitive)->SelectBy(ptView, this, ptProj))
					return dynamic_cast<EoDbText*>(Primitive);
			}
		}
	}
	return 0;
}
void AeSysView::OnOp0() {
	switch (theApp.CurrentMode()) {
		case ID_MODE_PRIMITIVE_EDIT:
		case ID_MODE_GROUP_EDIT:
			OnEditModeOptions();
			break;
	}
}
void AeSysView::OnOp2() {
	switch (theApp.CurrentMode()) {
		case ID_MODE_PRIMITIVE_EDIT:
			DoEditPrimitiveTransform(ID_OP2);
			break;

		case ID_MODE_GROUP_EDIT:
			DoEditGroupTransform(ID_OP2);
			break;
	}
}
void AeSysView::OnOp3() {
	switch (theApp.CurrentMode()) {
		case ID_MODE_PRIMITIVE_EDIT:
			DoEditPrimitiveTransform(ID_OP3);
			break;

		case ID_MODE_GROUP_EDIT:
			DoEditGroupTransform(ID_OP3);
			break;
	}
}
void AeSysView::OnOp4() {
	switch (theApp.CurrentMode()) {
		case ID_MODE_PRIMITIVE_EDIT:
			theApp.LoadModeResources(theApp.PrimaryMode());
			GetDocument()->InitializeGroupAndPrimitiveEdit();
			break;

		case ID_MODE_GROUP_EDIT:
			theApp.LoadModeResources(theApp.PrimaryMode());
			GetDocument()->InitializeGroupAndPrimitiveEdit();
			break;
	}
}
void AeSysView::OnOp5() {
	switch (theApp.CurrentMode()) {
		case ID_MODE_PRIMITIVE_EDIT:
			DoEditPrimitiveCopy();
			break;

		case ID_MODE_GROUP_EDIT:
			DoEditGroupCopy();
			break;
	}
}
void AeSysView::OnOp6() {
	switch (theApp.CurrentMode()) {
		case ID_MODE_PRIMITIVE_EDIT:
			DoEditPrimitiveTransform(ID_OP6);
			break;

		case ID_MODE_GROUP_EDIT:
			DoEditGroupTransform(ID_OP6);
			break;
	}
}
void AeSysView::OnOp7() {
	switch (theApp.CurrentMode()) {
		case ID_MODE_PRIMITIVE_EDIT:
			DoEditPrimitiveTransform(ID_OP7);
			break;

		case ID_MODE_GROUP_EDIT:
			DoEditGroupTransform(ID_OP7);
			break;
	}
}
void AeSysView::OnOp8() {
	switch (theApp.CurrentMode()) {
		case ID_MODE_PRIMITIVE_EDIT:
			DoEditPrimitiveTransform(ID_OP8);
			break;

		case ID_MODE_GROUP_EDIT:
			DoEditGroupTransform(ID_OP8);
			break;
	}
}
void AeSysView::OnReturn() {
	switch (theApp.CurrentMode()) {
		case ID_MODE_PRIMITIVE_EDIT:
			theApp.LoadModeResources(theApp.PrimaryMode());
			InitializeGroupAndPrimitiveEdit();
			break;

		case ID_MODE_GROUP_EDIT:
			theApp.LoadModeResources(theApp.PrimaryMode());
			InitializeGroupAndPrimitiveEdit();
			break;

		case ID_MODE_PRIMITIVE_MEND:
			MendPrimitiveReturn();
			break;
	}
}
void AeSysView::OnEscape() {
	switch (theApp.CurrentMode()) {
		case ID_MODE_PRIMITIVE_EDIT:
			DoEditPrimitiveEscape();
			break;

		case ID_MODE_GROUP_EDIT:
			DoEditGroupEscape();
			break;

		case ID_MODE_PRIMITIVE_MEND:
			MendPrimitiveEscape();
			break;
	}
}
void AeSysView::OnFind(void) {
	CString FindComboText;
	VerifyFindString(((CMainFrame*) AfxGetMainWnd())->GetFindCombo(), FindComboText);

	if (!FindComboText.IsEmpty()) {
		ATLTRACE2(atlTraceGeneral, 1, L"AeSysView::OnFind() ComboText = %s\n", FindComboText);
	}
}
void AeSysView::VerifyFindString(CMFCToolBarComboBoxButton * findComboBox, CString & findText) {

	if (findComboBox == NULL) { return; }

	const BOOL IsLastCommandFromButton = CMFCToolBar::IsLastCommandFromButton(findComboBox);

	if (IsLastCommandFromButton) {
		findText = findComboBox->GetText();
	}
	CComboBox* ComboBox = findComboBox->GetComboBox();

	if (!findText.IsEmpty()) {
		const int Count = ComboBox->GetCount();
		int Position = 0;

		while (Position < Count) {
			CString LBText;
			ComboBox->GetLBText(Position, LBText);

			if (LBText.GetLength() == findText.GetLength()) {
				if (LBText == findText) {
					break;
				}
			}
			Position++;
		}
		if (Position < Count) { // Text need to move to initial position
			ComboBox->DeleteString(Position);
		}
		ComboBox->InsertString(0, findText);
		ComboBox->SetCurSel(0);

		if (!IsLastCommandFromButton) {
			findComboBox->SetText(findText);
		}
	}
}
void AeSysView::OnEditFind() {
	ATLTRACE2(atlTraceGeneral, 1, L"AeSysView::OnEditFind() - Entering\n");
}
// Disables rubberbanding.
void AeSysView::RubberBandingDisable() {
	if (m_RubberbandType != None) {
		CDC* DeviceContext = GetDC();
		const int DrawMode = DeviceContext->SetROP2(R2_XORPEN);
		CPen GreyPen(PS_SOLID, 0, RubberbandColor);
		CPen* Pen = DeviceContext->SelectObject(&GreyPen);

		if (m_RubberbandType == Lines) {
			DeviceContext->MoveTo(m_RubberbandLogicalBeginPoint);
			DeviceContext->LineTo(m_RubberbandLogicalEndPoint);
		} else if (m_RubberbandType == Rectangles) {
			CBrush* Brush = (CBrush*) DeviceContext->SelectStockObject(NULL_BRUSH);
			DeviceContext->Rectangle(m_RubberbandLogicalBeginPoint.x, m_RubberbandLogicalBeginPoint.y, m_RubberbandLogicalEndPoint.x, m_RubberbandLogicalEndPoint.y);
			DeviceContext->SelectObject(Brush);
		}
		DeviceContext->SelectObject(Pen);
		DeviceContext->SetROP2(DrawMode);
		ReleaseDC(DeviceContext);
		m_RubberbandType = None;
	}
}
void AeSysView::RubberBandingStartAtEnable(const OdGePoint3d & point, ERubs type) {
	EoGePoint4d ptView(point, 1.);

	ModelViewTransformPoint(ptView);

	if (ptView.IsInView()) {
		m_RubberbandBeginPoint = point;

		m_RubberbandLogicalBeginPoint = DoViewportProjection(ptView);
		m_RubberbandLogicalEndPoint = m_RubberbandLogicalBeginPoint;
	}
	m_RubberbandType = type;
}
OdGePoint3d AeSysView::GetCursorPosition() {
	CPoint CursorPosition;

	::GetCursorPos(&CursorPosition);
	ScreenToClient(&CursorPosition);

	const OdGePoint3d Position(double(CursorPosition.x), double(CursorPosition.y), m_ptCursorPosDev.z);
	if (Position != m_ptCursorPosDev) {
		m_ptCursorPosDev = Position;

		m_ptCursorPosWorld = DoViewportProjectionInverse(m_ptCursorPosDev);
		EoGeMatrix3d TransformMatrix = ModelViewMatrix();
		m_ptCursorPosWorld.transformBy(TransformMatrix.invert());
		m_ptCursorPosWorld = SnapPointToGrid(m_ptCursorPosWorld);
	}
	return (m_ptCursorPosWorld);
}
OdGePoint3d AeSysView::GetWorldCoordinates(CPoint point) {
	OdGsViewPtr FirstView = m_LayoutHelper->viewAt(0);
	OdGePoint3d WCSPoint(point.x, point.y, 0.);

	WCSPoint.transformBy((FirstView->screenMatrix() * FirstView->projectionMatrix()).inverse());
	WCSPoint.z = 0.;
	WCSPoint.transformBy(OdAbstractViewPEPtr(FirstView)->eyeToWorld(FirstView));
	return WCSPoint;
}
void AeSysView::SetCursorPosition(const OdGePoint3d & cursorPosition) {
	EoGePoint4d ptView(cursorPosition, 1.);
	ModelViewTransformPoint(ptView);

	if (!ptView.IsInView()) { // Redefine the view so position becomes camera target
		OdGsViewPtr FirstView = m_LayoutHelper->viewAt(0);
		const OdGeVector3d DollyVector(cursorPosition - FirstView->target());
		FirstView->dolly(DollyVector);

		m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
		m_ViewTransform.BuildTransformMatrix();

		InvalidateRect(nullptr);

		ptView = EoGePoint4d(cursorPosition, 1.);
		ModelViewTransformPoint(ptView);
	}
	CPoint CursorPosition = DoViewportProjection(ptView);
	m_ptCursorPosDev.x = CursorPosition.x;
	m_ptCursorPosDev.y = CursorPosition.y;
	m_ptCursorPosDev.z = ptView.z / ptView.W();
	m_ptCursorPosWorld = cursorPosition;

	ClientToScreen(&CursorPosition);
	::SetCursorPos(CursorPosition.x, CursorPosition.y);
}
void AeSysView::SetModeCursor(int mode) {
	OdUInt16 ResourceIdentifier;

	switch (mode) {
		case ID_MODE_ANNOTATE:
			ResourceIdentifier = IDR_ANNOTATE_MODE;
			break;

		case ID_MODE_CUT:
			ResourceIdentifier = IDR_CUT_MODE;
			break;

		case ID_MODE_DIMENSION:
			ResourceIdentifier = IDR_DIMENSION_MODE;
			break;

		case ID_MODE_DRAW:
			ResourceIdentifier = IDR_DRAW_MODE;
			break;

		case ID_MODE_LPD:
			ResourceIdentifier = IDR_LPD_MODE;
			break;

		case ID_MODE_PIPE:
			ResourceIdentifier = IDR_PIPE_MODE;
			break;

		case ID_MODE_POWER:
			ResourceIdentifier = IDR_POWER_MODE;
			break;

		case ID_MODE_DRAW2:
			ResourceIdentifier = IDR_DRAW2_MODE;
			break;

		case ID_MODE_EDIT:
			ResourceIdentifier = IDR_EDIT_MODE;
			break;

		case ID_MODE_FIXUP:
			ResourceIdentifier = IDR_FIXUP_MODE;
			break;

		case ID_MODE_NODAL:
			ResourceIdentifier = IDR_NODAL_MODE;
			break;

		case ID_MODE_NODALR:
			ResourceIdentifier = IDR_NODALR_MODE;
			break;

		case ID_MODE_TRAP:
			ResourceIdentifier = IDR_TRAP_MODE;
			break;

		case ID_MODE_TRAPR:
			ResourceIdentifier = IDR_TRAPR_MODE;
			break;

		default:
			::SetCursor((HCURSOR) ::LoadImage(HINSTANCE(NULL), IDC_CROSS, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE));
			return;
	}
	HCURSOR CursorHandle = (HCURSOR) ::LoadImage(theApp.GetInstance(), MAKEINTRESOURCE(ResourceIdentifier), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE);
	VERIFY(CursorHandle);
	::SetCursor(CursorHandle);
	::SetClassLongPtr(this->GetSafeHwnd(), GCLP_HCURSOR, (long) CursorHandle);
}
void AeSysView::SetWorldScale(const double scale) {
	if (scale > FLT_EPSILON) {
		m_WorldScale = scale;
		UpdateStateInformation(Scale);

		CMainFrame* MainFrame = (CMainFrame*) (AfxGetMainWnd());
		MainFrame->GetPropertiesPane().GetActiveViewScaleProperty().SetValue(m_WorldScale);
	}
}
void AeSysView::OnViewStateInformation() {
	m_ViewStateInformation = !m_ViewStateInformation;
	AeSysDoc::GetDoc()->UpdateAllViews(nullptr);
}
void AeSysView::OnUpdateViewStateinformation(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_ViewStateInformation);
}
void AeSysView::UpdateStateInformation(EStateInformationItem item) {
	if (m_ViewStateInformation) {
		AeSysDoc* Document = AeSysDoc::GetDoc();
		CDC* DeviceContext = GetDC();

		CFont* Font = (CFont*) DeviceContext->SelectStockObject(ANSI_VAR_FONT);
		const UINT nTextAlign = DeviceContext->SetTextAlign(TA_LEFT | TA_TOP);
		const COLORREF crText = DeviceContext->SetTextColor(AppGetTextCol());
		const COLORREF crBk = DeviceContext->SetBkColor(~AppGetTextCol() & 0x00ffffff);

		TEXTMETRIC tm;
		DeviceContext->GetTextMetrics(&tm);

		CRect ClientRect;
		GetClientRect(&ClientRect);

		CRect rc;

		wchar_t szBuf[32];

		if ((item & WorkCount) == WorkCount) {
			rc.SetRect(0, ClientRect.top, 8 * tm.tmAveCharWidth, ClientRect.top + tm.tmHeight);
			swprintf_s(szBuf, 32, L"%-4i", Document->NumberOfGroupsInWorkLayer() + Document->NumberOfGroupsInActiveLayers());
			DeviceContext->ExtTextOutW(rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE, &rc, szBuf, (UINT) wcslen(szBuf), 0);
		}
		if ((item & TrapCount) == TrapCount) {
			rc.SetRect(8 * tm.tmAveCharWidth, ClientRect.top, 16 * tm.tmAveCharWidth, ClientRect.top + tm.tmHeight);
			swprintf_s(szBuf, 32, L"%-4i", Document->TrapGroupCount());
			DeviceContext->ExtTextOutW(rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE, &rc, szBuf, (UINT) wcslen(szBuf), 0);
		}
		if ((item & Pen) == Pen) {
			rc.SetRect(16 * tm.tmAveCharWidth, ClientRect.top, 22 * tm.tmAveCharWidth, ClientRect.top + tm.tmHeight);
			swprintf_s(szBuf, 32, L"P%-4i", pstate.ColorIndex());
			DeviceContext->ExtTextOutW(rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE, &rc, szBuf, (UINT) wcslen(szBuf), 0);
		}
		if ((item & Line) == Line) {
			rc.SetRect(22 * tm.tmAveCharWidth, ClientRect.top, 28 * tm.tmAveCharWidth, ClientRect.top + tm.tmHeight);
			swprintf_s(szBuf, 32, L"L%-4i", pstate.LinetypeIndex());
			DeviceContext->ExtTextOutW(rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE, &rc, szBuf, (UINT) wcslen(szBuf), 0);
		}
		if ((item & TextHeight) == TextHeight) {
			const EoDbCharacterCellDefinition CharacterCellDefinition = pstate.CharacterCellDefinition();
			rc.SetRect(28 * tm.tmAveCharWidth, ClientRect.top, 38 * tm.tmAveCharWidth, ClientRect.top + tm.tmHeight);
			swprintf_s(szBuf, 32, L"T%-6.2f", CharacterCellDefinition.Height());
			DeviceContext->ExtTextOutW(rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE, &rc, szBuf, (UINT) wcslen(szBuf), 0);
		}
		if ((item & Scale) == Scale) {
			rc.SetRect(38 * tm.tmAveCharWidth, ClientRect.top, 48 * tm.tmAveCharWidth, ClientRect.top + tm.tmHeight);
			swprintf_s(szBuf, 32, L"1:%-6.2f", WorldScale());
			DeviceContext->ExtTextOutW(rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE, &rc, szBuf, (UINT) wcslen(szBuf), 0);
		}
		if ((item & WndRatio) == WndRatio) {
			rc.SetRect(48 * tm.tmAveCharWidth, ClientRect.top, 58 * tm.tmAveCharWidth, ClientRect.top + tm.tmHeight);
			CString ZoomFactorAsString;
			ZoomFactorAsString.Format(L"=%-8.3f", ZoomFactor());
			DeviceContext->ExtTextOutW(rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE, &rc, ZoomFactorAsString, (UINT) ZoomFactorAsString.GetLength(), 0);
		}
		if ((item & DimLen) == DimLen || (item & DimAng) == DimAng) {
			rc.SetRect(58 * tm.tmAveCharWidth, ClientRect.top, 90 * tm.tmAveCharWidth, ClientRect.top + tm.tmHeight);
			CString LengthAndAngle;
			LengthAndAngle += theApp.FormatLength(theApp.DimensionLength(), theApp.GetUnits());
			LengthAndAngle += L" @ ";
			LengthAndAngle += theApp.FormatAngle(EoToRadian(theApp.DimensionAngle()));
			DeviceContext->ExtTextOutW(rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE, &rc, LengthAndAngle, (UINT) LengthAndAngle.GetLength(), 0);
		}
		DeviceContext->SetBkColor(crBk);
		DeviceContext->SetTextColor(crText);
		DeviceContext->SetTextAlign(nTextAlign);
		DeviceContext->SelectObject(Font);
		ReleaseDC(DeviceContext);
	}
}

const ODCOLORREF* AeSysView::CurrentPalette() const {
	const ODCOLORREF* Color = odcmAcadPalette(m_Background);
	return Color;
}
void AeSysView::SetRenderMode(OdGsView::RenderMode renderMode) {
	OdGsViewPtr FirstView = m_LayoutHelper->viewAt(0);

	if (FirstView->mode() != renderMode) {
		FirstView->setMode(renderMode);

		if (FirstView->mode() != renderMode) {
			::MessageBoxW(0, L"Render mode is not supported by the current device", L"Teigha", MB_ICONWARNING);
		} else {
			m_ViewTransform.SetRenderMode(renderMode);
			InvalidateRect(nullptr);
		}
	}
}

void AeSysView::ZoomWindow(OdGePoint3d point1, OdGePoint3d point2) {
	OdGsViewPtr FirstView = m_LayoutHelper->viewAt(0);

	const OdGeMatrix3d WorldToEye = OdAbstractViewPEPtr(FirstView)->worldToEye(FirstView);
	point1.transformBy(WorldToEye);
	point2.transformBy(WorldToEye);
	OdGeVector3d Vector = point2 - point1;
	if (OdNonZero(Vector.x) && OdNonZero(Vector.y)) {
		OdGePoint3d NewPosition = point1 + Vector / 2.;
		Vector.x = fabs(Vector.x);
		Vector.y = fabs(Vector.y);
		FirstView->dolly(NewPosition.asVector());
		const double wf = FirstView->fieldWidth() / Vector.x;
		const double hf = FirstView->fieldHeight() / Vector.y;
		FirstView->zoom(odmin(wf, hf));
		InvalidateRect(nullptr);
	}
}
void AeSysView::OnInsertBlockreference() {
	// <tas="Just a placeholder for BlockReference. It works but position, scale & rotation need to be specified."</tas>
	AeSysDoc* Document = GetDocument();

	if (Document->BlockTableSize() > 0) {
		EoDlgBlockInsert Dialog(Document);
		Dialog.DoModal();
	}
}