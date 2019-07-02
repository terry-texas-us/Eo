#pragma once
void WndProcPreviewClear(HWND previewWindow);

void WndProcPreviewUpdate(HWND previewWindow, EoDbBlock* block);

void _WndProcPreviewUpdate(HWND previewWindow, EoDbGroupList* groups);
