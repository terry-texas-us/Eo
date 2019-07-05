#include "stdafx.h"
#include "AeSysDoc.h"

int AeSysDoc::GetBlockReferenceCount(const CString& name) {
	auto Count {0};
	for (auto LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		auto Layer {GetLayerAt(LayerIndex)};
		Count += Layer->GetBlockReferenceCount(name);
	}
	CString Key;
	EoDbBlock* Block;
	auto Position {m_BlockTable.GetStartPosition()};
	while (Position != nullptr) {
		m_BlockTable.GetNextAssoc(Position, Key, Block);
		Count += Block->GetBlockReferenceCount(name);
	}
	return Count;
}

EoDbBlockTable* AeSysDoc::BlockTable() noexcept {
	return &m_BlockTable;
}

bool AeSysDoc::BlockTableIsEmpty() {
	return m_BlockTable.IsEmpty() == TRUE;
}

unsigned short AeSysDoc::BlockTableSize() {
	return static_cast<unsigned short>(m_BlockTable.GetSize());
}

POSITION AeSysDoc::GetFirstBlockPosition() {
	return m_BlockTable.GetStartPosition();
}

void AeSysDoc::GetNextBlock(POSITION& position, CString& name, EoDbBlock*& block) {
	m_BlockTable.GetNextAssoc(position, name, block);
}

void AeSysDoc::InsertBlock(const wchar_t* name, EoDbBlock* block) {
	m_BlockTable.SetAt(name, block);
}

bool AeSysDoc::LookupBlock(const wchar_t* name, EoDbBlock*& block) {
	if (m_BlockTable.Lookup(name, block) != 0) { return true; }
	block = nullptr;
	return false;
}

void AeSysDoc::RemoveAllBlocks() {
	CString Name;
	EoDbBlock* Block;
	auto BlockPosition {m_BlockTable.GetStartPosition()};
	while (BlockPosition != nullptr) {
		m_BlockTable.GetNextAssoc(BlockPosition, Name, Block);
		Block->DeletePrimitivesAndRemoveAll();
		delete Block;
	}
	m_BlockTable.RemoveAll();
}

void AeSysDoc::PurgeUnreferencedBlocks() {
	CString Name;
	EoDbBlock* Block;
	// <tas="Deletion by key may cause loop problems"</tas>
	auto BlockPosition {m_BlockTable.GetStartPosition()};
	while (BlockPosition != nullptr) {
		m_BlockTable.GetNextAssoc(BlockPosition, Name, Block);
		if (GetBlockReferenceCount(Name) == 0) {
			m_BlockTable.RemoveKey(Name);
			Block->DeletePrimitivesAndRemoveAll();
			delete Block;
		}
	}
}
