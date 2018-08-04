#pragma once

class CHapDecoder : 
	public CTransformFilter,
    public ISpecifyPropertyPages,
    public CPersistStream
{

public:

    DECLARE_IUNKNOWN;
    static CUnknown * WINAPI CreateInstance (LPUNKNOWN punk, HRESULT *phr);

	// Implement the ISpecifyPropertyPages interface
    STDMETHODIMP NonDelegatingQueryInterface (REFIID riid, void ** ppv);
	STDMETHODIMP GetPages(CAUUID *pPages);

    // Overrriden from CTransformFilter base class
    HRESULT Transform (IMediaSample *pIn, IMediaSample *pOut);
    HRESULT CheckInputType (const CMediaType *mtIn);
    HRESULT CheckTransform (const CMediaType *mtIn, const CMediaType *mtOut);
    HRESULT DecideBufferSize (IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties);
    HRESULT GetMediaType (int iPosition, CMediaType *pMediaType);

    // CPersistStream override
    STDMETHODIMP GetClassID (CLSID *pClsid);

	// DXT software decompression
	HRESULT Decompress (PBYTE pSrcBuffer, PBYTE pDestBuffer, DWORD dwSize);

private:

    // Constructor
    CHapDecoder (TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);
	~CHapDecoder();

    HRESULT Copy (IMediaSample *pSource, IMediaSample *pDest) const;

	size_t m_outputBufferSize = 0;

	// DXT DECOMPRESSION
	int _dxtFlags = 0;
	PBYTE _dxtBuffer = NULL;
	unsigned int _dxtBufferSize = 0;

	PBYTE _tmpBuffer = NULL;

	unsigned int _width;
	unsigned int _height;

	GUID m_subTypeIn;
	GUID m_subTypeOut;
};

// from hapcodec.h

// y must be 2^n
#define align_round(x,y) ((((unsigned int)(x))+(y-1))&(~(y-1)))

#define LAGARITH_RELEASE		// if this is a version to release, disables all debugging info 

inline void * lag_aligned_malloc(void *ptr, int size, int align, char *str) {
	if (ptr) {
		try {
#ifndef LAGARITH_RELEASE
			char msg[128];
			sprintf_s(msg, 128, "Buffer '%s' is not null, attempting to free it...", str);
			MessageBox(HWND_DESKTOP, msg, "Error", MB_OK | MB_ICONEXCLAMATION);
#endif
			_aligned_free(ptr);
		}
		catch (...) {
#ifndef LAGARITH_RELEASE
			char msg[256];
			sprintf_s(msg, 128, "An exception occurred when attempting to free non-null buffer '%s' in lag_aligned_malloc", str);
			MessageBox(HWND_DESKTOP, msg, "Error", MB_OK | MB_ICONEXCLAMATION);
#endif
		}
	}
	return _aligned_malloc(size, align);
}

#ifdef LAGARITH_RELEASE
#define lag_aligned_free(ptr, str) { \
	if ( ptr ){ \
		try {\
			_aligned_free((void*)ptr);\
		} catch ( ... ){ } \
	} \
	ptr=NULL;\
}
#else 
#define lag_aligned_free(ptr, str) { \
	if ( ptr ){ \
		try { _aligned_free(ptr); } catch ( ... ){\
			char err_msg[256];\
			sprintf_s(err_msg,256,"Error when attempting to free pointer %s, value = 0x%X - file %s line %d",str,ptr,__FILE__, __LINE__);\
			MessageBox (HWND_DESKTOP, err_msg, "Error", MB_OK | MB_ICONEXCLAMATION);\
		} \
	} \
	ptr=NULL;\
}
#endif
