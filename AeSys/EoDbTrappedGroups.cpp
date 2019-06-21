#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

void AeSysDoc::AddGroupsToTrap(EoDbGroupList* groups) {
	if (theApp.IsTrapHighlighted()) { UpdateGroupsInAllViews(EoDb::kGroupsSafeTrap, groups); }
	m_TrappedGroupList.AddTail(groups);
}

POSITION AeSysDoc::AddGroupToTrap(EoDbGroup* group) {
	if (theApp.IsTrapHighlighted()) {
		UpdateGroupInAllViews(EoDb::kGroupSafeTrap, group);
	}
	return m_TrappedGroupList.AddTail(group);
}

void AeSysDoc::CompressTrappedGroups() {
	if (m_TrappedGroupList.GetCount() <= 1) { return; }
	auto NewGroup {new EoDbGroup};
	auto GroupPosition {m_TrappedGroupList.GetHeadPosition()};
	while (GroupPosition != nullptr) {
		auto Group {m_TrappedGroupList.GetNext(GroupPosition)};
		AnyLayerRemove(Group);
		RemoveGroupFromAllViews(Group);
		NewGroup->AddTail(Group);
		// delete the original group but not its primitives
		delete Group;
	}
	// emtpy trap group list
	m_TrappedGroupList.RemoveAll();
	AddWorkLayerGroup(NewGroup);
	m_TrappedGroupList.AddTail(NewGroup);
	NewGroup->SortTextOnY();
}

void AeSysDoc::CopyTrappedGroups(const OdGeVector3d& translate) {
	EoGeMatrix3d TranslationMatrix;
	TranslationMatrix.setToTranslation(translate);
	auto GroupPosition {m_TrappedGroupList.GetHeadPosition()};
	while (GroupPosition != nullptr) {
		auto Group {m_TrappedGroupList.GetNext(GroupPosition)};
		auto NewGroup {new EoDbGroup(*Group)};
		AddWorkLayerGroup(NewGroup);
		UpdateGroupInAllViews(EoDb::kGroup, Group);
		Group->TransformBy(TranslationMatrix);
		const LPARAM Hint = theApp.IsTrapHighlighted() ? EoDb::kGroupSafeTrap : EoDb::kGroupSafe;
		UpdateGroupInAllViews(Hint, Group);
	}
}

void AeSysDoc::CopyTrappedGroupsToClipboard(AeSysView* view) {
	OpenClipboard(nullptr);
	EmptyClipboard();
	if (theApp.IsClipboardDataText()) {
		CString strBuf;
		auto GroupPosition {GetFirstTrappedGroupPosition()};
		while (GroupPosition != nullptr) {
			const auto Group {GetNextTrappedGroup(GroupPosition)};
			auto PrimitivePosition {Group->GetHeadPosition()};
			while (PrimitivePosition != nullptr) {
				auto Primitive {Group->GetNext(PrimitivePosition)};
				if (Primitive->IsKindOf(RUNTIME_CLASS(EoDbText))) {
					strBuf += dynamic_cast<EoDbText*>(Primitive)->Text();
					strBuf += L"\r\n";
				}
			}
		}
		const auto AllocationSize {(strBuf.GetLength() + 1) * sizeof(wchar_t)};
		GLOBALHANDLE ClipboardDataHandle = static_cast<GLOBALHANDLE>(GlobalAlloc(GHND, AllocationSize));
		if (ClipboardDataHandle != nullptr) {
			auto ClipboardData {static_cast<wchar_t*>(GlobalLock(ClipboardDataHandle))};
			if (ClipboardData != nullptr) {
				wcscpy_s(ClipboardData, AllocationSize, strBuf);
				GlobalUnlock(ClipboardDataHandle);
				SetClipboardData(CF_UNICODETEXT, ClipboardDataHandle);
			}
		}
	}
	if (theApp.IsClipboardDataImage()) {
		const auto PrimitiveState {g_PrimitiveState.Save()};
		auto MetaFile {CreateEnhMetaFileW(nullptr, nullptr, nullptr, nullptr)};
		m_TrappedGroupList.Display(view, CDC::FromHandle(MetaFile));
		auto MetaFileHandle {CloseEnhMetaFile(MetaFile)};
		SetClipboardData(CF_ENHMETAFILE, MetaFileHandle);
		auto DeviceContext {CDC::FromHandle(MetaFile)};
		if (DeviceContext) { g_PrimitiveState.Restore(*DeviceContext, PrimitiveState); }
	}
	if (theApp.IsClipboardDataGroups()) {
		CMemFile MemoryFile;
		MemoryFile.SetLength(96);
		MemoryFile.SeekToEnd();
		auto Buffer {new unsigned char[EoDbPrimitive::BUFFER_SIZE]};
		m_TrappedGroupList.Write(MemoryFile, Buffer);
		delete[] Buffer;
		OdGeExtents3d Extents;
		m_TrappedGroupList.GetExtents__(view, Extents);
		const auto MinimumPoint {Extents.minPoint()};
		const auto SizeOfBuffer {MemoryFile.GetLength()};
		MemoryFile.SeekToBegin();
		MemoryFile.Write(&SizeOfBuffer, sizeof(unsigned long));
		MemoryFile.Write(&MinimumPoint.x, sizeof(double));
		MemoryFile.Write(&MinimumPoint.y, sizeof(double));
		MemoryFile.Write(&MinimumPoint.z, sizeof(double));
		auto ClipboardDataHandle {GlobalAlloc(GHND, SIZE_T(SizeOfBuffer))};
		if (ClipboardDataHandle != nullptr) {
			auto ClipboardData {static_cast<wchar_t*>(GlobalLock(ClipboardDataHandle))};
			MemoryFile.SeekToBegin();
			MemoryFile.Read(ClipboardData, gsl::narrow_cast<unsigned>(SizeOfBuffer));
			GlobalUnlock(ClipboardDataHandle);
			SetClipboardData(theApp.ClipboardFormatIdentifierForEoGroups(), ClipboardDataHandle);
		}
	}
	CloseClipboard();
}

void AeSysDoc::DeleteAllTrappedGroups() {
	auto GroupPosition {m_TrappedGroupList.GetHeadPosition()};
	while (GroupPosition != nullptr) {
		auto Group {m_TrappedGroupList.GetNext(GroupPosition)};
		AnyLayerRemove(Group);
		RemoveGroupFromAllViews(Group);
		Group->DeletePrimitivesAndRemoveAll();
		delete Group;
	}
	m_TrappedGroupList.RemoveAll();
}

void AeSysDoc::ExpandTrappedGroups() {
	if (m_TrappedGroupList.IsEmpty()) { return; }
	EoDbGroupList* Groups = new EoDbGroupList;
	Groups->AddTail(&m_TrappedGroupList);
	m_TrappedGroupList.RemoveAll();
	auto GroupPosition {Groups->GetHeadPosition()};
	while (GroupPosition != nullptr) {
		auto Group {Groups->GetNext(GroupPosition)};
		auto PrimitivePosition {Group->GetHeadPosition()};
		while (PrimitivePosition != nullptr) {
			auto Primitive {Group->GetNext(PrimitivePosition)};
			auto NewGroup {new EoDbGroup};
			NewGroup->AddTail(Primitive);
			AddWorkLayerGroup(NewGroup);
			m_TrappedGroupList.AddTail(NewGroup);
		}
		AnyLayerRemove(Group);
		RemoveGroupFromAllViews(Group);
		delete Group;
	}
	delete Groups;
}

POSITION AeSysDoc::FindTrappedGroup(EoDbGroup* group) {
	return m_TrappedGroupList.Find(group);
}

POSITION AeSysDoc::GetFirstTrappedGroupPosition() const {
	return m_TrappedGroupList.GetHeadPosition();
}

EoDbGroup* AeSysDoc::GetNextTrappedGroup(POSITION& position) {
	return m_TrappedGroupList.GetNext(position);
}

EoDbGroupList* AeSysDoc::GroupsInTrap() noexcept {
	return &m_TrappedGroupList;
}

bool AeSysDoc::IsTrapEmpty() const {
	return m_TrappedGroupList.IsEmpty();
}

void AeSysDoc::ModifyTrappedGroupsColorIndex(short colorIndex) {
	m_TrappedGroupList.ModifyColorIndex(colorIndex);
}

void AeSysDoc::ModifyTrappedGroupsLinetypeIndex(short linetypeIndex) {
	m_TrappedGroupList.ModifyLinetypeIndex(linetypeIndex);
}

void AeSysDoc::ModifyTrappedGroupsNoteAttributes(EoDbFontDefinition& fontDef, EoDbCharacterCellDefinition& cellDef, int attributes) {
	m_TrappedGroupList.ModifyNotes(fontDef, cellDef, attributes);
}

void AeSysDoc::RemoveAllTrappedGroups() {
	if (!m_TrappedGroupList.IsEmpty()) {
		if (theApp.IsTrapHighlighted()) {
			UpdateGroupsInAllViews(EoDb::kGroupsSafe, &m_TrappedGroupList);
		}
		m_TrappedGroupList.RemoveAll();
	}
}

EoDbGroup* AeSysDoc::RemoveLastTrappedGroup() {
	return m_TrappedGroupList.RemoveTail();
}

POSITION AeSysDoc::RemoveTrappedGroup(EoDbGroup* group) {
	return m_TrappedGroupList.Remove(group);
}

void AeSysDoc::RemoveTrappedGroupAt(POSITION position) {
	m_TrappedGroupList.RemoveAt(position);
}

void AeSysDoc::SetTrapPivotPoint(const OdGePoint3d& pivotPoint) noexcept {
	m_TrapPivotPoint = pivotPoint;
}

void AeSysDoc::SquareTrappedGroups(AeSysView* view) {
	UpdateGroupsInAllViews(EoDb::kGroupsEraseSafeTrap, &m_TrappedGroupList);
	auto GroupPosition {m_TrappedGroupList.GetHeadPosition()};
	while (GroupPosition != nullptr) {
		auto Group {m_TrappedGroupList.GetNext(GroupPosition)};
		Group->Square(view);
	}
	UpdateGroupsInAllViews(EoDb::kGroupsSafeTrap, &m_TrappedGroupList);
}

void AeSysDoc::TransformTrappedGroups(const EoGeMatrix3d& transformMatrix) {
	if (theApp.IsTrapHighlighted()) {
		UpdateGroupsInAllViews(EoDb::kGroupsEraseSafeTrap, &m_TrappedGroupList);
	}
	m_TrappedGroupList.TransformBy(transformMatrix);
	if (theApp.IsTrapHighlighted()) {
		UpdateGroupsInAllViews(EoDb::kGroupsSafeTrap, &m_TrappedGroupList);
	}
}

int AeSysDoc::TrapGroupCount() const {
	return m_TrappedGroupList.GetCount();
}

OdGePoint3d AeSysDoc::TrapPivotPoint() const noexcept {
	return m_TrapPivotPoint;
}
