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

	bool m_bGenerateTransparencyBackground;
	bool m_useOMP;
};

// from hapcodec.h

// y must be 2^n
#define align_round(x,y) ((((unsigned int)(x))+(y-1))&(~(y-1)))

inline void * lag_aligned_malloc(void *ptr, int size, int align, char *str) {
	if (ptr) {
		try {
			_aligned_free(ptr);
		}
		catch (...) {
		}
	}
	return _aligned_malloc(size, align);
}

#define LAG_ALIGNED_FREE(ptr, str) { \
	if ( ptr ){ \
		try {\
			_aligned_free((void*)ptr);\
		} catch ( ... ){ } \
	} \
	ptr=NULL;\
}
