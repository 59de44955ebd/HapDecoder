#pragma once

// {7A47FC31-8A4D-48B8-8674-F0D2F23BB0BA}
DEFINE_GUID(CLSID_HapDecoder,
	0x7a47fc31, 0x8a4d, 0x48b8, 0x86, 0x74, 0xf0, 0xd2, 0xf2, 0x3b, 0xb0, 0xba);

// {1957444A-1B60-4389-BA26-78595B890C31}
DEFINE_GUID(CLSID_HapDecoderPropertyPage,
	0x1957444a, 0x1b60, 0x4389, 0xba, 0x26, 0x78, 0x59, 0x5b, 0x89, 0xc, 0x31);

//######################################
// HAP TYPES
//######################################

//{31706148 - 0000 - 0010 - 8000 - 00AA00389B71} = Hap1
#define FOURCC_Hap1 (MAKEFOURCC('H','a','p','1'))
DEFINE_GUID(MEDIASUBTYPE_Hap1,
	FOURCC_Hap1, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

//{35706148-0000-0010-8000-00AA00389B71} = Hap5 = Hap Alpha
#define FOURCC_Hap5 (MAKEFOURCC('H','a','p','5'))
DEFINE_GUID(MEDIASUBTYPE_Hap5,
	FOURCC_Hap5, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

//{35706148-0000-0010-8000-00AA00389B71} = HapY = Hap Q
#define FOURCC_HapY (MAKEFOURCC('H','a','p','Y'))
DEFINE_GUID(MEDIASUBTYPE_HapY,
	FOURCC_HapY, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

//######################################
// DXT TYPES
//######################################

//{31545844-0000-0010-8000-00AA00389B71} // Hap
DEFINE_GUID(MEDIASUBTYPE_DXT1,
	FOURCC_DXT1, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

//{35545844-0000-0010-8000-00AA00389B71} // Hap Alpha
DEFINE_GUID(MEDIASUBTYPE_DXT5,
	FOURCC_DXT5, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

//{59545844-0000-0010-8000-00AA00389B71} // Hap Q
#define FOURCC_DXTY (MAKEFOURCC('D','X','T','Y'))
DEFINE_GUID(MEDIASUBTYPE_DXTY,
	FOURCC_DXTY, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
