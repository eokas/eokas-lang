
#ifndef  _EOKAS_BASE_PIXELS_H_
#define  _EOKAS_BASE_PIXELS_H_

#include "header.h"

_BeginNamespace(eokas)

enum class PixelFormat
{
    Unknown						= 0x0000,

	A8_UNORM                    = 0x0101,
	R8_TYPELESS                 = 0x0102,
	R8_UINT                     = 0x0103,
	R8_SINT                     = 0x0104,
	R8_UNORM                    = 0x0105,
	R8_SNORM                    = 0x0106,
    
    R16_TYPELESS                = 0x0201,
    R16_FLOAT                   = 0x0202,
	R16_UINT                    = 0x0203,
    R16_SINT                    = 0x0204,
	R16_UNORM                   = 0x0205,
	R16_SNORM                   = 0x0206,   
	D16_UNORM                   = 0x0207,
    R8G8_TYPELESS               = 0x0208,
	R8G8_UINT                   = 0x0209,
    R8G8_SINT                   = 0x020A,
	R8G8_UNORM                  = 0x020B, 
	R8G8_SNORM                  = 0x020C,
	B5G6R5_UNORM                = 0x020D,
    B5G5R5A1_UNORM              = 0x020E,
	R4G4B4A4_UNORM				= 0x020F,
	B4G4R4A4_UNORM				= 0x0210,

	R8G8B8_UNORM				= 0x0301,
	B8G8R8_UNORM				= 0x0302,

	R32_TYPELESS                = 0x0401,   
    R32_FLOAT                   = 0x0402,
    R32_UINT                    = 0x0403,
    R32_SINT                    = 0x0404,
	D32_FLOAT                   = 0x0405,
	R24G8_TYPELESS              = 0x0406,
	R16G16_TYPELESS             = 0x0407,
    R16G16_FLOAT                = 0x0408,
    R16G16_UINT                 = 0x0409,
	R16G16_SINT                 = 0x040A,
	R16G16_UNORM                = 0x040B,   
    R16G16_SNORM                = 0x040C,    
    R11G11B10_FLOAT             = 0x040D,
    R10G10B10A2_TYPELESS        = 0x040E,
	R10G10B10A2_UINT            = 0x040F,
    R10G10B10A2_UNORM           = 0x0410,   
	R8G8B8A8_TYPELESS           = 0x0411,
	R8G8B8A8_UINT               = 0x0412,
	R8G8B8A8_SINT               = 0x0413,
    R8G8B8A8_UNORM              = 0x0414,
    R8G8B8A8_UNORM_SRGB         = 0x0415,   
    R8G8B8A8_SNORM              = 0x0416,    
	B8G8R8A8_TYPELESS           = 0x0417,
	B8G8R8A8_UNORM              = 0x0418,   	
    B8G8R8A8_UNORM_SRGB         = 0x0419,
    B8G8R8X8_TYPELESS           = 0x041A,
	B8G8R8X8_UNORM              = 0x041B,
    B8G8R8X8_UNORM_SRGB         = 0x041C,	
	D24_UNORM_S8_UINT           = 0x041D,
    R24_UNORM_X8_TYPELESS       = 0x041E,
    X24_TYPELESS_G8_UINT        = 0x041F,
    
	R16G16B16_TYPELESS			= 0x0601,
	R16G16B16_FLOAT				= 0x0602,
	R16G16B16_UINT				= 0x0603,
	R16G16B16_SINT				= 0x0604,
	R16G16B16_UNORM				= 0x0605,
	R16G16B16_SNORM				= 0x0606,

	R64_TYPELESS				= 0x0801,
	R64_FLOAT					= 0x0802,
	R64_UINT					= 0x0803,
	R64_SINT					= 0x0804,
	R32G32_TYPELESS             = 0x0805,
    R32G32_FLOAT                = 0x0806,
    R32G32_UINT                 = 0x0807,
    R32G32_SINT                 = 0x0808,
	R32G8X24_TYPELESS           = 0x0809,
	R16G16B16A16_TYPELESS       = 0x080A,
    R16G16B16A16_FLOAT          = 0x080B,
	R16G16B16A16_UINT           = 0x080C,
	R16G16B16A16_SINT           = 0x080D,
    R16G16B16A16_UNORM          = 0x080E,    
    R16G16B16A16_SNORM          = 0x080F,
	D32_FLOAT_S8X24_UINT        = 0x0810,
    R32_FLOAT_X8X24_TYPELESS    = 0x0811,
    X32_TYPELESS_G8X24_UINT     = 0x0812,
    
	R32G32B32_TYPELESS          = 0x0C01,
    R32G32B32_FLOAT             = 0x0C02,
    R32G32B32_UINT              = 0x0C03,
    R32G32B32_SINT              = 0x0C04,

	R64B64_TYPELESS				= 0x1001,
	R64B64_FLOAT				= 0x1002,
	R64B64_UINT					= 0x1003,
	R64B64_SINT					= 0x1004,
    R32G32B32A32_TYPELESS		= 0x1001,
    R32G32B32A32_FLOAT			= 0x1002,
    R32G32B32A32_UINT           = 0x1003,
    R32G32B32A32_SINT           = 0x1004,
};

// compute the byte count of a pixel-format
#define _PixelFormatSize(pixFmt) ((((u32_t)(pixFmt)) & 0x0000FF00) >> 8)

#define _R4G4B4A4(r, g, b, a) (((r)&0x0F<<12) | ((g)&0x0F<<8) | ((b)&0x0F<<4) | ((a)&0x0F))
#define _R4G4B4A4_R(color) ((color)>>12&0x0F)
#define _R4G4B4A4_G(color) ((color)>>8&0x0F)
#define _R4G4B4A4_B(color) ((color)>>4&0x0F)
#define _R4G4B4A4_A(color) ((color)&0x0F)

#define _R5G5B5A1(r, g, b, a) (((r)&0x1F<<11) | ((g)&0x1F<<6) | ((b)&0x1F<<1) | ((a)&0x01))
#define _R5G5B5A1_R(color) ((color)>>11&0x1F)
#define _R5G5B5A1_G(color) ((color)>>6&0x1F)
#define _R5G5B5A1_B(color) ((color)>>1&0x1F)
#define _R5G5B5A1_A(color) ((color)&0x1)

#define _R5G5B5X1(r, g, b) _R5G5B5A1(r, g, b, 1)
#define _R5G5B5X1_R(color) _R5G5B5A1_R(color)
#define _R5G5B5X1_G(color) _R5G5B5A1_G(color)
#define _R5G5B5X1_B(color) _R5G5B5A1_B(color)

#define _R5G6B5(r, g, b) (((r)&0x1F<<11) | ((g)&0x3F<<5) | ((b)&0x1F))
#define _R5G6B5_R(color) ((color)>>11&0x1F)
#define _R5G6B5_G(color) ((color)>>5&0x3F)
#define _R5G6B5_B(color) ((color)&0x1F)

#define _R8G8B8A8(r, g, b, a) (((r)&0xFF<<24) | ((g)&0xFF<<16) | ((b)&0xFF<<8) | ((a)&0xFF))
#define _R8G8B8A8_R(color) ((color)>>24&0xFF)
#define _R8G8B8A8_G(color) ((color)>>16&0xFF)
#define _R8G8B8A8_B(color) ((color)>>8&0xFF)
#define _R8G8B8A8_A(color) ((color)&0xFF)

#define _R8G8B8X8(r, g, b) _R8G8B8A8(r, g, b, 0xFF)
#define _R8G8B8X8_R(color) _R8G8B8A8_R(color)
#define _R8G8B8X8_G(color) _R8G8B8A8_G(color)
#define _R8G8B8X8_B(color) _R8G8B8A8_B(color)

#define _R16G16B16A16(r, g, b, a) (((r)&0xFFFF<<48) | ((g)&0xFFFF<<32) | ((b)&0xFFFF<<16) | ((a)&0xFFFF))
#define _R16G16B16A16_R(color) ((color)>>48&0xFFFF)
#define _R16G16B16A16_G(color) ((color)>>32&0xFFFF)
#define _R16G16B16A16_B(color) ((color)>>16&0xFFFF)
#define _R16G16B16A16_A(color) ((color)&0xFFFF)

class Pixelmap
{
public:
	Pixelmap();
	Pixelmap(const Pixelmap& pxmp);
	Pixelmap(u32_t width, u32_t height, PixelFormat format, void* data = nullptr);
	Pixelmap(const Pixelmap& pxmp, u32_t x, u32_t y, u32_t w, u32_t h);	
	virtual ~Pixelmap();

public:
	u32_t width() const;
	u32_t height() const;
	PixelFormat format() const;
	void* const data() const;
	Pixelmap getArea(u32_t x, u32_t y, u32_t w, u32_t h);
	void setArea(u32_t x, u32_t y, const Pixelmap& pxmp);
	void clear();

private:
	u32_t mWidth;
	u32_t mHeight;
	PixelFormat mFormat;
	u8_t* mData;
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_PIXELS_H_
