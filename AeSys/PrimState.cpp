#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysView.h"

// State list maintenance
CPrimState* psSav[] = {0, 0, 0, 0};

const CPrimState& CPrimState::operator=(const CPrimState& other) {
	m_FontDefinition = other.m_FontDefinition;

	m_ColorIndex = other.m_ColorIndex;
	m_LinetypeIndex = other.m_LinetypeIndex;
	m_PointDisplayMode = other.m_PointDisplayMode;
	m_HatchInteriorStyle = other.m_HatchInteriorStyle;
	m_HatchInteriorStyleIndex = other.m_HatchInteriorStyleIndex;

	return (*this);
}
EoDbCharacterCellDefinition CPrimState::CharacterCellDefinition() const noexcept {
	return (m_CharacterCellDefinition);
}
OdInt16 CPrimState::ColorIndex() const noexcept {
	return (m_ColorIndex);
}
EoDbFontDefinition CPrimState::FontDefinition() const {
	return (m_FontDefinition);
}
OdInt16 CPrimState::LinetypeIndex() const noexcept {
	return (m_LinetypeIndex);
}
OdInt16 CPrimState::PointDisplayMode() const noexcept {
	return m_PointDisplayMode;
}
OdInt16 CPrimState::HatchInteriorStyle() const noexcept {
	return (m_HatchInteriorStyle);
}
size_t CPrimState::HatchInteriorStyleIndex() const noexcept {
	return (m_HatchInteriorStyleIndex);
}
void CPrimState::Restore(CDC* deviceContext, int iSaveId) {
	if (iSaveId >= sizeof(psSav) / sizeof(psSav[0]))
		return;

	if (psSav[iSaveId] != 0) {
		SetPen(NULL, deviceContext, psSav[iSaveId]->ColorIndex(), psSav[iSaveId]->LinetypeIndex());

		m_FontDefinition = psSav[iSaveId]->m_FontDefinition;

		SetTxtAlign(deviceContext, m_FontDefinition.HorizontalAlignment(), m_FontDefinition.VerticalAlignment());

		SetHatchInteriorStyle(psSav[iSaveId]->HatchInteriorStyle());
		SetHatchInteriorStyleIndex(psSav[iSaveId]->HatchInteriorStyleIndex());

		delete psSav[iSaveId];
		psSav[iSaveId] = 0;
	}
}
int CPrimState::Save() {
	int iSaveId = sizeof(psSav) / sizeof(psSav[0]) - 1;

	while (iSaveId >= 0 && psSav[iSaveId] != 0)
		iSaveId--;

	if (iSaveId < 0) {
		theApp.WarningMessageBox(IDS_MSG_SAVE_STATE_LIST_ERROR);
	}
	else {
		SetHatchInteriorStyle(pstate.HatchInteriorStyle());
		psSav[iSaveId] = new CPrimState;
		*psSav[iSaveId] = pstate;
	}
	// return id to use for restore reference
	return (iSaveId);
}
void CPrimState::SetPen(AeSysView* view, CDC* deviceContext, OdInt16 colorIndex, OdInt16 linetypeIndex) noexcept {
	if (EoDbPrimitive::HighlightColorIndex() != 0) {
		colorIndex = EoDbPrimitive::HighlightColorIndex();
	}
	//if (colorIndex == EoDbPrimitive::COLORINDEX_BYLAYER) {
	//	colorIndex = EoDbPrimitive::LayerColorIndex();
	//}
	if (EoDbPrimitive::HighlightLinetypeIndex() != 0) {
		linetypeIndex = EoDbPrimitive::HighlightLinetypeIndex();
	}
	//if (linetype == EoDbPrimitive::LINETYPE_BYLAYER) {
	//	linetype = EoDbPrimitive::LayerLinetypeIndex();
	//}
	m_ColorIndex = colorIndex;
	m_LinetypeIndex = linetypeIndex;

	double LogicalWidth = 0.;

	if (view && view->PenWidthsOn()) {
		const int LogicalPixelsX = deviceContext->GetDeviceCaps(LOGPIXELSX);
		LogicalWidth = theApp.PenWidthsGet(colorIndex) * double(LogicalPixelsX);
		LogicalWidth *= EoMin(1., view->ZoomFactor());
		LogicalWidth = EoRound(LogicalWidth);
	}
	if (deviceContext) {
		ManagePenResources(deviceContext, colorIndex, int(LogicalWidth), linetypeIndex);
	}
}
void CPrimState::ManagePenResources(CDC* deviceContext, OdInt16 colorIndex, int penWidth, OdInt16 linetypeIndex) {
	static const int NumberOfPens = 8;
	static HPEN hPen[NumberOfPens] = {0, 0, 0, 0, 0, 0, 0, 0};
	static COLORREF	crColRef[NumberOfPens];
	static OdInt16 LinetypeIndexes[NumberOfPens];
	static int PenWidths[NumberOfPens];
	static HPEN hPenCur;

	switch (linetypeIndex) {
	case 0:
		linetypeIndex = PS_NULL;
		break;

	case 2:
		linetypeIndex = PS_DOT;
		break;

	case 3:
		linetypeIndex = PS_DASH;
		break;

	case 6:
		linetypeIndex = PS_DASHDOT;
		break;

	case 7:
		linetypeIndex = PS_DASHDOTDOT;
		break;

	default:
		linetypeIndex = PS_SOLID;
	}
	if (deviceContext) {
		deviceContext->SetTextColor(pColTbl[colorIndex]);
	}

	int iPen = 0;
	for (int i = 0; i < NumberOfPens; i++) {
		if (hPen[i] && LinetypeIndexes[i] == linetypeIndex && PenWidths[i] == penWidth && crColRef[i] == pColTbl[colorIndex]) {
			hPenCur = hPen[i];
			if (deviceContext) {
				deviceContext->SelectObject(CPen::FromHandle(hPenCur));
			}
			return;
		}
		if (hPen[i] == 0) {
			iPen = i;
		}
	}
	HPEN NewPenHandle = ::CreatePen(linetypeIndex, penWidth, pColTbl[colorIndex]);

	if (NewPenHandle) {
		hPenCur = NewPenHandle;
		if (deviceContext) {
			deviceContext->SelectObject(CPen::FromHandle(NewPenHandle));
		}
		if (hPen[iPen]) {
			::DeleteObject(hPen[iPen]);
		}
		hPen[iPen] = NewPenHandle;
		LinetypeIndexes[iPen] = linetypeIndex;
		PenWidths[iPen] = penWidth;
		crColRef[iPen] = pColTbl[colorIndex];
	}
}
void CPrimState::SetColorIndex(CDC* deviceContext, OdInt16 colorIndex) {
	m_ColorIndex = colorIndex;
	if (deviceContext) {
		ManagePenResources(deviceContext, colorIndex, 0, m_LinetypeIndex);
	}
}
void CPrimState::SetLinetypeIndexPs(CDC* deviceContext, OdInt16 linetypeIndex) {
	m_LinetypeIndex = linetypeIndex;
	if (deviceContext) {
		ManagePenResources(deviceContext, m_ColorIndex, 0, linetypeIndex);
	}
}
int CPrimState::SetROP2(CDC* deviceContext, int iDrawMode) {
	// Sets the current foreground mix mode. GDI uses the foreground mix mode to combine pens and
	// interiors of filled objects with the colors already on the screen. The foreground mix mode
	// defines how colors from the brush or pen and the colors in the existing image are to be combined.

	if (ColorPalette[0] == RGB(0xFF, 0xFF, 0xFF)) {
		if (iDrawMode == R2_XORPEN)
			iDrawMode = R2_NOTXORPEN;
	}
	return (deviceContext->SetROP2(iDrawMode));
}
void CPrimState::SetTxtAlign(CDC* deviceContext, EoDb::HorizontalAlignment horizontalAlignment, EoDb::VerticalAlignment verticalAlignment) {
	m_FontDefinition.SetHorizontalAlignment(horizontalAlignment);
	m_FontDefinition.SetVerticalAlignment(verticalAlignment);

	deviceContext->SetTextAlign(TA_LEFT | TA_BASELINE);
}
void CPrimState::SetCharacterCellDefinition(const EoDbCharacterCellDefinition& characterCellDefinition) noexcept {
	m_CharacterCellDefinition = characterCellDefinition;
}
void CPrimState::SetFontDefinition(CDC* deviceContext, const EoDbFontDefinition& fontDefinition) {
	m_FontDefinition = fontDefinition;
	SetTxtAlign(deviceContext, m_FontDefinition.HorizontalAlignment(), m_FontDefinition.VerticalAlignment());
}
void CPrimState::SetPointDisplayMode(OdInt16 pointDisplayMode) noexcept {
	m_PointDisplayMode = pointDisplayMode;
}
void CPrimState::SetHatchInteriorStyle(OdInt16 interiorStyle) noexcept {
	m_HatchInteriorStyle = interiorStyle;
}
void CPrimState::SetHatchInteriorStyleIndex(size_t styleIndex) noexcept {
	m_HatchInteriorStyleIndex = styleIndex;
}
