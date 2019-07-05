#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include <DbLayerTable.h>
#include "EoDb.h"
#include "Preview.h"

unsigned CALLBACK OfnHookProcFileTracing(HWND hDlg, const unsigned windowMessage, WPARAM wParam, LPARAM lParam) {
	auto Document {AeSysDoc::GetDoc()};
	switch (windowMessage) {
		case WM_INITDIALOG:
			WndProcPreviewClear(GetDlgItem(hDlg, IDC_LAYER_PREVIEW));
			return TRUE;
		case WM_NOTIFY: {
			const auto lpofn {reinterpret_cast<_OFNOTIFYW*>(lParam)};
			if (lpofn->hdr.code == CDN_FOLDERCHANGE) {
				WndProcPreviewClear(GetDlgItem(hDlg, IDC_LAYER_PREVIEW));
			} else if (lpofn->hdr.code == CDN_SELCHANGE) {
				wchar_t FilePath[MAX_PATH] {L"\0"};
				::SendMessage(GetParent(hDlg), CDM_GETFILEPATH, MAX_PATH, reinterpret_cast<LPARAM>(static_cast<wchar_t*>(FilePath)));
				CFileStatus FileStatus;
				if (CFile::GetStatus(FilePath, FileStatus) != 0) {
					const auto FileType {AeSys::GetFileType(FilePath)};
					if (FileType == EoDb::kTracing || FileType == EoDb::kJob) {
						auto Layer {Document->GetLayerAt(FilePath)};
						const auto PreviewWindow {GetDlgItem(hDlg, IDC_LAYER_PREVIEW)};
						if (Layer != nullptr) {
							_WndProcPreviewUpdate(PreviewWindow, Layer);
						} else {
							Layer = new EoDbLayer(L"", EoDbLayer::kIsResident | EoDbLayer::kIsInternal | EoDbLayer::kIsActive);
							Document->TracingLoadLayer(FilePath, Layer);
							_WndProcPreviewUpdate(PreviewWindow, Layer);
							Layer->DeleteGroupsAndRemoveAll();
							delete Layer;
						}
					}
				}
			}
			return TRUE;
		}
		case WM_COMMAND: {
			wchar_t FilePath[MAX_PATH] {L"\0"};
			::SendMessage(GetParent(hDlg), CDM_GETFILEPATH, MAX_PATH, reinterpret_cast<LPARAM>(static_cast<wchar_t*>(FilePath)));
			CFileStatus FileStatus;
			if (CFile::GetStatus(FilePath, FileStatus) == 0) {
				AeSys::WarningMessageBox(IDS_MSG_FILE_NOT_FOUND, FilePath);
				return TRUE;
			}
			const auto Name {PathFindFileNameW(FilePath)};
			const auto FileType {AeSys::GetFileType(FilePath)};
			if (FileType != EoDb::kTracing && FileType != EoDb::kJob) {
				AeSys::WarningMessageBox(IDS_MSG_INVALID_TRACING_FILE_NAME, FilePath);
				return TRUE;
			}
			switch (LOWORD(wParam)) {
				case IDC_APPEND: {
					const auto Layer {Document->GetWorkLayer()};
					Document->TracingLoadLayer(FilePath, Layer);
					Document->UpdateLayerInAllViews(EoDb::kLayerSafe, Layer);
					return TRUE;
				}
				case IDC_MAP: {
					auto FileOpenSuccess {false};
					auto Layer {Document->GetLayerAt(Name)};
					if (Layer != nullptr) {
						if (Layer->IsCurrent()) {
							AeSys::WarningMessageBox(IDS_MSG_CLOSE_TRACING_FIRST, Name);
						} else {
							FileOpenSuccess = true;
						}
					} else {
						const auto Layers {Document->LayerTable(OdDb::kForWrite)};
						auto LayerTableRecord {OdDbLayerTableRecord::createObject()};
						LayerTableRecord->setName(Name);
						Layer = new EoDbLayer(LayerTableRecord);
						FileOpenSuccess = Document->TracingLoadLayer(FilePath, Layer);
						if (FileOpenSuccess) {
							Document->AddLayerTo(Layers, Layer);
						} else {
							delete Layer;
							LayerTableRecord->erase(true);
						}
					}
					if (FileOpenSuccess) {
						Layer->MakeActive();
						Document->UpdateLayerInAllViews(EoDb::kLayerSafe, Layer);
					}
					return TRUE;
				}
				case IDC_TRAP: {
					auto pLayer {new EoDbLayer(L"", EoDbLayer::kIsResident | EoDbLayer::kIsInternal | EoDbLayer::kIsActive)};
					Document->TracingLoadLayer(FilePath, pLayer);
					Document->RemoveAllTrappedGroups();
					Document->AddGroupsToTrap(pLayer);
					Document->CopyTrappedGroupsToClipboard(AeSysView::GetActiveView());
					Document->RemoveAllTrappedGroups();
					pLayer->DeleteGroupsAndRemoveAll();
					delete pLayer;
					return TRUE;
				}
				case IDC_VIEW: {
					auto FileOpenSuccess {false};
					auto Layer {Document->GetLayerAt(Name)};
					if (Layer != nullptr) {
						if (Layer->IsCurrent()) {
							AeSys::WarningMessageBox(IDS_MSG_CLOSE_TRACING_FIRST, Name);
						} else {
							FileOpenSuccess = true;
						}
					} else {
						const auto Layers {Document->LayerTable(OdDb::kForWrite)};
						auto LayerTableRecord = OdDbLayerTableRecord::createObject();
						LayerTableRecord->setName(Name);
						Layer = new EoDbLayer(LayerTableRecord);
						FileOpenSuccess = Document->TracingLoadLayer(FilePath, Layer);
						if (FileOpenSuccess) {
							Document->AddLayerTo(Layers, Layer);
						} else {
							delete Layer;
							LayerTableRecord->erase(true);
						}
					}
					if (FileOpenSuccess) {
						Layer->SetIsLocked(true);
						Document->UpdateLayerInAllViews(EoDb::kLayerSafe, Layer);
					}
					return TRUE;
				}
				default: ;
			}
		}
		default: ;
	}
	return FALSE;
}
