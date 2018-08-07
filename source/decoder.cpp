#include <windows.h>
#include <streams.h>
#include <initguid.h>
#include <olectl.h>

// hap
#include <ppl.h>
#include "hap.h"
#include "YCoCg.h"
#include "YCoCgDXT.h"

// squish
#include "squish.h"

#include "decoder.h"
#include "resource.h"
#include "uids.h"
#include "conversion.h"
#include "props.h"

#include "dbg.h"

bool g_generateTransparencyBackground = FALSE;
bool g_useOMP = TRUE;

//######################################
// Setup information
//######################################

const AMOVIESETUP_MEDIATYPE sudPinTypes = {
	&MEDIATYPE_Video,       // Major type
	&MEDIASUBTYPE_NULL      // Minor type
};

const AMOVIESETUP_PIN sudpPins[] = {
	{ L"Input",             // Pins string name
	  FALSE,                // Is it rendered
	  FALSE,                // Is it an output
	  FALSE,                // Are we allowed none
	  FALSE,                // And allowed many
	  &CLSID_NULL,          // Connects to filter
	  NULL,                 // Connects to pin
	  1,                    // Number of types
	  &sudPinTypes          // Pin information
	},
	{ L"Output",            // Pins string name
	  FALSE,                // Is it rendered
	  TRUE,                 // Is it an output
	  FALSE,                // Are we allowed none
	  FALSE,                // And allowed many
	  &CLSID_NULL,          // Connects to filter
	  NULL,                 // Connects to pin
	  1,                    // Number of types
	  &sudPinTypes          // Pin information
	}
};

const AMOVIESETUP_FILTER sudHapDecoder = {
	&CLSID_HapDecoder,      // Filter CLSID
	L"HapDecoder",          // String name 
	MERIT_NORMAL,           // Filter merit - MERIT_DO_NOT_USE ?
	2,                      // Number of pins
	sudpPins                // Pin information
};

//######################################
// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance
//######################################
CFactoryTemplate g_Templates[] = {

	{ L"HapDecoder"
	, &CLSID_HapDecoder
	, CHapDecoder::CreateInstance
	, NULL
	, &sudHapDecoder }

	,

	{ L"Settings"
	, &CLSID_HapDecoderPropertyPage
	, CHapDecoderProperties::CreateInstance }

};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

//######################################
// CreateInstance
//######################################
CUnknown *CHapDecoder::CreateInstance(LPUNKNOWN punk, HRESULT *phr) {
	ASSERT(phr);

	CHapDecoder *pNewObject = new CHapDecoder(NAME("HapDecoder"), punk, phr);

	if (pNewObject == NULL) {
		if (phr) *phr = E_OUTOFMEMORY;
	}
	return pNewObject;
}

//######################################
// Constructor
//######################################
CHapDecoder::CHapDecoder(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr) :
	CTransformFilter(tszName, punk, CLSID_HapDecoder),
	CPersistStream(punk, phr)
{}

//######################################
// Destructor
//######################################
CHapDecoder::~CHapDecoder() {

	// Hacky fix for crashes related to running OpenMP threads when exiting.
	// There are obviously better solutions - TODO
	Sleep(200);

	LAG_ALIGNED_FREE(_dxtBuffer, "dxtBuffer");
	LAG_ALIGNED_FREE(_tmpBuffer, "tmpBuffer");
}

//######################################
// NonDelegatingQueryInterface
//######################################
STDMETHODIMP CHapDecoder::NonDelegatingQueryInterface(REFIID riid, void **ppv) {
	CheckPointer(ppv,E_POINTER);
	if (riid == IID_ISpecifyPropertyPages) {
		return GetInterface((ISpecifyPropertyPages *) this, ppv);
	} else {
		return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
	}
}

//######################################
//
//######################################
void HapMTDecode(HapDecodeWorkFunction function, void *info, unsigned int count, void *) {
	concurrency::parallel_for((unsigned int)0, count, [&](unsigned int i) {
		function(info, i);
	});
}

//######################################
// Transform
//######################################
HRESULT CHapDecoder::Transform (IMediaSample *pMediaSampleIn, IMediaSample *pMediaSampleOut) {

	CheckPointer(pMediaSampleIn, E_POINTER);
	CheckPointer(pMediaSampleOut, E_POINTER);

	HRESULT hr;

	// Copy the sample properties across
	hr = Copy(pMediaSampleIn, pMediaSampleOut);
	if (FAILED(hr)) return hr;

	// Pointers to the actual image buffers
	PBYTE pSrcBuffer;
	hr = pMediaSampleIn->GetPointer(&pSrcBuffer);
	if (FAILED(hr)) return hr;

	PBYTE pDestBuffer;
	hr = pMediaSampleOut->GetPointer(&pDestBuffer);
	if (FAILED(hr)) return hr;

	DWORD outputBufferDecodedSize;
	unsigned int outputBufferTextureFormat;

	if (m_subTypeOut == MEDIASUBTYPE_RGB32) {

		//######################################
		// Decpmpress texture to RGB32
		//######################################
		hr = Decompress(pSrcBuffer, pDestBuffer, pMediaSampleIn->GetActualDataLength());
		outputBufferDecodedSize = _width * _height * 4;

		if (FAILED(hr)) return hr;
	}
	else {

		//######################################
		// Return raw RAW DXT buffer
		//######################################

		// hap decode
		unsigned int res = HapDecode(
			pSrcBuffer,
			pMediaSampleIn->GetActualDataLength(),
			0,
			HapMTDecode,
			nullptr,

			pDestBuffer,

			(unsigned long)m_outputBufferSize,
			(unsigned long *)&outputBufferDecodedSize,
			&outputBufferTextureFormat
		);

		if (res != HapResult_No_Error) return E_FAIL;
	}

	pMediaSampleOut->SetActualDataLength(outputBufferDecodedSize);

	return NOERROR;
}

//######################################
// Decompresses texture to RGB32
//######################################
HRESULT CHapDecoder::Decompress (PBYTE pSrcBuffer, PBYTE pDestBuffer, DWORD dwSize) {

	// hap decode
	size_t outputBufferDecodedSize;
	unsigned int outputBufferTextureFormat;

	unsigned int res = HapDecode(
		pSrcBuffer,
		dwSize,
		0,
		HapMTDecode,
		nullptr,
		_dxtBuffer,
		(unsigned long)m_outputBufferSize,
		(unsigned long *)&outputBufferDecodedSize,
		&outputBufferTextureFormat
	);

	if (res != HapResult_No_Error) return E_FAIL;

	if (outputBufferTextureFormat == HapTextureFormat_YCoCg_DXT5) { // Hap Q

		int outputSize = DeCompressYCoCgDXT5(_dxtBuffer, _tmpBuffer, _width, _height, _width * 4);

		//ASSERT(outputSize == _width * _height * 4);
		ConvertCoCgAY8888ToBGRA(_tmpBuffer, pDestBuffer, _width, _height, _width * 4, _width * 4, m_useOMP);

	}
	else { // Hap1, Hap Alpha

		// Convert DXT to BGRA
		squish::DecompressImage(_tmpBuffer, _width, _height, _dxtBuffer, _dxtFlags);
		
		// Swap red-blue channels
		if (m_generateTransparencyBackground && outputBufferTextureFormat == HapTextureFormat_RGBA_DXT5) {
			Blend32bppTo32bppChecker(_width, _height, true, _tmpBuffer, pDestBuffer, m_useOMP);
		}
		else {
			ConvertBGRAtoRGBA(_width, _height, _tmpBuffer, pDestBuffer, m_useOMP);
		}
	}

	return S_OK;
}

//######################################
// Copy
// Make destination an identical copy of source
//######################################
HRESULT CHapDecoder::Copy (IMediaSample *pSource, IMediaSample *pDest) const {

	CheckPointer(pSource, E_POINTER);   
	CheckPointer(pDest ,E_POINTER);   

	// Copy the sample times
	REFERENCE_TIME TimeStart, TimeEnd;
	if (NOERROR == pSource->GetTime(&TimeStart, &TimeEnd)) {
		pDest->SetTime(&TimeStart, &TimeEnd);
	}

	LONGLONG MediaStart, MediaEnd;
	if (pSource->GetMediaTime(&MediaStart,&MediaEnd) == NOERROR) {
		pDest->SetMediaTime(&MediaStart,&MediaEnd);
	}

	// Copy the Sync point property
	HRESULT hr = pSource->IsSyncPoint();
	if (hr == S_OK) {
		pDest->SetSyncPoint(TRUE);
	}
	else if (hr == S_FALSE) {
		pDest->SetSyncPoint(FALSE);
	}
	else {  // an unexpected error has occured...
		return E_UNEXPECTED;
	}

	// Copy the media type
	AM_MEDIA_TYPE *pMediaType;
	pSource->GetMediaType(&pMediaType);
	pDest->SetMediaType(pMediaType);
	DeleteMediaType(pMediaType);

	// Copy the preroll property
	hr = pSource->IsPreroll();
	if (hr == S_OK) {
		pDest->SetPreroll(TRUE);
	}
	else if (hr == S_FALSE) {
		pDest->SetPreroll(FALSE);
	}
	else {  // an unexpected error has occured...
		return E_UNEXPECTED;
	}

	// Copy the discontinuity property
	hr = pSource->IsDiscontinuity();
	if (hr == S_OK) {
		pDest->SetDiscontinuity(TRUE);
	}
	else if (hr == S_FALSE) {
		pDest->SetDiscontinuity(FALSE);
	}
	else {  // an unexpected error has occured...
		return E_UNEXPECTED;
	}

	return NOERROR;
}

//######################################
// Check the input type is OK - return an error otherwise
//######################################
HRESULT CHapDecoder::CheckInputType (const CMediaType *pMediaType) {
	NOTE("CheckInputType");

	CheckPointer(pMediaType, E_POINTER);

	// check this is a VIDEOINFOHEADER type
	if (*pMediaType->FormatType() != FORMAT_VideoInfo) {
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	// Check the format looks reasonably ok
	ULONG Length = pMediaType->FormatLength();
	if (Length < SIZE_VIDEOHEADER) {
		NOTE("Format smaller than a VIDEOHEADER");
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	// Check if the media major type is MEDIATYPE_Video
	const GUID *pMajorType = pMediaType->Type();
	if (*pMajorType != MEDIATYPE_Video) {
		NOTE("Major type not MEDIATYPE_Video");
		return VFW_E_TYPE_NOT_ACCEPTED;;
	}

	// Check if the media subtype is a supported Hap type
	const GUID *pSubType = pMediaType->Subtype();
	if (*pSubType != MEDIASUBTYPE_Hap1 && *pSubType != MEDIASUBTYPE_Hap5 && *pSubType != MEDIASUBTYPE_HapY) return VFW_E_TYPE_NOT_ACCEPTED;;

	return S_OK;
}

//######################################
// GetMediaType
// Returns one of the filter's preferred output types, referenced by index number
//######################################
HRESULT CHapDecoder::GetMediaType (int iPosition, CMediaType *pMediaType) {

	// Is the input pin connected
	if (m_pInput->IsConnected() == FALSE) return E_UNEXPECTED;

	// This should never happen
	if (iPosition < 0) return E_INVALIDARG;

	// Do we have more items to offer
	if (iPosition > 3) return VFW_S_NO_MORE_ITEMS;

	CheckPointer(pMediaType, E_POINTER);

	// The ConnectionMediaType method retrieves the media type for the current pin connection, if any.
	HRESULT hr = m_pInput->ConnectionMediaType(pMediaType);
	if (FAILED(hr))	return hr;

	//ASSERT(pMediaType->formattype == FORMAT_VideoInfo);

	VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(pMediaType->pbFormat);

	switch (iPosition) {

		case 0: // RGB32
			pMediaType->cbFormat = 88;
			pMediaType->subtype = MEDIASUBTYPE_RGB32;
			pMediaType->SetSampleSize(pVih->bmiHeader.biWidth * pVih->bmiHeader.biHeight * 4);
			pMediaType->SetTemporalCompression(FALSE);

			pVih->dwBitRate = (DWORD)floor(10000000.0 / pVih->AvgTimePerFrame * pVih->bmiHeader.biWidth * pVih->bmiHeader.biHeight * 32.0 + 0.5);

			pVih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			pVih->bmiHeader.biCompression = BI_RGB;
			pVih->bmiHeader.biSizeImage = pVih->bmiHeader.biWidth * pVih->bmiHeader.biHeight * 4;
			pVih->bmiHeader.biBitCount = 32;
			break;

		case 1: // DXT1
			pMediaType->cbFormat = 88;
			pMediaType->subtype = MEDIASUBTYPE_DXT1;
			pMediaType->SetSampleSize(pVih->bmiHeader.biWidth * pVih->bmiHeader.biHeight);
			pMediaType->SetTemporalCompression(FALSE);

			//dwBitRate: Approximate data rate of the video stream, in bits per second.
			pVih->dwBitRate = (DWORD)floor(10000000.0 / pVih->AvgTimePerFrame * pVih->bmiHeader.biWidth * pVih->bmiHeader.biHeight * 8.0 + 0.5);

			pVih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			pVih->bmiHeader.biCompression = FOURCC_DXT1;
			pVih->bmiHeader.biSizeImage = pVih->bmiHeader.biWidth * pVih->bmiHeader.biHeight;
			pVih->bmiHeader.biBitCount = 24;
			break;

		case 2: // DXT5
			pMediaType->cbFormat = 88;
			pMediaType->subtype = MEDIASUBTYPE_DXT5;
			pMediaType->SetSampleSize(pVih->bmiHeader.biWidth * pVih->bmiHeader.biHeight * 2);
			pMediaType->SetTemporalCompression(FALSE);

			pVih->dwBitRate = (DWORD)floor(10000000.0 / pVih->AvgTimePerFrame * pVih->bmiHeader.biWidth * pVih->bmiHeader.biHeight * 16.0 + 0.5);

			pVih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			pVih->bmiHeader.biCompression = FOURCC_DXT5;
			pVih->bmiHeader.biSizeImage = pVih->bmiHeader.biWidth * pVih->bmiHeader.biHeight * 2;
			pVih->bmiHeader.biBitCount = 32;
			break;

		case 3: // DXTY
			pMediaType->cbFormat = 88;
			pMediaType->subtype = MEDIASUBTYPE_DXTY;
			pMediaType->SetSampleSize(pVih->bmiHeader.biWidth * pVih->bmiHeader.biHeight * 2);
			pMediaType->SetTemporalCompression(FALSE);

			pVih->dwBitRate = (DWORD)floor(10000000.0 / pVih->AvgTimePerFrame * pVih->bmiHeader.biWidth * pVih->bmiHeader.biHeight * 16.0 + 0.5);

			pVih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			pVih->bmiHeader.biCompression = FOURCC_DXTY;
			pVih->bmiHeader.biSizeImage = pVih->bmiHeader.biWidth * pVih->bmiHeader.biHeight * 2;
			pVih->bmiHeader.biBitCount = 24;
			break;

	}

	return S_OK;
}

//######################################
// CheckTransform
// Check if a transform can be done between these formats
//######################################
HRESULT CHapDecoder::CheckTransform (const CMediaType *pMediaTypeIn, const CMediaType *pMediaTypeOut) {

	CheckPointer(pMediaTypeIn, E_POINTER);
	CheckPointer(pMediaTypeOut, E_POINTER);

	// check the input subtype - do we need this here?
	GUID subTypeIn = pMediaTypeIn->subtype;
	if (subTypeIn != MEDIASUBTYPE_Hap1 && subTypeIn != MEDIASUBTYPE_Hap5 && subTypeIn != MEDIASUBTYPE_HapY) {
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	// Check the output major type.
	if (pMediaTypeOut->majortype != MEDIATYPE_Video) {
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	// Check the output format type.
	if ((pMediaTypeOut->formattype != FORMAT_VideoInfo) || (pMediaTypeOut->cbFormat < sizeof(VIDEOINFOHEADER))) {
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(pMediaTypeIn->pbFormat);
	_width = pVih->bmiHeader.biWidth;
	_height = pVih->bmiHeader.biHeight;

	//######################################
	// Check the output subtype - it must be either RGB32, or a DXT format that fits to the input subtype
	//######################################
	GUID subTypeOut = pMediaTypeOut->subtype;	
	if (
		(subTypeOut == MEDIASUBTYPE_RGB32) ||
		(subTypeIn == MEDIASUBTYPE_Hap1 && subTypeOut == MEDIASUBTYPE_DXT1) || // Hap 1
		(subTypeIn == MEDIASUBTYPE_Hap5 && subTypeOut == MEDIASUBTYPE_DXT5) || // Hap Alpha
		(subTypeIn == MEDIASUBTYPE_HapY && subTypeOut == MEDIASUBTYPE_DXTY)    // Hap Q
	){
		m_subTypeIn = subTypeIn;
		m_subTypeOut = subTypeOut;
		return S_OK;
	}

	return VFW_E_TYPE_NOT_ACCEPTED;
}

//######################################
// DecideBufferSize
// Tell the output pin's allocator what size buffers we
// require. Can only do this when the input is connected
//######################################
HRESULT CHapDecoder::DecideBufferSize (IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties) {

	// Is the input pin connected
	if (m_pInput->IsConnected() == FALSE) return E_UNEXPECTED;
 
	CheckPointer(pAlloc, E_POINTER);
	CheckPointer(pProperties, E_POINTER);

	// Calculate needed output buffer size
	if (m_subTypeOut == MEDIASUBTYPE_RGB32) {

		// reserve extra buffers for conversion
		_dxtFlags = (m_subTypeIn == MEDIASUBTYPE_Hap1 ? squish::kDxt1 : squish::kDxt5);
		_dxtBufferSize = squish::GetStorageRequirements(_width, _height, _dxtFlags);

		_dxtBuffer = (unsigned char *)lag_aligned_malloc(_dxtBuffer, _dxtBufferSize, 16, "dxtBuffer");
		if (!_dxtBuffer) return E_OUTOFMEMORY;

		int bufferSize = align_round(_width * 4, 8) * _height + 2048;
		_tmpBuffer = (unsigned char *)lag_aligned_malloc(_tmpBuffer, bufferSize, 16, "tmpBuffer");
		if (!_tmpBuffer) {
			LAG_ALIGNED_FREE(_dxtBuffer, "dxtBuffer");
			return E_OUTOFMEMORY;
		}

		m_outputBufferSize = _width * _height * 4;

	}
	else { // forwarding raw DXT
		_dxtFlags = (m_subTypeIn == MEDIASUBTYPE_Hap1 ? squish::kDxt1 : squish::kDxt5);
		m_outputBufferSize = squish::GetStorageRequirements(_width, _height, _dxtFlags) * 2; // ???

	}

	// Ask the allocator to reserve us some sample memory, NOTE the function
	// can succeed (that is return NOERROR) but still not have allocated the
	// memory that we requested, so we must check we got whatever we wanted

	pProperties->cBuffers = 1;
	pProperties->cbBuffer = m_outputBufferSize;

	// Set allocator properties.
	ALLOCATOR_PROPERTIES Actual;
	HRESULT hr = pAlloc->SetProperties(pProperties, &Actual);
	if (FAILED(hr)) return hr;

	// Even when it succeeds, check the actual result.
	if (pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer) {
		return E_FAIL;
	}

	// store compression settings for this session
	m_generateTransparencyBackground = g_generateTransparencyBackground;
	m_useOMP = g_useOMP;

	return S_OK;
}

//######################################
// GetClassID
//######################################
STDMETHODIMP CHapDecoder::GetClassID (CLSID *pClsid) {
	return CBaseFilter::GetClassID(pClsid);
}

//######################################
// GetPages
//######################################
STDMETHODIMP CHapDecoder::GetPages(CAUUID *pPages) {
	CheckPointer(pPages,E_POINTER);

	pPages->cElems = 1;
	pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
	if (pPages->pElems == NULL) return E_OUTOFMEMORY;

	*(pPages->pElems) = CLSID_HapDecoderPropertyPage;
	return NOERROR;
}

////////////////////////////////////////////////////////////////////////
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
////////////////////////////////////////////////////////////////////////

//######################################
// DllRegisterServer
//######################################
STDAPI DllRegisterServer() {
	return AMovieDllRegisterServer2(TRUE);
}

//######################################
// DllUnregisterServer
//######################################
STDAPI DllUnregisterServer() {
	return AMovieDllRegisterServer2(FALSE);
}

//######################################
// DllEntryPoint
//######################################
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  dwReason, LPVOID lpReserved) {
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}
