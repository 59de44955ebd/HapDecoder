#include <strsafe.h>

class CHapDecoderProperties : public CBasePropertyPage
{

public:
    static CUnknown * WINAPI CreateInstance (LPUNKNOWN lpunk, HRESULT *phr);

private:
    INT_PTR OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    HRESULT OnActivate ();
    HRESULT OnDeactivate ();
    HRESULT OnApplyChanges ();
    CHapDecoderProperties (LPUNKNOWN lpunk, HRESULT *phr);
};
