#include "stdafx.h"

#include "AeSysView.h"

#include "EoDbBlock.h"

#include "Preview.h"

CBitmap* WndProcPreview_Bitmap = NULL;

LRESULT CALLBACK WndProcPreview(HWND, UINT, WPARAM, LPARAM);

ATOM WINAPI RegisterPreviewWindowClass(HINSTANCE instance) noexcept {
	WNDCLASS Class;

	Class.style = CS_HREDRAW | CS_VREDRAW;
	Class.lpfnWndProc = WndProcPreview;
	Class.cbClsExtra = 0;
	Class.cbWndExtra = 0;
	Class.hInstance = instance;
	Class.hIcon = 0;
	Class.hCursor = static_cast<HCURSOR>(::LoadImageW(HINSTANCE(NULL), IDC_CROSS, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE));
	Class.hbrBackground	= (HBRUSH) ::GetStockObject(BLACK_BRUSH);
	Class.lpszMenuName = 0;
	Class.lpszClassName	= L"PreviewWindow";

	return ::RegisterClass(&Class);
}
LRESULT CALLBACK WndProcPreview(HWND hwnd, UINT message, UINT nParam, LPARAM lParam) {
	switch (message) {
		case WM_CREATE: {
			auto ActiveView {AeSysView::GetActiveView()};
			CDC* DeviceContext = (ActiveView) ? ActiveView->GetDC() : NULL;

			CRect rc;
			::GetClientRect(hwnd, &rc);
			WndProcPreview_Bitmap = new CBitmap;
			WndProcPreview_Bitmap->CreateCompatibleBitmap(DeviceContext, int(rc.right), int(rc.bottom));
		}
		return (FALSE);

		case WM_DESTROY:
			if (WndProcPreview_Bitmap != NULL) {
				delete WndProcPreview_Bitmap;
				WndProcPreview_Bitmap = NULL;
			}
			return (FALSE);

		case WM_PAINT: {
			PAINTSTRUCT ps;

			CRect rc;
			::GetClientRect(hwnd, &rc);

			CDC dc;
			dc.Attach(::BeginPaint(hwnd, &ps));

			CDC dcMem;
			dcMem.CreateCompatibleDC(NULL);

			CBitmap* Bitmap = dcMem.SelectObject(WndProcPreview_Bitmap);
			dc.BitBlt(0, 0, rc.right, rc.bottom, &dcMem, 0, 0, SRCCOPY);
			dcMem.SelectObject(Bitmap);

			dc.Detach();

			::EndPaint(hwnd, &ps);
		}

					   return (FALSE);

		case WM_LBUTTONDOWN:
			::SetFocus(hwnd);
			ATLTRACE2(atlTraceGeneral, 0, L"Preview WM_LBUTTONDOWN message\n");
			return (FALSE);

	}
	return DefWindowProc(hwnd, message, nParam, lParam);
}

void WndProcPreviewClear(HWND previewWindow) {
	CRect rc;
	::GetClientRect(previewWindow, &rc);

	CDC dcMem;
	dcMem.CreateCompatibleDC(0);

	CBitmap* Bitmap = (CBitmap*) dcMem.SelectObject(WndProcPreview_Bitmap);
	dcMem.PatBlt(0, 0, rc.right, rc.bottom, BLACKNESS);

	dcMem.SelectObject(Bitmap);
	::InvalidateRect(previewWindow, 0, TRUE);
}

void WndProcPreviewUpdate(HWND previewWindow, EoDbBlock* block) {
	auto ActiveView {AeSysView::GetActiveView()};

	CRect rc;
	::GetClientRect(previewWindow, &rc);

	CDC dcMem;
	dcMem.CreateCompatibleDC(NULL);

	CBitmap* Bitmap = dcMem.SelectObject(WndProcPreview_Bitmap);
	dcMem.PatBlt(0, 0, rc.right, rc.bottom, BLACKNESS);

	ActiveView->ViewportPushActive();
	ActiveView->SetViewportSize(rc.right, rc.bottom);
	ActiveView->SetDeviceWidthInInches(static_cast<double>(dcMem.GetDeviceCaps(HORZSIZE)) / EoMmPerInch);
	ActiveView->SetDeviceHeightInInches(static_cast<double>(dcMem.GetDeviceCaps(VERTSIZE)) / EoMmPerInch);

	OdGeExtents3d Extents;
	block->GetExtents_(ActiveView, Extents);
	const OdGePoint3d MinimumPoint = Extents.minPoint();
	const OdGePoint3d MaximumPoint = Extents.maxPoint();
	
	ActiveView->PushViewTransform();

	double FieldWidth = MaximumPoint.x - MinimumPoint.x;
	double FieldHeight = MaximumPoint.y - MinimumPoint.y;

	const double AspectRatio = ActiveView->ViewportHeightInInches() / ActiveView->ViewportWidthInInches();
	if (AspectRatio < FieldHeight / FieldWidth) {
		FieldWidth = FieldHeight / AspectRatio;
	} else {
		FieldHeight = FieldWidth * AspectRatio;
	}
	const OdGePoint3d Target((MinimumPoint.x + MaximumPoint.x) / 2., (MinimumPoint.y + MaximumPoint.y) / 2., 0.0);
	const OdGePoint3d Position(Target + OdGeVector3d::kZAxis * 50.);

	ActiveView->SetView(Position, Target, OdGeVector3d::kYAxis, FieldWidth, FieldHeight);

	const int PrimitiveState = pstate.Save();
	block->Display(ActiveView, &dcMem);
	pstate.Restore(&dcMem, PrimitiveState);

	ActiveView->PopViewTransform();
	ActiveView->ViewportPopActive();

	dcMem.SelectObject(Bitmap);
	::InvalidateRect(previewWindow, 0, TRUE);
}

void _WndProcPreviewUpdate(HWND previewWindow, EoDbGroupList* groups) {
	auto ActiveView {AeSysView::GetActiveView()};

	CRect rc;
	::GetClientRect(previewWindow, &rc);

	CDC dcMem;
	dcMem.CreateCompatibleDC(NULL);

	CBitmap* Bitmap = dcMem.SelectObject(WndProcPreview_Bitmap);
	dcMem.PatBlt(0, 0, rc.right, rc.bottom, BLACKNESS);

	ActiveView->ViewportPushActive();
	ActiveView->SetViewportSize(rc.right, rc.bottom);
	ActiveView->SetDeviceWidthInInches(static_cast<double>(dcMem.GetDeviceCaps(HORZSIZE)) / EoMmPerInch);
	ActiveView->SetDeviceHeightInInches(static_cast<double>(dcMem.GetDeviceCaps(VERTSIZE)) / EoMmPerInch);

	OdGeExtents3d Extents;
	groups->GetExtents__(ActiveView, Extents);
	const OdGePoint3d MinimumPoint = Extents.minPoint();
	const OdGePoint3d MaximumPoint = Extents.maxPoint();

	ActiveView->PushViewTransform();
	
	double FieldWidth = MaximumPoint.x - MinimumPoint.x;
	double FieldHeight = MaximumPoint.y - MinimumPoint.y;

	const double AspectRatio = ActiveView->ViewportHeightInInches() / ActiveView->ViewportWidthInInches();
	if (AspectRatio < FieldHeight / FieldWidth) {
		FieldWidth = FieldHeight / AspectRatio;
	} else {
		FieldHeight = FieldWidth * AspectRatio;
	}
	const OdGePoint3d Target((MinimumPoint.x + MaximumPoint.x) / 2., (MinimumPoint.y + MaximumPoint.y) / 2., 0.0);
	const OdGePoint3d Position(Target + OdGeVector3d::kZAxis * 50.);

	ActiveView->SetView(Position, Target, OdGeVector3d::kYAxis, FieldWidth, FieldHeight);
	
	const int PrimitiveState = pstate.Save();
	groups->Display(ActiveView, &dcMem);
	pstate.Restore(&dcMem, PrimitiveState);

	ActiveView->PopViewTransform();
	ActiveView->ViewportPopActive();

	dcMem.SelectObject(Bitmap);
	::InvalidateRect(previewWindow, 0, TRUE);
}
