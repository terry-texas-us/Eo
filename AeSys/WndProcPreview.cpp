#include "stdafx.h"

#include "AeSysView.h"

#include "EoDbBlock.h"

#include "Preview.h"

CBitmap* g_WndProcPreview_Bitmap = nullptr;

LRESULT CALLBACK WndProcPreview(HWND, unsigned, WPARAM, LPARAM);

ATOM WINAPI RegisterPreviewWindowClass(HINSTANCE instance) noexcept {
	WNDCLASS Class;

	Class.style = CS_HREDRAW | CS_VREDRAW;
	Class.lpfnWndProc = WndProcPreview;
	Class.cbClsExtra = 0;
	Class.cbWndExtra = 0;
	Class.hInstance = instance;
	Class.hIcon = nullptr;
	Class.hCursor = static_cast<HCURSOR>(LoadImageW(HINSTANCE(nullptr), IDC_CROSS, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE));
	Class.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	Class.lpszMenuName = nullptr;
	Class.lpszClassName = L"PreviewWindow";

	return ::RegisterClass(&Class);
}
LRESULT CALLBACK WndProcPreview(HWND hwnd, unsigned message, unsigned nParam, LPARAM lParam) {
	switch (message) {
		case WM_CREATE: {
			auto ActiveView {AeSysView::GetActiveView()};
			auto DeviceContext {ActiveView ? ActiveView->GetDC() : nullptr};

			CRect rc;
			GetClientRect(hwnd, &rc);
			g_WndProcPreview_Bitmap = new CBitmap;
			g_WndProcPreview_Bitmap->CreateCompatibleBitmap(DeviceContext, int(rc.right), int(rc.bottom));
		}
			return FALSE;

		case WM_DESTROY:
			if (g_WndProcPreview_Bitmap != nullptr) {
				delete g_WndProcPreview_Bitmap;
				g_WndProcPreview_Bitmap = nullptr;
			}
			return FALSE;

		case WM_PAINT: {
			PAINTSTRUCT ps;

			CRect rc;
			GetClientRect(hwnd, &rc);

			CDC dc;
			dc.Attach(BeginPaint(hwnd, &ps));

			CDC dcMem;
			dcMem.CreateCompatibleDC(NULL);

			auto Bitmap {dcMem.SelectObject(g_WndProcPreview_Bitmap)};
			dc.BitBlt(0, 0, rc.right, rc.bottom, &dcMem, 0, 0, SRCCOPY);
			dcMem.SelectObject(Bitmap);

			dc.Detach();

			EndPaint(hwnd, &ps);
		}
			return FALSE;

		case WM_LBUTTONDOWN:
			SetFocus(hwnd);
			return FALSE;
	}
	return DefWindowProc(hwnd, message, nParam, lParam);
}

void WndProcPreviewClear(HWND previewWindow) {
	CRect rc;
	GetClientRect(previewWindow, &rc);

	CDC dcMem;
	dcMem.CreateCompatibleDC(0);

	CBitmap* Bitmap = static_cast<CBitmap*>(dcMem.SelectObject(g_WndProcPreview_Bitmap));
	dcMem.PatBlt(0, 0, rc.right, rc.bottom, BLACKNESS);

	dcMem.SelectObject(Bitmap);
	InvalidateRect(previewWindow, 0, TRUE);
}

void WndProcPreviewUpdate(HWND previewWindow, EoDbBlock* block) {
	auto ActiveView {AeSysView::GetActiveView()};

	CRect rc;
	GetClientRect(previewWindow, &rc);

	CDC dcMem;
	dcMem.CreateCompatibleDC(NULL);

	auto Bitmap {dcMem.SelectObject(g_WndProcPreview_Bitmap)};
	dcMem.PatBlt(0, 0, rc.right, rc.bottom, BLACKNESS);

	ActiveView->ViewportPushActive();
	ActiveView->SetViewportSize(rc.right, rc.bottom);
	ActiveView->SetDeviceWidthInInches(static_cast<double>(dcMem.GetDeviceCaps(HORZSIZE)) / kMmPerInch);
	ActiveView->SetDeviceHeightInInches(static_cast<double>(dcMem.GetDeviceCaps(VERTSIZE)) / kMmPerInch);

	OdGeExtents3d Extents;
	block->GetExtents_(ActiveView, Extents);
	const auto MinimumPoint {Extents.minPoint()};
	const auto MaximumPoint {Extents.maxPoint()};

	ActiveView->PushViewTransform();

	auto FieldWidth {MaximumPoint.x - MinimumPoint.x};
	auto FieldHeight {MaximumPoint.y - MinimumPoint.y};

	const auto AspectRatio {ActiveView->ViewportHeightInInches() / ActiveView->ViewportWidthInInches()};
	if (AspectRatio < FieldHeight / FieldWidth) {
		FieldWidth = FieldHeight / AspectRatio;
	} else {
		FieldHeight = FieldWidth * AspectRatio;
	}
	const OdGePoint3d Target((MinimumPoint.x + MaximumPoint.x) / 2., (MinimumPoint.y + MaximumPoint.y) / 2., 0.0);
	const auto Position(Target + OdGeVector3d::kZAxis * 50.);

	ActiveView->SetView(Position, Target, OdGeVector3d::kYAxis, FieldWidth, FieldHeight);

	const auto PrimitiveState {pstate.Save()};
	block->Display(ActiveView, &dcMem);
	pstate.Restore(dcMem, PrimitiveState);

	ActiveView->PopViewTransform();
	ActiveView->ViewportPopActive();

	dcMem.SelectObject(Bitmap);
	InvalidateRect(previewWindow, 0, TRUE);
}

void _WndProcPreviewUpdate(HWND previewWindow, EoDbGroupList* groups) {
	auto ActiveView {AeSysView::GetActiveView()};

	CRect rc;
	GetClientRect(previewWindow, &rc);

	CDC dcMem;
	dcMem.CreateCompatibleDC(NULL);

	auto Bitmap {dcMem.SelectObject(g_WndProcPreview_Bitmap)};
	dcMem.PatBlt(0, 0, rc.right, rc.bottom, BLACKNESS);

	ActiveView->ViewportPushActive();
	ActiveView->SetViewportSize(rc.right, rc.bottom);
	ActiveView->SetDeviceWidthInInches(static_cast<double>(dcMem.GetDeviceCaps(HORZSIZE)) / kMmPerInch);
	ActiveView->SetDeviceHeightInInches(static_cast<double>(dcMem.GetDeviceCaps(VERTSIZE)) / kMmPerInch);

	OdGeExtents3d Extents;
	groups->GetExtents__(ActiveView, Extents);
	const auto MinimumPoint {Extents.minPoint()};
	const auto MaximumPoint {Extents.maxPoint()};

	ActiveView->PushViewTransform();

	auto FieldWidth {MaximumPoint.x - MinimumPoint.x};
	auto FieldHeight {MaximumPoint.y - MinimumPoint.y};

	const auto AspectRatio {ActiveView->ViewportHeightInInches() / ActiveView->ViewportWidthInInches()};
	if (AspectRatio < FieldHeight / FieldWidth) {
		FieldWidth = FieldHeight / AspectRatio;
	} else {
		FieldHeight = FieldWidth * AspectRatio;
	}
	const OdGePoint3d Target((MinimumPoint.x + MaximumPoint.x) / 2., (MinimumPoint.y + MaximumPoint.y) / 2., 0.0);
	const auto Position(Target + OdGeVector3d::kZAxis * 50.);

	ActiveView->SetView(Position, Target, OdGeVector3d::kYAxis, FieldWidth, FieldHeight);

	const auto PrimitiveState {pstate.Save()};
	groups->Display(ActiveView, &dcMem);
	pstate.Restore(dcMem, PrimitiveState);

	ActiveView->PopViewTransform();
	ActiveView->ViewportPopActive();

	dcMem.SelectObject(Bitmap);
	InvalidateRect(previewWindow, 0, TRUE);
}
