#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "afxodlgs.h"
#include "DbOle2Frame.h"
#include "OleItemHandlerBase.h"
#include "MemoryStream.h"

#include "Ed/EdCommandStack.h"
#include "DbSSet.h"

#include "DbUserIO.h"

#ifdef OD_OLE_SUPPORT
class OleDwgItem : public COleClientItem, public OdOleItemHandler {
	ODRX_HEAP_OPERATORS();

	OdSmartPtr<OdOleItemSimplestHandler> m_pRawData;

	class File : public CFile {
		OdStreamBuf&  m_stream;
	public:
		File(OdStreamBuf& stream) :
			m_stream(stream) {
				m_strFileName = L"OdDbOle2Frame binary data.";
				m_bCloseOnDelete = false;
				m_hFile = 0;
		}
		unsigned Read(void* lpBuf, unsigned nCount) {
			unsigned long nBytesLeft = unsigned long(m_stream.length()-m_stream.tell());
			if (nBytesLeft < nCount) {
				nCount = nBytesLeft;
			}
			m_stream.getBytes(lpBuf, nCount);
			return nCount;
		}
		void Write(const void* lpBuf, unsigned nCount) {
			m_stream.putBytes(lpBuf, nCount);
		}
	};

public:
	OdDbObjectId m_frameId;

public:
	ODRX_DECLARE_MEMBERS(OleDwgItem);

	~OleDwgItem() {
	}
	void addRef() {
		COleClientItem::ExternalAddRef();
	}
	void release() {
		COleClientItem::ExternalRelease();
	}

	static OdRxObjectPtr createNew() {
		OdRxObjectPtr pRes;
		pRes.attach(new OleDwgItem());
		return pRes;
	}

	struct OleItemClientData {
		LPVIEWOBJECT m_lpViewObject;
		DVASPECT m_nDrawAspect;
		void *hdc;
		RECTL rectl;
	};
	static void drawByMainThread(void *pArg) {
		OleItemClientData *pData = reinterpret_cast<OleItemClientData*>(pArg);
#ifdef _DEBUG
		HRESULT hr = 
#endif
			pData->m_lpViewObject->Draw(pData->m_nDrawAspect, -1, NULL, NULL, NULL, (HDC)pData->hdc, &(pData->rectl), NULL, NULL, 0);
#ifdef _DEBUG
		if (FAILED(hr)) {
			ODA_TRACE1("[OleDwgItem::draw] m_lpViewObject->Draw() returned %x\n", hr);
			if (HRESULT_FACILITY(hr) == FACILITY_WINDOWS) {
				hr = HRESULT_CODE(hr);
			}
			wchar_t* szError;
			
			if (::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &szError, 0, NULL) != 0) {
				ODA_TRACE1("[OleDwgItem::draw] Decoded HRESULT: %s", szError);
				::LocalFree(szError);
			}
		}
#endif
	}

	// OdGiSelfGdiDrawable
	void draw(const OdGiCommonDraw& drawObj, void* hdc, const OdGsDCRect& rect) const {
		if(m_pRawData.get()) {
			m_pRawData->draw(drawObj, hdc, rect);
			return ;
		}
		if (m_lpViewObject) {
			::FillRect((HDC)hdc, (LPRECT)&rect, ::GetSysColorBrush(COLOR_WINDOW));
			RECTL rectl = { rect.m_min.x, rect.m_max.y, rect.m_max.x, rect.m_min.y };
			OleItemClientData cliData = { m_lpViewObject, m_nDrawAspect, hdc, rectl };
			odExecuteMainThreadAction(drawByMainThread, &cliData);
		}
	}
	void load(OdStreamBuf& stream) {
		COleClientItem::Release();

		COleDocument* pOleDocument = 0;
		if (stream.isKindOf(OdOleItemInitStream::desc())) {
			m_frameId = OdOleItemInitStreamPtr(&stream)->frameId();
			OdDbDatabaseDocPtr pDb = OdDbDatabaseDoc::cast(m_frameId.database());
			if (pDb.get()) {
				pOleDocument = pDb->document();
			}
		}
		if (pOleDocument) {
			m_pRawData.release();

			File file(stream);
			CArchive archive(&file, CArchive::load|CArchive::bNoFlushOnDelete);
			archive.m_pDocument = pOleDocument;

			// Maintain reference balance:
			// COleDocument calls COleClientItem::InternalRelease() in its DeleteContents(),
			// but it does not add reference in COleDocument::AddItem();
			if(!m_pDocument) {
				pOleDocument->AddItem(this);
				addRef();
			}
			TRY {
				COleClientItem::Serialize(archive);
			}
			CATCH (CException, pException) {
				CString msg;
				pException->GetErrorMessage(msg.GetBuffer(256), 256);
				OdString msg2;
				msg2.format(L"%ls : \"%ls\"", (const wchar_t*) CString(pException->GetRuntimeClass()->m_lpszClassName), (const wchar_t*) msg);
				// remove invalid object from OLE document
				m_pDocument->RemoveItem(this);
				release();
				archive.Abort();
				throw OdError(msg2);
			}
			END_CATCH
		} else {
			// loading database is not associated with COdaMfcApp (COleDocument)
			// so do not load OLE object - just store its data.
			if (m_pRawData.isNull()) {
				m_pRawData = OdRxObjectImpl<OdOleItemSimplestHandler>::createObject();
			}
			m_pRawData->load(stream);
		}
	}
	void save(OdStreamBuf& stream) const {
		if (m_pRawData.get()) {
			m_pRawData->save(stream);
			return;
		}
		if (m_lpObject) {
			File file(stream);
			CArchive archive(&file, CArchive::store);

			const_cast<OleDwgItem*>(this)->COleClientItem::Serialize(archive);
		}
	}
	OdOleItemHandler::Type type() const {
		if (m_pRawData.get()) {
			return m_pRawData->type();
		}

		return Type(COleClientItem::GetType());
	}
	OdOleItemHandler::DvAspect drawAspect() const {
		if (m_pRawData.get()) {
			return m_pRawData->drawAspect();
		}
		return DvAspect(COleClientItem::GetDrawAspect());
	}
	void setDrawAspect(DvAspect drawAspect) {
		if (m_pRawData.get()) {
			m_pRawData->setDrawAspect(drawAspect);
			return ;
		}
		if (m_pDocument) {
			COleClientItem::SetDrawAspect(DVASPECT(drawAspect));
		}
	}
	OdString linkName() const {
		return linkPath();
	}
	OdString linkPath() const {
		if (m_pRawData.get()) {
			return m_pRawData->linkName();
		}
		IOleLinkPtr pOleLink(m_lpObject);
		if (pOleLink) {
			LPOLESTR displayName = 0;
			if (SUCCEEDED(pOleLink->GetSourceDisplayName(&displayName))) {
				_bstr_t res(displayName);
				IMallocPtr pMalloc;
				::CoGetMalloc(1, &pMalloc);
				if(pMalloc!=0) {
					pMalloc->Free(displayName);
				}
				return (const wchar_t*)res;
			}
		}
		return OdString::kEmpty;
	}
	OdString userType() const {
		if (m_pRawData.get()) {
			return m_pRawData->userType();
		}
		if (m_lpObject) {
			CString res;
			const_cast<OleDwgItem*>(this)->COleClientItem::GetUserType(USERCLASSTYPE_FULL, res);
			return (const wchar_t*) OdString(res);
		}
		return OdString::kEmpty;
	}
	OdOleItemHandler::DvAspect adviseType() const {
		if (m_pRawData.get()) {
			return m_pRawData->adviseType();
		}
		if (!m_lpViewObject) {
			return kContent;
		}
		unsigned long dwAspect;
		IAdviseSinkPtr pAdviseSink;
		VERIFY(m_lpViewObject->GetAdvise(&dwAspect, NULL, &pAdviseSink) == S_OK);
		return DvAspect(dwAspect);
	}
	bool monikerAssigned() const {
		if (m_pRawData.get()) {
			return m_pRawData->monikerAssigned();
		}
		return (m_bMoniker != 0);
	}
	unsigned long getCompoundDocumentDataSize() const {
		if (m_pRawData.get()) {
			return m_pRawData->getCompoundDocumentDataSize();
		}
		OdMemoryStreamPtr pBuff = OdMemoryStream::createNew();
		save(*pBuff.get());
		return (unsigned long)pBuff->tell();
	}
	void getCompoundDocument(OdStreamBuf& stream) const {
		if (m_pRawData.get()) {
			m_pRawData->getCompoundDocument(stream);
			return ;
		}
		save(stream);
	}
	void setCompoundDocument(unsigned long nDataSize, OdStreamBuf& stream) {
		if (m_pRawData.get()) {
			m_pRawData->setCompoundDocument(nDataSize, stream);
			return ;
		}
		load(stream);
	}
	void OnChange(OLE_NOTIFICATION nCode, unsigned long dwParam) {
		COleClientItem::OnChange(nCode, dwParam);

		// suppress boring save dialog on exit...
		m_pDocument->SetModifiedFlag(FALSE);

		OdDbOle2FramePtr pOleFrame = m_frameId.openObject(OdDb::kForWrite);
		if (pOleFrame.get()) {
			CSize newSize;
			if (GetExtent(&newSize)) {
				CSize oldSize(pOleFrame->unhandled_himetricWidth(), pOleFrame->unhandled_himetricHeight());
				if (oldSize != CSize(0, 0) && newSize != oldSize) {
					OdGeScale3d s;
					pOleFrame->unhandled_setHimetricSize(unsigned short(s.sx = newSize.cx), unsigned short(s.sy = newSize.cy));
					s.sz = 1.0;
					s.sx /= oldSize.cx;
					s.sy /= oldSize.cy;

					OdRectangle3d r3d;
					pOleFrame->position(r3d);

					OdGeMatrix3d xScaling = OdGeMatrix3d::scaling(s, r3d.upLeft);
					r3d.lowLeft. transformBy(xScaling);
					r3d.upLeft. transformBy(xScaling);
					r3d.upRight. transformBy(xScaling);
					r3d.lowRight. transformBy(xScaling);
					pOleFrame->setPosition(r3d);
				}
			}
		}
	}
};

ODRX_NO_CONS_DEFINE_MEMBERS(OleDwgItem, OdOleItemHandler);

class OleCommand : public OdEdCommand {
public:
	const OdString groupName() const {
		return (L"AeSys");
	}
};

class OLEOPEN_cmd : public OleCommand {
public:
	const OdString globalName() const {
		return (L"OLEOPEN");
	}
	void execute(OdEdCommandContext* commandContext) {
		OdDbCommandContextPtr CommandContext(commandContext);
		OdDbUserIOPtr UserIO = CommandContext->userIO();

		OdDbOle2FramePtr pOle2Frame;
		OdSmartPtr<OleDwgItem> pItem;
		OdDbSelectionSetIteratorPtr pIter = UserIO->select(L"Select an object <done>:")->newIterator();
		while (!pIter->done()) {
			pOle2Frame = OdDbOle2Frame::cast(pIter->objectId().safeOpenObject(OdDb::kForWrite));
			if(pOle2Frame.get()) {
				pItem = pOle2Frame->itemHandler();
				pItem->DoVerb(OLEIVERB_OPEN, NULL);
				return;
			}
			pIter->next();
		}
	}
};

class insertobj_cmd : public OleCommand {
	struct TRACKER : OdEdRealTracker {
		OdSmartPtr<OdDbOle2Frame> _pOleFrame;
		OdGePoint3d _orig;
		OdGeVector2d _size;
		double _dist;

		void setValue(double dist) {
			_dist = dist;
			OdRectangle3d r3d;

			OdGeVector2d size = _size.normal() * _dist;
			r3d.lowLeft.set(_orig.x, _orig.y, _orig.z);
			r3d.upLeft.set(_orig.x, _orig.y + size.y, _orig.z);
			r3d.upRight.set(_orig.x + size.x, _orig.y + size.y, _orig.z);
			r3d.lowRight.set(_orig.x + size.x, _orig.y, _orig.z);

			_pOleFrame->setPosition(r3d);
		}
		int addDrawables(OdGsView* pView) {
			pView->add(_pOleFrame, 0);
			return 1;
		}
		void removeDrawables(OdGsView* pView) {
			pView->erase(_pOleFrame);
		}
	};
public:
	const OdString globalName() const {
		return (L"insertobj");
	}
	void execute(OdEdCommandContext* commandContext) {
		OdDbCommandContextPtr CommandContext(commandContext);
		OdDbDatabaseDocPtr Database = CommandContext->database();
		OdSmartPtr<OdDbUserIO> UserIO = CommandContext->userIO();

		COleInsertDialog OleInsertDialog;
		
		if (OleInsertDialog.DoModal() != IDOK) { return; }

		COleDocument* OleDocument = Database->document();

		OdStaticRxObject<TRACKER> tracker;

		OdSmartPtr<OdDbOle2Frame> pOleFrame = tracker._pOleFrame = OdDbOle2Frame::createObject();
		pOleFrame->setDatabaseDefaults(Database);
		OdSmartPtr<OleDwgItem> pItem = pOleFrame->getItemHandler();

		OleDocument->AddItem(pItem);
		pItem->addRef(); // DN: not sure about it, but otherwise it crashes on exit...

		ASSERT_VALID(pItem);

		// Now create the OLE object/item
		TRY {
			if (!OleInsertDialog.CreateItem(pItem)) {
				AfxThrowMemoryException();
			}
			// try to get initial presentation data
			pItem->UpdateLink();

			CSize size;
			if (!pItem->GetExtent(&size)) {
				size = CSize(1000, 1000);
			}
			pOleFrame->unhandled_setHimetricSize(unsigned short(size.cx), unsigned short(size.cy));

			tracker._orig = UserIO->getPoint(L"Specify insertion point <0,0>:");

			// Set initial OLE frame position to origin, and size in world units:
			// Convert MM_HIMETRIC (0.01 millimeter) to Space Units:
			double s = Database->getMEASUREMENT()==OdDb::kEnglish ? 0.01 / 25.4 : 0.01;
			tracker._size.set(double(size.cx) * s, double(size.cy) * s);

			OdString sPmt;
			sPmt.format(L"Specify size <%g>:", tracker._size.length());
			tracker.setValue(UserIO->getDist(sPmt, 0, 0.0, OdString::kEmpty, &tracker));

			OdDbBlockTableRecordPtr pSpace = Database->getActiveLayoutBTRId().safeOpenObject(OdDb::kForWrite);
			pItem->m_frameId = pSpace->appendOdDbEntity(pOleFrame);

			// if insert new object -- initially show the object
			if (OleInsertDialog.GetSelectionType() == COleInsertDialog::createNewItem) {
				pItem->DoVerb(OLEIVERB_OPEN, NULL);
			}
			// suppress boring save dialog on exit...
			OleDocument->SetModifiedFlag(FALSE);
		}
		CATCH_ALL(e) {
			if (pOleFrame->isDBRO()) {
				pOleFrame->erase();
			}
			// clean up item
			pItem->Delete();
			::AfxMessageBox(L"Failed to insert OLE object...");
		}
		END_CATCH_ALL
	}
};

/// <remarks> Call just after odInitialize() to enable this implementation </remarks>
void rxInit_COleClientItem_handler() {
	OleDwgItem::rxInit();
	OdOleItemHandler::desc()->setConstructor(OleDwgItem::createNew);
	odedRegCmds()->addCommand(OdRxObjectImpl<OLEOPEN_cmd>::createObject());
	odedRegCmds()->addCommand(OdRxObjectImpl<insertobj_cmd>::createObject());
}

/// <remarks> Call just before odUninitialize() </remarks>
void rxUninit_COleClientItem_handler() {
	odedRegCmds()->removeGroup(L"AeSys");
	if (OdOleItemHandler::desc()->constructor() == OleDwgItem::createNew) {
		OdOleItemHandler::desc()->setConstructor(OdOleItemHandler::pseudoConstructor);
	}
	OleDwgItem::rxUninit();
}

void AeSysDoc::OnInsertOleobject() {
	BeginWaitCursor();
	ExecuteCommand(L"insertobj");
	EndWaitCursor();
}
#endif // OD_OLE_SUPPORT


