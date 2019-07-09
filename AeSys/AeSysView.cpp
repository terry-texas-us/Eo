#include "stdafx.h"
#include <DbViewport.h>
#include <DbViewportTable.h>
#include <DbViewportTableRecord.h>
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "PrimState.h"
#include <AbstractViewPE.h>
#include <ColorMapping.h>
#include <DbAbstractViewportData.h>
#include <DbViewTable.h>
#include <DbViewTableRecord.h>
#include <DbGsManager.h>
#include <RxVariantValue.h>
#include <SaveState.h>
#include <DbPageController.h>
#include <DbIdMapping.h>
#include <BmpTilesGen.h>
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

template <class T>
struct EoRectangle {
	T left;
	T top;
	T right;
	T bottom;

	T Width() { return right - left; }

	T Height() { return top - bottom; }
};
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
using ODGSPALETTE = OdArray<ODCOLORREF, OdMemoryAllocator<ODCOLORREF> >;
const double AeSysView::mc_MaximumWindowRatio = 999.0;
const double AeSysView::mc_MinimumWindowRatio = 0.001;
unsigned AeSysView::ms_RedrawMessage = 0;
IMPLEMENT_DYNCREATE(AeSysView, CView)
#pragma warning(push)
#pragma warning(disable : 4191) // (level 3) 'operator': unsafe conversion from 'type_of_expression' to 'type_required'
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
		ON_UPDATE_COMMAND_UI(ID_BACKGROUNDIMAGE_LOAD, OnUpdateBackgroundImageLoad)
		ON_COMMAND(ID_BACKGROUNDIMAGE_REMOVE, OnBackgroundImageRemove)
		ON_UPDATE_COMMAND_UI(ID_BACKGROUNDIMAGE_REMOVE, OnUpdateBackgroundImageRemove)
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
		ON_REGISTERED_MESSAGE(ms_RedrawMessage, OnRedraw)
		ON_COMMAND(ID_HELP_KEY, OnHelpKey)
		ON_COMMAND(ID_MODE_GROUP_EDIT, OnModeGroupEdit)
		ON_COMMAND(ID_MODE_PRIMITIVE_EDIT, OnModePrimitiveEdit)
		ON_COMMAND(ID_MODE_PRIMITIVE_MEND, OnModePrimitiveMend)
		ON_COMMAND(ID_PRIM_PERPJUMP, OnPrimitivePerpendicularJump)
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
		ON_COMMAND(ID_TOOLS_PRIMITIVE_SNAPTO, OnToolsPrimitiveSnapTo)
		ON_COMMAND(ID_VIEW_BACKGROUNDIMAGE, OnViewBackgroundImage)
		ON_UPDATE_COMMAND_UI(ID_VIEW_BACKGROUNDIMAGE, OnUpdateViewBackgroundImage)
		ON_COMMAND(ID_VIEW_ODOMETER, OnViewOdometer)
		ON_UPDATE_COMMAND_UI(ID_VIEW_ODOMETER, OnUpdateViewOdometer)
		ON_COMMAND(ID_VIEW_PARAMETERS, OnViewParameters)
		ON_COMMAND(ID_VIEW_PENWIDTHS, OnViewPenWidths)
		ON_UPDATE_COMMAND_UI(ID_VIEW_PENWIDTHS, OnUpdateViewPenWidths)
		ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
		ON_COMMAND(ID_VIEW_STATEINFORMATION, OnViewStateInformation)
		ON_UPDATE_COMMAND_UI(ID_VIEW_STATEINFORMATION, OnUpdateViewStateInformation)
		ON_COMMAND(ID_VIEW_TRUETYPEFONTS, OnViewTrueTypeFonts)
		ON_UPDATE_COMMAND_UI(ID_VIEW_TRUETYPEFONTS, OnUpdateViewTrueTypeFonts)
		ON_COMMAND(ID_VIEW_WINDOW, OnViewWindow)
		ON_COMMAND_RANGE(ID_VIEW_RENDERMODE_2DOPTIMIZED, ID_VIEW_RENDERMODE_SMOOTHSHADED, &AeSysView::OnViewRenderMode)
		ON_UPDATE_COMMAND_UI(ID_VIEW_RENDERMODE_2DOPTIMIZED, &AeSysView::OnUpdateViewRenderMode2dOptimized)
		ON_UPDATE_COMMAND_UI(ID_VIEW_RENDERMODE_WIREFRAME, &AeSysView::OnUpdateViewRenderModeWireframe)
		ON_UPDATE_COMMAND_UI(ID_VIEW_RENDERMODE_HIDDENLINE, &AeSysView::OnUpdateViewRenderModeHiddenLine)
		ON_UPDATE_COMMAND_UI(ID_VIEW_RENDERMODE_FLATSHADED, &AeSysView::OnUpdateViewRenderModeFlatShaded)
		ON_UPDATE_COMMAND_UI(ID_VIEW_RENDERMODE_SMOOTHSHADED, &AeSysView::OnUpdateViewRenderModeSmoothShaded)
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
		ON_UPDATE_COMMAND_UI(ID_WINDOW_ZOOMWINDOW, OnUpdateWindowZoomWindow)
		ON_COMMAND(ID_WINDOW_ZOOMSPECIAL, &AeSysView::OnWindowZoomSpecial)
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
		ON_COMMAND(ID_TRAPR_MODE_REMOVE_ADD, &AeSysView::OnTrapRemoveModeRemoveAdd)
		ON_COMMAND(ID_TRAPR_MODE_POINT, &AeSysView::OnTrapRemoveModePoint)
		ON_COMMAND(ID_TRAPR_MODE_STITCH, &AeSysView::OnTrapRemoveModeStitch)
		ON_COMMAND(ID_TRAPR_MODE_FIELD, &AeSysView::OnTrapRemoveModeField)
		ON_COMMAND(ID_TRAPR_MODE_LAST, &AeSysView::OnTrapRemoveModeLast)
		ON_COMMAND(ID_TRAPR_MODE_ENGAGE, &AeSysView::OnTrapRemoveModeEngage)
		ON_COMMAND(ID_TRAPR_MODE_MENU, &AeSysView::OnTrapRemoveModeMenu)
		ON_COMMAND(ID_TRAPR_MODE_MODIFY, &AeSysView::OnTrapRemoveModeModify)
		ON_COMMAND(ID_TRAPR_MODE_ESCAPE, &AeSysView::OnTrapRemoveModeEscape)
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
		ON_COMMAND(ID_INSERT_BLOCKREFERENCE, &AeSysView::OnInsertBlockReference)
END_MESSAGE_MAP()
#pragma warning (pop)
AeSysView::AeSysView() noexcept {
	m_Background = g_ViewBackgroundColor;
	SetEditModeMirrorScaleFactors(-1.0, 1.0, 1.0);
	SetEditModeScaleFactors(2.0, 2.0, 2.0);
	SetEditModeRotationAngles(0.0, 0.0, 45.0);
	m_Viewport.SetDeviceWidthInPixels(theApp.DeviceWidthInPixels());
	m_Viewport.SetDeviceHeightInPixels(theApp.DeviceHeightInPixels());
	m_Viewport.SetDeviceWidthInInches(MillimetersToInches(theApp.DeviceWidthInMillimeters()));
	m_Viewport.SetDeviceHeightInInches(MillimetersToInches(theApp.DeviceHeightInMillimeters()));
}
#ifdef _DEBUG
void AeSysView::AssertValid() const {
	CView::AssertValid();
}

void AeSysView::Dump(CDumpContext& dc) const {
	CView::Dump(dc);
}

AeSysDoc* AeSysView::GetDocument() const {
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(AeSysDoc)));
	return dynamic_cast<AeSysDoc*>(m_pDocument);
}
#endif //_DEBUG
void AeSysView::exeCmd(const OdString& commandName) {
	GetDocument()->ExecuteCommand(commandName);
	PropagateLayoutActiveViewChanges(true);
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
			// <tas="background and grid display are obscured by this update."/>
			m_LayoutHelper->update();
		}
		Document->DisplayUniquePoints();
		UpdateStateInformation(kAll);
		ModeLineDisplay();
		ValidateRect(nullptr);
	} catch (CException* Exception) {
		Exception->Delete();
	}
}

void AeSysView::OnInitialUpdate() {
	SetClassLongW(GetSafeHwnd(), GCLP_HBRBACKGROUND, reinterpret_cast<long>(CreateSolidBrush(g_ViewBackgroundColor)));
	CView::OnInitialUpdate();
	auto Document {GetDocument()};
	OdDbDatabase* Database {Document->m_DatabasePtr};
	setDatabase(Database);
	m_hWindowDC = ::GetDC(m_hWnd);
	if (ms_RedrawMessage == 0U) {
		ms_RedrawMessage = RegisterWindowMessageW(L"AeSys::AeSysView::WM_REDRAW");
	}
	CreateDevice();
	if (m_LayoutHelper.isNull()) {
		GetParent()->PostMessageW(WM_CLOSE);
		return;
	}
	Document->SetVectorizer(this);
	m_Editor.Initialize(m_LayoutHelper, Document->CommandContext0());
	theApp.OnModeDraw();
}

bool AeSysView::regenAbort() const noexcept {
	return false;
}

LRESULT AeSysView::OnRedraw(WPARAM /*wParam*/, LPARAM /*lParam*/) {
	if (m_IncompleteRegenerate) { return 1; }
	m_IncompleteRegenerate = true;
	m_RegenerateAbort = false;
	auto MainFrame {dynamic_cast<CMainFrame*>(theApp.GetMainWnd())};
	if (!regenAbort()) {
		try {
			MainFrame->StartTimer();
			if (m_LayoutHelper.get() != nullptr) {
				SetViewportBorderProperties();
				m_LayoutHelper->update();
			}
			if (!regenAbort()) {
				MainFrame->StopTimer(m_PaintMode == kRegenerate ? L"Regen" : L"Redraw");
			}
		} catch (const OdError& Error) {
			theApp.ErrorMessageBox(L"Rendering aborted", Error);
			GetParent()->PostMessageW(WM_CLOSE);
		} catch (const UserBreak&) {
			theApp.ErrorMessageBox(L"Rendering aborted", OdError(eUserBreak));
			GetParent()->PostMessageW(WM_CLOSE);
		}
#ifndef _DEBUG
		catch (...) {
			theApp.reportError(L"Rendering aborted", OdError("Unknown exception is caught..."));
			GetParent()->PostMessageW(WM_CLOSE);
		}
#endif // _DEBUG
	}
	m_RegenerateAbort = false;
	m_IncompleteRegenerate = false;
	m_PaintMode = kRedraw;
	return 1;
}

void AeSysView::OnPaint() {
	/* <tas="Code section to enable when custom redraw message processing added">
		m_RegenerateAbort = true;

		PAINTSTRUCT PaintStruct;
		BeginPaint(&PaintStruct);
		EndPaint(&PaintStruct);

		MSG Message;
		while(::PeekMessageW(&Message, nullptr, g_nRedrawMSG, g_nRedrawMSG, PM_REMOVE)) {
		;
		}
		PostMessageW(g_nRedrawMSG);
		</tas> */
	CView::OnPaint();
}

BOOL AeSysView::OnEraseBkgnd(CDC* deviceContext) {
	// TODO: Add your message handler code here and/or call default
	return __super::OnEraseBkgnd(deviceContext);
}

void AeSysView::OnSize(const unsigned type, const int cx, const int cy) {
	if (cx != 0 && cy != 0) {
		if (m_LayoutHelper.isNull()) {
			__super::OnSize(type, cx, cy);
		} else {
			SetViewportSize(cx, cy);
			m_LayoutHelper->onSize(OdGsDCRect(0, cx, cy, 0));
			const auto Target {OdGePoint3d(m_ViewTransform.FieldWidth() / 2., m_ViewTransform.FieldHeight() / 2., 0.0)};
			const auto Position {Target + OdGeVector3d::kZAxis * m_ViewTransform.LensLength()};
			OdGsViewPtr FirstView = m_LayoutHelper->viewAt(0);
			FirstView->setView(Position, Target, OdGeVector3d::kYAxis, m_ViewTransform.FieldWidth(), m_ViewTransform.FieldHeight());
			m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
			m_ViewTransform.BuildTransformMatrix();
			m_OverviewViewTransform = m_ViewTransform;
		}
	}
}

void AeSysView::OnDestroy() {
	GetDocument()->OnCloseVectorizer(this);
	m_Editor.Uninitialize();
	DestroyDevice();
	m_pPrinterDevice.release();
	::ReleaseDC(m_hWnd, m_hWindowDC);
	m_hWindowDC = nullptr;
	CView::OnDestroy();
}

int AeSysView::OnCreate(const LPCREATESTRUCT createStructure) {
	if (CView::OnCreate(createStructure) == -1) { return -1; }
	return 0;
}

OdGsView* AeSysView::GetLayoutActiveView() {
	return m_LayoutHelper->activeView();
}

const OdGsView* AeSysView::GetLayoutActiveView() const {
	return m_LayoutHelper->activeView();
}

OdGsView* AeSysView::GetLayoutActiveTopView() {
	auto ActiveView {GetLayoutActiveView()};
	if (!getDatabase()->getTILEMODE()) {
		auto ActiveViewport {getDatabase()->activeViewportId().safeOpenObject()};
		OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);
		if (!AbstractViewportData.isNull() && AbstractViewportData->gsView(ActiveViewport) != nullptr) {
			ActiveView = AbstractViewportData->gsView(ActiveViewport);
		}
	}
	return ActiveView;
}

const OdGsView* AeSysView::GetLayoutActiveTopView() const {
	auto ActiveView {GetLayoutActiveView()};
	if (!getDatabase()->getTILEMODE()) {
		auto ActiveViewport {getDatabase()->activeViewportId().safeOpenObject()};
		OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);
		if (!AbstractViewportData.isNull() && AbstractViewportData->gsView(ActiveViewport) != nullptr) {
			ActiveView = AbstractViewportData->gsView(ActiveViewport);
		}
	}
	return ActiveView;
}

inline bool RequireAutoRegen(OdGsView* view) {
	auto Device {view->device()};
	if (Device == nullptr) { return false; }
	auto DeviceProperties = Device->properties();
	if (!DeviceProperties.isNull()) {
		if (DeviceProperties->has(L"RegenCoef")) {
			return OdRxVariantValue(DeviceProperties->getAt(L"RegenCoef"))->getDouble() > 1.0;
		}
	}
	return false;
}

void AeSysView::PropagateLayoutActiveViewChanges(bool /*forceAutoRegen*/) const {
	OdGsViewPtr View {GetLayoutActiveView()};
	OdGsClientViewInfo ClientViewInfo;
	View->clientViewInfo(ClientViewInfo);
	OdRxObjectPtr ViewportObject {OdDbObjectId(ClientViewInfo.viewportObjectId).openObject(OdDb::kForWrite)};
	OdAbstractViewPEPtr AbstractView(ViewportObject);
	if (!AbstractView.isNull()) {
		const auto Target(View->target());
		auto Direction {View->position() - Target};
		const auto UpVector {View->upVector()};
		const auto FieldWidth {View->fieldWidth()};
		const auto FieldHeight {View->fieldHeight()};
		const auto Perspective {View->isPerspective()};
		const auto LensLength {View->lensLength()};
		if (Direction.isZeroLength()) {
			Direction = View->viewingMatrix().inverse().getCsZAxis();
			if (Direction.isZeroLength()) {
				Direction = OdGeVector3d::kZAxis;
			} else {
				Direction.normalize();
			}
		}
		if (!AbstractView->target(ViewportObject).isEqualTo(Target) || !AbstractView->direction(ViewportObject).isEqualTo(Direction) || !AbstractView->upVector(ViewportObject).isEqualTo(UpVector) || !OdEqual(AbstractView->fieldWidth(ViewportObject), FieldWidth) || !OdEqual(AbstractView->fieldHeight(ViewportObject), FieldHeight) || AbstractView->isPerspective(ViewportObject) != Perspective || !OdEqual(AbstractView->lensLength(ViewportObject), LensLength)) {
			OdGeVector2d ViewOffset;
			if (AbstractView->direction(ViewportObject).isEqualTo(Direction) && AbstractView->upVector(ViewportObject).isEqualTo(UpVector) && !Perspective && !AbstractView->isPerspective(ViewportObject)) {
				const auto vecX {UpVector.crossProduct(Direction).normal()};
				ViewOffset = AbstractView->viewOffset(ViewportObject);
				const auto PreviousTarget {AbstractView->target(ViewportObject) - vecX * ViewOffset.x - UpVector * ViewOffset.y};
				ViewOffset.x = vecX.dotProduct(Target - PreviousTarget);
				ViewOffset.y = UpVector.dotProduct(Target - PreviousTarget);
			}
			AbstractView->setView(ViewportObject, Target, Direction, UpVector, FieldWidth, FieldHeight, Perspective, ViewOffset);
			AbstractView->setLensLength(ViewportObject, LensLength);
			// Auto regen
			if (!theApp.DisableAutoRegenerate() && RequireAutoRegen(View)) { const_cast<AeSysView*>(this)->OnViewerRegen(); }
		}
		OdDb::RenderMode RenderMode;
		switch (View->mode()) {
			case OdGsView::kWireframe:
				RenderMode = OdDb::kWireframe;
				break;
			case OdGsView::kHiddenLine:
				RenderMode = OdDb::kHiddenLine;
				break;
			case OdGsView::kFlatShaded:
				RenderMode = OdDb::kFlatShaded;
				break;
			case OdGsView::kGouraudShaded:
				RenderMode = OdDb::kGouraudShaded;
				break;
			case OdGsView::kFlatShadedWithWireframe:
				RenderMode = OdDb::kFlatShadedWithWireframe;
				break;
			case OdGsView::kGouraudShadedWithWireframe:
				RenderMode = OdDb::kGouraudShadedWithWireframe;
				break;
			case OdGsView::kNone: case OdGsView::k2DOptimized: case OdGsView::kBoundingBox: default:
				RenderMode = OdDb::k2DOptimized;
		}
		if (AbstractView->renderMode(ViewportObject) != RenderMode) { AbstractView->setRenderMode(ViewportObject, RenderMode); }
		const OdDbObjectId ObjectVisualStyle(View->visualStyle());
		if (AbstractView->visualStyle(ViewportObject) != ObjectVisualStyle && !ObjectVisualStyle.isNull()) { AbstractView->setVisualStyle(ViewportObject, View->visualStyle()); }
	}
}

inline OdGsViewPtr GetOverallView(OdGsDevice* device) {
	OdGsViewPtr OverallView;
	auto PaperLayoutHelper {OdGsPaperLayoutHelper::cast(device)};
	if (PaperLayoutHelper.get() != nullptr) {
		OverallView = PaperLayoutHelper->overallView();
	}
	return OverallView;
}

inline OdGsViewPtr activeView(OdGsDevice* device) {
	OdGsViewPtr ActiveView;
	auto LayoutHelper {OdGsLayoutHelper::cast(device)};
	if (LayoutHelper.get() != nullptr) { ActiveView = LayoutHelper->activeView(); }
	return ActiveView;
}

void AeSysView::SetViewportBorderProperties() {
	const auto OverallView {GetOverallView(m_LayoutHelper)};
	const auto ActiveView {activeView(m_LayoutHelper)};
	const auto NumberOfViews {m_LayoutHelper->numViews()};
	if (NumberOfViews > 1) {
		for (auto ViewIndex = 0; ViewIndex < NumberOfViews; ++ViewIndex) {
			OdGsViewPtr View {m_LayoutHelper->viewAt(ViewIndex)};

			// If the model layout is active, and it has more then one viewport then make their borders visible.
			// If a paper layout is active, then make visible the borders of all but the overall viewport.
			if (View == OverallView || OdGsPaperLayoutHelper::cast(m_LayoutHelper).get() && View != ActiveView) {
				View->setViewportBorderVisibility(false);
			} else if (View != ActiveView) {
				View->setViewportBorderVisibility(true);
				View->setViewportBorderProperties(theApp.CurrentPalette()[7], 2);
			} else {
				View->setViewportBorderVisibility(true);
				View->setViewportBorderProperties(theApp.CurrentPalette()[7], 2);
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

void AeSysView::plotStyle(OdDbStub* psNameId, OdPsPlotStyleData& plotStyleData) const {
	OdGiContextForDbDatabase::plotStyle(psNameId, plotStyleData);
	if (m_bPlotGrayscale) { // #4203 : make grayscale preview if printer doesn't support color mode
		plotStyleData.setColorPolicy(static_cast<unsigned>(plotStyleData.colorPolicy()) | 2U);
	}
}

void AeSysView::PreparePlotStyles(const OdDbLayout* layout, const bool forceReload) {
	if (m_pPlotStyleTable.get() != nullptr && !forceReload) { return; }
	const OdDbDatabase* Database {GetDocument()->m_DatabasePtr};
	OdSmartPtr<OdDbLayout> CurrentLayout;
	if (layout == nullptr) {
		OdDbBlockTableRecordPtr LayoutBlock {Database->getActiveLayoutBTRId().safeOpenObject()};
		CurrentLayout = LayoutBlock->getLayoutId().safeOpenObject();
		layout = CurrentLayout;
	}
	m_bPlotPlotstyle = layout->plotPlotStyles();
	m_bShowPlotstyle = layout->showPlotStyles();
	if (isPlotGeneration() ? m_bPlotPlotstyle : m_bShowPlotstyle) {
		const auto LayoutStyleSheet {layout->getCurrentStyleSheet()};
		if (!LayoutStyleSheet.isEmpty()) {
			const auto TestPath {Database->appServices()->findFile(LayoutStyleSheet)};
			if (!TestPath.isEmpty()) {
				auto FileBuffer {odSystemServices()->createFile(TestPath)};
				if (FileBuffer.get() != nullptr) { loadPlotStyleTable(FileBuffer); }
			}
		}
	}
}

CString GetRegistryAcadProfilesKey(); // external defined in AeSys
static bool GetRegistryUnsignedLong(const HKEY key, const wchar_t* subKey, const wchar_t* name, unsigned long& value) noexcept {
	auto ReturnValue {false};
	HKEY KeyHandle {nullptr};
	if (RegOpenKeyExW(key, subKey, 0, KEY_READ, &KeyHandle) == ERROR_SUCCESS) {
		unsigned long ValueSize {sizeof(unsigned long)};
		if (RegQueryValueExW(KeyHandle, name, nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &ValueSize) == ERROR_SUCCESS) { ReturnValue = true; }
		RegCloseKey(KeyHandle);
	}
	return ReturnValue;
}

static bool GetAcadProfileRegistryUnsignedLong(const wchar_t* subKey, const wchar_t* name, unsigned long& value) {
	auto SubKey {GetRegistryAcadProfilesKey()};
	if (!SubKey.IsEmpty()) {
		if (subKey != nullptr) {
			SubKey += L"\\";
			SubKey += subKey;
		}
		return GetRegistryUnsignedLong(HKEY_CURRENT_USER, SubKey, name, value);
	}
	return false;
}

unsigned long AeSysView::glyphSize(const GlyphType glyphType) const {
	auto Processed {false};
	unsigned long Value {0};
	switch (glyphType) {
		case kLightGlyph:
			Processed = GetAcadProfileRegistryUnsignedLong(L"Light", L"Glyph size", Value);
			break;
		case kCameraGlyph:
			Processed = GetAcadProfileRegistryUnsignedLong(L"Dialogs\\AcCamera", L"GlyphSize", Value);
			break;
	}
	if (Processed) { return gsl::narrow_cast<unsigned long>(Value); }
	return OdGiContextForDbDatabase::glyphSize(glyphType);
}

void AeSysView::fillContextualColors(OdGiContextualColorsImpl* pCtxColors) {
	OdGiContextForDbDatabase::fillContextualColors(pCtxColors); // Fill by defaults first
	unsigned long Value {0};
#define SET_CTXCLR_ISOK(entry, subKey, name) \
	if (GetAcadProfileRegistryUnsignedLong(subKey, name, Value)) { pCtxColors->setContextualColor(OdGiContextualColorsImpl::entry, (ODCOLORREF) Value); }
#define SET_CTXCLRTINT_ISOK(entry, subKey, name) \
	if (GetAcadProfileRegistryUnsignedLong(subKey, name, Value)) { pCtxColors->setContextualColorTint(OdGiContextualColorsImpl::entry, Value != 0); }
	switch (pCtxColors->visualType()) {
		case OdGiContextualColorsImpl::k2dModel: SET_CTXCLR_ISOK(kGridMajorLinesColor, L"Drawing Window", L"2D Model grid major lines color");
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
		case OdGiContextualColorsImpl::kLayout: SET_CTXCLR_ISOK(kGridMajorLinesColor, L"Drawing Window", L"Layout grid major lines color");
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
		case OdGiContextualColorsImpl::k3dParallel: SET_CTXCLR_ISOK(kGridMajorLinesColor, L"Drawing Window", L"Parallel grid major lines color");
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
		case OdGiContextualColorsImpl::k3dPerspective: SET_CTXCLR_ISOK(kGridMajorLinesColor, L"Drawing Window", L"Perspective grid major lines color");
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
		case OdGiContextualColorsImpl::kBlock: SET_CTXCLR_ISOK(kGridMajorLinesColor, L"Drawing Window", L"BEdit grid major lines color");
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
		case OdGiContextualColorsImpl::kVisualTypeNotSet: case OdGiContextualColorsImpl::kNumVisualTypes: default:
			break;
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
void AeSysView::CreateDevice(const bool recreate) {
	CRect ClientRectangle;
	GetClientRect(&ClientRectangle);
	try {
		OdArray<OdGsViewPtr> PreviousViews;
		OdGsModelPtr Model;
		if (!recreate) {
			OdGsModulePtr GsModule {odrxDynamicLinker()->loadModule(theApp.RecentGsDevicePath(), false)};
			auto GsDevice {GsModule->createDevice()};
			auto DeviceProperties {GsDevice->properties()};
			if (DeviceProperties.get() != nullptr) {
				if (DeviceProperties->has(L"WindowHWND")) {
					DeviceProperties->putAt("WindowHWND", OdRxVariantValue(reinterpret_cast<OdIntPtr>(m_hWnd)));
				}
				if (DeviceProperties->has(L"WindowHDC")) {
					DeviceProperties->putAt(L"WindowHDC", OdRxVariantValue(reinterpret_cast<OdIntPtr>(m_hWindowDC)));
				}
				if (DeviceProperties->has(L"DoubleBufferEnabled")) {
					DeviceProperties->putAt(L"DoubleBufferEnabled", OdRxVariantValue(theApp.DoubleBufferEnabled()));
				}
				if (DeviceProperties->has(L"EnableSoftwareHLR")) {
					DeviceProperties->putAt(L"EnableSoftwareHLR", OdRxVariantValue(theApp.UseSoftwareHlr()));
				}
				if (DeviceProperties->has(L"DiscardBackFaces")) {
					DeviceProperties->putAt(L"DiscardBackFaces", OdRxVariantValue(theApp.DiscardBackFaces()));
				}
				if (DeviceProperties->has(L"BlocksCache")) {
					DeviceProperties->putAt(L"BlocksCache", OdRxVariantValue(theApp.BlocksCacheEnabled()));
				}
				if (DeviceProperties->has(L"EnableMultithreading")) {
					DeviceProperties->putAt(L"EnableMultithreading", OdRxVariantValue(theApp.GsDeviceMultithreadingEnabled()));
				}
				if (DeviceProperties->has(L"MaxRegenerateThreads")) {
					DeviceProperties->putAt(L"MaxRegenerateThreads", OdRxVariantValue(static_cast<unsigned short>(theApp.MtRegenerateThreadsCount())));
				}
				if (DeviceProperties->has(L"UseTextOut")) {
					DeviceProperties->putAt(L"UseTextOut", OdRxVariantValue(theApp.EnableTtfTextOut()));
				}
				if (DeviceProperties->has(L"UseTTFCache")) {
					DeviceProperties->putAt(L"UseTTFCache", OdRxVariantValue(theApp.EnableTtfCache()));
				}
				if (DeviceProperties->has(L"DynamicSubEntHlt")) {
					DeviceProperties->putAt(L"DynamicSubEntHlt", OdRxVariantValue(theApp.EnableDynamicSubEntHlt()));
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
					DeviceProperties->putAt(L"BlendingMode", OdRxVariantValue(unsigned long(0)));
				}
				if (DeviceProperties->has(L"GradientsAsBitmap")) {
					DeviceProperties->putAt(L"GradientsAsBitmap", OdRxVariantValue(theApp.enableGDIGradientsAsBitmap()));
				}
			   </tas> */
			}
			enableKeepPSLayoutHelperView(true);
			enableContextualColorsManagement(theApp.EnableContextualColors());
			setTtfPolyDrawMode(theApp.EnableTtfPolyDraw());
			enableGsModel(theApp.UseGsModel());
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
				PreviousViews.append(LayoutHelperIn->viewAt(ViewIndex));
			}
			Model = LayoutHelperIn->gsModel();
			LayoutHelperIn->eraseAllViews();
			const auto LayoutHelperOut {OdDbGsManager::setupActiveLayoutViews(LayoutHelperIn->underlyingDevice(), this)};
			m_LayoutHelper = LayoutHelperOut;
			LayoutHelperIn.release();
			m_Editor.Initialize(m_LayoutHelper, GetDocument()->CommandContext0());
		}
		m_layoutId = m_LayoutHelper->layoutId();
		const auto Palette {theApp.CurrentPalette()};
		ODGSPALETTE PaletteCopy;
		PaletteCopy.insert(PaletteCopy.begin(), Palette, Palette + 256);
		PaletteCopy[0] = theApp.ActiveBackground();
		m_LayoutHelper->setLogicalPalette(PaletteCopy.asArrayPtr(), 256);
		auto PaperLayoutHelper {OdGsPaperLayoutHelper::cast(m_LayoutHelper)};
		if (PaperLayoutHelper.isNull()) {
			m_PsOverall = false;
			m_LayoutHelper->setBackgroundColor(PaletteCopy[0]); // for model space
		} else {
			m_PsOverall = PaperLayoutHelper->overallView().get() == PaperLayoutHelper->activeView().get();
			m_LayoutHelper->setBackgroundColor(ODRGB(173, 174, 173)); // ACAD's color for paper bg
		}
		setPaletteBackground(theApp.ActiveBackground());
		SetViewportBorderProperties();
		if (ClientRectangle.Width() != 0 && ClientRectangle.Height() != 0) {
			m_LayoutHelper->onSize(OdGsDCRect(ClientRectangle.left, ClientRectangle.right, ClientRectangle.bottom, ClientRectangle.top));
			OdGsViewPtr FirstView {m_LayoutHelper->viewAt(0)};
			SetViewportSize(ClientRectangle.Width(), ClientRectangle.Height());
			m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
			m_ViewTransform.BuildTransformMatrix();
			m_OverviewViewTransform = m_ViewTransform;
		}
		PreparePlotStyles(nullptr, recreate);
		if (recreate) {
			// Call update to share cache from exist views
			m_LayoutHelper->update();
			// Invalidate views for exist Gs model (i. e. remove unused drawables and mark view props as invalid)
			if (!Model.isNull()) {
				const auto Views {PreviousViews.asArrayPtr()};
				const auto NumberOfViews {PreviousViews.size()};
				for (unsigned ViewIndex = 0; ViewIndex < NumberOfViews; ViewIndex++) {
					Model->invalidate(Views[ViewIndex]);
				}
			}
			// Release exist views to detach from Gs and keep released slots free.
			PreviousViews.clear();
		}
	} catch (const OdError& Error) {
		DestroyDevice();
		theApp.ErrorMessageBox(L"Graphic System Initialization Error", Error);
	}
}

void AeSysView::DestroyDevice() {
	m_LayoutHelper.release();
}

void AeSysView::OnBeginPrinting(CDC* deviceContext, CPrintInfo* printInformation) {
	ViewportPushActive();
	PushViewTransform();
	const auto HorizontalPixelWidth {deviceContext->GetDeviceCaps(HORZRES)};
	const auto VerticalPixelWidth {deviceContext->GetDeviceCaps(VERTRES)};
	SetViewportSize(HorizontalPixelWidth, VerticalPixelWidth);
	const auto HorizontalSize {static_cast<double>(deviceContext->GetDeviceCaps(HORZSIZE))};
	const auto VerticalSize {static_cast<double>(deviceContext->GetDeviceCaps(VERTSIZE))};
	SetDeviceWidthInInches(MillimetersToInches(HorizontalSize));
	SetDeviceHeightInInches(MillimetersToInches(VerticalSize));
	if (m_Plot) {
		unsigned HorizontalPages;
		unsigned VerticalPages;
		printInformation->SetMaxPage(NumPages(deviceContext, m_PlotScaleFactor, HorizontalPages, VerticalPages));
	} else {
		m_ViewTransform.AdjustWindow(static_cast<double>(VerticalPixelWidth) / static_cast<double>(HorizontalPixelWidth));
		m_ViewTransform.BuildTransformMatrix();
	}
}

void GenerateTiles(const HDC hdc, const RECT& drawRectangle, OdGsDevice* pBmpDevice, const long tileWidth, const long tileHeight) {
	CRect DestinationRectangle {drawRectangle};
	DestinationRectangle.NormalizeRect();
	OdGsDCRect Step(0, 0, 0, 0);
	OdGsDCRect Rectangle(drawRectangle.left, drawRectangle.right, drawRectangle.bottom, drawRectangle.top);
	const auto Width {abs(Rectangle.m_max.x - Rectangle.m_min.x)};
	Rectangle.m_max.x -= Rectangle.m_min.x;
	if (Rectangle.m_max.x < 0) {
		Rectangle.m_min.x = -Rectangle.m_max.x;
		Rectangle.m_max.x = 0;
		Step.m_min.x = tileWidth;
	} else {
		Rectangle.m_min.x = 0;
		Step.m_max.x = tileWidth;
	}
	const auto Height {abs(Rectangle.m_max.y - Rectangle.m_min.y)};
	Rectangle.m_max.y -= Rectangle.m_min.y;
	if (Rectangle.m_max.y < 0) {
		Rectangle.m_min.y = -Rectangle.m_max.y;
		Rectangle.m_max.y = 0;
		Step.m_min.y = tileHeight;
	} else {
		Rectangle.m_min.y = 0;
		Step.m_max.y = tileHeight;
	}
	const auto m {Width / tileWidth + (Width % tileWidth != 0 ? 1 : 0)};
	const auto n {Height / tileHeight + (Height % tileHeight != 0 ? 1 : 0)};
	BmpTilesGen tilesGen(pBmpDevice, Rectangle);
	pBmpDevice->onSize(Rectangle);
	const int dx {(Step.m_max.x - Step.m_min.x)};
	const int dy {(Step.m_max.y - Step.m_min.y)};
	const auto dx2 {m > 1 ? dx / abs(dx) * 8 : 0};
	const auto dy2 {n > 1 ? dy / abs(dy) * 8 : 0};
	BITMAPINFO BitmapInfo;
	BitmapInfo.bmiHeader.biBitCount = 24U;
	BitmapInfo.bmiHeader.biWidth = tileWidth + abs(dx2) * 2;
	BitmapInfo.bmiHeader.biHeight = tileHeight;
	BitmapInfo.bmiHeader.biClrImportant = 0;
	BitmapInfo.bmiHeader.biClrUsed = 0;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	BitmapInfo.bmiHeader.biSizeImage = 0;
	BitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	BitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	const auto MemoryDeviceContext = CreateCompatibleDC(hdc);
	if (MemoryDeviceContext != nullptr) {
		void* pBuf;
		const auto hBmp {CreateDIBSection(nullptr, &BitmapInfo, DIB_RGB_COLORS, &pBuf, nullptr, 0)};
		if (hBmp != nullptr) {
			const auto hOld {static_cast<HBITMAP>(SelectObject(MemoryDeviceContext, hBmp))};
			for (long i = 0; i < m; ++i) {
				for (long j = 0; j < n; ++j) {
					const int minx {Rectangle.m_min.x + i * dx};
					const auto maxx {minx + dx};
					const int miny {Rectangle.m_min.y + j * dy};
					const auto maxy {miny + dy};

					// render wider then a tile area to reduce gaps in lines.
					auto RasterImage {tilesGen.regenTile(OdGsDCRect(minx - dx2, maxx + dx2, miny - dy2, maxy + dy2))};
					RasterImage->scanLines(static_cast<unsigned char*>(pBuf), 0, static_cast<unsigned long>(tileHeight));
					BitBlt(hdc, DestinationRectangle.left + odmin(minx, maxx), DestinationRectangle.top + odmin(miny, maxy), tileWidth, tileHeight, MemoryDeviceContext, abs(dx2), 0, SRCCOPY);
				}
			}
			SelectObject(MemoryDeviceContext, hOld);
			DeleteObject(hBmp);
		}
		DeleteDC(MemoryDeviceContext);
	}
}

void AeSysView::OnPrint(CDC* deviceContext, CPrintInfo* printInformation) {
	const auto Database {getDatabase()};
	auto ActiveViewport {Database->activeViewportId().safeOpenObject(OdDb::kForWrite)};
	OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);
	const auto View {GetLayoutActiveView()};
	if (View != nullptr) {
		AbstractViewportData->setView(ActiveViewport, View);
	}
	setPlotGeneration(true);
	class KeepPreviousGiContextParameters {
		OdGiContextForDbDatabase* m_GiContext;
		bool m_PreviousGsModelState;
		ODCOLORREF m_PreviousBackgroundColor;
	public:
		explicit KeepPreviousGiContextParameters(OdGiContextForDbDatabase* giContext)
			: m_GiContext(giContext) {
			m_PreviousGsModelState = giContext->useGsModel();
			m_PreviousBackgroundColor = giContext->paletteBackground();
		}

		~KeepPreviousGiContextParameters() {
			m_GiContext->enableGsModel(m_PreviousGsModelState);
			m_GiContext->setPaletteBackground(m_PreviousBackgroundColor);
		}
	} PreviousGiContextParameters(this);

	// To get paper size of selected printer and to get properties (scale, offset) from PlotSettings to set using them OverAll View;
	//  Note: if we want to get the same Plot View for Paper Layout as AutoCAD then Iterator needs to create pseudo DC having the requisite settings & properties.
	//        Look at OnPreparePrinting() where we try to set required printer device.
	//        Otherwise CPreviewView uses settings and properties of current Printer/plotter (see CPreviewView::OnDraw()) to draw empty page on screen.
	auto IsPlotViaBitmap {AfxGetApp()->GetProfileIntW(L"options", L"Print/Preview via bitmap device", 1) != 0};
	if (m_pPrinterDevice.isNull()) {
		OdGsModulePtr GsModule = odrxDynamicLinker()->loadModule(theApp.RecentGsDevicePath());
		if (!IsPlotViaBitmap && GsModule.isNull()) {
			GsModule = odrxDynamicLinker()->loadModule(OdWinOpenGLModuleName);
		}
		OdGsDevicePtr GsPrinterDevice;
		if (IsPlotViaBitmap && GsModule.get() != nullptr) {
			GsPrinterDevice = GsModule->createBitmapDevice();
		} else {
			IsPlotViaBitmap = false;
			GsModule = odrxDynamicLinker()->loadModule(OdWinGDIModuleName);
			if (GsModule.get() != nullptr) {
				GsPrinterDevice = GsModule->createDevice();
			}
		}
		if (GsPrinterDevice.get() != nullptr) {
			enableGsModel(true);
			if (!GsPrinterDevice->properties().isNull() && GsPrinterDevice->properties()->has(L"EnableSoftwareHLR")) {
				GsPrinterDevice->properties()->putAt(L"EnableSoftwareHLR", OdRxVariantValue(theApp.UseSoftwareHlr()));
			}
			if (/*IsPlotViaBitmap &&*/ GsPrinterDevice->properties()->has(L"DPI")) { // #9633 (1)
				const auto MinimumLogicalPixels {odmin(deviceContext->GetDeviceCaps(LOGPIXELSX), deviceContext->GetDeviceCaps(LOGPIXELSY))};
				GsPrinterDevice->properties()->putAt(L"DPI", OdRxVariantValue(static_cast<unsigned long>(MinimumLogicalPixels)));
			}
			m_pPrinterDevice = OdDbGsManager::setupActiveLayoutViews(GsPrinterDevice, this);
			PreparePlotStyles();
			m_pPrinterDevice->setLogicalPalette(odcmAcadLightPalette(), 256);
			m_pPrinterDevice->setBackgroundColor(ODRGB(255, 255, 255));
			setPaletteBackground(m_pPrinterDevice->getBackgroundColor());
		}
	} else {
		IsPlotViaBitmap = m_pPrinterDevice->properties()->has(L"RasterImage");
		setPaletteBackground(m_pPrinterDevice->getBackgroundColor());
	}
	if (m_pPrinterDevice.get() != nullptr) {
		auto IsPrint90Degrees(false);
		auto IsPrint0Degrees(false);
		auto IsPrint180Degrees(false);
		auto IsPrint270Degrees(false);
		double PrinterWidth = deviceContext->GetDeviceCaps(PHYSICALWIDTH);
		if (printInformation->m_bPreview != 0) { PrinterWidth -= 2; }
		const auto PrinterHeight {static_cast<double>(deviceContext->GetDeviceCaps(PHYSICALHEIGHT))};
		const auto PrinterLeftMargin {static_cast<double>(deviceContext->GetDeviceCaps(PHYSICALOFFSETX))};
		const auto PrinterTopMargin {static_cast<double>(deviceContext->GetDeviceCaps(PHYSICALOFFSETY))};
		const auto PrinterMarginWidth {static_cast<double>(deviceContext->GetDeviceCaps(HORZRES))};
		const auto PrinterMarginHeight {static_cast<double>(deviceContext->GetDeviceCaps(VERTRES))};
		const auto LogicalPixelsX {static_cast<double>(deviceContext->GetDeviceCaps(LOGPIXELSX))};
		const auto LogicalPixelsY {static_cast<double>(deviceContext->GetDeviceCaps(LOGPIXELSY))};
		// const auto PrinterRightMargin {PrinterWidth - PrinterMarginWidth - PrinterLeftMargin};
		const auto PrinterBottomMargin {PrinterHeight - PrinterMarginHeight - PrinterTopMargin};
		struct {
			double x;
			double y;
		} Coefficient {MillimetersToInches(LogicalPixelsX), MillimetersToInches(LogicalPixelsY)};
		const auto IsModelLayout {m_pPrinterDevice->isKindOf(OdGsModelLayoutHelper::desc())};
		OdSmartPtr<OdDbLayout> Layout {m_pPrinterDevice->layoutId().safeOpenObject()};
		auto IsScaledToFit {Layout->useStandardScale() && OdDbPlotSettings::kScaleToFit == Layout->stdScaleType()};
		auto IsCentered {Layout->plotCentered()};
		const auto IsMetric {Layout->plotPaperUnits() != OdDbPlotSettings::kInches};
		const auto IsPrintLineweights {Layout->printLineweights() || Layout->showPlotStyles()};
		struct {
			double x;
			double y;
		} Offset {0.0, 0.0};
		Layout->getPlotOrigin(Offset.x, Offset.y); // in mm
		auto PaperImageOrigin {Layout->getPaperImageOrigin()}; // in mm
		auto LeftMargin {Layout->getLeftMargin()}; // in mm
		auto RightMargin {Layout->getRightMargin()}; // in mm
		auto TopMargin {Layout->getTopMargin()}; // in mm
		auto BottomMargin {Layout->getBottomMargin()}; // in mm
		const auto PlotType {Layout->plotType()};
		if (IsPrintLineweights && IsModelLayout) { // set LineWeight scale factor for model space
			auto ViewAt {m_pPrinterDevice->viewAt(0)};
			ViewAt->setLineweightToDcScale(MillimetersToInches(odmax(LogicalPixelsX, LogicalPixelsY)) * 0.01);
		}
		if (printInformation->m_bPreview != 0) { // Apply paper rotation to paper parameters
			switch (Layout->plotRotation()) {
				case OdDbPlotSettings::k0degrees:
					break;
				case OdDbPlotSettings::k90degrees: {
					const auto TemporaryMargin {TopMargin};
					TopMargin = RightMargin;
					RightMargin = BottomMargin;
					BottomMargin = LeftMargin;
					LeftMargin = TemporaryMargin;
					std::swap(Offset.x, Offset.y);
					break;
				}
				case OdDbPlotSettings::k180degrees:
					std::swap(BottomMargin, TopMargin);
					std::swap(RightMargin, LeftMargin);
					break;
				case OdDbPlotSettings::k270degrees: {
					const auto TemporaryMargin {TopMargin};
					TopMargin = LeftMargin;
					LeftMargin = BottomMargin;
					BottomMargin = RightMargin;
					RightMargin = TemporaryMargin;
					std::swap(Offset.x, Offset.y);
					break;
				}
			}
		} else { // printing
			auto PlotRotation {Layout->plotRotation()};
			if (PlotRotation == OdDbPlotSettings::k90degrees || PlotRotation == OdDbPlotSettings::k270degrees) {
				auto LandOrientation {DeviceCapabilitiesW(printInformation->m_pPD->GetDeviceName(), printInformation->m_pPD->GetPortName(), DC_ORIENTATION, nullptr, nullptr)};
				if (LandOrientation == 270) {
					PlotRotation = PlotRotation == OdDbPlotSettings::k90degrees ? OdDbPlotSettings::k270degrees : OdDbPlotSettings::k90degrees;
				}
			}
			switch (PlotRotation) {
				case OdDbPlotSettings::k0degrees:
					IsPrint0Degrees = true;
					break;
				case OdDbPlotSettings::k90degrees:
					IsPrint90Degrees = true;
					std::swap(TopMargin, RightMargin);
					std::swap(BottomMargin, LeftMargin);
					std::swap(BottomMargin, TopMargin);
					std::swap(Offset.x, Offset.y);
					Offset.y = -Offset.y;
					Offset.x = -Offset.x;
					break;
				case OdDbPlotSettings::k180degrees:
					IsPrint180Degrees = true;
					std::swap(RightMargin, LeftMargin);
					Offset.y = -Offset.y;
					Offset.x = -Offset.x;
					break;
				case OdDbPlotSettings::k270degrees:
					IsPrint270Degrees = true;
					std::swap(TopMargin, RightMargin);
					std::swap(BottomMargin, LeftMargin);
					std::swap(Offset.x, Offset.y);
					break;
			}
		}
		auto ScaleFactor {1.0};
		if (Layout->useStandardScale()) {
			Layout->getStdScale(ScaleFactor);
		} else {
			double Numerator;
			double Denominator;
			Layout->getCustomPrintScale(Numerator, Denominator);
			ScaleFactor = Numerator / Denominator;
		}
		EoRectangle<double> DrawableArea {}; // Paper drawable area using margins from layout (in mm)
		DrawableArea.left = -PrinterLeftMargin / Coefficient.x + LeftMargin;
		DrawableArea.top = -PrinterTopMargin / Coefficient.y + TopMargin;
		DrawableArea.right = DrawableArea.left + PrinterWidth / Coefficient.x - LeftMargin - RightMargin;
		DrawableArea.bottom = DrawableArea.top + PrinterHeight / Coefficient.y - TopMargin - BottomMargin;
		
		// Margin clip box calculation
		TopMargin *= Coefficient.y; /// in printer units
		RightMargin *= Coefficient.x;
		BottomMargin *= Coefficient.y;
		LeftMargin *= Coefficient.x;
		CRgn NewClipRegion;
		NewClipRegion.CreateRectRgn(0, 0, 1, 1);
		CRect MarginsClipBox;
		const auto ReturnValue {GetClipRgn(deviceContext->m_hDC, NewClipRegion)};
		const auto NullMarginsClipBox {ReturnValue == 0 || ReturnValue != 0 && GetLastError() != ERROR_SUCCESS};
		auto HeightScreenFactor {1.0};
		auto WidthScreenFactor {1.0};
		if (NullMarginsClipBox) { // printing way
			const auto X {LeftMargin - PrinterLeftMargin};
			const auto Y {TopMargin - PrinterTopMargin};
			MarginsClipBox.SetRect(static_cast<int>(X), static_cast<int>(Y), static_cast<int>(X + PrinterWidth - LeftMargin - RightMargin), static_cast<int>(Y + PrinterHeight - TopMargin - BottomMargin));
			HeightScreenFactor = WidthScreenFactor = 1.0;
		} else {
			NewClipRegion.GetRgnBox(&MarginsClipBox);
			HeightScreenFactor = static_cast<double>(MarginsClipBox.Height()) / PrinterMarginHeight;
			WidthScreenFactor = static_cast<double>(MarginsClipBox.Width()) / PrinterMarginWidth;
			MarginsClipBox.left += static_cast<long>((LeftMargin - PrinterLeftMargin) * WidthScreenFactor);
			MarginsClipBox.bottom -= static_cast<long>((BottomMargin - PrinterBottomMargin) * HeightScreenFactor);
			MarginsClipBox.top = MarginsClipBox.bottom - static_cast<long>((PrinterHeight - TopMargin - BottomMargin) * HeightScreenFactor);
			MarginsClipBox.right = MarginsClipBox.left + static_cast<long>((PrinterWidth - LeftMargin - RightMargin) * WidthScreenFactor);
		}
		// MarginsClipBox is calculated
		auto ClipBox(MarginsClipBox);
		OdGePoint3d ViewTarget;
		OdAbstractViewPEPtr AbstractView;
		OdRxObjectPtr ViewObject;
		auto OverallView {IsModelLayout ? OdGsModelLayoutHelperPtr(m_pPrinterDevice)->activeView() : OdGsPaperLayoutHelperPtr(m_pPrinterDevice)->overallView()};
		if (PlotType == OdDbPlotSettings::kView) {
			auto PlotViewName {Layout->getPlotViewName()};
			OdDbViewTableRecordPtr ViewTableRecord {static_cast<OdDbViewTablePtr>(Database->getViewTableId().safeOpenObject())->getAt(PlotViewName).safeOpenObject()};
			ViewTarget = ViewTableRecord->target(); // in plotPaperUnits
			AbstractView = OdAbstractViewPEPtr(ViewObject = ViewTableRecord);
		} else if (IsModelLayout) {
			OdDbViewportTablePtr ViewportTable {Database->getViewportTableId().safeOpenObject()};
			OdDbViewportTableRecordPtr ActiveViewport {ViewportTable->getActiveViewportId().safeOpenObject()};
			ViewTarget = ActiveViewport->target(); // in plotPaperUnits
			AbstractView = OdAbstractViewPEPtr(ViewObject = ActiveViewport);
		} else {
			const auto OverallViewportId {Layout->overallVportId()};
			OdDbViewportPtr pActiveVP = OverallViewportId.safeOpenObject();
			ViewTarget = pActiveVP->viewTarget(); // in plotPaperUnits
			AbstractView = OdAbstractViewPEPtr(ViewObject = pActiveVP);
		}
		const auto ViewportCenter {AbstractView->target(ViewObject)}; // in plotPaperUnits
		const auto IsPerspective {AbstractView->isPerspective(ViewObject)};
		const auto ViewportHeight {AbstractView->fieldHeight(ViewObject)}; // in plotPaperUnits
		const auto ViewportWidth {AbstractView->fieldWidth(ViewObject)}; // in plotPaperUnits
		const auto ViewDirection {AbstractView->direction(ViewObject)};
		const auto ViewUpVector {AbstractView->upVector(ViewObject)};
		const auto EyeToWorldTransform {AbstractView->eyeToWorld(ViewObject)};
		const auto WorldToEyeTransform {AbstractView->worldToEye(ViewObject)};
		auto SkipClipping {false};
		const auto IsPlanView {/*ViewTarget.isEqualTo(OdGePoint3d(0, 0, 0)) &&*/ ViewDirection.normal().isEqualTo(OdGeVector3d::kZAxis)};
		const auto OldTarget = ViewTarget;
		auto FieldWidth(ViewportWidth);
		auto FieldHeight(ViewportHeight);
		if (PlotType == OdDbPlotSettings::kWindow || PlotType == OdDbPlotSettings::kLimits && IsPlanView) {
			struct {
				double x {0.0};
				double y {0.0};
			} PlotWindowAreaMin;
			struct {
				double x;
				double y;
			} PlotWindowAreaMax {0.0, 0.0};
			if (PlotType == OdDbPlotSettings::kWindow) {
				Layout->getPlotWindowArea(PlotWindowAreaMin.x, PlotWindowAreaMin.y, PlotWindowAreaMax.x, PlotWindowAreaMax.y); // in plotPaperUnits
			} else {
				PlotWindowAreaMin = {Database->getLIMMIN().x, Database->getLIMMIN().y};
				PlotWindowAreaMax = {Database->getLIMMAX().x, Database->getLIMMAX().y};
			}
			FieldWidth = PlotWindowAreaMax.x - PlotWindowAreaMin.x;
			FieldHeight = PlotWindowAreaMax.y - PlotWindowAreaMin.y;
			const auto TargetToCenter {ViewportCenter - ViewTarget};
			ViewTarget.set((PlotWindowAreaMin.x + PlotWindowAreaMax.x) / 2., (PlotWindowAreaMin.y + PlotWindowAreaMax.y) / 2., 0);
			ViewTarget.transformBy(EyeToWorldTransform);
			ViewTarget -= TargetToCenter;
		} else if (PlotType == OdDbPlotSettings::kDisplay) {
			ViewTarget = ViewportCenter;
			FieldWidth = ViewportWidth;
			FieldHeight = ViewportHeight;
		} else if (PlotType == OdDbPlotSettings::kExtents || PlotType == OdDbPlotSettings::kLimits && !IsPlanView) {
			OdGeBoundBlock3d BoundBox;
			if (AbstractView->plotExtents(ViewObject, BoundBox)) { // Iterator also skip 'off layers'
				BoundBox.transformBy(EyeToWorldTransform);
				ViewTarget = (BoundBox.minPoint() + BoundBox.maxPoint().asVector()) / 2.;
				BoundBox.transformBy(WorldToEyeTransform);
				FieldWidth = fabs(BoundBox.maxPoint().x - BoundBox.minPoint().x);
				FieldHeight = fabs(BoundBox.maxPoint().y - BoundBox.minPoint().y);
			}
		} else if (PlotType == OdDbPlotSettings::kView) {
			ViewTarget = ViewportCenter;
			FieldWidth = ViewportWidth;
			FieldHeight = ViewportHeight;
		} else if (PlotType == OdDbPlotSettings::kLayout) {
			SkipClipping = true; // used full paper drawing area.
			FieldWidth = (DrawableArea.right - DrawableArea.left) / ScaleFactor; // drx in mm -> fieldWidth in mm
			FieldHeight = (DrawableArea.bottom - DrawableArea.top) / ScaleFactor;
			ViewTarget.set(FieldWidth / 2. - PaperImageOrigin.x - Offset.x / ScaleFactor, FieldHeight / 2. - PaperImageOrigin.y - Offset.y / ScaleFactor, 0); // in mm
			if (!IsMetric) {
				ViewTarget /= kMmPerInch; // must be in plot paper units
				FieldWidth = MillimetersToInches(FieldWidth);
				FieldHeight = MillimetersToInches(FieldHeight);
			}
			Offset.x = 0.0; // Iterator was applied to viewTarget, reset Iterator.
			Offset.y = 0.0;
			PaperImageOrigin.x = 0.0;
			PaperImageOrigin.y = 0.0;
			IsCentered = false;
			IsScaledToFit = false; // kLayout doesn't support Iterator.
		}
		if (PlotType != OdDbPlotSettings::kView) { // AlexR : 3952
			ViewTarget = ViewTarget.orthoProject(OdGePlane(OldTarget, ViewDirection));
		}
		// in plot paper units
		OverallView->setView(ViewTarget + ViewDirection, ViewTarget, ViewUpVector, FieldWidth, FieldHeight, IsPerspective ? OdGsView::kPerspective : OdGsView::kParallel);
		if (!IsMetric) {
			FieldWidth = InchesToMillimeters(FieldWidth);
			FieldWidth = InchesToMillimeters(FieldHeight);
		}
		if (IsScaledToFit) { // Scale factor can be stored in layout, but preview recalculate Iterator if bScaledToFit = true.
			// Some times stored factor isn't correct, due to autocad isn't update Iterator before saving.
			ScaleFactor = odmin((DrawableArea.right - DrawableArea.left) / FieldWidth, (DrawableArea.bottom - DrawableArea.top) / FieldHeight);
		}
		if (IsCentered) { // Offset also can be incorrectly saved.
			Offset.x = (DrawableArea.right - DrawableArea.left - FieldWidth * ScaleFactor) / 2.;
			Offset.y = (DrawableArea.bottom - DrawableArea.top - FieldHeight * ScaleFactor) / 2.;
			if (IsPrint90Degrees || IsPrint180Degrees) {
				Offset.y = -Offset.y;
				Offset.x = -Offset.x;
			}
		}
		if (IsPrint180Degrees || IsPrint90Degrees) {
			DrawableArea.left = DrawableArea.right - FieldWidth * ScaleFactor;
			DrawableArea.bottom = DrawableArea.top + FieldHeight * ScaleFactor;
		} else if (IsPrint0Degrees || IsPrint270Degrees) {
			DrawableArea.right = DrawableArea.left + FieldWidth * ScaleFactor;
			DrawableArea.top = DrawableArea.bottom - FieldHeight * ScaleFactor;
		} else { // preview
			DrawableArea.right = DrawableArea.left + FieldWidth * ScaleFactor;
			DrawableArea.top = DrawableArea.bottom - FieldHeight * ScaleFactor;
		}
		if (!SkipClipping) {
			if (IsPrint180Degrees || IsPrint90Degrees) {
				ClipBox.left = static_cast<long>(ClipBox.right - (DrawableArea.right - DrawableArea.left) * Coefficient.x * WidthScreenFactor);
				ClipBox.bottom = static_cast<long>(ClipBox.top + (DrawableArea.bottom - DrawableArea.top) * Coefficient.y * HeightScreenFactor);
			} else if (IsPrint0Degrees || IsPrint270Degrees) {
				ClipBox.right = static_cast<long>(ClipBox.left + (DrawableArea.right - DrawableArea.left) * Coefficient.x * WidthScreenFactor);
				ClipBox.top = static_cast<long>(ClipBox.bottom - (DrawableArea.bottom - DrawableArea.top) * Coefficient.y * HeightScreenFactor);
			} else { // preview
				ClipBox.right = static_cast<long>(ClipBox.left + (DrawableArea.right - DrawableArea.left) * Coefficient.x * WidthScreenFactor);
				ClipBox.top = static_cast<long>(ClipBox.bottom - (DrawableArea.bottom - DrawableArea.top) * Coefficient.y * HeightScreenFactor);
			}
			ClipBox.OffsetRect(static_cast<int>(Offset.x * Coefficient.x * WidthScreenFactor), int(-Offset.y * Coefficient.y * HeightScreenFactor));
		}
		OverallView->setViewport(OdGePoint2d(0, 0), OdGePoint2d(1, 1));
		CRect ResultClipBox;
		ResultClipBox.IntersectRect(&MarginsClipBox, &ClipBox);

		// Apply clip region to screen
		CRgn ClipBoxRegion;
		ClipBoxRegion.CreateRectRgnIndirect(&ResultClipBox);
		deviceContext->SelectClipRgn(&ClipBoxRegion);

		// Calculate viewport rect in printer units
		const auto x1 {static_cast<long>((Offset.x + DrawableArea.left) * Coefficient.x)};
		const auto x2 {static_cast<long>((Offset.x + DrawableArea.right) * Coefficient.x)};
		const auto y1 {static_cast<long>((-Offset.y + DrawableArea.top) * Coefficient.y)};
		const auto y2 {static_cast<long>((-Offset.y + DrawableArea.bottom) * Coefficient.y)};
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
			m_pPrinterDevice->properties()->putAt(L"WindowHDC", OdRxVariantValue(reinterpret_cast<OdIntPtr>(deviceContext->m_hDC)));
			m_pPrinterDevice->update(nullptr);
		} else {
			const CRect DrawRectangle(ViewportRectangle.m_min.x, ViewportRectangle.m_max.y, ViewportRectangle.m_max.x, ViewportRectangle.m_min.y);
			GenerateTiles(deviceContext->m_hDC, DrawRectangle, m_pPrinterDevice, 1000, 1000);
		}
	} else {
		AfxMessageBox(L"Can't initialize GS for printing...");
	}
	setPlotGeneration(false);
	CView::OnPrint(deviceContext, printInformation);
}

void AeSysView::OnEndPrinting(CDC* /*deviceContext*/, CPrintInfo* /*printInformation*/) {
	PopViewTransform();
	ViewportPopActive();
}

BOOL AeSysView::OnPreparePrinting(CPrintInfo* printInformation) {
	if (m_Plot) {
		CPrintInfo PrintInfo;
		if (theApp.GetPrinterDeviceDefaults(&PrintInfo.m_pPD->m_pd) != 0) {
			auto PrinterDeviceContext {PrintInfo.m_pPD->m_pd.hDC};
			if (PrinterDeviceContext == nullptr) {
				PrinterDeviceContext = PrintInfo.m_pPD->CreatePrinterDC();
			}
			if (PrinterDeviceContext != nullptr) {
				unsigned HorizontalPages;
				unsigned VerticalPages;
				CDC DeviceContext;
				DeviceContext.Attach(PrinterDeviceContext);
				printInformation->SetMaxPage(NumPages(&DeviceContext, m_PlotScaleFactor, HorizontalPages, VerticalPages));
				DeleteDC(DeviceContext.Detach());
			}
		}
	}
	return DoPreparePrinting(printInformation);
}

void AeSysView::OnDrag() {
	exeCmd(L"dolly ");
}

void AeSysView::OnUpdateDrag(CCmdUI* commandUserInterface) {
	commandUserInterface->Enable(static_cast<BOOL>(m_mode == kQuiescent));
}

void AeSysView::OnViewerRegen() {
	m_LayoutHelper->invalidate();
	if (m_LayoutHelper->gsModel() != nullptr) {
		m_LayoutHelper->gsModel()->invalidate(OdGsModel::kInvalidateAll);
	}
	m_PaintMode = kRegenerate;
	PostMessageW(WM_PAINT);
}

void AeSysView::OnViewerViewportRegen() {
	m_LayoutHelper->invalidate();
	if (m_LayoutHelper->gsModel() != nullptr) {
		m_LayoutHelper->gsModel()->invalidate(GetLayoutActiveView());
	}
	m_PaintMode = kRegenerate;
	PostMessageW(WM_PAINT);
}

void AeSysView::OnUpdateViewerRegen(CCmdUI* commandUserInterface) {
	commandUserInterface->Enable(static_cast<BOOL>(m_LayoutHelper->gsModel() != nullptr));
}

// <command_view>
bool AeSysView::CanClose() const {
	if (m_mode != kQuiescent) {
		AfxMessageBox(L"Can not exit while command is active.", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	return true;
}

class SaveViewParameters {
protected:
	AeSysView* m_View;
	HCURSOR m_Cursor;
public:
	SaveViewParameters(AeSysView* view, OdEdInputTracker* inputTracker, const HCURSOR cursor, const bool snap)
		: m_View(view)
		, m_Cursor(view->Cursor()) {
		view->Track(inputTracker);
		view->setCursor(cursor);
		if (snap) { view->m_Editor.InitializeSnapping(view->GetLayoutActiveTopView(), inputTracker); }
	}

	~SaveViewParameters() {
		m_View->Track(nullptr);
		m_View->setCursor(m_Cursor);
		m_View->m_Editor.UninitializeSnapping(m_View->GetLayoutActiveTopView());
	}
};

constexpr unsigned gc_BlinkCursorTimer = 888;
const unsigned gc_BlinkCursorRate = GetCaretBlinkTime();

void CALLBACK StringTrackerTimer(HWND window, unsigned message, unsigned timerId, unsigned long time);

class SaveViewParametersTimer : public SaveViewParameters {
	bool m_TimerSet;
public:
	SaveViewParametersTimer(AeSysView* view, OdEdStringTracker* tracker, const HCURSOR cursor)
		: SaveViewParameters(view, tracker, cursor, false) {
		if (tracker != nullptr) {
			tracker->setCursor(true);
			SetTimer(m_View->m_hWnd, gc_BlinkCursorTimer, gc_BlinkCursorRate, static_cast<TIMERPROC>(StringTrackerTimer));
			m_TimerSet = true;
		} else {
			m_TimerSet = false;
		}
	}

	~SaveViewParametersTimer() {
		if (m_TimerSet) { KillTimer(m_View->m_hWnd, gc_BlinkCursorTimer); }
	}
};

// Blink cursor timer
bool AeSysView::UpdateStringTrackerCursor() {
	if (m_mode == kGetString && m_Response.type != Response::kString) {
		if (m_Editor.TrackString(m_InputParser.result())) {
			GetLayoutActiveTopView()->invalidate();
			PostMessageW(WM_PAINT);
			return true;
		}
	}
	return false;
}

void CALLBACK StringTrackerTimer(const HWND window, unsigned /*message*/, const unsigned timerId, unsigned long /*time*/) {
	try {
		auto View {dynamic_cast<AeSysView*>(CWnd::FromHandle(window))};
		if (!View->UpdateStringTrackerCursor()) { KillTimer(window, timerId); }
	} catch (...) {
		KillTimer(window, timerId);
	}
}

// </command_view>
unsigned long AeSysView::getKeyState() noexcept {
	unsigned long KeyState(0);
	if (GetKeyState(VK_CONTROL) != 0) { KeyState |= MK_CONTROL; }
	if (GetKeyState(VK_SHIFT) != 0) { KeyState |= MK_SHIFT; }
	return KeyState;
}

OdGePoint3d AeSysView::getPoint(const OdString& prompt, const int options, OdEdPointTracker* tracker) {
	m_Prompt.empty();
	OdSaveState<OdString> SavedPrompt(m_Prompt);
	putString(prompt);
	OdSaveState<Mode> SavedMode(m_mode, kGetPoint);
	m_Response.type = Response::kNone;
	m_inpOptions = options;
	SaveViewParameters SavedViewParameters(this, tracker, LoadCursorW(nullptr, IDC_CROSS), !((options & OdEd::kGptNoOSnap) != 0));
	while (theApp.PumpMessage() != 0) {
		switch (m_Response.type) {
			case Response::kPoint:
				if ((m_inpOptions & OdEd::kGptBeginDrag) != 0) { SetCapture(); }
				return m_Response.point;
			case Response::kString:
				throw OdEdOtherInput(m_Response.string);
			case Response::kCancel:
				throw OdEdCancel();
			case Response::kNone: default:
				break;
		}
		long Idle = 0;
		while (theApp.OnIdle(Idle++) != 0) {
		}
	}
	throw OdEdCancel();
}

OdString AeSysView::getString(const OdString& prompt, const int options, OdEdStringTracker* tracker) {
	m_Prompt.empty();
	OdSaveState<OdString> SavedPrompt(m_Prompt);
	putString(prompt);
	OdSaveState<Mode> SavedMode(m_mode, kGetString);
	m_Response.type = Response::kNone;
	if (tracker != nullptr) { m_InputParser.reset(true); }
	m_inpOptions = options;
	SaveViewParametersTimer SavedViewParameters(this, tracker, LoadCursorW(nullptr, IDC_IBEAM));
	while (theApp.PumpMessage() != 0) {
		switch (m_Response.type) {
			case Response::kString:
				return m_Response.string;
			case Response::kCancel:
				throw OdEdCancel();
			case Response::kPoint: case Response::kNone: default:
				break;
		}
		long Idle = 0;
		while (theApp.OnIdle(Idle++) != 0) {
		}
	}
	throw OdEdCancel();
}

void AeSysView::putString(const OdString& string) {
	m_Prompt = string;
	const auto NewLineDelimiter {m_Prompt.reverseFind('\n')};
	const wchar_t* Text {string};
	if (NewLineDelimiter >= 0) { Text = Text + NewLineDelimiter + 1; }
	AeSys::AddStringToMessageList(Text);
	theApp.SetStatusPaneTextAt(gc_StatusInfo, Text);
}

void AeSysView::Track(OdEdInputTracker* inputTracker) {
	m_Editor.SetTracker(inputTracker);
}

HCURSOR AeSysView::Cursor() const noexcept {
	return m_hCursor;
}

void AeSysView::setCursor(const HCURSOR cursor) noexcept {
	m_hCursor = cursor;
	SetCursor(cursor);
}

void AeSysView::OnRefresh() {
	PostMessageW(WM_PAINT);
}

bool AeSysView::BeginDragCallback(const OdGePoint3d& point) {
	OdSaveState<Mode> SavedMode(m_mode, kDragDrop);
	GetDocument()->StartDrag(point);
	return true;
}

struct ReactorSort {
	using first_argument_type = OdDbObjectId;
	using second_argument_type = OdDbObjectId;
	using result_type = bool;

	bool operator()(const OdDbObjectId firstObjectId, const OdDbObjectId secondObjectId) {
		auto SecondObject {secondObjectId.openObject()};
		if (SecondObject.isNull()) { return false; }
		const auto SecondObjectReactors {SecondObject->getPersistentReactors()};
		return SecondObjectReactors.contains(firstObjectId);
	}
};

void TransformObjectSet(OdDbObjectIdArray& objects, const OdGeMatrix3d& transformMatrix) {
	std::sort(objects.begin(), objects.end(), ReactorSort());
	for (auto& Object : objects) {
		OdDbEntityPtr Entity {Object.safeOpenObject(OdDb::kForWrite)};
		Entity->transformBy(transformMatrix);
	}
}

// <command_console>
BOOL AeSysView::OnDrop(COleDataObject* dataObject, const DROPEFFECT dropEffect, const CPoint point) {
	auto ClipboardData {AeSysDoc::ClipboardData::Get(dataObject)};
	if (ClipboardData != nullptr) {
		auto Document {GetDocument()};
		OdDbDatabase* Database {Document->m_DatabasePtr};
		Database->startUndoRecord();
		const auto TransformMatrix {OdGeMatrix3d::translation(m_Editor.ToEyeToWorld(point.x, point.y) - ClipboardData->pickPoint())};
		if (m_mode == kDragDrop) {
			auto SelectionSet {Document->SelectionSet()};
			auto SelectionSetObjects {SelectionSet->objectIdArray()};
			if (GetKeyState(VK_CONTROL) & 0xff00) {
				auto IdMapping {OdDbIdMapping::createObject()};
				auto HostDatabase {Database};
				HostDatabase->deepCloneObjects(SelectionSetObjects, HostDatabase->getActiveLayoutBTRId(), *IdMapping);
				for (auto& SelectionSetObject : SelectionSetObjects) {
					OdDbIdPair Pair(SelectionSetObject);
					IdMapping->compute(Pair);
					SelectionSetObject = Pair.value();
				}
			}
			TransformObjectSet(SelectionSetObjects, TransformMatrix);
		} else {
			try {
				auto TemporaryDatabase {theApp.readFile(ClipboardData->tempFileName(), true, false, Oda::kShareDenyNo)};
				Database->insert(TransformMatrix, TemporaryDatabase);
			} catch (const OdError& Error) {
				AfxMessageBox(Error.description());
				return FALSE;
			}
		}
		return TRUE;
	}
	return __super::OnDrop(dataObject, dropEffect, point);
}
// </command_console>
DROPEFFECT AeSysView::OnDragOver(COleDataObject* dataObject, const unsigned long keyState, const CPoint point) {
	if (m_mode == kQuiescent || m_mode == kDragDrop) {
		if (AeSysDoc::ClipboardData::IsAcadDataAvailable(dataObject)) {
			return static_cast<DROPEFFECT>(GetKeyState(VK_CONTROL) & 0xff00 ? DROPEFFECT_COPY : DROPEFFECT_MOVE);
		}
	}
	return __super::OnDragOver(dataObject, keyState, point);
}

BOOL AeSysView::PreCreateWindow(CREATESTRUCT& createStructure) {
	// <tas="Modify the Window class or styles here by modifying the CREATESTRUCT"/>
	return CView::PreCreateWindow(createStructure);
}

void AeSysView::OnUpdate(CView* sender, const LPARAM hint, CObject* hintObject) {
	auto DeviceContext {GetDC()};
	if (DeviceContext == nullptr) { return; }
	const auto BackgroundColor {DeviceContext->GetBkColor()};
	DeviceContext->SetBkColor(g_ViewBackgroundColor);
	auto PrimitiveState {0};
	auto DrawMode {0};
	if ((hint & EoDb::kSafe) == EoDb::kSafe) { PrimitiveState = g_PrimitiveState.Save(); }
	if ((hint & EoDb::kErase) == EoDb::kErase) { DrawMode = g_PrimitiveState.SetROP2(*DeviceContext, R2_XORPEN); }
	if ((hint & EoDb::kTrap) == EoDb::kTrap) { EoDbPrimitive::SetHighlightColorIndex(theApp.TrapHighlightColor()); }
	switch (hint) {
		case EoDb::kPrimitive: case EoDb::kPrimitiveSafe: case EoDb::kPrimitiveEraseSafe:
			dynamic_cast<EoDbPrimitive*>(hintObject)->Display(this, DeviceContext);
			break;
		case EoDb::kGroup: case EoDb::kGroupSafe: case EoDb::kGroupEraseSafe: case EoDb::kGroupSafeTrap: case EoDb::kGroupEraseSafeTrap:
			dynamic_cast<EoDbGroup*>(hintObject)->Display(this, DeviceContext);
			break;
		case EoDb::kGroups: case EoDb::kGroupsSafe: case EoDb::kGroupsSafeTrap: case EoDb::kGroupsEraseSafeTrap:
			dynamic_cast<EoDbGroupList*>(hintObject)->Display(this, DeviceContext);
			break;
		case EoDb::kLayer: case EoDb::kLayerErase:
			dynamic_cast<EoDbLayer*>(hintObject)->Display(this, DeviceContext);
			break;
		default:
			CView::OnUpdate(sender, hint, hintObject);
	}
	if ((hint & EoDb::kTrap) == EoDb::kTrap) { EoDbPrimitive::SetHighlightColorIndex(0); }
	if ((hint & EoDb::kErase) == EoDb::kErase) { g_PrimitiveState.SetROP2(*DeviceContext, DrawMode); }
	if ((hint & EoDb::kSafe) == EoDb::kSafe) { g_PrimitiveState.Restore(*DeviceContext, PrimitiveState); }
	DeviceContext->SetBkColor(BackgroundColor);
	ReleaseDC(DeviceContext);
}

void AeSysView::Respond(const OdString& string) {
	m_Response.type = Response::kString;
	m_Response.string = string;
}

CRect AeSysView::ViewportRectangle() const {
	CRect ClientRectangle;
	GetClientRect(&ClientRectangle);
	return ClientRectangle;
}

CRect AeSysView::ViewRectangle(OdGsView* view) {
	OdGePoint3d LowerLeftPoint;
	OdGePoint3d UpperRightPoint;
	view->getViewport(reinterpret_cast<OdGePoint2d&>(LowerLeftPoint), reinterpret_cast<OdGePoint2d&>(UpperRightPoint));
	const auto ScreenMatrix {view->screenMatrix()};
	LowerLeftPoint.transformBy(ScreenMatrix);
	UpperRightPoint.transformBy(ScreenMatrix);
	return {OdRoundToLong(LowerLeftPoint.x), OdRoundToLong(UpperRightPoint.y), OdRoundToLong(UpperRightPoint.x), OdRoundToLong(LowerLeftPoint.y)};
}

void AeSysView::OnChar(const unsigned characterCodeValue, unsigned repeatCount, const unsigned flags) {
	__super::OnChar(characterCodeValue, repeatCount, flags);
	m_Response.string = m_InputParser.result();
	switch (characterCodeValue) {
		case VK_BACK:
			while (repeatCount-- != 0U) {
				m_InputParser.eraseChar();
			}
			break;
		case VK_ESCAPE:
			m_Response.type = Response::kCancel;
			m_InputParser.reset(false);
			switch (m_mode) {
				case kQuiescent:
					if (m_Editor.Unselect()) { PostMessageW(WM_PAINT); }
					break;
				case kGetPoint: case kGetString: case kDragDrop: default:
					break;
			}
			break;
		default:
			while (repeatCount-- != 0U) {
				if (!m_InputParser.addChar(static_cast<wchar_t>(characterCodeValue))) {
					m_InputParser.reset(false);
					switch (m_mode) {
						case kQuiescent:
							if (m_Response.string.isEmpty()) {
								GetDocument()->ExecuteCommand(AeSysDoc::RecentCommandName());
							} else {
								GetDocument()->ExecuteCommand(m_Response.string);
							}
							break;
						case kGetPoint: case kGetString:
							m_Response.type = Response::kString;
							break;
						case kDragDrop:
							break;
					}
				}
			}
			break;
	}
	if (m_mode == kGetString && m_Response.type != Response::kString && m_InputParser.result() != m_Response.string) {
		if (m_Editor.TrackString(m_InputParser.result())) {
			GetLayoutActiveTopView()->invalidate();
			PostMessageW(WM_PAINT);
		}
	}
	if (m_Prompt.isEmpty()) {
		m_Prompt = L"command: ";
	} else if (m_InputParser.result().isEmpty()) {
		theApp.SetStatusPaneTextAt(gc_StatusInfo, m_Prompt);
	} else {
		theApp.SetStatusPaneTextAt(gc_StatusInfo, m_InputParser.result());
	}
}

void AeSysView::OnKeyDown(const unsigned character, const unsigned repeatCount, const unsigned flags) {
	switch (character) {
		case VK_ESCAPE:
			break;
		case VK_F5:
			PostMessageW(WM_PAINT);
			break;
		case VK_DELETE:
			GetDocument()->DeleteSelection(false);
			PostMessageW(WM_PAINT);
			break;
		default: ;
	}
	__super::OnKeyDown(character, repeatCount, flags);
}

void AeSysView::OnLButtonDown(const unsigned flags, const CPoint point) {
	if (AeSys::customLButtonDownCharacters.IsEmpty()) {
		__super::OnLButtonDown(flags, point);
		switch (m_mode) {
			case kQuiescent:
				if (m_Editor.OnMouseLeftButtonClick(flags, point.x, point.y, this)) { PostMessageW(WM_PAINT); }
				break;
			case kGetPoint:
				m_Response.point = m_Editor.ToEyeToWorld(point.x, point.y);
				if (!((m_inpOptions & OdEd::kGptNoUCS) != 0) && !m_Editor.ToUcsToWorld(m_Response.point)) { break; }
				m_Editor.Snap(m_Response.point);
				m_Response.type = Response::kPoint;
				break;
			case kGetString: case kDragDrop:
				break;
		}
	} else {
		DoCustomMouseClick(AeSys::customLButtonDownCharacters);
	}
}

void AeSysView::OnLButtonUp(const unsigned flags, const CPoint point) {
	if (AeSys::customLButtonUpCharacters.IsEmpty()) {
		__super::OnLButtonUp(flags, point);
		if (m_mode == kGetPoint && GetCapture() == this) {
			m_Response.point = m_Editor.ToEyeToWorld(point.x, point.y);
			if (!((m_inpOptions & OdEd::kGptNoUCS) != 0) && !m_Editor.ToUcsToWorld(m_Response.point)) { return; }
			m_Response.type = Response::kPoint;
			ReleaseCapture();
		}
		m_Editor.SetEntityCenters();
	} else {
		DoCustomMouseClick(AeSys::customLButtonUpCharacters);
	}
}

void AeSysView::OnMButtonDown(const unsigned flags, const CPoint point) {
	m_MiddleButton = true;
	m_MousePosition = point;
	__super::OnMButtonDown(flags, point);
}

void AeSysView::OnMButtonUp(const unsigned flags, const CPoint point) {
	m_MiddleButton = false;
	__super::OnMButtonUp(flags, point);
}

void AeSysView::OnMouseMove(const unsigned flags, const CPoint point) {
	DisplayOdometer();
	if (m_MousePosition != point != 0) {
		switch (m_mode) {
			case kQuiescent:
				m_Editor.OnMouseMove(flags, point.x, point.y);
				break;
			case kGetPoint: {
				auto Point {m_Editor.ToEyeToWorld(point.x, point.y)};
				if (!((m_inpOptions & OdEd::kGptNoUCS) != 0) && !m_Editor.ToUcsToWorld(Point)) { return; }
				if (!((m_inpOptions & OdEd::kGptNoOSnap) != 0)) { m_Editor.Snap(Point); }
				m_Editor.TrackPoint(Point);
				break;
			}
			case kGetString: case kDragDrop: default:
				break;
		}
		if (m_LeftButton) {
			CClientDC ClientDeviceContext(this);
			CRect ZoomOldRectangle;
			ZoomOldRectangle.SetRect(m_MouseClick.x, m_MouseClick.y, m_MousePosition.x, m_MousePosition.y);
			ZoomOldRectangle.NormalizeRect();
			ZoomOldRectangle.InflateRect(1, 1);
			RedrawWindow(&ZoomOldRectangle);
			CRect ZoomRectangle;
			ZoomRectangle.SetRect(m_MouseClick.x, m_MouseClick.y, point.x, point.y);
			ZoomRectangle.NormalizeRect();
			ClientDeviceContext.DrawFocusRect(&ZoomRectangle);
		} else if (m_MiddleButton) {
			OdGsViewPtr FirstView {m_LayoutHelper->viewAt(0)};
			OdGeVector3d DollyVector(static_cast<double>(m_MousePosition.x - point.x), static_cast<double>(m_MousePosition.y - point.y), 0.0);
			DollyVector.transformBy((FirstView->screenMatrix() * FirstView->projectionMatrix()).inverse());
			FirstView->dolly(DollyVector);
			m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
			m_ViewTransform.BuildTransformMatrix();
			PostMessageW(WM_PAINT);
		} else if (m_RightButton) {
			Orbit(static_cast<double>(m_MousePosition.y - point.y) / 100.0, static_cast<double>(m_MousePosition.x - point.x) / 100.0);
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
		default: ;
	}
	if (m_RubberBandType != kNone) {
		auto DeviceContext {GetDC()};
		const auto DrawMode {DeviceContext->SetROP2(R2_XORPEN)};
		CPen RubberBandPen(PS_SOLID, 0, g_RubberBandColor);
		const auto Pen {DeviceContext->SelectObject(&RubberBandPen)};
		if (m_RubberBandType == kLines) {
			DeviceContext->MoveTo(m_RubberBandLogicalBeginPoint);
			DeviceContext->LineTo(m_RubberBandLogicalEndPoint);
			m_RubberBandLogicalEndPoint = point;
			DeviceContext->MoveTo(m_RubberBandLogicalBeginPoint);
			DeviceContext->LineTo(m_RubberBandLogicalEndPoint);
		} else if (m_RubberBandType == kRectangles) {
			const auto Brush {dynamic_cast<CBrush*>(DeviceContext->SelectStockObject(NULL_BRUSH))};
			DeviceContext->Rectangle(m_RubberBandLogicalBeginPoint.x, m_RubberBandLogicalBeginPoint.y, m_RubberBandLogicalEndPoint.x, m_RubberBandLogicalEndPoint.y);
			m_RubberBandLogicalEndPoint = point;
			DeviceContext->Rectangle(m_RubberBandLogicalBeginPoint.x, m_RubberBandLogicalBeginPoint.y, m_RubberBandLogicalEndPoint.x, m_RubberBandLogicalEndPoint.y);
			DeviceContext->SelectObject(Brush);
		}
		DeviceContext->SelectObject(Pen);
		DeviceContext->SetROP2(DrawMode);
		ReleaseDC(DeviceContext);
	}
}

BOOL AeSysView::OnMouseWheel(const unsigned flags, const short zDelta, const CPoint point) {
	//ScreenToClient(&point);
	//if (m_editor.OnMouseWheel(flags, point.x, point.y, zDelta)) {
	//    PostMessageW(WM_PAINT);
	//    propagateActiveViewChanges();
	//}
	DollyAndZoom(zDelta > 0 ? 1. / 0.9 : 0.9);
	InvalidateRect(nullptr);
	return __super::OnMouseWheel(flags, zDelta, point);
}

void AeSysView::OnRButtonDown(const unsigned flags, const CPoint point) {
	if (AeSys::customRButtonDownCharacters.IsEmpty()) {
		m_RightButton = true;
		m_MousePosition = point;
		__super::OnRButtonDown(flags, point);
	} else {
		DoCustomMouseClick(AeSys::customRButtonDownCharacters);
	}
}

void AeSysView::OnRButtonUp(const unsigned flags, const CPoint point) {
	if (AeSys::customRButtonUpCharacters.IsEmpty()) {
		m_RightButton = false;

		// <tas="Context menus using right mouse button goes here."/>
		__super::OnRButtonUp(flags, point);
	} else {
		DoCustomMouseClick(AeSys::customRButtonUpCharacters);
	}
}

struct OdExRegenCmd : OdEdCommand {
	OdGsLayoutHelper* m_LayoutHelper {nullptr};
	AeSysView* m_View {nullptr};

	const OdString groupName() const override { return L"REGEN"; }

	const OdString globalName() const override { return L"REGEN"; }

	[[nodiscard]] long flags() const override {
		return OdEdCommand::flags() | kNoUndoMarker;
	}

	void execute(OdEdCommandContext* /*edCommandContext*/) noexcept override {
		// <tas="placeholder until implemented" m_View->OnViewerRegen();"</tas>
	}
};

OdEdCommandPtr AeSysView::command(const OdString& commandName) {
	if (commandName.iCompare(L"REGEN") == 0) {
		auto c {OdRxObjectImpl<OdExRegenCmd>::createObject()};
		c->m_View = this;
		c->m_LayoutHelper = m_LayoutHelper;
		return c;
	}
	return m_Editor.Command(commandName);
}

OdExEditorObject& AeSysView::EditorObject() noexcept {
	return m_Editor;
}

const OdExEditorObject& AeSysView::EditorObject() const noexcept {
	return m_Editor;
}

bool AeSysView::IsModelSpaceView() const {
	return getDatabase()->getTILEMODE();
}

OdIntPtr AeSysView::drawableFilterFunctionId(OdDbStub* viewportId) const {
	if (theApp.PagingType() == OdDb::kPage || theApp.PagingType() == OdDb::kUnload) {
		return OdGiContextForDbDatabase::drawableFilterFunctionId(viewportId) | kDrawableFilterAppRangeStart;
	}
	return OdGiContextForDbDatabase::drawableFilterFunctionId(viewportId);
}

unsigned long AeSysView::drawableFilterFunction(const OdIntPtr functionId, const OdGiDrawable* drawable, const unsigned long flags) {
	if (theApp.PagingType() == OdDb::kPage || theApp.PagingType() == OdDb::kUnload) {
		getDatabase()->pageObjects();
	}
	return OdGiContextForDbDatabase::drawableFilterFunction(functionId & ~kDrawableFilterAppRangeMask, drawable, flags);
}

BOOL AeSysView::OnIdle(long /*count*/) {
	if (!m_LayoutHelper->isValid()) {
		PostMessageW(WM_PAINT);
	}
	return TRUE;
}

// <tas=Transition - code above sourced from ODA examples"</tas>
OdDbDatabasePtr AeSysView::Database() const {
	return GetDocument()->m_DatabasePtr;
}

void AeSysView::OnActivateFrame(const unsigned state, CFrameWnd* deactivateFrame) {
	CView::OnActivateFrame(state, deactivateFrame);
}

void AeSysView::OnActivateView(BOOL activate, CView* activateView, CView* deactivateView) {
	auto MainFrame {dynamic_cast<CMainFrame*>(AfxGetMainWnd())};
	if (activate != 0) {
		if (CopyAcceleratorTableW(MainFrame->m_hAccelTable, nullptr, 0) == 0) { // Accelerator table was destroyed when keyboard focus was killed - reload resource
			theApp.BuildModeSpecificAcceleratorTable();
		}
	}
	auto& ActiveViewScaleProperty {MainFrame->GetPropertiesPane().GetActiveViewScaleProperty()};
	ActiveViewScaleProperty.SetValue(m_WorldScale);
	ActiveViewScaleProperty.Enable(activate);
	SetCursorPosition(OdGePoint3d::kOrigin);
	CView::OnActivateView(activate, activateView, deactivateView);
}

void AeSysView::OnSetFocus(CWnd* oldWindow) {
	const auto MainFrame {dynamic_cast<CMainFrame*>(AfxGetMainWnd())};
	if (CopyAcceleratorTableW(MainFrame->m_hAccelTable, nullptr, 0) == 0) { // Accelerator table was destroyed when keyboard focus was killed - reload resource
		theApp.BuildModeSpecificAcceleratorTable();
	}
	CView::OnSetFocus(oldWindow);
}

void AeSysView::OnKillFocus(CWnd* newWindow) {
	const auto AcceleratorTableHandle = dynamic_cast<CMainFrame*>(AfxGetMainWnd())->m_hAccelTable;
	DestroyAcceleratorTable(AcceleratorTableHandle);
	CView::OnKillFocus(newWindow);
}

void AeSysView::OnPrepareDC(CDC* deviceContext, CPrintInfo* printInformation) {
	CView::OnPrepareDC(deviceContext, printInformation);
	if (deviceContext->IsPrinting() != 0) {
		if (m_Plot) {
			const auto HorizontalSizeInInches {MillimetersToInches(static_cast<double>(deviceContext->GetDeviceCaps(HORZSIZE))) / m_PlotScaleFactor};
			const auto VerticalSizeInInches {MillimetersToInches(static_cast<double>(deviceContext->GetDeviceCaps(VERTSIZE))) / m_PlotScaleFactor};
			unsigned HorizontalPages;
			unsigned VerticalPages;
			NumPages(deviceContext, m_PlotScaleFactor, HorizontalPages, VerticalPages);
			const auto X {(printInformation->m_nCurPage - 1) % HorizontalPages * HorizontalSizeInInches};
			const auto Y {(printInformation->m_nCurPage - 1) / HorizontalPages * VerticalSizeInInches};
			m_ViewTransform.SetProjectionPlaneField(0.0, 0.0, HorizontalSizeInInches, VerticalSizeInInches);
			const auto Target(OdGePoint3d(X, Y, 0.0));
			m_ViewTransform.SetTarget(Target);
			const auto Position(Target + OdGeVector3d::kZAxis);
			m_ViewTransform.SetPosition_(Position);
			m_ViewTransform.SetViewUp(OdGeVector3d::kYAxis);
			// <tas="Near Far clipping on Plot DC prepare?>
			m_ViewTransform.SetNearClipDistance(-1000.);
			m_ViewTransform.SetFarClipDistance(1000.);
			//</tas>
			m_ViewTransform.EnablePerspective(false);
			m_ViewTransform.BuildTransformMatrix();
		} else {
		}
	}
}

void AeSysView::OnContextMenu(CWnd*, const CPoint point) {
	theApp.ShowPopupMenu(IDR_CONTEXT_MENU, point, this);
}

void AeSysView::DoCustomMouseClick(const CString& characters) {
	auto Position {0};
	while (Position < characters.GetLength()) {
		if (characters.Find(L"{", Position) == Position) {
			Position++;
			OdString VirtualKey {characters.Tokenize(L"}", Position)};
			PostMessageW(WM_KEYDOWN, static_cast<unsigned>(_wtoi(VirtualKey)));
		} else {
			PostMessageW(WM_CHAR, characters[Position++]);
		}
	}
}

CMFCStatusBar& AeSysView::GetStatusBar() const {
	return dynamic_cast<CMainFrame*>(AfxGetMainWnd())->GetStatusBar();
}

void AeSysView::PopViewTransform() {
	if (m_ViewTransforms.IsEmpty() == 0) {
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

void AeSysView::PushModelTransform(const EoGeMatrix3d& transformation) {
	m_ModelTransform.PushModelTransform(transformation);
}

void AeSysView::PopModelTransform() {
	m_ModelTransform.PopModelTransform();
}

void AeSysView::BackgroundImageDisplay(CDC* deviceContext) {
	if (m_ViewBackgroundImage && static_cast<HBITMAP>(m_BackgroundImageBitmap) != nullptr) {
		const auto DestinationWidth {static_cast<int>(m_Viewport.WidthInPixels())};
		const auto DestinationHeight {static_cast<int>(m_Viewport.HeightInPixels())};
		BITMAP bm;
		m_BackgroundImageBitmap.GetBitmap(&bm);
		CDC MemoryDeviceContext;
		MemoryDeviceContext.CreateCompatibleDC(nullptr);
		const auto Bitmap {MemoryDeviceContext.SelectObject(&m_BackgroundImageBitmap)};
		const auto Palette {deviceContext->SelectPalette(&m_BackgroundImagePalette, FALSE)};
		deviceContext->RealizePalette();
		const auto Target {m_ViewTransform.Target()};
		const auto OverviewTarget {m_OverviewViewTransform.Target()};
		const auto U {Target.x - OverviewTarget.x};
		const auto V {Target.y - OverviewTarget.y};

		// Determine the region of the bitmap to transfer to display
		CRect rcWnd;
		rcWnd.left = lround((m_ViewTransform.FieldWidthMinimum() - OverviewUMin() + U) / OverviewUExt() * static_cast<double>(bm.bmWidth));
		rcWnd.top = lround((1. - (m_ViewTransform.FieldHeightMaximum() - OverviewVMin() + V) / OverviewVExt()) * static_cast<double>(bm.bmHeight));
		rcWnd.right = lround((m_ViewTransform.FieldWidthMaximum() - OverviewUMin() + U) / OverviewUExt() * static_cast<double>(bm.bmWidth));
		rcWnd.bottom = lround((1. - (m_ViewTransform.FieldHeightMinimum() - OverviewVMin() + V) / OverviewVExt()) * static_cast<double>(bm.bmHeight));
		const auto SourceWidth {rcWnd.Width()};
		const auto SourceHeight {rcWnd.Height()};
		deviceContext->StretchBlt(0, 0, DestinationWidth, DestinationHeight, &MemoryDeviceContext, gsl::narrow_cast<int>(rcWnd.left), gsl::narrow_cast<int>(rcWnd.top), SourceWidth, SourceHeight, SRCCOPY);
		MemoryDeviceContext.SelectObject(Bitmap);
		deviceContext->SelectPalette(Palette, FALSE);
	}
}

double AeSysView::OverviewUExt() const noexcept {
	return m_OverviewViewTransform.FieldWidth();
}

double AeSysView::OverviewUMin() const noexcept {
	return m_OverviewViewTransform.FieldWidthMinimum();
}

double AeSysView::OverviewVExt() const noexcept {
	return m_OverviewViewTransform.FieldHeight();
}

double AeSysView::OverviewVMin() const noexcept {
	return m_OverviewViewTransform.FieldHeightMinimum();
}

CPoint AeSysView::DoViewportProjection(const EoGePoint4d& point) const noexcept {
	return m_Viewport.DoProjection(point);
}

void AeSysView::DoViewportProjection(CPoint* pnt, const int iPts, EoGePoint4d* pt) const noexcept {
	m_Viewport.DoProjection(pnt, iPts, pt);
}

void AeSysView::DoViewportProjection(CPoint* pnt, EoGePoint4dArray& points) const {
	m_Viewport.DoProjection(pnt, points);
}

OdGePoint3d AeSysView::DoViewportProjectionInverse(const OdGePoint3d& point) const noexcept {
	auto Point {point};
	m_Viewport.DoProjectionInverse(Point);
	return Point;
}

double AeSysView::ViewportHeightInInches() const noexcept {
	return m_Viewport.HeightInInches();
}

double AeSysView::ViewportWidthInInches() const noexcept {
	return m_Viewport.WidthInInches();
}

void AeSysView::ViewportPopActive() {
	if (m_Viewports.IsEmpty() == 0) {
		m_Viewport = m_Viewports.RemoveTail();
	}
}

void AeSysView::ViewportPushActive() {
	m_Viewports.AddTail(m_Viewport);
}

void AeSysView::SetViewportSize(const int width, const int height) noexcept {
	m_Viewport.SetSize(width, height);
}

void AeSysView::SetDeviceHeightInInches(const double height) noexcept {
	m_Viewport.SetDeviceHeightInInches(height);
}

void AeSysView::SetDeviceWidthInInches(const double width) noexcept {
	m_Viewport.SetDeviceWidthInInches(width);
}
// AeSysView printing
void AeSysView::OnFilePlotFull() {
	m_Plot = true;
	m_PlotScaleFactor = 1.0F;
	CView::OnFilePrint();
}

void AeSysView::OnFilePlotHalf() {
	m_Plot = true;
	m_PlotScaleFactor = 0.5F;
	CView::OnFilePrint();
}

void AeSysView::OnFilePlotQuarter() {
	m_Plot = true;
	m_PlotScaleFactor = 0.25F;
	CView::OnFilePrint();
}

void AeSysView::OnFilePrint() {
	m_Plot = false;
	m_PlotScaleFactor = 1.0F;
	CView::OnFilePrint();
}

unsigned AeSysView::NumPages(CDC* deviceContext, const double scaleFactor, unsigned& horizontalPages, unsigned& verticalPages) {
	OdGeExtents3d Extents;
	GetDocument()->GetExtents___(this, Extents);
	const auto MinimumPoint {Extents.minPoint()};
	const auto MaximumPoint {Extents.maxPoint()};
	const auto HorizontalSizeInInches {MillimetersToInches(static_cast<double>(deviceContext->GetDeviceCaps(HORZSIZE)))};
	const auto VerticalSizeInInches {MillimetersToInches(static_cast<double>(deviceContext->GetDeviceCaps(VERTSIZE)))};
	// <tas="Pages counts possibly using + 0.5 which is done also in lround for calculation of horizontalPages and verticalPages. check operator precedence also."/>
	horizontalPages = gsl::narrow_cast<unsigned>(lround((MaximumPoint.x - MinimumPoint.x) * scaleFactor / HorizontalSizeInInches + 0.5));
	verticalPages = gsl::narrow_cast<unsigned>(lround((MaximumPoint.y - MinimumPoint.y) * scaleFactor / VerticalSizeInInches + 0.5));
	return horizontalPages * verticalPages;
}

void AeSysView::DisplayPixel(CDC* deviceContext, COLORREF colorReference, const OdGePoint3d& point) {
	EoGePoint4d View(point, 1.0);
	ModelViewTransformPoint(View);
	if (View.IsInView()) {
		deviceContext->SetPixel(DoViewportProjection(View), colorReference);
	}
}

void AeSysView::Orbit(const double x, const double y) {
	auto FirstView {m_LayoutHelper->viewAt(0)};
	FirstView->orbit(x, y);
	m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
}

void AeSysView::Dolly() {
	CPoint Point;
	GetCursorPos(&Point);
	ScreenToClient(&Point);
	OdGsViewPtr FirstView {m_LayoutHelper->viewAt(0)};
	auto Position(FirstView->position());
	Position.transformBy(FirstView->worldToDeviceMatrix());
	auto X {static_cast<int>(OdRound(Position.x))};
	auto Y {static_cast<int>(OdRound(Position.y))};
	X = Point.x - X;
	Y = Point.y - Y;
	OdGeVector3d DollyVector(static_cast<double>(X), static_cast<double>(Y), 0.0);
	DollyVector.transformBy((FirstView->screenMatrix() * FirstView->projectionMatrix()).inverse());
	FirstView->dolly(DollyVector);
	m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
	SetCursorPosition(FirstView->target());
}

void AeSysView::DollyAndZoom(const double zoomFactor) {
	Dolly();
	auto FirstView {m_LayoutHelper->viewAt(0)};
	FirstView->zoom(zoomFactor);
	m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
	SetCursorPosition(FirstView->target());
}

void AeSysView::OnSetupScale() {
	EoDlgSetScale SetScaleDialog;
	SetScaleDialog.scale = WorldScale();
	if (SetScaleDialog.DoModal() == IDOK) {
		SetWorldScale(SetScaleDialog.scale);
	}
}

void AeSysView::On3dViewsTop() {
	auto FirstView {m_LayoutHelper->viewAt(0)};
	m_ViewTransform.EnablePerspective(false);
	const auto Target(FirstView->target());
	const auto Position(Target + OdGeVector3d::kZAxis * FirstView->lensLength());
	const auto UpVector(OdGeVector3d::kYAxis);
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
	const auto UpVector(OdGeVector3d::kYAxis);
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
	const auto UpVector(OdGeVector3d::kZAxis);
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
	const auto UpVector(OdGeVector3d::kZAxis);
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
	const auto UpVector(OdGeVector3d::kZAxis);
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
	const auto UpVector(OdGeVector3d::kZAxis);
	FirstView->setView(Position, Target, UpVector, FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.SetView(Position, Target, UpVector, FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
}

void AeSysView::On3dViewsIsometric() {
	static auto LeftRight {0};
	static auto FrontBack {0};
	static auto AboveUnder {0};
	EoDlgSelectIsometricView Dialog;
	Dialog.leftRight = LeftRight;
	Dialog.frontBack = FrontBack;
	Dialog.aboveUnder = AboveUnder;
	if (Dialog.DoModal() != 0) {
		LeftRight = Dialog.leftRight;
		FrontBack = Dialog.frontBack;
		AboveUnder = Dialog.aboveUnder;
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
	Orbit(0.0, EoToRadian(-10.));
}

void AeSysView::OnCameraRotateRight() {
	Orbit(0.0, EoToRadian(10.));
}

void AeSysView::OnCameraRotateUp() {
	Orbit(EoToRadian(-10.), 0.0);
}

void AeSysView::OnCameraRotateDown() {
	Orbit(EoToRadian(10.), 0.0);
}

void AeSysView::OnViewParameters() {
	EoDlgViewParameters ViewParametersDialog;
	auto ModelView(m_ViewTransform);
	ViewParametersDialog.modelView = unsigned long(&ModelView);
	ViewParametersDialog.perspectiveProjection = static_cast<BOOL>(m_ViewTransform.IsPerspectiveOn());
	if (ViewParametersDialog.DoModal() == IDOK) {
		m_ViewTransform.EnablePerspective(ViewParametersDialog.perspectiveProjection == TRUE);
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

void AeSysView::OnViewRenderMode(const unsigned commandId) {
	const auto RenderMode {OdGsView::RenderMode(commandId - ID_VIEW_RENDERMODE_2DOPTIMIZED)};
	SetRenderMode(RenderMode);
}

void AeSysView::OnViewWindow() {
	CPoint CurrentPosition;
	GetCursorPos(&CurrentPosition);
	const auto WindowMenu {LoadMenuW(theApp.GetInstance(), MAKEINTRESOURCEW(IDR_WINDOW))};
	auto SubMenu {CMenu::FromHandle(GetSubMenu(WindowMenu, 0))};
	SubMenu->TrackPopupMenuEx(TPM_LEFTALIGN, CurrentPosition.x, CurrentPosition.y, AfxGetMainWnd(), nullptr);
	DestroyMenu(WindowMenu);
}

void AeSysView::OnWindowZoomWindow() {
	m_Points.clear();
	m_ZoomWindow = !m_ZoomWindow;
}

void AeSysView::OnUpdateWindowZoomWindow(CCmdUI* commandUserInterface) {
	commandUserInterface->SetCheck(static_cast<int>(m_ZoomWindow));
}

void AeSysView::OnViewOdometer() {
	m_ViewOdometer = !m_ViewOdometer;
	InvalidateRect(nullptr);
}

void AeSysView::OnViewRefresh() {
	InvalidateRect(nullptr);
}

void AeSysView::OnUpdateViewRenderMode2dOptimized(CCmdUI* commandUserInterface) {
	commandUserInterface->SetCheck(RenderMode() == OdGsView::k2DOptimized ? MF_CHECKED : MF_UNCHECKED);
}

void AeSysView::OnUpdateViewRenderModeSmoothShaded(CCmdUI* commandUserInterface) {
	commandUserInterface->SetCheck(RenderMode() == OdGsView::kGouraudShaded ? MF_CHECKED : MF_UNCHECKED);
}

void AeSysView::OnUpdateViewRenderModeFlatShaded(CCmdUI* commandUserInterface) {
	commandUserInterface->SetCheck(RenderMode() == OdGsView::kFlatShaded ? MF_CHECKED : MF_UNCHECKED);
}

void AeSysView::OnUpdateViewRenderModeHiddenLine(CCmdUI* commandUserInterface) {
	commandUserInterface->SetCheck(RenderMode() == OdGsView::kHiddenLine ? MF_CHECKED : MF_UNCHECKED);
}

void AeSysView::OnUpdateViewRenderModeWireframe(CCmdUI* commandUserInterface) {
	commandUserInterface->SetCheck(RenderMode() == OdGsView::kWireframe ? MF_CHECKED : MF_UNCHECKED);
}

void AeSysView::OnWindowNormal() {
	CopyActiveModelViewToPreviousModelView();
	DollyAndZoom(m_ViewTransform.FieldWidth() / m_Viewport.WidthInInches());
}

void AeSysView::OnWindowBest() {
	const auto FirstView {m_LayoutHelper->viewAt(0)};
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
	const auto Delta {-1. / (m_Viewport.WidthInInches() / m_ViewTransform.FieldWidth())};
	FirstView->dolly(OdGeVector3d(Delta, 0.0, 0.0));
	m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
}

void AeSysView::OnWindowPanRight() {
	auto FirstView {m_LayoutHelper->viewAt(0)};
	const auto Delta {1. / (m_Viewport.WidthInInches() / m_ViewTransform.FieldWidth())};
	FirstView->dolly(OdGeVector3d(Delta, 0.0, 0.0));
	m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
}

void AeSysView::OnWindowPanUp() {
	auto FirstView {m_LayoutHelper->viewAt(0)};
	const auto Delta {1. / (m_Viewport.HeightInInches() / m_ViewTransform.FieldHeight())};
	FirstView->dolly(OdGeVector3d(0.0, Delta, 0.0));
	m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
}

void AeSysView::OnWindowPanDown() {
	auto FirstView {m_LayoutHelper->viewAt(0)};
	const auto Delta {-1. / (m_Viewport.HeightInInches() / m_ViewTransform.FieldHeight())};
	FirstView->dolly(OdGeVector3d(0.0, Delta, 0.0));
	m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
	m_ViewTransform.BuildTransformMatrix();
	InvalidateRect(nullptr);
}

void AeSysView::OnWindowZoomSpecial() {
	EoDlgViewZoom ViewZoomDialog(this);
	const auto ZoomFactor(m_Viewport.WidthInInches() / m_ViewTransform.FieldWidth());
	ViewZoomDialog.m_ZoomFactor = ZoomFactor;
	if (ViewZoomDialog.DoModal() == IDOK) {
		CopyActiveModelViewToPreviousModelView();
		DollyAndZoom(ViewZoomDialog.m_ZoomFactor / ZoomFactor);
		InvalidateRect(nullptr);
	}
}

void AeSysView::OnSetupDimLength() {
	EoDlgSetLength SetLengthDialog;
	SetLengthDialog.title = L"Set Dimension Length";
	SetLengthDialog.length = theApp.DimensionLength();
	if (SetLengthDialog.DoModal() == IDOK) {
		theApp.SetDimensionLength(SetLengthDialog.length);
		UpdateStateInformation(kDimLen);
	}
}

void AeSysView::OnSetupDimAngle() {
	EoDlgSetAngle SetAngleDialog;
	SetAngleDialog.title = L"Set Dimension Angle";
	SetAngleDialog.angle = theApp.DimensionAngle();
	if (SetAngleDialog.DoModal() == IDOK) {
		theApp.SetDimensionAngle(SetAngleDialog.angle);
		UpdateStateInformation(kDimAng);
	}
}

void AeSysView::OnSetupUnits() {
	EoDlgSetUnitsAndPrecision SetUnitsAndPrecisionDialog;
	SetUnitsAndPrecisionDialog.units = theApp.GetUnits();
	SetUnitsAndPrecisionDialog.precision = theApp.ArchitecturalUnitsFractionPrecision();
	if (SetUnitsAndPrecisionDialog.DoModal() == IDOK) {
		theApp.SetUnits(SetUnitsAndPrecisionDialog.units);
		theApp.SetArchitecturalUnitsFractionPrecision(SetUnitsAndPrecisionDialog.precision);
	}
}

void AeSysView::OnSetupConstraints() {
	EoDlgSetupConstraints Dialog(this);
	if (Dialog.DoModal() == IDOK) {
		UpdateStateInformation(kAll);
	}
}

void AeSysView::OnSetupMouseButtons() {
	EoDlgSetupCustomMouseCharacters Dialog;
	if (Dialog.DoModal() == IDOK) {
	}
}

void AeSysView::OnRelativeMovesEngDown() {
	const auto Origin {GetCursorPosition()};
	auto RelativePoint {Origin};
	RelativePoint.y -= theApp.EngagedLength();
	RelativePoint.rotateBy(theApp.EngagedAngle(), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesEngDownRotate() {
	const auto Origin {GetCursorPosition()};
	auto RelativePoint {Origin};
	RelativePoint.y -= theApp.EngagedLength();
	RelativePoint.rotateBy(theApp.EngagedAngle() + EoToRadian(theApp.DimensionAngle()), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesEngIn() {
	auto RelativePoint {GetCursorPosition()};
	RelativePoint.z -= theApp.EngagedLength();
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesEngLeft() {
	const auto Origin {GetCursorPosition()};
	auto RelativePoint {Origin};
	RelativePoint.x -= theApp.EngagedLength();
	RelativePoint.rotateBy(theApp.EngagedAngle(), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesEngLeftRotate() {
	const auto Origin {GetCursorPosition()};
	auto RelativePoint {Origin};
	RelativePoint.x -= theApp.EngagedLength();
	RelativePoint.rotateBy(theApp.EngagedAngle() + EoToRadian(theApp.DimensionAngle()), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesEngOut() {
	auto RelativePoint {GetCursorPosition()};
	RelativePoint.z += theApp.EngagedLength();
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesEngRight() {
	const auto Origin {GetCursorPosition()};
	auto RelativePoint {Origin};
	RelativePoint.x += theApp.EngagedLength();
	RelativePoint.rotateBy(theApp.EngagedAngle(), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesEngRightRotate() {
	const auto Origin {GetCursorPosition()};
	auto RelativePoint {Origin};
	RelativePoint.x += theApp.EngagedLength();
	RelativePoint.rotateBy(theApp.EngagedAngle() + EoToRadian(theApp.DimensionAngle()), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesEngUp() {
	const auto Origin {GetCursorPosition()};
	auto RelativePoint {Origin};
	RelativePoint.y += theApp.EngagedLength();
	RelativePoint.rotateBy(theApp.EngagedAngle(), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesEngUpRotate() {
	const auto Origin {GetCursorPosition()};
	auto RelativePoint {Origin};
	RelativePoint.y += theApp.EngagedLength();
	RelativePoint.rotateBy(theApp.EngagedAngle() + EoToRadian(theApp.DimensionAngle()), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesDown() {
	auto RelativePoint {GetCursorPosition()};
	RelativePoint.y -= theApp.DimensionLength();
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesDownRotate() {
	const auto Origin {GetCursorPosition()};
	auto RelativePoint {Origin};
	RelativePoint.y -= theApp.DimensionLength();
	RelativePoint.rotateBy(EoToRadian(theApp.DimensionAngle()), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesLeft() {
	auto RelativePoint {GetCursorPosition()};
	RelativePoint.x -= theApp.DimensionLength();
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesLeftRotate() {
	const auto Origin {GetCursorPosition()};
	auto RelativePoint {Origin};
	RelativePoint.x -= theApp.DimensionLength();
	RelativePoint.rotateBy(EoToRadian(theApp.DimensionAngle()), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesIn() {
	auto RelativePoint {GetCursorPosition()};
	RelativePoint.z -= theApp.DimensionLength();
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesOut() {
	auto RelativePoint {GetCursorPosition()};
	RelativePoint.z += theApp.DimensionLength();
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesRight() {
	auto RelativePoint {GetCursorPosition()};
	RelativePoint.x += theApp.DimensionLength();
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesRightRotate() {
	const auto Origin {GetCursorPosition()};
	auto RelativePoint {Origin};
	RelativePoint.x += theApp.DimensionLength();
	RelativePoint.rotateBy(EoToRadian(theApp.DimensionAngle()), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesUp() {
	auto RelativePoint {GetCursorPosition()};
	RelativePoint.y += theApp.DimensionLength();
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnRelativeMovesUpRotate() {
	const auto Origin {GetCursorPosition()};
	auto RelativePoint {Origin};
	RelativePoint.y += theApp.DimensionLength();
	RelativePoint.rotateBy(EoToRadian(theApp.DimensionAngle()), OdGeVector3d::kZAxis, Origin);
	SetCursorPosition(RelativePoint);
}

void AeSysView::OnToolsPrimitiveSnapTo() {
	const auto CurrentPoint {GetCursorPosition()};
	OdGePoint3d PrimitiveSnapPoint;
	if (GroupIsEngaged()) {
		const auto Primitive {m_EngagedPrimitive};
		EoGePoint4d PointInView(CurrentPoint, 1.0);
		ModelViewTransformPoint(PointInView);
		EoDbHatch::SetEdgeToEvaluate(EoDbHatch::Edge());
		EoDbPolyline::SetEdgeToEvaluate(EoDbPolyline::Edge());
		if (Primitive->SelectUsingPoint(PointInView, this, PrimitiveSnapPoint)) {
			PrimitiveSnapPoint = Primitive->GoToNxtCtrlPt();
			m_ptDet = PrimitiveSnapPoint;
			Primitive->AddReportToMessageList(PrimitiveSnapPoint);
			SetCursorPosition(PrimitiveSnapPoint);
			return;
		}
	}
	if (SelectGroupAndPrimitive(CurrentPoint) != nullptr) {
		PrimitiveSnapPoint = m_ptDet;
		m_EngagedPrimitive->AddReportToMessageList(PrimitiveSnapPoint);
		SetCursorPosition(PrimitiveSnapPoint);
	}
}

void AeSysView::OnPrimitivePerpendicularJump() {
	auto CursorPosition {GetCursorPosition()};
	if (SelectGroupAndPrimitive(CursorPosition) != nullptr) {
		if (m_EngagedPrimitive->IsKindOf(RUNTIME_CLASS(EoDbLine)) != 0) {
			const auto Line {dynamic_cast<EoDbLine*>(m_EngagedPrimitive)};
			CursorPosition = Line->ProjPt_(m_ptCursorPosWorld);
			SetCursorPosition(CursorPosition);
		}
	}
}

void AeSysView::OnHelpKey() {
	::HtmlHelpW(AfxGetMainWnd()->GetSafeHwnd(), L"..\\AeSys\\hlp\\AeSys.chm::/menu_mode.htm", HH_DISPLAY_TOPIC, NULL);
}

AeSysView* AeSysView::GetActiveView() {
	const auto MdiFrameWnd {dynamic_cast<CMDIFrameWndEx*>(AfxGetMainWnd())};
	if (MdiFrameWnd == nullptr) { return nullptr; }
	const CMDIChildWndEx* MdiChildWnd = DYNAMIC_DOWNCAST(CMDIChildWndEx, MdiFrameWnd->MDIGetActive());
	if (MdiChildWnd == nullptr) { return nullptr; }
	const auto View {MdiChildWnd->GetActiveView()};

	// View can be wrong kind with splitter windows, or additional views in a single document.
	if (View->IsKindOf(RUNTIME_CLASS(AeSysView)) == 0) { return nullptr; }
	return dynamic_cast<AeSysView*>(View);
}

void AeSysView::OnUpdateViewOdometer(CCmdUI* commandUserInterface) {
	commandUserInterface->SetCheck(static_cast<int>(m_ViewOdometer));
}

void AeSysView::DisplayOdometer() {
	const auto Point {GetCursorPosition()};
	m_vRelPos = Point - GridOrigin();
	if (m_ViewOdometer) {
		const auto Units {theApp.GetUnits()};
		CString Position;
		Position += theApp.FormatLength(m_vRelPos.x, Units) + L", ";
		Position += theApp.FormatLength(m_vRelPos.y, Units) + L", ";
		Position += theApp.FormatLength(m_vRelPos.z, Units);
		if (m_RubberBandType == kLines) {
			const EoGeLineSeg3d Line(m_RubberBandBeginPoint, Point);
			const auto LineLength {Line.length()};
			const auto AngleInXyPlane {Line.AngleFromXAxis_xy()};
			Position += L" [" + theApp.FormatLength(LineLength, Units) + L" @ " + AeSys::FormatAngle(AngleInXyPlane) + L"]";
		}
		auto MainFrame {dynamic_cast<CMainFrame*>(AfxGetMainWnd())};
		MainFrame->SetStatusPaneTextAt(gc_StatusInfo, Position);
	}
}

void AeSysView::OnUpdateViewTrueTypeFonts(CCmdUI* commandUserInterface) {
	commandUserInterface->SetCheck(static_cast<int>(m_ViewTrueTypeFonts));
}

void AeSysView::OnBackgroundImageLoad() {
	CFileDialog FileDialog(TRUE, L"bmp", L"*.bmp");
	FileDialog.m_ofn.lpstrTitle = L"Load Background Image";
	if (FileDialog.DoModal() == IDOK) {
		EoDbBitmapFile BitmapFile(FileDialog.GetPathName());
		BitmapFile.Load(FileDialog.GetPathName(), m_BackgroundImageBitmap, m_BackgroundImagePalette);
		m_ViewBackgroundImage = true;
		InvalidateRect(nullptr);
	}
}

void AeSysView::OnBackgroundImageRemove() {
	if (static_cast<HBITMAP>(m_BackgroundImageBitmap) != nullptr) {
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

void AeSysView::OnUpdateViewBackgroundImage(CCmdUI* commandUserInterface) {
	commandUserInterface->Enable(static_cast<BOOL>(static_cast<HBITMAP>(m_BackgroundImageBitmap) != nullptr));
	commandUserInterface->SetCheck(static_cast<int>(m_ViewBackgroundImage));
}

void AeSysView::OnUpdateBackgroundImageLoad(CCmdUI* commandUserInterface) {
	commandUserInterface->Enable(static_cast<BOOL>(static_cast<HBITMAP>(m_BackgroundImageBitmap) == nullptr));
}

void AeSysView::OnUpdateBackgroundImageRemove(CCmdUI* commandUserInterface) {
	commandUserInterface->Enable(static_cast<BOOL>(static_cast<HBITMAP>(m_BackgroundImageBitmap) != nullptr));
}

void AeSysView::OnUpdateViewPenWidths(CCmdUI* commandUserInterface) {
	commandUserInterface->SetCheck(static_cast<int>(m_ViewPenWidths));
}

EoDbGroup* AeSysView::RemoveLastVisibleGroup() {
	if (m_VisibleGroupList.IsEmpty() != 0) {
		theApp.AddStringToMessageList(IDS_MSG_NO_DET_GROUPS_IN_VIEW);
		return nullptr;
	}
	return m_VisibleGroupList.RemoveTail();
}

void AeSysView::DeleteLastGroup() {
	const auto Group {RemoveLastVisibleGroup()};
	if (Group != nullptr) {
		auto Document {GetDocument()};
		Document->AnyLayerRemove(Group);
		if (Document->RemoveTrappedGroup(Group) != nullptr) { // Display it normal color so the erase xor will work
			Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
			UpdateStateInformation(kTrapCount);
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
	auto Position {GetFirstVisibleGroupPosition()};
	while (Position != nullptr) {
		auto Group {GetNextVisibleGroup(Position)};
		Group->BreakPolylines();
	}
}

void AeSysView::BreakAllSegRefs() {
	auto Position {GetFirstVisibleGroupPosition()};
	while (Position != nullptr) {
		auto Group {GetNextVisibleGroup(Position)};
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
	m_EngagedGroup = nullptr;
	m_EngagedPrimitive = nullptr;
}

EoDbGroup* AeSysView::SelSegAndPrimAtCtrlPt(const EoGePoint4d& point) {
	OdGePoint3d ControlPoint;
	m_EngagedGroup = nullptr;
	m_EngagedPrimitive = nullptr;
	auto TransformMatrix {ModelViewMatrix()};
	TransformMatrix.invert();
	auto Position {GetFirstVisibleGroupPosition()};
	while (Position != nullptr) {
		auto Group {GetNextVisibleGroup(Position)};
		const auto Primitive {Group->SelectControlPointBy(point, this, &ControlPoint)};
		if (Primitive != nullptr) {
			m_ptDet = ControlPoint;
			m_ptDet.transformBy(TransformMatrix);
			m_EngagedGroup = Group;
			m_EngagedPrimitive = Primitive;
		}
	}
	return m_EngagedGroup;
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

bool AeSysView::GroupIsEngaged() const noexcept {
	return m_EngagedGroup != nullptr;
}

double AeSysView::SelectApertureSize() const noexcept {
	return m_SelectApertureSize;
}

EoDbGroup* AeSysView::SelectGroupAndPrimitive(const OdGePoint3d& point) {
	OdGePoint3d PrimitiveSelectionPoint;
	m_EngagedGroup = nullptr;
	m_EngagedPrimitive = nullptr;
	EoGePoint4d PointInView(point, 1.0);
	ModelViewTransformPoint(PointInView);
	auto TransformMatrix {ModelViewMatrix()};
	TransformMatrix.invert();
	auto SelectionApertureSize {m_SelectApertureSize};
	EoDbHatch::SetEdgeToEvaluate(0);
	EoDbPolyline::SetEdgeToEvaluate(0);
	auto Position {GetFirstVisibleGroupPosition()};
	while (Position != nullptr) {
		auto Group {GetNextVisibleGroup(Position)};
		const auto Primitive {Group->SelectPrimitiveUsingPoint(PointInView, this, SelectionApertureSize, PrimitiveSelectionPoint)};
		if (Primitive != nullptr) {
			m_ptDet = PrimitiveSelectionPoint;
			m_ptDet.transformBy(TransformMatrix);
			m_EngagedGroup = Group;
			m_EngagedPrimitive = Primitive;
			return Group;
		}
	}
	return nullptr;
}

std::pair<EoDbGroup*, EoDbEllipse*> AeSysView::SelectCircleUsingPoint(const OdGePoint3d& point, const double tolerance) {
	auto GroupPosition {GetFirstVisibleGroupPosition()};
	while (GroupPosition != nullptr) {
		auto Group {GetNextVisibleGroup(GroupPosition)};
		auto PrimitivePosition {Group->GetHeadPosition()};
		while (PrimitivePosition != nullptr) {
			const auto Primitive {Group->GetNext(PrimitivePosition)};
			if (Primitive->IsKindOf(RUNTIME_CLASS(EoDbEllipse)) != 0) {
				auto Arc {dynamic_cast<EoDbEllipse*>(Primitive)};
				if (fabs(Arc->SweepAngle() - Oda2PI) <= DBL_EPSILON && Arc->MajorAxis().lengthSqrd() - Arc->MinorAxis().lengthSqrd() <= DBL_EPSILON) {
					if (point.distanceTo(Arc->Center()) <= tolerance) { return {Group, Arc}; }
				}
			}
		}
	}
	return {nullptr, nullptr};
}

std::pair<EoDbGroup*, EoDbLine*> AeSysView::SelectLineUsingPoint(const OdGePoint3d& point) {
	EoGePoint4d PointInView(point, 1.0);
	ModelViewTransformPoint(PointInView);
	auto GroupPosition {GetFirstVisibleGroupPosition()};
	while (GroupPosition != nullptr) {
		auto Group {GetNextVisibleGroup(GroupPosition)};
		auto PrimitivePosition = Group->GetHeadPosition();
		while (PrimitivePosition != nullptr) {
			const auto Primitive {Group->GetNext(PrimitivePosition)};
			if (Primitive->IsKindOf(RUNTIME_CLASS(EoDbLine)) != 0) {
				OdGePoint3d PointOnLine;
				if (Primitive->SelectUsingPoint(PointInView, this, PointOnLine)) {
					return {Group, dynamic_cast<EoDbLine*>(Primitive)};
				}
			}
		}
	}
	return {nullptr, nullptr};
}

EoDbText* AeSysView::SelectTextUsingPoint(const OdGePoint3d& point) {
	EoGePoint4d View(point, 1.0);
	ModelViewTransformPoint(View);
	auto GroupPosition {GetFirstVisibleGroupPosition()};
	while (GroupPosition != nullptr) {
		const auto Group {GetNextVisibleGroup(GroupPosition)};
		auto PrimitivePosition {Group->GetHeadPosition()};
		while (PrimitivePosition != nullptr) {
			const auto Primitive {Group->GetNext(PrimitivePosition)};
			if (Primitive->IsKindOf(RUNTIME_CLASS(EoDbText)) != 0) {
				OdGePoint3d ProjectedPoint;
				if (dynamic_cast<EoDbText*>(Primitive)->SelectUsingPoint(View, this, ProjectedPoint)) { return dynamic_cast<EoDbText*>(Primitive); }
			}
		}
	}
	return nullptr;
}

void AeSysView::OnOp0() {
	switch (theApp.CurrentMode()) {
		case ID_MODE_PRIMITIVE_EDIT: case ID_MODE_GROUP_EDIT:
			OnEditModeOptions();
			break;
		default: ;
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
		default: ;
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
		default: ;
	}
}

void AeSysView::OnOp4() {
	switch (theApp.CurrentMode()) {
		case ID_MODE_PRIMITIVE_EDIT:
			theApp.LoadModeResources(static_cast<unsigned>(theApp.PrimaryMode()));
			GetDocument()->InitializeGroupAndPrimitiveEdit();
			break;
		case ID_MODE_GROUP_EDIT:
			theApp.LoadModeResources(static_cast<unsigned>(theApp.PrimaryMode()));
			GetDocument()->InitializeGroupAndPrimitiveEdit();
			break;
		default: ;
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
		default: ;
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
		default: ;
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
		default: ;
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
		default: ;
	}
}

void AeSysView::OnReturn() {
	switch (theApp.CurrentMode()) {
		case ID_MODE_PRIMITIVE_EDIT:
			theApp.LoadModeResources(static_cast<unsigned>(theApp.PrimaryMode()));
			InitializeGroupAndPrimitiveEdit();
			break;
		case ID_MODE_GROUP_EDIT:
			theApp.LoadModeResources(static_cast<unsigned>(theApp.PrimaryMode()));
			InitializeGroupAndPrimitiveEdit();
			break;
		case ID_MODE_PRIMITIVE_MEND:
			MendPrimitiveReturn();
			break;
		default: ;
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
		default: ;
	}
}

void AeSysView::OnFind() {
	OdString FindComboText;
	VerifyFindString(CMainFrame::GetFindCombo(), FindComboText);
	if (!FindComboText.isEmpty()) {
		TRACE1("AeSysView::OnFind() ComboText = %s\n", static_cast<const wchar_t*>(FindComboText));
	}
}

void AeSysView::VerifyFindString(CMFCToolBarComboBoxButton* findComboBox, OdString& findText) {
	if (findComboBox == nullptr) { return; }
	const auto IsLastCommandFromButton {CMFCToolBar::IsLastCommandFromButton(findComboBox)};
	if (IsLastCommandFromButton != 0) { findText = findComboBox->GetText(); }
	auto ComboBox {findComboBox->GetComboBox()};
	if (!findText.isEmpty()) {
		const auto Count {gsl::narrow_cast<unsigned>(ComboBox->GetCount())};
		unsigned Position {0};
		while (Position < Count) {
			CString ListBoxText;
			ComboBox->GetLBText(static_cast<int>(Position), ListBoxText);
			if (ListBoxText.GetLength() == findText.getLength()) {
				if (static_cast<const wchar_t*>(ListBoxText) == findText) { break; }
			}
			Position++;
		}
		if (Position < Count) { // Text need to move to initial position
			ComboBox->DeleteString(Position);
		}
		ComboBox->InsertString(0, findText);
		ComboBox->SetCurSel(0);
		if (IsLastCommandFromButton == 0) { findComboBox->SetText(findText); }
	}
}

void AeSysView::OnEditFind() noexcept {
}

void AeSysView::RubberBandingDisable() {
	if (m_RubberBandType != kNone) {
		auto DeviceContext {GetDC()};
		if (DeviceContext == nullptr) { return; }
		const auto DrawMode {DeviceContext->SetROP2(R2_XORPEN)};
		CPen GreyPen(PS_SOLID, 0, g_RubberBandColor);
		const auto Pen {DeviceContext->SelectObject(&GreyPen)};
		if (m_RubberBandType == kLines) {
			DeviceContext->MoveTo(m_RubberBandLogicalBeginPoint);
			DeviceContext->LineTo(m_RubberBandLogicalEndPoint);
		} else if (m_RubberBandType == kRectangles) {
			const auto Brush {dynamic_cast<CBrush*>(DeviceContext->SelectStockObject(NULL_BRUSH))};
			DeviceContext->Rectangle(m_RubberBandLogicalBeginPoint.x, m_RubberBandLogicalBeginPoint.y, m_RubberBandLogicalEndPoint.x, m_RubberBandLogicalEndPoint.y);
			DeviceContext->SelectObject(Brush);
		}
		DeviceContext->SelectObject(Pen);
		DeviceContext->SetROP2(DrawMode);
		ReleaseDC(DeviceContext);
		m_RubberBandType = kNone;
	}
}

void AeSysView::RubberBandingStartAtEnable(const OdGePoint3d& point, const RubberBandingTypes type) {
	EoGePoint4d PointInView(point, 1.0);
	ModelViewTransformPoint(PointInView);
	if (PointInView.IsInView()) {
		m_RubberBandBeginPoint = point;
		m_RubberBandLogicalBeginPoint = DoViewportProjection(PointInView);
		m_RubberBandLogicalEndPoint = m_RubberBandLogicalBeginPoint;
	}
	m_RubberBandType = type;
}

OdGePoint3d AeSysView::GetCursorPosition() {
	CPoint CursorPosition;
	GetCursorPos(&CursorPosition);
	ScreenToClient(&CursorPosition);
	const OdGePoint3d Position(static_cast<double>(CursorPosition.x), static_cast<double>(CursorPosition.y), m_ptCursorPosDev.z);
	if (Position != m_ptCursorPosDev) {
		m_ptCursorPosDev = Position;
		m_ptCursorPosWorld = DoViewportProjectionInverse(m_ptCursorPosDev);
		auto Transform = ModelViewMatrix();
		m_ptCursorPosWorld.transformBy(Transform.invert());
		m_ptCursorPosWorld = SnapPointToGrid(m_ptCursorPosWorld);
	}
	return m_ptCursorPosWorld;
}

OdGePoint3d AeSysView::GetWorldCoordinates(const CPoint point) {
	OdGsViewPtr FirstView {m_LayoutHelper->viewAt(0)};
	OdGePoint3d WcsPoint(point.x, point.y, 0.0);
	WcsPoint.transformBy((FirstView->screenMatrix() * FirstView->projectionMatrix()).inverse());
	WcsPoint.z = 0.0;
	WcsPoint.transformBy(OdAbstractViewPEPtr(FirstView)->eyeToWorld(FirstView));
	return WcsPoint;
}

void AeSysView::SetCursorPosition(const OdGePoint3d& point) {
	EoGePoint4d ViewPoint(point, 1.0);
	ModelViewTransformPoint(ViewPoint);
	if (!ViewPoint.IsInView()) { // Redefine the view so position becomes camera target
		OdGsViewPtr FirstView = m_LayoutHelper->viewAt(0);
		const auto DollyVector(point - FirstView->target());
		FirstView->dolly(DollyVector);
		m_ViewTransform.SetView(FirstView->position(), FirstView->target(), FirstView->upVector(), FirstView->fieldWidth(), FirstView->fieldHeight());
		m_ViewTransform.BuildTransformMatrix();
		InvalidateRect(nullptr);
		ViewPoint = EoGePoint4d(point, 1.0);
		ModelViewTransformPoint(ViewPoint);
	}
	auto CursorPosition {DoViewportProjection(ViewPoint)};
	m_ptCursorPosDev.x = CursorPosition.x;
	m_ptCursorPosDev.y = CursorPosition.y;
	m_ptCursorPosDev.z = ViewPoint.z / ViewPoint.W();
	m_ptCursorPosWorld = point;
	ClientToScreen(&CursorPosition);
	SetCursorPos(CursorPosition.x, CursorPosition.y);
}

void AeSysView::SetModeCursor(const unsigned mode) {
	unsigned short ResourceIdentifier;
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
			SetCursor(static_cast<HCURSOR>(LoadImageW(HINSTANCE(nullptr), IDC_CROSS, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE)));
			return;
	}
	auto CursorHandle {static_cast<HCURSOR>(LoadImageW(theApp.GetInstance(), MAKEINTRESOURCEW(ResourceIdentifier), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE))};
	VERIFY(CursorHandle);
	SetCursor(CursorHandle);
	SetClassLongW(this->GetSafeHwnd(), GCLP_HCURSOR, reinterpret_cast<long>(CursorHandle));
}

void AeSysView::SetWorldScale(const double scale) {
	if (scale > FLT_EPSILON) {
		m_WorldScale = scale;
		UpdateStateInformation(kScale);
		auto MainFrame {dynamic_cast<CMainFrame*>(AfxGetMainWnd())};
		MainFrame->GetPropertiesPane().GetActiveViewScaleProperty().SetValue(m_WorldScale);
	}
}

void AeSysView::OnViewStateInformation() {
	m_ViewStateInformation = !m_ViewStateInformation;
	AeSysDoc::GetDoc()->UpdateAllViews(nullptr);
}

void AeSysView::OnUpdateViewStateInformation(CCmdUI* commandUserInterface) {
	commandUserInterface->SetCheck(static_cast<int>(m_ViewStateInformation));
}

void AeSysView::UpdateStateInformation(const StateInformationItem item) {
	if (m_ViewStateInformation) {
		auto Document {AeSysDoc::GetDoc()};
		auto DeviceContext {GetDC()};
		const auto Font {dynamic_cast<CFont*>(DeviceContext->SelectStockObject(SYSTEM_FONT))};
		const auto TextAlignAlignment {DeviceContext->SetTextAlign(TA_LEFT | TA_TOP)};
		const auto TextColor {DeviceContext->SetTextColor(AppGetTextCol())};
		const auto BackgroundColor {DeviceContext->SetBkColor(~AppGetTextCol() & 0x00ffffff)};
		TEXTMETRIC TextMetrics;
		DeviceContext->GetTextMetricsW(&TextMetrics);
		CRect ClientRectangle;
		GetClientRect(&ClientRectangle);
		CRect TextRectangle;
		wchar_t Buffer[32];
		if ((item & kWorkCount) == kWorkCount) {
			TextRectangle.SetRect(0, ClientRectangle.top, 8 * TextMetrics.tmAveCharWidth, ClientRectangle.top + TextMetrics.tmHeight);
			swprintf_s(Buffer, 32, L"%-4i", Document->NumberOfGroupsInWorkLayer() + Document->NumberOfGroupsInActiveLayers());
			DeviceContext->ExtTextOutW(TextRectangle.left, TextRectangle.top, ETO_CLIPPED | ETO_OPAQUE, &TextRectangle, Buffer, wcslen(Buffer), nullptr);
		}
		if ((item & kTrapCount) == kTrapCount) {
			TextRectangle.SetRect(8 * TextMetrics.tmAveCharWidth, ClientRectangle.top, 16 * TextMetrics.tmAveCharWidth, ClientRectangle.top + TextMetrics.tmHeight);
			swprintf_s(Buffer, 32, L"%-4i", Document->TrapGroupCount());
			DeviceContext->ExtTextOutW(TextRectangle.left, TextRectangle.top, ETO_CLIPPED | ETO_OPAQUE, &TextRectangle, Buffer, wcslen(Buffer), nullptr);
		}
		if ((item & kPen) == kPen) {
			TextRectangle.SetRect(16 * TextMetrics.tmAveCharWidth, ClientRectangle.top, 22 * TextMetrics.tmAveCharWidth, ClientRectangle.top + TextMetrics.tmHeight);
			swprintf_s(Buffer, 32, L"P%-4i", g_PrimitiveState.ColorIndex());
			DeviceContext->ExtTextOutW(TextRectangle.left, TextRectangle.top, ETO_CLIPPED | ETO_OPAQUE, &TextRectangle, Buffer, wcslen(Buffer), nullptr);
		}
		if ((item & kLine) == kLine) {
			TextRectangle.SetRect(22 * TextMetrics.tmAveCharWidth, ClientRectangle.top, 28 * TextMetrics.tmAveCharWidth, ClientRectangle.top + TextMetrics.tmHeight);
			swprintf_s(Buffer, 32, L"L%-4i", g_PrimitiveState.LinetypeIndex());
			DeviceContext->ExtTextOutW(TextRectangle.left, TextRectangle.top, ETO_CLIPPED | ETO_OPAQUE, &TextRectangle, Buffer, wcslen(Buffer), nullptr);
		}
		if ((item & kTextHeight) == kTextHeight) {
			const auto CharacterCellDefinition {g_PrimitiveState.CharacterCellDefinition()};
			TextRectangle.SetRect(28 * TextMetrics.tmAveCharWidth, ClientRectangle.top, 38 * TextMetrics.tmAveCharWidth, ClientRectangle.top + TextMetrics.tmHeight);
			swprintf_s(Buffer, 32, L"T%-6.2f", CharacterCellDefinition.Height());
			DeviceContext->ExtTextOutW(TextRectangle.left, TextRectangle.top, ETO_CLIPPED | ETO_OPAQUE, &TextRectangle, Buffer, wcslen(Buffer), nullptr);
		}
		if ((item & kScale) == kScale) {
			TextRectangle.SetRect(38 * TextMetrics.tmAveCharWidth, ClientRectangle.top, 48 * TextMetrics.tmAveCharWidth, ClientRectangle.top + TextMetrics.tmHeight);
			swprintf_s(Buffer, 32, L"1:%-6.2f", WorldScale());
			DeviceContext->ExtTextOutW(TextRectangle.left, TextRectangle.top, ETO_CLIPPED | ETO_OPAQUE, &TextRectangle, Buffer, wcslen(Buffer), nullptr);
		}
		if ((item & kWndRatio) == kWndRatio) {
			TextRectangle.SetRect(48 * TextMetrics.tmAveCharWidth, ClientRectangle.top, 58 * TextMetrics.tmAveCharWidth, ClientRectangle.top + TextMetrics.tmHeight);
			CString ZoomFactorAsString;
			ZoomFactorAsString.Format(L"=%-8.3f", ZoomFactor());
			DeviceContext->ExtTextOutW(TextRectangle.left, TextRectangle.top, ETO_CLIPPED | ETO_OPAQUE, &TextRectangle, ZoomFactorAsString, nullptr);
		}
		if ((item & kDimLen) == kDimLen || (item & kDimAng) == kDimAng) {
			TextRectangle.SetRect(58 * TextMetrics.tmAveCharWidth, ClientRectangle.top, 90 * TextMetrics.tmAveCharWidth, ClientRectangle.top + TextMetrics.tmHeight);
			CString LengthAndAngle;
			LengthAndAngle += theApp.FormatLength(theApp.DimensionLength(), theApp.GetUnits());
			LengthAndAngle += L" @ ";
			LengthAndAngle += AeSys::FormatAngle(EoToRadian(theApp.DimensionAngle()));
			DeviceContext->ExtTextOutW(TextRectangle.left, TextRectangle.top, ETO_CLIPPED | ETO_OPAQUE, &TextRectangle, LengthAndAngle, nullptr);
		}
		DeviceContext->SetBkColor(BackgroundColor);
		DeviceContext->SetTextColor(TextColor);
		DeviceContext->SetTextAlign(TextAlignAlignment);
		DeviceContext->SelectObject(Font);
		ReleaseDC(DeviceContext);
	}
}

const ODCOLORREF* AeSysView::CurrentPalette() const {
	const auto Color {odcmAcadPalette(m_Background)};
	return Color;
}

void AeSysView::SetRenderMode(const OdGsView::RenderMode renderMode) {
	OdGsViewPtr FirstView {m_LayoutHelper->viewAt(0)};
	if (FirstView->mode() != renderMode) {
		FirstView->setMode(renderMode);
		if (FirstView->mode() != renderMode) {
			::MessageBoxW(nullptr, L"Render mode is not supported by the current device", L"Open Design Alliance", MB_ICONWARNING);
		} else {
			m_ViewTransform.SetRenderMode(renderMode);
			InvalidateRect(nullptr);
		}
	}
}

void AeSysView::ZoomWindow(OdGePoint3d point1, OdGePoint3d point2) {
	OdGsViewPtr FirstView = m_LayoutHelper->viewAt(0);
	const auto WorldToEye {OdAbstractViewPEPtr(FirstView)->worldToEye(FirstView)};
	point1.transformBy(WorldToEye);
	point2.transformBy(WorldToEye);
	auto Vector = point2 - point1;
	if (OdNonZero(Vector.x) && OdNonZero(Vector.y)) {
		auto NewPosition = point1 + Vector / 2.;
		Vector.x = fabs(Vector.x);
		Vector.y = fabs(Vector.y);
		FirstView->dolly(NewPosition.asVector());
		const auto FieldWidth {FirstView->fieldWidth() / Vector.x};
		const auto FieldHeight {FirstView->fieldHeight() / Vector.y};
		FirstView->zoom(odmin(FieldWidth, FieldHeight));
		InvalidateRect(nullptr);
	}
}

void AeSysView::OnInsertBlockReference() {
	// <tas="Just a placeholder for BlockReference. It works but position, scale & rotation need to be specified."</tas>
	auto Document {GetDocument()};
	if (Document->BlockTableSize() > 0) {
		EoDlgBlockInsert Dialog(Document);
		Dialog.DoModal();
	}
}
