#include <windows.h>
#include <streams.h>
#include <commctrl.h>
#include <olectl.h>

#include "resource.h"
#include "uids.h"
#include "decoder.h"
#include "props.h"

#include "dbg.h"

extern bool g_generateTransparencyBackground;
extern bool g_useOMP;

//######################################
// CreateInstance
// Used by the DirectShow base classes to create instances
//######################################
CUnknown *CHapDecoderProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) {
    ASSERT(phr);
    CUnknown *punk = new CHapDecoderProperties(lpunk, phr);
    if (punk == NULL) {
        if (phr) *phr = E_OUTOFMEMORY;
    }
    return punk;
}

//######################################
// Constructor
//######################################
CHapDecoderProperties::CHapDecoderProperties(LPUNKNOWN pUnk, HRESULT *phr) :
    CBasePropertyPage(NAME("HapDecoder Property Page"), pUnk, IDD_HAPDECODERPROP, IDS_TITLE)
{
    ASSERT(phr);
}

//######################################
// OnReceiveMessage
// Handles the messages for our property window
//######################################
INT_PTR CHapDecoderProperties::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg== WM_COMMAND) {
        m_bDirty = TRUE;
        if (m_pPageSite) m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
        return (LRESULT) 1;
    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}

//######################################
// OnActivate
//######################################
HRESULT CHapDecoderProperties::OnActivate() {
	CheckDlgButton(m_Dlg, IDC_CHECK1, g_generateTransparencyBackground);
	CheckDlgButton(m_Dlg, IDC_CHECK2, g_useOMP);
    return NOERROR;
}

//######################################
// OnDeactivate
//######################################
HRESULT CHapDecoderProperties::OnDeactivate(void) {
    return NOERROR;
}

//######################################
// OnApplyChanges
//######################################
HRESULT CHapDecoderProperties::OnApplyChanges() {
	g_generateTransparencyBackground = IsDlgButtonChecked(m_Dlg, IDC_CHECK1);
	g_useOMP = IsDlgButtonChecked(m_Dlg, IDC_CHECK2);
    return NOERROR;
}
