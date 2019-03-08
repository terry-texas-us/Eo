#include "stdafx.h"
#include "AeSysDoc.h"

int AeSysDoc::GetBlockReferenceCount(const CString& name) {
	int Count = 0;

	for (int LayerIndex = 0; LayerIndex < GetLayerTableSize(); LayerIndex++) {
		EoDbLayer* Layer = GetLayerAt(LayerIndex);
		Count += Layer->GetBlockReferenceCount(name);
	}
	CString Key;
	EoDbBlock* Block;

	POSITION Position = m_BlockTable.GetStartPosition();
	while (Position != NULL) {
		m_BlockTable.GetNextAssoc(Position, Key, Block);
		Count += Block->GetBlockReferenceCount(name);
	}
	return Count;
}
EoDbBlockTable* AeSysDoc::BlockTable() {
	return (&m_BlockTable);
}
bool AeSysDoc::BlockTableIsEmpty() {
	return m_BlockTable.IsEmpty() == TRUE;
}
EoUInt16 AeSysDoc::BlockTableSize() {
	return (EoUInt16(m_BlockTable.GetSize()));
}
POSITION AeSysDoc::GetFirstBlockPosition() {
	return m_BlockTable.GetStartPosition();
}
void AeSysDoc::GetNextBlock(POSITION& position, CString& name, EoDbBlock*& block) {
	m_BlockTable.GetNextAssoc(position, name, block);
}
void AeSysDoc::InsertBlock(const OdString& name, EoDbBlock* block) {
	m_BlockTable.SetAt(LPCWSTR(name), block);
}
bool AeSysDoc::LookupBlock(CString name, EoDbBlock*& block) {
	if (m_BlockTable.Lookup(name, block)) {
		return true;
	}
	block = NULL;
	return false;
}
void AeSysDoc::RemoveAllBlocks() {
	CString Name;
	EoDbBlock* Block;

	POSITION BlockPosition = m_BlockTable.GetStartPosition();
	while (BlockPosition != NULL) {
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

	POSITION BlockPosition = m_BlockTable.GetStartPosition();
	while (BlockPosition != NULL) {
		m_BlockTable.GetNextAssoc(BlockPosition, Name, Block);
		if (GetBlockReferenceCount(Name) == 0) {
			m_BlockTable.RemoveKey(Name);
			Block->DeletePrimitivesAndRemoveAll();
			delete Block;
		}
	}
}
